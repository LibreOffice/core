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
import org.libreoffice.report.pentaho.model.OfficeDocument;
import org.libreoffice.report.pentaho.model.OfficeStylesCollection;
import org.libreoffice.report.pentaho.parser.style.OfficeStylesReadHandler;

import java.util.logging.Logger;

import org.jfree.report.JFreeReport;

import org.pentaho.reporting.libraries.resourceloader.Resource;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceKeyCreationException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * This is the root handler for the document context. The
 * 'office:document-context' element is encountered, if the document is split
 * into separate files.
 *
 * <p>Alternatively, there is the option to keep everything in one file. The root
 * element for that format is 'office:document'.</p>
 */
public class DocumentContentReadHandler extends AbstractXmlReadHandler
{

    private static final Logger LOGGER = Logger.getLogger(DocumentContentReadHandler.class.getName());
    private OfficeDocument report;
    private FontFaceDeclsReadHandler fontFaceReadHandler;
    private BodyReadHandler bodyReadHandler;
    private OfficeStylesCollection officeStylesCollection;

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    @Override
    protected void startParsing(final Attributes attrs)
            throws SAXException
    {
        super.startParsing(attrs);
        // parse the external 'styles.xml' if it exists
        // parse the external 'meta.xml' if it exists
        // parse the external 'settings.xml' if it exists
        this.report = parseContentXml();
        this.report.setVirtual(true);
        this.report.setType("document-content");
        this.report.setNamespace(OfficeNamespaces.OFFICE_NS);
        this.report.setVirtual(true);
        this.officeStylesCollection = parseStylesXml();
    }

    private OfficeStylesCollection parseStylesXml()
    {
        final ResourceKey contextKey = getRootHandler().getContext();
        final ResourceManager resourceManager = getRootHandler().getResourceManager();

        try
        {
            final ResourceKey key =
                    resourceManager.deriveKey(contextKey, "styles.xml");
            final Resource resource =
                    resourceManager.create(key, contextKey, OfficeStylesCollection.class);
            final OfficeStylesCollection styles =
                    (OfficeStylesCollection) resource.getResource();
            if (styles != null)
            {
                return styles;
            }
        }
        catch (ResourceKeyCreationException e)
        {
            // ignore ..
            LOGGER.config("Failed to create resource-key for 'styles.xml'. Ignoring: " + e);
        }
        catch (ResourceException e)
        {
            // ignore ..
            LOGGER.config("Failed to parse resource for 'styles.xml'. Ignoring: " + e);
        }

        return new OfficeStylesCollection();
    }

    private OfficeDocument parseContentXml()
    {
        // Check whether this is a content.xml.
        if (!OfficeNamespaces.OFFICE_NS.equals(getUri()) || "document-content".equals(getTagName()))
        {
            return new OfficeDocument();
        }

        // we may have to parse an existing content.xml.
        final ResourceKey contextKey = getRootHandler().getContext();
        final ResourceManager resourceManager = getRootHandler().getResourceManager();
        try
        {
            final ResourceKey key =
                    resourceManager.deriveKey(contextKey, "content.xml");
            final Resource resource =
                    resourceManager.create(key, contextKey, JFreeReport.class);
            final OfficeDocument doc = (OfficeDocument) resource.getResource();
            if (doc != null)
            {
                return doc;
            }
        }
        catch (ResourceKeyCreationException e)
        {
            // ignore ..
            LOGGER.config("Failed to create resource-key for 'content.xml'. Ignoring.");
        }
        catch (ResourceException e)
        {
            // ignore ..
            LOGGER.config("Failed to parse resource for 'content.xml'. Ignoring.");
        }
        return new OfficeDocument();

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
        if (OfficeNamespaces.OFFICE_NS.equals(uri))
        {
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
            else if ("body".equals(tagName))
            {
                bodyReadHandler = new BodyReadHandler();
                return bodyReadHandler;
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
    protected void doneParsing()
            throws SAXException
    {
        // The office-document is the only node of the report. It allows us to
        // switch the layout-processing implementation later on.

        report.setStylesCollection(officeStylesCollection);

        if (bodyReadHandler != null)
        {
            report.addNode(bodyReadHandler.getElement());
        }
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject()
            throws SAXException
    {
        return report;
    }
}
