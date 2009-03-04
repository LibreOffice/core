/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocumentContentReadHandler.java,v $
 * $Revision: 1.4 $
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
package com.sun.star.report.pentaho.parser.office;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeDocument;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.parser.style.OfficeStylesReadHandler;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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
 * <p/>
 * Alternativly, there is the option to keep everything in one file. The root
 * element for that format is 'office:document'.
 */
public class DocumentContentReadHandler extends AbstractXmlReadHandler
{
    private static final Log LOGGER = LogFactory.getLog(DocumentContentReadHandler.class);
    private OfficeDocument report;
    private FontFaceDeclsReadHandler fontFaceReadHandler;
    private BodyReadHandler bodyReadHandler;
    private OfficeStylesCollection officeStylesCollection;

    public DocumentContentReadHandler()
    {
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
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
            LOGGER.debug("Failed to create resource-key for 'styles.xml'. Ignoring.", e);
        }
        catch (ResourceException e)
        {
            // ignore ..
            LOGGER.debug("Failed to parse resource for 'styles.xml'. Ignoring.", e);
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
            LOGGER.debug("Failed to create resource-key for 'content.xml'. Ignoring.");
        }
        catch (ResourceException e)
        {
            // ignore ..
            LOGGER.debug("Failed to parse resource for 'content.xml'. Ignoring.");
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
