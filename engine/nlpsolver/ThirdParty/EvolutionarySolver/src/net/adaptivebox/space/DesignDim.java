/**
 * Description: provide the information for goodness evaluation of a target
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Mar  1, 2003
 * Xiaofeng Xie    May  3, 2004    Add grain value
 * Xiaofeng Xie    May 11, 2004    Add crowd distance
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

import net.adaptivebox.global.BasicBound;

public class DesignDim {
  // To discrete space with the given step. For example, for an integer variable,
  // The grain value can be set as 1.
  public double grain = 0;
  public BasicBound paramBound = new BasicBound(); // the range of a parameter

  public boolean isDiscrete() {
    return grain != 0;
  }

  public double getGrainedValue(double value) {
    if (grain == 0) {
      return value;
    } else if (grain > 0) {
      return paramBound.minValue + Math.rint((value - paramBound.minValue) / grain) * grain;
    } else {
      return paramBound.maxValue - Math.rint((paramBound.maxValue - value) / grain) * grain;
    }
  }
}
