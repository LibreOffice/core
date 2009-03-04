/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PageLayoutReadHandler.java,v $
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


package com.sun.star.report.pentaho.parser.style;

import java.util.ArrayList;

import com.sun.star.report.pentaho.model.PageLayout;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import java.util.List;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Reads a page-layout element.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class PageLayoutReadHandler extends ElementReadHandler
{
  private final PageLayout pageLayout;
  private final List childs;

  public PageLayoutReadHandler()
  {
    this.pageLayout = new PageLayout();
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
      pageLayout.addNode(handler.getElement());
    }
  }

  public PageLayout getPageLayout()
  {
    return pageLayout;
  }

  public Element getElement()
  {
    return pageLayout;
  }
}
