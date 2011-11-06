/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

    public void add(final CSSNumericValue value)
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

    public CSSNumericValue getResult()
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
