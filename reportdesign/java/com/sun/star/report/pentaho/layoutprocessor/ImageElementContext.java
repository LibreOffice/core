/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageElementContext.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:05 $
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


package com.sun.star.report.pentaho.layoutprocessor;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 30.03.2007
 */
public class ImageElementContext
{
  private int colSpan;
  private int rowSpan;
  private String[] rowStyles;
  private String[] colStyles;

  public ImageElementContext(final int colSpan, final int rowSpan)
  {
    this.colSpan = colSpan;
    this.rowSpan = rowSpan;
    this.colStyles = new String[colSpan];
    this.rowStyles = new String[rowSpan];
  }

  public int getColSpan()
  {
    return colSpan;
  }

  public int getRowSpan()
  {
    return rowSpan;
  }

  public String[] getRowStyles()
  {
    return rowStyles;
  }

  public String[] getColStyles()
  {
    return colStyles;
  }

  public void setRowStyle (final int pos, final String styleName)
  {
    rowStyles[pos] = styleName;
  }

  public void setColStyle (final int pos, final String styleName)
  {
    colStyles[pos] = styleName;
  }

  public String getRowStyle (final int pos)
  {
    return rowStyles[pos];
  }

  public String getColStyle (final int pos)
  {
    return colStyles[pos];
  }
}
