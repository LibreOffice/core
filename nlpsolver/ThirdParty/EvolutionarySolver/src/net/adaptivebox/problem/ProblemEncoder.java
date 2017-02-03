/**
 * Description: Encodes the specified problem into encoded information for
 * forming the goodness landscape.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    May 31, 2000
 * Xiaofeng Xie    Sep. 19, 2002
 * Xiaofeng Xie    Mar. 01, 2003
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
 * @Since MAOS1.0
 */

package net.adaptivebox.problem;

import net.adaptivebox.global.*;
import net.adaptivebox.space.*;
import net.adaptivebox.encode.*;
import net.adaptivebox.knowledge.*;

public abstract class ProblemEncoder {
  //Store the calculated results for the responses
  private final double[] tempResponseSet;  //temp values
  private final double[] tempLocation;  //temp values

  //the search space (S)
  private final DesignSpace designSpace;

  // For evaluate the response vector into encoded vector double[2]
  private final EvalStruct evalStruct;

  protected ProblemEncoder(int paramNum, int targetNum) throws Exception {
    designSpace = new DesignSpace(paramNum);
    evalStruct = new EvalStruct(targetNum);
    tempLocation = new double[paramNum];
    tempResponseSet = new double[targetNum];
  }

  public DesignSpace getDesignSpace() {
    return designSpace;
  }

  //set the default information for each dimension of search space (S)
  protected void setDefaultXAt(int i,  double min, double max, double grain) {
    DesignDim dd = new DesignDim();
    dd.grain = grain;
    dd.paramBound = new BasicBound(min, max);
    designSpace.setElemAt(dd, i);
  }



  //set the default information for evaluation each response
  protected void setDefaultYAt(int i,  double min, double max) {
    EvalElement ee = new EvalElement();
    ee.targetBound = new BasicBound(min, max);
    evalStruct.setElemAt(ee, i);
  }



  //get a fresh point
  public SearchPoint getFreshSearchPoint() {
    return new SearchPoint(designSpace.getDimension());
  }

  //get an encoded point
  public SearchPoint getEncodedSearchPoint() {
    SearchPoint point = getFreshSearchPoint();
    designSpace.initializeGene(point.getLocation());
    evaluate(point);
    return point;
  }

  //evaluate the point into encoded information
  public void evaluate(SearchPoint point) {
    //copy to temp point
    System.arraycopy(point.getLocation(), 0, this.tempLocation, 0, tempLocation.length);
    //mapping the temp point to original search space S
    designSpace.getMappingPoint(tempLocation);
    //calculate based on the temp point
    calcTargets(tempResponseSet, tempLocation);
    evalStruct.evaluate(point.getEncodeInfo(), tempResponseSet);
    point.setObjectiveValue(tempResponseSet[0]);
  }

  //calculate each response, must be implemented
  abstract protected double calcTargetAt(int index, double[] VX);

  // calculate all the responses VY[] based on given point VX[]
  private void calcTargets(double[] VY, double[] VX) {
    for(int i=0; i<VY.length; i++) {
      VY[i] = calcTargetAt(i, VX);
    }
  }
}

