/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextContentReadHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:12 $
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

package com.sun.star.report.pentaho.parser.text;

import java.util.ArrayList;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.StarXmlFactoryModule;
import com.sun.star.report.pentaho.parser.rpt.FixedContentReadHandler;
import com.sun.star.report.pentaho.parser.rpt.FormattedTextReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ImageReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * This is a generic implementation that accepts all input and adds special
 * handlers for the report-elements.
 *
 * @author Thomas Morgner
 */
public class TextContentReadHandler extends ElementReadHandler
{
  private Section section;
  private ArrayList children;
  private boolean copyType;

  public TextContentReadHandler(final Section section,
                                final boolean copyType)
  {
    this.children = new ArrayList();
    this.section = section;
    this.copyType = copyType;
  }

  public TextContentReadHandler(final Section section)
  {
    this (section, false);
  }

  public TextContentReadHandler()
  {
    this(new Section(), true);
  }


  /**
   * Starts parsing.
   *
   * @param attrs the attributes.
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected void startParsing(final Attributes attrs) throws SAXException
  {
    super.startParsing(attrs);
    final Element element = getElement();
    if (copyType)
    {
      copyElementType(element);
    }
    copyAttributes(attrs, element);
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
    if (OfficeNamespaces.OOREPORT_NS.equals(uri))
    {
      if ("fixed-content".equals(tagName))
      {
        final FixedContentReadHandler fixedContentReadHandler = new FixedContentReadHandler();
        children.add(fixedContentReadHandler);
        return fixedContentReadHandler;
      }
      if ("formatted-text".equals(tagName))
      {
        final FormattedTextReadHandler formattedTextReadHandler = new FormattedTextReadHandler();
        children.add(formattedTextReadHandler);
        return formattedTextReadHandler;
      }
      if ("image".equals(tagName))
      {
        final ImageReadHandler imageReadHandler = new ImageReadHandler();
        children.add(imageReadHandler);
        return imageReadHandler;
      }
      if ("sub-document".equals(tagName))
      {
        return null;
      }
    }

    final TextContentReadHandler readHandler = new TextContentReadHandler();
    children.add(readHandler);
    return readHandler;
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
    children.add(new StaticText(new String(ch, start, length)));
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
        section.addNode(handler.getElement());
      }
      else if (o instanceof StaticText)
      {
        section.addNode((StaticText) o);
      }
    }
  }


  public Element getElement()
  {
    return section;
  }
}
