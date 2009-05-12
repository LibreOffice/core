/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImageElementContext.java,v $
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


package com.sun.star.report.pentaho.layoutprocessor;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 30.03.2007
 */
public class ImageElementContext
{
  private final int colSpan;
  private final int rowSpan;
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
