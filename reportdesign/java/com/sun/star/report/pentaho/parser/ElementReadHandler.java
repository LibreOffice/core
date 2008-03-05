/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ElementReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:39:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.report.pentaho.parser;

import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.AbstractXmlReadHandler;
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
