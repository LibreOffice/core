/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ElementReadHandler.java,v $
 * $Revision: 1.4 $
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
