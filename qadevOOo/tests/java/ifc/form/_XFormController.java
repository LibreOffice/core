/*************************************************************************
 *
 *  $RCSfile: _XFormController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:34:25 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XWindow;
import com.sun.star.form.XFormController;
import com.sun.star.form.XFormControllerListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.form.XFormController</code>
* interface methods :
* <ul>
*  <li><code> getCurrentControl()</code></li>
*  <li><code> addActivateListener()</code></li>
*  <li><code> removeActivateListener()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'otherWindow'</code>
*  (of type <code>com.sun.star.awt.XWindow</code>):
*   The another window is used to activate it, causing deactivating
*   of the component tested. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XFormController
*/
public class _XFormController extends MultiMethodTest {

    public static XFormController oObj = null;
    XWindow otherWind = null;

    /**
     * Listener which determines and stores events occured.
     */
    protected class MyListener implements XFormControllerListener {
        public boolean activated = false ;
        public boolean deactivated = false ;
        public void disposing ( EventObject oEvent ) {}

        public void init() {
            activated = false;
            deactivated = false;
        }

        public void formActivated(EventObject ev) {
            activated = true ;
        }

        public void formDeactivated(EventObject ev) {
            deactivated = true ;
        }
    }

    MyListener listener = new MyListener() ;

    /**
    * Adds a listener, then switches focus between two windows.
    * The current controller must be deactivated and activated.<p>
    *
    * Has <b> OK </b> status if listener <code>deactivate</code>
    * and <code>activate</code> methods was called. <p>
    */
    public void _addActivateListener() {
        requiredMethod("getCurrentControl()");
        oObj.addActivateListener(listener) ;

        XWindow wind = (XWindow)UnoRuntime.queryInterface(XWindow.class, cntrl);
        wind.setFocus();
        shortWait();
        XWindow otherWind = (XWindow)tEnv.getObjRelation("otherWindow");
        otherWind.setFocus();
        shortWait();
        log.println("activated = " + listener.activated +
            ", deactivated = " + listener.deactivated) ;

        tRes.tested("addActivateListener()",
            listener.deactivated && listener.activated) ;
    }

    /**
    * Removes the litener added before, then switches focus between two windows.
    *
    * Has <b> OK </b> status if no listener methods were called. <p>
    */
    public void _removeActivateListener() {
        requiredMethod("addActivateListener()") ;

        oObj.removeActivateListener(listener);
        log.println("ActiveListener removed");
        listener.init();

        XWindow wind = (XWindow)UnoRuntime.queryInterface(XWindow.class, cntrl);
        wind.setFocus();
        shortWait();
        XWindow otherWind = (XWindow)tEnv.getObjRelation("otherWindow");
        otherWind.setFocus();
        shortWait();
        log.println("activated = " + listener.activated +
            ", deactivated = " + listener.deactivated) ;

        tRes.tested("removeActivateListener()",
            !listener.activated && !listener.deactivated);
    }

    XControl cntrl;

    /**
     * Retrieves current control and searches for it among child controls.
     *
     * Has <b>OK</b> status if the current control was found among component
     * children.
     */
    public void _getCurrentControl() {
        cntrl = oObj.getCurrentControl();
        XControl[] children = oObj.getControls() ;

        boolean res = false;
        for(int i = 0; i < children.length; i++) {
            if (children[i].equals(cntrl)) {
                log.println("Current control is equal to the object control" +
                    " #" + i + ":");
                log.println(cntrl);
                res = true;
                break;
            }
        }

        tRes.tested("getCurrentControl()", res) ;
    }

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }
}

