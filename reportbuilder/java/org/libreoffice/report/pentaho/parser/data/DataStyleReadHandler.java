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
package org.libreoffice.report.pentaho.parser.data;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.DataStyle;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.StaticText;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


public class DataStyleReadHandler extends ElementReadHandler
{

    private final DataStyle dataStyle;
    private final List<Object> children;
    private final boolean hasCData;

    public DataStyleReadHandler(final boolean hasCData)
    {
        this.hasCData = hasCData;
        this.dataStyle = new DataStyle();
        this.children = new ArrayList<Object>();
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.DATASTYLE_NS.equals(uri) || OfficeNamespaces.STYLE_NS.equals(uri))
        {
            final DataStyleReadHandler xrh = new DataStyleReadHandler("text".equals(tagName) || "currency-symbol".equals(tagName) || "embedded-text".equals(tagName));
            children.add(xrh);
            return xrh;
        }

        return null;
    }

    /**
     * This method is called to process the character data between element tags.
     *
     * @param ch     the character buffer.
     * @param start  the start index.
     * @param length the length.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    public void characters(final char[] ch, final int start, final int length)
            throws SAXException
    {
        if (hasCData)
        {
            children.add(new StaticText(new String(ch, start, length)));
        }
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < children.size(); i++)
        {
            final Object o = children.get(i);
            if (o instanceof ElementReadHandler)
            {
                final ElementReadHandler handler = (ElementReadHandler) o;
                dataStyle.addNode(handler.getElement());
            }
            else if (o instanceof StaticText)
            {
                dataStyle.addNode((StaticText) o);
            }
        }
    }

    public DataStyle getDataStyle()
    {
        return dataStyle;
    }

    public Element getElement()
    {
        return dataStyle;
    }
}
