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
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
*/
public class _XNamingService extends MultiMethodTest {

    public static XNamingService oObj = null;

    private XInterface regObject = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
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

