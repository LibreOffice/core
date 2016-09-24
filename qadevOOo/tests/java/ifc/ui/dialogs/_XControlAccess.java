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

package ifc.ui.dialogs;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.ui.dialogs.XControlAccess;
import com.sun.star.ui.dialogs.XControlInformation;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.ui.XFilePicker</code>
* interface methods :
* <ul>
*  <li><code> setControlProperty()</code></li>
*  <li><code> getControlProperty()</code></li>
* </ul> <p>
*
* For testing this interface the component must implement
* <code>com.sun.star.ui.dialogs.XControlInformation</code>
* interface. <p>
*
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ui.XFolderPicker
*/
public class _XControlAccess extends MultiMethodTest {

    public XControlAccess oObj = null;
    private String[] supControls = null ;
    private String[][] supProperties = null ;

    /**
     * Tries to query <code>com.sun.star.ui.dialogs.XControlInformation</code>
     * interface, and obtain properties' names of each available
     * control. <p>
     *
     * @throw StatusException if interface is not supported or
     * properties couldn't be get.
     */
    @Override
    protected void before() {
        XControlInformation xCI = UnoRuntime.queryInterface
            (XControlInformation.class, oObj);

        if (xCI == null) throw new StatusException
            (Status.failed("XControlInformation not supported")) ;

        supControls = xCI.getSupportedControls();
        supProperties = new String[supControls.length][];
        for (int i = 0; i < supControls.length; i++) {
            try {
                supProperties[i] =
                    xCI.getSupportedControlProperties(supControls[i]);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
                throw new StatusException
                    ("Exception while init.", e) ;
            }
        }
    }

    /**
     * Tries to change each property of each control.
     * Has <b>OK</b> status if values are properly changed.
     */
    public void _setControlProperty() {
        boolean result = true ;
        String error = "";

        for (int i = 0; i < supControls.length; i++) {
            log.println("Checking properties for control " + supControls[i]);
            for (int j = 0; j < supProperties[i].length; j++) {
                log.println("\t" + supProperties[i][j]);
                try {
                    Object oldVal = oObj.getControlProperty(supControls[i],
                        supProperties[i][j]);
                    Object newVal = util.ValueChanger.changePValue(oldVal);
                    if (supProperties[i][j].startsWith("Help")) {
                        newVal = "HID:133";
                    }
                    oObj.setControlProperty
                        (supControls[i], supProperties[i][j], newVal) ;
                    Object resVal = oObj.getControlProperty(supControls[i],
                        supProperties[i][j]);
                    log.println("\t Old:" + oldVal + ",New:" + newVal
                        + ",Result:" + resVal);
                    if (!util.ValueComparer.equalValue(newVal, resVal)) {
                        error += "####Property '"+supProperties[i][j]+
                            " of "+supControls[i]+" didn't work\n\r"+
                            "\t Old:" + oldVal + ",New:" + newVal
                        + ",Result:" + resVal+ "\n\r";
                    }
                    result &= util.ValueComparer.equalValue(newVal, resVal);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Unexpected exception:" );
                    e.printStackTrace(log);
                    result = false ;
                }
            }
        }

        log.println(error);

        tRes.tested("setControlProperty()", result) ;
        tRes.tested("getControlProperty()", result) ;
    }

    /**
     * Does nothing. Testing performed in <code>setControlProperty</code>
     * method test.
     */
    public void _getControlProperty() {}
}


