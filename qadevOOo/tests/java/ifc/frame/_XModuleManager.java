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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XModuleManager
*/
public class _XModuleManager extends MultiMethodTest {
    /** Test calls the method. <p>
     * The onject relations <CODE>XModuleManager.XFrame</CODE>,
     * <CODE>XModuleManager.XController</CODE> and <CODE>XModuleManager.XModel</CODE>
     * are sequenzes of <CODE>PropertyValue</CODE>. The value of a PropertyValue
     * containes a <CODE>XFrame</CODE>, <CODE>XController</CODE> or a
     * <CODE>XModel</CODE>. The name of the PropertyValue contains the expected return
     * value of method <CODE>indetify()</CODE> if the method was called with
     * coresponding value.<p>
     * As enhancement the method <CODE>identify()</CODE> was called with incvalid
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

    /** The method <CODE>identify()</CODE> was tesed for every entry in sequeze of
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
                if (oObj.identify(sequenze[i].Value).equals(
                                                        sequenze[i].Name)){
                     ok &= ok;
                }else{
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

