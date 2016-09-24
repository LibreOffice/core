/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.awt.MenuEvent;
import com.sun.star.awt.MenuItemStyle;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.XMenuBar;
import com.sun.star.awt.XMenuListener;
import com.sun.star.awt.XPopupMenu;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class UnoMenu extends UnoDialogSample implements XMenuListener {
private XTopWindow mxTopWindow = null;

public UnoMenu(XComponentContext _xContext, XMultiComponentFactory _xMCF) {
    super(_xContext, _xMCF);
}

    public static void main(String args[]){
        UnoMenu oUnoMenu = null;
        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            oUnoMenu = new UnoMenu(xContext, xMCF);
            oUnoMenu.mxTopWindow = oUnoMenu.showTopWindow( new Rectangle(100, 100, 500, 500));   //oUnoDialogSample.m_xWindowPeer,
            oUnoMenu.addMenuBar(oUnoMenu.mxTopWindow, oUnoMenu);
        }catch( Exception ex ) {
            ex.printStackTrace(System.err);
        }
    }


    public XPopupMenu getPopupMenu(){
    XPopupMenu xPopupMenu = null;
    try{
        // create a popup menu
        Object oPopupMenu = m_xMCF.createInstanceWithContext("com.sun.star.awt.PopupMenu", m_xContext);
        xPopupMenu = UnoRuntime.queryInterface(XPopupMenu.class, oPopupMenu);

        // ID must start be > 0
        short nId = 1;
        short nPos = 0;

        xPopupMenu.insertItem(nId++, "First Entry", MenuItemStyle.AUTOCHECK, nPos++);
        xPopupMenu.insertItem(nId++, "First Radio Entry", (short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), nPos++);
        xPopupMenu.insertItem(nId++, "Second Radio Entry", (short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), nPos++);
        xPopupMenu.insertItem(nId++, "Third RadioEntry",(short) (MenuItemStyle.RADIOCHECK + MenuItemStyle.AUTOCHECK), nPos++);
        xPopupMenu.insertSeparator(nPos++);
        xPopupMenu.insertItem(nId++, "Fifth Entry", (short) (MenuItemStyle.CHECKABLE + MenuItemStyle.AUTOCHECK), nPos++);
        xPopupMenu.insertItem(nId++, "Fourth Entry", (short) (MenuItemStyle.CHECKABLE + MenuItemStyle.AUTOCHECK), nPos++);
        xPopupMenu.insertItem(nId++, "Sixth Entry", (short) 0, nPos++);
        xPopupMenu.insertItem(nId++, "Close Dialog", (short) 0, nPos++);

        xPopupMenu.enableItem((short) 2, false);
        xPopupMenu.checkItem((short) 3, true);

        xPopupMenu.addMenuListener(this);
    }catch( Exception e ) {
        throw new java.lang.RuntimeException("cannot happen...", e);
    }
        return xPopupMenu;
    }


    private void addMenuBar(XTopWindow _xTopWindow, XMenuListener _xMenuListener){
    try{
        // create a menubar at the global MultiComponentFactory...
        Object oMenuBar = m_xMCF.createInstanceWithContext("com.sun.star.awt.MenuBar", m_xContext);
        // add the menu items...
        XMenuBar xMenuBar = UnoRuntime.queryInterface(XMenuBar.class, oMenuBar);
        xMenuBar.insertItem((short) 1, "~First MenuBar Item", com.sun.star.awt.MenuItemStyle.AUTOCHECK, (short) 0);
        xMenuBar.insertItem((short) 2, "~Second MenuBar Item", com.sun.star.awt.MenuItemStyle.AUTOCHECK, (short) 1);
        xMenuBar.setPopupMenu((short) 1, getPopupMenu());
        xMenuBar.addMenuListener(_xMenuListener);
        _xTopWindow.setMenuBar(xMenuBar);
    }catch( Exception e ) {
        throw new java.lang.RuntimeException("cannot happen...", e);
    }}

    protected void closeDialog(){
        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, mxTopWindow);
        if (xComponent != null){
            xComponent.dispose();
        }

        // to ensure that the Java application terminates
        System.exit( 0 );
    }

    private XTopWindow showTopWindow( Rectangle _aRectangle){
    XTopWindow xTopWindow = null;
    try {
        // The Toolkit is the creator of all windows...
        Object oToolkit = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
        XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, oToolkit);

        // set up a window description and create the window. A parent window is always necessary for this...
        com.sun.star.awt.WindowDescriptor aWindowDescriptor = new com.sun.star.awt.WindowDescriptor();
        // a TopWindow is contains a title bar and is able to include menus...
        aWindowDescriptor.Type = WindowClass.TOP;
        // specify the position and height of the window on the parent window
        aWindowDescriptor.Bounds = _aRectangle;
        // set the window attributes...
        aWindowDescriptor.WindowAttributes = WindowAttribute.SHOW + WindowAttribute.MOVEABLE + WindowAttribute.SIZEABLE + WindowAttribute.CLOSEABLE;

        // create the window...
        XWindowPeer xWindowPeer = xToolkit.createWindow(aWindowDescriptor);
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xWindowPeer);

        // create a frame and initialize it with the created window...
        Object oFrame = m_xMCF.createInstanceWithContext("com.sun.star.frame.Frame", m_xContext);
        m_xFrame = UnoRuntime.queryInterface(XFrame.class, oFrame);

        Object oDesktop = m_xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", m_xContext);
        XFramesSupplier xFramesSupplier = UnoRuntime.queryInterface(XFramesSupplier.class, oDesktop);
        m_xFrame.setCreator(xFramesSupplier);
        // get the XTopWindow interface..
        xTopWindow = UnoRuntime.queryInterface(XTopWindow.class, xWindow);
    } catch (com.sun.star.lang.IllegalArgumentException ex) {
        ex.printStackTrace();
    } catch (com.sun.star.uno.Exception ex) {
        ex.printStackTrace();
    }
        return xTopWindow;
    }



    public void itemSelected(MenuEvent menuEvent){
        // find out which menu item has been triggered,
        // by getting the menu-id...
        switch (menuEvent.MenuId){
            case 1:
                // add your menu-item-specific code here:
                break;
            case 2:
                // add your menu-item-specific code here:
                break;
            case 8:
                closeDialog();
            default:

        }
    }

    public void itemHighlighted(MenuEvent menuEvent) {
    }

    public void itemDeactivated(MenuEvent menuEvent) {
    }

    public void itemActivated(MenuEvent menuEvent) {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
