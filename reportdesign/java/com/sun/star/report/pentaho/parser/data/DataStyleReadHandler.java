/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataStyleReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 13:10:40 $
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


package com.sun.star.report.pentaho.parser.data;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.DataStyle;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.StaticText;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class DataStyleReadHandler extends ElementReadHandler
{
  private DataStyle dataStyle;
  private ArrayList children;
  private boolean hasCData;

  public DataStyleReadHandler(final boolean hasCData)
  {
    this.hasCData = hasCData;
    this.dataStyle = new DataStyle();
    this.children = new ArrayList();
  }

  /**
   * Returns the handler for a child element.
   *
   * @param tagName the tag name.
   * @param atts    the attributes.
   * @return the handler or null, if the tagname is invalid.
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected XmlReadHandler getHandlerForChild(final String uri,
                                              final String tagName,
                                              final Attributes atts)
      throws SAXException
  {
    if (OfficeNamespaces.DATASTYLE_NS.equals(uri) || OfficeNamespaces.STYLE_NS.equals(uri) )
    {
      final DataStyleReadHandler xrh = new DataStyleReadHandler("text".equals(tagName) || "currency-symbol".equals(tagName));
      children.add(xrh);
      return xrh;
    }

    return null;
  }

  /**
   * This method is called to process the character data between element tags.
   *
   * @param ch     the character buffer.
   * @param start  the start index.
   * @param length the length.
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  public void characters(final char[] ch, final int start, final int length)
      throws SAXException
  {
    if (hasCData)
    {
      children.add(new StaticText(new String(ch, start, length)));
    }
  }

  /**
   * Done parsing.
   *
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected void doneParsing() throws SAXException
  {
    for (int i = 0; i < children.size(); i++)
    {
      final Object o = children.get(i);
      if (o instanceof ElementReadHandler)
      {
        final ElementReadHandler handler = (ElementReadHandler) o;
        dataStyle.addNode(handler.getElement());
      }
      else if (o instanceof StaticText)
      {
        dataStyle.addNode((StaticText) o);
      }
    }
  }


  public DataStyle getDataStyle()
  {
    return dataStyle;
  }

  public Element getElement()
  {
    return dataStyle;
  }
}
