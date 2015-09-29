package net.adaptivebox.sco;

/**
 * Description: The description of social cognitive agent.
 *
 * @Information source: a) external library (L); b) the own memory: a point that
 * generated in the last learning cycle
 *
 * @Coefficients: TaoB and TaoW
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Mar 11, 2003
 * Xiaofeng Xie    May 11, 2004
 * Xiaofeng Xie    May 20, 2004
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
 *
 * @References:
 * [1] Xie X F, Zhang W J. Solving engineering design problems by social cognitive
 * optimization. Genetic and Evolutionary Computation Conference, 2004: 261-262
 */


import net.adaptivebox.global.*;
import net.adaptivebox.space.*;
import net.adaptivebox.goodness.*;
import net.adaptivebox.problem.*;
import net.adaptivebox.knowledge.*;

public class SCAgent {

  //Describes the problem to be solved (encode the point into intermediate information)
  private ProblemEncoder problemEncoder;
  //Forms the goodness landscape
  private IGoodnessCompareEngine specComparator;

  //the coefficients of SCAgent
  private static final int TaoB = 2;
  //The early version set TaoW as the size of external library (NL), but 4 is often enough
  private static final int TaoW = 4;

  //The referred external library
  private Library externalLib;
  //store the point that generated in current learning cycle
  private SearchPoint trailPoint;
  //the own memory: store the point that generated in last learning cycle
  private SearchPoint pcurrent_t;

  public void setExternalLib(Library lib) {
    externalLib = lib;
  }

  public void setProblemEncoder(ProblemEncoder encoder) {
    problemEncoder = encoder;
    trailPoint = problemEncoder.getFreshSearchPoint();
    pcurrent_t = problemEncoder.getEncodedSearchPoint();
  }

  public void setSpecComparator(IGoodnessCompareEngine comparer) {
    specComparator = comparer;
  }

  public SearchPoint generatePoint() {
    //generate a new point
    generatePoint(trailPoint);
    //evaluete the generated point
    problemEncoder.evaluate(trailPoint);
    return trailPoint;
  }

  private void generatePoint(ILocationEngine tempPoint) {
    SearchPoint Xmodel, Xrefer, libBPoint;

    // choose Selects a better point (libBPoint) from externalLib (L) based
    // on tournament selection
    int xb = externalLib.tournamentSelection(specComparator, TaoB, true);
    libBPoint = externalLib.getSelectedPoint(xb);
    // Compares pcurrent_t with libBPoint
    // The better one becomes model point (Xmodel)
    // The worse one becomes refer point (Xrefer)
    if(specComparator.compare(pcurrent_t.getEncodeInfo(), libBPoint.getEncodeInfo())==IGoodnessCompareEngine.LARGER_THAN) {
      Xmodel = libBPoint;
      Xrefer = pcurrent_t;
    } else {
      Xmodel = pcurrent_t;
      Xrefer = libBPoint;
    }
    // observational learning: generates a new point near the model point, which
    // the variation range is decided by the difference of Xmodel and Xrefer
    inferPoint(tempPoint, Xmodel, Xrefer, problemEncoder.getDesignSpace());
  }

  //1. Update the current point into the external library
  //2. Replace the current point by the generated point
  public void updateInfo() {
    //Selects a bad point kw from TaoW points in Library
    int xw = externalLib.tournamentSelection(specComparator, TaoW, false);
    //Repaces kw with pcurrent_t
    externalLib.getSelectedPoint(xw).importPoint(pcurrent_t);
    //Repaces pcurrent_t (x(t)) with trailPoint (x(t+1))
    pcurrent_t.importPoint(trailPoint);
   }

  //  1---model point, 2---refer point
   private boolean inferPoint(ILocationEngine newPoint, ILocationEngine point1,ILocationEngine point2, DesignSpace space){
     double[] newLoc = newPoint.getLocation();
     double[] real1 = point1.getLocation();
     double[] real2 = point2.getLocation();

     for (int i=0; i<newLoc.length; i++) {
       newLoc[i] = real1[i]*2-real2[i];
       //boundary handling
       newLoc[i] = space.boundAdjustAt(newLoc[i], i);
       newLoc[i] = RandomGenerator.doubleRangeRandom(newLoc[i], real2[i]);
     }
     return true;
   }
}

