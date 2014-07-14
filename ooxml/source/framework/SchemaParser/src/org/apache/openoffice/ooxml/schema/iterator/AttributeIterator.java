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
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Iterate over all attributes of a given node.
 *  References to attributes and attribute groups and their references are resolved.
 *
 *  If you want to iterate over all attributes in a node tree then add an outer NodeIterator.
 */
public class AttributeIterator
    implements Iterable<Attribute>
{
    public AttributeIterator (
        final INode aNode,
        final SchemaBase aSchemaBase)
    {
        maAttributes = new TreeSet<Attribute>();
        CollectAttributes(aNode, aSchemaBase);
    }




    public Iterator<Attribute> iterator ()
    {
        return maAttributes.iterator();
    }




    private void CollectAttributes (
        final INode aType,
        final SchemaBase aSchemaBase)
    {
        final Queue<INode> aTodo = new LinkedList<>();
        for (final INode aAttribute : aType.GetAttributes())
            aTodo.add(aAttribute);
        final INodeVisitor aVisitor = new NodeVisitorAdapter()
        {
            @Override public void Visit (final Attribute aAttribute)
            {
                maAttributes.add(aAttribute);
            }
            @Override public void Visit (final AttributeReference aAttributeReference)
            {
                aTodo.add(aAttributeReference.GetReferencedAttribute(aSchemaBase));
            }
            @Override public void Visit (final AttributeGroup aAttributeGroup)
            {
                for (final INode aGroupAttribute : aAttributeGroup.GetAttributes())
                    aTodo.add(aGroupAttribute);
            }
            @Override public void Visit (final AttributeGroupReference aAttributeGroupReference)
            {
                aTodo.add(aAttributeGroupReference.GetReferencedAttributeGroup(aSchemaBase));
            }
            @Override public void Default (final INode aNode)
            {
                throw new RuntimeException("can not handle attribute of type "+aNode.GetNodeType());
            }
        };
        while ( ! aTodo.isEmpty())
        {
            final INode aAttribute = aTodo.poll();
            if (aAttribute != null)
                aAttribute.AcceptVisitor(aVisitor);
        }
    }




    private final Set<Attribute> maAttributes;
}
