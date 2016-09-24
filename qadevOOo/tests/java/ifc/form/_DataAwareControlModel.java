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

import lib.MultiPropertyTest;
import util.utils;

import com.sun.star.form.XLoadable;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.form.DataAwareControlModel</code>
* service properties :
* <ul>
*  <li><code> DataField</code></li>
*  <li><code> BoundField</code></li>
*  <li><code> LabelControl</code></li>
* </ul> <p>
* This test need the following object relations :
* <ul>
*  <li> <code>'FL'</code> as <code>com.sun.star.form.XLoadable</code>
*    implementation : used to connect control to data source. </li>
*  <li> <code>'LC'</code> as <code>com.sun.star.uno.XInterface</code>
*    implementation : as value for LabelControl property (see property
*    documentation). </li>
*  <li> <code>'DataAwareControlModel.NewFieldName'</code> :
*   <b>optional</b> <code>String</code> relation which is used
*   for new value of DataField property.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.form.DataAwareControlModel
*/
public class _DataAwareControlModel extends MultiPropertyTest {

    /**
    * First checks if <code>com.sun.star.form.DataAwareControlModel</code>
    * service is supported by the object. Then <code>load</code> method
    * of <code>'FL'</code> relation is called to connect control model
    * to data source. Because the property is READONLY its value is
    * just checked to be non null. After that model is disconnected. <p>
    * Has <b> OK </b> status if the property has non null value. <p>
    * The following property tests are to be completed successfully before :
    * <ul>
    *  <li> <code> DataField </code> : to bind the control to some database
    *    field.</li>
    * </ul>
    */
    public void _BoundField() {
        requiredMethod("DataField") ;

        // This property is TRANSIENT and READONLY.
        boolean bResult = true;
        Object gValue = null;

        // get the loader to load the form
        XLoadable loader = (XLoadable) tEnv.getObjRelation("FL");

        try {
            if (loader.isLoaded()) {
                loader.unload() ;
            }
            loader.load();
            gValue = oObj.getPropertyValue("BoundField");
            loader.unload();
            bResult &= gValue != null;
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log) ;
            bResult = false;
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            bResult = false;
        }
        tRes.tested("BoundField", bResult);
    }

    /**
    * Sets the property to a Database field name, and then checks
    * if it was properly set. If <code>'DataAwareControlModel.NewFieldName'
    * </code> relation is not found, then new property value is
    * 'Address'. <p>
    * Has <b> OK </b> status if the property was properly set
    * and no exceptions were thrown. If old and new values are equal
    * the test is OK if no exceptions occurred and the value remains
    * the same.<p>
    */
    public void _DataField() {
        String relVal = (String) tEnv.getObjRelation
            ("DataAwareControlModel.NewFieldName") ;
        final String newVal = relVal == null ? "Address" : relVal ;
        testProperty("DataField", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object oldVal) {
                return newVal ;
            }
            @Override
            protected void checkResult(String propName, Object oldValue,
                Object newValue, Object resValue, Exception exception)
                throws java.lang.Exception{

                if (exception == null && oldValue.equals(newValue)) {
                    boolean res = true ;
                    if (newValue.equals(resValue)) {
                        log.println("Old value is equal to new value ('" +
                            oldValue + "'). Possibly no other suitable fields found.") ;
                    } else {
                        log.println("The value was '" + oldValue + "', set to the" +
                            " same value, but result is '" + resValue + "' : FAILED") ;
                        res = false ;
                    }

                    tRes.tested(propName, res) ;
                } else {
                    super.checkResult(propName, oldValue, newValue,
                                      resValue, exception);
                }
            }
        }) ;
    }

    /**
    * Sets the new value (<code>'LC'</code> relation if <code>null</code>
    * was before, and <code> null </code> otherwise and then checks if
    * the value have successfully changed. <p>
    * Has <b>OK</b> status if the value successfully changed.
    */
    public void _LabelControl() {
        final XInterface xTextLabel = (XInterface)tEnv.getObjRelation("LC");

        testProperty("LabelControl", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object oldVal) {
                if (utils.isVoid(oldVal)) return xTextLabel ;
                else return super.getNewValue("LabelControl",oldVal);
            }
        }) ;
    }

    /**
    * Forces environment recreation.
    */
    @Override
    public void after() {
        disposeEnvironment() ;
    }
}

