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
* Test is <b> NOT </b> multithread compilant. <p>
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

