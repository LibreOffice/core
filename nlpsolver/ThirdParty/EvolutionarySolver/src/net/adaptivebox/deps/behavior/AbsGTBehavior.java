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

import net.adaptivebox.goodness.IGoodnessCompareEngine;
import net.adaptivebox.knowledge.ILibEngine;
import net.adaptivebox.knowledge.Library;
import net.adaptivebox.knowledge.SearchPoint;
import net.adaptivebox.problem.ProblemEncoder;
import net.adaptivebox.space.BasicPoint;

abstract public class AbsGTBehavior implements ILibEngine {
  // The referred social library
  protected Library socialLib;

  // the own memory: store the personal best point
  protected SearchPoint pbest_t;

  public void setLibrary(Library lib) {
    socialLib = lib;
  }

  public void setPbest(SearchPoint pbest) {
    pbest_t = pbest;
  }

  abstract public void setMemPoints(SearchPoint pbest, BasicPoint pcurrent, BasicPoint pold);

  abstract public void generateBehavior(SearchPoint trailPoint, ProblemEncoder problemEncoder);

  abstract public void testBehavior(SearchPoint trailPoint, IGoodnessCompareEngine qualityComparator);
}
