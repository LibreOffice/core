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

package ifc.ucb;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sdbc.XResultSet;
import com.sun.star.ucb.XCachedDynamicResultSetFactory;
import com.sun.star.ucb.XCachedDynamicResultSetStubFactory;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.ucb.XCachedDynamicResultSetStubFactory</code>
* interface methods :
* <ul>
*  <li><code> createCachedDynamicResultSetStub()</code></li>
*  <li><code> connectToCache()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DynamicResultSet'</code> (of type
*   <code>com.sun.star.sdbc.XDynamicResultSet</code>):
*   this must be an imlementation of <code>
*   com.sun.star.ucb.DynamicResultSet</code> service.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XCachedDynamicResultSetStubFactory
*/
public class _XCachedDynamicResultSetStubFactory extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XCachedDynamicResultSetStubFactory oObj;
    private XDynamicResultSet resSet = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
    @Override
    public void before() {
        resSet = (XDynamicResultSet) tEnv.getObjRelation("DynamicResultSet") ;
        if (resSet == null) {
            log.println("!!! Relation not found !!!") ;
            throw new StatusException(Status.failed("!!! Relation not found !!!")) ;
        }
    }

    /**
    * Creates result set stub from result set. After that number
    * of rows from result set stub created and its source set are retrieved
    * using their static representations and compared. <p>
    * Has <b>OK</b> status if numbers of rows are equal and they are
    * greater then 0 (because JAR file contains at least one entry).
    */
    public void _createCachedDynamicResultSetStub() {
        boolean result = true ;

        XDynamicResultSet resSetStub = oObj.createCachedDynamicResultSetStub(resSet) ;

        if (resSetStub == null) {
            log.println("!!! Method returned null !!!") ;
            result = false ;
        } else {
            try {
                XResultSet resSetS = resSet.getStaticResultSet() ;
                XResultSet resSetStubS = resSetStub.getStaticResultSet() ;

                resSetStubS.last() ;
                int stubRowNum = resSetStubS.getRow() ;

                resSetS.last() ;
                int setRowNum = resSetS.getRow() ;

                result = stubRowNum == setRowNum && setRowNum > 0 ;

                log.println("Number of rows : stub=" + stubRowNum +
                    " set=" + setRowNum) ;
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("!!! Something wrong with result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            } catch (com.sun.star.ucb.ListenerAlreadySetException e) {
                log.println("!!! Can't get static result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            }
        }

        tRes.tested("createCachedDynamicResultSetStub()", result) ;
    }

    /**
    * Creates an instance of <code>CachedDynamicResultSet</code> service
    * which is not connected to any stub. Then tries to connect it to
    * <code>DynaminResultSet</code> created and passed as relation.
    * Connection is checked by retrieving and comparing of row numbers
    * of connected set and its source set. <p>
    * Has <b>OK</b> status if row numbers are equal and they are
    * greater then 0 (because JAR file contains at least one entry).
    */
    public void _connectToCache() throws Exception {
        boolean result = true ;

        Object fac = tParam.getMSF().createInstance
            ("com.sun.star.ucb.CachedDynamicResultSetFactory") ;

        XCachedDynamicResultSetFactory setFac = UnoRuntime.queryInterface
            (XCachedDynamicResultSetFactory.class, fac) ;

        XDynamicResultSet rmtSet = setFac.createCachedDynamicResultSet(null, null) ;

        try {
            oObj.connectToCache(resSet, rmtSet, null, null) ;
        } catch (com.sun.star.ucb.ListenerAlreadySetException e) {
            log.println("!!! Unexpected exception :" + e) ;
            result = false ;
        } catch (com.sun.star.ucb.AlreadyInitializedException e) {
            log.println("!!! Unexpected exception :" + e) ;
            result = false ;
        }

        if (result) {
            // checking connection to the source
            try {
                XResultSet statRmtSet = rmtSet.getStaticResultSet() ;
                XResultSet statResSet = resSet.getStaticResultSet() ;

                statRmtSet.last() ;
                int rmtRowNum = statRmtSet.getRow() ;

                statResSet.last() ;
                int resRowNum = statResSet.getRow() ;

                result = rmtRowNum == resRowNum && resRowNum > 0 ;

                log.println("Number of rows : destination=" + rmtRowNum +
                    " source=" + resRowNum) ;
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("!!! Something wrong with result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            } catch (com.sun.star.ucb.ListenerAlreadySetException e) {
                log.println("!!! Something wrong with result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            }
        }

        tRes.tested("connectToCache()", result) ;
    }

}

