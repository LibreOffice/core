/*
 * JavaTools.java
 *
 * Created on August 20, 2003, 1:23 PM
 */

package com.sun.star.wizards.common;

import com.sun.star.util.DateTime;
import com.sun.star.beans.PropertyValue;
import java.net.URLDecoder;
import java.util.*;
import java.io.File;

/**
 *
 * @author  bc93774
 */
public class JavaTools {

    /** Creates a new instance of JavaTools */
    public JavaTools() {
    }

    public static long getNullDateCorrection(long lDate) {
        java.util.Calendar oCal = java.util.Calendar.getInstance();
        oCal.set(1900, 1, 1);
        Date dTime = oCal.getTime();
        long lTime = dTime.getTime();
        long lDBNullDate = lTime / (3600 * 24000);
        long iDiffValue = lDBNullDate - lDate;
        return iDiffValue;
    }

    public static String[] copyStringArray(String[] FirstArray) {
        if (FirstArray != null) {
            String[] SecondArray = new String[FirstArray.length];
            for (int i = 0; i < FirstArray.length; i++) {
                SecondArray[i] = FirstArray[i];
            }
            return SecondArray;
        } else
            return null;
    }


    public static Object[] initializeArray(Object[] olist, Object ovalue){
        for (int i = 0; i < olist.length; i++)
            olist[i] = ovalue;
        return olist;
    }


    public static Object[][] initializeMultiDimArray(Object[][] olist, Object[] ovalue){
        for (int i = 0; i < olist.length; i++)
            olist[i] = ovalue;
        return olist;
    }



    public static int[] initializeintArray(int FieldCount, int nValue) {
        int[] LocintArray = new int[FieldCount];
        for (int i = 0; i < LocintArray.length; i++)
            LocintArray[i] = nValue;
        return LocintArray;
    }


    public static String[] multiDimListToArray(String[][] multidimlist) {
        String[] retlist = new String[] {};
        retlist = new String[multidimlist.length];
        for (int i = 0; i < multidimlist.length; i++) {
            retlist[i] = multidimlist[i][0];
        }
        return retlist;
    }

    public static String getlongestArrayItem(String[] StringArray) {
        String sLongestItem = "";
        int FieldCount = StringArray.length;
        int iOldLength = 0;
        int iCurLength = 0;
        for (int i = 0; i < FieldCount; i++) {
            iCurLength = StringArray[i].length();
            if (iCurLength > iOldLength) {
                iOldLength = iCurLength;
                sLongestItem = StringArray[i];
            }
        }
        return sLongestItem;
    }

    public static String ArraytoString(String[] LocArray) {
        String ResultString = "";
        int iLen = LocArray.length;
        for (int i = 0; i < iLen; i++) {
            ResultString += LocArray[i];
            if (i < iLen - 1)
                ResultString += ";";
        }
        return ResultString;
    }

    /**
     * @author bc93774
     * @param SearchList
     * @param SearchString
     * @return the index of the field that contains the string 'SearchString' or '-1' if not it is
     * not contained within the array
     */
    public static int FieldInList(String[] SearchList, String SearchString) {
        int FieldLen = SearchList.length;
        int retvalue = -1;
        for (int i = 0; i < FieldLen; i++) {
            if (SearchList[i].compareTo(SearchString) == 0) {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int FieldInList(String[] SearchList, String SearchString, int StartIndex) {
        int FieldLen = SearchList.length;
        int retvalue = -1;
        if (StartIndex < FieldLen) {
            for (int i = StartIndex; i < FieldLen; i++) {
                if (SearchList[i].compareTo(SearchString) == 0) {
                    retvalue = i;
                    break;
                }
            }
        }
        return retvalue;
    }

    public static int FieldInTable(String[][] SearchList, String SearchString) {
        int retvalue;
        if (SearchList.length > 0) {
            int FieldLen = SearchList.length;
            retvalue = -1;
            for (int i = 0; i < FieldLen; i++) {
                if (SearchList[i][0] != null) {
                    if (SearchList[i][0].compareTo(SearchString) == 0) {
                        retvalue = i;
                        break;
                    }
                }
            }
        } else
            retvalue = -1;
        return retvalue;
    }

    public static int FieldInIntTable(int[][] SearchList, int SearchValue) {
        int FieldLen = SearchList.length;
        int retvalue = -1;
        for (int i = 0; i < FieldLen; i++) {
            if (SearchList[i][0] == SearchValue) {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int getArraylength(Object[] MyArray) {
        int FieldCount = 0;
        if (MyArray != null)
            FieldCount = MyArray.length;
        return FieldCount;
    }

    /**
     * @author bc93774
     * This function bubble sorts an array of with 2 dimensions.
     * The default sorting order is the first dimension
     * Only if sort2ndValue is True the second dimension is the relevant for the sorting order
     */
    public static String[][] bubblesortList(String[][] SortList) {
        String DisplayDummy;
        int SortCount = SortList[0].length;
        int DimCount = SortList.length;
        for (int s = 0; s < SortCount; s++) {
            for (int t = 0; t < ((SortCount - s) - 1); t++) {
                if (SortList[0][t].compareTo(SortList[0][t + 1]) > 0) {
                    for (int k = 0; k < DimCount; k++) {
                        DisplayDummy = SortList[k][t];
                        SortList[k][t] = SortList[k][t + 1];
                        SortList[k][t + 1] = DisplayDummy;
                    }
                }
            }
        }
        return SortList;
    }

    /**
     * @param MainString
     * @param Token
     * @return
     */
    public static String[] ArrayoutofString(String MainString, String Token) {
        String[] StringArray;
        if (MainString.equals("") == false) {
            Vector StringVector = new Vector();
            String LocString = null;
            int iIndex;
            do {
                iIndex = MainString.indexOf(Token);
                if (iIndex < 0)
                    StringVector.addElement(MainString);
                else {
                    StringVector.addElement(MainString.substring(0, iIndex));
                    MainString = MainString.substring(iIndex + 1, MainString.length());
                }
            } while (iIndex >= 0);
            StringArray =  VectorToStringList(StringVector);
        } else
            StringArray = new String[0];
        return StringArray;
    }

    public static String replaceSubString(String MainString, String NewSubString, String OldSubString) {
        try {
            int NewIndex = 0;
            int OldIndex = 0;
            int NewSubLen = NewSubString.length();
            int OldSubLen = OldSubString.length();
            while (NewIndex != -1) {
                NewIndex = MainString.indexOf(OldSubString, OldIndex);
                if (NewIndex != -1) {
                    MainString = MainString.substring(0, NewIndex) + NewSubString + MainString.substring(NewIndex + OldSubLen);
                    OldIndex = NewIndex + NewSubLen;
                }
            }
            return MainString;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }


    public static String getFilenameOutOfPath(String sPath){
        String[] Hierarchy = ArrayoutofString(sPath, "/");
        return Hierarchy[Hierarchy.length - 1];
    }


    public static String getFileDescription(String sPath){
        String sFilename = getFilenameOutOfPath(sPath);
        String[] FilenameList = ArrayoutofString(sFilename, ".");
        String FileDescription = "";
        for (int i = 0; i < FilenameList.length - 1; i++) {
            FileDescription += FilenameList[i];
        }
        return FileDescription;
    }


    /**
     * @deprecated because the "\" sign is not properly convertered to the pathseparator
     * @param SystemPath
     * @return
     */
    public static String converttoURLNotation(String SystemPath) {
        try {
            java.io.File oFileObject = new java.io.File(SystemPath);
            char PathSeparator = java.io.File.separatorChar;
            SystemPath.replace('\\', '/');
            String URLPath = "file:/" + SystemPath;
            URLPath = JavaTools.replaceSubString(URLPath, "/", String.valueOf(PathSeparator));
            java.net.URL oURL = new java.net.URL(URLPath);
            URLPath = oURL.toExternalForm();
            URLPath = replaceSubString(URLPath, "file:///", "file:/");
            return URLPath;
        } catch (java.io.IOException jexception) {
            jexception.printStackTrace(System.out);
            return SystemPath;
        }
    }

    public static String convertfromURLNotation(String URLPath) {
        java.io.File oFileObject = new java.io.File(URLPath);
        char PathSeparator = File.separatorChar;
        java.net.URLDecoder oURL = new java.net.URLDecoder();
        try {
            URLPath = URLDecoder.decode(URLPath, "UTF-8");
        } catch (java.io.UnsupportedEncodingException jexception) {
            jexception.printStackTrace(System.out);
        }
        String SystemPath = replaceSubString(URLPath, "", "file:///");
        SystemPath = SystemPath.replace('/', PathSeparator);
        return SystemPath;
    }

    public static DateTime getDateTime(long timeMillis) {
        java.util.Calendar cal = java.util.Calendar.getInstance();
        cal.setTimeInMillis(timeMillis);
        DateTime dt = new DateTime();
        dt.Year = (short) cal.get(Calendar.YEAR);
        dt.Day = (short) cal.get(Calendar.DAY_OF_MONTH);
        dt.Month = (short) (cal.get(Calendar.MONTH) + 1);
        dt.Hours = (short) cal.get(Calendar.HOUR);
        dt.Minutes = (short) cal.get(Calendar.MINUTE);
        dt.Seconds = (short) cal.get(Calendar.SECOND);
        dt.HundredthSeconds = (short) cal.get(Calendar.MILLISECOND);
        return dt;
    }

    public static long getMillis(DateTime time) {
        java.util.Calendar cal = java.util.Calendar.getInstance();
        cal.set(time.Year, time.Month, time.Day, time.Hours, time.Seconds);
        return cal.getTimeInMillis();
    }

    public static String[] removeOutdatedFields(String[] baselist, String[] _complist) {
        String[] retarray = new String[] {};
        if ((baselist != null) && (_complist != null)) {
            Vector retvector = new Vector();
            String[] orderedcomplist = new String[_complist.length];
            System.arraycopy(_complist, 0, orderedcomplist, 0, _complist.length);
            for (int i = 0; i < baselist.length; i++)
                if (Arrays.binarySearch(orderedcomplist, baselist[i]) != -1)
                    retvector.add(baselist[i]);
            //          else
            // here you could call the method of a defined interface to notify the calling method
            //      }
            retarray = new String[retvector.size()];
            retvector.toArray(retarray);
        }
        return (retarray);
    }

    public static String[][] removeOutdatedFields(String[][] baselist, String[] _complist) {
        String[][] retarray = new String[][] {};
        if ((baselist != null) && (_complist != null)) {
            if (baselist.length > 0) {
                Vector retvector = new Vector();
                for (int i = 0; i < baselist.length; i++) {
                    if (FieldInList(_complist, baselist[i][0]) != -1)
                        retvector.add(baselist[i]);
                    //          else
                    // here you could call the method of a defined interface to notify the calling method
                }
                retarray = new String[retvector.size()][2];
                retvector.toArray(retarray);
            }
        }
        return (retarray);
    }

    public static PropertyValue[][] removeOutdatedFields(PropertyValue[][] baselist, String[] _complist) {
        PropertyValue[][] retarray = new PropertyValue[][] {
        };
        if ((baselist != null) && (_complist != null)) {
            Vector firstdimvector = new Vector();
            int b = 0;
            for (int n = 0; n < baselist.length; n++) {
                Vector secdimvector = new Vector();
                PropertyValue[] internalArray;
                int a = 0;
                for (int m = 0; m < baselist[n].length; m++) {
                    if (FieldInList(_complist, baselist[n][m].Name) > -1) {
                        secdimvector.add(baselist[n][m]);
                        a++;
                    }
                }
                if (a > 0) {
                    internalArray = new PropertyValue[a];
                    secdimvector.toArray(internalArray);
                    firstdimvector.add(internalArray);
                    b++;
                }
            }
            retarray = new PropertyValue[b][];
            firstdimvector.toArray(retarray);
        }
        return (retarray);
    }

    public static int getDuplicateFieldIndex(String[][] ocomplist) {
        for (int n = 0; n < ocomplist.length; n++) {
            String[] ocurValue = ocomplist[n];
            for (int m = n; m < ocomplist.length; m++) {
                if (m != n) {
                    for (int a = 0; a < ocurValue.length; a++) {
                        if (!ocurValue[a].equals(ocomplist[m][a]))
                            break;
                        if (a == ocurValue.length - 1)
                            return m;
                    }
                }
            }
        }
        return -1;
    }

    public static boolean isEqual(PropertyValue firstPropValue, PropertyValue secPropValue) {
        if (!firstPropValue.Name.equals(secPropValue.Name))
            return false;
        //TODO replace 'equals' with AnyConverter.getType(firstpropValue).equals(secPropValue) to check content and Type

        if (!firstPropValue.Value.equals(secPropValue.Value))
            return false;
        return (firstPropValue.Handle == secPropValue.Handle);
    }


    public static int[] getDuplicateFieldIndex(PropertyValue[][] ocomplist) {
        for (int n = 0; n < ocomplist.length; n++) {
            PropertyValue[] ocurValue = ocomplist[n];
            for (int m = n; m < ocurValue.length; m++) {
                PropertyValue odetValue = ocurValue[m];
                for (int s = 0; s < ocurValue.length; s++) {
                    if (s != m) {
                        if (isEqual(odetValue, ocurValue[s]))
                            return new int[] { n, s };
                    }
                }
            }
        }
        return new int[] { -1, -1 };
    }


    public static String[] removeArrayFields(String[] _sbaselist, String[] _scomplist){
        Vector StringVector = new Vector();
        for (int i = 0; i < _sbaselist.length; i++){
            if (FieldInList(_scomplist, _sbaselist[i]) == -1){
                StringVector.addElement(_sbaselist[i]);
            }
        }
        return VectorToStringList(StringVector);
    }



    public static String[] VectorToStringList(Vector _aVector){
        int fieldcount = _aVector.size();
        if (fieldcount > 0){
            String[] StringList = new String[fieldcount];
            _aVector.copyInto(StringList);
            return StringList;
        }
        else
            return new String[]{};
    }


    public static String getSuffixNumber(String _sbasestring){
        int suffixcharcount = 0;
        for (int i = _sbasestring.length()-1 ;i >= 0 ; i--){
            char b = _sbasestring.charAt(i);
            if ((b >= '0') && (b <= '9'))
                suffixcharcount++;
            else
                break;
        }
        int istart = _sbasestring.length() - suffixcharcount;
        return _sbasestring.substring(istart,_sbasestring.length());
    }



}
