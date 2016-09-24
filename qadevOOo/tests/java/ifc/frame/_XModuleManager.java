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

package ifc.frame;

import com.sun.star.beans.PropertyValue;

import com.sun.star.frame.XModuleManager;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.frame.UnknownModuleException;


/**
* Testing <code>com.sun.star.frame.XModuleManager</code>
* interface methods:
* <ul>
*  <li><code> identify() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XModuleManager.XFrame'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XFrame
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
*  <li> <code>'XModuleManager.XController'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XController
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
*  <li> <code>'XModuleManager.XModel'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XFrame
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.frame.XModuleManager
*/
public class _XModuleManager extends MultiMethodTest {
    /** Test calls the method. <p>
     * The onject relations <CODE>XModuleManager.XFrame</CODE>,
     * <CODE>XModuleManager.XController</CODE> and <CODE>XModuleManager.XModel</CODE>
     * are sequenzes of <CODE>PropertyValue</CODE>. The value of a PropertyValue
     * contains a <CODE>XFrame</CODE>, <CODE>XController</CODE> or a
     * <CODE>XModel</CODE>. The name of the PropertyValue contains the expected return
     * value of method <CODE>indetify()</CODE> if the method was called with
     * coresponding value.<p>
     * As enhancement the method <CODE>identify()</CODE> was called with invalid
     * parameter. In this case the thrown exceptions was catched.
     */
    public XModuleManager oObj = null;
    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns expected values, that's equal to
    * previously obtained object relation 'Frame'.
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> attachFrame() </code> : attachs frame obtained object
    * relation 'Frame' </li>
    * </ul>
    */

    private PropertyValue[] xFrameSeq = null;
    private PropertyValue[] xControllerSeq = null;
    private PropertyValue[] xModelSeq = null;
    /** Retrieves object relations. */

    @Override
    public void before() {

        xFrameSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XFrame") ;

        if (xFrameSeq == null) throw new StatusException
            (Status.failed("Relation 'xFrameSeq' not found.")) ;


        xControllerSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XController") ;

        if (xControllerSeq == null) throw new StatusException
            (Status.failed("Relation 'xControllerSeq' not found.")) ;


        xModelSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XModel") ;

        if (xModelSeq == null) throw new StatusException
            (Status.failed("Relation 'xModelSeq' not found.")) ;
    }

    /* The method <CODE>identify()</CODE> was used for every entry in sequence of
     * object relations.
     */
    public void _identify() {
        boolean ok = true;
        log.println("testing frame sequenze...");
        ok &= testSequenze(xFrameSeq);
        log.println("testing controller sequenze...");
        ok &= testSequenze(xControllerSeq);
        log.println("testing model sequenze...");
        ok &= testSequenze(xModelSeq);
        tRes.tested("identify()", ok);

        log.println("testing invalid objects...");
        try{
            oObj.identify(oObj);
        } catch (IllegalArgumentException e){
            log.println("expected exception.");
        } catch (UnknownModuleException e){
            log.println("expected exception.");
        }
    }

    private boolean testSequenze(PropertyValue[] sequenze){
        boolean ok = true;
        for (int i = 0 ; i < sequenze.length; i++){
            try{
                log.println("testing '" + sequenze[i].Name + "'");
                if (!oObj.identify(sequenze[i].Value).equals(
                                                        sequenze[i].Name)) {
                     log.println("failure: returned value: '" +
                                 oObj.identify(sequenze[i].Value) +
                                 "' ,expected value: '" + sequenze[i].Name + "'");
                     ok = false;
                }
            } catch (IllegalArgumentException e){
                log.println("Could not get value of sequenze '" +
                            sequenze[i].Name + "'");
                return false;

            } catch (UnknownModuleException e){
                log.println("Could not indetify value of sequenze '" +
                            sequenze[i].Name + "'");
                return false;
            }
        }
        return ok;
    }

}

