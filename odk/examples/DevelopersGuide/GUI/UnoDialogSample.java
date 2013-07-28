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
import com.sun.star.awt.PosSize;
import com.sun.star.awt.PushButtonType;
import com.sun.star.awt.Rectangle;
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
//import com.sun.star.awt.XMessageBoxFactory;
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
import com.sun.star.beans.PropertyValue;
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
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.XFileIdentifierConverter;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XMacroExpander;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;


// Anregung von DV:
// Position und Weite als Parameter uebergeben

public class UnoDialogSample implements XTextListener, XSpinListener, XActionListener, XFocusListener, XMouseListener, XItemListener, XAdjustmentListener, XKeyListener {
    protected XComponentContext m_xContext = null;
    protected com.sun.star.lang.XMultiComponentFactory m_xMCF;
    protected XMultiServiceFactory m_xMSFDialogModel;
    protected XModel m_xModel;
    protected XNameContainer m_xDlgModelNameContainer;
    protected XControlContainer m_xDlgContainer;
//    protected XNameAccess m_xDlgModelNameAccess;
    protected XControl m_xDialogControl;
    protected XDialog xDialog;
    protected XReschedule mxReschedule;
    protected XWindowPeer m_xWindowPeer = null;
    protected XTopWindow m_xTopWindow = null;
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
                    new Object[] { new Integer(380), Boolean.TRUE, "MyTestDialog", new Integer(102),new Integer(41), new Integer(0), new Short((short) 0), "OpenOffice", new Integer(380)});
            Object oFTHeaderModel = oUnoDialogSample.m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlFixedTextModel");
            XMultiPropertySet xFTHeaderModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oFTHeaderModel);
            xFTHeaderModelMPSet.setPropertyValues(
                    new String[] {"Height", "Label", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { new Integer(8), "This code-sample demonstrates how to create various controls in a dialog", "HeaderLabel", new Integer(106), new Integer(6), new Integer(300)});
            // add the model to the NameContainer of the dialog model
            oUnoDialogSample.m_xDlgModelNameContainer.insertByName("Headerlabel", oFTHeaderModel);
            oUnoDialogSample.insertFixedText(oUnoDialogSample, 106, 18, 100, 0, "My ~Label");
            oUnoDialogSample.insertCurrencyField(oUnoDialogSample, 106, 30, 60);
            oUnoDialogSample.insertProgressBar(106, 44, 100, 100);
            oUnoDialogSample.insertHorizontalFixedLine(106, 58, 100, "My FixedLine");
            oUnoDialogSample.insertEditField(oUnoDialogSample, oUnoDialogSample, 106, 72, 60);
            oUnoDialogSample.insertTimeField(106, 96, 50, 0, 170000, 1000);
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
            oUnoDialogSample.addRoadmap(oUnoDialogSample.getRoadmapItemStateChangeListener());
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


    /**
     * @param _sKeyName
     * @return
     */
    public XNameAccess getRegistryKeyContent(String _sKeyName){
        try {
            Object oConfigProvider;
            PropertyValue[] aNodePath = new PropertyValue[1];
            oConfigProvider = m_xMCF.createInstanceWithContext("com.sun.star.configuration.ConfigurationProvider", this.m_xContext);
            aNodePath[0] = new PropertyValue();
            aNodePath[0].Name = "nodepath";
            aNodePath[0].Value = _sKeyName;
            XMultiServiceFactory xMSFConfig = UnoRuntime.queryInterface(XMultiServiceFactory.class, oConfigProvider);
            Object oNode = xMSFConfig.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", aNodePath);
            XNameAccess xNameAccess = UnoRuntime.queryInterface(XNameAccess.class, oNode);
            return xNameAccess;
        } catch (Exception exception) {
            exception.printStackTrace(System.err);
            return null;
        }
    }


    /**
     * @param _sRegistryPath the path a registryNode
     * @param _sImageName the name of the image
     */
    public String getImageUrl(String _sRegistryPath, String _sImageName) {
        String sImageUrl = "";
        try {
            // retrive the configuration node of the extension
            XNameAccess xNameAccess = getRegistryKeyContent(_sRegistryPath);
            if (xNameAccess != null){
                if (xNameAccess.hasByName(_sImageName)){
                    // get the Image Url and process the Url by the macroexpander...
                    sImageUrl = (String) xNameAccess.getByName(_sImageName);
                    Object oMacroExpander = this.m_xContext.getValueByName("/singletons/com.sun.star.util.theMacroExpander");
                    XMacroExpander xMacroExpander = UnoRuntime.queryInterface(XMacroExpander.class, oMacroExpander);
                    sImageUrl = xMacroExpander.expandMacros(sImageUrl);
                    sImageUrl = sImageUrl.substring(new String("vnd.sun.star.expand:").length(), sImageUrl.length());
                    sImageUrl = sImageUrl.trim();
                    sImageUrl += "/" + _sImageName;
                }
            }
        } catch (Exception ex) {
        /* perform individual exception handling here.
         * Possible exception types are:
         * com.sun.star.lang.IllegalArgumentException,
         * com.sun.star.lang.WrappedTargetException,
         */
            ex.printStackTrace(System.err);
        }
        return sImageUrl;
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

            m_xTopWindow = UnoRuntime.queryInterface(XTopWindow.class, m_xDlgContainer);

            // link the dialog and its model...
            XControlModel xControlModel = UnoRuntime.queryInterface(XControlModel.class, oDialogModel);
            m_xDialogControl.setModel(xControlModel);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
    }



    public short executeDialogWithembeddedExampleSnippets() throws com.sun.star.script.BasicErrorException {
        if (m_xWindowPeer == null){
            createWindowPeer();
        }
        addRoadmap(getRoadmapItemStateChangeListener());
        insertRoadmapItem(0, true, "Introduction", 1);
        insertRoadmapItem(1, true, "Documents", 2);
        xDialog = UnoRuntime.queryInterface(XDialog.class, m_xDialogControl);
        return xDialog.execute();
    }


    public short executeDialog() throws com.sun.star.script.BasicErrorException {
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


    public void initialize(String[] PropertyNames, Object[] PropertyValues) throws com.sun.star.script.BasicErrorException{
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
     * @param parentPeer
     * @return
     * @throws java.lang.Exception
     */
    public XWindowPeer createWindowPeer(XWindowPeer _xWindowParentPeer) throws com.sun.star.script.BasicErrorException{
        try{
            if (_xWindowParentPeer == null){
                XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, m_xDlgContainer);
                xWindow.setVisible(false);
                Object tk = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
                XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, tk);
                mxReschedule = UnoRuntime.queryInterface(XReschedule.class, xToolkit);
                m_xDialogControl.createPeer(xToolkit, _xWindowParentPeer);
                m_xWindowPeer = m_xDialogControl.getPeer();
                return m_xWindowPeer;
            }
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
        return null;
    }


    public void calculateDialogPosition(XWindow _xWindow) {
        Rectangle aFramePosSize = m_xModel.getCurrentController().getFrame().getComponentWindow().getPosSize();
        Rectangle CurPosSize = _xWindow.getPosSize();
        int WindowHeight = aFramePosSize.Height;
        int WindowWidth = aFramePosSize.Width;
        int DialogWidth = CurPosSize.Width;
        int DialogHeight = CurPosSize.Height;
        int iXPos = ((WindowWidth / 2) - (DialogWidth / 2));
        int iYPos = ((WindowHeight / 2) - (DialogHeight / 2));
        _xWindow.setPosSize(iXPos, iYPos, DialogWidth, DialogHeight, PosSize.POS);
    }



    /**
     * Creates a peer for this
     * dialog, using the active OO frame
     * as the parent window.
     * @return
     * @throws java.lang.Exception
     */
    public XWindowPeer createWindowPeer() throws com.sun.star.script.BasicErrorException{
        return createWindowPeer(null);
    }

    public void endExecute() {
        xDialog.endExecute();
    }


    public Object insertControlModel(String ServiceName, String sName, String[] sProperties, Object[] sValues) throws com.sun.star.script.BasicErrorException{
        try {
            Object oControlModel = m_xMSFDialogModel.createInstance(ServiceName);
            XMultiPropertySet xControlMultiPropertySet = UnoRuntime.queryInterface(XMultiPropertySet.class, oControlModel);
            xControlMultiPropertySet.setPropertyValues(sProperties, sValues);
            m_xDlgModelNameContainer.insertByName(sName, oControlModel);
            return oControlModel;
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
            return null;
        }
    }


    public XFixedText insertFixedText(XMouseListener _xMouseListener, int _nPosX, int _nPosY, int _nWidth, int _nStep, String _sLabel){
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
                    new Object[] { new Integer(8), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nStep), new Integer(_nWidth)});
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


    public XTextComponent insertCurrencyField(XTextListener _xTextListener, int _nPositionX, int _nPositionY, int _nWidth){
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
                    new Object[] { new Integer(12), sName, new Integer(_nPositionX), new Integer(_nPositionY), new Integer(_nWidth)});

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



    public XPropertySet insertProgressBar(int _nPosX, int _nPosY, int _nWidth, int _nProgressMax){
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
                    new Object[] { new Integer(8), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oPBModel);
            xPBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oPBModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xPBModelPSet.setPropertyValue("ProgressValueMin", new Integer(0));
            xPBModelPSet.setPropertyValue("ProgressValueMax", new Integer(_nProgressMax));
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
                    new Object[] { new Integer(8), sName, new Integer(0), new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

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



    public void insertGroupBox(int _nPosX, int _nPosY, int _nHeight, int _nWidth){
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FrameControl");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oGBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlGroupBoxModel");
            XMultiPropertySet xGBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oGBModel);

            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xGBModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "PositionX", "PositionY", "Width"},
                    new Object[] { new Integer(_nHeight), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

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



    public XTextComponent insertEditField(XTextListener _xTextListener, XFocusListener _xFocusListener, int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] { new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), "MyText", new Integer(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oTFModel);
            XPropertySet xTFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oTFModel);

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xTFModelPSet.setPropertyValue("EchoChar", new Short((short) '*'));
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

    public XPropertySet insertTimeField(int _nPosX, int _nPosY, int _nWidth, long _nTime, long _nTimeMin, long _nTimeMax){
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
                    new Object[] { new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), Boolean.TRUE, new Integer(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oTFModel);
            xTFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oTFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xTFModelPSet.setPropertyValue("TimeFormat", new Short((short) 5));
            xTFModelPSet.setPropertyValue("TimeMin", new Long(_nTimeMin));
            xTFModelPSet.setPropertyValue("TimeMax", new Long(_nTimeMax));
            xTFModelPSet.setPropertyValue("Time", new Long(_nTime));
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



    public XPropertySet insertDateField(XSpinListener _xSpinListener, int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] {Boolean.TRUE, new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oDFModel);
            xDFModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oDFModel);

            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xDFModelPSet.setPropertyValue("DateFormat", new Short((short) 7));
            xDFModelPSet.setPropertyValue("DateMin", new Integer(20070401));
            xDFModelPSet.setPropertyValue("DateMax", new Integer(20070501));
            xDFModelPSet.setPropertyValue("Date", new Integer(20000415));
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


    public XPropertySet insertPatternField(int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] { new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

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


    public XPropertySet insertNumericField( int _nPosX, int _nPosY, int _nWidth,
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
                    new Object[] { new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), Boolean.TRUE, Boolean.TRUE, new Integer(_nWidth)});

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
            xNFModelPSet.setPropertyValue("DecimalAccuracy", new Short(_nDecimalAccuracy));
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



    public XPropertySet insertVerticalScrollBar(XAdjustmentListener _xAdjustmentListener, int _nPosX, int _nPosY, int _nHeight){
        XPropertySet xSBModelPSet = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "ScrollBar");

            Integer NOrientation = new Integer(com.sun.star.awt.ScrollBarOrientation.VERTICAL);

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oSBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlScrollBarModel");
            XMultiPropertySet xSBModelMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oSBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xSBModelMPSet.setPropertyValues(
                    new String[] {"Height", "Name", "Orientation", "PositionX", "PositionY", "Width"},
                    new Object[] { new Integer(_nHeight), sName, NOrientation, new Integer(_nPosX), new Integer(_nPosY), new Integer(8)});

            // The controlmodel is not really available until inserted to the Dialog container
            m_xDlgModelNameContainer.insertByName(sName, oSBModel);

            xSBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, oSBModel);
            // The following properties may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            xSBModelPSet.setPropertyValue("ScrollValueMin", new Integer(0));
            xSBModelPSet.setPropertyValue("ScrollValueMax", new Integer(100));
            xSBModelPSet.setPropertyValue("ScrollValue", new Integer(5));
            xSBModelPSet.setPropertyValue("LineIncrement", new Integer(2));
            xSBModelPSet.setPropertyValue("BlockIncrement", new Integer(10));

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
        String sIncSuffix = "";
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


    public XCheckBox insertCheckBox(XItemListener _xItemListener, int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] {new Integer(8), "~Include page number", sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nWidth)});

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xCBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, xCBMPSet);
            xCBModelPSet.setPropertyValue("TriState", Boolean.TRUE);
            xCBModelPSet.setPropertyValue("State", new Short((short) 1));

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




    public void insertRadioButtonGroup(short _nTabIndex, int _nPosX, int _nPosY, int _nWidth){
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "OptionButton");

            // create a controlmodel at the multiservicefactory of the dialog model...
            Object oRBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlRadioButtonModel");
            XMultiPropertySet xRBMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oRBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xRBMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "State", "TabIndex", "Width" } ,
                    new Object[] {new Integer(8), "~First Option", sName, new Integer(_nPosX), new Integer(_nPosY), new Short((short) 1), new Short(_nTabIndex++),new Integer(_nWidth)});
            // add the model to the NameContainer of the dialog model
            m_xDlgModelNameContainer.insertByName(sName, oRBModel);

            // create a unique name by means of an own implementation...
            sName = createUniqueName(m_xDlgModelNameContainer, "OptionButton");

            oRBModel = m_xMSFDialogModel.createInstance("com.sun.star.awt.UnoControlRadioButtonModel");
            xRBMPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, oRBModel);
            // Set the properties at the model - keep in mind to pass the property names in alphabetical order!
            xRBMPSet.setPropertyValues(
                    new String[]  {"Height", "Label", "Name", "PositionX", "PositionY", "TabIndex", "Width" } ,
                    new Object[] {new Integer(8), "~Second Option", sName, new Integer(130), new Integer(214), new Short(_nTabIndex), new Integer(150)});
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


    public XListBox insertListBox(int _nPosX, int _nPosY, int _nWidth, int _nStep, String[] _sStringItemList){
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
                    new Object[] {Boolean.TRUE, new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), new Integer(_nStep), _sStringItemList, new Integer(_nWidth)});
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


    public XComboBox insertComboBox(int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] {Boolean.TRUE, new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), sStringItemList, new Integer(_nWidth)});

            // The following property may also be set with XMultiPropertySet but we
            // use the XPropertySet interface merely for reasons of demonstration
            XPropertySet xCbBModelPSet = UnoRuntime.queryInterface(XPropertySet.class, xCbBModelMPSet);
            xCbBModelPSet.setPropertyValue("MaxTextLen", new Short((short) 10));
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



    public XPropertySet insertFormattedField(XSpinListener _xSpinListener, int _nPosX, int _nPosY, int _nWidth){
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
                    new Object[] { new Double(12348), new Integer(12), sName, new Integer(_nPosX), new Integer(_nPosY), Boolean.TRUE, Boolean.TRUE, new Integer(_nWidth)});

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
            xFFModelPSet.setPropertyValue("FormatKey", new Integer(nFormatKey));

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

    public void convertUnits(){
    //    iXPixelFactor = (int) (100000/xDevice.getInfo().PixelPerMeterX);
    }


    public XTextComponent insertFileControl(XTextListener _xTextListener, int _nPosX, int _nPosY, int _nWidth){
        XTextComponent xTextComponent = null;
        try{
            // create a unique name by means of an own implementation...
            String sName = createUniqueName(m_xDlgModelNameContainer, "FileControl");

            // retrieve the configured Work path...
            Object oPathSettings = m_xMCF.createInstanceWithContext("com.sun.star.util.PathSettings",m_xContext);
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
                    new Object[] { new Integer(14), sName, new Integer(_nPosX), new Integer(_nPosY), sSystemWorkPath, new Integer(_nWidth)});

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
                    new Object[] {new Integer(14), _sLabel, sName, new Integer(_nPosX), new Integer(_nPosY), new Short(_nPushButtonType), new Integer(_nWidth)});

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

    /** gets the WindowPeer of a frame
     *  @param _XTextDocument the instance of a textdocument
     *  @return the windowpeer of the frame
     */
    public XWindowPeer getWindowPeer(XTextDocument _xTextDocument){
        XModel xModel =  UnoRuntime.queryInterface(XModel.class, _xTextDocument);
        XFrame xFrame = xModel.getCurrentController().getFrame();
        XWindow xWindow = xFrame.getContainerWindow();
        XWindowPeer xWindowPeer =  UnoRuntime.queryInterface(XWindowPeer.class, xWindow);
        return xWindowPeer;
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
                //...
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
            throw new java.lang.RuntimeException("cannot happen...");
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
            xModelPropertySet.setPropertyValue("Enabled", new Boolean(bdoEnable));
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
    XSingleServiceFactory m_xSSFRoadmap;
    XIndexContainer m_xRMIndexCont;

    public void addRoadmap(XItemListener _xItemListener) {
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
                    new Object[] {Boolean.FALSE, new Integer(nDialogHeight - 26), sRoadmapName, new Integer(0), new Integer(0), "Steps", new Integer(85)});
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

    /**
     *To fully understand the example one has to be aware that the passed ???Index??? parameter
     * refers to the position of the roadmap item in the roadmapmodel container
     * whereas the variable ???_ID??? directyl references to a certain step of dialog.
     */
    public void insertRoadmapItem(int Index, boolean _bEnabled, String _sLabel, int _ID) {
        try {
            // a roadmap is a SingleServiceFactory that can only create roadmapitems that are the only possible
            // element types of the container
            Object oRoadmapItem = m_xSSFRoadmap.createInstance();
            XPropertySet xRMItemPSet = UnoRuntime.queryInterface(XPropertySet.class, oRoadmapItem);
            xRMItemPSet.setPropertyValue("Label", _sLabel);
            // sometimes steps are supposed to be set disabled depending on the program logic...
            xRMItemPSet.setPropertyValue("Enabled", new Boolean(_bEnabled));
            // in this context the "ID" is meant to refer to a step of the dialog
            xRMItemPSet.setPropertyValue("ID", new Integer(_ID));
            m_xRMIndexCont.insertByIndex(Index, oRoadmapItem);
        } catch (com.sun.star.uno.Exception exception) {
            exception.printStackTrace(System.err);
        }
    }


    public void keyReleased(KeyEvent keyEvent) {
        int i = keyEvent.KeyChar;
        int n = keyEvent.KeyCode;
        int m = keyEvent.KeyFunc;
    }

    public void keyPressed(KeyEvent keyEvent) {
    }

}
