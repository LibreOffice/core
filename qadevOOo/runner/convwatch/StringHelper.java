/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StringHelper.java,v $
 * $Revision: 1.5.8.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// LLA: moved to helper.StringHelper package convwatch;
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper public class StringHelper {
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper     public static String doubleQuote(String _sStr)
// LLA: moved to helper.StringHelper         {
// LLA: moved to helper.StringHelper             return "\"" + _sStr + "\"";
// LLA: moved to helper.StringHelper         }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper     public static String singleQuote(String _sStr)
// LLA: moved to helper.StringHelper         {
// LLA: moved to helper.StringHelper             return "'" + _sStr + "'";
// LLA: moved to helper.StringHelper         }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper     public static String removeQuoteIfNeed(String _sPath)
// LLA: moved to helper.StringHelper         {
// LLA: moved to helper.StringHelper             String sNewPath = _sPath;
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper             if (_sPath.startsWith("\"") ||
// LLA: moved to helper.StringHelper                 _sPath.startsWith("'"))
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 // remove trailing quotes, if exists
// LLA: moved to helper.StringHelper                 sNewPath = sNewPath.substring(1);
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper             if (_sPath.endsWith("\"") ||
// LLA: moved to helper.StringHelper                 _sPath.endsWith("'"))
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 // remove trailing quotes, if exists
// LLA: moved to helper.StringHelper                 sNewPath = sNewPath.substring(0, sNewPath.length() - 1);
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper             return sNewPath;
// LLA: moved to helper.StringHelper         }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper     public static String doubleQuoteIfNeed(String _sStr)
// LLA: moved to helper.StringHelper         {
// LLA: moved to helper.StringHelper             if (_sStr.startsWith("\"") && _sStr.endsWith("\""))
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 // don't quote twice
// LLA: moved to helper.StringHelper                 return _sStr;
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper             if (_sStr.indexOf(" ") == -1)
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 // don't quote, if there is no space in name
// LLA: moved to helper.StringHelper                 return _sStr;
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper             if (_sStr.indexOf("%") != -1)
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 return singleQuote(_sStr);
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper             return doubleQuote(_sStr);
// LLA: moved to helper.StringHelper         }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper     /**
// LLA: moved to helper.StringHelper      * Convert a value to a string with a given length, if the len is greater the len of the value string representation
// LLA: moved to helper.StringHelper      * fill it's front with '0'
// LLA: moved to helper.StringHelper      * So ("5", 4) will result in a string "0005"
// LLA: moved to helper.StringHelper      * @param _nValue
// LLA: moved to helper.StringHelper      * @param _nLen
// LLA: moved to helper.StringHelper      * @return
// LLA: moved to helper.StringHelper      */
// LLA: moved to helper.StringHelper     public static String createValueString(int _nValue, int _nLen)
// LLA: moved to helper.StringHelper         {
// LLA: moved to helper.StringHelper             String sValue = String.valueOf(_nValue);
// LLA: moved to helper.StringHelper             StringBuffer a = new StringBuffer();
// LLA: moved to helper.StringHelper             while (_nLen > sValue.length())
// LLA: moved to helper.StringHelper             {
// LLA: moved to helper.StringHelper                 a.append('0');
// LLA: moved to helper.StringHelper                 _nLen --;
// LLA: moved to helper.StringHelper             }
// LLA: moved to helper.StringHelper             a.append(sValue);
// LLA: moved to helper.StringHelper             return a.toString();
// LLA: moved to helper.StringHelper         }
// LLA: moved to helper.StringHelper
// LLA: moved to helper.StringHelper }
