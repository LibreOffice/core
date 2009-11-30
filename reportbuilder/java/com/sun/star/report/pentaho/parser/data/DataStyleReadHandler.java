/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataStyleReadHandler.java,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report.pentaho.parser.data;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.DataStyle;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import java.util.List;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.StaticText;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class DataStyleReadHandler extends ElementReadHandler
{

    private final DataStyle dataStyle;
    private final List children;
    private final boolean hasCData;

    public DataStyleReadHandler(final boolean hasCData)
    {
        this.hasCData = hasCData;
        this.dataStyle = new DataStyle();
        this.children = new ArrayList();
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
