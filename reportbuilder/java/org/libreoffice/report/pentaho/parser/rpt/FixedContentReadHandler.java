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
import org.libreoffice.report.pentaho.model.FixedTextElement;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.text.TextContentReadHandler;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 01.10.2006, 18:48:11
 *
 */
public class FixedContentReadHandler extends ElementReadHandler
{

    private final FixedTextElement element;

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
        if (OfficeNamespaces.TEXT_NS.equals(uri) && OfficeToken.P.equals(tagName))
        {
            // expect a paragraph (which will be ignored; it is a structural
            // component that needs not to be printed at all.
            return new TextContentReadHandler(element.getContent());
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
