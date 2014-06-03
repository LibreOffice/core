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

package org.apache.openoffice.ooxml.schema.model.base;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.complex.All;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.Choice;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexContent;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexTypeReference;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.ElementReference;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.complex.OccurrenceIndicator;
import org.apache.openoffice.ooxml.schema.model.complex.Sequence;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleContent;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;
import org.apache.openoffice.ooxml.schema.model.simple.Union;

/** Implementation of the INodeVisitor interface.
 *  All methods that are not overridden are redirected to the Default(INode)
 *  method.
 */
public class NodeVisitorAdapter
    implements INodeVisitor
{
    @Override
    public void Visit (final All aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final Any aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final Choice aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final ComplexContent aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final ComplexType aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final ComplexTypeReference aTypeReference)
    {
        Default(aTypeReference);
    }

    @Override
    public void Visit (final Element aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final ElementReference aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final Extension aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final Group aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final GroupReference aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final OccurrenceIndicator aIndicator)
    {
        Default(aIndicator);
    }

    @Override
    public void Visit(Sequence aNode)
    {
        Default(aNode);
    }

    @Override
    public void Visit (final BuiltIn aType)
    {
        Default(aType);
    }

    @Override
    public void Visit (final List aList)
    {
        Default(aList);
    }

    @Override
    public void Visit (final SimpleContent aSimpleContent)
    {
        Default(aSimpleContent);
    }

    @Override
    public void Visit (final SimpleType aSimpleType)
    {
        Default(aSimpleType);
    }

    @Override
    public void Visit (final SimpleTypeReference aSimpleTypeReference)
    {
        Default(aSimpleTypeReference);
    }

    @Override
    public void Visit (final Union aUnion)
    {
        Default(aUnion);
    }

    @Override
    public void Visit (final Restriction aRestriction)
    {
        Default(aRestriction);
    }

    @Override
    public void Visit (final AttributeGroup aAttributeGroup)
    {
        Default(aAttributeGroup);
    }

    @Override
    public void Visit (final AttributeGroupReference aAttributeGroupReference)
    {
        Default(aAttributeGroupReference);
    }

    @Override
    public void Visit (final AttributeReference aAttributeReference)
    {
        Default(aAttributeReference);
    }

    @Override
    public void Visit (final Attribute aAttribute)
    {
        Default(aAttribute);
    }

    public void Default (final INode aNode)
    {
    }
}
