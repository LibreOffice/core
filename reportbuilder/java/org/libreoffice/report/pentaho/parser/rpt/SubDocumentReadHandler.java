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
import org.libreoffice.report.pentaho.model.ObjectOleElement;
import org.libreoffice.report.pentaho.parser.draw.ObjectOleReadHandler;
import org.libreoffice.report.pentaho.parser.text.NoCDATATextContentReadHandler;

import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class SubDocumentReadHandler extends NoCDATATextContentReadHandler
{

    private final ObjectOleElement element;
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
        this(section, new ObjectOleElement());
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
            final XmlReadHandler readHandler;
            if (OfficeToken.OBJECT_OLE.equals(tagName))
            {
                readHandler = new ObjectOleReadHandler(element);
            }
            else if ("frame".equals(tagName))
            {
                readHandler = new SubDocumentReadHandler(new Section(), element);
            }
            else
            {
                readHandler = null;
            }
            if (readHandler != null)
            {
                getChildren().add(readHandler);
                return readHandler;
            }
        }
        return super.getHandlerForChild(uri, tagName, atts);
    }
}
