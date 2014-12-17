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

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getAvailableLocales() {
        Locale[] locales = oObj.getAvailableLocales("com.sun.star.linguistic2.Hyphenator");
        tRes.tested("getAvailableLocales()", locales.length > 0);
    }

}  // finish class XAvailableLocales


