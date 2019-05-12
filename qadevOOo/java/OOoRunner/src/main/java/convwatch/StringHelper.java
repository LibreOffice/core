/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
