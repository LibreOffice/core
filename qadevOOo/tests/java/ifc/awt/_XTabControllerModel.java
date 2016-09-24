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

import com.sun.star.awt.XTabControllerModel;

/**
* Testing <code>com.sun.star.awt.XTabControllerModel</code>
* interface methods :
* <ul>
*  <li><code> getGroupControl()</code></li>
*  <li><code> setGroupControl()</code></li>
*  <li><code> setControlModels()</code></li>
*  <li><code> getControlModels()</code></li>
*  <li><code> setGroup()</code></li>
*  <li><code> getGroupCount()</code></li>
*  <li><code> getGroup()</code></li>
*  <li><code> getGroupByName()</code></li>
* </ul> <p>
*
* Since the interface is <b>DEPRECATED</b> all methods have
* status SKIPPED.OK <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'Model1'</code> : <code>XControlModel</code> relation
*    , any control model belonging to the object tested.</li>
*  <li> <code>'Model2'</code> : <code>XControlModel</code> relation
*    , any control model belonging to the object tested.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XTabControllerModel
*/
public class _XTabControllerModel extends MultiMethodTest {

    public XTabControllerModel oObj = null;


    /**
    * Sets group control to <code>true</code> then calls
    * <code>getGroupControl()</code> method and checks the value. <p>
    * Has <b> OK </b> status if the method returns <code>true</code>
    * <p>
    */
    public void _getGroupControl() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupControl()", Status.skipped(true));
    }

    /**
    * Sets group control to <code>false</code> then calls
    * <code>getGroupControl()</code> method and checks the value. <p>
    * Has <b> OK </b> status if the method returns <code>false</code>
    * <p>
    */
    public void _setGroupControl() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setGroupControl()", Status.skipped(true));
    }

    /**
    * Test calls the method and sets control models to a single
    * model from 'Model1' relation. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _setControlModels() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setControlModels()", Status.skipped(true));
    }

    /**
    * Calls method and checks if models were properly set in
    * <code>setControlModels</code> method test. <p>
    * Has <b>OK</b> status if the model sequence set is equal
    * to the sequence get. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setControlModels </code> : to set sequence of models.</li>
    * </ul>
    */
    public void _getControlModels() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getControlModels()", Status.skipped(true));
    }

    /**
    * Sets the group named 'XTabControlModel' to sequence with a single
    * element from 'Model2' relation. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _setGroup() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setGroup()", Status.skipped(true));
    }

    /**
    * Calls method and checks if the group was properly set in
    * <code>setGroup</code> method test. <p>
    * Has <b>OK</b> status if the sequence set is equal
    * to the sequence get. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGroup </code> : to set the sequence.</li>
    * </ul>
    */
    public void _getGroup() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroup()", Status.skipped(true));
    }

    /**
    * Retrieves group named 'XTabControllerModel' added in <code>
    * setGroup</code> method test, and checks it. <p>
    * Has <b> OK </b> status if sequence get is equal to sequence set.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGroup </code> : to set the sequence.</li>
    * </ul>
    */
    public void _getGroupByName() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupByName()", Status.skipped(true));
    }

    /**
    * Gets number of groups.
    * Has <b> OK </b> status if the number is greater than 0.
    * <ul>
    *  <li> <code> setGroup </code> : to has at least one group.</li>
    * </ul>
    */
    public void _getGroupCount() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupCount()", Status.skipped(true));
    }

}

