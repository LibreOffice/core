/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeStyleReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:45:26 $
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


package com.sun.star.report.pentaho.parser.style;

import java.util.ArrayList;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.model.OfficeStyle;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * This class reads a single style rule. The resulting 'office-style' element
 * is added to an 'office-styles' set.
 */
public class OfficeStyleReadHandler extends ElementReadHandler
{
  private OfficeStyle officeStyle;
  private ArrayList childs;

  public OfficeStyleReadHandler()
  {
    this.officeStyle = new OfficeStyle();
    this.childs = new ArrayList();
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
  protected XmlReadHandler getHandlerForChild (final String uri,
                                               final String tagName,
                                               final Attributes atts)
          throws SAXException
  {
//    if (OfficeParserUtil.getInstance().isValidStyleElement(uri, tagName))
//    {
//    }
    final StyleDefinitionReadHandler readHandler =
            new StyleDefinitionReadHandler();
    childs.add(readHandler);
    return readHandler;
  }


  /**
   * Done parsing.
   *
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected void doneParsing() throws SAXException
  {
    for (int i = 0; i < childs.size(); i++)
    {
      final ElementReadHandler handler = (ElementReadHandler) childs.get(i);
      officeStyle.addNode(handler.getElement());
    }
  }

  public OfficeStyle getOfficeStyle()
  {
    return officeStyle;
  }

  public Element getElement()
  {
    return officeStyle;
  }
}
