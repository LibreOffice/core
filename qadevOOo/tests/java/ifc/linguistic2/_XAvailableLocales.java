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

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XAvailableLocales;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.linguistic2.XAvailableLocales</code>
* interface methods:
* <ul>
*   <li><code>getAvailableLocales()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XAvailableLocales
*/
public class _XAvailableLocales extends MultiMethodTest {

    public XAvailableLocales oObj = null;
    public static Locale[] locales = new Locale[0];

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getAvailableLocales() {
        locales = oObj.getAvailableLocales("com.sun.star.linguistic2.Hyphenator");
        tRes.tested("getAvailableLocales()", locales.length > 0);
    }

}  // finish class XAvailableLocales


