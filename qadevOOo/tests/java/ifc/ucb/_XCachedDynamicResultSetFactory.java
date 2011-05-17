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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XCachedDynamicResultSetFactory
*/
public class _XCachedDynamicResultSetFactory extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XCachedDynamicResultSetFactory oObj;
    private XDynamicResultSet resSetStub = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
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

