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

package org.apache.openoffice.ooxml.schema.model.complex;

import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeReference;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Representation of the 'extension' XML schema element.
 *  It extends a complex base type.
 */
public class Extension
    extends Node
    implements INodeReference
{
    public Extension (
        final Node aParent,
        final QualifiedName aBaseTypeName,
        final Location aLocation)
    {
        super(aParent, null, aLocation);
        maBaseTypeName = aBaseTypeName;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    @Override
    public NodeType GetNodeType()
    {
        return NodeType.Extension;
    }




    public QualifiedName GetBaseTypeName()
    {
        return maBaseTypeName;
    }




    public INode GetBaseType (final SchemaBase aSchema)
    {
        return aSchema.GetTypeForName(maBaseTypeName);
    }




    @Override
    public INode GetReferencedNode (final SchemaBase aSchema)
    {
        return GetBaseType(aSchema);
    }




    public Vector<INode> GetTypeNodes (final SchemaBase aSchemaBase)
    {
        final Vector<INode> aNodes = new Vector<>();

        AddNodes(aSchemaBase.GetTypeForName(maBaseTypeName), aNodes, aSchemaBase);
        for (final INode aChild : GetChildren())
            AddNodes(aChild, aNodes, aSchemaBase);

        return aNodes;
    }




    @Override
    public String toString ()
    {
        return "extension of base type "+maBaseTypeName.GetDisplayName();
    }




    private void AddNodes (
        final INode aParent,
        final Vector<INode> aNodes,
        final SchemaBase aSchemaBase)
    {
        INode aNode = aParent;
        while (true)
        {
            switch (aNode.GetNodeType())
            {
                case Extension:
                    aNode = ((Extension)aNode).GetBaseType(aSchemaBase);
                    break;

                case ComplexContent:
                case SimpleContent:
                case ComplexType:
                    switch(aNode.GetChildCount())
                    {
                        case 0:
                            return;
                        case 1:
                            aNode = aNode.GetOnlyChild();
                            break;
                        default:
                            throw new RuntimeException();
                    }
                    break;

                default:
                    aNodes.add(aNode);
                    return;
            }
        }
    }




    public QualifiedName maBaseTypeName;
}
