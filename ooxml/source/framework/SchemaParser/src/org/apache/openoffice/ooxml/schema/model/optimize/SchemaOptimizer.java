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

package org.apache.openoffice.ooxml.schema.model.optimize;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import org.apache.openoffice.ooxml.schema.iterator.AttributeNodeIterator;
import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Optimize the given schema base by creating a new one and adding only those
 *  complex types, simple types, groups, attributes and attribute groups, that
 *  are really used, i.e. used by one of the top level elements or by a type or
 *  group that is in use.
 */
public class SchemaOptimizer
{
    public SchemaOptimizer (
        final SchemaBase aOriginalSchemaBase)
    {
        maOriginalSchemaBase = aOriginalSchemaBase;
        maOptimizedSchemaBase = new SchemaBase();
        maTodoList = new LinkedList<>();
        maProcessedTypes = new HashSet<>();
    }




    public SchemaBase Run ()
    {
        // Seed the work list with the top-level elements.
        for (final Element aElement : maOriginalSchemaBase.TopLevelElements.GetUnsorted())
        {
            maOptimizedSchemaBase.TopLevelElements.Add(aElement);
            RequestType(aElement.GetTypeName());
        }

        final INodeVisitor aCopyVisitor = new CopyVisitor(
            maOriginalSchemaBase,
            maOptimizedSchemaBase);
        final INodeVisitor aRequestVisitor = new RequestVisitor(
            maOriginalSchemaBase,
            this);

        while ( ! maTodoList.isEmpty())
        {
            final INode aNode = maTodoList.poll();

            // Iterate over all child nodes and attributes.
            for (final INode aChild : new DereferencingNodeIterator(aNode, maOriginalSchemaBase, true))
            {
                aChild.AcceptVisitor(aCopyVisitor);
                aChild.AcceptVisitor(aRequestVisitor);
                for (final INode aAttribute : aChild.GetAttributes())
                    aAttribute.AcceptVisitor(aCopyVisitor);
                for (final INode aAttribute : new AttributeNodeIterator(aChild, maOriginalSchemaBase))
                    aAttribute.AcceptVisitor(aRequestVisitor);
            }

            // Request used namespaces.
            final QualifiedName aName = aNode.GetName();
            if (aName != null)
                maOptimizedSchemaBase.Namespaces.ProvideNamespace(aName.GetNamespaceURI(), aName.GetNamespacePrefix());
        }

        /*
        System.out.printf("%d original attributes\n", maOriginalSchemaBase.Attributes.GetCount());
        for (final Attribute aAttribute : maOriginalSchemaBase.Attributes.GetUnsorted())
            System.out.printf("%s\n",  aAttribute);
        System.out.printf("%d optimized attributes\n", maOptimizedSchemaBase.Attributes.GetCount());
        for (final Attribute aAttribute : maOptimizedSchemaBase.Attributes.GetUnsorted())
            System.out.printf("%s\n",  aAttribute);
            */

        return maOptimizedSchemaBase;
    }




    void RequestType (final QualifiedName aName)
    {
        final Node aNode = maOriginalSchemaBase.GetTypeForName(aName);
        if (aNode == null)
            throw new RuntimeException("there is no type named '"+aName+"' in the schema");
        else
            RequestType(aNode);
    }




    void RequestType (final INode aNode)
    {
        if (aNode.GetNodeType() == NodeType.SimpleTypeReference)
            System.out.println(aNode);
        if ( ! maProcessedTypes.contains(aNode))
        {
            maProcessedTypes.add(aNode);
            maTodoList.add(aNode);
        }
    }




    private final SchemaBase maOriginalSchemaBase;
    private final SchemaBase maOptimizedSchemaBase;
    private final Queue<INode> maTodoList;
    private final Set<INode> maProcessedTypes;
}
