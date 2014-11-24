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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;

/**
 * This class contains convenience methods for inserting components to a dialog.
 * <p>It was created for use with the automatic conversion of Basic XML Dialog
 * description files to a Java class which builds the same dialog through the UNO API.</p>
 * <p>It uses an Event-Listener method, which calls a method through reflection
 * when an event on a component is triggered.
 * see the classes CommonListener, MethodInvocation for details.</p>
 */
public class UnoDialog2 extends UnoDialog
{

    public UnoDialog2(XMultiServiceFactory xmsf)
    {
        super(xmsf);
    }

    public XButton insertButton(String sName, String actionPerformed, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {

        XButton xButton = (XButton) insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues, XButton.class);

        if (actionPerformed != null)
        {
            xButton.addActionListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ACTION_PERFORMED, actionPerformed, eventTarget);
        }
        return xButton;
    }

    public XButton insertButton(String sName, String actionPerformed, String[] sPropNames, Object[] oPropValues)
    {
        return insertButton(sName, actionPerformed, this, sPropNames, oPropValues);
    }

    public XButton insertImageButton(String sName, com.sun.star.awt.XActionListener actionPerformed, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {

        XButton xButton = (XButton) insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues, XButton.class);

        if (actionPerformed != null)
        {
            xButton.addActionListener(actionPerformed);
        }
        return xButton;
    }

    public XButton insertImageButton(String sName, com.sun.star.awt.XActionListener actionPerformed, String[] sPropNames, Object[] oPropValues)
    {
        return insertImageButton(sName, actionPerformed, this, sPropNames, oPropValues);
    }

    public XCheckBox insertCheckBox(String sName, String itemChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {

        XCheckBox xCheckBox = (XCheckBox) insertControlModel2("com.sun.star.awt.UnoControlCheckBoxModel", sName, sPropNames, oPropValues, XCheckBox.class);

        if (itemChanged != null)
        {
            xCheckBox.addItemListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        return xCheckBox;
    }

    public XCheckBox insertCheckBox(String sName, String itemChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertCheckBox(sName, itemChanged, this, sPropNames, oPropValues);
    }

    public XComboBox insertComboBox(String sName, String actionPerformed, String itemChanged, String textChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        XComboBox xComboBox = (XComboBox) insertControlModel2("com.sun.star.awt.UnoControlComboBoxModel", sName, sPropNames, oPropValues, XComboBox.class);
        if (actionPerformed != null)
        {
            xComboBox.addActionListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ACTION_PERFORMED, actionPerformed, eventTarget);
        }
        if (itemChanged != null)
        {
            xComboBox.addItemListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        if (textChanged != null)
        {
            XTextComponent xTextComponent = UnoRuntime.queryInterface(XTextComponent.class, xComboBox);
            xTextComponent.addTextListener(guiEventListener);
            guiEventListener.add(sName, EVENT_TEXT_CHANGED, textChanged, eventTarget);
        }
        return xComboBox;
    }

    public XComboBox insertComboBox(String sName, String actionPerformed, String itemChanged, String textChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertComboBox(sName, actionPerformed, textChanged, itemChanged, this, sPropNames, oPropValues);
    }

    public XListBox insertListBox(String sName, String actionPerformed, String itemChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        XListBox xListBox = (XListBox) insertControlModel2("com.sun.star.awt.UnoControlListBoxModel", sName, sPropNames, oPropValues, XListBox.class);
        if (actionPerformed != null)
        {
            xListBox.addActionListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ACTION_PERFORMED, actionPerformed, eventTarget);
        }
        if (itemChanged != null)
        {
            xListBox.addItemListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        return xListBox;
    }

    public XListBox insertListBox(String sName, String actionPerformed, String itemChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertListBox(sName, actionPerformed, itemChanged, this, sPropNames, oPropValues);
    }

    public XRadioButton insertRadioButton(String sName, String itemChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        XRadioButton xRadioButton = (XRadioButton) insertControlModel2("com.sun.star.awt.UnoControlRadioButtonModel", sName, sPropNames, oPropValues, XRadioButton.class);
        if (itemChanged != null)
        {
            xRadioButton.addItemListener(guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        return xRadioButton;
    }

    public XRadioButton insertRadioButton(String sName, String itemChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertRadioButton(sName, itemChanged, this, sPropNames, oPropValues);
    }

    public XControl insertTitledBox(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oTitledBox = insertControlModel2("com.sun.star.awt.UnoControlGroupBoxModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XControl.class, oTitledBox);
    }

    public XTextComponent insertTextField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XTextComponent) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlEditModel", sPropNames, oPropValues, XTextComponent.class);
    }

    public XTextComponent insertTextField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertTextField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XControl insertImage(String sName, String[] sPropNames, Object[] oPropValues)
    {
        return (XControl) insertControlModel2("com.sun.star.awt.UnoControlImageControlModel", sName, sPropNames, oPropValues, XControl.class);
    }

    public XControl insertInfoImage(int _posx, int _posy, int _iStep)
    {
        XControl xImgControl = insertImage(Desktop.getUniqueName(getDlgNameAccess(), "imgHint"),
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_IMAGEURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Short.valueOf((short) 0), 10, UIConsts.INFOIMAGEURL, Integer.valueOf(_posx), Integer.valueOf(_posy), Boolean.FALSE, Integer.valueOf(_iStep), 10
                });
        super.getPeerConfiguration().setImageUrl(getModel(xImgControl), UIConsts.INFOIMAGEURL, UIConsts.INFOIMAGEURL_HC);
        return xImgControl;
    }

    /**
     * This method is used for creating Edit, Currency, Date, Formatted, Pattern, File
     * and Time edit components.
     */
    private Object insertEditField(String sName, String sTextChanged, Object eventTarget, String sModelClass, String[] sPropNames, Object[] oPropValues, Class<? extends XInterface> type)
    {
        XTextComponent xField = (XTextComponent) insertControlModel2(sModelClass, sName, sPropNames, oPropValues, XTextComponent.class);
        if (sTextChanged != null)
        {
            xField.addTextListener(guiEventListener);
            guiEventListener.add(sName, EVENT_TEXT_CHANGED, sTextChanged, eventTarget);
        }
        return UnoRuntime.queryInterface(type, xField);
    }

    public XControl insertFileControl(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XControl) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlFileControlModel", sPropNames, oPropValues, XControl.class);
    }

    public XControl insertFileControl(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertFileControl(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XCurrencyField insertCurrencyField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XCurrencyField) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlCurrencyFieldModel", sPropNames, oPropValues, XCurrencyField.class);
    }

    public XCurrencyField insertCurrencyField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertCurrencyField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XDateField insertDateField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XDateField) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlDateFieldModel", sPropNames, oPropValues, XDateField.class);
    }

    public XDateField insertDateField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertDateField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XNumericField insertNumericField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XNumericField) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlNumericFieldModel", sPropNames, oPropValues, XNumericField.class);
    }

    public XNumericField insertNumericField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertNumericField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XTimeField insertTimeField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XTimeField) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlTimeFieldModel", sPropNames, oPropValues, XTimeField.class);
    }

    public XTimeField insertTimeField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertTimeField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XPatternField insertPatternField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XPatternField) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlPatternFieldModel", sPropNames, oPropValues, XPatternField.class);
    }

    public XPatternField insertPatternField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertPatternField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XTextComponent insertFormattedField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XTextComponent) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlFormattedFieldModel", sPropNames, oPropValues, XTextComponent.class);
    }

    public XTextComponent insertFormattedField(String sName, String sTextChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertFormattedField(sName, sTextChanged, this, sPropNames, oPropValues);
    }

    public XControl insertFixedLine(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oLine = insertControlModel2("com.sun.star.awt.UnoControlFixedLineModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XControl.class, oLine);
    }

    public XScrollBar insertScrollBar(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oScrollBar = insertControlModel2("com.sun.star.awt.UnoControlScrollBarModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XScrollBar.class, oScrollBar);
    }

    public XProgressBar insertProgressBar(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oProgressBar = insertControlModel2("com.sun.star.awt.UnoControlProgressBarModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XProgressBar.class, oProgressBar);
    }

    public XControl insertGroupBox(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oGroupBox = insertControlModel2("com.sun.star.awt.UnoControlGroupBoxModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XControl.class, oGroupBox);
    }

    public Object insertControlModel2(String serviceName, String componentName, String[] sPropNames, Object[] oPropValues)
    {
        try
        {
            XInterface xControlModel = insertControlModel(serviceName, componentName, new String[]
                    {
                    }, new Object[]
                    {
                    });
            Helper.setUnoPropertyValues(xControlModel, sPropNames, oPropValues);
            Helper.setUnoPropertyValue(xControlModel, PropertyNames.PROPERTY_NAME, componentName);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        return xDlgContainer.getControl(componentName);
    }

    public Object insertControlModel2(String serviceName, String componentName, String[] sPropNames, Object[] oPropValues, Class<? extends XInterface> type)
    {
        return UnoRuntime.queryInterface(type, insertControlModel2(serviceName, componentName, sPropNames, oPropValues));
    }

    public String translateURL(String relativeURL)
    {
        return PropertyNames.EMPTY_STRING;
    }

    public static Object getControlModel(Object unoControl)
    {
        return UnoRuntime.queryInterface(XControl.class, unoControl).getModel();
    }

    public int showMessageBox(String windowServiceName, int windowAttribute, String MessageText)
    {
        return SystemDialog.showMessageBox(xMSF, this.xControl.getPeer(), windowServiceName, windowAttribute, MessageText);
    }
}
