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

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeReference;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

public class AttributeGroupReference
    extends Node
    implements INodeReference
{
    public AttributeGroupReference (
        final QualifiedName aReferencedElementName,
        final Location aLocation)
    {
        super(null, null, aLocation);
        maReferencedElement = aReferencedElementName;
    }




    @Override
    public INode GetOnlyChild ()
    {
        throw new RuntimeException("AttributeGroupReference has no children");
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    public QualifiedName GetReferencedName ()
    {
        return maReferencedElement;
    }




    public AttributeGroup GetReferencedAttributeGroup (final SchemaBase aSchemaBase)
    {
        return aSchemaBase.AttributeGroups.Get(maReferencedElement);
    }




    @Override
    public INode GetReferencedNode (final SchemaBase aSchemaBase)
    {
        return GetReferencedAttributeGroup(aSchemaBase);
    }




    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.AttributeGroupReference;
    }




    @Override
    public String toString ()
    {
        return "reference to attribute group "+maReferencedElement.GetDisplayName();
    }




    private final QualifiedName maReferencedElement;
}
