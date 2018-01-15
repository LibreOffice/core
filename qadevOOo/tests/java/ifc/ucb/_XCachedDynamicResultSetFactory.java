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
import com.sun.star.ucb.XDynamicResultSet;

/**
* Testing <code>com.sun.star.ucb.XCachedDynamicResultSetFactory</code>
* interface methods :
* <ul>
*  <li><code> createCachedDynamicResultSet()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CachedDynamicResultSetStub'</code> (of type
*   <code>com.sun.star.sdbc.XDynamicResultSet</code>):
*   this must be an imlementation of <code>
*   com.sun.star.ucb.CachedDynamicResultSetStub</code> service.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XCachedDynamicResultSetFactory
*/
public class _XCachedDynamicResultSetFactory extends MultiMethodTest {

    /**
     * Contains the tested object.
     */
    public XCachedDynamicResultSetFactory oObj;
    private XDynamicResultSet resSetStub = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
    @Override
    public void before() {
        resSetStub = (XDynamicResultSet)
            tEnv.getObjRelation("CachedDynamicResultSetStub") ;
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
    public void _createCachedDynamicResultSet() {
        boolean result = true ;

        XDynamicResultSet resSet = oObj.createCachedDynamicResultSet
            (resSetStub, null) ;

        if (resSet == null) {
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

        tRes.tested("createCachedDynamicResultSet()", result) ;

    }
}

