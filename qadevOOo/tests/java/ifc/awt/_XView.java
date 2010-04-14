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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Size;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XView;

/**
* Testing <code>com.sun.star.awt.XView</code>
* interface methods:
* <ul>
*  <li><code> setGraphics() </code></li>
*  <li><code> getGraphics() </code></li>
*  <li><code> getSize() </code></li>
*  <li><code> draw() </code></li>
*  <li><code> setZoom() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'GRAPHICS'</code> (of type <code>XGraphics</code>):
*   used as a parameter to setGraphics() </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XView
*/
public class _XView extends MultiMethodTest {
    public XView oObj = null;

    /**
    * After obtaining object relation 'GRAPHICS', test calls the method. <p>
    * Has <b> OK </b> status if the method returns true.
    */
    public void _setGraphics() {
        XGraphics graph = (XGraphics) tEnv.getObjRelation("GRAPHICS");
        boolean isSet = oObj.setGraphics(graph);
        if ( !isSet ) {
            log.println("setGraphics() returns false");
        }
        tRes.tested("setGraphics()", isSet);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _getGraphics() {
        requiredMethod("setGraphics()");
        XGraphics graph = oObj.getGraphics();
        if (graph == null) {
            log.println("getGraphics() returns NULL");
        }
        tRes.tested("getGraphics()", graph != null);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns structure with fields that
    * are not equal to zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _getSize() {
        requiredMethod("setGraphics()");
        Size aSize = oObj.getSize();
        boolean res = (aSize.Height != 0) && (aSize.Width != 0);
        if ( !res ) {
            log.println("Height: " + aSize.Height);
            log.println("Width: " + aSize.Width);
        }
        tRes.tested("getSize()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _draw() {
        requiredMethod("setGraphics()");
        oObj.draw(20, 20);
        tRes.tested("draw()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _setZoom() {
        requiredMethod("setGraphics()");
        oObj.setZoom(2,2);
        tRes.tested("setZoom()", true);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

}

