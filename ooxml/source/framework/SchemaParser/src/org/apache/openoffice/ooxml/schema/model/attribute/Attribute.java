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

package org.apache.openoffice.ooxml.schema.model.attribute;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.parser.FormDefault;

/* Representation of a single attribute.
 */
public class Attribute
    extends AttributeBase
{
    public Attribute (
        final QualifiedName aName,
        final QualifiedName aTypeName,
        final String sUse,
        final String sDefault,
        final String sFixed,
        final FormDefault eFormDefault,
        final Location aLocation)
    {
        super(aName, sUse, sDefault, sFixed, eFormDefault, aLocation);
        maTypeName = aTypeName;
    }




    public QualifiedName GetTypeName ()
    {
        return maTypeName;
    }




    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.Attribute;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    @Override
    public String toString ()
    {
        return String.format(
            "attribute %s of type %s, %s",
            GetName().GetDisplayName(),
            maTypeName.GetDisplayName(),
            super.toString());
    }




    private final QualifiedName maTypeName;
}
