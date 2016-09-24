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
    @Override
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("Connectable");
        xConnect = UnoRuntime.queryInterface(
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
    @Override
    public void after() {
        this.disposeEnvironment() ;
    }
}

