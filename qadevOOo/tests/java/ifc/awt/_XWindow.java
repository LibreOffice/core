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

import com.sun.star.awt.FocusEvent;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.PaintEvent;
import com.sun.star.awt.Point;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.WindowEvent;
import com.sun.star.awt.XFocusListener;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XMouseMotionListener;
import com.sun.star.awt.XPaintListener;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowListener;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.EventObject;
import lib.MultiMethodTest;
import util.ValueComparer;

/**
* Testing <code>com.sun.star.awt.XWindow</code>
* interface methods :
* <ul>
*  <li><code> setPosSize()</code></li>
*  <li><code> getPosSize()</code></li>
*  <li><code> setVisible()</code></li>
*  <li><code> setEnable()</code></li>
*  <li><code> setFocus()</code></li>
*  <li><code> addWindowListener()</code></li>
*  <li><code> removeWindowListener()</code></li>
*  <li><code> addFocusListener()</code></li>
*  <li><code> removeFocusListener()</code></li>
*  <li><code> addKeyListener()</code></li>
*  <li><code> removeKeyListener()</code></li>
*  <li><code> addMouseListener()</code></li>
*  <li><code> removeMouseListener()</code></li>
*  <li><code> addMouseMotionListener()</code></li>
*  <li><code> removeMouseMotionListener()</code></li>
*  <li><code> addPaintListener()</code></li>
*  <li><code> removePaintListener()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XWindow.AnotherWindow'</code> (of type <code>XWindow</code>):
*   Some another window which can gain focus so the tested one
*   must lost it. </li>
*  <li> <code>'XWindow.ControlShape'</code> <b>optional</b>
*   (of type <code>XControlShape</code>):
*   Some shapes cann't change their size within fixed ControlShape
*   and their size could be changed only if size of container
*   ControlShape is changed. For such shapes this relation should
*   be passed for proper <code>addWindowListener</code> test. </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XWindow
*/
public class _XWindow extends MultiMethodTest {
    public XWindow oObj = null;
    private Rectangle posSize = null ;
    private XWindow win = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _getPosSize() {
        posSize = oObj.getPosSize() ;
        tRes.tested("getPosSize()", posSize != null) ;
    }

    /**
    * After defining Rectangle structure to be set, test calls the method. <p>
    * Has <b> OK </b> status if structure obtained using getPosSize() is
    * equal to structure previously set using setPosSize(). <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getPosSize() </code> :  returns the outer bounds of
    *  the window </li>
    * </ul>
    */
    public void _setPosSize() {
        Rectangle newRec = new Rectangle();

        requiredMethod("getPosSize()");
        newRec.X = posSize.X + 1;
        newRec.Y = posSize.Y + 1;
        newRec.Width = posSize.Width - 3;
        newRec.Height = posSize.Height - 3;
        oObj.setPosSize(newRec.X, newRec.Y, newRec.Width, newRec.Height,
            PosSize.POSSIZE);
        Rectangle gPS = oObj.getPosSize();
        log.println("Was : (" + posSize.X + ", " + posSize.Y + ", " +
            posSize.Width + ", " + posSize.Height + "), ");
        log.println("Set : (" + newRec.X + ", " + newRec.Y + ", " +
            newRec.Width + ", " + newRec.Height + "), ");
        log.println("Get : (" + gPS.X + ", " + gPS.Y + ", " +
            gPS.Width + ", " + gPS.Height + "). ");
        tRes.tested("setPosSize()", ValueComparer.equalValue(newRec, gPS) );
    }

    /**
    * At first object relation 'XWindow.AnotherWindow' is obtained.
    * Then test calls the methods of two 'XWindow' objects several times to be
    * sure that the focus has changed. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setFocus() {
        win = (XWindow) tEnv.getObjRelation("XWindow.AnotherWindow");
        oObj.setFocus();
        win.setFocus();
        oObj.setFocus();
        tRes.tested("setFocus()", true);
    }

    /**
    * Test calls the method twice with two parameters: 'true' and 'false'. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setVisible() {
        oObj.setVisible(false);
        oObj.setVisible(true);
        tRes.tested("setVisible()", true);
    }

    /**
    * Test calls the method twice with two parameters: 'true' and 'false'. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setEnable() {
        oObj.setEnable(false);
        oObj.setEnable(true);
        tRes.tested("setEnable()", true);
    }


    /**
    * A class we use to test addWindowListener() and
    * removeWindowListener()
    */
    public static class TestWindowListener implements XWindowListener {
        public boolean resized = false ;
        public boolean moved = false ;
        public boolean hidden = false ;
        public boolean shown = false ;

        public void init() {
            resized = false ;
            moved = false ;
            hidden = false ;
            shown = false ;
        }

        public void windowResized(WindowEvent e) {
            resized = true ;
        }
        public void windowMoved(WindowEvent e) {
            moved = true ;
        }
        public void windowHidden(EventObject e) {
            hidden = true ;
        }
        public void windowShown(EventObject e) {
            shown = true ;
        }
        public void disposing(EventObject e) {}

    }

    private final TestWindowListener wListener = new TestWindowListener() ;


    /**
    * Test calls the method. Then we check if listener's methods were called
    * when we move, resize, hide and show the window. The resizing is
    * performed depending on 'XWindow.ControlShape' existence. If this
    * relation exists then the size and position of container control
    * shape is changed, else the position and size of window itself is
    * changed<p>
    *
    * Has <b> OK </b> status if methods of wListener were called when
    * corresponding events occurred. <p>
    *
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> setPosSize() </code>: sets the outer bounds of the
    *  window</li>
    *  <li> <code> setVisible() </code>: shows or hides the window
    *  depending on the parameter</li>
    * </ul>
    */
    public void _addWindowListener() {
        executeMethod("setPosSize()");
        executeMethod("setVisible()");
        boolean result = true ;

        oObj.addWindowListener(wListener);

        // testing wListener.windowMoved()
        XControlShape ctrlShape = (XControlShape)
            tEnv.getObjRelation("XWindow.ControlShape");
        log.println("change object position and size...");

        if (ctrlShape != null) {
            try {
                Size sz = ctrlShape.getSize();
                sz.Height += 100;
                ctrlShape.setSize(sz);
                Point pos = ctrlShape.getPosition();
                pos.X += 100 ;
                ctrlShape.setPosition(pos);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("Couldn't change size or position: ");
                e.printStackTrace(log);
            }
        } else {
            oObj.setPosSize(posSize.X + 2, 0, 0, 0, PosSize.X);
            oObj.setPosSize(0, 0, 100, 100, PosSize.WIDTH);
        }

        waitForEventIdle();
        boolean res = wListener.resized && wListener.moved &&
            !wListener.hidden && !wListener.shown;
        result &= res;
        if (!res) {
            log.println("\twindowHidden()  wasn't called: " + !wListener.hidden);
            log.println("\twindowShown()   wasn't called: " + !wListener.shown);
            log.println("\twindowResized()    was called: " + wListener.resized);
            log.println("\twindowMoved()      was called: " + wListener.moved);
        } else {
            log.println("windowMoved() and windowResized() was called");
        }

        // testing wListener.windowHidden()
        wListener.init();
        waitForEventIdle();
        log.println("set object invisible...");
        oObj.setVisible(false);
        waitForEventIdle();
        res = wListener.hidden && !wListener.resized
                        && !wListener.moved && !wListener.shown;
        result &= res;
        if (!res) {
            log.println("\twindowHidden()     was called: " + wListener.hidden);
            log.println("\twindowShown()   wasn't called: " + !wListener.shown);
            log.println("\twindowResized() wasn't called: " + !wListener.resized);
            log.println("\twindowMoved()   wasn't called: " + !wListener.moved);
        } else {
            log.println("windowHidden() was called");
        }

        // testing wListener.windowShown()
        wListener.init() ;
        waitForEventIdle();
        log.println("set object visible...");
        oObj.setVisible(true) ;
        waitForEventIdle();
        res = wListener.shown && !wListener.resized &&
                !wListener.hidden && !wListener.moved;
        result &= res;
        if (!res) {
            log.println("\twindowHidden()  wasn't called: " + !wListener.hidden);
            log.println("\twindowShown()      was called: " + wListener.shown);
            log.println("\twindowResized() wasn't called: " + !wListener.resized);
            log.println("\twindowMoved()   wasn't called: " + !wListener.moved);
        } else {
            log.println("windowShown() was called");
        }

        tRes.tested("addWindowListener()", result) ;
    }


    /**
    * Test calls the method. Then we change window and check that listener's
    * methods were not called. <p>
    * Has <b> OK </b> status if listener does not react on window events.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addWindowListener() </code>: adds window listener to the
    *  object </li>
    * </ul>
    */
    public void _removeWindowListener() {
        requiredMethod("addWindowListener()");
        oObj.removeWindowListener(wListener);
        wListener.init();
        oObj.setPosSize(posSize.X, posSize.Y,
            posSize.Width , posSize.Height, PosSize.POSSIZE);
        oObj.setVisible(false);
        oObj.setVisible(true);
        boolean res = !(wListener.resized || wListener.moved
            || wListener.hidden || wListener.shown);

        tRes.tested("removeWindowListener()", res);
    }

    /**
    * A class we use to test addFocusListener() and
    * removeFocusListener()
    */
    public static class TestFocusListener implements XFocusListener {
        public boolean gained = false ;
        public boolean lost = false ;

        public void focusGained(FocusEvent e) {
            gained = true ;
        }
        public void focusLost(FocusEvent e) {
            lost = true ;
        }
        public void init() {
            gained = false;
            lost = false;
        }
        public void disposing(EventObject e) {}

    }

    private final TestFocusListener fListener = new TestFocusListener();

    /**
    * Test calls the method. Then we change focus and check that listener's
    * methods were called. <p>
    * Has <b> OK </b> status if methods of fListener were called when
    * corresponding events occurred. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFocus() </code>: sets the focus to the window </li>
    * </ul>
    */
    public void _addFocusListener() {
        boolean result = true ;

        requiredMethod("setFocus()");
        oObj.addFocusListener(fListener) ;

        // testing fListener.lost()
        oObj.setFocus();
        waitForEventIdle();
        win.setFocus();
        waitForEventIdle();
        result &= fListener.lost;
        if (!fListener.lost) {
            log.println("Lost focus was not notified about") ;
        }

        // testing fListener.gained()
        oObj.setFocus() ;
        waitForEventIdle();
        result &= fListener.gained;
        if (!fListener.gained) {
            log.println("Gained focus was not notified about") ;
        }

        tRes.tested("addFocusListener()", result) ;
    }

    /**
    * Test calls the method. Then we change focus and check that listener's
    * methods were not called. <p>
    * Has <b> OK </b> status if listener does not react on focus changing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addFocusListener() </code> : adds focus listener to
    * the object </li>
    * </ul>
    */
    public void _removeFocusListener() {
        requiredMethod("addFocusListener()");
        oObj.removeFocusListener(fListener);
        fListener.init();
        oObj.setFocus();
        win.setFocus();
        oObj.setFocus();
        boolean res = !(fListener.gained || fListener.lost);
        tRes.tested("removeFocusListener()", res);
    }

    /**
    * A class we use to test addKeyListener() and
    * removeKeyListener()
    */
    public static class TestKeyListener implements XKeyListener {
        public boolean pressed = false;
        public boolean released = false;
        public void keyPressed(KeyEvent e) { pressed = true; }
        public void keyReleased(KeyEvent e) { released = true; }
        public void disposing(EventObject e) {}
        public void init() { pressed = false; released = false; }
    }

    private final TestKeyListener kListener = new TestKeyListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _addKeyListener() {
        oObj.addKeyListener(kListener);
        tRes.tested("addKeyListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addKeyListener() </code> : adds key listener to
    *  the object </li>
    * </ul>
    */
    public void _removeKeyListener() {
        requiredMethod("addKeyListener()");
        oObj.removeKeyListener(kListener);
        tRes.tested("removeKeyListener()", true);
    }

    /**
    * A class we use to test addMouseListener() and
    * removeMouseListener()
    */
    public static class TestMouseListener implements XMouseListener {
        public boolean pressed = false;
        public boolean released = false;
        public boolean entered = false;
        public boolean exited = false;

        public void mousePressed(MouseEvent e) {
            pressed = true;
        }

        public void mouseReleased(MouseEvent e) {
            released = true;
        }

        public void mouseEntered(MouseEvent e) {
            entered = true;
        }

        public void mouseExited(MouseEvent e) {
            exited = true;
        }

        public void disposing(EventObject e) {}

        public void init() {
            pressed = false;
            released = false;
            exited = false;
            entered = false;
        }

    }

    private final TestMouseListener mListener = new TestMouseListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _addMouseListener() {
        oObj.addMouseListener(mListener);
        tRes.tested("addMouseListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addMouseListener() </code> : adds mouse listener to
    * the object</li>
    * </ul>
    */
    public void _removeMouseListener() {
        requiredMethod("addMouseListener()");
        oObj.removeMouseListener(mListener);
        tRes.tested("removeMouseListener()", true);
    }

    /**
    * A class we use to test addMouseMotionListener() and
    * removeMouseMotionListener()
    */
    public static class TestMouseMotionListener implements XMouseMotionListener {
        public boolean dragged = false;
        public boolean moved = false;

        public void mouseDragged(MouseEvent e) {
            dragged = true;
        }

        public void mouseMoved(MouseEvent e) {
            moved = true;
        }

        public void disposing(EventObject e) {}

        public void init() {
            dragged = false;
            moved = false;
        }

    }

    private final TestMouseMotionListener mmListener = new TestMouseMotionListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _addMouseMotionListener() {
        oObj.addMouseMotionListener(mmListener);
        tRes.tested("addMouseMotionListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addMouseMotionListener() </code> : adds mouse motion
    * listener to the object</li>
    * </ul>
    */
    public void _removeMouseMotionListener() {
        requiredMethod("addMouseMotionListener()");
        oObj.removeMouseMotionListener(mmListener);
        tRes.tested("removeMouseMotionListener()", true);
    }

    /**
    * A class we use to test addPaintListener() and
    * removePaintListener()
    */
    public static class TestPaintListener implements XPaintListener {
        public boolean paint = false;

        public void windowPaint(PaintEvent e) {
            paint = true;
        }

        public void disposing(EventObject e) {}

        public void init() {
            paint = false;
        }

    }

    private final TestPaintListener pListener = new TestPaintListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _addPaintListener() {
        oObj.addPaintListener(pListener);
        tRes.tested("addPaintListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addPaintListener() </code> : adds paint listener to
    * the object </li>
    * </ul>
    */
    public void _removePaintListener() {
        requiredMethod("addPaintListener()");
        oObj.removePaintListener(pListener);
        tRes.tested("removePaintListener()", true);
    }

}

