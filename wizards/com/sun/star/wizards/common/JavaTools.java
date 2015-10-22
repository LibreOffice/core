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

import com.sun.star.beans.PropertyValue;
import java.util.*;
import java.io.File;

import com.sun.star.lib.util.UrlToFileMapper;
import java.net.MalformedURLException;
import java.net.URL;

public class JavaTools
{

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

    /**converts a list of Integer values included in an Integer vector to a list of int values
     */
    public static int[] IntegerTointList(java.util.List<Integer> _aIntegerVector)
    {
        int[] nintValues = null;
        if (_aIntegerVector.size() > 0) {
            int i = 0;
            nintValues = new int[_aIntegerVector.size()];
            for (Integer nIntegerValue : _aIntegerVector) {
                nintValues[i++] = nIntegerValue.intValue();
            }
        }
        return nintValues;
    }

    /**converts a list of Boolean values included in a Boolean vector to a list of boolean values
     */
    public static boolean[] BooleanTobooleanList(java.util.List<Boolean> _aBooleanVector)
    {
        boolean[] bbooleanValues = null;
        if (_aBooleanVector.size() > 0) {
            int i = 0;
            bbooleanValues = new boolean[_aBooleanVector.size()];
            for (Boolean bBooleanValue : _aBooleanVector) {
                bbooleanValues[i++] = bBooleanValue.booleanValue();
            }
        }
        return bbooleanValues;
    }

    public static String getlongestArrayItem(String[] StringArray)
    {
        String sLongestItem = PropertyNames.EMPTY_STRING;
        int iCurLength;
        int iOldLength = 0;
        for (String str : StringArray)
        {
            iCurLength = str.length();
            if (iCurLength > iOldLength)
            {
                iOldLength = iCurLength;
                sLongestItem = str;
            }
        }
        return sLongestItem;
    }

    public static String ArraytoString(String[] LocArray)
    {
        StringBuilder ResultString = new StringBuilder(PropertyNames.EMPTY_STRING);
        boolean bActive = false;
        for (String str : LocArray) {
            if (bActive) {
                 ResultString.append(PropertyNames.SEMI_COLON);
            } else {
                bActive = true;
            }
            ResultString.append(str);
        }
        return ResultString.toString();
    }

    /**
     * @return the index of the field that contains the string 'SearchString' or '-1' if not it is
     * not contained within the array
     */
    public static int FieldInList(String[] SearchList, String SearchString) {
        int retvalue = -1;
        for (int i = 0; i < SearchList.length; i++) {
            if (SearchList[i].equals(SearchString)) {
                retvalue = i;
                break;
            }
        }
        return retvalue;
    }

    public static int FieldInTable(String[][] SearchList, String SearchString)
    {
        int retvalue = -1;
        if (SearchList != null) {
            int FieldLen = SearchList.length;
            if (FieldLen > 0) {
                for (int i = 0; i < FieldLen; i++) {
                    if (SearchList[i][0] != null) {
                        if (SearchList[i][0].equals(SearchString)) {
                            retvalue = i;
                            break;
                        }
                    }
                }
            }
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

    public static String[] ArrayoutofString(String MainString, String Token)
    {
        String[] StringArray;
        if (!MainString.equals(PropertyNames.EMPTY_STRING))
        {
            ArrayList<String> StringVector = new ArrayList<String>();
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

    public static String[] removeOutdatedFields(String[] baselist, String[] _complist)
    {
        String[] retarray = new String[]
        {
        };
        if ((baselist != null) && (_complist != null))
        {
            ArrayList<String> retvector = new ArrayList<String>();
            for (int i = 0; i < baselist.length; i++)
            {
                if (FieldInList(_complist, baselist[i]) > -1)
                {
                    retvector.add(baselist[i]);
                // here you could call the method of a defined interface to notify the calling method
                }
            }
            retarray = new String[retvector.size()];
            retvector.toArray(retarray);
        }
        return retarray;
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
                    // here you could call the method of a defined interface to notify the calling method
                    }
                }
                retarray = new String[retvector.size()][2];
                retvector.toArray(retarray);
            }
        }
        return retarray;
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

    private static boolean isEqual(PropertyValue firstPropValue, PropertyValue secPropValue)
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

    public static String[] removefromList(String[] _sbaselist, String[] _sdellist) {
        ArrayList<String> tempbaselist = new ArrayList<String>();
        for (String _sbase : _sbaselist) {
            if (FieldInList(_sdellist, _sbase) == -1) {
                tempbaselist.add(_sbase);
            }
        }
        String[] sretlist = new String[tempbaselist.size()];
        tempbaselist.toArray(sretlist);
        return sretlist;
    }

    /**
     * compares two strings. If one of them is empty and the other one is null it also returns true
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
