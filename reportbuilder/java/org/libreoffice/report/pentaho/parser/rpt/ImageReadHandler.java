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
package org.libreoffice.report.pentaho.parser.rpt;

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.ImageElement;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.xlink.XLinkReadHandler;

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
    @Override
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);
        final String formula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
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
    @Override
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
            // expect a report control. The control will modify the current
            // element (as we do not separate the elements that strictly...)
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
    @Override
    protected void doneParsing() throws SAXException
    {
        // if we have static content (as well as or only), that one goes into the
        // alternate-content attribute right now. It is part of the output target
        // and style rules to deal with them properly.
        if (xLinkReadHandler != null)
        {
            contentElement.setAttribute(OfficeNamespaces.OOREPORT_NS,
                    "alternate-content", xLinkReadHandler.getUri());
        }
    }

    @Override
    public Element getElement()
    {
        return contentElement;
    }
}
