/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XContextMenuInterception.java,v $
 * $Revision: 1.3.8.1 $
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
        docModel = (XModel) UnoRuntime.queryInterface(
                XModel.class,tEnv.getObjRelation("FirstModel"));

        xCI = (XContextMenuInterceptor) UnoRuntime.queryInterface(
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
        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                XExtendedToolkit.class, toolkit);

        XAccessible xRoot = null;

        AccessibilityTools at = new AccessibilityTools();

        try {
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                    tk.getTopWindow(0));

            xRoot = at.getAccessibleObject(xWindow);
            at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get Window");
        }

        XAccessibleContext oPopMenu = at.getAccessibleObjectForRole(xRoot, AccessibleRole.POPUP_MENU,true);

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

        xWindow = at.getCurrentWindow(xMSF, xModel);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XInterface oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = (XAccessibleComponent) UnoRuntime.queryInterface(
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
