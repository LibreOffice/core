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
import java.util.regex.Pattern;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;

/** Representation of the 'restriction' XML schema element.
 *  It defines constraints on a another simple type.
 *  Examples for such restrictions are minimum and maximum values
 *  (inclusive or exclusive), patterns or length constraints of strings,
 *  sets of valid values.
 */
public class Restriction
    extends Node
{
    public Restriction (
        final Node aParent,
        final String sBaseType,
        final Location aLocation)
    {
        super(aParent, null, aLocation);
        msBaseType = sBaseType;
        maEnumerations = new TreeSet<>();
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




    @Override
    public String toString ()
    {
        final StringBuffer aBuffer = new StringBuffer("restriction based on ");
        aBuffer.append(msBaseType);

        if (mnMinInclusive != null)
        {
            aBuffer.append(",minInclusive=");
            aBuffer.append(mnMinInclusive);
        }
        if (mnMinExclusive != null)
        {
            aBuffer.append(",minExclusive=");
            aBuffer.append(mnMinExclusive);
        }
        if (mnMaxInclusive != null)
        {
            aBuffer.append(",maxInclusive=");
            aBuffer.append(mnMaxInclusive);
        }
        if (mnMaxExclusive != null)
        {
            aBuffer.append(",maxExclusive=");
            aBuffer.append(mnMaxExclusive);
        }
        if (mnLength != null)
        {
            aBuffer.append(",length=");
            aBuffer.append(mnLength);
        }
        if (mnMinLength != null)
        {
            aBuffer.append(",minLength=");
            aBuffer.append(mnMinLength);
        }
        if (mnMaxLength != null)
        {
            aBuffer.append(",maxLength=");
            aBuffer.append(mnMaxLength);
        }
        if (maPattern != null)
        {
            aBuffer.append(",pattern=\"");
            aBuffer.append(maPattern);
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
    }




    public void SetMinInclusive (final String sValue)
    {
        mnMinInclusive = Double.parseDouble(sValue);
    }




    public void SetMinExclusive (final String sValue)
    {
        mnMinExclusive = Double.parseDouble(sValue);
    }




    public void SetMaxInclusive (final String sValue)
    {
        mnMaxInclusive = Double.parseDouble(sValue);
    }




    public void SetMaxExclusive (final String sValue)
    {
        mnMaxExclusive = Double.parseDouble(sValue);
    }




    public void SetLength (final String sValue)
    {
        mnLength = Integer.parseInt(sValue);
    }




    public void SetMinLength (final String sValue)
    {
        mnMinLength = Integer.parseInt(sValue);
    }




    public void SetMaxLength (final String sValue)
    {
        mnMaxLength = Integer.parseInt(sValue);
    }




    public void SetPattern (final String sValue)
    {
        maPattern = Pattern.compile(sValue);
    }




    private final String msBaseType;
    private final Set<String> maEnumerations;
    private Double mnMinInclusive;
    private Double mnMinExclusive;
    private Double mnMaxInclusive;
    private Double mnMaxExclusive;
    private Integer mnLength;
    private Integer mnMinLength;
    private Integer mnMaxLength;
    private Pattern maPattern;
}
