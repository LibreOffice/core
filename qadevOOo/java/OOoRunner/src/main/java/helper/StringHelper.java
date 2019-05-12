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



package helper;

public class StringHelper
{

    public static String doubleQuote(String _sStr)
        {
            return "\"" + _sStr + "\"";
        }

    public static String singleQuote(String _sStr)
        {
            return "'" + _sStr + "'";
        }

    /**
     * removes quotes if both exists at start and at end
     */
    public static String removeSurroundQuoteIfExists(String _sPath)
        {
            String sNewPath = _sPath;
            boolean bRemoveQuotes = false;
            if (
                (_sPath.startsWith("\"") && _sPath.endsWith("\"")) ||
                (_sPath.startsWith("'") && _sPath.endsWith("'"))
                )
            {
                // remove trailing quotes, if exists
                sNewPath = sNewPath.substring(1);

                // remove trailing quotes, if exists
                sNewPath = sNewPath.substring(0, sNewPath.length() - 1);
            }
            return sNewPath;
        }

    public static String removeQuoteIfExists(String _sPath)
        {
            String sNewPath = _sPath;

            if (_sPath.startsWith("\"") ||
                _sPath.startsWith("'"))
            {
                // remove trailing quotes, if exists
                sNewPath = sNewPath.substring(1);
            }

            if (_sPath.endsWith("\"") ||
                _sPath.endsWith("'"))
            {
                // remove trailing quotes, if exists
                sNewPath = sNewPath.substring(0, sNewPath.length() - 1);
            }
            return sNewPath;
        }

    public static String doubleQuoteIfNeed(String _sStr)
        {
            if (_sStr.startsWith("\"") && _sStr.endsWith("\""))
            {
                // don't quote twice
                return _sStr;
            }
            if (_sStr.indexOf(" ") == -1)
            {
                // don't quote, if there is no space in name
                return _sStr;
            }
            if (_sStr.indexOf("%") != -1)
            {
                return singleQuote(_sStr);
            }

            return doubleQuote(_sStr);
        }

    /**
     * Convert a value to a string with a given length, if the len is greater the len of the value string representation
     * fill it's front with '0'
     * So ("5", 4) will result in a string "0005"
     * @param _nValue
     * @param _nLen
     * @return
     */
    public static String createValueString(int _nValue, int _nLen)
        {
            String sValue = String.valueOf(_nValue);
            StringBuffer a = new StringBuffer();
            while (_nLen > sValue.length())
            {
                a.append('0');
                _nLen --;
            }
            a.append(sValue);
            return a.toString();
        }

}
