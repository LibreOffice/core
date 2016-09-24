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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;

/**
* Testing <code>com.sun.star.awt.XControlContainer</code>
* interface methods:
* <ul>
*   <li><code> setStatusText() </code></li>
*   <li><code> addControl() </code></li>
*   <li><code> removeControl() </code></li>
*   <li><code> getControl() </code></li>
*   <li><code> getControls() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CONTROL1'</code> (of type <code>XControl</code>):
*  used as a parameter to addControl(), getControl() and removeControl()</li>
*  <li> <code>'CONTROL2'</code> (of type <code>XControl</code>):
*  used as a parameter to addControl(), getControl() and removeControl()</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XControlContainer
*/
public class _XControlContainer extends MultiMethodTest {
    public XControlContainer oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setStatusText() {
        oObj.setStatusText("testing XControlContainer::setStatusText(String)");
        tRes.tested("setStatusText()",true);
    }

    /**
    * Test calls the method twice - two controls gotten from object relations
    * 'CONTROL1' and 'CONTROL2' added to container.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _addControl() {
        oObj.addControl("CONTROL1", (XControl)tEnv.getObjRelation("CONTROL1"));
        oObj.addControl("CONTROL2", (XControl)tEnv.getObjRelation("CONTROL2"));
        tRes.tested("addControl()",true);
    }

    /**
    * Test calls the method with object relation 'CONTROL1' as a
    * parameter. Then control gotten from container is checked, and if returned
    * value is null then another control 'CONTROL2' is removed from container,
    * otherwise returned value of method test is 'false'.<p>
    * Has <b> OK </b> status if control is removed successfully.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    *  <li> <code> getControl() </code> : gets control from container </li>
    *  <li> <code> getControls() </code> : gets controls from container</li>
    * </ul>
    */
    public void _removeControl() {
        boolean result = true;

        requiredMethod("addControl()");
        requiredMethod("getControl()");
        requiredMethod("getControls()");
        oObj.removeControl( (XControl) tEnv.getObjRelation("CONTROL1") );
        XControl ctrl = oObj.getControl("CONTROL1");
        if (ctrl != null) {
            result = false;
            log.println("'removeControl()' fails; Control still exists");
        } else {
            oObj.removeControl( (XControl) tEnv.getObjRelation("CONTROL2") );
        }
        tRes.tested("removeControl()", result);
    }

    /**
    * Test calls the method with 'CONTROL1' as a parameter, then we just
    * compare returned object and object relation 'CONTROL1'.<p>
    * Has <b> OK </b> status if value returned by the method is equal to
    * a corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    * </ul>
    */
    public void _getControl() {
        requiredMethod("addControl()");
        XControl xCtrlComp = oObj.getControl("CONTROL1");
        XControl xCl = (XControl) tEnv.getObjRelation("CONTROL1");
        tRes.tested("getControl()", xCtrlComp.equals(xCl));
    }

    /**
    * Test calls the method, then returned value is checked.<p>
    * Has <b> OK </b> status if returned array consists of at least two
    * elements.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    * </ul>
    */
    public void _getControls() {
        requiredMethod("addControl()");
        XControl[] xCtrls = oObj.getControls();
        tRes.tested("getControls()",xCtrls.length >= 2);
    }
}

