/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeStyle.java,v $
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


package com.sun.star.report.pentaho.model;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

/**
 * Represents an automatic or manual style definition.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class OfficeStyle extends Section
{
  public OfficeStyle()
  {
    setNamespace(OfficeNamespaces.STYLE_NS);
    setType("style");
  }

  public String getStyleName()
  {
    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
  }

  public void setStyleName(final String name)
  {
    setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
  }

  /**
   * A parent style name must be a common style (it cannot be an automatic
   * style) and has to exist. If no parent style is given, an implementation
   * specific default style is used.
   *
   * @return
   */
  public String getStyleParent()
  {
    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "parent-style-name");
  }

  public void setStyleParent(final String parentName)
  {
    setAttribute(OfficeNamespaces.STYLE_NS, "parent-style-name", parentName);
  }

  public String getStyleFamily()
  {
    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "family");
  }

  public void setStyleFamily(final String family)
  {
    setAttribute(OfficeNamespaces.STYLE_NS, "family", family);
  }

//  public String getMasterPageName()
//  {
//    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "master-page-name");
//  }

  public Element getParagraphProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "paragraph-properties");
  }

  public Element getTextProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "text-properties");
  }

  public Element getTableRowProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "table-row-properties");
  }

  public Element getTableProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "table-properties");
  }

  public Element getTableColumnProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "table-column-properties");
  }

  public Element getSectionProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "section-properties");
  }

  public Element getTableCellProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, "table-cell-properties");
  }

  public Element getGraphicProperties ()
  {
    return findFirstChild(OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
  }
}
