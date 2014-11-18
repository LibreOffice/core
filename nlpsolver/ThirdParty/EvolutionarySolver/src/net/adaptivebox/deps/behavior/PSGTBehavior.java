/**
 * Description: The description of particle swarm (PS) Generate-and-test Behavior.
 *
  #Supported parameters:
  NAME    VALUE_type   Range      DefaultV        Description
  c1       real        [0, 2]     1.494           PSAgent: learning factor for pbest
  c2       real        [0, 2]     1.494           PSAgent: learning factor for gbest
  w        real        [0, 1]     0.729           PSAgent: inertia weight
  CL       real        [0, 0.1]   0               PSAgent: chaos factor
  //Other choices for c1, c2, w, and CL: (2, 2, 0.4, 0.001)

 * Author          Create/Modi     Note
 * Xiaofeng Xie    May 11, 2004
 * Xiaofeng Xie    Jul 01, 2008
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
 *
 * References:
 * [1] Kennedy J, Eberhart R C. Particle swarm optimization. IEEE Int. Conf. on
 * Neural Networks, Perth, Australia, 1995: 1942-1948
 *   For original particle swarm idea
 * [2] Shi Y H, Eberhart R C. A Modified Particle Swarm Optimizer. IEEE Inter. Conf.
 * on Evolutionary Computation, Anchorage, Alaska, 1998: 69-73
 *   For the inertia weight: adjust the trade-off between exploitation & exploration
 * [3] Clerc M, Kennedy J. The particle swarm - explosion, stability, and
 * convergence in a multidimensional complex space. IEEE Trans. on Evolutionary
 * Computation. 2002, 6 (1): 58-73
 *   Constriction factor: ensures the convergence
 * [4] Xie X F, Zhang W J, Yang Z L. A dissipative particle swarm optimization.
 * Congress on Evolutionary Computation, Hawaii, USA, 2002: 1456-1461
 *   The CL parameter
 * [5] Xie X F, Zhang W J, Bi D C. Optimizing semiconductor devices by self-
 * organizing particle swarm. Congress on Evolutionary Computation, Oregon, USA,
 * 2004: 2017-2022
 *   Further experimental analysis on the convergence of PSO
 * [6] X F Xie, W J Zhang. SWAF: swarm algorithm framework for numerical
 *     optimization. Genetic and Evolutionary Computation Conference (GECCO),
 *     Seattle, WA, USA, 2004: 238-250
 *     -> a generate-and-test behavior
 *
 */

package net.adaptivebox.deps.behavior;

import net.adaptivebox.goodness.*;
import net.adaptivebox.knowledge.*;
import net.adaptivebox.problem.*;
import net.adaptivebox.space.*;

public class PSGTBehavior extends AbsGTBehavior {
  // Two normally choices for (c1, c2, weight), i.e., (2, 2, 0.4), or (1.494, 1.494, 0.729)
  // The first is used in dissipative PSO (cf. [4]) as CL>0, and the second is achieved by using
  // constriction factors (cf. [3])
  public double c1=2;
  public double c2=2;
  public double weight = 0.4; //inertia weight

  public double CL=0;  //See ref[4], normally be 0.001~0.005

  //the own memory: store the point that generated in old learning cycle
  private BasicPoint pold_t;
  //the own memory: store the point that generated in last learning cycle
  private BasicPoint pcurrent_t;
  //the own memory: store the personal best point
  private SearchPoint pbest_t;

  public void setMemPoints(SearchPoint pbest, BasicPoint pcurrent, BasicPoint pold) {
    pcurrent_t = pcurrent;
    pbest_t = pbest;
    pold_t = pold;
  }

  @Override
  public void generateBehavior(SearchPoint trailPoint, ProblemEncoder problemEncoder) {
    SearchPoint gbest_t = socialLib.getGbest();
    DesignSpace designSpace = problemEncoder.getDesignSpace();
    int DIMENSION = designSpace.getDimension();
    double deltaxb, deltaxbm;
    for (int b=0;b<DIMENSION;b++) {
      if (Math.random()<CL) {
        designSpace.mutationAt(trailPoint.getLocation(), b);
      } else {
        deltaxb = weight*(pcurrent_t.getLocation()[b]-pold_t.getLocation()[b])
             + c1*Math.random()*(pbest_t.getLocation()[b]-pcurrent_t.getLocation()[b])
             + c2*Math.random()*(gbest_t.getLocation()[b]-pcurrent_t.getLocation()[b]);
        //limitation for delta_x
        deltaxbm = 0.5*designSpace.getMagnitudeIn(b);
        if(deltaxb<-deltaxbm) {
          deltaxb = -deltaxbm;
        } else if (deltaxb>deltaxbm) {
          deltaxb = deltaxbm;
        }
        trailPoint.getLocation()[b] = pcurrent_t.getLocation()[b]+deltaxb;
      }
    }
  }

  @Override
  public void testBehavior(SearchPoint trailPoint, IGoodnessCompareEngine qualityComparator) {
    Library.replace(qualityComparator, trailPoint, pbest_t);
    pold_t.importLocation(pcurrent_t);
    pcurrent_t.importLocation(trailPoint);
   }

}

