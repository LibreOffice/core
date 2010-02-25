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

package ifc.awt;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.awt.XControl;
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
        XTabControllerModel setModel = oObj.getModel();
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
        XControlContainer setContainer = oObj.getContainer();
        XControlContainer relContainer = (XControlContainer )
                                            tEnv.getObjRelation("CONTAINER");
        tRes.tested("getContainer()", Status.skipped(true) );
    }

    /**
    * Test calls the method, then checks returned sequence.<p>
    * Has <b> OK </b> status if returned sequence is not null.<p>
    */
    public void _getControls() {
        XControl[] aControl = oObj.getControls();
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

