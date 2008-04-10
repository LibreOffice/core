/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StringHelper.java,v $
 * $Revision: 1.5 $
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

package convwatch;

public class StringHelper {

    public static String doubleQuote(String _sStr)
        {
            return "\"" + _sStr + "\"";
        }

    public static String singleQuote(String _sStr)
        {
            return "'" + _sStr + "'";
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
