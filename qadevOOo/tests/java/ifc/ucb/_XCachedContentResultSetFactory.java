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
import com.sun.star.ucb.XCachedContentResultSetFactory;

/**
* Testing <code>com.sun.star.ucb.XCachedContentResultSetFactory</code>
* interface methods :
* <ul>
*  <li><code> createCachedContentResultSet()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CachedContentResultSetStub'</code> (of type
*   <code>com.sun.star.sdbc.XResultSet</code>):
*   this must be an imlementation of <code>
*   com.sun.star.ucb.CachedContentResultSetStub</code> service.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XCachedContentResultSetFactory
*/
public class _XCachedContentResultSetFactory extends MultiMethodTest {

    /**
     * Contains the tested object.
     */
    public XCachedContentResultSetFactory oObj;
    private XResultSet resSetStub = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
    @Override
    public void before() {
        resSetStub = (XResultSet)
            tEnv.getObjRelation("CachedContentResultSetStub") ;
        if (resSetStub == null) {
            log.println("!!! Relation not found !!!") ;
            throw new StatusException
                (Status.failed("!!! Relation not found !!!")) ;
        }
    }

    /**
    * Creates result set from result set stub. After that number
    * of rows from result set created and its stub are retrieved
    * using their static representations and comared. <p>
    * Has <b>OK</b> status if numbers of rows are equal and they are
    * greater then 0 (because JAR file contains at least one entry).
    */
    public void _createCachedContentResultSet() {
        boolean result = true ;

        XResultSet resSet = oObj.createCachedContentResultSet
            (resSetStub, null) ;

        if (resSet == null) {
            log.println("!!! Method returned null !!!") ;
            result = false ;
        } else {
            try {
                resSetStub.last() ;
                int stubRowNum = resSetStub.getRow() ;

                resSet.last() ;
                int setRowNum = resSet.getRow() ;

                result = stubRowNum == setRowNum && setRowNum > 0 ;

                log.println("Number of rows : stub=" + stubRowNum +
                    " set=" + setRowNum) ;
            } catch (com.sun.star.sdbc.SQLException e) {
                log.println("!!! Something wrong with result sets :") ;
                e.printStackTrace(log) ;
                result = false ;
            }
        }

        tRes.tested("createCachedContentResultSet()", result) ;

    }
}

