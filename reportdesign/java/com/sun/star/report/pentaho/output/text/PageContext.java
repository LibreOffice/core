/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageContext.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:08 $
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


package com.sun.star.report.pentaho.output.text;

import org.jfree.layouting.input.style.values.CSSNumericValue;
import com.sun.star.report.pentaho.styles.LengthCalculator;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 24.03.2007
 */
public class PageContext
{
  public static final int KEEP_TOGETHER_OFF = 0;
  public static final int KEEP_TOGETHER_GROUP = 1;
  public static final int KEEP_TOGETHER_FIRST_DETAIL = 2;

  private PageContext parent;

  private String header;
  private CSSNumericValue headerHeight;
  private String footer;
  private CSSNumericValue footerHeight;
  private int keepTogether;
  private Integer columnCount;
  private boolean sectionOpen;

  public PageContext()
  {
    this(null);
  }

  public PageContext(final PageContext parent)
  {
    this.parent = parent;
    this.columnCount = null;
    if (parent != null)
    {
      this.keepTogether = parent.getKeepTogether();
    }
  }

  public int getActiveColumns()
  {
    PageContext pc = this;
    while (pc != null)
    {
      if (columnCount != null)
      {
        return columnCount.intValue();
      }
      pc = pc.getParent();
    }
    return 1;
  }

  public void setColumnCount(final Integer columnCount)
  {
    this.columnCount = columnCount;
  }

  public Integer getColumnCount()
  {
    return columnCount;
  }

  public String getHeader()
  {
    return header;
  }

  public void setHeader(final String header, final CSSNumericValue height)
  {
    this.header = header;
    this.headerHeight = height;
  }

  public String getFooter()
  {
    return footer;
  }


  public CSSNumericValue getHeaderHeight()
  {
    return headerHeight;
  }

  public CSSNumericValue getFooterHeight()
  {
    return footerHeight;
  }

  public void setFooter(final String footer, final CSSNumericValue height)
  {
    this.footer = footer;
    this.footerHeight = height;
  }

  public int getKeepTogether()
  {
    return keepTogether;
  }

  public void setKeepTogether(final int keepTogether)
  {
    this.keepTogether = keepTogether;
  }

  public PageContext getParent()
  {
    return parent;
  }

  public CSSNumericValue getAllFooterSize()
  {
    if (parent == null)
    {
      return footerHeight;
    }

    final LengthCalculator lnc = new LengthCalculator();
    PageContext pc = this;
    while (pc != null)
    {
      lnc.add(pc.getFooterHeight());
      pc = pc.getParent();
    }
    return lnc.getResult();
  }

  public CSSNumericValue getAllHeaderSize()
  {
    if (parent == null)
    {
      return headerHeight;
    }

    final LengthCalculator lnc = new LengthCalculator();
    PageContext pc = this;
    while (pc != null)
    {
      lnc.add(pc.getHeaderHeight());
      pc = pc.getParent();
    }
    return lnc.getResult();
  }


  public String getPageFooterContent()
  {
    if (parent == null)
    {
      return getFooter();
    }

    final StringBuffer b = new StringBuffer();

    PageContext pc = this;
    while (pc != null)
    {
      final String footer = pc.getFooter();
      if (footer != null)
      {
        b.append(footer);
      }
      pc = pc.getParent();
    }

    if (b.length() != 0)
    {
      return b.toString();
    }
    return null;
  }

  public String getPageHeaderContent()
  {
    if (parent == null)
    {
      return getHeader();
    }

    final StringBuffer b = new StringBuffer();

    PageContext pc = this;
    while (pc != null)
    {
      final String header = pc.getHeader();
      if (header != null)
      {
        b.append(header);
      }
      pc = pc.getParent();
    }

    if (b.length() != 0)
    {
      return b.toString();
    }
    return null;
  }


  public boolean isSectionOpen()
  {
    return sectionOpen;
  }

  public void setSectionOpen(final boolean sectionOpen)
  {
    this.sectionOpen = sectionOpen;
  }
}
