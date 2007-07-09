/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentContentReadHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.pentaho.parser.office;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeDocument;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.parser.style.OfficeStylesReadHandler;
import org.jfree.report.JFreeReport;
import org.jfree.resourceloader.Resource;
import org.jfree.resourceloader.ResourceException;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceKeyCreationException;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.util.Log;
import org.jfree.xmlns.parser.AbstractXmlReadHandler;
import org.jfree.xmlns.parser.XmlReadHandler;
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

  private OfficeStylesCollection parseStylesXml ()
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
      Log.debug("Failed to create resource-key for 'styles.xml'. Ignoring.", e);
    }
    catch (ResourceException e)
    {
      // ignore ..
      Log.debug("Failed to parse resource for 'styles.xml'. Ignoring.", e);
    }

    return new OfficeStylesCollection();
  }

  private OfficeDocument parseContentXml()
  {
    if (OfficeNamespaces.OFFICE_NS.equals(getUri()) == false)
    {
      return new OfficeDocument();
    }

    // Check whether this is a content.xml.
    if ("document-content".equals(getTagName()) != false)
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
      final OfficeDocument report = (OfficeDocument) resource.getResource();
      if (report != null)
      {
        return report;
      }
    }
    catch (ResourceKeyCreationException e)
    {
      // ignore ..
      Log.debug("Failed to create resource-key for 'content.xml'. Ignoring.");
    }
    catch (ResourceException e)
    {
      // ignore ..
      Log.debug("Failed to parse resource for 'content.xml'. Ignoring.");
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
    if (!OfficeNamespaces.OFFICE_NS.equals(uri))
    {
      return null;
    }

    if ("font-face-decls".equals(tagName))
    {
      if (fontFaceReadHandler == null)
      {
        fontFaceReadHandler = new FontFaceDeclsReadHandler
            (officeStylesCollection.getFontFaceDecls());
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
