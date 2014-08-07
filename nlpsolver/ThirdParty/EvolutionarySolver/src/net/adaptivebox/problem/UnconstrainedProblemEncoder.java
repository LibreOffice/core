/**
 * Description: For unconstrained function
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Dec 28, 2001
 * Xiaofeng Xie    Mar 02, 2003
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
 * @Since MAOS1.0
 */

package net.adaptivebox.problem;

import net.adaptivebox.global.*;

public abstract class UnconstrainedProblemEncoder extends ProblemEncoder {
  protected UnconstrainedProblemEncoder(int NX) throws Exception {
    super(NX, 1);
    setDefaultYAt(0, BasicBound.MINDOUBLE, BasicBound.MINDOUBLE);
  }

  protected double calcTargetAt(int index, double[] VX) {
    return calcTarget(VX);
  }
  abstract public double calcTarget(double[] VX);
}
