/*************************************************************************
 *
 *  $RCSfile: _XTopWindow.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:14:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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

