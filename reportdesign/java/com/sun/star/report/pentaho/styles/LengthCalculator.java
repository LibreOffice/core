/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LengthCalculator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:49:35 $
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


package com.sun.star.report.pentaho.styles;

import org.jfree.layouting.input.style.values.CSSNumericType;
import org.jfree.layouting.input.style.values.CSSNumericValue;

/**
 * A helper class that sums up various CSS-length. The various unit types are
 * kept separate until the final result is computed to minimize the computation
 * inaccuracy.
 *
 * @author Thomas Morgner
 * @since 15.03.2007
 */
public class LengthCalculator
{
  // and centimeter (x10)
  private double millimeter;

  // and pica (x12) and inch(x72). Px is assumed to be in 96dpi.
  private double point;

  private double pixel;

  public LengthCalculator()
  {
  }

  public void add (final CSSNumericValue value)
  {
    if (value == null)
    {
      return;
    }

    final CSSNumericType numericType = value.getType();
    if (numericType == CSSNumericType.CM)
    {
      millimeter += value.getValue() * 10;
    }
    else if (numericType == CSSNumericType.MM)
    {
      millimeter += value.getValue();
    }
    else if (numericType == CSSNumericType.PT)
    {
      point += value.getValue();
    }
    else if (numericType == CSSNumericType.PC)
    {
      point += 12 * value.getValue();
    }
    else if (numericType == CSSNumericType.INCH)
    {
      point += 72 * value.getValue();
    }
    else if (numericType == CSSNumericType.PX)
    {
      pixel += value.getValue();
    }
    // Log.debug ("Adding " + value  + " [mm: " + millimeter + "] [pt: " + point + "] px: [" + pixel + "]");
  }

  public CSSNumericValue getResult ()
  {
    if (pixel == 0 && point == 0)
    {
      return CSSNumericValue.createValue(CSSNumericType.MM, millimeter);
    }
    if (pixel == 0 && millimeter == 0)
    {
      return CSSNumericValue.createValue(CSSNumericType.PT, point);
    }
    if (point == 0 && millimeter == 0)
    {
      return CSSNumericValue.createValue(CSSNumericType.PX, pixel);
    }
    // else convert it.

    double result = point;
    result += (millimeter * 10 * 72 / 254);
    result += pixel * 72 / 96;

    return CSSNumericValue.createValue(CSSNumericType.PT, result);
  }
}
