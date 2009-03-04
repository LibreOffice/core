/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LengthCalculator.java,v $
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
    // LOGGER.debug ("Adding " + value  + " [mm: " + millimeter + "] [pt: " + point + "] px: [" + pixel + "]");
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
