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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XWindow;
import com.sun.star.form.XFormController;
import com.sun.star.form.XFormControllerListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.form.XFormController</code>
* interface methods :
* <ul>
*  <li><code> getCurrentControl()</code></li>
*  <li><code> addActivateListener()</code></li>
*  <li><code> removeActivateListener()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'otherWindow'</code>
*  (of type <code>com.sun.star.awt.XWindow</code>):
*   The another window is used to activate it, causing deactivating
*   of the component tested. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XFormController
*/
public class _XFormController extends MultiMethodTest {

    public static XFormController oObj = null;
    XWindow otherWind = null;

    /**
     * Listener which determines and stores events occurred.
     */
    protected static class MyListener implements XFormControllerListener {
        public boolean activated = false ;
        public boolean deactivated = false ;
        public void disposing ( EventObject oEvent ) {}

        public void init() {
            activated = false;
            deactivated = false;
        }

        public void formActivated(EventObject ev) {
            activated = true ;
        }

        public void formDeactivated(EventObject ev) {
            deactivated = true ;
        }
    }

    MyListener listener = new MyListener() ;

    /**
    * Adds a listener, then switches focus between two windows.
    * The current controller must be deactivated and activated.<p>
    *
    * Has <b> OK </b> status if listener <code>deactivate</code>
    * and <code>activate</code> methods was called. <p>
    */
    public void _addActivateListener() {
        requiredMethod("getCurrentControl()");
        oObj.addActivateListener(listener) ;

        XWindow wind = UnoRuntime.queryInterface(XWindow.class, cntrl);
        wind.setFocus();
        waitForEventIdle();
        XWindow otherWind = (XWindow)tEnv.getObjRelation("otherWindow");
        otherWind.setFocus();
        waitForEventIdle();
        log.println("activated = " + listener.activated +
            ", deactivated = " + listener.deactivated) ;

        tRes.tested("addActivateListener()",
            listener.deactivated && listener.activated) ;
    }

    /**
    * Removes the listener added before, then switches focus between two windows.
    *
    * Has <b> OK </b> status if no listener methods were called. <p>
    */
    public void _removeActivateListener() {
        requiredMethod("addActivateListener()") ;

        oObj.removeActivateListener(listener);
        log.println("ActiveListener removed");
        listener.init();

        XWindow wind = UnoRuntime.queryInterface(XWindow.class, cntrl);
        wind.setFocus();
        waitForEventIdle();
        XWindow otherWind = (XWindow)tEnv.getObjRelation("otherWindow");
        otherWind.setFocus();
        waitForEventIdle();
        log.println("activated = " + listener.activated +
            ", deactivated = " + listener.deactivated) ;

        tRes.tested("removeActivateListener()",
            !listener.activated && !listener.deactivated);
    }

    XControl cntrl;

    /**
     * Retrieves current control and searches for it among child controls.
     *
     * Has <b>OK</b> status if the current control was found among component
     * children.
     */
    public void _getCurrentControl() {
        cntrl = oObj.getCurrentControl();
        XControl[] children = oObj.getControls() ;

        boolean res = false;
        for(int i = 0; i < children.length; i++) {
            if (children[i].equals(cntrl)) {
                log.println("Current control is equal to the object control" +
                    " #" + i + ":");
                log.println(cntrl);
                res = true;
                break;
            }
        }

        tRes.tested("getCurrentControl()", res) ;
    }
}

