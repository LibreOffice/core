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

package ifc.ui.dialogs;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.ValueChanger;
import util.ValueComparer;

import com.sun.star.ui.dialogs.XFilePickerControlAccess;

/**
* Testing <code>com.sun.star.ui.XFilePickerControlAccess</code>
* interface methods :
* <ul>
*  <li><code> setValue()</code></li>
*  <li><code> getValue()</code></li>
*  <li><code> setLabel()</code></li>
*  <li><code> getLabel()</code></li>
*  <li><code> getFocused()</code></li>
*  <li><code> enableControl()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XFilePickerControlAccess.ControlID'</code> (of type
*    <code>Short</code>) : control identifier in the extended
*    FilePicker dialog. </li>
*  <li> <code>'XFilePickerControlAccess.ControlValue'</code> (of type
*    <code>Object</code>) <b>optional</b> (but mostly desirable
*    since the control has emtpy initial value):
*    the value which can set for the control . </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ui.XFilePickerControlAccess
*/
public class _XFilePickerControlAccess extends MultiMethodTest {

    public XFilePickerControlAccess oObj = null;
    private short cntlID = -1 ;
    private Object oldVal = null ;
    private String oldLab = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    public void before() {
        Short ID = (Short) tEnv.getObjRelation
            ("XFilePickerControlAccess.ControlID") ;
        if (ID == null) {
            log.println("!!! Relation not found !!!") ;
            throw new StatusException(Status.failed("Relation not found")) ;
        }

        cntlID = ID.shortValue() ;
    }

    /**
    * Gets the value of the control and stores it. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getValue() {
        boolean result = true ;

        try {
            oldVal = oObj.getValue(cntlID,(short)0);
        } catch (Exception e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("getValue()", result) ;
    }

    /**
    * Changes the value gotten by <code>getValue</code> method,
    * or gets the value from relation if it exits.
    * Sets this value and then check if it was properly set. <p>
    * Has <b>OK</b> status if <code>getValue</code> method returns
    * the same value which was set. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getValue </code> </li>
    * </ul>
    */
    public void _setValue() {
        requiredMethod("getValue()") ;
        boolean result = true ;

        Object newVal = tEnv.getObjRelation
            ("XFilePickerControlAccess.ControlValue");
        if (newVal == null) {
            newVal = ValueChanger.changePValue(oldVal) ;
        }

        Object getVal = null ;

        try {
            oObj.setValue(cntlID, (short)0 , newVal) ;
            getVal = oObj.getValue(cntlID,(short)0) ;
        } catch (Exception e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        result &= ValueComparer.equalValue(newVal, getVal) ;

        tRes.tested("setValue()", result) ;
    }

    /**
    * Gets the label of the control and stores it. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getLabel() {

        try {
            oldLab = oObj.getLabel(cntlID);
        } catch (Exception e) {
            e.printStackTrace(log) ;
        }

        tRes.tested("getLabel()", oldLab != null) ;
    }

    /**
    * Changes the label gotten by <code>getLabel</code> method,
    * set this label and then check if it was properly set. <p>
    * Has <b>OK</b> status if <code>getLabel</code> method returns
    * the same value which was set. <p>
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> getLabel </code> </li>
    * </ul>
    */
    public void _setLabel() {
        requiredMethod("getLabel()") ;
        boolean result = true ;

        String newVal = oldLab + "_" ;
        String getVal = null ;

        try {
            oObj.setLabel(cntlID, newVal) ;
            getVal = oObj.getLabel(cntlID) ;
        } catch (Exception e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        result &= newVal.equals(getVal) ;

        tRes.tested("setLabel()", result) ;
    }

    /**
    * Disables and then enables the control. Can be checked only
    * interactively. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _enableControl() {
        boolean result = true ;

        try {
            oObj.enableControl(cntlID, false) ;
            oObj.enableControl(cntlID, true) ;
        } catch (Exception e) {
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("enableControl()", result) ;
    }
}


