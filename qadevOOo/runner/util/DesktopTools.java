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
package util;

import helper.ConfigHelper;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.ArrayList;

import lib.StatusException;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.adapter.ByteArrayToXInputStreamAdapter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;
import com.sun.star.view.XViewSettingsSupplier;

/**
 * contains helper methods for the Desktop
 */
public class DesktopTools
{

    /**
     * Queries the XComponentLoader
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained XComponentLoader
     */
    private static XComponentLoader getCLoader(XMultiServiceFactory xMSF)
    {
        XComponentLoader oCLoader = UnoRuntime.queryInterface(
                XComponentLoader.class, createDesktop(xMSF));

        return oCLoader;
    }

    /**
     * Creates an Instance of the Desktop service
     *
     * @param xMSF the MultiServiceFactory
     * @return the gained XDesktop object
     */
    public static XDesktop createDesktop(XMultiServiceFactory xMSF)
    {
        XDesktop xDesktop;

        try
        {
            xDesktop = UnoRuntime.queryInterface(
                XDesktop.class, xMSF.createInstance("com.sun.star.comp.framework.Desktop"));
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new IllegalArgumentException("Desktop Service not available", e);
        }

        return xDesktop;
    }

    /**
     * returns a XEnumeration containing all components containing on the desktop
     * @param xMSF the XMultiServiceFactory
     * @return XEnumeration of all components on the desktop
     */
    public static XEnumeration getAllComponents(XMultiServiceFactory xMSF)
    {
        return createDesktop(xMSF).getComponents().createEnumeration();
    }



    /**
     * returns the current component on the desktop
     * @param xMSF the XMultiServiceFactory
     * @return XComponent of the current component on the desktop
     */
    public static XFrame getCurrentFrame(XMultiServiceFactory xMSF)
    {
        return createDesktop(xMSF).getCurrentFrame();
    }

    /**
     * returns an object array of all open documents
     * @param xMSF the MultiServiceFactory
     * @return returns an Array of document kinds like ["swriter"]
     */
    public static Object[] getAllOpenDocuments(XMultiServiceFactory xMSF)
    {
        ArrayList<XComponent> components = new ArrayList<XComponent>();

        XEnumeration allComp = getAllComponents(xMSF);

        while (allComp.hasMoreElements())
        {
            try
            {
                XComponent xComponent = UnoRuntime.queryInterface(
                        XComponent.class, allComp.nextElement());

                if (getDocumentType(xComponent) != null)
                {
                    components.add(xComponent);
                }

            }
            catch (com.sun.star.container.NoSuchElementException e)
            {
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
            }
        }
        return components.toArray();
    }

    /**
     * Returns the document type for the given XComponent of an document
     * @param xComponent the document to query for its type
     * @return possible:
     * <ul>
     * <li>swriter</li>
     * <li>scalc</li>
     * <li>sdraw</li>
     * <li>smath</li>
     * </ul>
     * or <CODE>null</CODE>
     */
    private static String getDocumentType(XComponent xComponent)
    {
        XServiceInfo sInfo = UnoRuntime.queryInterface(
                XServiceInfo.class, xComponent);

        if (sInfo == null)
        {
            return "";
        }
        else if (sInfo.supportsService("com.sun.star.sheet.SpreadsheetDocument"))
        {
            return "scalc";
        }
        else if (sInfo.supportsService("com.sun.star.text.TextDocument"))
        {
            return "swriter";
        }
        else if (sInfo.supportsService("com.sun.star.drawing.DrawingDocument"))
        {
            return "sdraw";
        }
        else if (sInfo.supportsService("com.sun.star.presentation.PresentationDocument"))
        {
            return "simpress";
        }
        else if (sInfo.supportsService("com.sun.star.formula.FormulaProperties"))
        {
            return "smath";
        }
        else
        {
            return null;
        }
    }

    /**
     * Opens a new document of a given kind
     * with arguments
     * @return the XComponent Interface of the document
     * @param kind the kind of document to load.<br>
     * possible:
     * <ul>
     * <li>swriter</li>
     * <li>scalc</li>
     * <li>sdaw</li>
     * <li>smath</li>
     * </ul>
     * @param Args arguments which passed to the document to load
     * @param xMSF the MultiServiceFactory
     */
    public static XComponent openNewDoc(XMultiServiceFactory xMSF, String kind,
            PropertyValue[] Args)
    {
        XComponent oDoc = null;

        try
        {
            oDoc = getCLoader(xMSF).loadComponentFromURL("private:factory/" + kind,
                    "_blank", 0, Args);
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new IllegalArgumentException("Document could not be opened", e);
        }

        return oDoc;
    }

    /**
     * loads a document of from a given url
     * with arguments
     * @return the XComponent Interface of the document
     * @param url the URL of the document to load.
     * @param Args arguments which passed to the document to load
     * @param xMSF the MultiServiceFactory
     */
    public static XComponent loadDoc(XMultiServiceFactory xMSF, String url,
            PropertyValue[] Args)
    {
        XComponent oDoc = null;
        if (Args == null)
        {
            Args = new PropertyValue[0];
        }
        try
        {
            oDoc = getCLoader(xMSF).loadComponentFromURL(url, "_blank", 0, Args);
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new IllegalArgumentException("Document could not be loaded", e);
        }

        bringWindowToFront(oDoc);
        return oDoc;
    }

    /**
     * loads a document of from a given path using an input stream
     *
     * @param xMSF the MultiServiceFactory
     * @param filePath the path of the document to load.
     * @return the XComponent Interface of the document
     */
    public static XComponent loadDocUsingStream(XMultiServiceFactory xMSF, String filePath)
    {
        XInputStream inputStream = null;
        try {
            final InputStream inputFile = new BufferedInputStream(
                    new FileInputStream(filePath));
            final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
            final byte[] byteBuffer = new byte[4096];
            int byteBufferLength = 0;
            while ((byteBufferLength = inputFile.read(byteBuffer)) > 0)
                bytes.write(byteBuffer, 0, byteBufferLength);
            inputFile.close();
            inputStream = new ByteArrayToXInputStreamAdapter(
                    bytes.toByteArray());
        } catch (java.io.IOException e) {
            e.printStackTrace();
        }

        PropertyValue[] loadProps = new PropertyValue[1];
        loadProps[0] = new PropertyValue();
        loadProps[0].Name = "InputStream";
        loadProps[0].Value = inputStream;

        XComponent oDoc = null;
        try
        {
            oDoc = getCLoader(xMSF).loadComponentFromURL("private:stream", "_blank", 0, loadProps);
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new IllegalArgumentException("Document could not be loaded", e);
        }
        return oDoc;
    }

    /**
     * closes a given document
     * @param DocumentToClose the document to close
     */
    public static void closeDoc(XInterface DocumentToClose)
    {
        if (DocumentToClose == null)
        {
            return;
        }

        String kd = System.getProperty("KeepDocument");
        if (kd != null)
        {
            System.out.println("The property 'KeepDocument' is set and so the document won't be disposed");
            return;
        }
        XModifiable modified = UnoRuntime.queryInterface(XModifiable.class, DocumentToClose);
        XCloseable closer = UnoRuntime.queryInterface(XCloseable.class, DocumentToClose);

        try
        {
            if (modified != null)
            {
                modified.setModified(false);
            }
            closer.close(true);
        }
        catch (com.sun.star.util.CloseVetoException e)
        {
            System.out.println("Couldn't close document");
        }
        catch (com.sun.star.lang.DisposedException e)
        {
            System.out.println("Couldn't close document");
        }
        catch (NullPointerException e)
        {
            System.out.println("Couldn't close document");
        }
        catch (com.sun.star.beans.PropertyVetoException e)
        {
            System.out.println("Couldn't close document");
        }
    }

    /**
     * Creates a floating XWindow with the size of X=500 Y=100 width=400 height=600
     * @param xMSF the MultiServiceFactory
     * @throws lib.StatusException if it is not possible to create a floating window a lib.StatusException was thrown
     * @return a floating XWindow
     */
    public static XWindowPeer createFloatingWindow(XMultiServiceFactory xMSF)
            throws StatusException
    {
        return createFloatingWindow(xMSF, 500, 100, 400, 600);
    }

    /**
     * Creates a floating XWindow on the given position and size.
     * @return a floating XWindow
     * @param X the X-Position of the floating XWindow
     * @param Y the Y-Position of the floating XWindow
     * @param width the width of the floating XWindow
     * @param height the height of the floating XWindow
     * @param xMSF the MultiServiceFactory
     * @throws lib.StatusException if it is not possible to create a floating window a lib.StatusException was thrown
     */
    public static XWindowPeer createFloatingWindow(XMultiServiceFactory xMSF, int X, int Y, int width, int height)
            throws StatusException
    {

        XInterface oObj = null;

        try
        {
            oObj = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
        }
        catch (com.sun.star.uno.Exception e)
        {
            throw new StatusException("Couldn't get toolkit", e);
        }

        XToolkit tk = UnoRuntime.queryInterface(
                XToolkit.class, oObj);

        WindowDescriptor descriptor = new com.sun.star.awt.WindowDescriptor();

        descriptor.Type = com.sun.star.awt.WindowClass.TOP;
        descriptor.WindowServiceName = "modelessdialog";
        descriptor.ParentIndex = -1;

        Rectangle bounds = new com.sun.star.awt.Rectangle();
        bounds.X = X;
        bounds.Y = Y;
        bounds.Width = width;
        bounds.Height = height;

        descriptor.Bounds = bounds;
        descriptor.WindowAttributes = (com.sun.star.awt.WindowAttribute.BORDER +
                com.sun.star.awt.WindowAttribute.MOVEABLE +
                com.sun.star.awt.WindowAttribute.SIZEABLE +
                com.sun.star.awt.WindowAttribute.CLOSEABLE +
                com.sun.star.awt.VclWindowPeerAttribute.CLIPCHILDREN);

        XWindowPeer xWindow = null;

        try
        {
            xWindow = tk.createWindow(descriptor);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            throw new StatusException("Could not create window", e);
        }

        return xWindow;

    }

    /**
     * zoom to have a view over the hole page
     * @param xDoc the document to zoom
     */
    public static void zoomToEntirePage(XMultiServiceFactory xMSF, XInterface xDoc)
    {
        try
        {
            XModel xMod = UnoRuntime.queryInterface(XModel.class, xDoc);
            XInterface oCont = xMod.getCurrentController();
            XViewSettingsSupplier oVSSupp = UnoRuntime.queryInterface(XViewSettingsSupplier.class, oCont);

            XInterface oViewSettings = oVSSupp.getViewSettings();
            XPropertySet oViewProp = UnoRuntime.queryInterface(XPropertySet.class, oViewSettings);
            oViewProp.setPropertyValue("ZoomType",
                    Short.valueOf(com.sun.star.view.DocumentZoomType.ENTIRE_PAGE));

            util.utils.waitForEventIdle(xMSF);
        }
        catch (Exception e)
        {
            System.out.println("Could not zoom to entire page: " + e.toString());
        }

    }

    /**
     * This function docks the Navigator onto the right side of the window.</p>
     * Note:<P>
     * Since the svt.viewoptions cache the view configuration at start up
     * the change of the docking will be effective at a restart.
     * @param xMSF the XMultiServiceFactory
     */
    public static void dockNavigator(XMultiServiceFactory xMSF)
    {
        // prepare Window settings
        try
        {
            ConfigHelper aConfig = new ConfigHelper(xMSF,
                    "org.openoffice.Office.Views", false);

            aConfig.getOrInsertGroup("Windows", "10366");

            aConfig.updateGroupProperty(
                    "Windows", "10366", "WindowState", "952,180,244,349;1;0,0,0,0;");

            aConfig.insertOrUpdateExtensibleGroupProperty(
                    "Windows", "10366", "UserData", "Data", "V2,V,0,AL:(5,16,0/0/244/349,244;610)");

            // Is node "SplitWindow2" available? If not, insert it.
            aConfig.getOrInsertGroup("Windows", "SplitWindow2");

            aConfig.insertOrUpdateExtensibleGroupProperty(
                    "Windows", "SplitWindow2", "UserData", "UserItem", "V1,2,1,0,10366");

            aConfig.flush();
            aConfig = null;

        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
    }


    /**
     * This function brings a document to the front.<P>
     * NOTE: it is not possible to change the window order of your Window-Manager!!
     * Only the order of Office documents are changeable.
     * @param xModel the XModel of the document to bring to top
     */
    public static void bringWindowToFront(XModel xModel)
    {
        XTopWindow xTopWindow =
                UnoRuntime.queryInterface(
        XTopWindow.class,
        xModel.getCurrentController().getFrame().getContainerWindow());

        xTopWindow.toFront();
    }

    public static void bringWindowToFront(XComponent xComponent)
    {
        XModel xModel = UnoRuntime.queryInterface(XModel.class, xComponent);
        if (xModel != null)
        {
            bringWindowToFront(xModel);
        }
    }
}
