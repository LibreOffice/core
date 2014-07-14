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

package org.apache.openoffice.ooxml.schema.model.schema;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

/** Front end of a shared SchemaBase object of a single master schema file (such
 *  as wml.xsd).
 *  Most important member is the TopLevelElements list of top level elements,
 *  which are unique to each master schema.
 */
public class Schema
{
    public Schema (
            final String sShortName,
            final SchemaBase aBase)
    {
        msShortName = sShortName;
        TopLevelElements = new TypeContainer<>();

        Namespaces = aBase.Namespaces;
        ComplexTypes = aBase.ComplexTypes;
        SimpleTypes = aBase.SimpleTypes;
        Groups = aBase.Groups;
        AttributeGroups = aBase.AttributeGroups;
        Attributes = aBase.Attributes;
    }




    public Node GetTypeForName (final QualifiedName aName)
    {
        final String sTypeName = aName.GetDisplayName();

        if (ComplexTypes.Contains(sTypeName))
            return ComplexTypes.Get(sTypeName);
        else if (SimpleTypes.Contains(sTypeName))
            return SimpleTypes.Get(sTypeName);
        else if (Groups.Contains(sTypeName))
            return Groups.Get(sTypeName);
        else if (TopLevelElements.Contains(sTypeName))
            return TopLevelElements.Get(sTypeName);
        else
            return null;
    }




    public String GetShortName ()
    {
        return msShortName;
    }




    public Schema GetOptimizedSchema (final SchemaBase aSchemaBase)
    {
        final Schema aOptimizedSchema = new Schema(msShortName, aSchemaBase);
        for (final Element aElement : TopLevelElements.GetUnsorted())
        {
            aOptimizedSchema.TopLevelElements.Add(aElement);
        }

        return aOptimizedSchema;
    }




    private final String msShortName;
    public final TypeContainer<Element> TopLevelElements;
    public final NamespaceMap Namespaces;
    public final TypeContainer<ComplexType> ComplexTypes;
    public final TypeContainer<SimpleType> SimpleTypes;
    public final TypeContainer<Group> Groups;
    public final TypeContainer<AttributeGroup> AttributeGroups;
    public final TypeContainer<Attribute> Attributes;
}
