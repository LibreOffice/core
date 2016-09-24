package net.adaptivebox.deps;

/**
 * Description: The description of agent with hybrid differential evolution and particle swarm.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Jun 10, 2004
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
 * @References:
 * [1] Zhang W J, Xie X F. DEPSO: hybrid particle swarm with differential
 * evolution operator. IEEE International Conference on Systems, Man & Cybernetics,
 * Washington D C, USA, 2003: 3816-3821
 * [2] X F Xie, W J Zhang. SWAF: swarm algorithm framework for numerical
 *     optimization. Genetic and Evolutionary Computation Conference (GECCO),
 *     Seattle, WA, USA, 2004: 238-250
 *     -> an agent perspective
 */

import net.adaptivebox.deps.behavior.*;
import net.adaptivebox.goodness.IGoodnessCompareEngine;
import net.adaptivebox.knowledge.*;
import net.adaptivebox.problem.*;
import net.adaptivebox.space.*;

public class DEPSAgent implements ILibEngine {

  //Describes the problem to be solved
  private ProblemEncoder problemEncoder;
  //Forms the goodness landscape
  private IGoodnessCompareEngine qualityComparator;

  //store the point that generated in current learning cycle
  private SearchPoint trailPoint;

  //temp variable
  private AbsGTBehavior selectGTBehavior;

  //the own memory: store the point that generated in old learning cycle
  private BasicPoint pold_t;
  //the own memory: store the point that generated in last learning cycle
  private BasicPoint pcurrent_t;
  //the own memory: store the personal best point
  private SearchPoint pbest_t;

  //Generate-and-test Behaviors
  private DEGTBehavior deGTBehavior;
  private PSGTBehavior psGTBehavior;
  public double switchP = 0.5;

  public void setLibrary(Library lib) {
    deGTBehavior.setLibrary(lib);
    psGTBehavior.setLibrary(lib);
  }

  public void setProblemEncoder(ProblemEncoder encoder) {
    problemEncoder = encoder;
    trailPoint = problemEncoder.getFreshSearchPoint();
    pold_t = problemEncoder.getFreshSearchPoint();
    pcurrent_t = problemEncoder.getFreshSearchPoint();
  }

  public void setSpecComparator(IGoodnessCompareEngine comparer) {
    qualityComparator = comparer;
  }

  public void setPbest(SearchPoint pbest) {
    pbest_t = pbest;
  }

  private AbsGTBehavior getGTBehavior() {
    if (Math.random()<switchP) {
      return deGTBehavior;
    } else {
      return psGTBehavior;
    }
  }

  public void setGTBehavior(AbsGTBehavior gtBehavior) {
    if (gtBehavior instanceof DEGTBehavior) {
      deGTBehavior = ((DEGTBehavior)gtBehavior);
      deGTBehavior.setPbest(pbest_t);
      return;
    }
    if (gtBehavior instanceof PSGTBehavior) {
      psGTBehavior = ((PSGTBehavior)gtBehavior);
      psGTBehavior.setMemPoints(pbest_t, pcurrent_t, pold_t);
      return;
    }
  }

  public void generatePoint() {
    // generates a new point in the search space (S) based on
    // its memory and the library
    selectGTBehavior = this.getGTBehavior();
    selectGTBehavior.generateBehavior(trailPoint, problemEncoder);
    //evaluate into goodness information
    problemEncoder.evaluate(trailPoint);
  }

  public void learn() {
    selectGTBehavior.testBehavior(trailPoint, qualityComparator);
  }

  public SearchPoint getMGState() {
    return trailPoint;
  }
}

