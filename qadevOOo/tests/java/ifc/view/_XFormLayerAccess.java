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
 * Test is <b> NOT </b> multithread compliant. <p>
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
    @Override
    public void before() {
        xForm = (XForm) tEnv.getObjRelation("XFormLayerAccess.XForm");
        if (xForm == null) {
            throw new StatusException(Status.failed("Object raltion 'XFormLayerAccess.XForm' is null"));
        }
    }

    /**
     * Test disables the FormDesignMode and calls the method. <p>
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

        log.println("set back DesignMode to previous state");
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

