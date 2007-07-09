/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeStyles.java,v $
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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;

import org.jfree.report.structure.Element;

/**
 * Holds one style type, either an automatic, common or master style. This is a
 * marker container that defines the nature of the styles contained within this
 * container. (Yeah, it is awkward, but that's how the document model describes
 * it.)
 *
 * The style family can be one of paragraph, text, section, table, table-column,
 * table-row, table-cell, table-page, chart, default, drawing-page, graphic,
 * presentation, control and ruby.
 *
 * @author Thomas Morgner
 * @since 07.03.2007
 */
public class OfficeStyles extends Element
{
  public static class StyleKey implements Serializable
  {
    private static final long serialVersionUID = 4931878927362887477L;
    private String family;
    private String name;

    public StyleKey(final String family, final String name)
    {
      if (family == null)
      {
        throw new NullPointerException();
      }
      this.family = family;
      this.name = name;
    }


    public String getFamily()
    {
      return family;
    }

    public String getName()
    {
      return name;
    }

    public boolean equals(final Object obj)
    {
      if (this == obj)
      {
        return true;
      }
      if (obj == null || getClass() != obj.getClass())
      {
        return false;
      }

      final StyleKey styleKey = (StyleKey) obj;

      if (!family.equals(styleKey.family))
      {
        return false;
      }
      if (name != null ? !name.equals(styleKey.name) : styleKey.name != null)
      {
        return false;
      }

      return true;
    }

    public int hashCode()
    {
      int result = family.hashCode();
      result = 31 * result + (name != null ? name.hashCode() : 0);
      return result;
    }
  }

  private HashMap pageStyles;
  private HashMap dataStyles;
  private HashMap styles;
  private ArrayList otherChilds;

  public OfficeStyles()
  {
    this.styles = new HashMap();
    this.dataStyles = new HashMap();
    this.pageStyles = new HashMap();
    this.otherChilds = new ArrayList();
  }

  public OfficeStyle getStyle(final String family, final String name)
  {
    return (OfficeStyle) styles.get(new StyleKey(family, name));
  }

  public void addStyle (final OfficeStyle style)
  {
    if (style == null)
    {
      throw new NullPointerException();
    }
    final String styleFamily = style.getStyleFamily();
    if (styleFamily == null)
    {
      throw new NullPointerException();
    }
    if (style.getStyleName() == null)
    {
      throw new NullPointerException();
    }
    styles.put(new StyleKey(styleFamily, style.getStyleName()), style);
  }

  public void addPageStyle (final PageLayout style)
  {
    pageStyles.put(style.getStyleName(), style);
  }

  public PageLayout getPageStyle (final String name)
  {
    return (PageLayout) pageStyles.get (name);
  }

  public void addDataStyle(final DataStyle style)
  {
    dataStyles.put (style.getStyleName(), style);
  }

  public DataStyle getDataStyle (final String name)
  {
    return (DataStyle) dataStyles.get(name);
  }

  public void addOtherNode (final Element node)
  {
    otherChilds.add(node);
  }

  public DataStyle[] getAllDataStyles()
  {
    return (DataStyle[]) dataStyles.values().toArray
        (new DataStyle[dataStyles.size()]);
  }

  public PageLayout[] getAllPageStyles()
  {
    return (PageLayout[]) pageStyles.values().toArray
        (new PageLayout[pageStyles.size()]);
  }

  public OfficeStyle[] getAllStyles()
  {
    return (OfficeStyle[]) styles.values().toArray
        (new OfficeStyle[styles.size()]);
  }

  public Element[] getOtherStyles()
  {
    return (Element[]) otherChilds.toArray(new Element[otherChilds.size()]);
  }

  public boolean containsStyle(final String family, final String name)
  {
    return styles.containsKey(new StyleKey(family, name));
  }

  public boolean containsDataStyle(final String styleName)
  {
    return dataStyles.containsKey(styleName);
  }
}
