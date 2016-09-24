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
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XView;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.awt.XControl</code>
* interface methods:
* <ul>
*  <li><code> setContext() </code></li>
*  <li><code> getContext() </code></li>
*  <li><code> createPeer() </code></li>
*  <li><code> getPeer() </code></li>
*  <li><code> setModel() </code></li>
*  <li><code> getModel() </code></li>
*  <li><code> setDesignMode() </code></li>
*  <li><code> isDesignMode() </code></li>
*  <li><code> isTransparent() </code></li>
*  <li><code> getView() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CONTEXT'</code> (of type <code>XInterface</code>):
*  used as a parameter to setContext() and for testing getContext().</li>
*  <li> <code>'WINPEER'</code> (of type <code>XWindowPeer</code>):
*  used as a parameter to createPeer() and for testing getPeer()</li>
*  <li> <code>'TOOLKIT'</code> (of type <code>XToolkit</code>):
*  used as a parameter to createPeer()</li>
*  <li> <code>'MODEL'</code> (of type <code>XControlModel</code>):
*  used as a parameter to setModel() and for testing getModel()</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XControl
*/
public class _XControl extends MultiMethodTest {
    public XControl oObj = null;
    public XControlModel aModel = null;
    public boolean desMode;

    /**
    * After test calls the method, the Context is set to a corresponding
    * object relation.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setContext() {
        XInterface cont = (XInterface) tEnv.getObjRelation("CONTEXT");
        oObj.setContext(cont);
        tRes.tested("setContext()",true);
    }

    /**
    * After test calls the method, the Context is gotten and compared
    * with object relation 'CONTEXT'.<p>
    * Has <b> OK </b> status if get value is equals to value set before.<p>
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> setContext() </code> : set Context to a corresponding
    * object relation</li>
    * </ul>
    */
    public void _getContext() {
        requiredMethod("setContext()");
        XInterface cont = (XInterface) tEnv.getObjRelation("CONTEXT");
        Object get = oObj.getContext();
        boolean res = get.equals(cont);
        if (!res) {
            log.println("!!! Error: getting: "+get.toString());
            log.println("!!! expected: "+cont.toString());
        }
        tRes.tested("getContext()",res);
    }


    /**
    * The objects needed to create peer are obtained
    * from corresponding object relations, then the peer is created.
    * <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _createPeer() {
        XWindowPeer the_win = (XWindowPeer) tEnv.getObjRelation("WINPEER");
        XToolkit the_kit = (XToolkit) tEnv.getObjRelation("TOOLKIT");
        oObj.createPeer(the_kit,the_win);
        tRes.tested("createPeer()",true);
    }

    /*
    * Test calls the method. Then the object relation 'WINPEER' is
    * obtained, and compared with the peer, gotten from (XControl) oObj
    * variable.<p>
    * Has <b> OK </b> status if peer gotten isn't null
    * The following method tests are to be completed successfully before:
    */
    public void _getPeer() {
        requiredMethod("createPeer()");
        boolean res = false;
        XWindowPeer get = oObj.getPeer();
        if (get == null) {
            log.println("The method 'getPeer' returns NULL");
        } else {
           res = true;
        }
        tRes.tested("getPeer()",res);
    }


    /**
    * At first current model is obtained and saved to variable aModel.
    * Then object relation 'MODEL' is gotten and test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setModel() {
        aModel = oObj.getModel();
        XControlModel the_model = (XControlModel) tEnv.getObjRelation("MODEL");
        oObj.setModel(the_model);
        tRes.tested("setModel()",true);
    }

    /**
    * Test calls the method, then object relation 'MODEL' is gotten and
    * compared with object returned by the method. Then previously saved
    * value of model (aModel) restored to (XControl) oObj<p>
    * Has <b> OK </b> status if models set and get are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setModel() </code> : setting model from corresponding
    *  object relation </li>
    * </ul>
    */
    public void _getModel() {
        requiredMethod("setModel()");
        XControlModel the_model = (XControlModel) tEnv.getObjRelation("MODEL");
        XControlModel get = oObj.getModel();
        boolean res = get.equals(the_model);
        if (!res) {
            log.println("getting: "+get.toString());
            log.println("expected: "+the_model.toString());
        }
        if (aModel != null) {
        oObj.setModel(aModel);
    }
        tRes.tested("getModel()",res);
    }

    /**
    * Test calls the method. Then mode is checked using isDesignMode().<p>
    * Has <b> OK </b> status if mode is swithed.
    */
    public void _setDesignMode() {
    desMode = oObj.isDesignMode();
        oObj.setDesignMode(!desMode);
        tRes.tested("setDesignMode()",oObj.isDesignMode() == !desMode);
    }

    /**
    * The mode is changed and result is checked.<p>
    * Has <b> OK </b> status if the mode changed successfully.
    */
    public void _isDesignMode() {
    requiredMethod("setDesignMode()");
        oObj.setDesignMode(desMode);
        tRes.tested("isDesignMode()", oObj.isDesignMode() == desMode);
    }

    /**
    * Test calls the method.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _isTransparent() {
        oObj.isTransparent();
        tRes.tested("isTransparent()",true );
    }

    /**
    * Test calls the method.<p>
    * Has <b> OK </b> status if the method returns not null. <p>
    */
    public void _getView() {
        XView the_view = oObj.getView();
        tRes.tested("getView()", the_view != null);
    }

}


