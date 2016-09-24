
/**
 * Description: Contains a set of points.
 *
 * Author          Create/Modi     Note
 * Xiaofeng Xie    Mar 7, 2003
 * Xiaofeng Xie    May 3, 2003
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
 * @version 1.1
 * @Since MAOS1.0
 */
package net.adaptivebox.knowledge;

import net.adaptivebox.global.*;
import net.adaptivebox.goodness.*;
import net.adaptivebox.problem.*;

public class Library {
  private final SearchPoint[] libPoints;
  private int gIndex = -1;

  public Library(int number, ProblemEncoder problemEncoder){
    libPoints = new SearchPoint[number];
    for (int i=0; i<number; i++) {
      libPoints[i] = problemEncoder.getEncodedSearchPoint();
    }
  }

  public SearchPoint getGbest() {
    return getSelectedPoint(gIndex);
  }

  public void refreshGbest(IGoodnessCompareEngine qualityComparator) {
    gIndex = tournamentSelection(qualityComparator, getPopSize()-1, true);
  }

  public int getPopSize() {
    return libPoints.length;
  }

  public SearchPoint getSelectedPoint(int index) {
    return libPoints[index];
  }

  public static boolean replace(IGoodnessCompareEngine comparator, SearchPoint outPoint, SearchPoint tobeReplacedPoint) {
    boolean isBetter = false;
    if(comparator.compare(outPoint.getEncodeInfo(), tobeReplacedPoint.getEncodeInfo())<IGoodnessCompareEngine.LARGER_THAN) {
      tobeReplacedPoint.importPoint(outPoint);
      isBetter = true;
    }
    return isBetter;
  }

  public int tournamentSelection(IGoodnessCompareEngine comparator, int times, boolean isBetter) {
    int[] indices = RandomGenerator.randomSelection(getPopSize(), times);
    int currentIndex = indices[0];
    for (int i=1; i<indices.length; i++) {
      int compareValue = comparator.compare(libPoints[indices[i]].getEncodeInfo(), libPoints[currentIndex].getEncodeInfo());
      if (isBetter == (compareValue<IGoodnessCompareEngine.LARGER_THAN)) {
        currentIndex = indices[i];
      }
    }
    return currentIndex;
  }

  public double getExtremalVcon(boolean isMAX) {
    double val=BasicBound.MINDOUBLE;
    for(int i=0; i<libPoints.length; i++) {
      if(libPoints[i].getEncodeInfo()[0]>val==isMAX) {
        val = libPoints[i].getEncodeInfo()[0];
      }
    }
    return val;
  }

  public int getVconThanNum(double allowedCons) {
    int num=0;
    for(int i=0; i<libPoints.length; i++) {
      if(libPoints[i].getEncodeInfo()[0]<=allowedCons) {
        num++;
      }
    }
    return num;
  }

}



