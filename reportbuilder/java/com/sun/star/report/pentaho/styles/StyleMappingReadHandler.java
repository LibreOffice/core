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


package com.sun.star.report.pentaho.styles;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 12.03.2007
 */
public class StyleMappingReadHandler extends AbstractXmlReadHandler
{

    private StyleMappingRule rule;

    public StyleMappingReadHandler()
    {
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs)
            throws SAXException
    {
        final String elementNamespace = attrs.getValue(getUri(),
                "element-namespace");
        if (elementNamespace == null)
        {
            throw new ParseException("Required attribute 'element-namespace' is missing", getLocator());
        }

        final String elementName = attrs.getValue(getUri(), "element-name");

        if (elementName == null)
        {
            throw new ParseException("Required attribute 'element-name' is missing", getLocator());
        }

        final String attributeNamespace = attrs.getValue(getUri(),
                "attribute-namespace");
        final String attributeName = attrs.getValue(getUri(), "attribute-name");

        final boolean listOfValues =
                "styleNameRefs".equals(attrs.getValue(getUri(), "type"));

        final String family = attrs.getValue(getUri(), "style-family");
        final StyleMapperKey key = new StyleMapperKey(elementNamespace, elementName, attributeNamespace, attributeName);
        rule = new StyleMappingRule(key, family, listOfValues);
    }

    public StyleMappingRule getRule()
    {
        return rule;
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject()
            throws SAXException
    {
        return rule;
    }
}
