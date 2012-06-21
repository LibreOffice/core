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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.Locale;
import com.sun.star.lang.XServiceDisplayName;

/**
* Testing <code>com.sun.star.lang.XServiceDisplayName</code>
* interface methods:
* <ul>
*   <li><code>getServiceDisplayName()</code></li>
* </ul> <p>
* @see com.sun.star.lang.XServiceDisplayName
*/
public class _XServiceDisplayName extends MultiMethodTest {

    public XServiceDisplayName oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getServiceDisplayName() {
        String dn = oObj.getServiceDisplayName(new Locale("de","DE",""));

        tRes.tested("getServiceDisplayName()", dn != null);
    }

}  // finish class MTest


