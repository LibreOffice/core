/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentStylesReadHandler.java,v $
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
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.style.OfficeStylesReadHandler;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.XmlReadHandler;
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
  private OfficeStylesCollection officeStylesCollection;
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
