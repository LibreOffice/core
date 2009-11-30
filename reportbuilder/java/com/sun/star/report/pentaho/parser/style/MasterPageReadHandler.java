/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterPageReadHandler.java,v $
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

import com.sun.star.report.pentaho.model.OfficeMasterPage;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import java.util.List;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class MasterPageReadHandler extends ElementReadHandler
{
  private final OfficeMasterPage masterPage;
  private final List otherHandlers;

  public MasterPageReadHandler()
  {
    masterPage = new OfficeMasterPage();
    this.otherHandlers = new ArrayList();
  }

  public OfficeMasterPage getMasterPage()
  {
    return masterPage;
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
      masterPage.addNode(handler.getElement());
    }
  }


  public Element getElement()
  {
    return masterPage;
  }
}
