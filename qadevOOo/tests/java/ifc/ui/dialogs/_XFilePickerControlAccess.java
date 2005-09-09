/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFilePickerControlAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:37:34 $
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
*    <code>Object</code>) <b>optional</b> (but mostly desireable
*    since the control has emtpy initial value):
*    the value which can set for the control . </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if <code>getLael</code> method returns
    * the same value which was set. <p>
    * The following method tests are to be completed successfully before :
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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


