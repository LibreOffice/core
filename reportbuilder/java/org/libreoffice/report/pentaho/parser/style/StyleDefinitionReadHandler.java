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

import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * Reads all children of a style-definition. This simply copies everything that
 * is contained in the source-file into a generic structure that can be
 * written out later.
 */
public class StyleDefinitionReadHandler extends ElementReadHandler
{

    private final Section rawSection;
    private final List<ElementReadHandler> children;

    public StyleDefinitionReadHandler()
    {
        this.rawSection = new Section();
        this.children = new ArrayList<ElementReadHandler>();
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
    @Override
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        final StyleDefinitionReadHandler readHandler =
                new StyleDefinitionReadHandler();
        children.add(readHandler);
        return readHandler;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    @Override
    protected void doneParsing()
            throws SAXException
    {
        for (int i = 0; i < children.size(); i++)
        {
            final ElementReadHandler handler = children.get(i);
            rawSection.addNode(handler.getElement());
        }
    }

    @Override
    public Element getElement()
    {
        return rawSection;
    }
}
