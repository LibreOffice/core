/**
 * Description: Output methods for Array
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


public class OutputMethods {

  public OutputMethods() {
  }

  public static String outputVectorAsStr(double[] vector){
    if(vector==null) return "NULL";
    String totalStr = "";
    for(int i=0;i<vector.length;i++){
      totalStr += vector[i];
      if(i!=vector.length-1) {
        totalStr += "\t";
      }
    }
    totalStr+="\r\n";
    return totalStr;
  }

  public static void outputVector(double[] vector){
    for(int i=0;i<vector.length;i++){
      System.out.print(vector[i]+" \t");
    }
    System.out.println("");
  }
}