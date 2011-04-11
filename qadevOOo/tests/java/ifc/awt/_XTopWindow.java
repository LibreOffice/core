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

import java.io.PrintWriter;

import lib.MultiMethodTest;

import com.sun.star.awt.XMenuBar;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XTopWindowListener;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.awt.XTopWindow</code>
* interface methods :
* <ul>
*  <li><code> addTopWindowListener()</code></li>
*  <li><code> removeTopWindowListener()</code></li>
*  <li><code> toFront()</code></li>
*  <li><code> toBack()</code></li>
*  <li><code> setMenuBar()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XTopWindow
*/
public class _XTopWindow extends MultiMethodTest {

    public XTopWindow oObj = null;

    /**
    * Listener implementation which sets flags on different
    * method calls.
    */
    protected class TestListener implements XTopWindowListener {
        private PrintWriter log = null ;
        public boolean activated = false ;
        public boolean deactivated = false ;

        public TestListener(PrintWriter log) {
            this.log = log ;
        }

        public void initListener() {
            activated = false;
            deactivated = false;
        }

        public void windowOpened(EventObject e) {
            log.println("windowOpened() called") ;
        }
        public void windowClosing(EventObject e) {
            log.println("windowClosing() called") ;
        }
        public void windowClosed(EventObject e) {
            log.println("windowClosed() called") ;
        }
        public void windowMinimized(EventObject e) {
            log.println("windowMinimized() called") ;
        }
        public void windowNormalized(EventObject e) {
            log.println("windowNormalized() called") ;
        }
        public void windowActivated(EventObject e) {
            activated = true;
            log.println("windowActivated() called") ;
        }
        public void windowDeactivated(EventObject e) {
            deactivated = true;
            log.println("windowDeactivated() called") ;
        }
        public void disposing(EventObject e) {}
    }

    protected TestListener listener = null ;

    XTextDocument aTextDoc = null;


    protected void before() {
        aTextDoc = util.WriterTools.createTextDoc((XMultiServiceFactory)tParam.getMSF());
    }

    /**
     * Adds a listener . <p>
     *
     * Has <b>OK</b> status always (listener calls are checked in
     * other methods. <p>
     */
    public void _addTopWindowListener() {
        listener = new TestListener(log) ;

        oObj.addTopWindowListener(listener) ;

        tRes.tested("addTopWindowListener()", true);
    }

    /**
     * Removes a listener added before. <p>
     * Has <b>OK</b> status always. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> toBack </code> : to have a definite method execution
     *    order.</li>
     * </ul>
     */
    public void _removeTopWindowListener() {
        executeMethod("toBack()");

        oObj.removeTopWindowListener(listener);

        tRes.tested("removeTopWindowListener()", true);
    }

    /**
     * Moves the window to front and check the listener calls. <p>
     * Has <b>OK</b> status if listener <code>activated</code> method
     * was called.
     */
    public void _toFront() {
        requiredMethod("addTopWindowListener()");
        listener.initListener();
        oObj.toFront();
        shortWait();

        tRes.tested("toFront()", listener.activated && !listener.deactivated);
    }

    /**
     * This method doesn't do anything the Office implementation. <p>
     * So it has always <b>OK</b> status
     */
    public void _toBack() {
        oObj.toBack();
        tRes.tested("toBack()", true);
    }

    /**
    * Creates a simple menu bar and adds to the window. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _setMenuBar() {
        XMenuBar menu = null ;
        boolean result = true ;

        try {
            menu = (XMenuBar) UnoRuntime.queryInterface(XMenuBar.class,
                ((XMultiServiceFactory)tParam.getMSF()).
                createInstance("com.sun.star.awt.MenuBar")) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't instanciate MenuBar service") ;
            result = false ;
        }

        menu.insertItem((short)1, "MenuItem",
            com.sun.star.awt.MenuItemStyle.CHECKABLE, (short)1) ;

        oObj.setMenuBar(menu) ;

        tRes.tested("setMenuBar()", result) ;
    }

    /**
     * Disposes the document created in <code>before</code> method.
     */
    protected void after() {
        aTextDoc.dispose();
    }

    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }
}

