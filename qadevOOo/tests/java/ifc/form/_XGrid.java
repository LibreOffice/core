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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XGrid;

/**
* Testing <code>com.sun.star.form.XGrid</code>
* interface methods :
* <ul>
*  <li><code> getCurrentColumnPosition()</code></li>
*  <li><code> setCurrentColumnPosition(short nPos)</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XGrid
*/
public class _XGrid extends MultiMethodTest {
    public XGrid oObj = null;
    public short positionValue;

    /**
    * The method called. Then CurrentColumnPosition is obtained and
    * compared with previously changed value.
    * Has <b> OK </b> status if the method successfully returns
    * and the returned value is correct.
    */
    public void _getCurrentColumnPosition() {
        boolean result;

        requiredMethod("setCurrentColumnPosition()");
        log.println("Testing getCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() == positionValue) {
            result = true;
        } else {
            result = false;
        }
        tRes.tested("getCurrentColumnPosition()", result);
    }

    /**
    * The method called. Then CurrentColumnPosition is obtained, changed
    * and saved.
    * Has <b> OK </b> status if no exception has occurred.
    */
    public void _setCurrentColumnPosition() {
        log.println("Testing setCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() != 0) {
            positionValue = 0;
        } else {
            positionValue = 1;
        }
        oObj.setCurrentColumnPosition(positionValue);
        tRes.tested("setCurrentColumnPosition()", true);
    }

}
