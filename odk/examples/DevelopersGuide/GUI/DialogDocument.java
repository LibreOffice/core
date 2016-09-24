/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.awt.PushButtonType;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XView;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class DialogDocument extends UnoDialogSample {

    public DialogDocument(XComponentContext _xContext, XMultiComponentFactory _xMCF) {
        super(_xContext, _xMCF);
    }

    public static void main(String args[]){
        DialogDocument oDialogDocument = null;
        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            oDialogDocument = new DialogDocument(xContext, xMCF);
            oDialogDocument.initialize( new String[] {"Height", "Moveable", "Name","PositionX","PositionY", "Step", "TabIndex","Title","Width"},
                    new Object[] { Integer.valueOf(400), Boolean.TRUE, "Dialog1", Integer.valueOf(102),Integer.valueOf(41), Integer.valueOf(1), Short.valueOf((short) 0), "Document-Dialog", Integer.valueOf(300)});
            oDialogDocument.createWindowPeer();
            Object oFTHeaderModel = oDialogDocument.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTHeaderModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
            xFTHeaderModelMPSet.setPropertyValues(
                    new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(8), "This code-sample demonstrates how to display an office document in a dialog window", "HeaderLabel", Integer.valueOf(6), Integer.valueOf(6), Integer.valueOf(300)});
            // add the model to the NameContainer of the dialog model
            oDialogDocument.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
            oDialogDocument.showDocumentinDialogWindow(oDialogDocument.m_xWindowPeer, new Rectangle(40, 50, 420, 550), "private:factory/swriter");

            oDialogDocument.insertButton(oDialogDocument, 126, 370, 50, "~Close dialog", (short) PushButtonType.OK_value);
            oDialogDocument.executeDialog();
        }catch( Exception ex ) {
            ex.printStackTrace(System.err);
        } finally{
            //make sure always to dispose the component and free the memory!
            if (oDialogDocument != null){
                if (oDialogDocument.m_xComponent != null) {
                    oDialogDocument.m_xComponent.dispose();
                }
            }
        }

        System.exit( 0 );
    }

    public void showDocumentinDialogWindow(XWindowPeer _xParentWindowPeer, Rectangle _aRectangle, String _sUrl){
        try {
            // The Toolkit is the creator of all windows...
            Object oToolkit = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
            XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, oToolkit);

            // set up a window description and create the window. A parent window is always necessary for this...
            com.sun.star.awt.WindowDescriptor aWindowDescriptor = new com.sun.star.awt.WindowDescriptor();
            // a simple window is enough for this purpose...
            aWindowDescriptor.Type = WindowClass.SIMPLE;
            aWindowDescriptor.WindowServiceName = "dockingwindow";
            // assign the parent window peer as described in the idl description...
            aWindowDescriptor.Parent = _xParentWindowPeer;
            aWindowDescriptor.ParentIndex = 1;
            aWindowDescriptor.Bounds = _aRectangle;

            // set the window attributes...
            // The attribute CLIPCHILDREN causes the parent to not repaint the areas of the children...
            aWindowDescriptor.WindowAttributes = VclWindowPeerAttribute.CLIPCHILDREN + WindowAttribute.BORDER + WindowAttribute.SHOW;
            XWindowPeer xWindowPeer = xToolkit.createWindow(aWindowDescriptor);
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xWindowPeer);
            XView xView = UnoRuntime.queryInterface(XView.class, xWindow);

            // create a frame and initialize it with the created window...
            Object oFrame = m_xMCF.createInstanceWithContext("com.sun.star.frame.Frame", m_xContext);
            // The frame should be of global scope because it's within the responsibility to dispose it after usage
            m_xFrame = UnoRuntime.queryInterface(XFrame.class, oFrame);
            m_xFrame.initialize(xWindow);

            // load the document and open it in preview mode
            XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, m_xFrame);
            PropertyValue[] aPropertyValues = new PropertyValue[2];
            PropertyValue aPropertyValue = new PropertyValue();
            aPropertyValue.Name = "Preview";
            aPropertyValue.Value = Boolean.TRUE;
            aPropertyValues[0] = aPropertyValue;
            aPropertyValue = new PropertyValue();
            aPropertyValue.Name = "ReadOnly";
            aPropertyValue.Value = Boolean.TRUE;
            aPropertyValues[1] = aPropertyValue;
            xComponentLoader.loadComponentFromURL(_sUrl, "_self", 0, aPropertyValues);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            throw new java.lang.RuntimeException("cannot happen...", ex);
        } catch (com.sun.star.uno.Exception ex) {
            throw new java.lang.RuntimeException("cannot happen...", ex);
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
