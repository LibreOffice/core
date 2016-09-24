/**
 * Description: The description of generate-and-test behavior.
 *
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    May 17, 2004
 * Xiaofeng Xie    Jul 01, 2008
 *
 * @version 1.0
 * @Since MAOS1.0
 *
 * @References:
 * [1] X F Xie, W J Zhang. SWAF: swarm algorithm framework for numerical
 *     optimization. Genetic and Evolutionary Computation Conference (GECCO),
 *     Seattle, WA, USA, 2004: 238-250
 *     -> a generate-and-test behavior
 */
package net.adaptivebox.deps.behavior;

import net.adaptivebox.goodness.*;
import net.adaptivebox.knowledge.*;
import net.adaptivebox.problem.*;

abstract public class AbsGTBehavior {
  //The referred social library
  protected Library socialLib;

  public void setLibrary(Library lib) {
    socialLib = lib;
  }

  abstract public void generateBehavior(SearchPoint trailPoint, ProblemEncoder problemEncoder);

  abstract public void testBehavior(SearchPoint trailPoint, IGoodnessCompareEngine qualityComparator);
}

