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

import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class UnoMenu2 extends UnoMenu implements XMouseListener{

public UnoMenu2(XComponentContext _xContext, XMultiComponentFactory _xMCF) {
    super(_xContext, _xMCF);
}

    public static void main(String args[]){
        UnoMenu2 oUnoMenu2 = null;
        try {
        XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
        if(xContext != null )
            System.out.println("Connected to a running office ...");
        XMultiComponentFactory xMCF = xContext.getServiceManager();
        oUnoMenu2 = new UnoMenu2(xContext, xMCF);
        oUnoMenu2.initialize( new String[] {"Height", "Moveable", "Name","PositionX","PositionY", "Step", "TabIndex","Title","Width"},
                                    new Object[] { new Integer(140), Boolean.TRUE, "Dialog1", new Integer(102),new Integer(41), new Integer(1), new Short((short) 0), "Menu-Dialog", new Integer(200)});

        Object oFTHeaderModel = oUnoMenu2.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
        XMultiPropertySet xFTHeaderModelMPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
        xFTHeaderModelMPSet.setPropertyValues(
            new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
            new Object[] { new Integer(8), "This code-sample demonstrates the creation of a popup-menu", "HeaderLabel", new Integer(6), new Integer(6), new Integer(200)});
        // add the model to the NameContainer of the dialog model
        oUnoMenu2.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
        oUnoMenu2.addLabelForPopupMenu();
        oUnoMenu2.executeDialog();
        }catch( Exception ex ) {
            ex.printStackTrace(System.err);
        }
        finally{
        //make sure always to dispose the component and free the memory!
        if (oUnoMenu2 != null) {
            if (oUnoMenu2.m_xComponent != null){
                oUnoMenu2.m_xComponent.dispose();
            }
        }
        System.exit( 0 );
    }}


    public void addLabelForPopupMenu(){
    try{
        String sName = "lblPopup";
        Object oFTModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
        XMultiPropertySet xFTModelMPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oFTModel);
        // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
        xFTModelMPSet.setPropertyValues(
            new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
            new Object[] { new Integer(8), "Right-click here", sName, new Integer(50), new Integer(50), new Integer(100)});
        // add the model to the NameContainer of the dialog model
        m_xDlgModelNameContainer.insertByName(sName, oFTModel);
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, m_xDlgContainer.getControl(sName));
        xWindow.addMouseListener(this);
    }catch( Exception e ) {
        System.err.println( e + e.getMessage());
        e.printStackTrace();
    }}

    protected void closeDialog(){
        xDialog.endExecute();
    }

    public void mouseReleased(MouseEvent mouseEvent) {
    }

    public void mousePressed(MouseEvent mouseEvent) {
        if (mouseEvent.PopupTrigger){
            Rectangle aRectangle = new Rectangle(mouseEvent.X, mouseEvent.Y, 0, 0);
            XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, mouseEvent.Source);
            getPopupMenu().execute( xControl.getPeer(), aRectangle, com.sun.star.awt.PopupMenuDirection.DEFAULT);
        }
    }

    public void mouseExited(MouseEvent mouseEvent) {
    }

    public void mouseEntered(MouseEvent mouseEvent) {
    }

    public void disposing(EventObject eventObject) {
    }
}
