/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <cmath>
#include "parsing/amf3_generator.h"
#include "scripting/argconv.h"
#include "scripting/toplevel/Integer.h"
#include "scripting/flash/utils/ByteArray.h"

using namespace std;
using namespace lightspark;

ASFUNCTIONBODY_ATOM(Integer,_toString)
{
	if(Class<Integer>::getClass(sys)->prototype->getObj() == obj.getObject())
	{
		ret = asAtom::fromString(sys,"0");
		return;
	}

	int radix=10;
	if(argslen==1)
		radix=args[0].toUInt();

	if(radix==10)
	{
		char buf[20];
		snprintf(buf,20,"%i",obj.toInt());
		ret = asAtom::fromObject(abstract_s(sys,buf));
	}
	else
	{
		tiny_string s=Number::toStringRadix(obj.toNumber(), radix);
		ret = asAtom::fromObject(abstract_s(sys,s));
	}
}

ASFUNCTIONBODY_ATOM(Integer,_valueOf)
{
	if(Class<Integer>::getClass(sys)->prototype->getObj() == obj.getObject())
	{
		ret.setInt(0);
		return;
	}

	if(!obj.is<Integer>())
			throw Class<TypeError>::getInstanceS(sys,"");

	ASATOM_INCREF(obj);
	ret = obj;
}

ASFUNCTIONBODY_ATOM(Integer,_constructor)
{
	Integer* th=obj.as<Integer>();
	if(argslen==0)
	{
		//The int is already initialized to 0
		return;
	}
	th->val=args[0].toInt();
}

ASFUNCTIONBODY_ATOM(Integer,generator)
{
	if (argslen == 0)
		ret = asAtom((int32_t)0);
	else
		ret = asAtom(args[0].toInt());
}

TRISTATE Integer::isLess(ASObject* o)
{
	switch(o->getObjectType())
	{
		case T_INTEGER:
			{
				Integer* i=static_cast<Integer*>(o);
				return (val < i->toInt())?TTRUE:TFALSE;
			}
			break;

		case T_UINTEGER:
			{
				UInteger* i=static_cast<UInteger*>(o);
				return (val < 0 || ((uint32_t)val)  < i->val)?TTRUE:TFALSE;
			}
			break;
		
		case T_NUMBER:
			{
				Number* i=static_cast<Number*>(o);
				if(std::isnan(i->toNumber())) return TUNDEFINED;
				return (val < i->toNumber())?TTRUE:TFALSE;
			}
			break;
		
		case T_STRING:
			{
				double val2=o->toNumber();
				if(std::isnan(val2)) return TUNDEFINED;
				return (val<val2)?TTRUE:TFALSE;
			}
			break;
		
		case T_BOOLEAN:
			{
				Boolean* i=static_cast<Boolean*>(o);
				return (val < i->toInt())?TTRUE:TFALSE;
			}
			break;
		
		case T_UNDEFINED:
			{
				return TUNDEFINED;
			}
			break;
			
		case T_NULL:
			{
				return (val < 0)?TTRUE:TFALSE;
			}
			break;

		default:
			break;
	}
	
	asAtom val2p;
	o->toPrimitive(val2p);
	double val2=val2p.toNumber();
	if(std::isnan(val2)) return TUNDEFINED;
	return (val<val2)?TTRUE:TFALSE;
}

bool Integer::isEqual(ASObject* o)
{
	switch(o->getObjectType())
	{
		case T_INTEGER:
			return val==o->toInt();
		case T_UINTEGER:
			return val >= 0 && val==o->toInt();
		case T_NUMBER:
			return val==o->toNumber();
		case T_BOOLEAN:
			return val==o->toInt();
		case T_STRING:
			return val==o->toNumber();
		case T_NULL:
		case T_UNDEFINED:
			return false;
		default:
			return o->isEqual(this);
	}
}

tiny_string Integer::toString()
{
	return Integer::toString(val);
}

/* static helper function */
tiny_string Integer::toString(int32_t val)
{
	char buf[20];
	if(val<0)
	{
		//This can be a slow path, as it not used for array access
		snprintf(buf,20,"%i",val);
		return tiny_string(buf,true);
	}
	buf[19]=0;
	char* cur=buf+19;

	int v=val;
	do
	{
		cur--;
		*cur='0'+(v%10);
		v/=10;
	}
	while(v!=0);
	return tiny_string(cur,true); //Create a copy
}

void Integer::sinit(Class_base* c)
{
	CLASS_SETUP(c, ASObject, _constructor, CLASS_SEALED | CLASS_FINAL);
	c->isReusable = true;
	c->setVariableAtomByQName("MAX_VALUE",nsNameAndKind(),asAtom(numeric_limits<int32_t>::max()),CONSTANT_TRAIT);
	c->setVariableAtomByQName("MIN_VALUE",nsNameAndKind(),asAtom(numeric_limits<int32_t>::min()),CONSTANT_TRAIT);
	c->setDeclaredMethodByQName("toString",AS3,Class<IFunction>::getFunction(c->getSystemState(),_toString),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toFixed",AS3,Class<IFunction>::getFunction(c->getSystemState(),_toFixed,1),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toExponential",AS3,Class<IFunction>::getFunction(c->getSystemState(),_toExponential,1),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toPrecision",AS3,Class<IFunction>::getFunction(c->getSystemState(),_toPrecision,1),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("valueOf",AS3,Class<IFunction>::getFunction(c->getSystemState(),_valueOf),NORMAL_METHOD,true);
	c->prototype->setVariableByQName("toExponential","",Class<IFunction>::getFunction(c->getSystemState(),Integer::_toExponential, 1),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toFixed","",Class<IFunction>::getFunction(c->getSystemState(),Integer::_toFixed, 1),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toPrecision","",Class<IFunction>::getFunction(c->getSystemState(),Integer::_toPrecision, 1),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toString","",Class<IFunction>::getFunction(c->getSystemState(),Integer::_toString),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("valueOf","",Class<IFunction>::getFunction(c->getSystemState(),_valueOf),DYNAMIC_TRAIT);
}

void Integer::serialize(ByteArray* out, std::map<tiny_string, uint32_t>& stringMap,
				std::map<const ASObject*, uint32_t>& objMap,
				std::map<const Class_base*, uint32_t>& traitsMap)
{
	if (out->getObjectEncoding() == ObjectEncoding::AMF0)
	{
		// write as double
		out->writeByte(amf0_number_marker);
		out->serializeDouble(val);
		return;
	}
	if(val>=0x40000000 || val<=(int32_t)0xbfffffff)
	{
		// write as double
		out->writeByte(double_marker);
		out->serializeDouble(val);
	}
	else
	{
		out->writeByte(integer_marker);
		out->writeU29((uint32_t)val);
	}
}

bool Integer::fromStringFlashCompatible(const char* cur, int64_t& ret, int radix)
{
	//Skip whitespace chars
	while(ASString::isEcmaSpace(g_utf8_get_char(cur)))
		cur = g_utf8_next_char(cur);

	int64_t multiplier=1;
	//Skip and take note of minus sign
	if(*cur=='-')
	{
		multiplier=-1;
		cur++;
	}
	if (radix == 0 && (g_str_has_prefix(cur,"0x") || g_str_has_prefix(cur,"0X")))
	{
		radix = 16;
		cur+=2;
	}
	//Skip leading zeroes
	if (radix == 0)
	{
		int count=0;
		while(*cur=='0')
		{
			cur++;
			count++;
		}

		//The string consisted of all zeroes
		if(count>0 && *cur=='\0')
		{
			ret = 0;
			return true;
		}
	}
	
	errno=0;
	char *end;
	ret=g_ascii_strtoll(cur, &end, radix);

	if(end==cur || errno==ERANGE)
		return false;

	ret*=multiplier;
	return true;
}

int32_t Integer::stringToASInteger(const char* cur, int radix)
{
	int64_t value;
	bool valid=Integer::fromStringFlashCompatible(cur, value, 0);

	if (!valid)
		return 0;
	else
		return static_cast<int32_t>(value & 0xFFFFFFFF);
}

ASFUNCTIONBODY_ATOM(Integer,_toExponential)
{
	double v = obj.toNumber();
	int32_t fractionDigits;
	ARG_UNPACK_ATOM(fractionDigits, 0);
	if (argslen == 0 || args[0].is<Undefined>())
	{
		if (v == 0)
			fractionDigits = 1;
		else
			fractionDigits = imin(imax((int32_t)ceil(::log10(::fabs(v))), 1), 20);
	}
	ret = asAtom::fromObject(abstract_s(sys,Number::toExponentialString(v, fractionDigits)));
}

ASFUNCTIONBODY_ATOM(Integer,_toFixed)
{
	int fractiondigits;
	ARG_UNPACK_ATOM (fractiondigits, 0);
	ret = asAtom::fromObject(abstract_s(sys,Number::toFixedString(obj.toNumber(), fractiondigits)));
}

ASFUNCTIONBODY_ATOM(Integer,_toPrecision)
{
	if (argslen == 0 || args[0].is<Undefined>())
	{
		ret = asAtom::fromObject(abstract_s(sys,obj.toString(sys)));
		return;
	}
	int precision;
	ARG_UNPACK_ATOM (precision);
	ret = asAtom::fromObject(abstract_s(sys,Number::toPrecisionString(obj.toNumber(), precision)));
}
