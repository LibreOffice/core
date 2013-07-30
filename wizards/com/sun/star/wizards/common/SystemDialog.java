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
package com.sun.star.wizards.common;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.dialogs.*;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.XStringSubstitution;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.frame.XFrame;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XMessageBox;
import com.sun.star.beans.PropertyValue;

public class SystemDialog
{

    Object systemDialog;
    XFilePicker xFilePicker;
    XFolderPicker2 xFolderPicker;
    XFilterManager xFilterManager;
    XInitialization xInitialize;
    XExecutableDialog xExecutable;
    XComponent xComponent;
    XFilePickerControlAccess xFilePickerControlAccess;
    XMultiServiceFactory xMSF;
    public XStringSubstitution xStringSubstitution;
    public String sStorePath;

    /**
     *
     * @param xMSF
     * @param ServiceName
     * @param type  according to com.sun.star.ui.dialogs.TemplateDescription
     */
    public SystemDialog(XMultiServiceFactory xMSF, String ServiceName, short type)
    {
        try
        {
            this.xMSF = xMSF;
            systemDialog = xMSF.createInstance(ServiceName);
            xFilePicker = UnoRuntime.queryInterface(XFilePicker.class, systemDialog);
            xFolderPicker = UnoRuntime.queryInterface(XFolderPicker2.class, systemDialog);
            xFilterManager = UnoRuntime.queryInterface(XFilterManager.class, systemDialog);
            xInitialize = UnoRuntime.queryInterface(XInitialization.class, systemDialog);
            xExecutable = UnoRuntime.queryInterface(XExecutableDialog.class, systemDialog);
            xComponent = UnoRuntime.queryInterface(XComponent.class, systemDialog);
            xFilePickerControlAccess = UnoRuntime.queryInterface(XFilePickerControlAccess.class, systemDialog);
            xStringSubstitution = createStringSubstitution(xMSF);
            Short[] listAny = new Short[]
            {
                new Short(type)
            };
            if (xInitialize != null)
            {
                xInitialize.initialize(listAny);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace();
        }
    }

    public static SystemDialog createStoreDialog(XMultiServiceFactory xmsf)
    {
        return new SystemDialog(xmsf, "com.sun.star.ui.dialogs.FilePicker", TemplateDescription.FILESAVE_AUTOEXTENSION);
    }

    public static SystemDialog createOpenDialog(XMultiServiceFactory xmsf)
    {
        return new SystemDialog(xmsf, "com.sun.star.ui.dialogs.FilePicker", TemplateDescription.FILEOPEN_SIMPLE);
    }

    public static SystemDialog createFolderDialog(XMultiServiceFactory xmsf)
    {
        return new SystemDialog(xmsf, "com.sun.star.ui.dialogs.FolderPicker", (short) 0);
    }

    public static SystemDialog createOfficeFolderDialog(XMultiServiceFactory xmsf)
    {
        return new SystemDialog(xmsf, "com.sun.star.ui.dialogs.OfficeFolderPicker", (short) 0);
    }

    private String subst(String path)
    {
        try
        {
            return xStringSubstitution.substituteVariables(path, false);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return path;
        }
    }

    /**
     * ATTENTION a BUG : The extension calculated
     * here gives the last 3 chars of the filename - what
     * if the extension is of 4 or more chars?
     *
     * @param DisplayDirectory
     * @param DefaultName
     * @param sDocuType
     * @return
     */
    public String callStoreDialog(String DisplayDirectory, String DefaultName, String sDocuType)
    {
        String sExtension = DefaultName.substring(DefaultName.length() - 3, DefaultName.length());
        addFilterToDialog(sExtension, sDocuType, true);
        return callStoreDialog(DisplayDirectory, DefaultName);
    }

    /**
     * 
     * @param displayDir
     * @param defaultName
     * given url to a local path. 
     * @return
     */
    public String callStoreDialog(String displayDir, String defaultName)
    {
        sStorePath = null;
        try
        {
            xFilePickerControlAccess.setValue(com.sun.star.ui.dialogs.ExtendedFilePickerElementIds.CHECKBOX_AUTOEXTENSION, (short) 0, Boolean.TRUE);
            xFilePicker.setDefaultName(defaultName);
            xFilePicker.setDisplayDirectory(subst(displayDir));
            if (execute(xExecutable))
            {
                String[] sPathList = xFilePicker.getFiles();
                sStorePath = sPathList[0];
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace();
        }
        return sStorePath;
    }

    public String callFolderDialog(String title, String description, String displayDir)
    {
        try
        {
            xFolderPicker.setDisplayDirectory(subst(displayDir));
        }
        catch (com.sun.star.lang.IllegalArgumentException iae)
        {
            iae.printStackTrace();
            throw new IllegalArgumentException(iae.getMessage());
        }
        xFolderPicker.setTitle(title);
        xFolderPicker.setDescription(description);
        if (execute(xFolderPicker))
        {
            return xFolderPicker.getDirectory();
        }
        else
        {
            return null;
        }
    }

    private boolean execute(XExecutableDialog execDialog)
    {
        return execDialog.execute() == 1;
    }

    public String[] callOpenDialog(boolean multiSelect, String displayDirectory)
    {

        try
        {
            xFilePicker.setMultiSelectionMode(multiSelect);
            xFilePicker.setDisplayDirectory(subst(displayDirectory));
            if (execute(xExecutable))
            {
                return xFilePicker.getFiles();
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace();
        }
        return null;
    }

    //("writer_StarOffice_XML_Writer_Template")    'StarOffice XML (Writer)
    public void addFilterToDialog(String sExtension, String filterName, boolean setToDefault)
    {
        try
        {
            //get the localized filtername
            String uiName = getFilterUIName(filterName);
            String pattern = "*." + sExtension;

            //add the filter
            addFilter(uiName, pattern, setToDefault);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void addFilter(String uiName, String pattern, boolean setToDefault)
    {
        try
        {
            xFilterManager.appendFilter(uiName, pattern);
            if (setToDefault)
            {
                xFilterManager.setCurrentFilter(uiName);
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * converts the name returned from getFilterUIName_(...) so the
     * product name is correct.
     * @param filterName
     * @return
     */
    private String getFilterUIName(String filterName)
    {
        String prodName = Configuration.getProductName(xMSF);
        String[][] s = new String[][]
        {
            {
                getFilterUIName_(filterName)
            }
        };
        s[0][0] = JavaTools.replaceSubString(s[0][0], prodName, "%productname%");
        return s[0][0];
    }

    /**
     * note the result should go through conversion of the product name.
     * @param filterName
     * @return the UI localized name of the given filter name.
     */
    private String getFilterUIName_(String filterName)
    {
        try
        {
            Object oFactory = xMSF.createInstance("com.sun.star.document.FilterFactory");
            Object oObject = Helper.getUnoObjectbyName(oFactory, filterName);
            Object oArrayObject = AnyConverter.toArray(oObject);
            PropertyValue[] xPropertyValue = (PropertyValue[]) oArrayObject; //UnoRuntime.queryInterface(XPropertyValue.class, oObject);
            int MaxCount = xPropertyValue.length;
            for (int i = 0; i < MaxCount; i++)
            {
                PropertyValue aValue = xPropertyValue[i];
                if (aValue != null && aValue.Name.equals("UIName"))
                {
                    return AnyConverter.toString(aValue.Value);
                }
            }
            throw new NullPointerException("UIName property not found for Filter " + filterName);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static int showErrorBox(XMultiServiceFactory xMSF, String ResName, String ResPrefix, int ResID, String AddTag, String AddString)
    {
        Resource oResource;
        String ProductName = Configuration.getProductName(xMSF);
        oResource = new Resource(xMSF, ResName, ResPrefix);
        String sErrorMessage = oResource.getResText(ResID);
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, ProductName, "%PRODUCTNAME");
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, String.valueOf((char) 13), "<BR>");
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, AddString, AddTag);
        return SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sErrorMessage);
    }

    public static int showErrorBox(XMultiServiceFactory xMSF, String ResName, String ResPrefix, int ResID)
    {
        Resource oResource;
        String ProductName = Configuration.getProductName(xMSF);
        oResource = new Resource(xMSF, ResName, ResPrefix);
        String sErrorMessage = oResource.getResText(ResID);
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, ProductName, "%PRODUCTNAME");
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, String.valueOf((char) 13), "<BR>");
        return showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sErrorMessage);
    }

    /*
     * example:
     * (xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, "message")
     */
    /**
     * @param windowServiceName one of the following strings:
     * "ErrorBox", "WarningBox", "MessBox", "InfoBox", "QueryBox". 
     * There are other values possible, look
     * under src/toolkit/source/awt/vcltoolkit.cxx
     * @param windowAttribute see com.sun.star.awt.VclWindowPeerAttribute
     * @return 0 = cancel, 1 = ok, 2 = yes,  3 = no(I'm not sure here) 
     * other values check for yourself ;-)
     */
    public static int showMessageBox(XMultiServiceFactory xMSF, String windowServiceName, int windowAttribute, String MessageText)
    {

        short iMessage = 0;
        try
        {
            if (MessageText == null)
            {
                return 0;
            }
            XFrame xFrame = Desktop.getActiveFrame(xMSF);
            XWindowPeer xWindowPeer = UnoRuntime.queryInterface(XWindowPeer.class, xFrame.getComponentWindow());
            return showMessageBox(xMSF, xWindowPeer, windowServiceName, windowAttribute, MessageText);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return iMessage;
    }

    /**
     * just like the other showMessageBox(...) method, but receives a
     * peer argument to use to create the message box.
     * @param xMSF
     * @param peer
     * @param windowServiceName
     * @param windowAttribute
     * @param MessageText
     * @return
     */
    public static int showMessageBox(XMultiServiceFactory xMSF, XWindowPeer peer, String windowServiceName, int windowAttribute, String MessageText)
    {
        // If the peer is null we try to get one from the desktop...
        if (peer == null)
        {
            return showMessageBox(xMSF, windowServiceName, windowAttribute, MessageText);
        }
        short iMessage = 0;
        try
        {
            XInterface xAWTToolkit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
            XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, xAWTToolkit);
            com.sun.star.awt.WindowDescriptor oDescriptor = new com.sun.star.awt.WindowDescriptor();
            oDescriptor.WindowServiceName = windowServiceName;
            oDescriptor.Parent = peer;
            oDescriptor.Type = com.sun.star.awt.WindowClass.MODALTOP;
            oDescriptor.WindowAttributes = windowAttribute;
            XWindowPeer xMsgPeer = xToolkit.createWindow(oDescriptor);
            XMessageBox xMsgbox = UnoRuntime.queryInterface(XMessageBox.class, xMsgPeer);
            XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, xMsgbox);
            xMsgbox.setMessageText(MessageText);
            iMessage = xMsgbox.execute();
            xComponent.dispose();
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace(System.err);
        }
        return iMessage;
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF)
    {
        Object xPathSubst = null;
        try
        {
            xPathSubst = xMSF.createInstance(
                    "com.sun.star.util.PathSubstitution");
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
        if (xPathSubst != null)
        {
            return UnoRuntime.queryInterface(
                    XStringSubstitution.class, xPathSubst);
        }
        else
        {
            return null;
        }
    }
}
