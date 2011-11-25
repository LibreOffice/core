/**
 * Description: For comparison of goodness in landscape with penalty method.
 *
 * @Applied domain: efficiently for ridge class feasible space (SF), such as
 *  the problem with eqaulity constraints
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    May 29, 2004
 *
 * @version 1.0
  *
 * [1] Runarsson T P, Yao X. Stochastic ranking for constrained evolutionary
 * optimization. IEEE Trans. on Evolutionary Computation. 2000, 4 (3): 284-294
 *
 */

package net.adaptivebox.goodness;

import net.adaptivebox.global.*;

public class PenaltyComparator implements IGoodnessCompareEngine {
  public double Rg = 0;

  public PenaltyComparator() {}

  public PenaltyComparator(double rg) {
    this.Rg = rg;
  }

  public double calcPenaltyValue(double fit1, double fit2) {
    return fit1+Rg*fit2;
  }
  /**
   *  check the magnitude of two array, the frontial is more important
   * Stoch ranking: array size = 2
   **/
  public int compare(double[] fit1, double[] fit2) {
    return GlobalCompare.compare(calcPenaltyValue(fit1[1], fit1[0]), calcPenaltyValue(fit2[1], fit2[0]));
  }

}
