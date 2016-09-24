/**
 * Description: provide the information for evaluating a set of targets values
 * into encoded information (For formation the goodness landscape by comparing)
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Mar 1, 2003
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
 * @References:
 * [1] Deb K. An efficient constraint handling method for genetic algorithms.
 * Computer Methods in Applied Mechanics and Engineering, 2000, 186(2-4): 311-338
 */

package net.adaptivebox.encode;

public class EvalStruct {
  // The information for evaluating all the responses
  private EvalElement[] evalElems = null;

  public EvalStruct(int elemsNum) {
    evalElems = new EvalElement[elemsNum];
  }

  public void setElemAt(EvalElement dim, int index) {
    evalElems[index] = dim;
  }

  //convert response values into encoded information double[2]
  public void evaluate(double[] evalRes, double[] targetValues) {
    evalRes[0] = evalRes[1] = 0;
    for(int i=0; i<evalElems.length; i++) {
      if (evalElems[i].isOptType()) {
        //The objectives (OPTIM type)
        //The multi-objective will be translated into single-objective
        evalRes[1] += evalElems[i].evaluateOPTIM(targetValues[i]);
      } else {
        //The constraints (CONS type)
        //If evalRes[0] equals to 0, then be a feasible point, i.e. satisfies
        // all the constraints
        evalRes[0] += evalElems[i].evaluateCONS(targetValues[i]);
      }
    }
  }
}

