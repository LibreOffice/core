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
import com.sun.star.container.XNameAccess;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.document.XEventsSupplier;
import com.sun.star.document.XTypeDetection;
import com.sun.star.drawing.XDrawPagesSupplier;
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
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCellRange;
import com.sun.star.task.XInteractionHandler;
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
import com.sun.star.frame.XStorable;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;

public class OfficeDocument
{

    private XWindowPeer xWindowPeer;
    private XMultiServiceFactory xMSF;

    /** Creates a new instance of OfficeDocument */
    public OfficeDocument(XMultiServiceFactory _xMSF)
    {
        xMSF = _xMSF;
    }

    public static void attachEventCall(XComponent xComponent, String EventName, String EventType, String EventURL)
    {
        try
        {
            XEventsSupplier xEventssSuppl = UnoRuntime.queryInterface(XEventsSupplier.class, xComponent);
            PropertyValue[] oEventProperties = new PropertyValue[2];
            oEventProperties[0] = new PropertyValue();
            oEventProperties[0].Name = "EventType";
            oEventProperties[0].Value = EventType; // "Service", "StarBasic"
            oEventProperties[1] = new PropertyValue();
            oEventProperties[1].Name = "Script"; //PropertyNames.URL;
            oEventProperties[1].Value = EventURL;
            xEventssSuppl.getEvents().replaceByName(EventName, oEventProperties);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

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

    /**
     * Create a new office document, attached to the given frame.
     * @param frame
     * @param sDocumentType e.g. swriter, scalc, ( simpress, scalc : not tested)
     * @return the document Component (implements XComponent) object ( XTextDocument, or XSpreadsheedDocument )
     */
    public static Object createNewDocument(XFrame frame, String sDocumentType, boolean preview, boolean readonly)
    {

        PropertyValue[] loadValues = new PropertyValue[2];
        loadValues[0] = new PropertyValue();
        loadValues[0].Name = PropertyNames.READ_ONLY;
        loadValues[0].Value = readonly ? Boolean.TRUE : Boolean.FALSE;
        loadValues[1] = new PropertyValue();
        loadValues[1].Name = "Preview";
        loadValues[1].Value = preview ? Boolean.TRUE : Boolean.FALSE;

        Object oDocument = null;
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        XInterface xInterface = null;
        String sURL = "private:factory/" + sDocumentType;

        try
        {
            xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, frame);
            /*if (frame.getName() == null || frame.getName().equals(PropertyNames.EMPTY_STRING));
            frame.setName("T" + System.currentTimeMillis());*/
            XComponent xComponent = xComponentLoader.loadComponentFromURL(sURL, "_self", 0, loadValues);

            if (sDocumentType.equals("swriter"))
            {
                oDocument = UnoRuntime.queryInterface(XTextDocument.class, xComponent);
            }
            else if (sDocumentType.equals("scalc"))
            {
                oDocument = UnoRuntime.queryInterface(XSpreadsheetDocument.class, xComponent);
            //TODO:
            //                else if (sDocumentType == "simpress")
            //                else if (sDocumentType == "sdraw")
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return oDocument;
    }

    public static XFrame createNewFrame(XMultiServiceFactory xMSF, XTerminateListener listener)
    {
        return createNewFrame(xMSF, listener, "_blank");
    }

    public static XFrame createNewFrame(XMultiServiceFactory xMSF, XTerminateListener listener, String FrameName)
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

    public static XFrame createNewPreviewFrame(XMultiServiceFactory xMSF, XTerminateListener listener)
    {
        XToolkit xToolkit = null;
        try
        {
            xToolkit = UnoRuntime.queryInterface(XToolkit.class, xMSF.createInstance("com.sun.star.awt.Toolkit"));
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

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
        XWindowPeer xPeer = null;
        try
        {
            xPeer = UnoRuntime.queryInterface(XWindowPeer.class, xToolkit.createWindow(aDescriptor));
        }
        catch (IllegalArgumentException e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xPeer);

        //define some further properties of the frame window
        //if it's needed .-)
        //xPeer->setBackground(...);

        //create new empty frame and set window on it
        XFrame xFrame = null;
        try
        {
            xFrame = UnoRuntime.queryInterface(XFrame.class, xMSF.createInstance("com.sun.star.frame.Frame"));
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        xFrame.initialize(xWindow);

        //from now this frame is useable ...
        //and not part of the desktop tree.
        //You are alone with him .-)

        if (listener != null)
        {
            Desktop.getDesktop(xMSF).addTerminateListener(listener);
        }

        return xFrame;

    }

    public static Object load(XInterface xInterface, String sURL, String sFrame, PropertyValue[] xValues)
    {
        //        XComponent xComponent = null;
        Object oDocument = null;
        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        //XInterface xInterface = null;
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

    public static boolean store(XMultiServiceFactory xMSF, XComponent xComponent, String StorePath, String FilterName, boolean bStoreToUrl)
    {
        try
        {
            XStorable xStoreable = UnoRuntime.queryInterface(XStorable.class, xComponent);
            PropertyValue[] oStoreProperties;
            if (FilterName.length() > 0)
            {
                oStoreProperties = new PropertyValue[2];
                oStoreProperties[0] = new PropertyValue();
                oStoreProperties[0].Name = "FilterName";
                oStoreProperties[0].Value = FilterName;
                oStoreProperties[1] = new PropertyValue();
                oStoreProperties[1].Name = "InteractionHandler";
                oStoreProperties[1].Value = UnoRuntime.queryInterface(XInteractionHandler.class, xMSF.createInstance("com.sun.star.comp.uui.UUIInteractionHandler"));
            }
            else
            {
                oStoreProperties = new PropertyValue[0];
            }
            if (bStoreToUrl)
            {
                xStoreable.storeToURL(StorePath, oStoreProperties);
            }
            else
            {
                xStoreable.storeAsURL(StorePath, oStoreProperties);
            }
            return true;
        }
        catch (Exception exception)
        {

            exception.printStackTrace(System.err);
            return false;
        }
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

    public static PropertyValue[] getFileMediaDecriptor(XMultiServiceFactory xmsf, String url)
            throws Exception
    {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");

        PropertyValue[][] mediaDescr = new PropertyValue[1][1];
        mediaDescr[0][0] = new PropertyValue();
        mediaDescr[0][0].Name = PropertyNames.URL;
        mediaDescr[0][0].Value = url;

        String type = UnoRuntime.queryInterface(XTypeDetection.class, typeDetect).queryTypeByDescriptor(mediaDescr, true);

        XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, typeDetect);
        if (type.equals(PropertyNames.EMPTY_STRING))
        {
            return null;
        }
        else
        {
            return (PropertyValue[]) xNameAccess.getByName(type);
        }
    }

    public static PropertyValue[] getTypeMediaDescriptor(XMultiServiceFactory xmsf, String type)
            throws Exception
    {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");
        XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, typeDetect);
        return (PropertyValue[]) xNameAccess.getByName(type);
    }

    /**
     * returns the count of slides in a presentation,
     * or the count of pages in a draw document.
     * @param model a presentation or a draw document
     * @return the number of slides/pages in the given document.
     */
    public static int getSlideCount(Object model)
    {
        XDrawPagesSupplier xDrawPagesSupplier = UnoRuntime.queryInterface(XDrawPagesSupplier.class, model);
        return xDrawPagesSupplier.getDrawPages().getCount();
    }

    public static XDocumentProperties getDocumentProperties(Object document)
    {
        XDocumentPropertiesSupplier xDocumentPropertiesSupplier = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, document);
        return xDocumentPropertiesSupplier.getDocumentProperties();
    }

    public static int showMessageBox(XMultiServiceFactory xMSF, String windowServiceName, int windowAttribute, String MessageText)
    {
//      if (getWindowPeer() != null)
        //      return SystemDialog.showMessageBox(xMSF, xWindowPeer, windowServiceName, windowAttribute, MessageText);
//      else
        return SystemDialog.showMessageBox(xMSF, windowServiceName, windowAttribute, MessageText);
    }

    /**
     * @return Returns the xWindowPeer.
     */
    public XWindowPeer getWindowPeer()
    {
        return xWindowPeer;
    }

    /**
     * @param windowPeer The xWindowPeer to set.
     * Should be called as soon as a Windowpeer of a wizard dialog is available
     * The windowpeer is needed to call a Messagebox
     */
    public void setWindowPeer(XWindowPeer windowPeer)
    {
        xWindowPeer = windowPeer;
    }
}
