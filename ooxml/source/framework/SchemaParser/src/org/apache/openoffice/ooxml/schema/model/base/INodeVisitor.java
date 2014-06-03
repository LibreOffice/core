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

/** Interface for the visitor pattern.
 *  Use a node visitor instead of INode/NodeType and casting INode to a derived
 *  class.
 *  See also the default implementation NodeVisitorAdapter.
 */
public interface INodeVisitor
{
    // Complex nodes.
    void Visit (final All aAll);
    void Visit (final Any aAny);
    void Visit (final ComplexContent aComplexContent);
    void Visit (final ComplexType aNode);
    void Visit (final ComplexTypeReference aTypeReference);
    void Visit (final Choice aChoice);
    void Visit (final Element aElement);
    void Visit (final ElementReference aElementReference);
    void Visit (final Extension aNode);
    void Visit (final Group aGroup);
    void Visit (final GroupReference aGroupReference);
    void Visit (final OccurrenceIndicator aOccurrenceIndicator);
    void Visit (final Sequence aNode);

    // Simple nodes.
    void Visit (final BuiltIn aType);
    void Visit (final List aList);
    void Visit (final Restriction aRestriction);
    void Visit (final SimpleContent aSimpleContent);
    void Visit (final SimpleType aSimpleType);
    void Visit (final SimpleTypeReference aSimpleTypeReference);
    void Visit (final Union aUnion);

    // Attributes.
    void Visit (final AttributeGroup attributeGroup);
    void Visit (final AttributeReference attributeReference);
    void Visit (final Attribute attribute);
    void Visit (final AttributeGroupReference attributeGroupReference);
}
