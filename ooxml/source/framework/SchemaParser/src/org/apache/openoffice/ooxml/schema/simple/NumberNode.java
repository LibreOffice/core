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

package org.apache.openoffice.ooxml.schema.simple;

import java.util.Map;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltInType;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

public class NumberNode<T extends Comparable<T>>
    implements ISimpleTypeNode
{
    NumberNode (final BuiltInType eType)
    {
        meType = eType;
        mbIsList = false;
    }




    public BuiltInType GetNumberType ()
    {
        return meType;
    }




    T ParseString (final String sValue)
    {
        switch(meType)
        {
            case Float:
                return (T)(Float)Float.parseFloat(sValue);

            case Double:
                return (T)(Double)Double.parseDouble(sValue);

            case Byte:
                return (T)(Byte)Byte.parseByte(sValue);

            case Int:
            case UnsignedShort:
                return (T)(Integer)Integer.parseInt(sValue);

            case Short:
            case UnsignedByte:
                return (T)(Short)Short.parseShort(sValue);

            case Long:
            case UnsignedInt:
            case Integer:
                return (T)(Long)Long.parseLong(sValue);

            default:
                throw new RuntimeException("unsupported type "+meType);
        }
    }




    @Override
    public void ApplyRestriction (
        final Restriction aNode,
        final Map<String,Integer> aValueToIdMap)
    {
        if (aNode.HasFeature(Restriction.MinExclusiveBit))
            ApplyMinimum(ParseString(aNode.GetMinExclusive()), false);
        if (aNode.HasFeature(Restriction.MinInclusiveBit))
            ApplyMinimum(ParseString(aNode.GetMinInclusive()), true);

        if (aNode.HasFeature(Restriction.MaxExclusiveBit))
            ApplyMaximum(ParseString(aNode.GetMaxExclusive()), false);
        if (aNode.HasFeature(Restriction.MaxInclusiveBit))
            ApplyMaximum(ParseString(aNode.GetMaxInclusive()), true);

        if (aNode.HasFeature(Restriction.EnumerationBit))
        {
            final Vector<T> aValues = new Vector<>();
            for (final String sEnumerationValue : aNode.GetEnumeration())
                aValues.add(ParseString(sEnumerationValue));
            ApplyEnumeration(aValues);
        }
    }




    @Override
    public void Print (final Log aLog)
    {
        aLog.printf("%s\n", toString());
    }




    @Override
    public String toString ()
    {
        final StringBuffer sMessage = new StringBuffer();
        sMessage.append(meType);
        if (maEnumeration != null)
        {
            sMessage.append(", restricted to values");
            for (final T nValue : maEnumeration)
            {
                sMessage.append(' ');
                sMessage.append(nValue);
            }
        }
        else if (maMinimumValue!=null || maMaximumValue!=null)
        {
            sMessage.append(" restricted to ");

            if (maMinimumValue != null)
            {
                sMessage.append(maMinimumValue);
                if (mbIsMinimumInclusive)
                    sMessage.append(" <= ");
                else
                    sMessage.append(" < ");
            }
            sMessage.append("value");
            if (maMaximumValue != null)
            {
                if (mbIsMaximumInclusive)
                    sMessage.append(" <= ");
                else
                    sMessage.append(" < ");
                sMessage.append(maMaximumValue);
            }
        }
        else
            sMessage.append(", not restricted");

        return sMessage.toString();
    }




    private void ApplyMinimum (
        final T nValue,
        final boolean bIsInclusive)
    {
        if (maEnumeration != null)
            throw new RuntimeException("minimum can not be applied to an enumeration");
        else if (maMinimumValue != null)
        {
            final int nComparison = maMinimumValue.compareTo(nValue);
            if (nComparison > 0)
                throw new RuntimeException("second restriction tries to enlarge value space");
            else if (nComparison == 0)
                if (mbIsMinimumInclusive && ! bIsInclusive)
                    throw new RuntimeException("second restriction tries to enlarge value space");
        }
        maMinimumValue = nValue;
        mbIsMinimumInclusive = bIsInclusive;
    }





    private void ApplyMaximum (
        final T nValue,
        final boolean bIsInclusive)
    {
        if (maEnumeration != null)
            throw new RuntimeException("maximum can not be applied to an enumeration");
        else if (maMaximumValue != null)
        {
            final int nComparison = maMaximumValue.compareTo(nValue);
            if (nComparison < 0)
                throw new RuntimeException("second restriction tries to enlarge value space");
            else if (nComparison == 0)
                if ( ! mbIsMaximumInclusive && bIsInclusive)
                    throw new RuntimeException("second restriction tries to enlarge value space");
        }
        maMaximumValue = nValue;
        mbIsMaximumInclusive = bIsInclusive;
    }




    private void ApplyEnumeration (final Vector<T> aValues)
    {
        if (maEnumeration!=null || maMaximumValue!=null || maMinimumValue!=null)
            throw new RuntimeException("can not apply enumeration to existing restriction");
        maEnumeration = aValues;
    }




    @Override
    public boolean IsList ()
    {
        return mbIsList;
    }




    @Override
    public void SetIsList ()
    {
        mbIsList = true;
    }




    @Override
    public void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }


    public enum RestrictionType
    {
        Size,
        Enumeration,
        None
    }
    public RestrictionType GetRestrictionType ()
    {
        if (maEnumeration != null)
            return RestrictionType.Enumeration;
        else if (maMinimumValue!=null || maMaximumValue!=null)
            return RestrictionType.Size;
        else
            return RestrictionType.None;
    }



    public Iterable<T> GetEnumerationRestriction ()
    {
        return maEnumeration;
    }




    public T GetMinimum ()
    {
        return maMinimumValue;
    }




    public T GetMaximum ()
    {
        return maMaximumValue;
    }



    public boolean IsMinimumInclusive ()
    {
        return mbIsMinimumInclusive;
    }




    public boolean IsMaximumInclusive ()
    {
        return mbIsMaximumInclusive;
    }




    private final BuiltInType meType;
    private T maMinimumValue;
    private boolean mbIsMinimumInclusive;
    private T maMaximumValue;
    private boolean mbIsMaximumInclusive;
    private Vector<T> maEnumeration;
    private boolean mbIsList;
}
