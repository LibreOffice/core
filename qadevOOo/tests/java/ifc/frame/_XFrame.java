/*************************************************************************
 *
 *  $RCSfile: _XFrame.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-10-06 13:30:36 $
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

package ifc.frame;

import com.sun.star.awt.XWindow;
//import com.sun.star.awt.XWindow;
import com.sun.star.frame.FrameAction;
import com.sun.star.frame.FrameActionEvent;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFrameActionListener;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.TestEnvironment;
import util.SOfficeFactory;

/**
* Testing <code>com.sun.star.frame.XFrame</code>
* interface methods:
* <ul>
*  <li><code> getName() </code></li>
*  <li><code> setName() </code></li>
*  <li><code> activate() </code></li>
*  <li><code> deactivate() </code></li>
*  <li><code> isActive() </code></li>
*  <li><code> addFrameActionListener() </code></li>
*  <li><code> removeFrameActionListener() </code></li>
*  <li><code> getCreator() </code></li>
*  <li><code> getComponentWindow() </code></li>
*  <li><code> getContainerWindow() </code></li>
*  <li><code> getController() </code></li>
*  <li><code> isTop() </code></li>
*  <li><code> findFrame() </code></li>
*  <li><code> contextChanged() </code></li>
*  <li><code> setCreator() </code></li>
*  <li><code> setComponent() </code></li>
*  <li><code> initialize() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XFrame'</code> (of type <code>XFrame</code>)
*  <b>optional</b>: any frame named 'XFrame'.
*  Could be used by <code>findFrame</code> method to try
*  to find other frame than itself.</li>
*
*  <li> <code>'Desktop'</code> (of type <code>Object</code>):
*  if exsists, then desktop component is tested</li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XFrame
*/
public class _XFrame extends MultiMethodTest {
    final FrameAction[] actionEvent = new FrameAction[1] ;
    final boolean[] listenerCalled = new boolean[] {false} ;
    final boolean[] activatedCalled = new boolean[] {false} ;
    final boolean[] deactivatedCalled = new boolean[] {false} ;
    final TestFrameActionListener listener =
        new TestFrameActionListener() ;
    public static XFrame oObj = null;

    /**
    * Class used to test listeners.
    */
    private class TestFrameActionListener
        implements XFrameActionListener {

        public void frameAction(FrameActionEvent e) {
            listenerCalled[0] = true ;
            activatedCalled[0] |= e.Action == FrameAction.FRAME_ACTIVATED;
            deactivatedCalled[0] |= e.Action == FrameAction.FRAME_DEACTIVATING;
            actionEvent[0] = e.Action;
        }

        public void disposing(EventObject e) {}

    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null.
    */
    public void _getName() {
        String name = oObj.getName() ;
        if (name == null)
            log.println("getName() returned null: FAILED") ;

        tRes.tested("getName()", name!=null) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if set and gotten names are equal.
    */
    public void _setName() {
        String sName = "XFrame" ;

        oObj.setName(sName);
        String gName = oObj.getName();
        boolean res = sName.equals(gName);
        if (! res)
            log.println("setName('" + sName +
                "'), but getName() return '" + gName + "'") ;
        tRes.tested("setName()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _activate() {
        oObj.activate() ;
        tRes.tested("activate()", true) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _deactivate() {
        oObj.deactivate() ;
        oObj.activate() ;
        tRes.tested("deactivate()", true) ;
    }

    /**
    * Test calls the method. Then frame is deactivated and method called
    * again. <p>
    * Has <b> OK </b> status if isDesktop() returns true or if the method
    * always display real status of a frame during activation/deactivation.
    */
    public void _isActive() {
        boolean result = true;

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop is always active");
            tRes.tested("isActive()", oObj.isActive()) ;
            return;
        }

        oObj.deactivate();
        result &= !oObj.isActive();
        if (oObj.isActive())
            log.println("after deactivate() method call, isActive() returned true");
        oObj.activate();
        result &= oObj.isActive();
        if (!oObj.isActive())
            log.println("after activate() method call, isActive() returned false") ;
        boolean res = isDesktop(log,tEnv,"isActive()");
        if (res) result=res;

        tRes.tested("isActive()", result) ;
    }

    /**
    * Test calls the method. Then frame status (activated/deactivated) is
    * changed, and the listener is checked.<p>
    * Has <b> OK </b> status if isDesktop() method returnes true, or if the
    * listener was called and frame was activated.
    */
    public void _addFrameActionListener() {
        boolean result = true ;

        oObj.addFrameActionListener(listener) ;
        oObj.activate() ;
        oObj.deactivate() ;
        oObj.activate() ;

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("No actions supported by Desktop");
            tRes.tested("addFrameActionListener()", true) ;
            return;
        }

        try {
            Thread.sleep(500);
        }catch (InterruptedException ex) {}


        if (!listenerCalled[0]) {
            log.println("listener was not called.") ;
            result = false ;
        } else {
            if (!activatedCalled[0]) {
                log.println("Listener was called, FRAME_ACTIVATED was not") ;
                result = false ;
            }
            if (!deactivatedCalled[0]) {
                log.println("Listener was called, FRAME_DEACTIVATING was not") ;
                result = false ;
            }
        }

        boolean res = isDesktop(log, tEnv, "addFrameActionListener()");
        if (res) result=res;

        tRes.tested("addFrameActionListener()", result) ;
    }

    /**
    * Test calls the method.  Then frame status (activated/deactivated) is
    * changed, and the listener is checked.<p>
    * Has <b> OK </b> status if isDesktop() method returns true, or if the
    * method actually removes listener so it does not react on
    * activate/deactivate events. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addFrameActionListener() </code>: adds action listener
    * to a frame </li>
    * </ul>
    */
    public void _removeFrameActionListener() {
        boolean result = true;

        requiredMethod("addFrameActionListener()");
        listenerCalled[0] = false;
        oObj.removeFrameActionListener(listener);
        oObj.activate();
        oObj.deactivate();
        oObj.activate();
        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("No actions supported by Desktop");
            tRes.tested("removeFrameActionListener()", true) ;
            return;
        }

        if (listenerCalled[0])
            log.println("Listener wasn't removed, and was called");
        boolean res = isDesktop(log, tEnv, "removeFrameActionListener()");
        if (res) result=res; else result = (!listenerCalled[0]);

        tRes.tested("removeFrameActionListener()", result);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if isDesktop() method returns true or if the method
    * does not return null.
    */
    public void _getCreator() {
        boolean result = true;

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop has no creator");
            tRes.tested("getCreator()", true) ;
            return;
        }

        XFramesSupplier creator = oObj.getCreator() ;
        if (creator == null)
            log.println("getCreator() returns null") ;
        boolean res = isDesktop(log,tEnv,"getCreator()");
        if (res) result=res; else result = (creator != null);
        tRes.tested("getCreator()", result) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if isDesktop() method returns true or if the method
    * does not return null.
    */
    public void _getComponentWindow() {
        boolean result = true;

        XWindow win = oObj.getComponentWindow() ;

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop has no component window");
            tRes.tested("getComponentWindow()", true) ;
            return;
        }

        if (win == null)
            log.println("getComponentWindow() returns null") ;
        boolean res = isDesktop(log,tEnv,"getComponentWindow()");
        if (res) result=res; else result = (win != null);
        tRes.tested("getComponentWindow()", result) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if isDesktop() method returns true or if the method
    * does not return null.
    */
    public void _getContainerWindow() {
        boolean result = true;

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop has no container window");
            tRes.tested("getContainerWindow()", true) ;
            return;
        }

        XWindow win = oObj.getContainerWindow() ;
        if (win == null)
            log.println("getContainerWindow() returns null") ;
        boolean res = isDesktop(log,tEnv,"getContainerWindow()");
        if (res) result=res; else result = (win != null);
        tRes.tested("getContainerWindow()", result) ;
    }

    /**
    * Test calls the method. Then returned controller is checked. <p>
    * Has <b> OK </b> status if isDesktop() method returns true or
    * if the method returns non-null controller, having frame that's equal to
    * a (XFrame) oObj.
    */
    public void _getController() {
        boolean result = true;
        XController ctrl = oObj.getController();

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop has no controller");
            tRes.tested("getController()", true) ;
            return;
        }

        if (ctrl == null) {
            log.println("getController() returns null");
            result = false;
        } else {
            XFrame frm = ctrl.getFrame();
            if (!oObj.equals(frm)) {
                log.println("Frame returned by controller not " +
                    "equals to frame testing");
                result = false;
            }
        }
        boolean res = isDesktop(log, tEnv, "getController()");
        if (res) result=res;
        tRes.tested("getController()", result) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _isTop() {
        log.println("isTop() = " + oObj.isTop());
        tRes.tested("isTop()", true) ;
    }

    /**
    * After obtaining an object relation 'XFrame', test tries to find a frame
    * named 'XFrame'. <p>
    * Has <b> OK </b> status if the method returns non-null object that's equal
    * to previously obtained object relation.
    */
    public void _findFrame() {
        boolean result = true ;

        XFrame aFrame = (XFrame) tEnv.getObjRelation("XFrame");

        if (aFrame != null) {
            log.println("Trying to find a frame with name 'XFrame' ...");
            XFrame frame = oObj.findFrame("XFrame",
                com.sun.star.frame.FrameSearchFlag.GLOBAL) ;
            if (frame == null) {
                log.println("findFrame(\"XFrame,com.sun.star.frame.FrameSearchFlag.GLOBAL\") returns null") ;
                result = false ;
            } else if ( !aFrame.equals(frame) ) {
                log.println("findFrame(\"XFrame,com.sun.star.frame.FrameSearchFlag.GLOBAL\") "
                    + " returns frame which is not equal to passed in relation") ;
                result = false ;
            }
        }

        log.println("Trying to find a frame with name '_self' ...");
        XFrame frame = oObj.findFrame("_self",
            com.sun.star.frame.FrameSearchFlag.AUTO) ;
        if (frame == null) {
            log.println("findFrame(\"_self\") returns null") ;
            result = false ;
        } else if ( !oObj.equals(frame) ) {
            log.println("findFrame(\"_self\") "
                + " returns frame which is not equal to tested") ;
            result = false ;
        }

        tRes.tested("findFrame()", result) ;
    }

    /**
    * At first new listener is added, then test calls the method and result
    * is checked. <p>
    * Has <b> OK </b> status if isDesktop() method returnes true or if the
    * listener was called and proper event past to listener.
    */
    public void _contextChanged() {
        boolean result = true;
        TestFrameActionListener listener = new TestFrameActionListener();

        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop cann't change context");
            tRes.tested("contextChanged()", true) ;
            return;
        }

        listenerCalled[0] = false;
        oObj.addFrameActionListener(listener);
        try {
            oObj.contextChanged();
            if ( !listenerCalled[0] ) {
                log.println("listener was not called on contextChanged() call.") ;
                result = false;
            } else if (actionEvent[0] != FrameAction.CONTEXT_CHANGED) {
                log.println("listener was called, but Action != CONTEXT_CHANGED") ;
                result = false;
            }
        } finally {
            oObj.removeFrameActionListener(listener);
        }

        boolean res = isDesktop(log, tEnv, "contextChanged()");
        if (res) result = res;
        tRes.tested("contextChanged()", result);
    }


    /**
    * Test calls the method. Remembered old creater is restored at the end. <p>
    * Has <b> OK </b> status if the method sucessfully set new value to (XFrame)
    * oObj object.
    */
    public void _setCreator() {
        if (tEnv.getTestCase().getObjectName().equals("Desktop")) {
            log.println("Desktop has no creator");
            tRes.tested("setCreator()", true) ;
            return;
        }

        XFramesSupplier oldCreator = oObj.getCreator() ;
        oObj.setCreator(null) ;
        tRes.tested("setCreator()", oObj.getCreator() == null) ;
        oObj.setCreator(oldCreator) ;
    }

    /**
    * Test calls the method, then result is checked.<p>
    * Has <b> OK </b> status if method returns true, and values, set by the
    * method are nulls, or if method returns false, and values are not changed.
    * This method destroy the object. Therfore all other methods have to be
    * executed before :
    * <ul>
    *  <li> <code> getName() </code>
    *  <li> <code> setName() </code>
    *  <li> <code> activate() </code>
    *  <li> <code> deactivate() </code>
    *  <li> <code> isActive() </code>
    *  <li> <code> addFrameActionListener() </code>
    *  <li> <code> getComponentWindow() </code>
    *  <li> <code> getContainerWindow() </code>
    *  <li> <code> getController() </code>
    *  <li> <code> isTop() </code>
    *  <li> <code> findFrame() </code>
    *  <li> <code> contextChanged() </code>
    *  <li> <code> setCreator() </code>
    *  object</li>
    * </ul>
    */
    public void _setComponent() {
        // setComponent() destr
        requiredMethod("getName()") ;
        requiredMethod("setName()") ;
        requiredMethod("activate()") ;
        requiredMethod("deactivate()") ;
        requiredMethod("isActive()") ;
        requiredMethod("addFrameActionListener()") ;
        requiredMethod("getComponentWindow()") ;
        requiredMethod("getContainerWindow()") ;
        requiredMethod("getController()") ;
        requiredMethod("isTop()") ;
        requiredMethod("findFrame()") ;
        requiredMethod("contextChanged()") ;
        requiredMethod("setCreator()") ;

        boolean result = true;

        XWindow oldWindow = oObj.getComponentWindow();
        XController oldController = oObj.getController();
        boolean rs = oObj.setComponent(null, null);
        if (rs) {  // component must be changed
            result &= oObj.getComponentWindow() == null;
            result &= oObj.getController() == null;
            if (!result)
                log.println("setComponent() returns true, but component is " +
                 "not changed.");
        } else {   // frame is not allowed to change component
            result &= oObj.getComponentWindow() == oldWindow ;
            result &= oObj.getController() == oldController ;
            if (!result)
                log.println("setComponent() returns false, but component is" +
                 "changed.");
        }
        tRes.tested("setComponent()", result);

    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setComponent() </code> : sets window and controller to the
    *  object</li>
    * </ul>
    */
    public void _initialize() {
        requiredMethod("setComponent()") ;
        XWindow win = oObj.getContainerWindow() ;
        oObj.initialize(win) ;
        tRes.tested("initialize()", true) ;
    }

    /**
    * Checks does relation 'Desktop' exist. Returns true if exist.
    */
    public static boolean isDesktop(PrintWriter log,
            TestEnvironment env, String method) {
        Object isD = env.getObjRelation("Desktop");
        if (isD != null) {
            log.println("The Desktop doesn't support the method " + method);
            log.println("It will always return true");
            return true;
        }
        else {
            return false;
        }
    }
}
