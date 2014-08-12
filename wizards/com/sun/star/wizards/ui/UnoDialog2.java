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
import com.sun.star.wizards.ui.event.*;

/**
 * This class contains convenience methods for inserting components to a dialog.
 * It was created for use with the automatic conversion of Basic XML Dialog
 * description files to a Java class which builds the same dialog through the UNO API.<br/>
 * It uses an Event-Listener method, which calls a method through reflection
 * wenn an event on a component is trigered.
 * see the classes AbstractListener, CommonListener, MethodInvocation for details.
 */
public class UnoDialog2 extends UnoDialog
{

    /**
     * Override this method to return another listener.
     * @return
     */
    private AbstractListener createListener()
    {
        return new CommonListener();
    }

    public UnoDialog2(XMultiServiceFactory xmsf)
    {
        super(xmsf, new String[]
                {
                }, new Object[]
                {
                });
        guiEventListener = createListener();
    }

    public XButton insertButton(String sName, String actionPerformed, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {

        XButton xButton = (XButton) insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues, XButton.class);

        if (actionPerformed != null)
        {
            xButton.addActionListener((XActionListener) guiEventListener);
            guiEventListener.add(sName, EVENT_ACTION_PERFORMED, actionPerformed, eventTarget);
        }
        return xButton;
    }

    public XButton insertButton(String sName, String actionPerformed, String[] sPropNames, Object[] oPropValues)
    {
        return insertButton(sName, actionPerformed, this, sPropNames, oPropValues);
    }

    public XButton insertImageButton(String sName, com.sun.star.awt.XActionListener actionPerformed, String[] sPropNames, Object[] oPropValues)
    {

        XButton xButton = (XButton) insertControlModel2("com.sun.star.awt.UnoControlButtonModel", sName, sPropNames, oPropValues, XButton.class);

        if (actionPerformed != null)
        {
            xButton.addActionListener(actionPerformed);
        }
        return xButton;
    }

    public XCheckBox insertCheckBox(String sName, String itemChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {

        XCheckBox xCheckBox = (XCheckBox) insertControlModel2("com.sun.star.awt.UnoControlCheckBoxModel", sName, sPropNames, oPropValues, XCheckBox.class);

        if (itemChanged != null)
        {
            xCheckBox.addItemListener((XItemListener) guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        return xCheckBox;
    }

    public XCheckBox insertCheckBox(String sName, String itemChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertCheckBox(sName, itemChanged, this, sPropNames, oPropValues);
    }

    public XListBox insertListBox(String sName, String actionPerformed, String itemChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        XListBox xListBox = (XListBox) insertControlModel2("com.sun.star.awt.UnoControlListBoxModel", sName, sPropNames, oPropValues, XListBox.class);
        if (actionPerformed != null)
        {
            xListBox.addActionListener((XActionListener) guiEventListener);
            guiEventListener.add(sName, EVENT_ACTION_PERFORMED, actionPerformed, eventTarget);
        }
        if (itemChanged != null)
        {
            xListBox.addItemListener((XItemListener) guiEventListener);
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
            xRadioButton.addItemListener((XItemListener) guiEventListener);
            guiEventListener.add(sName, EVENT_ITEM_CHANGED, itemChanged, eventTarget);
        }
        return xRadioButton;
    }

    public XRadioButton insertRadioButton(String sName, String itemChanged, String[] sPropNames, Object[] oPropValues)
    {
        return insertRadioButton(sName, itemChanged, this, sPropNames, oPropValues);
    }



    public XTextComponent insertTextField(String sName, String sTextChanged, Object eventTarget, String[] sPropNames, Object[] oPropValues)
    {
        return (XTextComponent) insertEditField(sName, sTextChanged, eventTarget, "com.sun.star.awt.UnoControlEditModel", sPropNames, oPropValues, XTextComponent.class);
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
            xField.addTextListener((XTextListener) guiEventListener);
            guiEventListener.add(sName, EVENT_TEXT_CHANGED, sTextChanged, eventTarget);
        }
        return UnoRuntime.queryInterface(type, xField);
    }

    public XControl insertFixedLine(String sName, String[] sPropNames, Object[] oPropValues)
    {
        Object oLine = insertControlModel2("com.sun.star.awt.UnoControlFixedLineModel", sName, sPropNames, oPropValues);
        return UnoRuntime.queryInterface(XControl.class, oLine);
    }







    private Object insertControlModel2(String serviceName, String componentName, String[] sPropNames, Object[] oPropValues)
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

    private Object insertControlModel2(String serviceName, String componentName, String[] sPropNames, Object[] oPropValues, Class<? extends XInterface> type)
    {
        return UnoRuntime.queryInterface(type, insertControlModel2(serviceName, componentName, sPropNames, oPropValues));
    }



    public int showMessageBox(String windowServiceName, int windowAttribute, String MessageText)
    {
        return SystemDialog.showMessageBox(xMSF, this.xControl.getPeer(), windowServiceName, windowAttribute, MessageText);
    }
}
