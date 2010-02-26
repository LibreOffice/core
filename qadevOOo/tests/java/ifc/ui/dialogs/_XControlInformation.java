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
     * Also wrong control name (non-existant and empty) are checked.<p>
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

        boolean result = true;

        supProperties = new String[supControls.length][];
        for (int i = 0; i < supControls.length; i++) {
            log.println("Getting proeprties for control: " + supControls[i]);
            try {
                supProperties[i] =
                    oObj.getSupportedControlProperties(supControls[i]);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Unexpected exception:" + e);
                result = false ;
            }
            result &= supProperties[i] != null;
        }

        try {
            Object prop = oObj.getSupportedControlProperties("NoSuchControl") ;
            result &= prop == null;
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
            log.println("Checking proeprties for control " + supControls[i]);
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


