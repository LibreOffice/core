/**
 * Description: Global package for comparison.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Jun 15, 2002    xiaofengxie@tsinghua.org.cn
 *
 *
 * @version 1.0
 * @Since MAOS1.0
 */


package net.adaptivebox.global;

public class GlobalCompare {

/* compare the data1 and data2, if data1=data2, return 0
 * if data1 < data2, return LESS_THAN, else if data1 > data2, LARGER_THAN
 **/
  static public int compare(double data1, double data2) {
    if (data1 < data2)
      return CompareValue.LESS_THAN;
    else if (data1 > data2)
      return CompareValue.LARGER_THAN;
    else
      return CompareValue.EQUAL_TO;
  }


}
