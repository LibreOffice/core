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
    public Locale[] locales = new Locale[0];

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

        // now create an invalid locale
        Locale invalid = new Locale("myLanguage","myCountry","");
        res &= !oObj.hasLocale(invalid);

        tRes.tested("hasLocale()", res);
    }

}  // finish class XSupportedLocales


