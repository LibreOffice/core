/*************************************************************************
 *
 *  $RCSfile: _XCachedDynamicResultSetStubFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:23:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.ucb;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.XMultiServiceFactory;
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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XCachedDynamicResultSetStubFactory
*/
public class _XCachedDynamicResultSetStubFactory extends MultiMethodTest {

    /**
     * Conatins the tested object.
     */
    public XCachedDynamicResultSetStubFactory oObj;
    private XDynamicResultSet resSet = null ;
    private XDynamicResultSet resSetStub = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If relation not found.
    */
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

        resSetStub = oObj.createCachedDynamicResultSetStub(resSet) ;

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
    public void _connectToCache() {
        boolean result = true ;

        XCachedDynamicResultSetFactory setFac = null ;

        try {
            Object fac = ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.ucb.CachedDynamicResultSetFactory") ;

            setFac = (XCachedDynamicResultSetFactory) UnoRuntime.queryInterface
                (XCachedDynamicResultSetFactory.class, fac) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Cant instantiate a service") ;
            e.printStackTrace(log) ;
            result = false ;
        }

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

