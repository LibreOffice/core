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
package com.sun.star.wizards.document;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.*;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.sheet.XCellRangeData;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFrames;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;

public class OfficeDocument
{

    public static void dispose(XMultiServiceFactory xMSF, XComponent xComponent)
    {
        try
        {
            if (xComponent != null)
            {
                XModifiable xModified = UnoRuntime.queryInterface(XModifiable.class, xComponent);
                XModel xModel = UnoRuntime.queryInterface(XModel.class, xComponent);
                XFrame xFrame = xModel.getCurrentController().getFrame();
                if (xModified.isModified())
                {
                    xModified.setModified(false);
                }
                Desktop.dispatchURL(xMSF, ".uno:CloseDoc", xFrame);
            }
        }
        catch (PropertyVetoException exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public static XFrame createNewFrame(XMultiServiceFactory xMSF, XTerminateListener listener)
    {
        return createNewFrame(xMSF, listener, "_blank");
    }

    private static XFrame createNewFrame(XMultiServiceFactory xMSF, XTerminateListener listener, String FrameName)
    {
        XFrame xFrame = null;
        if (FrameName.equalsIgnoreCase("WIZARD_LIVE_PREVIEW"))
        {
            xFrame = createNewPreviewFrame(xMSF, listener);
        }
        else
        {
            XFrame xF = UnoRuntime.queryInterface(XFrame.class, Desktop.getDesktop(xMSF));
            xFrame = xF.findFrame(FrameName, 0);
            if (listener != null)
            {
                XFramesSupplier xFS = UnoRuntime.queryInterface(XFramesSupplier.class, xF);
                XFrames xFF = xFS.getFrames();
                xFF.remove(xFrame);
                XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, xF);
                xDesktop.addTerminateListener(listener);
            }
        }
        return xFrame;
    }

    private static XFrame createNewPreviewFrame(XMultiServiceFactory xMSF, XTerminateListener listener)
    {
        XFrame xFrame = null;
        try
        {
            XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, xMSF.createInstance("com.sun.star.awt.Toolkit"));

            //describe the window and its properties
            WindowDescriptor aDescriptor = new WindowDescriptor();
            aDescriptor.Type = com.sun.star.awt.WindowClass.TOP;
            aDescriptor.WindowServiceName = "window";
            aDescriptor.ParentIndex = -1;
            aDescriptor.Parent = null;
            aDescriptor.Bounds = new Rectangle(10, 10, 640, 480);
            aDescriptor.WindowAttributes = WindowAttribute.BORDER |
                WindowAttribute.MOVEABLE |
                WindowAttribute.SIZEABLE |
                //WindowAttribute.CLOSEABLE            |
                VclWindowPeerAttribute.CLIPCHILDREN;

            //create a new blank container window
            XWindowPeer xPeer = UnoRuntime.queryInterface(XWindowPeer.class, xToolkit.createWindow(aDescriptor));
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xPeer);

            //define some further properties of the frame window
            //if it's needed .-)
            //xPeer->setBackground(...);

            //create new empty frame and set window on it
            xFrame = UnoRuntime.queryInterface(XFrame.class, xMSF.createInstance("com.sun.star.frame.Frame"));
            xFrame.initialize(xWindow);

            //from now this frame is useable ...
            //and not part of the desktop tree.
            //You are alone with him .-)

            if (listener != null)
            {
                Desktop.getDesktop(xMSF).addTerminateListener(listener);
            }
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return xFrame;

    }

    public static Object load(XInterface xInterface, String sURL, String sFrame, PropertyValue[] xValues)
    {
        Object oDocument = null;
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        try
        {
            xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, xInterface);
            com.sun.star.lang.XComponent xComponent = xComponentLoader.loadComponentFromURL(sURL, sFrame, 0, xValues);

            XServiceInfo xComponentService = UnoRuntime.queryInterface(XServiceInfo.class, xComponent);
            if (xComponentService.supportsService("com.sun.star.text.TextDocument"))
            {
                oDocument = UnoRuntime.queryInterface(XTextDocument.class, xComponent);            //TODO: write if clauses for Calc, Impress and Draw
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return oDocument;
    }

    public static boolean close(XComponent xComponent)
    {
        boolean bState = false;
        XModel xModel = UnoRuntime.queryInterface(XModel.class, xComponent);

        if (xModel != null)
        {
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, xModel);

            if (xCloseable != null)
            {
                try
                {
                    xCloseable.close(true);
                    bState = true;
                }
                catch (com.sun.star.util.CloseVetoException exCloseVeto)
                {
                    System.out.println("could not close doc");
                    bState = false;
                }
            }
            else
            {
                XComponent xDisposeable = UnoRuntime.queryInterface(XComponent.class, xModel);
                xDisposeable.dispose();
                bState = true;
            }
        }
        return bState;
    }

    public static void ArraytoCellRange(Object[][] datalist, Object oTable, int xpos, int ypos)
    {
        try
        {
            int rowcount = datalist.length;
            if (rowcount > 0)
            {
                int colcount = datalist[0].length;
                if (colcount > 0)
                {
                    XCellRange xCellRange = UnoRuntime.queryInterface(XCellRange.class, oTable);
                    XCellRange xNewRange = xCellRange.getCellRangeByPosition(xpos, ypos, (colcount + xpos) - 1, (rowcount + ypos) - 1);
                    XCellRangeData xDataArray = UnoRuntime.queryInterface(XCellRangeData.class, xNewRange);
                    xDataArray.setDataArray(datalist);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

}
