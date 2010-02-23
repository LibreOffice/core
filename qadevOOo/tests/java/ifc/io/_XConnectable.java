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

package ifc.io;

import lib.MultiMethodTest;

import com.sun.star.io.XConnectable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XConnectable</code>
* interface methods:
* <ul>
*   <li><code>setPredecessor()</code></li>
*   <li><code>getPredecessor()</code></li>
*   <li><code>setSuccessor()</code></li>
*   <li><code>getSuccessor()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Connectable'</code> (supports the <code>XConnectable</code>
*   interface):
*   another 0bject to connect </li>
* </ul>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XConnectable
*/
public class _XConnectable extends MultiMethodTest {

    public XConnectable oObj = null;

    private XConnectable xConnect = null ;

    /**
     * Get another connectable object from object relations.
    */
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("Connectable");
        xConnect = (XConnectable)UnoRuntime.queryInterface(
                                                    XConnectable.class, x) ;
    }

    /**
    * Test calls the method using interface <code>XConnectable</code>
    * received in method <code>before()</code> as parameter. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setPredecessor() {
        oObj.setPredecessor(xConnect) ;

        tRes.tested("setPredecessor()", true) ;
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set in the method <code>setPredecessor()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setPredecessor() </code></li>
    * </ul>
    */
    public void _getPredecessor() {
        requiredMethod("setPredecessor()") ;

        XConnectable gConnect = oObj.getPredecessor() ;

        tRes.tested("getPredecessor()", xConnect.equals(gConnect)) ;
    }

    /**
    * Test calls the method using interface <code>XConnectable</code>
    * received in method <code>before()</code> as parameter. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setSuccessor() {
        oObj.setSuccessor(xConnect) ;

        tRes.tested("setSuccessor()", true) ;
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set in the method <code>setSuccessor()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setSuccessor() </code></li>
    * </ul>
    */
    public void _getSuccessor() {
        requiredMethod("setSuccessor()") ;

        XConnectable gConnect = oObj.getSuccessor() ;

        tRes.tested("getSuccessor()", xConnect.equals(gConnect)) ;
    }

    /**
    * Forces object environment recreation.
    */
    public void after() {
        this.disposeEnvironment() ;
    }
}

