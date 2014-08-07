/**
 * Description: provide the information for the search space (S)
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Mar 2, 2003
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
 * [1] Zhang W J, Xie X F, Bi D C. Handling boundary constraints for numerical
 * optimization by particle swarm flying in periodic search space. Congress
 * on Evolutionary Computation, Oregon, USA, 2004
 * @ especially for particle swarm agent
 */

package net.adaptivebox.space;
import net.adaptivebox.global.*;

public class DesignSpace {
  //The information of all the dimension
  private DesignDim[] dimProps;

  public DesignSpace(int dim) {
    dimProps = new DesignDim[dim];
  }

  public DesignDim getDimAt(int index) {
    return dimProps[index];
  }

  public void setElemAt(DesignDim elem, int index) {
    dimProps[index] = elem;
  }

  public int getDimension() {
    if (dimProps==null) {
      return -1;
    }
    return dimProps.length;
  }

  public double boundAdjustAt(double val, int dim){
    return dimProps[dim].paramBound.boundAdjust(val);
  }

  public void annulusAdjust (double[] location){
    for (int i=0; i<getDimension(); i++) {
      location[i] = dimProps[i].paramBound.annulusAdjust(location[i]);
    }
  }

  public void randomAdjust (double[] location){
    for (int i=0; i<getDimension(); i++) {
      location[i] = dimProps[i].paramBound.randomAdjust(location[i]);
    }
  }

  public boolean satisfyCondition(double[] location){
    for (int i=0; i<getDimension(); i++) {
      if (!dimProps[i].paramBound.isSatisfyCondition(location[i])) {
        return false;
      }
    }
    /*If the limits are not violated, return TRUE*/
    return(true);
  }

  public void mutationAt(double[] location, int i){
    location[i] = dimProps[i].paramBound.getRandomValue();
  }

  public double mutationUniformAtPointAsCenter (double pointX, int i){
    double length = this.getMagnitudeIn(i)/2;
    pointX += RandomGenerator.doubleRangeRandom(-1*length, length);

    return pointX;
  }

  public double getUpValueAt(int dimensionIndex) {
    return dimProps[dimensionIndex].paramBound.maxValue;
  }

  public double getLowValueAt(int dimensionIndex) {
    return dimProps[dimensionIndex].paramBound.minValue;
  }

  public double getMagnitudeIn(int dimensionIndex) {
    return dimProps[dimensionIndex].paramBound.getLength();
  }


  public boolean initilizeGeneAtPointAsCenter(double[] tempX){
    if (tempX.length!=this.getDimension()) {
      return false;
    }
    for(int i=0;i<tempX.length;i++) {
      double length = this.getMagnitudeIn(i)/2;
      tempX[i]+=RandomGenerator.doubleRangeRandom(-1*length, length);
    }
    return true;
  }

  public void initializeGene(double[] tempX){
    for(int i=0;i<tempX.length;i++) tempX[i] =  dimProps[i].paramBound.getRandomValue(); //Global.RandomGenerator.doubleRangeRandom(9.8, 10);
  }

  public double[] getFreshGene() {
    double[] tempX = new double[this.getDimension()];
    initializeGene(tempX);
    return tempX;
  }
  public void getMappingPoint(double[] point) {
    for(int i=0; i<getDimension(); i++) {
      point[i] = dimProps[i].paramBound.annulusAdjust(point[i]);
      if(dimProps[i].isDiscrete()) {
        point[i] = dimProps[i].getGrainedValue(point[i]);
      }
    }
  }

  public double[] getRealLoc(double[] imageLoc) {
    double[] realLoc = new double[imageLoc.length];
    System.arraycopy(imageLoc, 0, realLoc, 0, imageLoc.length);
    annulusAdjust(realLoc);
    return realLoc;
  }
}

