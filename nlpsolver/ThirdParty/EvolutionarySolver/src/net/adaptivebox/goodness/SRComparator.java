/**
 * Description: For comparison of goodness in landscape with stoch. ranking.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    May 29, 2004
 *
 * [1] Runarsson T P, Yao X. Stochastic ranking for constrained evolutionary
 * optimization. IEEE Trans. on Evolutionary Computation. 2000, 4 (3): 284-294
 *
 */

package net.adaptivebox.goodness;

import net.adaptivebox.global.*;

public class SRComparator implements IGoodnessCompareEngine, IUpdateCycleEngine {
  public double Pf = 0.5;

  public SRComparator() {}

  public SRComparator(double pf) {
    this.Pf = pf;
  }

  public void updateCycle(int t) {
    Pf *=0.995;
  }

  /**
   *  check the magnitude of two array, the frontial is more important
   * Stoch ranking: array size = 2
   **/
  public int compare(double[] fit1, double[] fit2) {
    if((fit1[0]==fit2[0])||Pf>Math.random()) {
      return GlobalCompare.compare(fit1[1], fit2[1]);
    } else {
      return GlobalCompare.compare(fit1[0], fit2[0]);
    }
  }

}
