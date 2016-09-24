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
package com.sun.star.wizards.ui;

import com.sun.star.awt.*;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.*;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.*;
import com.sun.star.wizards.common.*;
import java.util.HashMap;

public class UnoDialog
{

    public XMultiServiceFactory xMSF;
    XMultiServiceFactory MSFDialogModel;
    private XNameContainer xDlgNames;
    XControlContainer xDlgContainer;
    private XNameAccess m_xDlgNameAccess;
    public XControl xControl;
    public XDialog xDialog;
    public XReschedule xReschedule;
    public XWindow xWindow;
    public XComponent xComponent;
    public XInterface xDialogModel;
    private XInterface xUnoDialog;
    private XVclWindowPeer xVclWindowPeer;
    public HashMap<String, Integer> ControlList;
    public Resource m_oResource;
    public XWindowPeer xWindowPeer = null;
    private PeerConfig m_oPeerConfig;

    public UnoDialog(XMultiServiceFactory xMSF)
    {
        try
        {
            this.xMSF = xMSF;
            ControlList = new HashMap<String, Integer>();
            xDialogModel = (XInterface) xMSF.createInstance("com.sun.star.awt.UnoControlDialogModel");
            XMultiPropertySet xMultiPSetDlg = UnoRuntime.queryInterface(XMultiPropertySet.class, xDialogModel);
            xMultiPSetDlg.setPropertyValues(new String[0], new Object[0]);
            MSFDialogModel = UnoRuntime.queryInterface(XMultiServiceFactory.class, xDialogModel);
            xUnoDialog = (XInterface) xMSF.createInstance("com.sun.star.awt.UnoControlDialog");
            xControl = UnoRuntime.queryInterface(XControl.class, xUnoDialog);
            XControlModel xControlModel = UnoRuntime.queryInterface(XControlModel.class, xDialogModel);
            xControl.setModel(xControlModel);
            UnoRuntime.queryInterface(XPropertySet.class, xDialogModel);
            xDlgContainer = UnoRuntime.queryInterface(XControlContainer.class, xUnoDialog);
            xDlgNames = UnoRuntime.queryInterface(XNameContainer.class, xDialogModel);
            xComponent = UnoRuntime.queryInterface(XComponent.class, xUnoDialog);
            xWindow = UnoRuntime.queryInterface(XWindow.class, xUnoDialog);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public int getControlKey(Object EventObject, HashMap<String, Integer> ControlList)
    {
        int iKey;
        XControl xContrl = UnoRuntime.queryInterface(XControl.class, EventObject);
        XControlModel xControlModel = xContrl.getModel();
        XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
        try
        {
            String sName = (String) xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME);
            Integer KeyObject = ControlList.get(sName);
            iKey = KeyObject.intValue();
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            iKey = 2000;
        }
        return iKey;
    }

    private void createPeerConfiguration()
    {
        m_oPeerConfig = new PeerConfig(this);
    }

    public PeerConfig getPeerConfiguration()
    {
        if (m_oPeerConfig == null)
        {
            createPeerConfiguration();
        }
        return m_oPeerConfig;
    }

    XNameAccess getDlgNameAccess()
    {
        if (m_xDlgNameAccess == null)
        {
            m_xDlgNameAccess = UnoRuntime.queryInterface(XNameAccess.class, xDialogModel);
        }
        return m_xDlgNameAccess;
    }
    public void setControlProperty(String ControlName, String PropertyName, Object PropertyValue)
    {
        try
        {
            if (PropertyValue != null)
            {
                if (!getDlgNameAccess().hasByName(ControlName))
                {
                    return;
                }
                Object xControlModel = getDlgNameAccess().getByName(ControlName);
                XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
                if (AnyConverter.isArray(PropertyValue))
                {
                    com.sun.star.uno.Type seqType = new com.sun.star.uno.Type(PropertyValue.getClass());
                    PropertyValue = new com.sun.star.uno.Any(seqType, PropertyValue);
                }
                xPSet.setPropertyValue(PropertyName, PropertyValue);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public Resource getResource()
    {
        return m_oResource;
    }

    public void setControlProperties(String ControlName, String[] PropertyNames, Object[] PropertyValues)
    {
        try
        {
            if (PropertyValues != null)
            {
                if (!getDlgNameAccess().hasByName(ControlName))
                {
                    return;
                }
                Object xControlModel = getDlgNameAccess().getByName(ControlName);
                XMultiPropertySet xMultiPSet = UnoRuntime.queryInterface(XMultiPropertySet.class, xControlModel);
                xMultiPSet.setPropertyValues(PropertyNames, PropertyValues);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public Object getControlProperty(String ControlName, String PropertyName)
    {
        try
        {
            Object xControlModel = getDlgNameAccess().getByName(ControlName);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            return xPSet.getPropertyValue(PropertyName);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }



    public double getMAPConversionFactor(String ControlName)
    {
        XControl xControl2 = xDlgContainer.getControl(ControlName);
        XView xView = UnoRuntime.queryInterface(XView.class, xControl2);
        Size aSize = xView.getSize();
        double dblMAPWidth = ((Integer) Helper.getUnoPropertyValue(xControl2.getModel(), PropertyNames.PROPERTY_WIDTH)).intValue();
        return ((aSize.Width) / dblMAPWidth);
    }

    public Size getpreferredLabelSize(String LabelName, String sLabel)
    {
        XControl xControl2 = xDlgContainer.getControl(LabelName);
        XFixedText xFixedText = UnoRuntime.queryInterface(XFixedText.class, xControl2);
        String OldText = xFixedText.getText();
        xFixedText.setText(sLabel);
        XLayoutConstrains xLayoutConstrains = UnoRuntime.queryInterface(XLayoutConstrains.class, xControl2);
        Size aSize = xLayoutConstrains.getPreferredSize();
        xFixedText.setText(OldText);
        return aSize;
    }

    public void removeSelectedItems(XListBox xListBox)
    {
        short[] SelList = xListBox.getSelectedItemsPos();
        int Sellen = SelList.length;
        for (int i = Sellen - 1; i >= 0; i--)
        {
            xListBox.removeItems(SelList[i], (short) 1);
        }
    }

    public static int getListBoxItemCount(XListBox _xListBox)
    {
        // This function may look ugly, but this is the only way to check the count
        // of values in the model,which is always right.
        // the control is only a view and could be right or not.
        final String[] fieldnames = (String[]) Helper.getUnoPropertyValue(getModel(_xListBox), PropertyNames.STRING_ITEM_LIST);
        return fieldnames.length;
    }

    public static short getSelectedItemPos(XListBox _xListBox)
    {
        short ipos[] = (short[]) Helper.getUnoPropertyValue(getModel(_xListBox), PropertyNames.SELECTED_ITEMS);
        return ipos[0];
    }

    public static boolean isListBoxSelected(XListBox _xListBox)
    {
        short ipos[] = (short[]) Helper.getUnoPropertyValue(getModel(_xListBox), PropertyNames.SELECTED_ITEMS);
        return ipos.length > 0;
    }



    public XFixedText insertLabel(String sName, String[] sPropNames, Object[] oPropValues)
    {
        try
        {
            Object oFixedText = insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", sName, sPropNames, oPropValues);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oFixedText);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            Object oLabel = xDlgContainer.getControl(sName);
            return UnoRuntime.queryInterface(XFixedText.class, oLabel);
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
            return null;
        }
    }

    public XButton insertButton(String sName, int iControlKey, XActionListener xActionListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        Object oButtonModel = insertControlModel("com.sun.star.awt.UnoControlButtonModel", sName, sProperties, sValues);
        XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        Object objectButton = xDlgContainer.getControl(sName);
        XButton xButton = UnoRuntime.queryInterface(XButton.class, objectButton);
        if (xActionListener != null)
        {
            xButton.addActionListener(xActionListener);
        }
        Integer ControlKey = Integer.valueOf(iControlKey);
        if (ControlList != null)
        {
            ControlList.put(sName, ControlKey);
        }
        return xButton;
    }

    public XScrollBar insertScrollBar(String sName, int iControlKey, XAdjustmentListener xAdjustmentListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            Object oScrollModel = insertControlModel("com.sun.star.awt.UnoControlScrollBarModel", sName, sProperties, sValues);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oScrollModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            Object oScrollBar = xDlgContainer.getControl(sName);
            XScrollBar xScrollBar = UnoRuntime.queryInterface(XScrollBar.class, oScrollBar);
            if (xAdjustmentListener != null)
            {
                xScrollBar.addAdjustmentListener(xAdjustmentListener);
            }
            Integer ControlKey = Integer.valueOf(iControlKey);
            if (ControlList != null)
            {
                ControlList.put(sName, ControlKey);
            }
            return xScrollBar;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public XTextComponent insertTextField(String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        return insertEditField("com.sun.star.awt.UnoControlEditModel", sName, iControlKey, xTextListener, sProperties, sValues);
    }

    public XTextComponent insertFormattedField(String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        return insertEditField("com.sun.star.awt.UnoControlFormattedFieldModel", sName, iControlKey, xTextListener, sProperties, sValues);
    }

    private XTextComponent insertEditField(String ServiceName, String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            XInterface xTextModel = insertControlModel(ServiceName, sName, sProperties, sValues);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xTextModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            XControl xTextField = xDlgContainer.getControl(sName);
            XTextComponent xTextBox = UnoRuntime.queryInterface(XTextComponent.class, xTextField);
            if (xTextListener != null)
            {
                xTextBox.addTextListener(xTextListener);
            }
            Integer ControlKey = Integer.valueOf(iControlKey);
            ControlList.put(sName, ControlKey);
            return xTextBox;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public XListBox insertListBox(String sName, int iControlKey, XActionListener xActionListener, XItemListener xItemListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        XInterface xListBoxModel = insertControlModel("com.sun.star.awt.UnoControlListBoxModel", sName, sProperties, sValues);
        XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, xListBoxModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        XControl xControlListBox = xDlgContainer.getControl(sName);
        XListBox xListBox = UnoRuntime.queryInterface(XListBox.class, xControlListBox);
        if (xItemListener != null)
        {
            xListBox.addItemListener(xItemListener);
        }
        if (xActionListener != null)
        {
            xListBox.addActionListener(xActionListener);
        }
        Integer ControlKey = Integer.valueOf(iControlKey);
        ControlList.put(sName, ControlKey);
        return xListBox;
    }

    public XRadioButton insertRadioButton(String sName, int iControlKey, XItemListener xItemListener, String[] sProperties, Object[] sValues)
    {
        XRadioButton xRadioButton = insertRadioButton(sName, iControlKey, sProperties, sValues);
        if (xItemListener != null)
        {
            xRadioButton.addItemListener(xItemListener);
        }
        return xRadioButton;
    }

    public XButton insertRadioButton(String sName, int iControlKey, XActionListener xActionListener, String[] sProperties, Object[] sValues)
    {
        XRadioButton xRadioButton = insertRadioButton(sName, iControlKey, sProperties, sValues);
        XButton xButton = UnoRuntime.queryInterface(XButton.class, xRadioButton);
        if (xActionListener != null)
        {
            xButton.addActionListener(xActionListener);
        }
        return xButton;
    }

    public XRadioButton insertRadioButton(String sName, int iControlKey, String[] sProperties, Object[] sValues)
    {
        XRadioButton xRadioButton = insertRadioButton(sName, sProperties, sValues);
        Integer ControlKey = Integer.valueOf(iControlKey);
        ControlList.put(sName, ControlKey);
        return xRadioButton;
    }

    public XRadioButton insertRadioButton(String sName, String[] sProperties, Object[] sValues)
    {
        try
        {
            XInterface oRadioButtonModel = insertControlModel("com.sun.star.awt.UnoControlRadioButtonModel", sName, sProperties, sValues);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oRadioButtonModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            XControl xControlRadioButton = xDlgContainer.getControl(sName);
            return UnoRuntime.queryInterface(XRadioButton.class, xControlRadioButton);
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    /**
     * The problem with setting the visibility of controls is that changing the current step
     * of a dialog will automatically make all controls visible. The PropertyNames.PROPERTY_STEP property always wins against
     * the property "visible". Therefore a control meant to be invisible is placed on a step far far away.
     * Afterwards the step property of the dialog has to be set with "repaintDialogStep". As the performance
     * of that method is very bad it should be used only once for all controls
     * @param controlname the name of the control
     * @param bIsVisible sets the control visible or invisible
     */
    public void setControlVisible(String controlname, boolean bIsVisible)
    {
        try
        {
            int iCurDialogStep = AnyConverter.toInt(Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_STEP));
            if (bIsVisible)
            {
                setControlProperty(controlname, PropertyNames.PROPERTY_STEP, Integer.valueOf(iCurDialogStep));
            }
            else
            {
                setControlProperty(controlname, PropertyNames.PROPERTY_STEP, UIConsts.INVISIBLESTEP);
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    /**
     * repaints the currentDialogStep
     */
    public void repaintDialogStep()
    {
        try
        {
            int ncurstep = AnyConverter.toInt(Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_STEP));
            Helper.setUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP, 99);
            Helper.setUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP, Integer.valueOf(ncurstep));
        }
        catch (com.sun.star.lang.IllegalArgumentException exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public XInterface insertControlModel(String ServiceName, String sName, String[] sProperties, Object[] sValues)
    {
        try
        {
            XInterface xControlModel = (XInterface) MSFDialogModel.createInstance(ServiceName);
            Helper.setUnoPropertyValues(xControlModel, sProperties, sValues);
            xDlgNames.insertByName(sName, xControlModel);
            return xControlModel;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public void setFocus(String ControlName)
    {
        Object oFocusControl = xDlgContainer.getControl(ControlName);
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, oFocusControl);
        xWindow.setFocus();
    }

    public void selectListBoxItem(XListBox xListBox, short iFieldsSelIndex)
    {
        if (iFieldsSelIndex > -1)
        {
            int FieldCount = xListBox.getItemCount();
            if (FieldCount > 0)
            {
                if (iFieldsSelIndex < FieldCount)
                {
                    xListBox.selectItemPos(iFieldsSelIndex, true);
                }
                else
                {
                    xListBox.selectItemPos((short) (iFieldsSelIndex - 1), true);
                }
            }
        }
    }

    /** deselects a Listbox. MultipleMode is not supported
     */
    public static void deselectListBox(XInterface _xBasisListBox)
    {
        Object oListBoxModel = getModel(_xBasisListBox);
        Object sList = Helper.getUnoPropertyValue(oListBoxModel, PropertyNames.STRING_ITEM_LIST);
        Helper.setUnoPropertyValue(oListBoxModel, PropertyNames.STRING_ITEM_LIST, new String[]
                {
                });
        Helper.setUnoPropertyValue(oListBoxModel, PropertyNames.STRING_ITEM_LIST, sList);
    }

    public void calculateDialogPosition(Rectangle FramePosSize)
    {
        // TODO: check if it would be useful or possible to create a dialog peer, that can be used for the messageboxes to
        // maintain modality when they pop up.
        Rectangle CurPosSize = xWindow.getPosSize();
        int WindowHeight = FramePosSize.Height;
        int WindowWidth = FramePosSize.Width;
        int DialogWidth = CurPosSize.Width;
        int DialogHeight = CurPosSize.Height;
        int iXPos = ((WindowWidth / 2) - (DialogWidth / 2));
        int iYPos = ((WindowHeight / 2) - (DialogHeight / 2));
        xWindow.setPosSize(iXPos, iYPos, DialogWidth, DialogHeight, PosSize.POS);
    }

    /**
     *
     * @return 0 for cancel, 1 for ok
     */
    public short executeDialog(Rectangle FramePosSize) throws com.sun.star.uno.Exception
    {
        if (xControl.getPeer() == null)
        {
            throw new java.lang.IllegalArgumentException("Please create a peer, using your own frame");
        }
        calculateDialogPosition(FramePosSize);
        if (xWindowPeer == null)
        {
            createWindowPeer();
        }
        xVclWindowPeer = UnoRuntime.queryInterface(XVclWindowPeer.class, xWindowPeer);
        this.BisHighContrastModeActivated = Boolean.valueOf(this.isHighContrastModeActivated());
        xDialog = UnoRuntime.queryInterface(XDialog.class, xUnoDialog);
        return xDialog.execute();
    }

    /**
     * @return 0 for cancel, 1 for ok.
     */
    public short executeDialog(XInterface xComponent) throws com.sun.star.uno.Exception
    {
        XFrame frame = UnoRuntime.queryInterface(XFrame.class, xComponent);
        if (frame != null)
        {
            XWindow w = frame.getComponentWindow();
            if (w != null)
            {
                return executeDialog(w.getPosSize());
            }
        }

        return executeDialog(new Rectangle(0, 0, 640, 400));
    }

    /**
     * When possible, use the other executeDialog methods, since
     * there may be problems retrieving the actual active frame,
     * for example under linux.
     * @return 0 for cancel, 1 for ok
     */
    public short executeDialog() throws com.sun.star.uno.Exception
    {
        return executeDialog(Desktop.getActiveFrame(xMSF));
    }

    public void modifyFontWeight(String ControlName, float FontWeight)
    {
        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Weight = FontWeight;
        setControlProperty(ControlName, PropertyNames.FONT_DESCRIPTOR, oFontDesc);
    }

    /**
     * create a peer for this dialog, using the given peer as a parent.
     */
    public XWindowPeer createWindowPeer(XWindowPeer parentPeer) throws com.sun.star.uno.Exception
    {
        xWindow.setVisible(false);
        Object tk = xMSF.createInstance("com.sun.star.awt.Toolkit");
        if (parentPeer == null)
        {
            parentPeer = UnoRuntime.queryInterface(XToolkit.class, tk).getDesktopWindow();
        }
        XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, tk);
        xReschedule = UnoRuntime.queryInterface(XReschedule.class, xToolkit);
        // TEUER!
        xControl.createPeer(xToolkit, parentPeer);
        xWindowPeer = xControl.getPeer();
        return xControl.getPeer();
    }

    /**
     * Creates a peer for this dialog, using the active OO frame as the parent window.
     */
    public XWindowPeer createWindowPeer() throws com.sun.star.uno.Exception
    {
        return createWindowPeer(null);
    }

    public static Object getModel(Object control)
    {
        XControl xControl = UnoRuntime.queryInterface(XControl.class, control);
        return xControl.getModel();
    }

    public static void setEnabled(Object control, boolean enabled)
    {
        setEnabled(control, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    private static void setEnabled(Object control, Boolean enabled)
    {
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_ENABLED, enabled);
    }

    /**
     * @param oControlModel the model of a control
     * @return the LabelType according to UIConsts.CONTROLTYPE
     */
    private static int getControlModelType(Object oControlModel)
    {
        XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, oControlModel);
        if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlFixedTextModel"))
        {
            return UIConsts.CONTROLTYPE.FIXEDTEXT;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlButtonModel"))
        {
            return UIConsts.CONTROLTYPE.BUTTON;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlCurrencyFieldModel"))
        {
            return UIConsts.CONTROLTYPE.CURRENCYFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlDateFieldModel"))
        {
            return UIConsts.CONTROLTYPE.DATEFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlFixedLineModel"))
        {
            return UIConsts.CONTROLTYPE.FIXEDLINE;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlFormattedFieldModel"))
        {
            return UIConsts.CONTROLTYPE.FORMATTEDFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlRoadmapModel"))
        {
            return UIConsts.CONTROLTYPE.ROADMAP;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlNumericFieldModel"))
        {
            return UIConsts.CONTROLTYPE.NUMERICFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlPatternFieldModel"))
        {
            return UIConsts.CONTROLTYPE.PATTERNFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlHyperTextModel"))
        {
            return UIConsts.CONTROLTYPE.HYPERTEXT;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlProgressBarModel"))
        {
            return UIConsts.CONTROLTYPE.PROGRESSBAR;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlTimeFieldModel"))
        {
            return UIConsts.CONTROLTYPE.TIMEFIELD;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlImageControlModel"))
        {
            return UIConsts.CONTROLTYPE.IMAGECONTROL;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlRadioButtonModel"))
        {
            return UIConsts.CONTROLTYPE.RADIOBUTTON;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlCheckBoxModel"))
        {
            return UIConsts.CONTROLTYPE.CHECKBOX;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlEditModel"))
        {
            return UIConsts.CONTROLTYPE.EDITCONTROL;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlComboBoxModel"))
        {
            return UIConsts.CONTROLTYPE.COMBOBOX;
        }
        else if (xServiceInfo.supportsService("com.sun.star.awt.UnoControlListBoxModel")) // TODO Comboboxes??
        {
            return UIConsts.CONTROLTYPE.LISTBOX;
        }
        else
        {
            return UIConsts.CONTROLTYPE.UNKNOWN;
        }
    }

    /**
     * @return the name of the property that contains the value of a controlmodel
     */
    public static String getDisplayProperty(Object oControlModel)
    {
        int itype = getControlModelType(oControlModel);
        return getDisplayProperty(itype);
    }

    /**
     * @param itype The type of the control conforming to UIConst.ControlType
     * @return the name of the property that contains the value of a controlmodel
     */
    private static String getDisplayProperty(int itype)
    {
        switch (itype)
        {
            case UIConsts.CONTROLTYPE.FIXEDTEXT:
                return PropertyNames.PROPERTY_LABEL;
            case UIConsts.CONTROLTYPE.BUTTON:
                return PropertyNames.PROPERTY_LABEL;
            case UIConsts.CONTROLTYPE.FIXEDLINE:
                return PropertyNames.PROPERTY_LABEL;
            case UIConsts.CONTROLTYPE.NUMERICFIELD:
                return "Value";
            case UIConsts.CONTROLTYPE.CURRENCYFIELD:
                return "Value";
            case UIConsts.CONTROLTYPE.FORMATTEDFIELD:
                return "EffectiveValue";
            case UIConsts.CONTROLTYPE.DATEFIELD:
                return "Date";
            case UIConsts.CONTROLTYPE.TIMEFIELD:
                return "Time";
            case UIConsts.CONTROLTYPE.SCROLLBAR:
                return "ScrollValue";
            case UIConsts.CONTROLTYPE.PROGRESSBAR:
                return "ProgressValue";
            case UIConsts.CONTROLTYPE.IMAGECONTROL:
                return PropertyNames.PROPERTY_IMAGEURL;
            case UIConsts.CONTROLTYPE.RADIOBUTTON:
                return PropertyNames.PROPERTY_STATE;
            case UIConsts.CONTROLTYPE.CHECKBOX:
                return PropertyNames.PROPERTY_STATE;
            case UIConsts.CONTROLTYPE.EDITCONTROL:
                return "Text";
            case UIConsts.CONTROLTYPE.COMBOBOX:
                return "Text";
            case UIConsts.CONTROLTYPE.PATTERNFIELD:
                return "Text";
            case UIConsts.CONTROLTYPE.LISTBOX:
                return PropertyNames.SELECTED_ITEMS;
            default:
                return PropertyNames.EMPTY_STRING;
        }
    }

    public void addResourceHandler(String _Module)
    {
        m_oResource = new Resource(xMSF, _Module);
    }

    public static short setInitialTabindex(int _istep)
    {
        return (short) (_istep * 100);
    }
    private Boolean BisHighContrastModeActivated = null;

    private boolean isHighContrastModeActivated()
    {
        if (xVclWindowPeer != null)
        {
            if (BisHighContrastModeActivated == null)
            {
                int nUIColor;
                try
                {
                    nUIColor = AnyConverter.toInt(this.xVclWindowPeer.getProperty("DisplayBackgroundColor"));
                }
                catch (IllegalArgumentException e)
                {
                    e.printStackTrace(System.err);
                    return false;
                }
                //TODO: The following methods could be wrapped in an own class implementation
                int nRed = getRedColorShare(nUIColor);
                int nGreen = getGreenColorShare(nUIColor);
                int nBlue = getBlueColorShare(nUIColor);
                int nLuminance = ((nBlue * 28 + nGreen * 151 + nRed * 77) / 256);
                boolean bisactivated = (nLuminance <= 25);
                BisHighContrastModeActivated = Boolean.valueOf(bisactivated);
                return bisactivated;
            }
            else
            {
                return BisHighContrastModeActivated.booleanValue();
            }
        }
        else
        {
            return false;
        }
    }

    private static int getRedColorShare(int _nColor)
    {
        int nRed = _nColor / 65536;
        return nRed;
    }

    private static int getGreenColorShare(int _nColor)
    {
        int nRedModulo = _nColor % 65536;
        int nGreen = nRedModulo / 256;
        return nGreen;
    }

    private static int getBlueColorShare(int _nColor)
    {
        int nRedModulo = _nColor % 65536;
        int nGreenModulo = (nRedModulo % 256);
        int nBlue = nGreenModulo;
        return nBlue;
    }

    public String getWizardImageUrl(int _nResId, int _nHCResId)
    {
        if (isHighContrastModeActivated())
        {
            return "private:resource/wzi/image/" + _nHCResId;
        }
        else
        {
            return "private:resource/wzi/image/" + _nResId;
        }
    }

    public String getImageUrl(String _surl, String _shcurl)
    {
        if (isHighContrastModeActivated())
        {
            return _shcurl;
        }
        else
        {
            return _surl;
        }
    }

    public static short getListBoxLineCount()
    {
        return (short)20;
    }
}
