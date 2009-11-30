/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocumentStylesReadHandler.java,v $
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
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.style.OfficeStylesReadHandler;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * The root parser for a 'styles.xml' document. This generates the global
 * (or common) style collection. These styles contain the named common styles
 * and the page layouts.
 *
 * @author Thomas Morgner
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
    return null;
  }


  public Element getElement()
  {
    return officeStylesCollection;
  }
}
