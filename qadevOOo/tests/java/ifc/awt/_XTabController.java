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
import lib.Status;

import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XTabController;
import com.sun.star.awt.XTabControllerModel;

/**
* This interface is DEPRECATED !!!
* All test results are SKIPPED.OK now.
* Testing <code>com.sun.star.awt.XTabController</code>
* interface methods :
* <ul>
*  <li><code> activateFirst()</code></li>
*  <li><code> activateLast()</code></li>
*  <li><code> activateTabOrder()</code></li>
*  <li><code> autoTabOrder()</code></li>
*  <li><code> getContainer()</code></li>
*  <li><code> getCotrols()</code></li>
*  <li><code> getModel()</code></li>
*  <li><code> setContainer()</code></li>
*  <li><code> setModel()</code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'MODEL'</code> : <code>XTabControllerModel</code> a model for
*   the object</li>
*  <li> <code>'CONTAINER'</code> : <code>XControlContainer</code> a container
*   for the object</li>
* <ul> <p>
* @see com.sun.star.awt.XTabController
*/
public class _XTabController extends MultiMethodTest {
    public XTabController oObj = null;

    /**
    * Test calls the method with object relation 'MODEL' as a parameter.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setModel() {
        oObj.setModel( (XTabControllerModel) tEnv.getObjRelation("MODEL"));
        tRes.tested("setModel()", Status.skipped(true) );
    }

    /**
    * Test calls the method, then checks returned value.<p>
    * Has <b> OK </b> status if method returns a value that equals to
    * corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setModel() </code> : sets model for the object </li>
    * </ul>
    */
    public void _getModel() {
        requiredMethod("setModel()");
        oObj.getModel();
        tRes.tested("getModel()", Status.skipped(true));
    }

    /**
    * Test calls the method with object relation 'CONTAINER' as a parameter.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setContainer() {
        oObj.setContainer( (XControlContainer)
            tEnv.getObjRelation("CONTAINER"));
        tRes.tested("setContainer()", Status.skipped(true));
    }

    /**
    * Test calls the method, then checks returned value.<p>
    * Has <b> OK </b> status if method returns a value that equals to
    * corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setContainer() </code> : sets container for the object</li>
    * </ul>
    */
    public void _getContainer() {
        requiredMethod( "setContainer()");
        oObj.getContainer();
        tEnv.getObjRelation("CONTAINER");
        tRes.tested("getContainer()", Status.skipped(true) );
    }

    /**
    * Test calls the method, then checks returned sequence.<p>
    * Has <b> OK </b> status if returned sequence is not null.<p>
    */
    public void _getControls() {
        oObj.getControls();
        tRes.tested("getControls()", Status.skipped(true) );
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _autoTabOrder() {
        oObj.autoTabOrder();
        tRes.tested("autoTabOrder()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateTabOrder() {
        oObj.activateTabOrder();
        tRes.tested("activateTabOrder()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateFirst() {
        oObj.activateFirst();
        tRes.tested("activateFirst()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateLast() {
        oObj.activateLast();
        tRes.tested("activateLast()", Status.skipped(true));
    }
}

