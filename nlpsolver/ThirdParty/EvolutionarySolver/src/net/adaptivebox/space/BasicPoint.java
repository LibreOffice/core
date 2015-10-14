/**
 * Description: provide the location information of a point
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

package net.adaptivebox.space;

public class BasicPoint implements ILocationEngine {
  //store the location information in the search space (S)
  private final double[] location;

  public BasicPoint(int dim) {
    location = new double[dim];
  }

  public double[] getLocation() {
    return location;
  }

  public void importLocation(double[] pointLoc) {
    System.arraycopy(pointLoc, 0, location, 0, pointLoc.length);
  }

  public void importLocation(ILocationEngine point) {
    importLocation(point.getLocation());
  }
}