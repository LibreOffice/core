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
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("OutputStream");
        oStream = (XOutputStream) UnoRuntime.queryInterface
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
    public void after() {
        this.disposeEnvironment() ;
    }
}

