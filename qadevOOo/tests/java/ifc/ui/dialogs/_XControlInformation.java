/*************************************************************************
 *
 *  $RCSfile: _XControlInformation.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:26:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


