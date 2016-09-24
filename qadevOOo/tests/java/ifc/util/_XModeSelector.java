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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XModeSelector;

/**
* Testing <code>com.sun.star.util.XModeSelector</code>
* interface methods :
* <ul>
*  <li><code>setMode()</code></li>
*  <li><code>getMode()</code></li>
*  <li><code>getSupportedModes()</code></li>
*  <li><code>supportsMode()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.util.XModeSelector
*/
public class _XModeSelector extends MultiMethodTest {
    public XModeSelector oObj = null;

    String[] supportedModes;
    /**
    * Calls the method and as argument pass one of the supported modes
    * that was returned by method getSupportedMode.<p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _setMode() {
        requiredMethod("getSupportedModes()");
        try {
            oObj.setMode(supportedModes[0]);
        } catch(com.sun.star.lang.NoSupportException e) {
            log.println("Method setMode() doesn't support mode '"
                 + supportedModes[0] + "'");
            tRes.tested("setMode()", false);
            return ;
        }
        tRes.tested("setMode()", true);
    }

    /**
    * Calls the method and check returned value.<p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is equal to value that was set by method setMode.
    */
    public void _getMode() {
        requiredMethod("setMode()");
        String curMode = oObj.getMode();
        tRes.tested("getMode()", curMode.equals(supportedModes[0]));
    }

    /**
    * Calls the method and checks value returned by method.<p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
    public void _getSupportedModes() {
        supportedModes = oObj.getSupportedModes();
        tRes.tested("getSupportedModes()", supportedModes != null);
    }

    /**
    * Calls the method. First  one of the supported modes that was returned
    * by method getSupportedMode is passed as argument.
    * Then the method is called again and the mode that is certainly not supported
    * is passed. Checks up returned values in both cases.<p>
    * Has <b> OK </b> status if no runtime exceptions occurred,
    * returned value is true in first call and is false in second call.
    */
    public void _supportsMode() {
        requiredMethod("getSupportedModes()");
        boolean result = oObj.supportsMode(supportedModes[0]) &&
            ! oObj.supportsMode(supportedModes[0] + "_ForTest");
        tRes.tested("supportsMode()", result);
    }
}// finish class _XModeSelector

