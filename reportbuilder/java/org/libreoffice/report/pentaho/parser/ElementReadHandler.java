/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.parser;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 03.07.2006, 14:22:34
 *
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
