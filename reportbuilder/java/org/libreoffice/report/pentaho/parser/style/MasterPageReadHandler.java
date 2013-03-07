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
package org.libreoffice.report.pentaho.parser.style;

import org.libreoffice.report.pentaho.model.OfficeMasterPage;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * Todo: Document me!
 *
 * @since 13.03.2007
 */
public class MasterPageReadHandler extends ElementReadHandler
{

    private final OfficeMasterPage masterPage;
    private final List<ElementReadHandler> otherHandlers;

    public MasterPageReadHandler()
    {
        masterPage = new OfficeMasterPage();
        this.otherHandlers = new ArrayList<ElementReadHandler>();
    }

    public OfficeMasterPage getMasterPage()
    {
        return masterPage;
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        final StyleDefinitionReadHandler readHandler =
                new StyleDefinitionReadHandler();
        otherHandlers.add(readHandler);
        return readHandler;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing()
            throws SAXException
    {
        for (int i = 0; i < otherHandlers.size(); i++)
        {
            final ElementReadHandler handler = otherHandlers.get(i);
            masterPage.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return masterPage;
    }
}
