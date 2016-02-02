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

package com.sun.star.script.framework.security;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;

import com.sun.star.beans.XPropertySet;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.container.XNameContainer;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.lib.uno.helper.WeakBase;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.script.framework.log.LogUtils;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class SecurityDialog extends WeakBase implements
    XComponent, XServiceInfo, XDialog, XInitialization {

    static final String __serviceName =
        "com.sun.star.script.framework.security.SecurityDialog";

    private static final String _label1Name = "Label1";

    private static final String _label1String =
        "This document contains macros. Do you want to allow these macros to be run?";

    private static final String _label2Name = "Label2";

    private static final String _label2String =
        "This document contains macros. According to the security settings, the";

    private static final String _label3Name = "Label3";

    private static final String _label3String =
        "macros in this document should not be run. Do you want to run them";

    private static final String _label4Name = "Label4";
    private static final String _label4String = "anyway?";

    private static final String _checkBoxName = "CheckBox";

    private static final String _checkBoxString =
        "Add this directory to the list of secure paths: ";

    private static final String _label5Name = "Label5";

    private static final String _title = "Run Macro";
    private static final String _runMacro = "Run";
    private static final String _runButtonName = "Run";
    private static final String _doNotRunMacro = "Do Not Run";
    private static final String _doNotRunButtonName = "DoNotRun";

    private static final int dialogX = 100;
    private static final int dialogY = 100;
    private static final int dialogW = 235;
    private static final int dialogH = 47;

    private int cbIncrW = -20;
    private int cbIncrH = 19;

    private static final int runButtonW = 40;
    private static final int runButtonH = 13;
    private static final int doNotRunButtonW = 40;
    private static final int doNotRunButtonH = 13;

    // label for warning dialog
    private static final int label1X = 20;
    private static final int label1Y = 9;
    private static final int label1W = 210;
    private static final int label1H = 10;

    // labels for confirmation dialog
    private static final int label2X = 22;
    private static final int label2Y = 7;
    private static final int label2W = 210;
    private static final int label2H = 8;
    private static final int label3X = 22;
    private static final int label3Y = 15;
    private static final int label3W = 210;
    private static final int label3H = 8;
    private static final int label4X = 22;
    private static final int label4Y = 23;
    private static final int label4W = 210;
    private static final int label4H = 8;

    // checkbox for confirmation dialog
    private static final int checkBoxX = 22;
    private static final int checkBoxY = 40;
    private static final int checkBoxW = 210;
    private static final int checkBoxH = 9;
    // extra label if path longer than 21 chars
    private static final int label5X = 22;
    private static final int label5Y = 48;
    private static final int label5W = 210;
    private static final int label5H = 9;

    private boolean checkBoxDialog;
    private short _checkBoxState = 0;
    private boolean extraPathLine = false;
    private String checkBoxPath = "";
    private String checkBoxPath2 = "";
    private static final int lineWrapLength = 21;
    private static final int lineWrapH = 12;
    private String _pushed = _doNotRunButtonName;

    private final XComponentContext _xComponentContext;
    private XDialog _xDialog;

    public SecurityDialog(XComponentContext xComponentContext) {
        LogUtils.DEBUG("SecurityDialog ctor");
        _xComponentContext = xComponentContext;
    }

    public void initialize(Object[] args) throws RuntimeException {
        LogUtils.DEBUG("SecurityDialog init");

        // figure out if we need a checkbox
        if (args.length == 1 && AnyConverter.isString(args[0])) {
            //check args is a path
            // set checkBoxPath with the arg
            LogUtils.DEBUG("checkbox");

            try {
                checkBoxPath = AnyConverter.toString(args[0]);
            } catch (IllegalArgumentException e) {
                throw new RuntimeException(e);
            }

            LogUtils.DEBUG("path: " + checkBoxPath);
            checkBoxDialog = true;

            if (checkBoxPath.length() > lineWrapLength) {
                extraPathLine = true;
                cbIncrH += lineWrapH;
                checkBoxPath2 = checkBoxPath.substring(lineWrapLength);
                checkBoxPath = checkBoxPath.substring(0, lineWrapLength);
            }

        } else {
            LogUtils.DEBUG("no checkbox: # of args=" + args.length);
            cbIncrW = 0;
            cbIncrH = 0;
            checkBoxDialog = false;
        }

        // now try and create the dialog
        try {
            _xDialog = createDialog();
        } catch (com.sun.star.uno.Exception e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

    }

    // static component operations
    public static XSingleServiceFactory __getServiceFactory(String implName,
            XMultiServiceFactory multiFactory, XRegistryKey regKey) {

        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(SecurityDialog.class.getName())) {

            xSingleServiceFactory =
                FactoryHelper.getServiceFactory(SecurityDialog.class,
                                                SecurityDialog.__serviceName,
                                                multiFactory, regKey);

        }

        return xSingleServiceFactory;
    }

    // XServiceInfo
    public String getImplementationName() {
        return getClass().getName();
    }

    // XServiceInfo
    public boolean supportsService(/*IN*/String serviceName) {
        return serviceName.equals(__serviceName);
    }

    // XServiceInfo
    public String[] getSupportedServiceNames() {
        String[] retValue = new String[0];
        retValue[0] = __serviceName;
        return retValue;
    }

    /** method for creating a dialog at runtime
     */
    private XDialog createDialog() throws com.sun.star.uno.Exception {

        // get the service manager from the component context
        XMultiComponentFactory xMultiComponentFactory =
            _xComponentContext.getServiceManager();

        // create the dialog model and set the properties
        Object dialogModel =
            xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.awt.UnoControlDialogModel", _xComponentContext);

        XPropertySet xPSetDialog =
            UnoRuntime.queryInterface(XPropertySet.class, dialogModel);

        xPSetDialog.setPropertyValue("PositionX", Integer.valueOf(dialogX));
        xPSetDialog.setPropertyValue("PositionY", Integer.valueOf(dialogY));
        xPSetDialog.setPropertyValue("Width", Integer.valueOf(dialogW + cbIncrW));
        xPSetDialog.setPropertyValue("Height", Integer.valueOf(dialogH + cbIncrH));
        xPSetDialog.setPropertyValue("Title", _title);

        // get the service manager from the dialog model
        XMultiServiceFactory xMultiServiceFactory =
            UnoRuntime.queryInterface(XMultiServiceFactory.class, dialogModel);

        // create the Run Macro button model and set the properties
        Object runButtonModel =
            xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlButtonModel");

        XPropertySet xPSetButton =
            UnoRuntime.queryInterface(XPropertySet.class, runButtonModel);

        LogUtils.DEBUG("run: x=" + (((dialogW + cbIncrW) / 2) - runButtonW - 1));
        LogUtils.DEBUG("run: y=" + (dialogH + cbIncrH - runButtonH - 1));

        xPSetButton.setPropertyValue(
            "PositionX", Integer.valueOf((((dialogW + cbIncrW) / 2) - runButtonW - 1)));

        xPSetButton.setPropertyValue(
            "PositionY", Integer.valueOf(dialogH + cbIncrH - runButtonH - 1));

        xPSetButton.setPropertyValue("Width", Integer.valueOf(runButtonW));
        xPSetButton.setPropertyValue("Height", Integer.valueOf(runButtonH));
        xPSetButton.setPropertyValue("Name", _runButtonName);
        xPSetButton.setPropertyValue("TabIndex", Short.valueOf((short)1));
        xPSetButton.setPropertyValue("Label", _runMacro);

        // create the Don't Run Macro button model and set the properties
        Object doNotRunButtonModel =
            xMultiServiceFactory.createInstance(
                "com.sun.star.awt.UnoControlButtonModel");

        xPSetButton =
            UnoRuntime.queryInterface(XPropertySet.class, doNotRunButtonModel);

        LogUtils.DEBUG("dontrun: x=" + (((dialogW + cbIncrW) / 2) - 1));
        LogUtils.DEBUG("dontrun: y=" + (dialogH + cbIncrH - doNotRunButtonH - 1));

        xPSetButton.setPropertyValue(
            "PositionX", Integer.valueOf((((dialogW + cbIncrW) / 2) + 1)));

        xPSetButton.setPropertyValue(
            "PositionY", Integer.valueOf((dialogH + cbIncrH - doNotRunButtonH - 1)));

        xPSetButton.setPropertyValue("Width", Integer.valueOf(doNotRunButtonW));
        xPSetButton.setPropertyValue("Height", Integer.valueOf(doNotRunButtonH));
        xPSetButton.setPropertyValue("Name", _doNotRunButtonName);
        xPSetButton.setPropertyValue("TabIndex", Short.valueOf((short)0));
        xPSetButton.setPropertyValue("Label", _doNotRunMacro);

        // insert the control models into the dialog model
        XNameContainer xNameCont =
            UnoRuntime.queryInterface(XNameContainer.class, dialogModel);

        xNameCont.insertByName(_runButtonName, runButtonModel);
        xNameCont.insertByName(_doNotRunButtonName, doNotRunButtonModel);

        if (checkBoxDialog) {
            LogUtils.DEBUG("creating label & checkbox");

            // create the label model and set the properties
            Object label2Model =
                xMultiServiceFactory.createInstance(
                    "com.sun.star.awt.UnoControlFixedTextModel");

            XPropertySet xPSetLabel =
                UnoRuntime.queryInterface(XPropertySet.class, label2Model);

            xPSetLabel.setPropertyValue("PositionX", Integer.valueOf(label2X));
            xPSetLabel.setPropertyValue("PositionY", Integer.valueOf(label2Y));
            xPSetLabel.setPropertyValue("Width", Integer.valueOf(label2W));
            xPSetLabel.setPropertyValue("Height", Integer.valueOf(label2H));
            xPSetLabel.setPropertyValue("Name", _label2Name);
            xPSetLabel.setPropertyValue("TabIndex", Short.valueOf((short)1));
            xPSetLabel.setPropertyValue("Label", _label2String);

            // create the label model and set the properties
            Object label3Model =
                xMultiServiceFactory.createInstance(
                    "com.sun.star.awt.UnoControlFixedTextModel");

            XPropertySet xPSetLabel3 =
                UnoRuntime.queryInterface(XPropertySet.class, label3Model);

            xPSetLabel3.setPropertyValue("PositionX", Integer.valueOf(label3X));
            xPSetLabel3.setPropertyValue("PositionY", Integer.valueOf(label3Y));
            xPSetLabel3.setPropertyValue("Width", Integer.valueOf(label3W));
            xPSetLabel3.setPropertyValue("Height", Integer.valueOf(label3H));
            xPSetLabel3.setPropertyValue("Name", _label3Name);
            xPSetLabel3.setPropertyValue("TabIndex", Short.valueOf((short)1));
            xPSetLabel3.setPropertyValue("Label", _label3String);

            // create the label model and set the properties
            Object label4Model =
                xMultiServiceFactory.createInstance(
                    "com.sun.star.awt.UnoControlFixedTextModel");

            XPropertySet xPSetLabel4 =
                UnoRuntime.queryInterface(XPropertySet.class, label4Model);

            xPSetLabel4.setPropertyValue("PositionX", Integer.valueOf(label4X));
            xPSetLabel4.setPropertyValue("PositionY", Integer.valueOf(label4Y));
            xPSetLabel4.setPropertyValue("Width", Integer.valueOf(label4W));
            xPSetLabel4.setPropertyValue("Height", Integer.valueOf(label4H));
            xPSetLabel4.setPropertyValue("Name", _label4Name);
            xPSetLabel4.setPropertyValue("TabIndex", Short.valueOf((short)1));
            xPSetLabel4.setPropertyValue("Label", _label4String);

            // create the checkbox model and set the properties
            Object checkBoxModel =
                xMultiServiceFactory.createInstance(
                    "com.sun.star.awt.UnoControlCheckBoxModel");

            XPropertySet xPSetCheckBox =
                UnoRuntime.queryInterface(XPropertySet.class, checkBoxModel);

            xPSetCheckBox.setPropertyValue("PositionX", Integer.valueOf(checkBoxX));
            xPSetCheckBox.setPropertyValue("PositionY", Integer.valueOf(checkBoxY));
            xPSetCheckBox.setPropertyValue("Width", Integer.valueOf(checkBoxW));
            xPSetCheckBox.setPropertyValue("Height", Integer.valueOf(checkBoxH));
            xPSetCheckBox.setPropertyValue("State", Short.valueOf((short)0));
            xPSetCheckBox.setPropertyValue("Name", _checkBoxName);
            xPSetCheckBox.setPropertyValue("TabIndex", Short.valueOf((short)1));
            xPSetCheckBox.setPropertyValue("Label", _checkBoxString + checkBoxPath);

            // insert the control models into the dialog model
            xNameCont.insertByName(_label2Name, label2Model);
            xNameCont.insertByName(_label3Name, label3Model);
            xNameCont.insertByName(_label4Name, label4Model);
            xNameCont.insertByName(_checkBoxName, checkBoxModel);

            if (extraPathLine) {

                // create the label model and set the properties
                Object label5Model =
                    xMultiServiceFactory.createInstance(
                        "com.sun.star.awt.UnoControlFixedTextModel");

                XPropertySet xPSetLabel5 =
                    UnoRuntime.queryInterface(XPropertySet.class, label5Model);

                xPSetLabel5.setPropertyValue("PositionX", Integer.valueOf(label5X));
                xPSetLabel5.setPropertyValue("PositionY", Integer.valueOf(label5Y));
                xPSetLabel5.setPropertyValue("Width", Integer.valueOf(label5W));
                xPSetLabel5.setPropertyValue("Height", Integer.valueOf(label5H));
                xPSetLabel5.setPropertyValue("Name", _label5Name);
                xPSetLabel5.setPropertyValue("TabIndex", Short.valueOf((short)1));
                xPSetLabel5.setPropertyValue("Label", checkBoxPath2);
                xNameCont.insertByName(_label5Name, label5Model);
            }
        } else {
            // create the label model and set the properties
            Object labelModel =
                xMultiServiceFactory.createInstance(
                    "com.sun.star.awt.UnoControlFixedTextModel");

            XPropertySet xPSetLabel =
                UnoRuntime.queryInterface(XPropertySet.class, labelModel);

            xPSetLabel.setPropertyValue("PositionX", Integer.valueOf(label1X));
            xPSetLabel.setPropertyValue("PositionY", Integer.valueOf(label1Y));
            xPSetLabel.setPropertyValue("Width", Integer.valueOf(label1W));
            xPSetLabel.setPropertyValue("Height", Integer.valueOf(label1H));
            xPSetLabel.setPropertyValue("Name", _label1Name);
            xPSetLabel.setPropertyValue("TabIndex", Short.valueOf((short)1));
            xPSetLabel.setPropertyValue("Label", _label1String);

            // insert the control models into the dialog model
            xNameCont.insertByName(_label1Name, labelModel);
        }

        // create the dialog control and set the model
        Object dialog =
            xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.awt.UnoControlDialog", _xComponentContext);

        XControl xControl =
            UnoRuntime.queryInterface(XControl.class, dialog);

        XControlModel xControlModel =
            UnoRuntime.queryInterface(XControlModel.class, dialogModel);

        xControl.setModel(xControlModel);

        // add an action listener to the button control
        XControlContainer xControlCont =
            UnoRuntime.queryInterface(XControlContainer.class, dialog);

        // Add to yes button
        Object objectButton = xControlCont.getControl(_runButtonName);

        XButton xButton =
            UnoRuntime.queryInterface(XButton.class, objectButton);

        xButton.addActionListener(new ActionListenerImpl(_runButtonName));

        // add to no button
        objectButton = xControlCont.getControl(_doNotRunButtonName);

        xButton =
            UnoRuntime.queryInterface(XButton.class, objectButton);

        xButton.addActionListener(new ActionListenerImpl(_doNotRunButtonName));

        if (checkBoxDialog) {
            // add to checkbox
            Object objectCheckBox = xControlCont.getControl(_checkBoxName);

            XCheckBox xCheckBox =
                UnoRuntime.queryInterface(XCheckBox.class, objectCheckBox);

            xCheckBox.addItemListener(new ItemListenerImpl(xControlCont));
        }

        // create a peer
        Object toolkit =
            xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.awt.ExtToolkit", _xComponentContext);

        XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class, toolkit);
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xControl);
        xWindow.setVisible(false);
        xControl.createPeer(xToolkit, null);

        // return the dialog
        XDialog xDialog = UnoRuntime.queryInterface(XDialog.class, dialog);
        return xDialog;
    }

    public short execute() {

        short result = 0;
        _pushed = _doNotRunButtonName;
        LogUtils.DEBUG("*DF* Before execute ");
        _xDialog.execute();
        LogUtils.DEBUG("*DF* After execute ");

        if (_pushed.equals(_runButtonName)) {
            result += 1;
        }

        if (_checkBoxState == 1) {
            result += 2;
        }

        return result;
    }

    public void endExecute() {
        _xDialog.endExecute();
    }

    public String getTitle() {
        return _xDialog.getTitle();
    }

    public void setTitle(String Title) {
        _xDialog.setTitle(Title);
    }

    public void dispose() {

        XComponent xComponent =
            UnoRuntime.queryInterface(XComponent.class, _xDialog);

        xComponent.dispose();
    }

    public void addEventListener(com.sun.star.lang.XEventListener xListener) {

        XComponent xComponent =
            UnoRuntime.queryInterface(XComponent.class, _xDialog);

        xComponent.addEventListener(xListener);
    }

    public void removeEventListener(com.sun.star.lang.XEventListener aListener) {

        XComponent xComponent =
            UnoRuntime.queryInterface(XComponent.class, _xDialog);

        xComponent.removeEventListener(aListener);
    }

    private class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        private final String _buttonName;

        private ActionListenerImpl(String buttonName) {
            _buttonName = buttonName;
        }

        // XEventListener
        public void disposing(EventObject eventObject) {
        }

        // XActionListener
        public void actionPerformed(ActionEvent actionEvent) {
            _pushed = _buttonName;
            LogUtils.DEBUG("** Button pushed ->" + _pushed);
            _xDialog.endExecute();
        }
    }

    private class ItemListenerImpl implements com.sun.star.awt.XItemListener {

        private XCheckBox _xCheckBox;

        private ItemListenerImpl(XControlContainer xControlCont) {
            Object objectCheckBox = xControlCont.getControl(_checkBoxName);

            _xCheckBox =
                UnoRuntime.queryInterface(XCheckBox.class, objectCheckBox);

        }

        // XEventListener
        public void disposing(EventObject eventObject) {
            _xCheckBox = null;
        }

        // XAdjustmentListener
        public void itemStateChanged(ItemEvent itemEvent) {
            _checkBoxState = _xCheckBox.getState();
            LogUtils.DEBUG("** checkbox state ->" + _checkBoxState);
        }
    }
}
