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


package ifc.ui;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.XContextMenuInterception;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import helper.ContextMenuInterceptor;
import java.awt.Robot;
import java.awt.event.InputEvent;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.AccessibilityTools;
import util.DesktopTools;
import util.utils;

public class _XContextMenuInterception extends MultiMethodTest {

    private XModel docModel = null;
    private XContextMenuInterceptor xCI = null;
    public XContextMenuInterception oObj = null;
    private XWindow xWindow = null;
    private XMultiServiceFactory xMSF = null;
    private Point point = null;

    public void before() {
        docModel = UnoRuntime.queryInterface(
                XModel.class,tEnv.getObjRelation("FirstModel"));

        xCI = UnoRuntime.queryInterface(
                XContextMenuInterceptor.class, new ContextMenuInterceptor());

        xMSF = (XMultiServiceFactory)tParam.getMSF();

        //ensure that the first model is focused

        log.println("ensure that the first model is focused");
        DesktopTools.bringWindowToFront(docModel);

        utils.shortWait(3000);
    }

    public void after() {
        if (xCI != null) {
            oObj.releaseContextMenuInterceptor(xCI);
        }
    }

    public void _registerContextMenuInterceptor() {
        oObj.registerContextMenuInterceptor(xCI);
        openContextMenu(docModel);
        boolean res = checkHelpEntry();
        releasePopUp();
        tRes.tested("registerContextMenuInterceptor()",res);
    }

    public void _releaseContextMenuInterceptor() {
        requiredMethod("registerContextMenuInterceptor()");
        oObj.releaseContextMenuInterceptor(xCI);
        openContextMenu(docModel);
        boolean res = checkHelpEntry();
        releasePopUp();
        tRes.tested("releaseContextMenuInterceptor()",!res);
    }

    private boolean checkHelpEntry(){
        XInterface toolkit = null;
        boolean res = true;

        log.println("get accesibility...");
        try{
            toolkit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e){
            log.println("could not get Toolkit " + e.toString());
        }
        XExtendedToolkit tk = UnoRuntime.queryInterface(
                XExtendedToolkit.class, toolkit);

        XAccessible xRoot = null;

        AccessibilityTools at = new AccessibilityTools();

        try {
            xWindow = UnoRuntime.queryInterface(XWindow.class,
                    tk.getTopWindow(0));

            xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get Window");
        }

        XAccessibleContext oPopMenu = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.POPUP_MENU,true);

        log.println("ImplementationName: " + util.utils.getImplName(oPopMenu));

        XAccessible xHelp = null;
        try{
            log.println("Try to get second entry of context menu...");
            xHelp = oPopMenu.getAccessibleChild(1);

        } catch (IndexOutOfBoundsException e){
            throw new StatusException("Not possible to get second entry of context menu",e);
        }

        if (xHelp == null) throw new StatusException(new Status("second entry of context menu is NULL", false));

        XAccessibleContext xHelpCont = xHelp.getAccessibleContext();

        if ( xHelpCont == null )
            throw new StatusException(new Status("No able to retrieve accessible context from first entry of context menu",false));

        String aAccessibleName = xHelpCont.getAccessibleName();
        if ( !aAccessibleName.equals( "Help" )) {
            log.println("Accessible name found = "+aAccessibleName );
            log.println("Second entry of context menu is not from context menu interceptor");
            res=false;
        }

        return res;

    }

    private void openContextMenu(XModel xModel){

        log.println("try to open contex menu...");
        AccessibilityTools at = new AccessibilityTools();

        xWindow = AccessibilityTools.getCurrentWindow(xMSF, xModel);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        XInterface oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = UnoRuntime.queryInterface(
                XAccessibleComponent.class, oObj);

        point = window.getLocationOnScreen();
        Rectangle rect = window.getBounds();

        log.println("klick mouse button...");
        try {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON3_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON3_MASK);
            System.out.println("done");
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        utils.shortWait(1000);

    }

    private void releasePopUp() {
        log.println("release the popup menu");
        try {
            Robot rob = new Robot();
            int x = point.X;
            int y = point.Y;
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }
    }
}
