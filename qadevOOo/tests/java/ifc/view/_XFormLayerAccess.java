/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XFormLayerAccess.java,v $
 * $Revision: 1.3 $
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

package ifc.view;

import com.sun.star.form.XForm;
import com.sun.star.form.runtime.XFormController;
import lib.MultiMethodTest;

import com.sun.star.view.XFormLayerAccess;
import lib.Status;
import lib.StatusException;


/**
 * Testing <code>com.sun.star.view.XFormLayerAccess</code>
 * interface methods :
 * <ul>
 *  <li><code> getFromController()</code></li>
 *  <li><code> isFormDesignMode()</code></li>
 *  <li><code> setFormDesignMode()</code></li>
 * </ul> <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XFormLayerAccess
 */
public class _XFormLayerAccess extends MultiMethodTest {

    // oObj filled by MultiMethodTest

    public XFormLayerAccess oObj = null;

    private XForm xForm = null;

    /**
     * checks if the object relation <CODE>XFormLayerAccess.XForm</CODE>
     * is available
     */
    public void before() {
        xForm = (XForm) tEnv.getObjRelation("XFormLayerAccess.XForm");
        if (xForm == null) {
            throw new StatusException(Status.failed("Object raltion 'XFormLayerAccess.XForm' is null"));
        }
    }

    /**
     * Test disables the FormDesignMode and calls the mthod. <p>
     * Has <b> OK </b> status if the method returns
     * a not empty object of kind of com.sun.star.form.XFormController<P>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setFormDesignMode() </code></li>
     * </ul>
     * @see com.sun.star.view.XFormLayerAccess
     */
    public void _getFromController(){
        requiredMethod("setFormDesignMode()") ;

        log.println("try to get current DesignMode...");
        boolean currentMode = oObj.isFormDesignMode();
        log.println("DesignMode is " + currentMode);

        log.println("enable DesignMode");
        oObj.setFormDesignMode(false);

        log.println("test for getFromController() ");
        XFormController xFormCont = oObj.getFormController(xForm);

        if (xFormCont == null)
            log.println("ERROR: Could not get FromContoller");

        log.println("set back DesignMode to previouse state");
        oObj.setFormDesignMode(currentMode);

        tRes.tested("getFromController()", xFormCont != null );
    }

    /**
     * This test calls the test for <code>setFormDesignMode()</CODE>.
     * Has <b> OK </b> status if the test for  setFormDesignMode() returns
     * <code>true</code> since the tests use <CODE>isFormDesignMode()</CODE><P>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setFormDesignMode() </code></li>
     * </ul>
     */
    public void _isFormDesignMode(){
        requiredMethod("setFormDesignMode()") ;

        log.println("test for isFormDesignMode() is ok since test for 'setFormDesingMode()' use it");
        tRes.tested("isFormDesignMode()", true);
    }

    /**
     * This test gets the current FormDesignMode, change it to the opposite and checks if the expected value of
     * method isFormDesignmode() was given. Then the FormDesignmode was set back to the original value.<P>
     * Has <B> OK </B> if expected values are returned.
     *
     */

    public void _setFormDesignMode(){
        log.println("test for setFormDesignMode() and isFormDesignMode() ");

        log.println("try to get current DesignMode...");
        boolean currentMode = oObj.isFormDesignMode();
        log.println("DesignMode is " + currentMode);

        log.println("try to change to " + !currentMode + "...");
        oObj.setFormDesignMode(!currentMode);
        log.println("try to get new DesignMode...");
        boolean newMode = oObj.isFormDesignMode();
        log.println("DesignMode is " + newMode);

        boolean bOK = (newMode != currentMode);

        if ( !bOK)
            log.println("ERROR: both modes are equal");

        log.println("set back DesignMode to " + currentMode);
        oObj.setFormDesignMode(currentMode);

        log.println("try to get DesignMode...");
        boolean oldMode = oObj.isFormDesignMode();

        bOK &= (bOK &(currentMode == oldMode));

        if (currentMode != oldMode)
            log.println("ERROR: could not change back");

        tRes.tested("setFormDesignMode()", bOK );
    }

}  // finish class _XFormLayerAccess

