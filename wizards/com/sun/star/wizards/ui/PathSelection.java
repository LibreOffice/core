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

import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;

public class PathSelection
{

    UnoDialog2 CurUnoDialog;
    XMultiServiceFactory xMSF;
    int iDialogType;
    int iTransferMode;
    public String sDefaultDirectory = PropertyNames.EMPTY_STRING;
    public String sDefaultName = PropertyNames.EMPTY_STRING;
    public String sDefaultFilter = PropertyNames.EMPTY_STRING;
    public boolean usedPathPicker = false;
    public XPathSelectionListener xAction;
    public XTextComponent xSaveTextBox;
    private final int CMDSELECTPATH = 1;
    private final int TXTSAVEPATH = 1;

    public static class DialogTypes
    {

        public static final int FOLDER = 0;
        public static final int FILE = 1;
    }

    public static class TransferMode
    {

        public static final int SAVE = 0;
        public static final int LOAD = 1;
    }

    public PathSelection(XMultiServiceFactory xMSF, UnoDialog2 CurUnoDialog, int TransferMode, int DialogType)
    {
        this.CurUnoDialog = CurUnoDialog;
        this.xMSF = xMSF;
        this.iDialogType = DialogType;
        this.iTransferMode = TransferMode;

    }

    public void insert(int DialogStep, int XPos, int YPos, int Width, short CurTabIndex, String LabelText, boolean Enabled, String TxtHelpURL, String BtnHelpURL)
    {

        CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSaveAs", new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                        Boolean.valueOf(Enabled), 8, LabelText, new Integer(XPos), new Integer(YPos), new Integer(DialogStep), new Short(CurTabIndex), new Integer(Width)
                });

        xSaveTextBox = CurUnoDialog.insertTextField("txtSavePath", "callXPathSelectionListener", this, new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                        Boolean.valueOf(Enabled), 12, TxtHelpURL, new Integer(XPos), new Integer(YPos + 10), new Integer(DialogStep), new Short((short) (CurTabIndex + 1)), new Integer(Width - 26)
                });
        //CurUnoDialog.setControlProperty("txtSavePath", PropertyNames.READ_ONLY, Boolean.TRUE);
        CurUnoDialog.setControlProperty("txtSavePath", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        CurUnoDialog.insertButton("cmdSelectPath", "triggerPathPicker", this, new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                        Boolean.valueOf(Enabled), 14, BtnHelpURL, "...", new Integer(XPos + Width - 16), new Integer(YPos + 9), new Integer(DialogStep), new Short((short) (CurTabIndex + 2)), 16
                });

    }

    public void addSelectionListener(XPathSelectionListener xAction)
    {
        this.xAction = xAction;
    }

    public String getSelectedPath()
    {
        return xSaveTextBox.getText();
    }

    public void initializePath()
    {
        try
        {
            FileAccess myFA = new FileAccess(xMSF);
            xSaveTextBox.setText(myFA.getPath(sDefaultDirectory + "/" + sDefaultName, null));
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void triggerPathPicker()
    {
        try
        {
            switch (iTransferMode)
            {
                case TransferMode.SAVE:
                    switch (iDialogType)
                    {
                        case DialogTypes.FOLDER:
                            //TODO: write code for picking a folder for saving
                            break;
                        case DialogTypes.FILE:
                            usedPathPicker = true;
                            SystemDialog myFilePickerDialog = SystemDialog.createStoreDialog(xMSF);
                            myFilePickerDialog.callStoreDialog(sDefaultDirectory, sDefaultName, sDefaultFilter);
                            String sStorePath = myFilePickerDialog.sStorePath;
                            if (sStorePath != null)
                            {
                                FileAccess myFA = new FileAccess(xMSF);
                                xSaveTextBox.setText(myFA.getPath(sStorePath, null));
                                sDefaultDirectory = FileAccess.getParentDir(sStorePath);
                                sDefaultName = FileAccess.getFilename(sStorePath);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case TransferMode.LOAD:
                    switch (iDialogType)
                    {
                        case DialogTypes.FOLDER:
                            //TODO: write code for picking a folder for loading
                            break;
                        case DialogTypes.FILE:
                            //TODO: write code for picking a file for loading
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public void callXPathSelectionListener()
    {
        if (xAction != null)
        {
            xAction.validatePath();
        }
    }
}
