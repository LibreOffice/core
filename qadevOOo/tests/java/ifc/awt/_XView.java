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
* Test is <b> NOT </b> multithread compliant. <p>
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
    @Override
    protected void after() {
        disposeEnvironment();
    }

}

