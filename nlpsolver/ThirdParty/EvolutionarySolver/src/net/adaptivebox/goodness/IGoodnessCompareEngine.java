/**
 * Description: For comparison of goodness.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Feb 19, 2004
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
 */

package net.adaptivebox.goodness;

public abstract interface IGoodnessCompareEngine {
  int LARGER_THAN = 2;
  int EQUAL_TO = 1;
  int LESS_THAN = 0;

  /**
   * check the magnitude of two IEncodeEngine
   *
   * LARGER_THAN: goodness1 is worse than goodness2
   *
   * LESS_THAN: goodness1 is better than goodness2
   *
   * EQUAL_TO : goodness1 is equal to goodness2
   */
  int compare(double[] goodness1, double[] goodness2);
}
