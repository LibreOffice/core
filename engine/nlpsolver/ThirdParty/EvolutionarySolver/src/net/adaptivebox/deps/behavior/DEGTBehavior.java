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

import net.adaptivebox.global.RandomGenerator;
import net.adaptivebox.goodness.IGoodnessCompareEngine;
import net.adaptivebox.knowledge.Library;
import net.adaptivebox.knowledge.SearchPoint;
import net.adaptivebox.problem.ProblemEncoder;
import net.adaptivebox.space.BasicPoint;

public class DEGTBehavior extends AbsGTBehavior {
  //Number of differential vectors, normally be 1 or 2
  private static final int DVNum = 2;

  //scale constant: (0, 1.2], normally be 0.5
  public double MIN_FACTOR = 0.5;

  //scale constant: (0, 1.2], normally be 0.5
  public double MAX_FACTOR = 0.5;

  //crossover constant: [0, 1], normally be 0.1 or 0.9
  public double CR = 0.9;

  @Override
  public void setMemPoints(SearchPoint pbest, BasicPoint pcurrent, BasicPoint pold) {
    pbest_t = pbest;
  }

  /**
   * Crossover and mutation for a single vector element done in a single step.
   *
   * @param index             Index of the trial vector element to be changed.
   * @param trialVector       Trial vector reference.
   * @param globalVector      Global best found vector reference.
   * @param differenceVectors List of vectors used for difference delta
   *                          calculation.
   */
  private void crossoverAndMutation(int index, double trialVector[], double globalVector[],
      BasicPoint differenceVectors[]) {
    double delta = 0D;

    for (int i = 0; i < differenceVectors.length; i++) {
      delta += (i % 2 == 0 ? +1D : -1D) * differenceVectors[i].getLocation()[index];
    }

    trialVector[index] = globalVector[index] + RandomGenerator.doubleRangeRandom(MIN_FACTOR, MAX_FACTOR) * delta;
  }

  @Override
  public void generateBehavior(SearchPoint trailPoint, ProblemEncoder problemEncoder) {
    BasicPoint[] referPoints = getReferPoints();
    int DIMENSION = problemEncoder.getDesignSpace().getDimension();
    int guaranteeIndex = RandomGenerator.intRangeRandom(0, DIMENSION - 1);

    double[] trailVector = trailPoint.getLocation();
    double[] locaclVector = pbest_t.getLocation();
    double[] globalVector = socialLib.getGbest().getLocation();

    /* Handle first part of the trial vector. */
    for (int index = 0; index < guaranteeIndex; index++) {
      if (CR <= RandomGenerator.doubleZeroOneRandom()) {
        trailVector[index] = locaclVector[index];
        continue;
      }

      crossoverAndMutation(index, trailVector, globalVector, referPoints);
    }

    /* Guarantee for at least one change in the trial vector. */
    crossoverAndMutation(guaranteeIndex, trailVector, globalVector, referPoints);

    /* Handle second part of the trial vector. */
    for (int index = guaranteeIndex + 1; index < DIMENSION; index++) {
      if (CR <= RandomGenerator.doubleZeroOneRandom()) {
        trailVector[index] = locaclVector[index];
        continue;
      }

      crossoverAndMutation(index, trailVector, globalVector, referPoints);
    }
  }

  @Override
  public void testBehavior(SearchPoint trailPoint, IGoodnessCompareEngine qualityComparator) {
    Library.replace(qualityComparator, trailPoint, pbest_t);
  }

  private SearchPoint[] getReferPoints() {
    SearchPoint[] referPoints = new SearchPoint[DVNum * 2];
    for (int i = 0; i < referPoints.length; i++) {
      referPoints[i] = socialLib.getRandomPoint();
    }
    return referPoints;
  }
}
