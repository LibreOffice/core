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
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

/** This visitor is called to process individual type nodes.
 *  Actions can be adding top-level elements, complex types, simple types and
 *  groups and attribute groups to the optimized schema.
 */
public class ProcessTypeVisitor
    extends NodeVisitorAdapter
{
    ProcessTypeVisitor (
        final Schema aSourceSchema,
        final Schema aTargetSchema,
        final SchemaOptimizer aSchemaOptimizer)
    {
        maSourceSchema = aSourceSchema;
        maTargetSchema = aTargetSchema;
        maSchemaOptimizer = aSchemaOptimizer;
    }




    @Override public void Visit (final ComplexType aComplexType)
    {
        maTargetSchema.ComplexTypes.Add(aComplexType);

        // Add requests for types referenced by child nodes (sequences, elements, etc.)
        maSchemaOptimizer.RequestReferencedTypes(aComplexType);
    }

    @Override public void Visit (final Group aGroup)
    {
        maTargetSchema.Groups.Add(aGroup);
        maSchemaOptimizer.RequestReferencedTypes(aGroup);
    }
    @Override public void Visit (final SimpleType aSimpleType)
    {
        maTargetSchema.SimpleTypes.Add(aSimpleType);
        maSchemaOptimizer.RequestReferencedTypes(aSimpleType);
    }

    @Override public void Visit (final Attribute aAttribute)
    {
        maSchemaOptimizer.RequestType(aAttribute.GetTypeName());
    }
    @Override public void Visit (final AttributeGroup aAttributeGroup)
    {
        maTargetSchema.AttributeGroups.Add(aAttributeGroup);
    }
    @Override public void Visit (final AttributeReference aAttributeReference)
    {
        maTargetSchema.Attributes.Add(
            aAttributeReference.GetReferencedAttribute(maSourceSchema));
    }
    @Override public void Visit (final AttributeGroupReference aAttributeGroupReference)
    {
        maTargetSchema.AttributeGroups.Add(
            aAttributeGroupReference.GetReferencedAttributeGroup(maSourceSchema));
    }
    @Override public void Default (final INode aNode)
    {
        switch(aNode.GetNodeType())
        {
            case BuiltIn:
                break;

            default:
                throw new RuntimeException("don't know how to process "+aNode.toString());
        }
    }




    private final Schema maSourceSchema;
    private final Schema maTargetSchema;
    private final SchemaOptimizer maSchemaOptimizer;
}
