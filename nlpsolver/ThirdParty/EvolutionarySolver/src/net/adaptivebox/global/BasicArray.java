/**
 * Description: basic operations on Arrays
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Oct. 9, 2002
 *
 */

package net.adaptivebox.global;

public class BasicArray {
  public static double getMinValue(double[] v) {
    double mv = Double.MAX_VALUE;
    for (int i=0; i<v.length; i++) {
      if (v[i]<mv) {
        mv=v[i];
      }
    }
    return mv;
  }
  public static double getMaxValue(double[] v) {
    double mv = -Double.MAX_VALUE;
    for (int i=0; i<v.length; i++) {
      if (v[i]>mv) {
        mv=v[i];
      }
    }
    return mv;
  }

}
