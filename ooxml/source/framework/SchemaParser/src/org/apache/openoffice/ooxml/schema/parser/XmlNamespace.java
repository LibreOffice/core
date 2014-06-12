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

package org.apache.openoffice.ooxml.schema.parser;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

/** The http://www.w3.org/XML/1998/namespace namespace is
 *  implicitly included in all schema files.
 *
 *  This class makes the necessary additions to namespace map and Schema object.
 */
public class XmlNamespace
{
    public static final String NamespaceURI = "http://www.w3.org/XML/1998/namespace";
    public static final String NamespacePrefix = "xml";

    public static void Apply (final SchemaBase aSchemaBase)
    {
        aSchemaBase.Namespaces.ProvideNamespace(NamespaceURI, NamespacePrefix);

        final QualifiedName aStSpaceSimpleTypeName = new QualifiedName(NamespaceURI, NamespacePrefix, "ST__space");
        aSchemaBase.Attributes.Add(
            new Attribute(
                new QualifiedName(NamespaceURI, NamespacePrefix, "space"),
                aStSpaceSimpleTypeName,
                "optional",
                null,
                null,
                FormDefault.unqualified,
                null));

        final SimpleType aType = new SimpleType(null, aStSpaceSimpleTypeName, null);
        final Restriction aRestriction = new Restriction(aType, "xsd:token", null);
        aRestriction.AddEnumeration("default");
        aRestriction.AddEnumeration("preserve");
        aSchemaBase.SimpleTypes.Add(aType);
    }
}
