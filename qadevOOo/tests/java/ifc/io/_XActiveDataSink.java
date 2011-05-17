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

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XActiveDataSink</code>
* interface methods :
* <ul>
*  <li><code> setInputStream()</code></li>
*  <li><code> getInputStream()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'InputStream'</code>
*  (of type <code>com.sun.star.io.XInputStream</code>):
*   acceptable input stream which can be set by <code>setInputStream</code> </li>
* <ul> <p>
*
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XActiveDataSink
*/
public class _XActiveDataSink extends MultiMethodTest {

    public XActiveDataSink oObj = null;

    private XInputStream iStream = null;

        /**
        * Take the XInputStream from the environment for setting and getting
    */
    public void before() {
            XInterface x = (XInterface)tEnv.getObjRelation("InputStream");
            iStream = (XInputStream) UnoRuntime.queryInterface
                    (XInputStream.class, x) ;
    }

    /**
    * Just sets new input stream. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _setInputStream() {
        oObj.setInputStream(iStream) ;

        tRes.tested("setInputStream()", true) ;
    }

    /**
    * First retrieves current input stream, then sets to new
    * input stream (if old was <code>null</code>) or to null.
    * Then input stream retrieved again and checked to be not
    * equal to the old one. <p>
    * Has <b>OK</b> status if old and new streams retrieved are
    * not equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setInputStream() </code> : to be sure the method
    *   works without exceptions. </li>
    * </ul>
    */
    public void _getInputStream() {
        requiredMethod("setInputStream()") ;

        Object oldStream = oObj.getInputStream() ;
        XInputStream newStream = oldStream == null ? iStream : null ;

        oObj.setInputStream(newStream) ;
        Object getStream = oObj.getInputStream() ;

                tRes.tested("getInputStream()", getStream != oldStream) ;
    }

    public void after() {
        this.disposeEnvironment() ;
    }
}


