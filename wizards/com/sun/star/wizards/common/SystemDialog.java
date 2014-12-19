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
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XFrame;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XMessageBox;

public class SystemDialog
{

    public static int showErrorBox(XMultiServiceFactory xMSF, String ResPrefix, int ResID, String AddTag, String AddString)
    {
        Resource oResource;
        String ProductName = Configuration.getProductName(xMSF);
        oResource = new Resource(xMSF, ResPrefix);
        String sErrorMessage = oResource.getResText(ResID);
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, ProductName, "%PRODUCTNAME");
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, String.valueOf((char) 13), "<BR>");
        sErrorMessage = JavaTools.replaceSubString(sErrorMessage, AddString, AddTag);
        return SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sErrorMessage);
    }

    public static int showErrorBox(XMultiServiceFactory xMSF, String ResPrefix, int ResID)
    {
        Resource oResource;
        String ProductName = Configuration.getProductName(xMSF);
        oResource = new Resource(xMSF, ResPrefix);
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

}
