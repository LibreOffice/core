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

package org.apache.openoffice.ooxml.schema.iterator;

import java.util.Iterator;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeReference;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Iterate over all nodes in a node tree.  References to groups and elements
 *  are resolved and the referenced nodes are included in the iteration.
 */
public class DereferencingNodeIterator
    implements Iterable<INode>

{
    public DereferencingNodeIterator (
        final INode aRoot,
        final SchemaBase aSchemaBase,
        final boolean bIncludeReferencingNodes)
    {
        maRoot = aRoot;
        maSchemaBase = aSchemaBase;
        mbIncludeReferencingNodes = bIncludeReferencingNodes;
    }




    @Override
    public Iterator<INode> iterator()
    {
        return CollectNodes(maRoot, mbIncludeReferencingNodes).iterator();
    }




    private Vector<INode> CollectNodes (
        final INode aRoot,
        final boolean bIncludeReferencingNodes)
    {
        final Vector<INode> aNodes = new Vector<>();
        AddNodes(aNodes, aRoot, bIncludeReferencingNodes);
        return aNodes;
    }




    private void AddNodes (
        final Vector<INode> aNodes,
        final INode aRoot,
        final boolean bIncludeReferencingNodes)
    {
        for (final INode aNode : new NodeIterator(aRoot))
        {
            switch(aNode.GetNodeType())
            {
                case AttributeGroupReference:
                case AttributeReference:
                case ComplexTypeReference:
                case ElementReference:
                case GroupReference:
                case SimpleTypeReference:
                case Extension:
                    if (mbIncludeReferencingNodes)
                        aNodes.add(aNode);
                    for (final INode aChild : new DereferencingNodeIterator(
                        ((INodeReference)aNode).GetReferencedNode(maSchemaBase),
                        maSchemaBase,
                        bIncludeReferencingNodes))
                    {
                        aNodes.add(aChild);
                    }
                    break;

                default:
                    aNodes.add(aNode);
            }
        }
    }




    private final INode maRoot;
    private final SchemaBase maSchemaBase;
    private final boolean mbIncludeReferencingNodes;
}
