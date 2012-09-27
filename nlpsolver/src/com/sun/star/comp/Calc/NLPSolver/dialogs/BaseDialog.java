/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.comp.Calc.NLPSolver.dialogs;

import com.sun.star.awt.InvalidateStyle;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.comp.Calc.NLPSolver.dialogs.controls.BaseControl;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The BaseDialog represents the base for all dialogs used within the addon.
 * It automatically loads the necessary interfaces to access OpenOffice.org dialogs.
 */
public abstract class BaseDialog extends BaseControl {
    
    private XMultiComponentFactory xMCF;
    private Object toolkit;
    private XMultiServiceFactory xMSF;
    protected XWindow xWindow;
    protected XDialog xDialog;
    protected XWindowPeer xWindowPeer;
    protected ModalState modalState;
    
    @Override
    public String getName() {
        return null;
    }
    
    public XMultiServiceFactory getMultiServiceFactory() {
        return xMSF;
    }

    public XFrame getCurrentFrame() {
        try {
            Object oDesktop = xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", context);
            XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, oDesktop);
            XComponent xComponent = xDesktop.getCurrentComponent();
            XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xComponent);
            XController xController = xModel.getCurrentController();
            return xController.getFrame();
        } catch (Exception e) {
            return null;
        }
    }

    public Rectangle getWorkspaceDimensions() {
        return getCurrentFrame().getComponentWindow().getPosSize();
    }
    
    public BaseDialog(XComponentContext context, String title, int x, int y, int width, int height) {
        super(context);
        modalState = ModalState.Exit;
        try {
            xMCF = context.getServiceManager();
            setUnoModel(xMCF.createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", context));
            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, getUnoModel());
            
            setProperty("Title", title);
            setPosition(x, y);
            setSize(width, height);

            unoControl = xMCF.createInstanceWithContext("com.sun.star.awt.UnoControlDialog", context);
            XControl xControl = (XControl)UnoRuntime.queryInterface(XControl.class, unoControl);
            XControlModel xControlModel = (XControlModel)UnoRuntime.queryInterface(XControlModel.class, getUnoModel());
            xControl.setModel(xControlModel);
            
            toolkit = xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", context);
            XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class, toolkit);
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, unoControl);
            xWindow.setVisible(false);
            XWindowPeer xParentWindowPeer = (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, getCurrentFrame().getComponentWindow());
            xControl.createPeer(xToolkit, xParentWindowPeer);
            xWindowPeer = xControl.getPeer();
            
            xDialog = (XDialog) UnoRuntime.queryInterface(XDialog.class, unoControl);
            
            //center if necessary
            if (x < 0 || y < 0) {
                Rectangle workspacePosSize = getWorkspaceDimensions();
                Rectangle dialogPosSize = xWindow.getPosSize();
                if (x < 0)
                    dialogPosSize.X = workspacePosSize.X + (workspacePosSize.Width / 2) - (dialogPosSize.Width / 2);
                if (y < 0)
                    dialogPosSize.Y = workspacePosSize.Y + (workspacePosSize.Height / 2) - (dialogPosSize.Height / 2);
                
                xWindow.setPosSize(dialogPosSize.X, dialogPosSize.Y,
                        dialogPosSize.Width, dialogPosSize.Height, PosSize.POS);
            }
            
        } catch (Exception ex) {
            Logger.getLogger(BaseDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, unoControl);
        xComponent.dispose();
        super.finalize();
    }
    
    public ModalState showModal() {
        xWindow.setVisible(true);
        xDialog.execute();
        return modalState;
    }
    
    public void close() {
        xDialog.endExecute();
        xWindow.setVisible(false);
    }

    public void setCloseable(boolean closeable) {
        setProperty("Closeable", new Boolean(closeable));
    }

    public void repaint() {
        xWindowPeer.invalidate((short)(InvalidateStyle.CHILDREN /*| InvalidateStyle.NOERASE*/ |
                InvalidateStyle.UPDATE | InvalidateStyle.TRANSPARENT));
    }
       
}
