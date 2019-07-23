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
import com.sun.star.ucb.XCachedContentResultSetStubFactory;

/**
* Testing <code>com.sun.star.ucb.XCachedContentResultSetStubFactory</code>
* interface methods :
* <ul>
*  <li><code> createCachedContentResultSetStub()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ContentResultSet'</code> (of type
*   <code>com.sun.star.sdbc.XResultSet</code>):
*   this must be an implementation of <code>
*   com.sun.star.ucb.ContentResultSet</code> service.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XCachedContentResultSetStubFactory
*/
public class _XCachedContentResultSetStubFactory extends MultiMethodTest {

    /**
     * Contains the tested object.
     */
    public XCachedContentResultSetStubFactory oObj;
    private XResultSet resSet = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
    @Override
    public void before() {
        resSet = (XResultSet) tEnv.getObjRelation("ContentResultSet") ;
        if (resSet == null) {
            log.println("!!! Relation not found !!!") ;
            throw new StatusException(Status.failed("!!! Relation not found !!!")) ;
        }
    }

    /**
    * Creates cached result set stub from static result set. After that number
    * of rows in cached result set created and its source set are retrieved
    * and comared. <p>
    * Has <b>OK</b> status if numbers of rows are equal and they are
    * greater then 0 (because JAR file contains at least one entry).
    */
    public void _createCachedContentResultSetStub() {
        boolean result = true ;

        XResultSet resSetStub = oObj.createCachedContentResultSetStub
            (resSet) ;

        if (resSetStub == null) {
            log.println("!!! Method returned null !!!") ;
            result = false ;
        } else {
            try {
                resSetStub.last() ;
                int stubRowNum = resSetStub.getRow() ;

                resSet.last() ;
                int setRowNum = resSet.getRow() ;

                result = stubRowNum == setRowNum && setRowNum > 0 ;

                log.println("Number of rows : set=" + setRowNum +
                    " stub=" + stubRowNum) ;
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("!!! Something wrong with result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            }
        }

        tRes.tested("createCachedContentResultSetStub()", result) ;

    }
}

