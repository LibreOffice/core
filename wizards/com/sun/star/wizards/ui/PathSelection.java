/*
 ************************************************************************
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
    public String sDefaultDirectory = "";
    public String sDefaultName = "";
    public String sDefaultFilter = "";
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
                    new Boolean(Enabled), new Integer(8), LabelText, new Integer(XPos), new Integer(YPos), new Integer(DialogStep), new Short(CurTabIndex), new Integer(Width)
                });

        xSaveTextBox = CurUnoDialog.insertTextField("txtSavePath", "callXPathSelectionListener", this, new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                    new Boolean(Enabled), new Integer(12), TxtHelpURL, new Integer(XPos), new Integer(YPos + 10), new Integer(DialogStep), new Short((short) (CurTabIndex + 1)), new Integer(Width - 26)
                });
        //CurUnoDialog.setControlProperty("txtSavePath", "ReadOnly", Boolean.TRUE);
        CurUnoDialog.setControlProperty("txtSavePath", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        CurUnoDialog.insertButton("cmdSelectPath", "triggerPathPicker", this, new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, new Object[]
                {
                    new Boolean(Enabled), new Integer(14), BtnHelpURL, "...", new Integer(XPos + Width - 16), new Integer(YPos + 9), new Integer(DialogStep), new Short((short) (CurTabIndex + 2)), new Integer(16)
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
                                sDefaultName = myFA.getFilename(sStorePath);
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
