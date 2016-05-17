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
import com.sun.star.awt.XDialog;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.graphic.XGraphic;
import com.sun.star.graphic.XGraphicProvider;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.ucb.XFileIdentifierConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class ImageControlSample extends UnoDialogSample{
    /**
     * Creates a new instance of ImageControlSample
     */
    public ImageControlSample(XComponentContext _xContext, XMultiComponentFactory _xMCF){
        super(_xContext, _xMCF);
        super.createDialog(_xMCF);
    }

    // to start this script pass a parameter denoting the system path to a graphic to be displayed
    public static void main(String args[]) {
        ImageControlSample oImageControlSample = null;
        try {
            if (args.length == 0) {
                System.out.println("Please pass a parameter denoting the system path to your graphic!");
                return;
            }
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null ){
                System.out.println("Connected to a running office ...");
            }
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            oImageControlSample = new ImageControlSample(xContext, xMCF);
            oImageControlSample.initialize( new String[] {"Height", "Moveable", "Name","PositionX","PositionY", "Step", "TabIndex","Title","Width"},
                    new Object[] { Integer.valueOf(100), Boolean.TRUE, "MyTestDialog", Integer.valueOf(102),Integer.valueOf(41), Integer.valueOf(0), Short.valueOf((short) 0), "OpenOffice", Integer.valueOf(230)});
            Object oFTHeaderModel = oImageControlSample.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTHeaderModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
            xFTHeaderModelMPSet.setPropertyValues(
                    new String[] {"Height", "Label", "MultiLine", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(16), "This code-sample demonstrates how to create an ImageControlSample within a dialog", Boolean.TRUE, "HeaderLabel", Integer.valueOf(6), Integer.valueOf(6), Integer.valueOf(210)});
            // add the model to the NameContainer of the dialog model
            oImageControlSample.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
            XPropertySet xICModelPropertySet = oImageControlSample.insertImageControl(68, 30, 32, 90);
            oImageControlSample.insertButton(oImageControlSample, 90, 75, 50, "~Close dialog", (short) PushButtonType.OK_value);
            oImageControlSample.createWindowPeer();
            // note: due to issue i76718 ("Setting graphic at a controlmodel required dialog peer") the graphic of the image control
            // may not be set before the peer of the dialog has been created.
            XGraphic xGraphic = oImageControlSample.getGraphic(oImageControlSample.m_xMCF, args[0]);
            xICModelPropertySet.setPropertyValue("Graphic", xGraphic);
            oImageControlSample.xDialog = UnoRuntime.queryInterface(XDialog.class, oImageControlSample.m_xDialogControl);
            oImageControlSample.executeDialog();
        }catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        } finally{
            //make sure always to dispose the component and free the memory!
            if (oImageControlSample != null){
                if (oImageControlSample.m_xComponent != null){
                    oImageControlSample.m_xComponent.dispose();
                }
            }
        }
        System.exit( 0 );
    }


    public XPropertySet insertImageControl(int _nPosX, int _nPosY, int _nHeight, int _nWidth){
        XPropertySet xICModelPropertySet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ImageControl");
            // convert the system path to the image to a FileUrl

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oICModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlImageControlModel");
            XMultiPropertySet xICModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oICModel);
            xICModelPropertySet =UnoRuntime.queryInterface(XPropertySet.class, oICModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            // The image is not scaled
            xICModelMPSet.setPropertyValues(
                    new String[] {"Border", "Height", "Name", "PositionX", "PositionY", "ScaleImage", "Width"},
                    new Object[] { Short.valueOf((short) 1), Integer.valueOf(_nHeight), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Boolean.FALSE, Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oICModel);
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.IllegalArgumentException,
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.container.ElementExistException,
             * com.sun.star.beans.PropertyVetoException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
        return xICModelPropertySet;
    }


// creates a UNO graphic object that can be used to be assigned
// to the property "Graphic" of a controlmodel
    public XGraphic getGraphic(XMultiComponentFactory _xMCF, String _sImageSystemPath){
        XGraphic xGraphic = null;
        try{
            java.io.File oFile = new java.io.File(_sImageSystemPath);
            Object oFCProvider = _xMCF.createInstanceWithContext("com.sun.star.ucb.FileContentProvider", this.m_xContext);
            XFileIdentifierConverter xFileIdentifierConverter = UnoRuntime.queryInterface(XFileIdentifierConverter.class, oFCProvider);
            String sImageUrl = xFileIdentifierConverter.getFileURLFromSystemPath(_sImageSystemPath, oFile.getAbsolutePath());

            // create a GraphicProvider at the global service manager...
            Object oGraphicProvider = m_xMCF.createInstanceWithContext("com.sun.star.graphic.GraphicProvider", m_xContext);
            XGraphicProvider xGraphicProvider = UnoRuntime.queryInterface(XGraphicProvider.class, oGraphicProvider);
            // create the graphic object
            PropertyValue[] aPropertyValues = new PropertyValue[1];
            PropertyValue aPropertyValue = new PropertyValue();
            aPropertyValue.Name = "URL";
            aPropertyValue.Value = sImageUrl;
            aPropertyValues[0] = aPropertyValue;
            xGraphic = xGraphicProvider.queryGraphic(aPropertyValues);
            return xGraphic;
        }catch (com.sun.star.uno.Exception ex){
            throw new java.lang.RuntimeException("cannot happen...", ex);
        }}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
