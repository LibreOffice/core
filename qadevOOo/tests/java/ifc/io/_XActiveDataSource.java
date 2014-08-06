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

import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XOutputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XActiveDataSource</code>
* interface methods:
* <ul>
*   <li><code>setOutputStream()</code></li>
*   <li><code>getOutputStream()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'OutputStream'</code>
*  (of type <code>com.sun.star.io.OutputStream</code>):
*   acceptable output stream which can be set by <code>setOutputStream</code> </li>
* <ul> <p>
*
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XOutputStream
*/
public class _XActiveDataSource extends MultiMethodTest {

    public XActiveDataSource oObj = null;

    private XOutputStream oStream = null;

    /**
    * Take the XOutputStream from the environment for setting and getting.
    */
    @Override
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("OutputStream");
        oStream = UnoRuntime.queryInterface
                (XOutputStream.class, x) ;
    }

    /**
    * Test calls the method using interface <code>XOutputStream</code>
    * received in method <code>before()</code> as parameter. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setOutputStream() {
        oObj.setOutputStream(oStream);
        tRes.tested("setOutputStream()", true);
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set in the method <code>setOutputStream()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setOutputStream() </code></li>
    * </ul>
    */
    public void _getOutputStream() {
        requiredMethod("setOutputStream()");

        tRes.tested("getOutputStream()",
            oStream.equals(oObj.getOutputStream()));
    }

    /**
    * Forces object environment recreation.
    */
    @Override
    public void after() {
        this.disposeEnvironment() ;
    }
}

