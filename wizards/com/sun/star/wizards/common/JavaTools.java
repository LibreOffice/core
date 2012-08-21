/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.common;

import com.sun.star.util.DateTime;
import com.sun.star.beans.PropertyValue;
import java.util.*;
import java.io.File;

import com.sun.star.lib.util.UrlToFileMapper;
import java.net.MalformedURLException;
import java.net.URL;

/**
 *
 * @author  bc93774
 */
public class JavaTools
{

    /** Creates a new instance of JavaTools */
    public JavaTools()
    {
    }

/*
    public static void main(String args[])
    {
        String sPath = PropertyNames.EMPTY_STRING;
        DateTime oDateTime = null;
        long n;
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";   //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        try
        {
            XMultiServiceFactory xLocMSF = com.sun.star.wizards.common.Desktop.connect(ConnectStr);
            if (xLocMSF != null)
            {
                System.out.println("Connected to " + ConnectStr);
                oDateTime = getDateTime(9500000);
                sPath = convertfromURLNotation("file:///E:/trash/Web%20Wizard.xcu");
                n = getMillis(oDateTime);
                int a = 1;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }
*/
    public static String[] copyStringArray(String[] FirstArray)
    {
        if (FirstArray != null)
        {
            String[] SecondArray = new String[FirstArray.length];
            System.arraycopy(FirstArray, 0, SecondArray, 0, FirstArray.length);
            return SecondArray;
        }
        else
        {
            return null;
        }
    }

    public static Object[] initializeArray(Object[] olist, Object ovalue)
    {
        for (int i = 0; i < olist.length; i++)
        {
            olist[i] = ovalue;
        }
        return olist;
    }

    public static Object[][] initializeMultiDimArray(Object[][] olist, Object[] ovalue)
    {
        for (int i = 0; i < olist.length; i++)
        {
            olist[i] = ovalue;
        }
        return olist;
    }

    public static String[] ArrayOutOfMultiDimArray(String _sMultiDimArray[][], int _index)
    {
        String[] sRetArray = null;
        if (_sMultiDimArray != null)
        {
            sRetArray = new String[_sMultiDimArray.length];
            for (int i = 0; i < _sMultiDimArray.length; i++)
            {
                sRetArray[i] = _sMultiDimArray[i][_index];
            }
        }
        return sRetArray;
    }

    public static int[] initializeintArray(int FieldCount, int nValue)
    {
        int[] LocintArray = new int[FieldCount];
        for (int i = 0; i < LocintArray.length; i++)
        {
            LocintArray[i] = nValue;
        }
        return LocintArray;
    }

    /**converts a list of Integer values included in an Integer vector to a list of int values
     *
     * 
     * @param _aIntegerVector
     * @return
     */
    public static int[] IntegerTointList(Vector<Integer> _aIntegerVector)
    {
        try
        {
            Integer[] nIntegerValues = new Integer[_aIntegerVector.size()];
            int[] nintValues = new int[_aIntegerVector.size()];
            _aIntegerVector.toArray(nIntegerValues);
            for (int i = 0; i < nIntegerValues.length; i++)
            {
                nintValues[i] = nIntegerValues[i].intValue();
            }
            return nintValues;
        }
        catch (RuntimeException e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }

    /**converts a list of Boolean values included in a Boolean vector to a list of boolean values
     * 
     * 
     * @param _aBooleanVector
     * @return
     */
    public static boolean[] BooleanTobooleanList(Vector<Boolean> _aBooleanVector)
    {
        try
        {
            Boolean[] bBooleanValues = new Boolean[_aBooleanVector.size()];
            boolean[] bbooleanValues = new boolean[_aBooleanVector.size()];
            _aBooleanVector.toArray(bBooleanValues);
            for (int i = 0; i < bBooleanValues.length; i++)
            {
                bbooleanValues[i] = bBooleanValues[i].booleanValue();
            }
            return bbooleanValues;
        }
        catch (RuntimeException e)
        {
            e.printStackTrace(System.err);
            return null;
        }
    }

    public static String[] multiDimListToArray(String[][] multidimlist)
    {
        String[] retlist = new String[]
        {
        };
        retlist = new String[multidimlist.length];
        for (int i = 0; i < multidimlist.length; i++)
        {
            retlist[i] = multidimlist[i][0];
        }
        return retlist;
    }

    public static String getlongestArrayItem(String[] StringArray)
    {
        String sLongestItem = PropertyNames.EMPTY_STRING;
        int FieldCount = StringArray.length;
        int iOldLength = 0;
        int iCurLength = 0;
        for (int i = 0; i < FieldCount; i++)
        {
            iCurLength = StringArray[i].length();
            if (iCurLength > iOldLength)
            {
                iOldLength = iCurLength;
                sLongestItem = StringArray[i];
            }
        }
        return sLongestItem;
    }

    public static String ArraytoString(String[] LocArray)
    {
        StringBuilder ResultString = new StringBuilder(PropertyNames.EMPTY_STRING);
        int iLen = LocArray.length;
        for (int i = 0; i < iLen; i++)
        {
            ResultString.append(LocArray[i]);
            if (i < iLen - 1)
            {
                ResultString.append(PropertyNames.SEMI_COLON);
            }
        }
        return ResultString.toString();
    }

    /**
     * @author bc93774
     * @param SearchList
     * @param SearchString
     * @return the index of the field that contains the string 'SearchString' or '-1' if not it is
     * not contained within the array
     */
    public static int FieldInList(String[] SearchList, String SearchString)
    {
        int FieldLen = SearchList.length;
        int retvalue = -1;
        for (int i = 0; i < FieldLen; i++)
        {
            if (SearchList[i].compareTo(SearchString) == 0)
            {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int FieldInList(String[] SearchList, String SearchString, int StartIndex)
    {
        int FieldLen = SearchList.length;
        int retvalue = -1;
        if (StartIndex < FieldLen)
        {
            for (int i = StartIndex; i < FieldLen; i++)
            {
                if (SearchList[i].compareTo(SearchString) == 0)
                {
                    retvalue = i;
                    break;
                }
            }
        }
        return retvalue;
    }

    public static int FieldInTable(String[][] SearchList, String SearchString)
    {
        int retvalue;
        if (SearchList.length > 0)
        {
            int FieldLen = SearchList.length;
            retvalue = -1;
            for (int i = 0; i < FieldLen; i++)
            {
                if (SearchList[i][0] != null)
                {
                    if (SearchList[i][0].compareTo(SearchString) == 0)
                    {
                        retvalue = i;
                        break;
                    }
                }
            }
        }
        else
        {
            retvalue = -1;
        }
        return retvalue;
    }

    public static int FieldInIntTable(int[][] SearchList, int SearchValue)
    {
        int retvalue = -1;
        for (int i = 0; i < SearchList.length; i++)
        {
            if (SearchList[i][0] == SearchValue)
            {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int FieldInIntTable(int[] SearchList, int SearchValue, int _startindex)
    {
        int retvalue = -1;
        for (int i = _startindex; i < SearchList.length; i++)
        {
            if (SearchList[i] == SearchValue)
            {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int FieldInIntTable(int[] SearchList, int SearchValue)
    {
        return FieldInIntTable(SearchList, SearchValue, 0);
    }

    public static int getArraylength(Object[] MyArray)
    {
        int FieldCount = 0;
        if (MyArray != null)
        {
            FieldCount = MyArray.length;
        }
        return FieldCount;
    }

    /**
     * @author bc93774
     * This function bubble sorts an array of with 2 dimensions.
     * The default sorting order is the first dimension
     * Only if sort2ndValue is True the second dimension is the relevant for the sorting order
     */
    public static String[][] bubblesortList(String[][] SortList)
    {
        String DisplayDummy;
        int SortCount = SortList[0].length;
        int DimCount = SortList.length;
        for (int s = 0; s < SortCount; s++)
        {
            for (int t = 0; t < SortCount - s - 1; t++)
            {
                if (SortList[0][t].compareTo(SortList[0][t + 1]) > 0)
                {
                    for (int k = 0; k < DimCount; k++)
                    {
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
    public static String[] ArrayoutofString(String MainString, String Token)
    {
        String[] StringArray;
        if (!MainString.equals(PropertyNames.EMPTY_STRING))
        {
            ArrayList<String> StringVector = new ArrayList<String>();
            String LocString = null;
            int iIndex;
            do
            {
                iIndex = MainString.indexOf(Token);
                if (iIndex < 0)
                {
                    StringVector.add(MainString);
                }
                else
                {
                    StringVector.add(MainString.substring(0, iIndex));
                    MainString = MainString.substring(iIndex + 1, MainString.length());
                }
            }
            while (iIndex >= 0);
            int FieldCount = StringVector.size();
            StringArray = new String[FieldCount];
            StringVector.toArray(StringArray);
        }
        else
        {
            StringArray = new String[0];
        }
        return StringArray;
    }

    public static String replaceSubString(String MainString, String NewSubString, String OldSubString)
    {
        try
        {
            int NewIndex = 0;
            int OldIndex = 0;
            int NewSubLen = NewSubString.length();
            int OldSubLen = OldSubString.length();
            while (NewIndex != -1)
            {
                NewIndex = MainString.indexOf(OldSubString, OldIndex);
                if (NewIndex != -1)
                {
                    MainString = MainString.substring(0, NewIndex) + NewSubString + MainString.substring(NewIndex + OldSubLen);
                    OldIndex = NewIndex + NewSubLen;
                }
            }
            return MainString;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static String getFilenameOutOfPath(String sPath)
    {
        String[] Hierarchy = ArrayoutofString(sPath, "/");
        return Hierarchy[Hierarchy.length - 1];
    }

    public static String getFileDescription(String sPath)
    {
        String sFilename = getFilenameOutOfPath(sPath);
        String[] FilenameList = ArrayoutofString(sFilename, ".");
        StringBuilder FileDescription = new StringBuilder(PropertyNames.EMPTY_STRING);
        for (int i = 0; i < FilenameList.length - 1; i++)
        {
            FileDescription.append(FilenameList[i]);
        }
        return FileDescription.toString();
    }

    public static String convertfromURLNotation(String _sURLPath)
    {
        String sPath = PropertyNames.EMPTY_STRING;
        try
        {
            URL oJavaURL = new URL(_sURLPath);
            File oFile = UrlToFileMapper.mapUrlToFile(oJavaURL);
            sPath = oFile.getAbsolutePath();
        }
        catch (MalformedURLException e)
        {
            e.printStackTrace(System.err);
        }
        return sPath;
    }

    public static DateTime getDateTime(long timeMillis)
    {
        java.util.Calendar cal = java.util.Calendar.getInstance();
        setTimeInMillis(cal, timeMillis);
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

    public static long getTimeInMillis(Calendar _calendar)
    {
        java.util.Date dDate = _calendar.getTime();
        return dDate.getTime();
    }

    public static void setTimeInMillis(Calendar _calendar, long _timemillis)
    {
        java.util.Date dDate = new java.util.Date();
        dDate.setTime(_timemillis);
        _calendar.setTime(dDate);
    }

    public static long getMillis(DateTime time)
    {
        java.util.Calendar cal = java.util.Calendar.getInstance();
        cal.set(time.Year, time.Month, time.Day, time.Hours, time.Minutes, time.Seconds);
        return getTimeInMillis(cal);
    }

    public static String[] removeOutdatedFields(String[] baselist, String[] _complist)
    {
        String[] retarray = new String[]
        {
        };
        if ((baselist != null) && (_complist != null))
        {
            ArrayList<String> retvector = new ArrayList<String>();
//          String[] orderedcomplist = new String[_complist.length];
//          System.arraycopy(_complist, 0, orderedcomplist, 0, _complist.length);
            for (int i = 0; i < baselist.length; i++)
//              if (Arrays.binarySearch(orderedcomplist, baselist[i]) != -1)
            {
                if (FieldInList(_complist, baselist[i]) > -1)
                {
                    retvector.add(baselist[i]);
                //          else
                // here you could call the method of a defined interface to notify the calling method                                           
                //      }       
                }
            }
            retarray = new String[retvector.size()];
            retvector.toArray(retarray);
        }
        return (retarray);
    }

    public static String[][] removeOutdatedFields(String[][] baselist, String[] _complist, int _compindex)
    {
        String[][] retarray = new String[][] {};
        if ((baselist != null) && (_complist != null))
        {
            if (baselist.length > 0)
            {
                ArrayList<String[]> retvector = new ArrayList<String[]>();
                for (int i = 0; i < baselist.length; i++)
                {
                    String sValue = baselist[i][_compindex];
                    if (FieldInList(_complist, sValue) != -1)
                    {
                        retvector.add(baselist[i]);
                    //          else
                    // here you could call the method of a defined interface to notify the calling method                                           
                    }
                }
                retarray = new String[retvector.size()][2];
                retvector.toArray(retarray);
            }
        }
        return (retarray);
    }

    public static String[][] removeOutdatedFields(String[][] baselist, String[] _complist)
    {
        return removeOutdatedFields(baselist, _complist, 0);
    }

    public static PropertyValue[][] removeOutdatedFields(PropertyValue[][] baselist, String[] _complist)
    {
        if ((baselist != null) && (_complist != null))
        {
            ArrayList<PropertyValue[]> firstdimvector = new ArrayList<PropertyValue[]>();
            for (int n = 0; n < baselist.length; n++)
            {
                ArrayList<PropertyValue> secdimvector = new ArrayList<PropertyValue>();
                for (int m = 0; m < baselist[n].length; m++)
                {
                    if (FieldInList(_complist, baselist[n][m].Name) > -1)
                    {
                        secdimvector.add(baselist[n][m]);
                    }
                }
                if (!secdimvector.isEmpty())
                {
                    PropertyValue[] internalArray = new PropertyValue[secdimvector.size()];
                    secdimvector.toArray(internalArray);
                    firstdimvector.add(internalArray);
                }
            }
            PropertyValue[][] retarray = new PropertyValue[firstdimvector.size()][];
            return firstdimvector.toArray(retarray);
        }
        return new PropertyValue[][]
        {
        };
    }

    /**
     * searches a multidimensional array for duplicate fields. According to the following example
     * SlaveFieldName1 ;SlaveFieldName2; SlaveFieldName3
     * MasterFieldName1;MasterFieldName2;MasterFieldName3 
     * The entries SlaveFieldNameX and MasterFieldNameX are grouped together and then the created groups are compared
     * If a group is duplicate the entry of the second group is returned.
     * @param _scomplist
     * @return
     */
    public static int getDuplicateFieldIndex(String[][] _scomplist)
    {
        int retvalue = -1;
        if (_scomplist.length > 0)
        {
            int fieldcount = _scomplist[0].length;
            String[] sDescList = new String[fieldcount];
            for (int m = 0; m < fieldcount; m++)
            {
                for (int n = 0; n < _scomplist.length; n++)
                {
                    if (n == 0)
                    {
                        sDescList[m] = "";
                    }
                    sDescList[m] += _scomplist[n][m];
                }
            }
            return getDuplicateFieldIndex(sDescList);
        }
        return retvalue;
    }

    /**
     * not tested!!!!!
     * @param scomplist
     * @return
     */
    public static int getDuplicateFieldIndex(String[] scomplist)
    {
        for (int n = 0; n < scomplist.length; n++)
        {
            String scurvalue = scomplist[n];
            for (int m = n; m < scomplist.length; m++)
            {
                if (m != n)
                {
                    if (scurvalue.equals(scomplist[m]))
                    {
                        return m;
                    }
                }
            }
        }
        return -1;
    }

    public static int getDuplicateFieldIndex(String[] _scomplist, String _fieldname)
    {
        int iduplicate = 0;
        for (int n = 0; n < _scomplist.length; n++)
        {
            if (_scomplist[n].equals(_fieldname))
            {
                iduplicate++;
                if (iduplicate == 2)
                {
                    return n;
                }
            }
        }
        return -1;
    }

    public static boolean isEqual(PropertyValue firstPropValue, PropertyValue secPropValue)
    {
        if (!firstPropValue.Name.equals(secPropValue.Name))
        {
            return false;
        //TODO replace 'equals' with AnyConverter.getType(firstpropValue).equals(secPropValue) to check content and Type
        }
        if (!firstPropValue.Value.equals(secPropValue.Value))
        {
            return false;
        }
        return (firstPropValue.Handle == secPropValue.Handle);
    }

    public static int[] getDuplicateFieldIndex(PropertyValue[][] ocomplist)
    {
        for (int n = 0; n < ocomplist.length; n++)
        {
            PropertyValue[] ocurValue = ocomplist[n];
            for (int m = n; m < ocurValue.length; m++)
            {
                PropertyValue odetValue = ocurValue[m];
                for (int s = 0; s < ocurValue.length; s++)
                {
                    if (s != m)
                    {
                        if (isEqual(odetValue, ocurValue[s]))
                        {
                            return new int[]
                                    {
                                        n, s
                                    };
                        }
                    }
                }
            }
        }
        return new int[]
                {
                    -1, -1
                };
    }

    public static String getSuffixNumber(String _sbasestring)
    {
        int suffixcharcount = 0;
        for (int i = _sbasestring.length() - 1; i >= 0; i--)
        {
            char b = _sbasestring.charAt(i);
            if ((b >= '0') && (b <= '9'))
            {
                suffixcharcount++;
            }
            else
            {
                break;
            }
        }
        int istart = _sbasestring.length() - suffixcharcount;
        return _sbasestring.substring(istart, _sbasestring.length());
    }

    public static String[] removefromList(String[] _sbaselist, String[] _sdellist)
    {
        ArrayList<String> tempbaselist = new ArrayList<String>();
        for (int i = 0; i < _sbaselist.length; i++)
        {
            if (FieldInList(_sdellist, _sbaselist[i]) == -1)
            {
                tempbaselist.add(_sbaselist[i]);
            }
        }
        String[] sretlist = new String[tempbaselist.size()];
        tempbaselist.toArray(sretlist);
        return sretlist;
    }

    /**
     * compares two strings. If one of them is empty and the other one is null it also returns true
     * @param sFirstString
     * @param sSecondString
     * @return
     */
    public static boolean isSame(String sFirstString, String sSecondString)
    {
        boolean bissame = false;
        if (sFirstString == null)
        {
            if (sSecondString != null)
            {
                bissame = sSecondString.equals(PropertyNames.EMPTY_STRING);
            }
            else
            {
                bissame = true;
            }
        }
        else
        {
            if (sFirstString.equals(PropertyNames.EMPTY_STRING))
            {
                bissame = (sSecondString == null);
            }
            else if (sSecondString != null)
            {
                bissame = sFirstString.equals(sSecondString);
            }
        }
        return bissame;
    }
}
