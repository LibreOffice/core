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


package com.sun.star.report.pentaho.parser;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 03.07.2006, 14:22:34
 *
 * @author Thomas Morgner
 */
public abstract class ElementReadHandler extends AbstractXmlReadHandler
{

    public ElementReadHandler()
    {
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        final Element element = getElement();
        copyElementType(element);
        copyAttributes(attrs, element);
    }

    protected void copyElementType(final Element element)
    {
        element.setType(getTagName());
        element.setNamespace(getUri());
    }

    protected void copyAttributes(final Attributes attrs, final Element element)
    {
        final int length = attrs.getLength();
        for (int i = 0; i < length; i++)
        {
            final String value = attrs.getValue(i);
            final String namespace = attrs.getURI(i);
            final String attr = attrs.getLocalName(i);
            element.setAttribute(namespace, attr, value);
        }
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject() throws SAXException
    {
        return getElement();
    }

    public abstract Element getElement();
}
