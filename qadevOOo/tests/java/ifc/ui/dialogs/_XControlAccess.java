/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XControlAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:36:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XFolderPicker
*/
public class _XControlAccess extends MultiMethodTest {

    public XControlAccess oObj = null;
    private XControlInformation xCI = null ;
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
    protected void before() {
        xCI = (XControlInformation) UnoRuntime.queryInterface
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


