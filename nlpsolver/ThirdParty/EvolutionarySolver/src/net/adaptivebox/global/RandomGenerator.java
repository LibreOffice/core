/**
 * Description: For generating random numbers.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Feb 22, 2001
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

package net.adaptivebox.global;

public class RandomGenerator {

/**This function returns a random integer number between the lowLimit and upLimit.
 * @param lowLimit lower limits
 *        upLimit The upper limits (between which the random number is to be generated)
 * @return int return value
 * Example: for find [0,1,2]
*/
public static int intRangeRandom(int lowLimit,int upLimit){
//  int num = (int)Math.rint(doubleRangeRandom(lowLimit,upLimit));
  int num = (int)Math.floor(doubleRangeRandom(lowLimit,upLimit+1)-1E-10);
  return(num);
}

/**This function returns a random float number between the lowLimit and upLimit.
 * @param lowLimit lower limits
 *        upLimit The upper limits (between which the random number is to be generated)
 * @return double return value
*/
public static double doubleRangeRandom(double lowLimit,double upLimit){
  double num = lowLimit + Math.random()*(upLimit-lowLimit);
  return(num);
}

/**This function returns true or false with a random probability.
 * @return int return value
  */
  public static boolean booleanRandom(){
    boolean value = true;
    double temp=Math.random();
    if (temp<0.5) value=false;
    return value;
  }

  public static int[] randomSelection(boolean[] types, int times) {
    int validNum = 0;
    for(int i=0; i<types.length; i++) {
      if(!types[i]) {
        validNum++;
      }
    }
    int[] totalIndices = new int[validNum];
    validNum = 0;
    for(int i=0; i<types.length; i++) {
      if(!types[i]) {
        totalIndices[validNum] = i;
        validNum++;
        if(validNum==totalIndices.length) break;
      }
    }
    return randomSelection(totalIndices, times);
  }

//  public static int[] randomSelection(boolean[] types, int times) {
//    int realTimes = times;
//    if(realTimes>types.length) realTimes = types.length;
//    boolean[] internalTypes = (boolean[])types.clone();
//    int upper = types.length-1;
//    int[] indices = new int[realTimes];
//    if(realTimes==types.length) {
//      for(int i=0; i<indices.length; i++) {
//        indices[i] = i;
//      }
//      return indices;
//    }
//    int i = 0;
//    while(i<realTimes) {
//      indices[i] = intRangeRandom(0, upper);
//      if(!internalTypes[indices[i]]) {
//        internalTypes[indices[i]] = true;
//        i++;
//      }
//    }
//    return indices;
//  }

  public static int[] randomSelection(int low, int up, int times){
    int[] totalIndices = new int[up-low];
    for (int i=low; i<up; i++) {
      totalIndices[i] = i;
    }
    return randomSelection(totalIndices, times);
  }

  public static int getRealV(double randTypeV) {
    if(randTypeV<=0) return 0;
    int realV = (int)Math.ceil(randTypeV);
    if(Math.random()<(randTypeV-realV)) realV++;
    return realV;
  }

  public static int[] randomSelection(int[] totalIndices, int times) {
    if (times>=totalIndices.length) {
      return totalIndices;
    }
    int[] indices = randomSelection(totalIndices.length, times);
    for(int i=0; i<indices.length; i++) {
      indices[i] = totalIndices[indices[i]];
    }
    return indices;
  }

  public static int[] randomSelection(int maxNum, int times) {
    if(times<=0) return new int[0];
    int realTimes = Math.min(maxNum, times);
    boolean[] flags = new boolean[maxNum];
//    Arrays.fill(flags, false);
    boolean isBelowHalf = times<maxNum*0.5;
    int virtualTimes = realTimes;
    if(!isBelowHalf) {
      virtualTimes = maxNum-realTimes;
    }
    int i = 0;
    int upper = maxNum-1;
    int[] indices = new int[realTimes];

    while(i<virtualTimes) {
      indices[i] = intRangeRandom(0, upper);
      if(!flags[indices[i]]) {
        flags[indices[i]] = true;
        i++;
      }
    }
    if(!isBelowHalf) {
      int j=0;
      for(i=0; i<maxNum; i++) {
        if(flags[i]==isBelowHalf) {
          indices[j] = i;
          j++;
          if(j==realTimes) break;
        }
      }
    }
    return indices;
  }
}