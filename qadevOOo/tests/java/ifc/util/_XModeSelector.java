/*************************************************************************
 *
 *  $RCSfile: _XModeSelector.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:30:24 $
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
    * Has <b> OK </b> status if no runtime exceptions occured.
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
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is equal to value that was set by method setMode.
    */
    public void _getMode() {
        requiredMethod("setMode()");
        String curMode = oObj.getMode();
        tRes.tested("getMode()", curMode.equals(supportedModes[0]));
    }

    /**
    * Calls the method and checks value returned by method.<p>
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured,
    * returned value is true in first call and is false in second call.
    */
    public void _supportsMode() {
        requiredMethod("getSupportedModes()");
        boolean result = oObj.supportsMode(supportedModes[0]) &&
            ! oObj.supportsMode(supportedModes[0] + "_ForTest");
        tRes.tested("supportsMode()", result);
    }
}// finish class _XModeSelector

