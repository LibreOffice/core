/**
 * Description: For comparison of goodness in landscape with loosed constraints
 *  which varied adaptively according to the social information.
 *
 * Applied domain: efficiently for ridge class feasible space (SF), such as
 *  the problem with equality constraints
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
 * [1] Xie X F, Zhang W J, Bi D C. Handling equality constraints by adaptive
 * relaxing rule for swarm algorithms. Congress on Evolutionary Computation,
 * Oregon, USA, 2004
 */

package net.adaptivebox.goodness;

import net.adaptivebox.knowledge.*;
import net.adaptivebox.global.*;


public class ACRComparator implements IGoodnessCompareEngine, IUpdateCycleEngine {
  private final Library socialPool;
  private double epsilon_t = 0;

  private static final double RU = 0.75;
  private static final double RL = 0.25;
  private static final double BETAF = 0.618;
  private static final double BETAL = 0.618;
  private static final double BETAU = 1.382;

  private final double T;

  private static final double TthR = 0.5;

  public ACRComparator(Library lib, int T) {
    socialPool = lib;
    this.T = T;
    //set the (epsilon_t|t=0) as the maximum CONS value among the SearchPoints in the library
    epsilon_t = lib.getExtremalVcon(true);
  }

  private static int compare(double data1, double data2) {
    if (data1 < data2)
      return LESS_THAN;
    else if (data1 > data2)
      return LARGER_THAN;
    else
      return EQUAL_TO;
  }

  public int compare(double[] fit1, double[] fit2) {
    if(Math.max(fit1[0], fit2[0])<=Math.max(0, epsilon_t)) { //epsilon>0
      return compare(fit1[1], fit2[1]);
    } else {
      return compare(fit1[0], fit2[0]);
    }
  }

  public void updateCycle(int t) {
    //calculates the ratio
    double rn = (double)socialPool.getVconThanNum(epsilon_t)/(double)socialPool.getPopSize();
    if(t>TthR*T &&T!=-1) { //Forcing sub-rule
      epsilon_t *= BETAF;
    } else {  	          //Ratio-keeping sub-rules
      if(rn>RU) {
        epsilon_t *= BETAL;  //Shrink
      }
      if(rn<RL) {
        epsilon_t *= BETAU;  //Relax
      }
    }
  }
}
