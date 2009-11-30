/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImageReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.model.ImageElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.xlink.XLinkReadHandler;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
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

    private final ImageElement contentElement;
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
//        final String preserveIRI = attrs.getValue(OfficeNamespaces.OOREPORT_NS, OfficeToken.PRESERVE_IRI);
        if (formula != null && formula.length() != 0)
        {
            // now, the evaulated content ends up in the 'content' attribute of the
            // element.
            final FormulaExpression valueExpression = new FormulaExpression();
            valueExpression.setFormula(formula);
            contentElement.setFormula(valueExpression);
        }

        contentElement.setNamespace(OfficeNamespaces.FORM_NS);
        contentElement.setType(OfficeToken.IMAGE);
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
        if (OfficeNamespaces.DRAWING_NS.equals(uri) && OfficeToken.IMAGE_DATA.equals(tagName))
        {
            xLinkReadHandler = new XLinkReadHandler();
            return xLinkReadHandler;
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
