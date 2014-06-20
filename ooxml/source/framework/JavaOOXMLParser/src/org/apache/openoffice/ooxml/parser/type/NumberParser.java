/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.parser.type;

import java.util.HashSet;
import java.util.Set;

import org.apache.openoffice.ooxml.parser.NameMap;

public class NumberParser implements ISimpleTypeParser
{
    public NumberParser (final String[] aLine)
    {
        switch(aLine[5])
        {
            case "u1":
                meNumberType = NumberType.Boolean;
                meJavaNumberType = JavaNumberType.Boolean;
                break;
            case "s8":
                meNumberType = NumberType.Byte;
                meJavaNumberType = JavaNumberType.Byte;
                break;
            case "u8":
                meNumberType = NumberType.UnsignedByte;
                meJavaNumberType = JavaNumberType.Short;
                break;
            case "s16":
                meNumberType = NumberType.Short;
                meJavaNumberType = JavaNumberType.Short;
                break;
            case "u16":
                meNumberType = NumberType.UnsignedShort;
                meJavaNumberType = JavaNumberType.Integer;
                break;
            case "s32":
                meNumberType = NumberType.Int;
                meJavaNumberType = JavaNumberType.Integer;
                break;
            case "u32":
                meNumberType = NumberType.UnsignedInt;
                meJavaNumberType = JavaNumberType.Long;
                break;
            case "s64":
                meNumberType = NumberType.Long;
                meJavaNumberType = JavaNumberType.Long;
                break;
            case "u64":
                meNumberType = NumberType.UnsignedLong;
                meJavaNumberType = JavaNumberType.Long;
                break;
            case "s*":
                meNumberType = NumberType.Integer;
                meJavaNumberType = JavaNumberType.Long;
                break;
            case "f":
                meNumberType = NumberType.Float;
                meJavaNumberType = JavaNumberType.Float;
                break;
            case "d":
                meNumberType = NumberType.Double;
                meJavaNumberType = JavaNumberType.Double;
                break;
            default:
                throw new RuntimeException("unsupported numerical type "+aLine[5]);
        }

        switch(aLine[6])
        {
            case "E":
                meRestrictionType = RestrictionType.Enumeration;
                maEnumeration = new HashSet<>();
                for (int nIndex=7; nIndex<aLine.length; ++nIndex)
                    maEnumeration.add(ParseNumber(aLine[nIndex]));
                break;

            case "S":
                meRestrictionType = RestrictionType.Size;
                for (int nIndex=7; nIndex<=9; nIndex+=2)
                    if (nIndex<aLine.length)
                        switch (aLine[nIndex])
                        {
                            case "<=":
                                maMaximumValue = ParseNumber(aLine[nIndex+1]);
                                mbIsMaximumInclusive = true;
                                break;
                            case "<":
                                maMaximumValue = ParseNumber(aLine[nIndex+1]);
                                mbIsMaximumInclusive = false;
                                break;
                            case ">=":
                                maMinimumValue = ParseNumber(aLine[nIndex+1]);
                                mbIsMinimumInclusive = true;
                                break;
                            case ">":
                                maMinimumValue = ParseNumber(aLine[nIndex+1]);
                                mbIsMinimumInclusive = false;
                                break;
                        }
                break;

            case "N":
                meRestrictionType = RestrictionType.None;
                break;

            default:
                throw new RuntimeException("unsupported numerical restriction "+aLine[6]);
        }
    }




    @Override
    public Object Parse(
        final String sRawValue,
        final NameMap aAttributeValueMap)
    {
        final Object aNumber = ParseNumber(sRawValue);
        switch(meRestrictionType)
        {
            case Enumeration:
                if (maEnumeration.contains(aNumber))
                    return aNumber;
                else
                    return null;

            case Size:
                if (maMinimumValue != null)
                    if (mbIsMinimumInclusive)
                    {
                        if (CompareTo(aNumber, maMinimumValue, meJavaNumberType) < 0)
                            return null;
                    }
                    else
                    {
                        if (CompareTo(aNumber, maMinimumValue, meJavaNumberType) <= 0)
                            return null;
                    }
                if (maMaximumValue != null)
                    if (mbIsMaximumInclusive)
                    {
                        if (CompareTo(aNumber, maMaximumValue, meJavaNumberType) > 0)
                            return null;
                    }
                    else
                    {
                        if (CompareTo(aNumber, maMaximumValue, meJavaNumberType) >= 0)
                            return null;
                    }
                return aNumber;

            case None:
                return aNumber;

            default:
                throw new RuntimeException();
        }
    }




    Object ParseNumber (final String sNumber)
    {
        switch(meJavaNumberType)
        {
            case Boolean: return Boolean.parseBoolean(sNumber);
            case Byte: return Byte.parseByte(sNumber);
            case Short: return Short.parseShort(sNumber);
            case Integer: return Integer.parseInt(sNumber);
            case Long: return Long.parseLong(sNumber);
            case Float: return Float.parseFloat(sNumber);
            case Double: return Double.parseDouble(sNumber);
            default:
                throw new RuntimeException();
        }
    }




    private static int CompareTo (
        final Object aLeft,
        final Object aRight,
        final JavaNumberType eType)
    {
        switch(eType)
        {
            case Boolean:
                return ((Boolean)aLeft).compareTo((Boolean)aRight);
            case Byte:
                return ((Byte)aLeft).compareTo((Byte)aRight);
            case Short:
                return ((Short)aLeft).compareTo((Short)aRight);
            case Integer:
                return ((Integer)aLeft).compareTo((Integer)aRight);
            case Long:
                return ((Long)aLeft).compareTo((Long)aRight);
            case Float:
                return ((Float)aLeft).compareTo((Float)aRight);
            case Double:
                return ((Double)aLeft).compareTo((Double)aRight);
            default:
                throw new RuntimeException();
        }
    }




    enum NumberType
    {
        Boolean,
        Byte,
        UnsignedByte,
        Short,
        UnsignedShort,
        Int,
        UnsignedInt,
        Long,
        UnsignedLong,
        Integer,
        Float,
        Double
    }
    enum JavaNumberType
    {
        Boolean,
        Byte,
        Short,
        Integer,
        Long,
        Float,
        Double
    }
    enum RestrictionType
    {
        Enumeration,
        Size,
        None
    }
    private final NumberType meNumberType;
    private final JavaNumberType meJavaNumberType;
    private final RestrictionType meRestrictionType;
    private Set<Object> maEnumeration;
    private Object maMinimumValue;
    private boolean mbIsMinimumInclusive;
    private Object maMaximumValue;
    private boolean mbIsMaximumInclusive;
}
