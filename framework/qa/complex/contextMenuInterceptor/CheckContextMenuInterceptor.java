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

package complex.contextMenuInterceptor;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.XContextMenuInterception;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.File;
import util.AccessibilityTools;
import util.DesktopTools;
import util.DrawTools;
import util.SOfficeFactory;
import org.openoffice.test.OfficeFileUrl;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;


/**
 *
 */
public class CheckContextMenuInterceptor
{

    XMultiServiceFactory xMSF = null;
    XFrame xFrame = null;
    Point point = null;
    XWindow xWindow = null;
    com.sun.star.lang.XComponent xDrawDoc;

    @Before
    public void before()
    {
        xMSF = getMSF();
    }

    @After
    public void after() throws Exception
    {
        System.out.println("release the popup menu");
        try
        {
            Robot rob = new Robot();
            int x = point.X;
            int y = point.Y;
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        }
        catch (java.awt.AWTException e)
        {
            System.out.println("couldn't press mouse button");
        }

        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xFrame);

        xClose.close(true);

        xFrame = null;
    }

    @Test
    public void checkContextMenuInterceptor() throws Exception
    {
        System.out.println(" **** Context Menu Interceptor *** ");

        // intialize the test document
        xDrawDoc = DrawTools.createDrawDoc(xMSF);

        SOfficeFactory SOF = SOfficeFactory.getFactory(xMSF);
        XShape oShape = SOF.createShape(xDrawDoc, 5000, 5000, 1500, 1000, "GraphicObject");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc, 0)).add(oShape);

        com.sun.star.frame.XModel xModel =
                UnoRuntime.queryInterface(com.sun.star.frame.XModel.class, xDrawDoc);

        // get the frame for later usage
        xFrame = xModel.getCurrentController().getFrame();

        // ensure that the document content is optimal visible
        DesktopTools.zoomToEntirePage(xMSF, xDrawDoc);

        XBitmap xBitmap = null;

        // adding graphic as ObjRelation for GraphicObjectShape
        XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class, oShape);
        System.out.println("Inserting a shape into the document");

        try
        {
            String sFile = OfficeFileUrl.getAbsolute(new File("space-metal.jpg"));
            oShapeProps.setPropertyValue("GraphicURL", sFile);
            Object oProp = oShapeProps.getPropertyValue("GraphicObjectFillBitmap");
            xBitmap = (XBitmap) AnyConverter.toObject(new Type(XBitmap.class), oProp);
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
        }
        catch (com.sun.star.beans.PropertyVetoException e)
        {
        }
        catch (com.sun.star.beans.UnknownPropertyException e)
        {
        }

        // reuse the frame
        com.sun.star.frame.XController xController = xFrame.getController();
        XContextMenuInterception xContextMenuInterception = null;
        XContextMenuInterceptor xContextMenuInterceptor = null;

        if (xController != null)
        {
            System.out.println("Creating context menu interceptor");

            // add our context menu interceptor
            xContextMenuInterception =
                    UnoRuntime.queryInterface(XContextMenuInterception.class, xController);

            if (xContextMenuInterception != null)
            {
                ContextMenuInterceptor aContextMenuInterceptor = new ContextMenuInterceptor(xBitmap);
                xContextMenuInterceptor =
                        UnoRuntime.queryInterface(XContextMenuInterceptor.class, aContextMenuInterceptor);

                System.out.println("Register context menu interceptor");
                xContextMenuInterception.registerContextMenuInterceptor(xContextMenuInterceptor);
            }
        }

        openContextMenu(UnoRuntime.queryInterface(XModel.class, xDrawDoc));

        checkHelpEntry();

        // remove our context menu interceptor
        if (xContextMenuInterception != null
                && xContextMenuInterceptor != null)
        {
            System.out.println("Release context menu interceptor");
            xContextMenuInterception.releaseContextMenuInterceptor(
                    xContextMenuInterceptor);
        }
    }

    private void checkHelpEntry() throws Exception
    {
        XInterface toolkit = null;

        System.out.println("get accesibility...");
        try
        {
            toolkit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println("could not get Toolkit " + e.toString());
        }
        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, toolkit);

        XAccessible xRoot = null;

        try
        {
            xWindow = UnoRuntime.queryInterface(XWindow.class, tk.getTopWindow(0));

            xRoot = AccessibilityTools.getAccessibleObject(xWindow);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            System.out.println("Couldn't get Window");
        }

        XAccessibleContext oPopMenu = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.POPUP_MENU);

        System.out.println("ImplementationName: " + util.utils.getImplName(oPopMenu));

        XAccessible xHelp = null;
        System.out.println("Try to get first entry of context menu...");
        xHelp = oPopMenu.getAccessibleChild(0);
        if (xHelp == null)
        {
            fail("first entry of context menu is NULL");
        }

        XAccessibleContext xHelpCont = xHelp.getAccessibleContext();
        if (xHelpCont == null)
        {
            fail("No able to retrieve accessible context from first entry of context menu");
        }

        String aAccessibleName = xHelpCont.getAccessibleName();
        if (!aAccessibleName.equals("Help"))
        {
            System.out.println("Accessible name found = " + aAccessibleName);
            fail("First entry of context menu is not from context menu interceptor");
        }

        System.out.println("try to get first children of Help context...");
        xHelpCont.getAccessibleChild(0);
    }

    private void openContextMenu(XModel aModel)
    {

        System.out.println("try to open contex menu...");

        xWindow = AccessibilityTools.getCurrentWindow(aModel);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        XInterface oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);

        point = window.getLocationOnScreen();
        Rectangle rect = window.getBounds();

        System.out.println("click mouse button...");
        try
        {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON3_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON3_MASK);
            System.out.println("done");
        }
        catch (java.awt.AWTException e)
        {
            System.out.println("couldn't press mouse button");
        }

        util.utils.waitForEventIdle(getMSF());
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
