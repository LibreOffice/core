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
package org.libreoffice.report.pentaho.parser.office;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.FontFaceDeclsSection;
import org.libreoffice.report.pentaho.model.FontFaceElement;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.style.FontFaceReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * Reads the font-face declarations section. This one can only contain
 * font-face elements.
 *
 * @since 13.03.2007
 */
public class FontFaceDeclsReadHandler extends ElementReadHandler
{

    private final FontFaceDeclsSection fontFaceDecls;
    private final List<FontFaceReadHandler> fontFaceReadHandlers;

    public FontFaceDeclsReadHandler(final FontFaceDeclsSection fontFaceDecls)
    {
        this.fontFaceDecls = fontFaceDecls;
        this.fontFaceReadHandlers = new ArrayList<FontFaceReadHandler>();
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
        if (!OfficeNamespaces.STYLE_NS.equals(uri))
        {
            return null;
        }

        if ("font-face".equals(tagName))
        {
            final FontFaceReadHandler frh = new FontFaceReadHandler();
            fontFaceReadHandlers.add(frh);
            return frh;
        }
        return null;
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
        for (int i = 0; i < fontFaceReadHandlers.size(); i++)
        {
            final FontFaceReadHandler handler = fontFaceReadHandlers.get(i);
            fontFaceDecls.addFontFace((FontFaceElement) handler.getElement());
        }
    }

    @Override
    public Element getElement()
    {
        return fontFaceDecls;
    }
}
