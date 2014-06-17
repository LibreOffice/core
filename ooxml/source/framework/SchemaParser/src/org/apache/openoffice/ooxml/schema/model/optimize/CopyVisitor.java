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

import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

/** This visitor is called to copy the used types from the source to the target
 *  schema base.
 */
public class CopyVisitor
    extends NodeVisitorAdapter
{
    CopyVisitor (
        final SchemaBase aSourceSchemaBase,
        final SchemaBase aTargetSchemaBase)
    {
        maSourceSchemaBase = aSourceSchemaBase;
        maTargetSchemaBase = aTargetSchemaBase;
    }




    @Override public void Visit (final ComplexType aComplexType)
    {
        maTargetSchemaBase.ComplexTypes.Add(aComplexType);
    }

    @Override public void Visit (final GroupReference aGroupReference)
    {
        maTargetSchemaBase.Groups.Add(
            aGroupReference.GetReferencedGroup(maSourceSchemaBase));
    }

    @Override public void Visit (final SimpleType aSimpleType)
    {
        maTargetSchemaBase.SimpleTypes.Add(aSimpleType);
    }

    @Override public void Visit (final AttributeReference aAttributeReference)
    {
        maTargetSchemaBase.Attributes.Add(
            aAttributeReference.GetReferencedAttribute(maSourceSchemaBase));
    }

    @Override public void Visit (final AttributeGroupReference aAttributeGroupReference)
    {
        maTargetSchemaBase.AttributeGroups.Add(
            aAttributeGroupReference.GetReferencedAttributeGroup(maSourceSchemaBase));
    }

    @Override public void Default (final INode aNode)
    {
        switch(aNode.GetNodeType())
        {
            case All:
            case Any:
            case Attribute:
            case AttributeGroup:
            case BuiltIn:
            case Choice:
            case ComplexContent:
            case Element:
            case ElementReference:
            case Extension:
            case Group:
            case List:
            case OccurrenceIndicator:
            case Restriction:
            case Sequence:
            case SimpleContent:
            case SimpleTypeReference:
            case Union:
                break;

            default:
                throw new RuntimeException("don't know how to copy "+aNode.toString());
        }
    }




    private final SchemaBase maSourceSchemaBase;
    private final SchemaBase maTargetSchemaBase;
}
