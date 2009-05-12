/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleMappingReadHandler.java,v $
 * $Revision: 1.3 $
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


package com.sun.star.report.pentaho.styles;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;
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
