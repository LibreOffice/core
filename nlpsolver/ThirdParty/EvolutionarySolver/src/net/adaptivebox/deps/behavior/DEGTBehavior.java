/**
 * Description: The description of differential evolution Generate-and-Test Behavior.

  #Supported parameters:
  NAME    VALUE_type   Range      DefaultV        Description
  FACTOR   real        (0, 1.2]   0.5             DEAgent: scale constant
  CR       real        [0, 1]     0.9             DEAgent: crossover constant
  //Other choices for FACTOR and CR: (0.5, 0.1)

 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    May 11, 2004
 * Xiaofeng Xie    Jul 01, 2008
 *
 * @version 1.0
 * @Since MAOS1.0
 *
 * @References:
 * [1] Storn R, Price K. Differential evolution - a simple and efficient
 *     heuristic for global optimization over continuous spaces. Journal of
 *     Global Optimization, 1997, 11: 341-359
 *     The original differential evolution idea
 * [2] X F Xie, W J Zhang. SWAF: swarm algorithm framework for numerical
 *     optimization. Genetic and Evolutionary Computation Conference (GECCO),
 *     Seattle, WA, USA, 2004: 238-250
 *     -> a generate-and-test behavior
 */

package net.adaptivebox.deps.behavior;

import net.adaptivebox.goodness.*;
import net.adaptivebox.global.*;
import net.adaptivebox.knowledge.*;
import net.adaptivebox.problem.*;
import net.adaptivebox.space.*;

public class DEGTBehavior extends AbsGTBehavior implements ILibEngine {
  private static final int DVNum = 2;  //Number of differential vectors, normally be 1 or 2
  public double FACTOR = 0.5; //scale constant: (0, 1.2], normally be 0.5
  public double CR = 0.9;     //crossover constant: [0, 1], normally be 0.1 or 0.9

  //the own memory: store the point that generated in last learning cycle
  private SearchPoint pbest_t;

  public void setPbest(SearchPoint pbest) {
    pbest_t = pbest;
  }

  @Override
  public void generateBehavior(SearchPoint trailPoint, ProblemEncoder problemEncoder) {
    SearchPoint gbest_t = socialLib.getGbest();

    BasicPoint[] referPoints = getReferPoints();
    int DIMENSION = problemEncoder.getDesignSpace().getDimension();
    int rj = RandomGenerator.intRangeRandom(0, DIMENSION-1);
    for (int k=0; k<DIMENSION; k++) {
      if (Math.random()<CR || k == DIMENSION-1) {
        double Dabcd = 0;
        for(int i=0; i<referPoints.length; i++) {
          Dabcd += (i%2==0 ? +1D : -1D)*referPoints[i].getLocation()[rj];
        }
        trailPoint.getLocation()[rj] = gbest_t.getLocation()[rj]+FACTOR*Dabcd;
      } else {
        trailPoint.getLocation()[rj] = pbest_t.getLocation()[rj];
      }
      rj = (rj+1)%DIMENSION;
    }
  }

  @Override
  public void testBehavior(SearchPoint trailPoint, IGoodnessCompareEngine qualityComparator) {
    Library.replace(qualityComparator, trailPoint, pbest_t);
  }

  private SearchPoint[] getReferPoints() {
    SearchPoint[] referPoints = new SearchPoint[DVNum*2];
    for(int i=0; i<referPoints.length; i++) {
      referPoints[i] = socialLib.getSelectedPoint(RandomGenerator.intRangeRandom(0, socialLib.getPopSize()-1));
    }
    return referPoints;
  }
}

