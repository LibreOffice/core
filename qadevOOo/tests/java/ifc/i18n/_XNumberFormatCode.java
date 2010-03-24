/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

