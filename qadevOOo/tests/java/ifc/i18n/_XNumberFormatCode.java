/*************************************************************************
 *
 *  $RCSfile: _XNumberFormatCode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:41:45 $
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

package ifc.i18n;

import lib.MultiMethodTest;

import com.sun.star.i18n.KNumberFormatType;
import com.sun.star.i18n.KNumberFormatUsage;
import com.sun.star.i18n.NumberFormatCode;
import com.sun.star.i18n.NumberFormatIndex;
import com.sun.star.i18n.XNumberFormatCode;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XNumberFormatCode</code>
* interface methods:
* <ul>
*  <li><code> getDefault() </code></li>
*  <li><code> getFormatCode() </code></li>
*  <li><code> getAllFormatCode() </code></li>
*  <li><code> getAllFormatCodes() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XNumberFormatCode
*/
public class _XNumberFormatCode extends MultiMethodTest {
    public XNumberFormatCode oObj = null;
    public String[] languages = new String[]
        {"de","en","es","fr","ko","ko","zh"};
    public String[] countries = new String[]
        {"DE","US","ES","FR","KR","KR","CN"};

    /**
    * Test calls the method twice with two different format types as
    * parameters for each locale. Result is checked after every call.<p>
    * Has <b> OK </b> status if both times returned structure's field 'Code'
    * does not equal to empty string.
    */
    public void _getDefault() {
        boolean res = true;
        NumberFormatCode nfc = null;

        for (int i=0;i<7;i++) {
            nfc = oObj.getDefault(KNumberFormatType.SHORT,
                KNumberFormatUsage.DATE, getLocale(i));
            String str = nfc.Code;
            if (str.equals("")) {
                log.println("'NumberFormat.code.equals(\"\") = true' for"
                    + " language: " + languages[i]);
                log.println("Usage: oObj.getDefault(KNumberFormatType.SHORT,"
                    + " KNumberFormatUsage.DATE,new Locale(" + languages[i]
                    + "," + countries[i] + ",\"\");");
            }
            res &= !str.equals("");

            nfc = oObj.getDefault(KNumberFormatType.LONG,
                KNumberFormatUsage.DATE,getLocale(i));
            str = nfc.Code;
            if (str.equals("")) {
                log.println("'NumberFormat.code.equals(\"\") = true' for "
                    + "language: " + languages[i]);
                log.println("Usage: oObj.getDefault(KNumberFormatType.LONG,"
                    + " KNumberFormatUsage.DATE,new Locale(" + languages[i]
                    + "," + countries[i] + ",\"\");");
            }
            res &= ( ! str.equals("") );
        }
        tRes.tested("getDefault()", res);
    }

    /**
    * Test calls the method twice for each locale with two different arguments.
    * After every call result is checked.<p>
    * Has <b> OK </b> status if both times returned structure's field 'Code'
    * does not equal to a empty string.
    */
    public void _getFormatCode() {
        boolean res = true;
        NumberFormatCode nfc = null;

        for (int i=0;i<7;i++) {
            nfc = oObj.getFormatCode
                (NumberFormatIndex.DATE_SYSTEM_SHORT,getLocale(i));
            res &= ( ! nfc.Code.equals("") );
            nfc = oObj.getFormatCode
                (NumberFormatIndex.DATE_SYSTEM_LONG,getLocale(i));
            res &= ( ! nfc.Code.equals("") );
        }
        tRes.tested("getFormatCode()", res);
    }

    /**
    * Test calls the method twice with two different arguments for each locale.
    * After every call result is checked.<p>
    * Has <b> OK </b> status if both times returned array's length does not
    * equal to zero.
    */
    public void _getAllFormatCode() {
        boolean res = true;
        NumberFormatCode[] nfc = null;

        for (int i=0;i<7;i++) {
            nfc = oObj.getAllFormatCode(KNumberFormatUsage.DATE, getLocale(i));
            res &= ( nfc.length != 0 );
            nfc = oObj.getAllFormatCode(KNumberFormatUsage.TIME, getLocale(i));
            res &= ( nfc.length != 0 );
        }
        tRes.tested("getAllFormatCode()", res);
    }

    /**
    * Test calls the method for each locale. <p>
    * Has <b> OK </b> status if returned array's length does not equal to zero.
    */
    public void _getAllFormatCodes() {
        boolean res = true;
        NumberFormatCode[] nfc = null;

        for (int i=0;i<7;i++) {
            nfc = oObj.getAllFormatCodes(getLocale(i));
            res &= ( nfc.length != 0 );
        }
        tRes.tested("getAllFormatCodes()", res);
    }

    /**
    * Method returns locale for a given language and country.
    * @param localeIndex index of needed locale.
    * @return Locale by the index from arrays defined above
    */
    public Locale getLocale(int k) {
        return new Locale(languages[k], countries[k], "");
    }



} // end XNumberFormatCode

