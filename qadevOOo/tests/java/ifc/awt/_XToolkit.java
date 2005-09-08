/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XToolkit.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:02:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XRegion;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindowPeer;

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
* Test is <b> NOT </b> multithread compilant. <p>
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
                res = true;
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while checking 'createWindow':");
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
                res = true;
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while checking 'createWindows':");
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

