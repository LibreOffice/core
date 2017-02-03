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

import com.sun.star.ui.dialogs.XControlInformation;

/**
* Testing <code>com.sun.star.ui.XControlInformation</code>
* interface methods :
* <ul>
*  <li><code> getSupportedControls()</code></li>
*  <li><code> isControlSupported()</code></li>
*  <li><code> getSupportedControlProperties()</code></li>
*  <li><code> isControlPropertySupported()</code></li>
* </ul> <p>
*
* @see com.sun.star.ui.XFolderPicker
*/
public class _XControlInformation extends MultiMethodTest {

    public XControlInformation oObj = null;
    private String[] supControls = null ;
    private String[][] supProperties = null ;

    /**
     * Gets supported controls and stores them. <p>
     * Has <b>OK</b> status if not <code>null</code> returned.
     */
    public void _getSupportedControls() {
        supControls = oObj.getSupportedControls();

        tRes.tested("getSupportedControls()", supControls != null) ;
    }

    /**
     * For every available control check if it is supported.
     * Also wrong control name (non-existent and empty) are checked.<p>
     *
     * Has <b>OK</b> status if <code>true</code> returned for valid
     * control names and <code>false</code> for invalid.<p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedControls </code> to have
     *      valid control names</li>
     * </ul>
     */
    public void _isControlSupported() {
        requiredMethod("getSupportedControls()") ;

        boolean result = true ;

        log.println("Supported controls :");
        for (int i = 0; i < supControls.length; i++) {
            log.println("  " + supControls[i]);
            result &= oObj.isControlSupported(supControls[i]) ;
        }

        result &= !oObj.isControlSupported("SuchNameMustNotExist");
        result &= !oObj.isControlSupported("");

        tRes.tested("isControlSupported()", result) ;
    }

    /**
     * For each control obtains its properties and stores them. Then tries to
     * obtain properties for control with invalid name. <p>
     *
     * Has <b>OK</b> status if properties arrays are not null and exception
     * thrown or null returned for control with invalid name <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedControls </code> to have
     *      valid control names</li>
     * </ul>
     */
    public void _getSupportedControlProperties() {
        requiredMethod("getSupportedControls()") ;

        supProperties = new String[supControls.length][];
        for (int i = 0; i < supControls.length; i++) {
            log.println("Getting properties for control: " + supControls[i]);
            try {
                supProperties[i] =
                    oObj.getSupportedControlProperties(supControls[i]);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Unexpected exception:" + e);
            }
        }

        try {
            oObj.getSupportedControlProperties("NoSuchControl");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception getting properties " +
                "for wrong control:" + e);
        }

        tRes.tested("getSupportedControlProperties()", true) ;
    }

    /**
     * <ul>
     *   <li>For each property of each control checks if it is supported.</li>
     *   <li>For each control checks if non-existent property
     *      (with wrong name and with empty name) supported.</li>
     *   <li>Tries to check the property of non-existent control </li>
     * </ul>
     * <p>
     * Has <b>OK</b> status if <code>true</code> returned for the first case,
     *   <code>false</code> for the second, and <code>false</code> or exception
     *   for the third.<p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedControlProperties </code> to have a set of
     *      valid properties </li>
     * </ul>
     */
    public void _isControlPropertySupported() {
        requiredMethod("getSupportedControlProperties()") ;

        boolean result = true;

        for (int i = 0; i < supControls.length; i++) {
            log.println("Checking properties for control " + supControls[i]);
            for (int j = 0; j < supProperties[i].length; j++) {
                log.println("   " + supProperties[i][j]);
                try {
                    result &= oObj.isControlPropertySupported
                        (supControls[i], supProperties[i][j]) ;
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    log.println("Unexpected exception:" + e);
                    result = false ;
                }
            }

            try {
                result &= !oObj.isControlPropertySupported
                    (supControls[i], "NoSuchPropertyForThisControl") ;
                result &= !oObj.isControlPropertySupported
                    (supControls[i], "") ;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println
                    ("Unexpected exception (just false must be returned):" + e);
                result = false ;
            }
        }

        try {
            result &= !oObj.isControlPropertySupported("NoSuchControl", "") ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception: " + e);
        }

        tRes.tested("isControlPropertySupported()", result) ;
    }
}


