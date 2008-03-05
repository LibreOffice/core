/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FixedContentReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:41:54 $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.model.FixedTextElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.text.TextContentReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.IgnoreAnyChildReadHandler;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 01.10.2006, 18:48:11
 *
 * @author Thomas Morgner
 */
public class FixedContentReadHandler extends ElementReadHandler
{

    private FixedTextElement element;

    public FixedContentReadHandler()
    {
        element = new FixedTextElement();
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
        if (OfficeNamespaces.TEXT_NS.equals(uri))
        {
            // expect a paragraph (which will be ignored; it is a structural
            // component that needs not to be printed at all.
            if ("p".equals(tagName))
            {

                return new TextContentReadHandler(element.getContent());
            }
        }

        if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            // expect a report control. The control will modifiy the current
            // element (as we do not separate the elements that strictly ..)
            if ("report-control".equals(tagName))
            {
                return new IgnoreAnyChildReadHandler();
            }
            if ("report-element".equals(tagName))
            {
                return new ReportElementReadHandler(element);
            }
        }
        return null;
    }

    public Element getElement()
    {
        return element;
    }
}
