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

package ifc.uno;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;


/**
* Testing <code>com.sun.star.uno.XNamingService</code>
* interface methods. <p>
* This test need the following object relations :
* <ul>
*  <li> <code>'XNamingService.RegisterObject'</code> :
*     object to be registered.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
*/
public class _XNamingService extends MultiMethodTest {

    public static XNamingService oObj = null;

    private XInterface regObject = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    @Override
    public void before() {
        regObject = (XInterface)
            tEnv.getObjRelation("XNamingService.RegisterObject") ;
        if (regObject == null)
            throw new StatusException("Can't create object to register",
                new NullPointerException()) ;
    }

    /**
    * Test calls the method and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    */
    public void _registerObject() throws StatusException{
        try {
            oObj.registerObject("MyFactory", regObject) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception while registering object :" + e) ;
            tRes.tested("registerObject()", false) ;
            return ;
        }

        tRes.tested("registerObject()", true) ;
    }

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns the same object
    * that was registered and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerObject </code> : to get in this test the
    *    object that was registered.</li>
    * </ul>
    */
    public void _getRegisteredObject() {
        requiredMethod("registerObject()") ;

        try {
            Object getObject = oObj.getRegisteredObject("MyFactory") ;

            tRes.tested("getRegisteredObject()" ,
                regObject.equals(getObject)) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception calling method :" + e) ;
            tRes.tested("getRegisteredObject()", false) ;
            return ;
        }
    }

    /**
    * Test calls the method and trying to get revoked object. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and if the method <code>getRegisteredObject</code> returns NULL or
    * throws expected exception. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerObject </code> : to revoke the object registered</li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getRegisteredObject </code> : before object will be
    *    revoked </li>
    * </ul>
    */
    public void _revokeObject() {
        requiredMethod("registerObject()") ;
        executeMethod("getRegisteredObject()") ;

        try {
            oObj.revokeObject("MyFactory");
            log.println("Object was revoked");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception revoking object :" + e) ;
            tRes.tested("revokeObject()", false) ;
        }

        boolean res = true;

        try {
            log.println("Trying to getRegistered object ...");
            Object objregObj = oObj.getRegisteredObject("MyFactory");
            log.println("No exception");
            res &= objregObj == null;
            if (res) {
                log.println("But NULL was returned");
            }
        } catch(com.sun.star.uno.Exception e) {
            log.println("Expected exception - OK");
        }

        tRes.tested("revokeObject()", res);
    }
}

