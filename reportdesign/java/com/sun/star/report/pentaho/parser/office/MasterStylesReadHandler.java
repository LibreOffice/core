/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterStylesReadHandler.java,v $
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

import java.util.ArrayList;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.style.StyleDefinitionReadHandler;
import com.sun.star.report.pentaho.parser.style.MasterPageReadHandler;
import com.sun.star.report.pentaho.model.OfficeMasterStyles;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class MasterStylesReadHandler extends ElementReadHandler
{
  private OfficeMasterStyles masterStyles;
  private ArrayList otherHandlers;
  private ArrayList masterPageHandlers;

  public MasterStylesReadHandler(final OfficeMasterStyles masterStyles)
  {
    this.masterStyles = masterStyles;
    this.masterPageHandlers = new ArrayList();
    this.otherHandlers = new ArrayList();
  }

  public OfficeMasterStyles getMasterStyles()
  {
    return masterStyles;
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
    if (OfficeNamespaces.STYLE_NS.equals(uri))
    {
      if ("master-page".equals(tagName))
      {
        final MasterPageReadHandler mrh = new MasterPageReadHandler();
        masterPageHandlers.add(mrh);
        return mrh;
      }
    }

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
      final ElementReadHandler handler =
          (ElementReadHandler) otherHandlers.get(i);
      masterStyles.getOtherNodes().addNode(handler.getElement());
    }

    for (int i = 0; i < masterPageHandlers.size(); i++)
    {
      final MasterPageReadHandler handler =
          (MasterPageReadHandler) masterPageHandlers.get(i);
      masterStyles.addMasterPage(handler.getMasterPage());
    }
  }

  public Element getElement()
  {
    return masterStyles;
  }
}
