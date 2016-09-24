/**
 * Description: For formation the basic goodness landscape.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Jun 24, 2003     Created
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
 *
 * @version 1.0
 * @Since MAOS1.2
 *
 * [1] Deb K. An efficient constraint handling method for genetic algorithms.
 * Computer Methods in Applied Mechanics and Engineering, 2000, 186(2-4): 311-338
 */

package net.adaptivebox.goodness;

public class BCHComparator implements IGoodnessCompareEngine {

/* check the magnitude of two array, the frontal is more important
 **/
  private static int compareArray(double[] fit1, double[] fit2) {
    for (int i=0; i<fit1.length; i++) {
      if (fit1[i]>fit2[i]) {
        return LARGER_THAN; //Large than
      } else if (fit1[i]<fit2[i]){
        return LESS_THAN; //Less than
      }
    }
    return IGoodnessCompareEngine.EQUAL_TO; //same
  }

  public int compare(double[] fit1, double[] fit2) {
    return compareArray(fit1, fit2);
  }
}
