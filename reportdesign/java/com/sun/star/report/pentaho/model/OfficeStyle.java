/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeStyle.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:07 $
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


package com.sun.star.report.pentaho.model;

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
    return findFirstChild(OfficeNamespaces.STYLE_NS, "graphic-properties");
  }
}
