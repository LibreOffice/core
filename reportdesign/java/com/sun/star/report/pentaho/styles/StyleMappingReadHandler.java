/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleMappingReadHandler.java,v $
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


package com.sun.star.report.pentaho.styles;

import org.jfree.xmlns.parser.AbstractXmlReadHandler;
import org.jfree.xmlns.parser.ParseException;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 12.03.2007
 */
public class StyleMappingReadHandler extends AbstractXmlReadHandler
{
  private StyleMappingRule rule;

  public StyleMappingReadHandler()
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
    final String elementNamespace = attrs.getValue(getUri(),
        "element-namespace");
    if (elementNamespace == null)
    {
      throw new ParseException
          ("Required attribute 'element-namespace' is missing", getLocator());
    }

    final String elementName = attrs.getValue(getUri(), "element-name");

    if (elementName == null)
    {
      throw new ParseException
          ("Required attribute 'element-name' is missing", getLocator());
    }

    final String attributeNamespace = attrs.getValue(getUri(),
        "attribute-namespace");
    final String attributeName = attrs.getValue(getUri(), "attribute-name");

    final boolean listOfValues =
        "styleNameRefs".equals(attrs.getValue(getUri(), "type"));

    final String family = attrs.getValue(getUri(), "style-family");
    final StyleMapperKey key = new StyleMapperKey
        (elementNamespace, elementName, attributeNamespace, attributeName);
    rule = new StyleMappingRule(key, family, listOfValues);
  }


  public StyleMappingRule getRule()
  {
    return rule;
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
    return rule;
  }
}
