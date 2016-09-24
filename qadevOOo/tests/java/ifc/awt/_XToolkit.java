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

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XRegion;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.awt.XToolkit</code>
* interface methods:
* <ul>
*  <li><code> getDesktopWindow() </code></li>
*  <li><code> getWorkArea() </code></li>
*  <li><code> createWindow() </code></li>
*  <li><code> createWindows() </code></li>
*  <li><code> createScreenCompatibleDevice() </code></li>
*  <li><code> createRegion() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XToolkit
*/
public class _XToolkit extends MultiMethodTest {
    public XToolkit oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status always, because Desktop component
    * currently is not supported as visible.
    */
    public void _getDesktopWindow() {
        XWindowPeer win = oObj.getDesktopWindow();
        if (win == null) {
            log.println("getDesktopWindow() returns NULL");
        }
        tRes.tested("getDesktopWindow()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _getWorkArea() {
        Rectangle area = oObj.getWorkArea();
        tRes.tested("getWorkArea()", area != null);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _createWindow() {
        boolean res = false;
        try {
            XWindowPeer cWin = oObj.createWindow(
                createDesc(new Rectangle(0,0,100,100)));
            if (cWin == null) {
                log.println("createWindow() create a NULL Object");
            } else {
                UnoRuntime.queryInterface(XComponent.class, cWin).dispose();
                res = true;
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while checking 'createWindow':");
            ex.printStackTrace(log);
        }
        tRes.tested("createWindow()", res);
    }

    /**
    * After defining of WindowDescriptor array, test calls the method. <p>
    * Has <b> OK </b> status if all elements of the returned array are
    * not null.
    */
    public void _createWindows() {
        boolean res = false;
        try {
            WindowDescriptor[] descs = new WindowDescriptor[2];
            descs[0] = createDesc(new Rectangle(0,0,100,100));
            descs[1] = createDesc(new Rectangle(100,100,200,200));
            XWindowPeer[] cWins = oObj.createWindows(descs);
            if ( (cWins[0] == null) || (cWins[1] == null) ) {
                log.println("createWindows() creates NULL Windows");
            } else {
                UnoRuntime.queryInterface(XComponent.class, cWins[0]).dispose();
                UnoRuntime.queryInterface(XComponent.class, cWins[1]).dispose();
                res = true;
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while checking 'createWindows':");
            ex.printStackTrace(log);
        }
        tRes.tested("createWindows()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _createScreenCompatibleDevice() {
        XDevice dev = oObj.createScreenCompatibleDevice(100, 100);
        tRes.tested("createScreenCompatibleDevice()", dev != null);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _createRegion() {
        XRegion reg = oObj.createRegion();
        tRes.tested("createRegion()", reg != null);
    }

    /**
    * Just creates the WindowDescriptor as an argument for createWindow().
    */
    public WindowDescriptor createDesc(Rectangle rect) {
        XWindowPeer win = (XWindowPeer) tEnv.getObjRelation("WINPEER");
        return new WindowDescriptor(com.sun.star.awt.WindowClass.TOP,
            "", win, (short) -1, rect, com.sun.star.awt.WindowAttribute.SHOW);
    }

}

