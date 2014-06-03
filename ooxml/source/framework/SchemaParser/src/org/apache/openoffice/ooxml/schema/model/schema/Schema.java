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
import org.apache.openoffice.ooxml.schema.model.optimize.SchemaOptimizer;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

/** Representation of a set of schema files (typically one master file and the included schema filed.)
 */
public class Schema
{
    public Schema ()
    {
        Namespaces = new NamespaceMap();
        ComplexTypes = new TypeContainer<>();
        SimpleTypes = new TypeContainer<>();
        Groups = new TypeContainer<>();
        AttributeGroups = new TypeContainer<>();
        Attributes = new TypeContainer<>();
        TopLevelElements = new TypeContainer<>();

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
        else if (TopLevelElements.Contains(sTypeName))
            return TopLevelElements.Get(sTypeName);
        else
            return null;
    }




    /** Create a new schema object that contains only the used types, i.e.
     *  types that are reachable via element transitions, starting with the
     *  top level elements.
     */
    public Schema GetOptimizedSchema ()
    {
        return new SchemaOptimizer(this).Run();
    }




    public final NamespaceMap Namespaces;
    public final TypeContainer<ComplexType> ComplexTypes;
    public final TypeContainer<SimpleType> SimpleTypes;
    public final TypeContainer<Group> Groups;
    public final TypeContainer<AttributeGroup> AttributeGroups;
    public final TypeContainer<Attribute> Attributes;
    public final TypeContainer<Element> TopLevelElements;
}
