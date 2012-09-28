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
package ifc.sheet;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XEnhancedMouseClickHandler;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XEnhancedMouseClickBroadcaster;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import lib.MultiMethodTest;
import util.AccessibilityTools;
import util.DesktopTools;
import java.awt.Robot;
import java.awt.event.InputEvent;


public class _XEnhancedMouseClickBroadcaster extends MultiMethodTest {
    public XEnhancedMouseClickBroadcaster oObj;
    protected boolean mousePressed = false;
    protected boolean mouseReleased = false;
    protected XEnhancedMouseClickHandler listener = new MyListener();
    private XModel docModel = null;

    public void before() {
        docModel = UnoRuntime.queryInterface(
                XModel.class,tEnv.getObjRelation("FirstModel"));
        DesktopTools.bringWindowToFront(docModel);
    }

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
        log.println("try to open contex menu...");
        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)tParam.getMSF(), docModel);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XInterface oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = UnoRuntime.queryInterface(
                XAccessibleComponent.class, oObj);

        Point point = window.getLocationOnScreen();
        Rectangle rect = window.getBounds();

        log.println("klick mouse button...");
        try {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2)+50;
            int y = point.Y + (rect.Height / 2)+50;
            rob.mouseMove(x, y);
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON3_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON3_MASK);
            System.out.println("done");
            System.out.println("warte");
            shortWait();
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON1_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
            System.out.println("done "+rob.getAutoDelay());
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }


        return true;
    }

    private void shortWait() {
        try {
            Thread.sleep(2000);
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
