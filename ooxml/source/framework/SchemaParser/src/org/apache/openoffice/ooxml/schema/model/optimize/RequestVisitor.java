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

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;

/** A visitor that is called for all nodes of a complex or simple type to mark
 *  the referenced types as being used.
 */
public class RequestVisitor
    extends NodeVisitorAdapter
{
    RequestVisitor (
        final SchemaBase aSourceSchema,
        final SchemaOptimizer aOptimizer)
    {
        maSourceSchemaBase = aSourceSchema;
        maSchemaOptimizer = aOptimizer;
    }


    @Override public void Visit (final Attribute aAttribute)
    {
        maSchemaOptimizer.RequestType(aAttribute.GetTypeName());
    }

    @Override public void Visit (final AttributeReference aAttributeReference)
    {
        maSchemaOptimizer.RequestType(aAttributeReference.GetReferencedName());
    }

    @Override public void Visit (final AttributeGroupReference aAttributeGroupReference)
    {
        maSchemaOptimizer.RequestType(aAttributeGroupReference.GetReferencedName());
    }

    @Override public void Visit (final Element aElement)
    {
        maSchemaOptimizer.RequestType(aElement.GetTypeName());
    }

    @Override public void Visit (final Extension aExtension)
    {
        maSchemaOptimizer.RequestType(aExtension.GetBaseTypeName());
    }

    @Override public void Visit (final GroupReference aReference)
    {
        maSchemaOptimizer.RequestType(aReference.GetReferencedGroup(maSourceSchemaBase));
    }

    @Override public void Visit (final List aList)
    {
        maSchemaOptimizer.RequestType(aList.GetItemType());
    }

    @Override public void Visit (final SimpleTypeReference aReference)
    {
        maSchemaOptimizer.RequestType(aReference.GetReferencedSimpleType(maSourceSchemaBase));
    }

    @Override public void Default (final INode aNode)
    {
        switch (aNode.GetNodeType())
        {
            case All:
            case Any:
            case AttributeGroup:
            case BuiltIn:
            case Choice:
            case ComplexContent:
            case ComplexType:
            case ElementReference:
            case Group:
            case List:
            case OccurrenceIndicator:
            case Sequence:
            case SimpleContent:
            case SimpleType:
            case Union:
                break;

            default:
                throw new RuntimeException(
                    String.format("don't know how to request %s which was defined at %s",
                        aNode.toString(),
                        aNode.GetLocation()));
        }
    }

    private final SchemaBase maSourceSchemaBase;
    private final SchemaOptimizer maSchemaOptimizer;
}
