/*************************************************************************
 *
 *  $RCSfile: _XEnhancedMouseClickBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2005-01-13 17:40:27 $
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
package ifc.sheet;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XEnhancedMouseClickHandler;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XEnhancedMouseClickBroadcaster;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;
import lib.StatusException;

import util.AccessibilityTools;
import util.utils;

import java.awt.Robot;
import java.awt.event.InputEvent;


public class _XEnhancedMouseClickBroadcaster extends MultiMethodTest {
    public XEnhancedMouseClickBroadcaster oObj;
    protected boolean mousePressed = false;
    protected boolean mouseReleased = false;
    protected XEnhancedMouseClickHandler listener = new MyListener();

    public void _addEnhancedMouseClickHandler() {
        oObj.addEnhancedMouseClickHandler(listener);
        clickOnSheet();

        //make sure that the listener is removed even if the test fails
        if ((!mousePressed) || (!mouseReleased)) {
            oObj.removeEnhancedMouseClickHandler(listener);
        }

        tRes.tested("addEnhancedMouseClickHandler()",
            mousePressed && mouseReleased);
    }

    public void _removeEnhancedMouseClickHandler() {
        requiredMethod("addEnhancedMouseClickHandler()");
        mousePressed = false;
        mouseReleased = false;
        oObj.removeEnhancedMouseClickHandler(listener);
        clickOnSheet();
        tRes.tested("removeEnhancedMouseClickHandler()",
            (!mousePressed) && (!mouseReleased));
    }

    protected boolean clickOnSheet() {
        log.println("Trying to get AccessibleSpreadsheet");

        AccessibilityTools at = new AccessibilityTools();
        XComponent xSheetDoc =
            (XComponent) tEnv.getObjRelation("DOCUMENT");
        XModel xModel =
            (XModel) UnoRuntime.queryInterface(XModel.class, xSheetDoc);
        XWindow xWindow =
            at.getCurrentWindow((XMultiServiceFactory) tParam.getMSF(),
                xModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.TABLE);

        XAccessibleComponent AccessibleSpreadsheet =
            (XAccessibleComponent) UnoRuntime.queryInterface(XAccessibleComponent.class,
                AccessibilityTools.SearchedContext);

        log.println("Got " + utils.getImplName(AccessibleSpreadsheet));

        Object toolkit = null;

        try {
            toolkit =
                ((XMultiServiceFactory) tParam.getMSF())
                .createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk =
            (XExtendedToolkit) UnoRuntime.queryInterface(XExtendedToolkit.class,
                toolkit);

        XTopWindow tw = null;

        int k = tk.getTopWindowCount();

        for (int i = 0; i < k; i++) {
            try {
                XTopWindow tw_temp = tk.getTopWindow(i);
                XAccessible xacc =
                    (XAccessible) UnoRuntime.queryInterface(XAccessible.class,
                        tw_temp);

                if (xacc != null) {
                    if (xacc.getAccessibleContext().getAccessibleName()
                                .indexOf("d2") > 0) {
                        tw = tw_temp;
                    }

                    ;
                } else {
                    log.println("\t unknown window");
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
        }

        if (tw == null) {
            System.out.println("No TopWindow :-(");

            return false;
        }

        tw.toFront();

        Point point = AccessibleSpreadsheet.getLocationOnScreen();
        Rectangle rect = AccessibleSpreadsheet.getBounds();

        log.println(
            "Clicking in the center of the AccessibleSpreadsheet");

        try {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            shortWait();
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
            shortWait();
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        return true;
    }

    private void shortWait() {
        try {
            Thread.currentThread().sleep(500);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    protected class MyListener implements XEnhancedMouseClickHandler {
        public void disposing(
            com.sun.star.lang.EventObject eventObject) {
        }

        public boolean mousePressed(
            com.sun.star.awt.EnhancedMouseEvent enhancedMouseEvent) {
            log.println("mousePressed");
            mousePressed = true;

            return true;
        }

        public boolean mouseReleased(
            com.sun.star.awt.EnhancedMouseEvent enhancedMouseEvent) {
            log.println("mouseReleased");
            mouseReleased = true;

            return true;
        }
    }
}
