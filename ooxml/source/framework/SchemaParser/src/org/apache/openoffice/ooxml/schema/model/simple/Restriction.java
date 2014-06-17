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

package org.apache.openoffice.ooxml.schema.model.simple;

import java.util.Set;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Representation of the 'restriction' XML schema element.
 *  It defines constraints on a another simple type.
 *  Examples for such restrictions are minimum and maximum values
 *  (inclusive or exclusive), patterns or length constraints of strings,
 *  sets of valid values.
 */
public class Restriction
    extends Node
{
    public final static int MinInclusiveBit = 0x0001;
    public final static int MinExclusiveBit = 0x0002;
    public final static int MaxInclusiveBit = 0x0004;
    public final static int MaxExclusiveBit = 0x0008;
    public final static int LengthBit = 0x0010;
    public final static int MinLengthBit = 0x0020;
    public final static int MaxLengthBit = 0x0040;
    public final static int PatternBit = 0x0080;
    public final static int EnumerationBit = 0x0100;

    public Restriction (
        final Node aParent,
        final QualifiedName aBaseType,
        final Location aLocation)
    {
        super(aParent, null, aLocation);
        maBaseType = aBaseType;
        maEnumerations = new TreeSet<>();
        mnFeatures = 0;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    @Override
    public NodeType GetNodeType()
    {
        return NodeType.Restriction;
    }




    public QualifiedName GetBaseType ()
    {
        return maBaseType;
    }




    @Override
    public String toString ()
    {
        final StringBuffer aBuffer = new StringBuffer("restriction based on ");
        aBuffer.append(maBaseType.GetDisplayName());

        if (msMinInclusive != null)
        {
            aBuffer.append(",minInclusive=");
            aBuffer.append(msMinInclusive);
        }
        if (msMinExclusive != null)
        {
            aBuffer.append(",minExclusive=");
            aBuffer.append(msMinExclusive);
        }
        if (msMaxInclusive != null)
        {
            aBuffer.append(",maxInclusive=");
            aBuffer.append(msMaxInclusive);
        }
        if (msMaxExclusive != null)
        {
            aBuffer.append(",maxExclusive=");
            aBuffer.append(msMaxExclusive);
        }
        if (HasFeature(LengthBit))
        {
            aBuffer.append(",length=");
            aBuffer.append(mnLength);
        }
        if (HasFeature(MinLengthBit))
        {
            aBuffer.append(",minLength=");
            aBuffer.append(mnMinLength);
        }
        if (HasFeature(MaxLengthBit))
        {
            aBuffer.append(",maxLength=");
            aBuffer.append(mnMaxLength);
        }
        if (msPattern != null)
        {
            aBuffer.append(",pattern=\"");
            aBuffer.append(msPattern);
            aBuffer.append('"');
        }
        if ( ! maEnumerations.isEmpty())
        {
            aBuffer.append(",enumerations");
            aBuffer.append(maEnumerations);
        }
        return aBuffer.toString();
    }



    public void AddEnumeration (final String sValue)
    {
        maEnumerations.add(sValue);
        mnFeatures |= EnumerationBit;
    }




    public void SetMinInclusive (final String sValue)
    {
        msMinInclusive = sValue;
        assert( ! HasFeature(MinExclusiveBit));
        mnFeatures |= MinInclusiveBit;
    }




    public void SetMinExclusive (final String sValue)
    {
        msMinExclusive = sValue;
        assert( ! HasFeature(MinInclusiveBit));
        mnFeatures |= MinExclusiveBit;
    }




    public void SetMaxInclusive (final String sValue)
    {
        msMaxInclusive = sValue;
        assert( ! HasFeature(MaxExclusiveBit));
        mnFeatures |= MaxInclusiveBit;
    }




    public void SetMaxExclusive (final String sValue)
    {
        msMaxExclusive = sValue;
        assert( ! HasFeature(MaxInclusiveBit));
        mnFeatures |= MaxExclusiveBit;
    }




    public void SetLength (final String sValue)
    {
        mnLength = Integer.parseInt(sValue);
        assert( ! HasFeature(MinLengthBit|MaxLengthBit));
        mnFeatures |= LengthBit;
    }




    public void SetMinLength (final String sValue)
    {
        mnMinLength = Integer.parseInt(sValue);
        assert( ! HasFeature(LengthBit));
        mnFeatures |= MinLengthBit;
    }




    public void SetMaxLength (final String sValue)
    {
        mnMaxLength = Integer.parseInt(sValue);
        assert( ! HasFeature(LengthBit));
        mnFeatures |= MaxLengthBit;
    }




    public void SetPattern (final String sValue)
    {
        msPattern = sValue;
        mnFeatures |= PatternBit;
    }




    public String GetMinInclusive ()
    {
        return msMinInclusive;
    }




    public String GetMinExclusive ()
    {
        return msMinExclusive;
    }




    public String GetMaxInclusive ()
    {
        return msMaxInclusive;
    }




    public String GetMaxExclusive ()
    {
        return msMaxExclusive;
    }




    public Set<String> GetEnumeration()
    {
        return maEnumerations;
    }




    public int GetLength()
    {
        return mnLength;
    }




    public int GetMinimumLength()
    {
        return mnMinLength;
    }




    public int GetMaximumLength()
    {
        return mnMaxLength;
    }





    public String GetPattern()
    {
        return msPattern;
    }




    public int GetFeatureBits ()
    {
        return mnFeatures;
    }




    public boolean HasFeature (final int nBitMask)
    {
        return (mnFeatures & nBitMask) != 0;
    }




    private final QualifiedName maBaseType;
    private final Set<String> maEnumerations;
    private String msMinInclusive;
    private String msMinExclusive;
    private String msMaxInclusive;
    private String msMaxExclusive;
    private int mnLength;
    private int mnMinLength;
    private int mnMaxLength;
    private String msPattern;
    private int mnFeatures;
}
