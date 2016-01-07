/**
 * Description: provide the information for evaluating of a response (target)
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Mar 1, 2003
 * Xiaofeng Xie    May 11, 2004
*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Please acknowledge the author(s) if you use this code in any way.
 */

package net.adaptivebox.encode;

import net.adaptivebox.global.*;

public class EvalElement {

  //The weight for each response (target)
  private static final double weight = 1;
  /**
   * The expected range of the response value, forms the following objective:
   *
   * <pre>
   * NO minValue   maxValue : THE ELEMENT OF BasicBound
   * 1  MINDOUBLE, MINDOUBLE: the minimize objective
   * 2  MAXDOUBLE, MAXDOUBLE: the maximize objective
   * 3  MINDOUBLE, v        : the less than constraint  {@literal (<v)}
   * 4  v        , MAXDOUBLE: the larger than constraint {@literal (>v)}
   * 5  v1       , v2       : the region constraint, i.e. belongs to [v1, v2]
   *
   * OPTIM type: the No.1 and No.2
   * CONS  type: the last three
   * </pre>
   */
  public BasicBound targetBound = new BasicBound();

  public boolean isOptType() {
    return ((targetBound.minValue==BasicBound.MINDOUBLE&&targetBound.maxValue==BasicBound.MINDOUBLE)||
            (targetBound.minValue==BasicBound.MAXDOUBLE&&targetBound.maxValue==BasicBound.MAXDOUBLE));
  }

  public double evaluateCONS(double targetValue) {
    if(targetValue<targetBound.minValue) {
      return weight*(targetBound.minValue-targetValue);
    }
    if(targetValue>targetBound.maxValue) {
      return weight*(targetValue-targetBound.maxValue);
    }
    return 0;
  }

  public double evaluateOPTIM(double targetValue) {
    if(targetBound.maxValue==BasicBound.MINDOUBLE) { //min mode
      return weight*targetValue;
    } else { //max
      return -weight*targetValue;
    }
  }
}

