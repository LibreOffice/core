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

import java.util.HashMap;
import java.util.Map;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.optimize.SchemaOptimizer;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;


/** Container of elements, complex types, simple types, etc for a set of
 *  master schema files and the included and imported secondary schema files.
 *
 *  See Schema objects for the set of top level elements that are unique to
 *  each master schema file.
 */
public class SchemaBase
{
    public SchemaBase ()
    {
        Namespaces = new NamespaceMap();
        TopLevelElements = new TypeContainer<>();
        ComplexTypes = new TypeContainer<>();
        SimpleTypes = new TypeContainer<>();
        Groups = new TypeContainer<>();
        AttributeGroups = new TypeContainer<>();
        Attributes = new TypeContainer<>();
        AttributeValueToIdMap = new HashMap<>();

        // Initialize the list of simple types with all known built ins (
        // these are implicitly defined).
        for (final BuiltIn aType : BuiltIn.GetTypes())
            SimpleTypes.Add(aType);
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
        else if (Attributes.Contains(sTypeName))
            return Attributes.Get(sTypeName);
        else if (AttributeGroups.Contains(sTypeName))
            return AttributeGroups.Get(sTypeName);
        else
            return null;
    }




    public Node GetSimpleTypeForName (final QualifiedName aName)
    {
        final String sTypeName = aName.GetDisplayName();

        if (SimpleTypes.Contains(sTypeName))
            return SimpleTypes.Get(aName.GetDisplayName());
        else
            return null;
    }




    /** Create a new schema object that contains only the used types, i.e.
     *  types that are reachable via element transitions, starting with the
     *  top level elements.
     */
    public SchemaBase GetOptimizedSchema (final Iterable<Schema> aTopLevelSchemas)
    {
        return new SchemaOptimizer(this).Run();
    }




    public final NamespaceMap Namespaces;
    public final TypeContainer<Element> TopLevelElements;
    public final TypeContainer<ComplexType> ComplexTypes;
    public final TypeContainer<SimpleType> SimpleTypes;
    public final TypeContainer<Group> Groups;
    public final TypeContainer<AttributeGroup> AttributeGroups;
    public final TypeContainer<Attribute> Attributes;
    public final Map<String,Integer> AttributeValueToIdMap;
}
