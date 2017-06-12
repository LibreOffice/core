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

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.AdjustmentEvent;
import com.sun.star.awt.AdjustmentType;
import com.sun.star.awt.FocusChangeReason;
import com.sun.star.awt.FocusEvent;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.KeyEvent;
import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.PushButtonType;
import com.sun.star.awt.SpinEvent;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XAdjustmentListener;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XComboBox;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XFocusListener;
import com.sun.star.awt.XItemEventBroadcaster;
import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XKeyListener;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XPointer;
import com.sun.star.awt.XReschedule;
import com.sun.star.awt.XScrollBar;
import com.sun.star.awt.XSpinField;
import com.sun.star.awt.XSpinListener;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XTextListener;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.ucb.XFileIdentifierConverter;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.Date;
import com.sun.star.util.Time;
import com.sun.star.util.thePathSettings;


// Suggestion: hand the position and width over as parameters

public class UnoDialogSample implements XTextListener, XSpinListener, XActionListener, XFocusListener, XMouseListener, XItemListener, XAdjustmentListener, XKeyListener {
    protected XComponentContext m_xContext = null;
    protected com.sun.star.lang.XMultiComponentFactory m_xMCF;
    protected XMultiServiceFactory m_xMSFDialogModel;
    private XModel m_xModel;
    protected XNameContainer m_xDlgModelNameContainer;
    protected XControlContainer m_xDlgContainer;
//    protected XNameAccess m_xDlgModelNameAccess;
    protected XControl m_xDialogControl;
    protected XDialog xDialog;

    protected XWindowPeer m_xWindowPeer = null;

    protected XFrame m_xFrame = null;
    protected XComponent m_xComponent = null;


    /**
     * Creates a new instance of UnoDialogSample
     */
    public UnoDialogSample(XComponentContext _xContext, XMultiComponentFactory _xMCF) {
        m_xContext = _xContext;
        m_xMCF = _xMCF;
        createDialog(m_xMCF);
    }


    public static void main(String args[]) {
        UnoDialogSample oUnoDialogSample = null;

        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            oUnoDialogSample = new UnoDialogSample(xContext, xMCF);
            oUnoDialogSample.initialize( new String[] {"Height", "Moveable", "Name","PositionX","PositionY", "Step", "TabIndex","Title","Width"},
                    new Object[] { Integer.valueOf(380), Boolean.TRUE, "MyTestDialog", Integer.valueOf(102),Integer.valueOf(41), Integer.valueOf(0), Short.valueOf((short) 0), "OpenOffice", Integer.valueOf(380)});
            Object oFTHeaderModel = oUnoDialogSample.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTHeaderModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
            xFTHeaderModelMPSet.setPropertyValues(
                    new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(8), "This code-sample demonstrates how to create various controls in a dialog", "HeaderLabel", Integer.valueOf(106), Integer.valueOf(6), Integer.valueOf(300)});
            // add the model to the NameContainer of the dialog model
            oUnoDialogSample.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
            oUnoDialogSample.insertFixedText(oUnoDialogSample, 106, 18, 100, 0, "My ~Label");
            oUnoDialogSample.insertCurrencyField(oUnoDialogSample, 106, 30, 60);
            oUnoDialogSample.insertProgressBar(106, 44, 100, 100);
            oUnoDialogSample.insertHorizontalFixedLine(106, 58, 100, "My FixedLine");
            oUnoDialogSample.insertEditField(oUnoDialogSample, oUnoDialogSample, 106, 72, 60);
            oUnoDialogSample.insertTimeField(106, 96, 50, new Time(0,(short)0,(short)0,(short)10,false), new Time((short)0,(short)0,(short)0,(short)0,false), new Time((short)0,(short)0,(short)0,(short)17,false));
            oUnoDialogSample.insertDateField(oUnoDialogSample, 166, 96, 50);
            oUnoDialogSample.insertGroupBox(102, 124, 70, 100);
            oUnoDialogSample.insertPatternField(106, 136, 50);
            oUnoDialogSample.insertNumericField(106, 152, 50, 0.0, 1000.0, 500.0, 100.0, (short) 1);
            oUnoDialogSample.insertCheckBox(oUnoDialogSample, 106, 168, 150);
            oUnoDialogSample.insertRadioButtonGroup((short) 50, 130, 200, 150);
            oUnoDialogSample.insertListBox(106, 230, 50, 0, new String[]{"First Item", "Second Item"});
            oUnoDialogSample.insertComboBox(106, 250, 50);
            oUnoDialogSample.insertFormattedField(oUnoDialogSample, 106, 270, 100);
            oUnoDialogSample.insertVerticalScrollBar(oUnoDialogSample, 230, 230, 52);
            oUnoDialogSample.insertFileControl(oUnoDialogSample, 106, 290, 200 );
            oUnoDialogSample.insertButton(oUnoDialogSample, 106, 320, 50, "~Close dialog", (short) PushButtonType.OK_value);
            oUnoDialogSample.createWindowPeer();
            oUnoDialogSample.addRoadmap();
            oUnoDialogSample.insertRoadmapItem(0, true, "Introduction", 1);
            oUnoDialogSample.insertRoadmapItem(1, true, "Documents", 2);
            oUnoDialogSample.xDialog = UnoRuntime.queryInterface(XDialog.class, oUnoDialogSample.m_xDialogControl);
            oUnoDialogSample.executeDialog();
        }catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        } finally{
            //make sure always to dispose the component and free the memory!
            if (oUnoDialogSample != null){
                if (oUnoDialogSample.m_xComponent != null){
                    oUnoDialogSample.m_xComponent.dispose();
                }
            }
        }

        System.exit( 0 );
    }


    protected void createDialog(XMultiComponentFactory _xMCF) {
        try {
            Object oDialogModel =  _xMCF.createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", m_xContext);

            // The XMultiServiceFactory of the dialogmodel is needed to instantiate the controls...
            m_xMSFDialogModel = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDialogModel);

            // The named container is used to insert the created controls into...
            m_xDlgModelNameContainer = UnoRuntime.queryInterface(XNameContainer.class, oDialogModel);

            // create the dialog...
            Object oUnoDialog = _xMCF.createInstanceWithContext("com.sun.star.awt.UnoControlDialog", m_xContext);
            m_xDialogControl = UnoRuntime.queryInterface(XControl.class, oUnoDialog);

            // The scope of the control container is public...
            m_xDlgContainer = UnoRuntime.queryInterface(XControlContainer.class, oUnoDialog);

            UnoRuntime.queryInterface(XTopWindow.class, m_xDlgContainer);

            // link the dialog and its model...
            XControlModel xControlModel = UnoRuntime.queryInterface(XControlModel.class, oDialogModel);
            m_xDialogControl.setModel(xControlModel);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
    }






    public short executeDialog() {
        if (m_xWindowPeer == null) {
            createWindowPeer();
        }
        xDialog = UnoRuntime.queryInterface(XDialog.class, m_xDialogControl);
        m_xComponent = UnoRuntime.queryInterface(XComponent.class, m_xDialogControl);
        return xDialog.execute();
    }




    public XItemListener getRoadmapItemStateChangeListener(){
        return new RoadmapItemStateChangeListener(m_xMSFDialogModel);
    }


    public void initialize(String[] PropertyNames, Object[] PropertyValues) {
        try{
            XMultiPropertySet xMultiPropertySet = UnoRuntime.queryInterface(XMultiPropertySet.class, m_xDlgModelNameContainer);
            xMultiPropertySet.setPropertyValues(PropertyNames, PropertyValues);
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace(System.err);
        }}



    /**
     * create a peer for this
     * dialog, using the given
     * peer as a parent.
     */
    private XWindowPeer createWindowPeer(XWindowPeer _xWindowParentPeer) {
        try{
            if (_xWindowParentPeer == null){
                XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, m_xDlgContainer);
                xWindow.setVisible(false);
                Object tk = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
                XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, tk);
                UnoRuntime.queryInterface(XReschedule.class, xToolkit);
                m_xDialogControl.createPeer(xToolkit, _xWindowParentPeer);
                m_xWindowPeer = m_xDialogControl.getPeer();
                return m_xWindowPeer;
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
        return null;
    }






    /**
     * Creates a peer for this
     * dialog, using the active OO frame
     * as the parent window.
     */
    public XWindowPeer createWindowPeer() {
        return createWindowPeer(null);
    }







    private XFixedText insertFixedText(XMouseListener _xMouseListener, int _nPosX, int _nPosY, int _nWidth, int _nStep, String _sLabel){
        XFixedText xFixedText = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "Label");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oFTModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFTModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!

            xFTModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Step", "Width"},
                    new Object[] { Integer.valueOf(8), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nStep), Integer.valueOf(_nWidth)});
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oFTModel);

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xFTPSet = UnoRuntime.queryInterface(XPropertySet.class, oFTModel);
            xFTPSet.setPropertyValue("Label", _sLabel);

            // reference the control by the Name
            XControl xFTControl = m_xDlgContainer.getControl(sName);
            xFixedText = UnoRuntime.queryInterface(XFixedText.class, xFTControl);
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xFTControl);
            xWindow.addMouseListener(_xMouseListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xFixedText;
    }


    private XTextComponent insertCurrencyField(XTextListener _xTextListener, int _nPositionX, int _nPositionY, int _nWidth){
        XTextComponent xTextComponent = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "CurrencyField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oCFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlCurrencyFieldModel");
            XMultiPropertySet xCFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oCFModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xCFModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(12), sName, Integer.valueOf(_nPositionX), Integer.valueOf(_nPositionY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oCFModel);
            XPropertySet xCFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oCFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xCFModelPSet.setPropertyValue("PrependCurrencySymbol", Boolean.TRUE);
            xCFModelPSet.setPropertyValue("CurrencySymbol", "$");
            xCFModelPSet.setPropertyValue("Value", new Double(2.93));

            // add a textlistener that is notified on each change of the controlvalue...
            Object oCFControl = m_xDlgContainer.getControl(sName);
            xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, oCFControl);
            xTextComponent.addTextListener(_xTextListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xTextComponent;
    }



    private XPropertySet insertProgressBar(int _nPosX, int _nPosY, int _nWidth, int _nProgressMax){
        XPropertySet xPBModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ProgressBar");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oPBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlProgressBarModel");

            XMultiPropertySet xPBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oPBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xPBModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(8), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oPBModel);
            xPBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oPBModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xPBModelPSet.setPropertyValue("ProgressValueMin", Integer.valueOf(0));
            xPBModelPSet.setPropertyValue("ProgressValueMax", Integer.valueOf(_nProgressMax));
        } catch (com.sun.star.uno.Exception ex) {
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
        return xPBModelPSet;
    }



    public void insertHorizontalFixedLine(int _nPosX, int _nPosY, int _nWidth, String _sLabel){
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FixedLine");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oFLModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedLineModel");
            XMultiPropertySet xFLModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFLModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xFLModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "Orientation", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(8), sName, Integer.valueOf(0), Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oFLModel);

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xFLPSet = UnoRuntime.queryInterface(XPropertySet.class, oFLModel);
            xFLPSet.setPropertyValue("Label", _sLabel);
        } catch (com.sun.star.uno.Exception ex) {
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
    }



    private void insertGroupBox(int _nPosX, int _nPosY, int _nHeight, int _nWidth){
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FrameControl");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oGBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlGroupBoxModel");
            XMultiPropertySet xGBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oGBModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xGBModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(_nHeight), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oGBModel);

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xGBPSet = UnoRuntime.queryInterface(XPropertySet.class, oGBModel);
            xGBPSet.setPropertyValue("Label", "~My GroupBox");
        } catch (com.sun.star.uno.Exception ex) {
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
    }



    private XTextComponent insertEditField(XTextListener _xTextListener, XFocusListener _xFocusListener, int _nPosX, int _nPosY, int _nWidth){
        XTextComponent xTextComponent = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "TextField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oTFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlEditModel");
            XMultiPropertySet xTFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oTFModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xTFModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Text", "Width"},
                    new Object[] { Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), "MyText", Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oTFModel);
            XPropertySet xTFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oTFModel);

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xTFModelPSet.setPropertyValue("EchoChar", Short.valueOf((short) '*'));
            XControl xTFControl = m_xDlgContainer.getControl(sName);

            // add a textlistener that is notified on each change of the controlvalue...
            xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, xTFControl);
            XWindow xTFWindow = UnoRuntime.queryInterface(XWindow.class, xTFControl);
            xTFWindow.addFocusListener(_xFocusListener);
            xTextComponent.addTextListener(_xTextListener);
            xTFWindow.addKeyListener(this);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xTextComponent;
    }

    private XPropertySet insertTimeField(int _nPosX, int _nPosY, int _nWidth, Time _aTime, Time _aTimeMin, Time _aTimeMax){
        XPropertySet xTFModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "TimeField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oTFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlTimeFieldModel");
            XMultiPropertySet xTFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oTFModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xTFModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Spin", "Width"},
                    new Object[] { Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Boolean.TRUE, Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oTFModel);
            xTFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oTFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xTFModelPSet.setPropertyValue("TimeFormat", Short.valueOf((short) 5));
            xTFModelPSet.setPropertyValue("TimeMin", _aTimeMin);
            xTFModelPSet.setPropertyValue("TimeMax", _aTimeMax);
            xTFModelPSet.setPropertyValue("Time", _aTime);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xTFModelPSet;
    }



    private XPropertySet insertDateField(XSpinListener _xSpinListener, int _nPosX, int _nPosY, int _nWidth){
        XPropertySet xDFModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "DateField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oDFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlDateFieldModel");
            XMultiPropertySet xDFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oDFModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xDFModelMPSet.setPropertyValues(
                    new String[] {"Dropdown", "Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] {Boolean.TRUE, Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oDFModel);
            xDFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oDFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xDFModelPSet.setPropertyValue("DateFormat", Short.valueOf((short) 7));
            xDFModelPSet.setPropertyValue("DateMin", new Date((short)1, (short)4, (short)2007));
            xDFModelPSet.setPropertyValue("DateMax", new Date((short)1, (short)5, (short)2007));
            xDFModelPSet.setPropertyValue("Date", new Date((short)15, (short)4, (short)2000));
            Object oDFControl = m_xDlgContainer.getControl(sName);

            // add a SpinListener that is notified on each change of the controlvalue...
            XSpinField xSpinField = UnoRuntime.queryInterface(XSpinField.class, oDFControl);
            xSpinField.addSpinListener(_xSpinListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xDFModelPSet;
    }


    private XPropertySet insertPatternField(int _nPosX, int _nPosY, int _nWidth){
        XPropertySet xPFModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "PatternField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oPFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlPatternFieldModel");
            XMultiPropertySet xPFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oPFModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xPFModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oPFModel);
            xPFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oPFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xPFModelPSet.setPropertyValue("LiteralMask", "__.05.2007");
            // Allow only numbers for the first two digits...
            xPFModelPSet.setPropertyValue("EditMask", "NNLLLLLLLL");
            // verify the user input immediately...
            xPFModelPSet.setPropertyValue("StrictFormat", Boolean.TRUE);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xPFModelPSet;
    }


    private XPropertySet insertNumericField( int _nPosX, int _nPosY, int _nWidth,
            double _fValueMin, double _fValueMax, double _fValue,
            double _fValueStep, short _nDecimalAccuracy){
        XPropertySet xNFModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "NumericField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oNFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlNumericFieldModel");
            XMultiPropertySet xNFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oNFModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xNFModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Spin", "StrictFormat", "Width"},
                    new Object[] { Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Boolean.TRUE, Boolean.TRUE, Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oNFModel);
            xNFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oNFModel);
            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xNFModelPSet.setPropertyValue("ValueMin", new Double(_fValueMin));
            xNFModelPSet.setPropertyValue("ValueMax", new Double(_fValueMax));
            xNFModelPSet.setPropertyValue("Value", new Double(_fValue));
            xNFModelPSet.setPropertyValue("ValueStep", new Double(_fValueStep));
            xNFModelPSet.setPropertyValue("ShowThousandsSeparator", Boolean.TRUE);
            xNFModelPSet.setPropertyValue("DecimalAccuracy", Short.valueOf(_nDecimalAccuracy));
        } catch (com.sun.star.uno.Exception ex) {
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
        return xNFModelPSet;
    }



    private XPropertySet insertVerticalScrollBar(XAdjustmentListener _xAdjustmentListener, int _nPosX, int _nPosY, int _nHeight){
        XPropertySet xSBModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ScrollBar");

            Integer NOrientation = Integer.valueOf(com.sun.star.awt.ScrollBarOrientation.VERTICAL);

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oSBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlScrollBarModel");
            XMultiPropertySet xSBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oSBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xSBModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "Orientation", "PositionX", "PositionY", "Width"},
                    new Object[] { Integer.valueOf(_nHeight), sName, NOrientation, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(8)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oSBModel);

            xSBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oSBModel);
            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xSBModelPSet.setPropertyValue("ScrollValueMin", Integer.valueOf(0));
            xSBModelPSet.setPropertyValue("ScrollValueMax", Integer.valueOf(100));
            xSBModelPSet.setPropertyValue("ScrollValue", Integer.valueOf(5));
            xSBModelPSet.setPropertyValue("LineIncrement", Integer.valueOf(2));
            xSBModelPSet.setPropertyValue("BlockIncrement", Integer.valueOf(10));

            // Add an Adjustment that will listen to changes of the scrollbar...
            XControl xSBControl = m_xDlgContainer.getControl(sName);
            XScrollBar xScrollBar = UnoRuntime.queryInterface(XScrollBar.class, xSBControl);
            xScrollBar.addAdjustmentListener(_xAdjustmentListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xSBModelPSet;
    }


    /** makes a String unique by appending a numerical suffix
     * @param _xElementContainer the com.sun.star.container.XNameAccess container
     * that the new Element is going to be inserted to
     * @param _sElementName the StemName of the Element
     */
    public static String createUniqueName(XNameAccess _xElementContainer, String _sElementName) {
        boolean bElementexists = true;
        int i = 1;
        String BaseName = _sElementName;
        while (bElementexists) {
            bElementexists = _xElementContainer.hasByName(_sElementName);
            if (bElementexists) {
                i += 1;
                _sElementName = BaseName + Integer.toString(i);
            }
        }
        return _sElementName;
    }


    private XCheckBox insertCheckBox(XItemListener _xItemListener, int _nPosX, int _nPosY, int _nWidth){
        XCheckBox xCheckBox = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "CheckBox");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oCBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlCheckBoxModel");

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            XMultiPropertySet xCBMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oCBModel);
            xCBMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "Width" } ,
                    new Object[] {Integer.valueOf(8), "~Include page number", sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nWidth)});

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xCBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, xCBMPSet);
            xCBModelPSet.setPropertyValue("TriState", Boolean.TRUE);
            xCBModelPSet.setPropertyValue("State", Short.valueOf((short) 1));

            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oCBModel);
            XControl xCBControl = m_xDlgContainer.getControl(sName);
            xCheckBox = UnoRuntime.queryInterface(XCheckBox.class, xCBControl);
            // An ActionListener will be notified on the activation of the button...
            xCheckBox.addItemListener(_xItemListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xCheckBox;
    }




    private void insertRadioButtonGroup(short _nTabIndex, int _nPosX, int _nPosY, int _nWidth){
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "OptionButton");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oRBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlRadioButtonModel");
            XMultiPropertySet xRBMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oRBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xRBMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "State", "TabIndex", "Width" } ,
                    new Object[] {Integer.valueOf(8), "~First Option", sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Short.valueOf((short) 1), Short.valueOf(_nTabIndex++),Integer.valueOf(_nWidth)});
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oRBModel);

            // create a unique name by means of an own implementation...
            sName = createUniqueName(m_xDlgModelNameContainer, "OptionButton");

            oRBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlRadioButtonModel");
            xRBMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oRBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xRBMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "TabIndex", "Width" } ,
                    new Object[] {Integer.valueOf(8), "~Second Option", sName, Integer.valueOf(130), Integer.valueOf(214), Short.valueOf(_nTabIndex), Integer.valueOf(150)});
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oRBModel);
        } catch (com.sun.star.uno.Exception ex) {
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
    }


    private XListBox insertListBox(int _nPosX, int _nPosY, int _nWidth, int _nStep, String[] _sStringItemList){
        XListBox xListBox = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ListBox");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oListBoxModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlListBoxModel");
            XMultiPropertySet xLBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oListBoxModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xLBModelMPSet.setPropertyValues(
                    new String[]  {"Dropdown", "Height", "Name", "PositionX", "PositionY", "Step", "StringItemList", "Width" } ,
                    new Object[] {Boolean.TRUE, Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Integer.valueOf(_nStep), _sStringItemList, Integer.valueOf(_nWidth)});
            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xLBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, xLBModelMPSet);
            xLBModelPSet.setPropertyValue("MultiSelection", Boolean.TRUE);
            short[] nSelItems = new short[] {(short) 1, (short) 3};
            xLBModelPSet.setPropertyValue("SelectedItems", nSelItems);
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, xLBModelMPSet);
            XControl xControl = m_xDlgContainer.getControl(sName);
            // retrieve a ListBox that is more convenient to work with than the Model of the ListBox...
            xListBox = UnoRuntime.queryInterface(XListBox.class, xControl);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xListBox;
    }


    private XComboBox insertComboBox(int _nPosX, int _nPosY, int _nWidth){
        XComboBox xComboBox = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ComboBox");

            String[] sStringItemList = new String[]{"First Entry", "Second Entry", "Third Entry", "Fourth Entry"};

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oComboBoxModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlComboBoxModel");
            XMultiPropertySet xCbBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oComboBoxModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xCbBModelMPSet.setPropertyValues(
                    new String[]  {"Dropdown", "Height", "Name", "PositionX", "PositionY", "StringItemList", "Width" } ,
                    new Object[] {Boolean.TRUE, Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), sStringItemList, Integer.valueOf(_nWidth)});

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xCbBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, xCbBModelMPSet);
            xCbBModelPSet.setPropertyValue("MaxTextLen", Short.valueOf((short) 10));
            xCbBModelPSet.setPropertyValue("ReadOnly", Boolean.FALSE);

            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, xCbBModelMPSet);
            XControl xControl = m_xDlgContainer.getControl(sName);

            // retrieve a ListBox that is more convenient to work with than the Model of the ListBox...
            xComboBox = UnoRuntime.queryInterface(XComboBox.class, xControl);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xComboBox;
    }



    private XPropertySet insertFormattedField(XSpinListener _xSpinListener, int _nPosX, int _nPosY, int _nWidth){
        XPropertySet xFFModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FormattedField");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oFFModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFormattedFieldModel");
            XMultiPropertySet xFFModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFFModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xFFModelMPSet.setPropertyValues(
                    new String[] {"EffectiveValue", "Height",  "Name", "PositionX", "PositionY", "StrictFormat", "Spin", "Width"},
                    new Object[] { new Double(12348), Integer.valueOf(12), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Boolean.TRUE, Boolean.TRUE, Integer.valueOf(_nWidth)});

            xFFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oFFModel);
            // to define a numberformat you always need a locale...
            com.sun.star.lang.Locale aLocale = new com.sun.star.lang.Locale();
            aLocale.Country = "US";
            aLocale.Language = "en";
            // this Format is only compliant to the english locale!
            String sFormatString = "NNNNMMMM DD, YYYY";

            // a NumberFormatsSupplier has to be created first "in the open countryside"...
            Object oNumberFormatsSupplier = m_xMCF.createInstanceWithContext("com.sun.star.util.NumberFormatsSupplier", m_xContext);
            XNumberFormatsSupplier xNumberFormatsSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, oNumberFormatsSupplier);
            XNumberFormats xNumberFormats = xNumberFormatsSupplier.getNumberFormats();
            // is the numberformat already defined?
            int nFormatKey = xNumberFormats.queryKey(sFormatString, aLocale, true);
            if (nFormatKey == -1){
                // if not then add it to the NumberFormatsSupplier
                nFormatKey = xNumberFormats.addNew(sFormatString, aLocale);
            }

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xFFModelPSet.setPropertyValue("FormatsSupplier", xNumberFormatsSupplier);
            xFFModelPSet.setPropertyValue("FormatKey", Integer.valueOf(nFormatKey));

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oFFModel);

            // finally we add a Spin-Listener to the control
            XControl xFFControl = m_xDlgContainer.getControl(sName);
            // add a SpinListener that is notified on each change of the controlvalue...
            XSpinField xSpinField = UnoRuntime.queryInterface(XSpinField.class, xFFControl);
            xSpinField.addSpinListener(_xSpinListener);

        } catch (com.sun.star.uno.Exception ex) {
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
        return xFFModelPSet;
    }




    private XTextComponent insertFileControl(XTextListener _xTextListener, int _nPosX, int _nPosY, int _nWidth){
        XTextComponent xTextComponent = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FileControl");

            // retrieve the configured Work path...
            Object oPathSettings = thePathSettings.get(m_xContext);
            XPropertySet xPropertySet = com.sun.star.uno.UnoRuntime.queryInterface(XPropertySet.class, oPathSettings);
            String sWorkUrl = (String) xPropertySet.getPropertyValue("Work");

            // convert the Url to a system path that is "human readable"...
            Object oFCProvider = m_xMCF.createInstanceWithContext("com.sun.star.ucb.FileContentProvider", m_xContext);
            XFileIdentifierConverter xFileIdentifierConverter = UnoRuntime.queryInterface(XFileIdentifierConverter.class, oFCProvider);
            String sSystemWorkPath = xFileIdentifierConverter.getSystemPathFromFileURL(sWorkUrl);

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oFCModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFileControlModel");

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            XMultiPropertySet xFCModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFCModel);
            xFCModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Text", "Width"},
                    new Object[] { Integer.valueOf(14), sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), sSystemWorkPath, Integer.valueOf(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oFCModel);
            XPropertySet xFCModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oFCModel);

            // add a textlistener that is notified on each change of the controlvalue...
            XControl xFCControl = m_xDlgContainer.getControl(sName);
            xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, xFCControl);
            XWindow xFCWindow = UnoRuntime.queryInterface(XWindow.class, xFCControl);
            xTextComponent.addTextListener(_xTextListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xTextComponent;
    }


    public XButton insertButton(XActionListener _xActionListener, int _nPosX, int _nPosY, int _nWidth, String _sLabel, short _nPushButtonType){
        XButton xButton = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "CommandButton");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oButtonModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlButtonModel");
            XMultiPropertySet xButtonMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oButtonModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xButtonMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "PushButtonType", "Width" } ,
                    new Object[] {Integer.valueOf(14), _sLabel, sName, Integer.valueOf(_nPosX), Integer.valueOf(_nPosY), Short.valueOf(_nPushButtonType), Integer.valueOf(_nWidth)});

            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oButtonModel);
            XControl xButtonControl = m_xDlgContainer.getControl(sName);
            xButton = UnoRuntime.queryInterface(XButton.class, xButtonControl);
            // An ActionListener will be notified on the activation of the button...
            xButton.addActionListener(_xActionListener);
        } catch (com.sun.star.uno.Exception ex) {
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
        return xButton;
    }



    public XFrame getCurrentFrame(){
        XFrame xRetFrame = null;
        try {
            Object oDesktop = m_xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", m_xContext);
            XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, oDesktop);
            xRetFrame = xDesktop.getCurrentFrame();
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
        return xRetFrame;
    }


    public void textChanged(TextEvent textEvent) {
        try {
            // get the control that has fired the event,
            XControl xControl = UnoRuntime.queryInterface(XControl.class, textEvent.Source);
            XControlModel xControlModel = xControl.getModel();
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            String sName = (String) xPSet.getPropertyValue("Name");
            // just in case the listener has been added to several controls,
            // we make sure we refer to the right one
            if (sName.equals("TextField1")){
                String sText = (String) xPSet.getPropertyValue("Text");
                System.out.println(sText);
                // insert your code here to validate the text of the control...
            }
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
    }



    public void up(SpinEvent spinEvent) {
        try {
            // get the control that has fired the event,
            XControl xControl = UnoRuntime.queryInterface(XControl.class, spinEvent.Source);
            XControlModel xControlModel = xControl.getModel();
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            String sName = (String) xPSet.getPropertyValue("Name");
            // just in case the listener has been added to several controls,
            // we make sure we refer to the right one
            if (sName.equals("FormattedField1")){
                double fvalue = AnyConverter.toDouble(xPSet.getPropertyValue("EffectiveValue"));
                System.out.println("Controlvalue:  " + fvalue);
                // insert your code here to validate the value of the control...
            }
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
    }


    public void down(SpinEvent spinEvent) {
    }

    public void last(SpinEvent spinEvent) {
    }

    public void first(SpinEvent spinEvent) {
    }

    public void disposing(EventObject rEventObject) {
    }


    public void actionPerformed(ActionEvent rEvent) {
        try{
            // get the control that has fired the event,
            XControl xControl = UnoRuntime.queryInterface(XControl.class, rEvent.Source);
            XControlModel xControlModel = xControl.getModel();
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            String sName = (String) xPSet.getPropertyValue("Name");
            // just in case the listener has been added to several controls,
            // we make sure we refer to the right one
            if (sName.equals("CommandButton1")) {

            }
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
    }


    public void focusLost(FocusEvent _focusEvent) {
        short nFocusFlags = _focusEvent.FocusFlags;
        int nFocusChangeReason = nFocusFlags & FocusChangeReason.TAB;
        if (nFocusChangeReason == FocusChangeReason.TAB) {
            // get the window of the Window that has gained the Focus...
            // Note that the xWindow is just a representation of the controlwindow
            // but not of the control itself
            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, _focusEvent.NextFocus);
        }
    }


    public void focusGained(FocusEvent focusEvent) {
    }

    public void mouseReleased(MouseEvent mouseEvent) {
    }

    public void mousePressed(MouseEvent mouseEvent) {
    }

    public void mouseExited(MouseEvent mouseEvent) {
    }

    public void mouseEntered(MouseEvent _mouseEvent) {
        try {
            // retrieve the control that the event has been invoked at...
            XControl xControl = UnoRuntime.queryInterface(XControl.class, _mouseEvent.Source);
            Object tk = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
            XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, tk);
            // create the peer of the control by passing the windowpeer of the parent
            // in this case the windowpeer of the control
            xControl.createPeer(xToolkit, m_xWindowPeer);
            // create a pointer object "in the open countryside" and set the type accordingly...
            Object oPointer = this.m_xMCF.createInstanceWithContext("com.sun.star.awt.Pointer", this.m_xContext);
            XPointer xPointer = UnoRuntime.queryInterface(XPointer.class, oPointer);
            xPointer.setType(com.sun.star.awt.SystemPointer.REFHAND);
            // finally set the created pointer at the windowpeer of the control
            xControl.getPeer().setPointer(xPointer);
        } catch (com.sun.star.uno.Exception ex) {
            throw new java.lang.RuntimeException("cannot happen...", ex);
        }
    }



    public void itemStateChanged(ItemEvent itemEvent) {
        try{
            // retrieve the control that the event has been invoked at...
            XCheckBox xCheckBox = UnoRuntime.queryInterface(XCheckBox.class, itemEvent.Source);
            // retrieve the control that we want to disable or enable
            XControl xControl = UnoRuntime.queryInterface(XControl.class, m_xDlgContainer.getControl("CommandButton1"));
            XPropertySet xModelPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xControl.getModel());
            short nState = xCheckBox.getState();
            boolean bdoEnable = true;
            switch (nState){
                case 1:     // checked
                    bdoEnable = true;
                    break;
                case 0:     // not checked
                case 2:     // don't know
                    bdoEnable = false;
                    break;
            }
            // Alternatively we could have done it also this way:
            // bdoEnable = (nState == 1);
            xModelPropertySet.setPropertyValue("Enabled", Boolean.valueOf(bdoEnable));
        }catch (com.sun.star.uno.Exception ex){
            /* perform individual exception handling here.
             * Possible exception types are:
             * com.sun.star.lang.IllegalArgumentException
             * com.sun.star.lang.WrappedTargetException,
             * com.sun.star.beans.UnknownPropertyException,
             * com.sun.star.beans.PropertyVetoException
             * com.sun.star.uno.Exception
             */
            ex.printStackTrace(System.err);
        }
    }


    public void adjustmentValueChanged(AdjustmentEvent _adjustmentEvent) {
        switch (_adjustmentEvent.Type.getValue()){
            case AdjustmentType.ADJUST_ABS_value:
                System.out.println( "The event has been triggered by dragging the thumb..." );
                break;
            case AdjustmentType.ADJUST_LINE_value:
                System.out.println( "The event has been triggered by a single line move.." );
                break;
            case AdjustmentType.ADJUST_PAGE_value:
                System.out.println( "The event has been triggered by a block move..." );
                break;
        }
        System.out.println( "The value of the scrollbar is: " + _adjustmentEvent.Value);
    }



// Globally available object variables of the roadmapmodel
    XPropertySet m_xRMPSet;
    private XSingleServiceFactory m_xSSFRoadmap;
    private XIndexContainer m_xRMIndexCont;

    public void addRoadmap() {
        XPropertySet xDialogModelPropertySet = null;
        try {
            // create a unique name by means of an own implementation...
            String sRoadmapName = createUniqueName(m_xDlgModelNameContainer, "Roadmap");

            xDialogModelPropertySet = UnoRuntime.queryInterface(XPropertySet.class, m_xMSFDialogModel);
            // Similar to the office assistants the roadmap is adjusted to the height of the dialog
            // where a certain space is left at the bottom for the buttons...
            int nDialogHeight = ((Integer) xDialogModelPropertySet.getPropertyValue("Height")).intValue();

            // instantiate the roadmapmodel...
            Object oRoadmapModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlRoadmapModel");

            // define the properties of the roadmapmodel
            XMultiPropertySet xRMMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oRoadmapModel);
            xRMMPSet.setPropertyValues( new String[] {"Complete", "Height", "Name", "PositionX", "PositionY", "Text", "Width" },
                    new Object[] {Boolean.FALSE, Integer.valueOf(nDialogHeight - 26), sRoadmapName, Integer.valueOf(0), Integer.valueOf(0), "Steps", Integer.valueOf(85)});
            m_xRMPSet = UnoRuntime.queryInterface(XPropertySet.class, oRoadmapModel);

            // add the roadmapmodel to the dialog container..
            m_xDlgModelNameContainer.insertByName(sRoadmapName, oRoadmapModel);

            // the roadmapmodel is a SingleServiceFactory to instantiate the roadmapitems...
            m_xSSFRoadmap = UnoRuntime.queryInterface(XSingleServiceFactory.class, oRoadmapModel);
            m_xRMIndexCont = UnoRuntime.queryInterface(XIndexContainer.class, oRoadmapModel);

            // add the itemlistener to the control...
            XControl xRMControl = this.m_xDlgContainer.getControl(sRoadmapName);
            XItemEventBroadcaster xRMBroadcaster = UnoRuntime.queryInterface(XItemEventBroadcaster.class, xRMControl);
            xRMBroadcaster.addItemListener( getRoadmapItemStateChangeListener() );
        } catch (java.lang.Exception jexception) {
            jexception.printStackTrace(System.err);
        }
    }

    /*
     * To fully understand the example one has to be aware that the passed ???Index??? parameter
     * refers to the position of the roadmap item in the roadmapmodel container
     * whereas the variable ???_ID??? directly references to a certain step of dialog.
     */
    public void insertRoadmapItem(int Index, boolean _bEnabled, String _sLabel, int _ID) {
        try {
            // a roadmap is a SingleServiceFactory that can only create roadmapitems that are the only possible
            // element types of the container
            Object oRoadmapItem = m_xSSFRoadmap.createInstance();
            XPropertySet xRMItemPSet = UnoRuntime.queryInterface(XPropertySet.class, oRoadmapItem);
            xRMItemPSet.setPropertyValue("Label", _sLabel);
            // sometimes steps are supposed to be set disabled depending on the program logic...
            xRMItemPSet.setPropertyValue("Enabled", Boolean.valueOf(_bEnabled));
            // in this context the "ID" is meant to refer to a step of the dialog
            xRMItemPSet.setPropertyValue("ID", Integer.valueOf(_ID));
            m_xRMIndexCont.insertByIndex(Index, oRoadmapItem);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
    }


    public void keyReleased(KeyEvent keyEvent) {
    }

    public void keyPressed(KeyEvent keyEvent) {
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
