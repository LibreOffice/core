/**
 * Description: provide the location and encoded goodness information
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
package net.adaptivebox.knowledge;

import net.adaptivebox.encode.IEncodeEngine;
import net.adaptivebox.global.BasicBound;
import net.adaptivebox.space.BasicPoint;

public class SearchPoint extends BasicPoint implements IEncodeEngine {
  // store the encode information for goodness evaluation
  // encodeInfo[0]: the sum of constraints (if it equals to 0, then be a feasible point)
  // encodeInfo[1]: the value of objective function
  private final double[] encodeInfo = new double[2];
  private double objectiveValue;

  public SearchPoint(int dim) {
    super(dim);
    for (int i = 0; i < encodeInfo.length; i++) {
      encodeInfo[i] = BasicBound.MAXDOUBLE;
    }
  }

  public double[] getEncodeInfo() {
    return encodeInfo;
  }

  private void importEncodeInfo(double[] info) {
    System.arraycopy(info, 0, encodeInfo, 0, encodeInfo.length);
  }

  private void importEncodeInfo(IEncodeEngine point) {
    importEncodeInfo(point.getEncodeInfo());
  }

  // Replace self by given point
  public void importPoint(SearchPoint point) {
    importLocation(point);
    importEncodeInfo(point);
    setObjectiveValue(point.getObjectiveValue());
  }

  public double getObjectiveValue() {
    return objectiveValue;
  }

  public void setObjectiveValue(double objectiveValue) {
    this.objectiveValue = objectiveValue;
  }

  public boolean isFeasible() {
    return encodeInfo[0] == 0; // no constraint violations
  }
}
