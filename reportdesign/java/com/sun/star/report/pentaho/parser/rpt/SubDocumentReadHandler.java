/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SubDocumentReadHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:44:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

import com.sun.star.report.pentaho.model.ObjectOleElement;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.draw.ObjectOleReadHandler;
import com.sun.star.report.pentaho.parser.text.NoCDATATextContentReadHandler;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.jfree.xmlns.parser.IgnoreAnyChildReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class SubDocumentReadHandler extends NoCDATATextContentReadHandler
{

    private ObjectOleElement element = null;
    private boolean ignore = false;

    public SubDocumentReadHandler(final ObjectOleElement element)
    {
        this.element = element;
    }

    public SubDocumentReadHandler(final Section section, final ObjectOleElement element)
    {
        super(section);
        this.element = element;
    }

    public SubDocumentReadHandler(final Section section)
    {
        super(section);
        this.element = new ObjectOleElement();
        ignore = true;
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        if (!ignore)
        {
            super.startParsing(attrs);
        }
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
            if ("master-detail-fields".equals(tagName))
            {
                return new MasterDetailReadHandler(element);
            }
        }
        if (OfficeNamespaces.DRAWING_NS.equals(uri))
        {
            XmlReadHandler readHandler = null;
            if ("object-ole".equals(tagName))
            {
                readHandler = new ObjectOleReadHandler(element);
            }
            else if ("frame".equals(tagName))
            {
                readHandler = new SubDocumentReadHandler(new Section(), element);
            }
            if (readHandler != null)
            {
                getChildren().add(readHandler);
                return readHandler;
            }
        }
        return super.getHandlerForChild(uri, tagName, atts);
    }

    public Element getElement()
    {

        return super.getElement();
    }
}
