/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                    new Object[] { new Integer(400), Boolean.TRUE, "Dialog1", new Integer(102),new Integer(41), new Integer(1), new Short((short) 0), "Document-Dialog", new Integer(300)});
            oDialogDocument.createWindowPeer();
            Object oFTHeaderModel = oDialogDocument.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTHeaderModelMPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
            xFTHeaderModelMPSet.setPropertyValues(
                    new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { new Integer(8), "This code-sample demonstrates how to display an office document in a dialog window", "HeaderLabel", new Integer(6), new Integer(6), new Integer(300)});
            // add the model to the NameContainer of the dialog model
            oDialogDocument.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
            oDialogDocument.showDocumentinDialogWindow(oDialogDocument.m_xWindowPeer, new Rectangle(40, 50, 420, 550), "private:factory/swriter");

            oDialogDocument.insertButton(oDialogDocument, 126, 370, 50, "~Close dialog", (short) PushButtonType.OK_value);
            oDialogDocument.executeDialog();
        }catch( Exception ex ) {
            ex.printStackTrace(System.out);
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
            XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class, oToolkit);

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
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xWindowPeer);
            XView xView = (XView) UnoRuntime.queryInterface(XView.class, xWindow);

            // create a frame and initialize it with the created window...
            Object oFrame = m_xMCF.createInstanceWithContext("com.sun.star.frame.Frame", m_xContext);
            // The frame should be of global scope because it's within the responsibility to dispose it after usage
            m_xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, oFrame);
            m_xFrame.initialize(xWindow);

            // load the document and open it in preview mode
            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, m_xFrame);
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
            ex.printStackTrace();
            throw new java.lang.RuntimeException("cannot happen...");
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
            throw new java.lang.RuntimeException("cannot happen...");
        }
    }

}
