/**
 * Description: operations for the a text string.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Feb 22, 2001
 * Xiaofeng Xie    May 12, 2004
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

import java.io.*;
import java.util.*;

public class GlobalString {
  public static final String NEGLECT_TAG = "#$@";
  public static final String EQUAL_TAG = "=";

/**
  * Tokenize a String with given key.
  * @param      input      the String to be tokenized.
  * @param      tokenKey   the delimiters.
  * @return  a String array that include the elements of input string that
  * divided by the tokenKey.
  */
  public static String[] tokenize(String input , String tokenKey) {
    ArrayList<String> v = new ArrayList<String>();
    StringTokenizer t = new StringTokenizer(input, tokenKey);
    while (t.hasMoreTokens())
      v.add(t.nextToken());
    return v.toArray(new String[v.size()]);
  }

  public static String[] getMeaningfulLines(String srcStr) throws Exception {
    return getMeaningfulLines(srcStr, NEGLECT_TAG);
  }

  public static String getMeaningfulLine(BufferedReader outReader) throws Exception {
    return getMeaningfulLine(outReader, NEGLECT_TAG);
  }

  public static int getCharLoc(char data, String str) {
    for(int i=0; i<str.length(); i++) {
      if(str.charAt(i)==data) return i;
    }
    return -1;
  }
  public static String trim(String origStr, String discardStr) {
    String str = origStr;
    do {
      if(str.length()==0) return str;
      if(getCharLoc(str.charAt(0), discardStr)!=-1) str = str.substring(1);
      else if(getCharLoc(str.charAt(str.length()-1), discardStr)!=-1) str = str.substring(0, str.length()-1);
      else {return str;}
    } while(true);
  }

  public static boolean getFirstCharExist(String str, String chars) throws Exception {
    int neglectFirstCharLength = chars.length();
    for(int i=0; i<neglectFirstCharLength; i++) {
      if(str.startsWith(chars.substring(i, i+1))) {
        return true;
      }
    }
    return false;
  }

  public static String getMeaningfulLine(BufferedReader outReader, String neglectFirstChars) throws Exception {
    String str;
    boolean isNeglect = true;
    int i = 0;
    do {
      str = outReader.readLine();
      if (str==null) {
        return null;
      }
      str = trim(str, " \t");
      if(str.length()>0) {
        isNeglect = getFirstCharExist(str, neglectFirstChars);
      }
    } while (isNeglect);
    return str;
  }

   public static String[] getMeaningfulLines(String srcStr, String neglectFirstChars) throws Exception {
    StringReader outStringReader = new StringReader(srcStr);
    BufferedReader outReader = new BufferedReader(outStringReader);
    ArrayList<String> origData = new ArrayList<String>();
    while(true) {
        String str = getMeaningfulLine(outReader, neglectFirstChars);
        if (str==null) {
            break;
        }
        origData.add(str);
    }
    return convert1DVectorToStringArray(origData);
  }

  /**
   * convert vector to 1D String array
   */
  public static String[] convert1DVectorToStringArray(ArrayList<String> toToConvert) {
    if (toToConvert==null) return null;
    String[] objs = new String[toToConvert.size()];
    for (int i=0; i<toToConvert.size(); i++) {
        objs[i] = getObjString(toToConvert.get(i));
    }
    return(objs);
  }

  public static String getObjString(Object nObj) {
    if(nObj instanceof String) return (String)nObj;
    return nObj.toString();
  }

  static public int toInteger(Object oVal) throws Exception {
    if(oVal==null) throw new Exception("Null string");
    return new Integer(oVal.toString()).intValue();
  }

  static public double toDouble(Object oVal) throws Exception {
    if(oVal==null) throw new Exception("Null string");
    return new Double(oVal.toString()).doubleValue();
  }

  public static Object toObject(String key) throws Exception{
    Class cls = Class.forName(key);
    return cls.newInstance();
  }
}