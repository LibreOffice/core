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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XSupportedLocales;

/**
* Testing <code>com.sun.star.linguistic2.XSupportedLocales</code>
* interface methods:
* <ul>
*   <li><code>getLocales()</code></li>
*   <li><code>hasLocale()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XSupportedLocales
*/
public class _XSupportedLocales extends MultiMethodTest {

    public XSupportedLocales oObj = null;
    public static Locale[] locales = new Locale[0];

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getLocales() {
        locales = oObj.getLocales();
        tRes.tested("getLocales()", locales.length > 0);
    }

    /**
    * Test calls the method for every language from list of supported languages,
    * calls the method for one unsupported language and checks
    * all returned values. <p>
    * Has <b> OK </b> status if all returned values for supported languages
    * are equal to true and if returned value for unsupported language is equal
    * to false. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLocales() </code> : to have list of
    *  all supported languages </li>
    * </ul>
    */
    public void _hasLocale() {
        requiredMethod("getLocales()");
        boolean res = true;

        // first check all available locales
        for (int i=0; i< locales.length; i++) {
            res &= oObj.hasLocale(locales[i]);
        }

        // now create a invalid locale
        Locale invalid = new Locale("myLanguage","myCountry","");
        res &= !oObj.hasLocale(invalid);

        tRes.tested("hasLocale()", res);
    }

}  // finish class XSupportedLocales


