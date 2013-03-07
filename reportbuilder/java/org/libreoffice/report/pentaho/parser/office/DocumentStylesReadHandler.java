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
import org.libreoffice.report.pentaho.model.OfficeStylesCollection;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.style.OfficeStylesReadHandler;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * The root parser for a 'styles.xml' document. This generates the global
 * (or common) style collection. These styles contain the named common styles
 * and the page layouts.
 *
 * @since 08.03.2007
 */
public class DocumentStylesReadHandler extends ElementReadHandler
{

    private final OfficeStylesCollection officeStylesCollection;
    private FontFaceDeclsReadHandler fontFaceReadHandler;

    public DocumentStylesReadHandler()
    {
        officeStylesCollection = new OfficeStylesCollection();
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
        if (!OfficeNamespaces.OFFICE_NS.equals(uri))
        {
            return null;
        }

        if ("font-face-decls".equals(tagName))
        {
            if (fontFaceReadHandler == null)
            {
                fontFaceReadHandler = new FontFaceDeclsReadHandler(officeStylesCollection.getFontFaceDecls());
            }
            return fontFaceReadHandler;
        }
        else if ("automatic-styles".equals(tagName))
        {
            return new OfficeStylesReadHandler(officeStylesCollection.getAutomaticStyles());
        }
        else if ("styles".equals(tagName))
        {
            return new OfficeStylesReadHandler(officeStylesCollection.getCommonStyles());
        }
        else if ("master-styles".equals(tagName))
        {
            return new MasterStylesReadHandler(officeStylesCollection.getMasterStyles());
        }
        return null;
    }

    public Element getElement()
    {
        return officeStylesCollection;
    }
}
