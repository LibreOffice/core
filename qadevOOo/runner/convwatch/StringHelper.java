/*************************************************************************
 *
 *  $RCSfile: StringHelper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-12-10 16:59:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
