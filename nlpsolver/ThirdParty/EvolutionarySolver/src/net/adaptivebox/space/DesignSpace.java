/**
 * Description: provide the information for the search space (S)
 *
 * Author          Create/Modi     Note
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
 * especially for particle swarm agent
 */

package net.adaptivebox.space;

public class DesignSpace {
  //The information of all the dimension
  private DesignDim[] dimProps;

  public DesignSpace(int dim) {
    dimProps = new DesignDim[dim];
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

  public void mutationAt(double[] location, int i){
    location[i] = dimProps[i].paramBound.getRandomValue();
  }







  public double getMagnitudeIn(int dimensionIndex) {
    return dimProps[dimensionIndex].paramBound.getLength();
  }




  public void initializeGene(double[] tempX){
    for(int i=0;i<tempX.length;i++) tempX[i] =  dimProps[i].paramBound.getRandomValue(); //Global.RandomGenerator.doubleRangeRandom(9.8, 10);
  }

  public void getMappingPoint(double[] point) {
    for(int i=0; i<getDimension(); i++) {
      point[i] = dimProps[i].paramBound.annulusAdjust(point[i]);
      if(dimProps[i].isDiscrete()) {
        point[i] = dimProps[i].getGrainedValue(point[i]);
      }
    }
  }

}

