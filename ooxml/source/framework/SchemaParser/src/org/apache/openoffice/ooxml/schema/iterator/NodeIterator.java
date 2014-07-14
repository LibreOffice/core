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

/** Iterate over all nodes in a node tree.
 *  The root node is typically a simple or complex type.
 *  References to elements or groups etc. are not followed.  Use the
 *  DereferencingNodeIterator for that.
 */
public class NodeIterator
    implements Iterable<INode>
{
    public NodeIterator (final INode aRoot)
    {
        maNodes = new Vector<>();
        AddNodes(aRoot);
    }




    @Override
    public Iterator<INode> iterator ()
    {
        return maNodes.iterator();
    }




    /** Recursively add child nodes depth first.
     */
    private void AddNodes (final INode aNode)
    {
        maNodes.add(aNode);
        for (final INode aChild : aNode.GetChildren())
            AddNodes(aChild);
    }




    private Vector<INode> maNodes;
}
