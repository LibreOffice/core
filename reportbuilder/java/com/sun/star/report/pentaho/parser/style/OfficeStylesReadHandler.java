/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeStylesReadHandler.java,v $
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
import org.jfree.report.modules.factories.report.flow.SectionReadHandler;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.data.DataStyleReadHandler;
import java.util.List;

public class OfficeStylesReadHandler extends ElementReadHandler
{
  private final List textStyleChilds;
  private final List dataStyleChilds;
  private final List otherStyleChilds;
  private final List pageLayoutChilds;
  private final OfficeStyles officeStyles;

  public OfficeStylesReadHandler(final OfficeStyles officeStyles)
  {
    this.officeStyles = officeStyles;
    this.pageLayoutChilds = new ArrayList();
    this.dataStyleChilds = new ArrayList();
    this.textStyleChilds = new ArrayList();
    this.otherStyleChilds = new ArrayList();
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
  protected XmlReadHandler getHandlerForChild (final String uri, final String tagName,
                                               final Attributes atts)
          throws SAXException
  {
    if (OfficeNamespaces.STYLE_NS.equals(uri))
    {
      if ("style".equals(tagName))
      {
        final OfficeStyleReadHandler xrh = new OfficeStyleReadHandler();
        textStyleChilds.add(xrh);
        return xrh;
      }
      else if ("page-layout".equals(tagName))
      {
        final PageLayoutReadHandler prh = new PageLayoutReadHandler();
        pageLayoutChilds.add(prh);
        return prh;
      }
    }
    else if (OfficeNamespaces.DATASTYLE_NS.equals(uri))
    {
      final DataStyleReadHandler xrh = new DataStyleReadHandler(false);
      dataStyleChilds.add(xrh);
      return xrh;
    }

    final SectionReadHandler genericReadHander = new SectionReadHandler();
    otherStyleChilds.add(genericReadHander);
    return genericReadHander;
  }

  /**
   * Done parsing.
   *
   * @throws org.xml.sax.SAXException if there is a parsing error.
   */
  protected void doneParsing() throws SAXException
  {
    for (int i = 0; i < textStyleChilds.size(); i++)
    {
      final OfficeStyleReadHandler handler =
          (OfficeStyleReadHandler) textStyleChilds.get(i);
      officeStyles.addStyle(handler.getOfficeStyle());
    }

    for (int i = 0; i < pageLayoutChilds.size(); i++)
    {
      final PageLayoutReadHandler handler =
          (PageLayoutReadHandler) pageLayoutChilds.get(i);
      officeStyles.addPageStyle(handler.getPageLayout());
    }

    for (int i = 0; i < dataStyleChilds.size(); i++)
    {
      final DataStyleReadHandler handler =
          (DataStyleReadHandler) dataStyleChilds.get(i);
      officeStyles.addDataStyle(handler.getDataStyle());
    }

    for (int i = 0; i < otherStyleChilds.size(); i++)
    {
      final SectionReadHandler handler =
          (SectionReadHandler) otherStyleChilds.get(i);
      officeStyles.addOtherNode((Element) handler.getNode());
    }
  }

  public Element getElement()
  {
    return officeStyles;
  }
}
