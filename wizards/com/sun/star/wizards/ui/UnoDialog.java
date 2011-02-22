/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
package com.sun.star.wizards.ui;

import com.sun.star.awt.*;
import com.sun.star.awt.XReschedule;
import com.sun.star.beans.Property;
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
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.ui.event.*;

import java.util.Hashtable;

public class UnoDialog implements EventNames
{

    public XMultiServiceFactory xMSF;
    public XMultiServiceFactory MSFDialogModel;
    public XNameContainer xDlgNames;
    public XControlContainer xDlgContainer;
    private XNameAccess m_xDlgNameAccess;
    public XControl xControl;
    public XDialog xDialog;
    public XReschedule xReschedule;
    public XWindow xWindow;
    public XComponent xComponent;
    public XInterface xDialogModel;
    public XInterface xUnoDialog;
    public XPropertySet xPSetDlg;
    public XVclWindowPeer xVclWindowPeer;
    public Hashtable ControlList;
    public Resource m_oResource;
    public XWindowPeer xWindowPeer = null;
    private PeerConfig m_oPeerConfig;
    protected AbstractListener guiEventListener;

    public AbstractListener getGuiEventListener()
    {
        return guiEventListener;
    }

    public UnoDialog(XMultiServiceFactory xMSF, String[] PropertyNames, Object[] PropertyValues)
    {
        try
        {
            this.xMSF = xMSF;
            ControlList = new Hashtable();
            xDialogModel = (XInterface) xMSF.createInstance("com.sun.star.awt.UnoControlDialogModel");
            XMultiPropertySet xMultiPSetDlg = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, xDialogModel);
            xMultiPSetDlg.setPropertyValues(PropertyNames, PropertyValues);
            MSFDialogModel = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xDialogModel);
            xUnoDialog = (XInterface) xMSF.createInstance("com.sun.star.awt.UnoControlDialog");
            xControl = (XControl) UnoRuntime.queryInterface(XControl.class, xUnoDialog);
            XControlModel xControlModel = (XControlModel) UnoRuntime.queryInterface(XControlModel.class, xDialogModel);
            xControl.setModel(xControlModel);
            xPSetDlg = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDialogModel);
            xDlgContainer = (XControlContainer) UnoRuntime.queryInterface(XControlContainer.class, xUnoDialog);
            xDlgNames = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xDialogModel);
            // xDlgNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDialogModel);
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xUnoDialog);
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xUnoDialog);

        // setPeerConfiguration(); // LLA: will be done, if really used!
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public int getControlKey(Object EventObject, Hashtable ControlList)
    {
        int iKey;
        XControl xContrl = (XControl) UnoRuntime.queryInterface(XControl.class, EventObject);
        XControlModel xControlModel = xContrl.getModel();
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
        try
        {
            String sName = (String) xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME);
            Integer KeyObject = (Integer) ControlList.get(sName);
            iKey = KeyObject.intValue();
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            iKey = 2000;
        }
        return iKey;
    }

    public void createPeerConfiguration()
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
            m_xDlgNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDialogModel);
        }
        return m_xDlgNameAccess;
    }
    public void setControlProperty(String ControlName, String PropertyName, Object PropertyValue)
    {
        try
        {
            if (PropertyValue != null)
            {
                if (getDlgNameAccess().hasByName(ControlName) == false)
                {
                    return;
                }
                Object xControlModel = getDlgNameAccess().getByName(ControlName);
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
                if (AnyConverter.isArray(PropertyValue))
                {
                    com.sun.star.uno.Type seqType = new com.sun.star.uno.Type(PropertyValue.getClass());
                    PropertyValue = new com.sun.star.uno.Any(seqType, PropertyValue);
                /*              PropertyValue = Helper.getArrayValue(PropertyValue);
                if (PropertyValue == null)
                PropertyValue = new short[]{};
                 */
                }
                xPSet.setPropertyValue(PropertyName, PropertyValue);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
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
                if (getDlgNameAccess().hasByName(ControlName) == false)
                {
                    return;
                }
                Object xControlModel = getDlgNameAccess().getByName(ControlName);
                XMultiPropertySet xMultiPSet = (XMultiPropertySet) UnoRuntime.queryInterface(XMultiPropertySet.class, xControlModel);
                xMultiPSet.setPropertyValues(PropertyNames, PropertyValues);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public Object getControlProperty(String ControlName, String PropertyName)
    {
        try
        {
            Object xControlModel = getDlgNameAccess().getByName(ControlName);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            Object oPropValue = xPSet.getPropertyValue(PropertyName);
            //      if (AnyConverter.isArray(oPropValue))
            //          return Helper.getArrayValue(oPropValue);
            //      else
            return oPropValue;
        }
        catch (com.sun.star.uno.Exception exception)
        { // com.sun.star.container.NoSuchElementException, com.sun.star.beans.UnknownPropertyException,
            exception.printStackTrace(System.out); // com.sun.star.lang.WrappedTargetException, com.sun.star.beans.PropertyVetoException
            return null; // com.sun.star.lang.IllegalArgumentException
        }
    }

    public void printControlProperties(String ControlName)
    {
        try
        {
            Object xControlModel = getDlgNameAccess().getByName(ControlName);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xControlModel);
            Property[] allProps = xPSet.getPropertySetInfo().getProperties();
            for (int i = 0; i < allProps.length; i++)
            {
                String sName = allProps[i].Name;
                System.out.println(sName);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        { // com.sun.star.container.NoSuchElementException, com.sun.star.beans.UnknownPropertyException,
            exception.printStackTrace(System.out); // com.sun.star.lang.WrappedTargetException, com.sun.star.beans.PropertyVetoException
        }
    }

    public double getMAPConversionFactor(String ControlName)
    {
        XControl xControl2 = xDlgContainer.getControl(ControlName);
        XView xView = (XView) UnoRuntime.queryInterface(XView.class, xControl2);
        Size aSize = xView.getSize();
        double dblMAPWidth = (double) (((Integer) Helper.getUnoPropertyValue(xControl2.getModel(), PropertyNames.PROPERTY_WIDTH)).intValue());
        double dblFactor = (((double) (aSize.Width)) / dblMAPWidth);
        return dblFactor;
    }

    public Size getpreferredLabelSize(String LabelName, String sLabel)
    {
        XControl xControl2 = xDlgContainer.getControl(LabelName);
        XFixedText xFixedText = (XFixedText) UnoRuntime.queryInterface(XFixedText.class, xControl2);
        String OldText = xFixedText.getText();
        xFixedText.setText(sLabel);
        XLayoutConstrains xLayoutConstrains = (XLayoutConstrains) UnoRuntime.queryInterface(XLayoutConstrains.class, xControl2);
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

    public void addSingleItemtoListbox(XListBox xListBox, String ListItem, short iSelIndex)
    {
        xListBox.addItem(ListItem, xListBox.getItemCount());
        if (iSelIndex != -1)
        {
            xListBox.selectItemPos(iSelIndex, true);
        }
    }

    public XFixedText insertLabel(String sName, String[] sPropNames, Object[] oPropValues)
    {
        try
        {
            Object oFixedText = insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", sName, sPropNames, oPropValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oFixedText);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            Object oLabel = xDlgContainer.getControl(new String(sName));
            return (XFixedText) UnoRuntime.queryInterface(XFixedText.class, oLabel);
        }
        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
            return null;
        }
    }

    /*    public XButton insertButton(String sName, int iControlKey, XActionListener xActionListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception{
    Object oButtonModel = insertControlModel("com.sun.star.awt.UnoControlButtonModel", sName, sProperties, sValues);
    XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
    xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
    Object objectButton = xDlgContainer.getControl( new String(sName));
    XButton xButton = ( XButton ) UnoRuntime.queryInterface( XButton.class, objectButton );
    xButton.addActionListener(xActionListener);
    Integer ControlKey = new Integer(iControlKey);
    if (ControlList != null)
    ControlList.put(sName, ControlKey);
    return xButton;
    } */
    public XButton insertButton(String sName, int iControlKey, XActionListener xActionListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        Object oButtonModel = insertControlModel("com.sun.star.awt.UnoControlButtonModel", sName, sProperties, sValues);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        Object objectButton = xDlgContainer.getControl(new String(sName));
        XButton xButton = (XButton) UnoRuntime.queryInterface(XButton.class, objectButton);
        if (xActionListener != null)
        {
            xButton.addActionListener(xActionListener);
        }
        Integer ControlKey = new Integer(iControlKey);
        if (ControlList != null)
        {
            ControlList.put(sName, ControlKey);
        }
        return xButton;
    }

    public void insertCheckBox(String sName, int iControlKey, XItemListener xItemListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        Object oButtonModel = insertControlModel("com.sun.star.awt.UnoControlCheckBoxModel", sName, sProperties, sValues);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oButtonModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        Object objectCheckBox = xDlgContainer.getControl(new String(sName));
        XCheckBox xCheckBox = (XCheckBox) UnoRuntime.queryInterface(XCheckBox.class, objectCheckBox);
        if (xItemListener != null)
        {
            xCheckBox.addItemListener(xItemListener);
        }
        Integer ControlKey = new Integer(iControlKey);
        if (ControlList != null)
        {
            ControlList.put(sName, ControlKey);
        }
    }

    public void insertNumericField(String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        Object oNumericFieldModel = insertControlModel("com.sun.star.awt.UnoControlNumericFieldModel", sName, sProperties, sValues);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oNumericFieldModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        Object objectNumericField = xDlgContainer.getControl(new String(sName));
        XTextComponent xNumericField = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, objectNumericField);
        if (xTextListener != null)
        {
            xNumericField.addTextListener(xTextListener);
        }
        Integer ControlKey = new Integer(iControlKey);
        if (ControlList != null)
        {
            ControlList.put(sName, ControlKey);
        }
    }

    public XScrollBar insertScrollBar(String sName, int iControlKey, XAdjustmentListener xAdjustmentListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            Object oScrollModel = insertControlModel("com.sun.star.awt.UnoControlScrollBarModel", sName, sProperties, sValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oScrollModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            Object oScrollBar = xDlgContainer.getControl(new String(sName));
            XScrollBar xScrollBar = (XScrollBar) UnoRuntime.queryInterface(XScrollBar.class, oScrollBar);
            if (xAdjustmentListener != null)
            {
                xScrollBar.addAdjustmentListener(xAdjustmentListener);
            }
            Integer ControlKey = new Integer(iControlKey);
            if (ControlList != null)
            {
                ControlList.put(sName, ControlKey);
            }
            return xScrollBar;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public XTextComponent insertTextField(String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        XTextComponent xTextBox = insertEditField("com.sun.star.awt.UnoControlEditModel", sName, iControlKey, xTextListener, sProperties, sValues);
        return xTextBox;
    }

    public XTextComponent insertFormattedField(String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        XTextComponent xTextBox = insertEditField("com.sun.star.awt.UnoControlFormattedFieldModel", sName, iControlKey, xTextListener, sProperties, sValues);
        return xTextBox;
    }

    public XTextComponent insertEditField(String ServiceName, String sName, int iControlKey, XTextListener xTextListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            XInterface xTextModel = insertControlModel(ServiceName, sName, sProperties, sValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            XControl xTextField = xDlgContainer.getControl(new String(sName));
            XTextComponent xTextBox = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, xTextField);
            if (xTextListener != null)
            {
                xTextBox.addTextListener(xTextListener);
            }
            Integer ControlKey = new Integer(iControlKey);
            ControlList.put(sName, ControlKey);
            return xTextBox;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public XListBox insertListBox(String sName, int iControlKey, XActionListener xActionListener, XItemListener xItemListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        XInterface xListBoxModel = insertControlModel("com.sun.star.awt.UnoControlListBoxModel", sName, sProperties, sValues);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xListBoxModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        XControl xControlListBox = xDlgContainer.getControl(new String(sName));
        XListBox xListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, xControlListBox);
        if (xItemListener != null)
        {
            xListBox.addItemListener(xItemListener);
        }
        if (xActionListener != null)
        {
            xListBox.addActionListener(xActionListener);
        }
        Integer ControlKey = new Integer(iControlKey);
        ControlList.put(sName, ControlKey);
        return xListBox;
    }

    public XComboBox insertComboBox(String sName, int iControlKey, XActionListener xActionListener, XTextListener xTextListener, XItemListener xItemListener, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        XInterface xComboBoxModel = insertControlModel("com.sun.star.awt.UnoControlComboBoxModel", sName, sProperties, sValues);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xComboBoxModel);
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
        XControl xControlComboBox = xDlgContainer.getControl(new String(sName));
        XComboBox xComboBox = (XComboBox) UnoRuntime.queryInterface(XComboBox.class, xControlComboBox);
        if (xItemListener != null)
        {
            xComboBox.addItemListener(xItemListener);
        }
        if (xTextListener != null)
        {
            XTextComponent xTextComponent = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, xComboBox);
            xTextComponent.addTextListener(xTextListener);
        }
        if (xActionListener != null)
        {
            xComboBox.addActionListener(xActionListener);
        }
        Integer ControlKey = new Integer(iControlKey);
        ControlList.put(sName, ControlKey);
        return xComboBox;
    }

    public XRadioButton insertRadioButton(String sName, int iControlKey, XItemListener xItemListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            XRadioButton xRadioButton = insertRadioButton(sName, iControlKey, sProperties, sValues);
            if (xItemListener != null)
            {
                xRadioButton.addItemListener(xItemListener);
            }
            return xRadioButton;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public XButton insertRadioButton(String sName, int iControlKey, XActionListener xActionListener, String[] sProperties, Object[] sValues)
    {
        try
        {
            XRadioButton xRadioButton = insertRadioButton(sName, iControlKey, sProperties, sValues);
            XButton xButton = (com.sun.star.awt.XButton) UnoRuntime.queryInterface(XButton.class, xRadioButton);
            if (xActionListener != null)
            {
                xButton.addActionListener(xActionListener);
            }
            return xButton;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public XRadioButton insertRadioButton(String sName, int iControlKey, String[] sProperties, Object[] sValues) throws com.sun.star.uno.Exception
    {
        XRadioButton xRadioButton = insertRadioButton(sName, sProperties, sValues);
        Integer ControlKey = new Integer(iControlKey);
        ControlList.put(sName, ControlKey);
        return xRadioButton;
    }

    public XRadioButton insertRadioButton(String sName, String[] sProperties, Object[] sValues)
    {
        try
        {
            XInterface oRadioButtonModel = insertControlModel("com.sun.star.awt.UnoControlRadioButtonModel", sName, sProperties, sValues);
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oRadioButtonModel);
            xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName);
            XControl xControlRadioButton = xDlgContainer.getControl(new String(sName));
            XRadioButton xRadioButton = (XRadioButton) UnoRuntime.queryInterface(XRadioButton.class, xControlRadioButton);
            return xRadioButton;
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    /**
     * @author bc93774
     * The problem with setting the visibility of controls is that changing the current step
     * of a dialog will automatically make all controls visible. The PropertyNames.PROPERTY_STEP property always wins against
     * the property "visible". Therfor a control meant to be invisible is placed on a step far far away.
     * @param the name of the control
     * @param iStep  change the step if you want to make the control invisible
     */
    private void setControlVisible(String controlname, int iStep)
    {
        try
        {
            int iCurStep = AnyConverter.toInt(getControlProperty(controlname, PropertyNames.PROPERTY_STEP));
            setControlProperty(controlname, PropertyNames.PROPERTY_STEP, new Integer(iStep));
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    /**
     * @author bc93774
     * The problem with setting the visibility of controls is that changing the current step
     * of a dialog will automatically make all controls visible. The PropertyNames.PROPERTY_STEP property always wins against
     * the property "visible". Therfor a control meant to be invisible is placed on a step far far away.
     * Afterwards the step property of the dialog has to be set with "repaintDialogStep". As the performance
     * of that method is very bad it should be used only once for all controls
     * @param controlname the name of the control
     * @param bIsVisible sets the control visible or invisible
     */
    public void setControlVisible(String controlname, boolean bIsVisible)
    {
        try
        {
            int iCurControlStep = AnyConverter.toInt(getControlProperty(controlname, PropertyNames.PROPERTY_STEP));
            int iCurDialogStep = AnyConverter.toInt(Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_STEP));
            int iNewStep;
            if (bIsVisible)
            {
                setControlProperty(controlname, PropertyNames.PROPERTY_STEP, new Integer(iCurDialogStep));
            }
            else
            {
                setControlProperty(controlname, PropertyNames.PROPERTY_STEP, UIConsts.INVISIBLESTEP);
            }
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    /**
     * repaints the currentDialogStep
     * @author bc93774
     */
    public void repaintDialogStep()
    {
        try
        {
            int ncurstep = AnyConverter.toInt(Helper.getUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_STEP));
            Helper.setUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP, 99);
            Helper.setUnoPropertyValue(xDialogModel, PropertyNames.PROPERTY_STEP, new Integer(ncurstep));
        }
        catch (com.sun.star.uno.Exception exception)
        {
            exception.printStackTrace(System.out);
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
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public void setFocus(String ControlName)
    {
        Object oFocusControl = xDlgContainer.getControl(ControlName);
        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, oFocusControl);
        xWindow.setFocus();
    }

    public static String[] combineListboxList(String sFirstEntry, String[] MainList)
    {
        try
        {
            String[] FirstList = new String[]
            {
                sFirstEntry
            };
            String[] ResultList = new String[MainList.length + 1];
            System.arraycopy(FirstList, 0, ResultList, 0, 1);
            System.arraycopy(MainList, 0, ResultList, 1, MainList.length);
            return ResultList;
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.out);
            return null;
        }
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
     *
     * @param _xBasisListBox
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
        // Todo: check if it would be useful or possible to create a dialog peer, that can be used for the messageboxes to
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
     * @param FramePosSize
     * @return 0 for cancel, 1 for ok
     * @throws com.sun.star.uno.Exception
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
        xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, xWindowPeer);
//      xVclWindowPeer.setProperty("AutoMnemonics", new Boolean(true));
        this.BisHighContrastModeActivated = new Boolean(this.isHighContrastModeActivated());
        xDialog = (XDialog) UnoRuntime.queryInterface(XDialog.class, xUnoDialog);
        return xDialog.execute();
    }

    public void setVisible(UnoDialog parent) throws com.sun.star.uno.Exception
    {
        calculateDialogPosition(parent.xWindow.getPosSize());
        if (xWindowPeer == null)
        {
            createWindowPeer();
        }
        XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, xWindowPeer);
        xDialog = (XDialog) UnoRuntime.queryInterface(XDialog.class, xUnoDialog);
        this.xWindow.setVisible(true);
    }

    /**
     *
     * @param parent
     * @return 0 for cancel, 1 for ok.
     * @throws com.sun.star.uno.Exception
     */
    public short executeDialog(UnoDialog parent)
            throws com.sun.star.uno.Exception
    {
        return executeDialog(parent.xWindow.getPosSize());
    }

    /**
     *
     * @param xComponent
     * @return 0 for cancel, 1 for ok.
     * @throws com.sun.star.uno.Exception
     */
    public short executeDialog(XInterface xComponent) throws com.sun.star.uno.Exception
    {
        XFrame frame = (XFrame) UnoRuntime.queryInterface(XFrame.class, xComponent);
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

    public void setAutoMnemonic(String ControlName, boolean bValue)
    {
        Object oControl = xDlgContainer.getControl(ControlName);
        xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oControl);
        XWindowPeer xWindowPeer = xControl.getPeer();
        XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, xControl.getPeer());
        xVclWindowPeer.setProperty("AutoMnemonics", new Boolean(bValue));
    }

    public void modifyFontWeight(String ControlName, float FontWeight)
    {
        FontDescriptor oFontDesc = new FontDescriptor();
        oFontDesc.Weight = FontWeight;
        setControlProperty(ControlName, PropertyNames.FONT_DESCRIPTOR, oFontDesc);
    }

    /**
     * create a peer for this
     * dialog, using the given
     * peer as a parent.
     * @param parentPeer
     * @return
     * @throws java.lang.Exception
     */
    public XWindowPeer createWindowPeer(XWindowPeer parentPeer) throws com.sun.star.uno.Exception
    {
        xWindow.setVisible(false);
        Object tk = xMSF.createInstance("com.sun.star.awt.Toolkit");
        if (parentPeer == null)
        {
            parentPeer = ((XToolkit) UnoRuntime.queryInterface(XToolkit.class, tk)).getDesktopWindow();
        }
        XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class, tk);
        xReschedule = (XReschedule) UnoRuntime.queryInterface(XReschedule.class, xToolkit);
        // TEUER!
        xControl.createPeer(xToolkit, parentPeer);
        xWindowPeer = xControl.getPeer();
        return xControl.getPeer();
    }

    /**
     * Creates a peer for this
     * dialog, using the active OO frame
     * as the parent window.
     * @return
     * @throws java.lang.Exception
     */
    public XWindowPeer createWindowPeer() throws com.sun.star.uno.Exception
    {
        return createWindowPeer(null);
    }

    // deletes the first entry when this is equal to "DelEntryName"
    // returns true when a new item is selected
    public void deletefirstListboxEntry(String ListBoxName, String DelEntryName)
    {
        XControl xListControl = xDlgContainer.getControl(ListBoxName);
        XListBox xListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, xListControl);
        String FirstItem = xListBox.getItem((short) 0);
        if (FirstItem.equals(DelEntryName))
        {
            short SelPos = xListBox.getSelectedItemPos();
            xListBox.removeItems((short) 0, (short) 1);
            if (SelPos > 0)
            {
                setControlProperty(ListBoxName, PropertyNames.SELECTED_ITEMS, new short[SelPos]);
                xListBox.selectItemPos((short) (SelPos - 1), true);
            }
        }
    }

    public void setPeerProperty(String ControlName, String PropertyName, Object PropertyValue)
    {
        Object oControl = xDlgContainer.getControl(ControlName);
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oControl);
        XWindowPeer xControlPeer = xControl.getPeer();
        XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, xControlPeer);
        xVclWindowPeer.setProperty(PropertyName, PropertyValue);
    }

    public static Object getModel(Object control)
    {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, control);
        XControlModel xModel = xControl.getModel();
        return xModel;
    }

    public static void setEnabled(Object control, boolean enabled)
    {
        setEnabled(control, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public static void setEnabled(Object control, Boolean enabled)
    {
        Helper.setUnoPropertyValue(getModel(control), PropertyNames.PROPERTY_ENABLED, enabled);
    }

    /**
     * @author bc93774
     * @param oControlModel the model of a control
     * @return the LabelType according to UIConsts.CONTROLTYPE
     */
    public static int getControlModelType(Object oControlModel)
    {
        XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oControlModel);
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
     * @author bc93774
     * @param oControlModel
     * @return the name of the property that contains the value of a controlmodel
     */
    public static String getDisplayProperty(Object oControlModel)
    {
        XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, oControlModel);
        int itype = getControlModelType(oControlModel);
        return getDisplayProperty(itype);
    }

    /**
     * @author bc93774
     * @param itype The type of the control conforming to UIConst.ControlType
     * @return the name of the property that contains the value of a controlmodel
     */
    public static String getDisplayProperty(int itype)
    {
        // String propertyname = PropertyNames.EMPTY_STRING;
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

    public void addResourceHandler(String _Unit, String _Module)
    {
        m_oResource = new Resource(xMSF, _Unit, _Module);
    }

    public static short setInitialTabindex(int _istep)
    {
        return (short) (_istep * 100);
    }
    private Boolean BisHighContrastModeActivated = null;

    public boolean isHighContrastModeActivated()
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
                    e.printStackTrace(System.out);
                    return false;
                }
                //TODO: The following methods could be wrapped in an own class implementation
                int nRed = getRedColorShare(nUIColor);
                int nGreen = getGreenColorShare(nUIColor);
                int nBlue = getBlueColorShare(nUIColor);
                int nLuminance = ((nBlue * 28 + nGreen * 151 + nRed * 77) / 256);
                boolean bisactivated = (nLuminance <= 25);
                BisHighContrastModeActivated = new Boolean(bisactivated);
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

    public static int getRedColorShare(int _nColor)
    {
        int nRed = (int) _nColor / 65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
        int nGreenModulo = (nRedModulo % 256);
        int nBlue = nGreenModulo;
        return nRed;
    }

    public static int getGreenColorShare(int _nColor)
    {
        int nRed = (int) _nColor / 65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
        return nGreen;
    }

    public static int getBlueColorShare(int _nColor)
    {
        int nRed = (int) _nColor / 65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
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
