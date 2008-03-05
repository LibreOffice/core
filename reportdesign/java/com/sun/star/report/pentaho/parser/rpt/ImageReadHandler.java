/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageReadHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:43:52 $
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

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ImageElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.xlink.XLinkReadHandler;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.IgnoreAnyChildReadHandler;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Deals with Image-content. There are two ways to specify the image;
 * as formula or as static image data.
 *
 * @author Thomas Morgner
 */
public class ImageReadHandler extends ElementReadHandler
{

    private ImageElement contentElement;
    private XLinkReadHandler xLinkReadHandler;

    public ImageReadHandler()
    {
        contentElement = new ImageElement();
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);
        final String formula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
//        final String preserveIRI = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "preserve-IRI");
        if (formula != null && formula.length() != 0)
        {
            // now, the evaulated content ends up in the 'content' attribute of the
            // element.
            final FormulaExpression valueExpression = new FormulaExpression();
            valueExpression.setFormula(formula);
            contentElement.setFormula(valueExpression);
        }

        contentElement.setNamespace(OfficeNamespaces.FORM_NS);
        contentElement.setType("image");
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
        if (OfficeNamespaces.DRAWING_NS.equals(uri))
        {
            if ("image-data".equals(tagName))
            {
                xLinkReadHandler = new XLinkReadHandler();
                return xLinkReadHandler;
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
                return new ReportElementReadHandler(contentElement);
            }
        }
        return null;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        // if we have static content (as well or only), that one goes into the
        // alternate-content attribute right now. It is part of the output target
        // and style rules to deal with them properly ..
        if (xLinkReadHandler != null)
        {
            contentElement.setAttribute(OfficeNamespaces.OOREPORT_NS,
                    "alternate-content", xLinkReadHandler.getUri());
        }
    }

    public Element getElement()
    {
        return contentElement;
    }
}
