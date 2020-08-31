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
package com.sun.star.wizards.table;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;
import com.sun.star.wizards.ui.event.XActionListenerAdapter;
import com.sun.star.wizards.ui.event.XItemListenerAdapter;

public class PrimaryKeyHandler implements XFieldSelectionListener
{

    private final TableWizard CurUnoDialog;
    private final XRadioButton optAddAutomatically;
    private final XRadioButton optUseExisting;
    private final XRadioButton optUseSeveral;
    private final XCheckBox chkcreatePrimaryKey;
    private final XCheckBox chkApplyAutoValueExisting;
    private final XCheckBox chkApplyAutoValueAutomatic;
    private final XListBox lstSinglePrimeKey;
    private final XFixedText lblPrimeFieldName;
    private final FieldSelection curPrimaryKeySelection;
    private String[] fieldnames;
    private final TableDescriptor curTableDescriptor;
    private final boolean bAutoPrimaryKeysupportsAutoIncrementation;
    private static final String SAUTOMATICKEYFIELDNAME = "ID";

    public PrimaryKeyHandler(TableWizard _CurUnoDialog, TableDescriptor _curTableDescriptor)
    {
        this.CurUnoDialog = _CurUnoDialog;
        curTableDescriptor = _curTableDescriptor;
        bAutoPrimaryKeysupportsAutoIncrementation = curTableDescriptor.oTypeInspector.isAutoIncrementationSupported();
        short curtabindex = (short) ((TableWizard.SOPRIMARYKEYPAGE * 100) - 20);
        Integer IPRIMEKEYSTEP = Integer.valueOf(TableWizard.SOPRIMARYKEYPAGE);
        final String sExplanations = CurUnoDialog.m_oResource.getResText("RID_TABLE_26");
        final String screatePrimaryKey = CurUnoDialog.m_oResource.getResText("RID_TABLE_27");
        final String slblPrimeFieldName = CurUnoDialog.m_oResource.getResText("RID_TABLE_31");
        final String sApplyAutoValue = CurUnoDialog.m_oResource.getResText("RID_TABLE_33");
        final String sAddAutomatically = CurUnoDialog.m_oResource.getResText("RID_TABLE_28");
        final String sUseExisting = CurUnoDialog.m_oResource.getResText("RID_TABLE_29");
        final String sUseSeveral = CurUnoDialog.m_oResource.getResText("RID_TABLE_30");
        final String slblAvailableFields = CurUnoDialog.m_oResource.getResText("RID_QUERY_4");
        final String slblSelPrimaryFields = CurUnoDialog.m_oResource.getResText("RID_TABLE_32");
        CurUnoDialog.insertLabel("lblExplanation",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    40, sExplanations, Boolean.TRUE, 91, 27, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 233
                });

        chkcreatePrimaryKey = CurUnoDialog.insertCheckBox("chkcreatePrimaryKey", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CHK_USEPRIMEKEY", screatePrimaryKey, 97, 70, Short.valueOf((short) 1), IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 160
                });

        optAddAutomatically = CurUnoDialog.insertRadioButton("optAddAutomatically", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_AUTOMATIC", sAddAutomatically, 106, 82, Short.valueOf((short) 1), IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 200
                });

        optUseExisting = CurUnoDialog.insertRadioButton("optUseExisting", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //94
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_SINGLE", sUseExisting, 106, 104, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 200
                });

        optUseSeveral = CurUnoDialog.insertRadioButton("optUseSeveral", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_SEVERAL", sUseSeveral, 106, 132, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 200
                });

        chkApplyAutoValueAutomatic = CurUnoDialog.insertCheckBox("chkApplyAutoValueAutomatic", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //107
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CK_PK_AUTOVALUE_AUTOMATIC", sApplyAutoValue, 116, 92, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 68
                });

        lblPrimeFieldName = CurUnoDialog.insertLabel("lblPrimeFieldName",
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, UIConsts.INTEGERS[8], slblPrimeFieldName, 116, 117, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 46
                });

        lstSinglePrimeKey = CurUnoDialog.insertListBox("lstSinglePrimeKey", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        onPrimeKeySelected();
                    }
                }, null,
                new String[]
                {
                    "Dropdown",
                    PropertyNames.PROPERTY_ENABLED,
                    PropertyNames.PROPERTY_HEIGHT,
                    PropertyNames.PROPERTY_HELPURL,
                    "LineCount",
                    PropertyNames.PROPERTY_POSITION_X,
                    PropertyNames.PROPERTY_POSITION_Y,
                    PropertyNames.PROPERTY_STEP,
                    PropertyNames.PROPERTY_TABINDEX,
                    PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE,
                    Boolean.FALSE,
                    12,
                    "HID:WIZARDS_HID_DLGTABLE_LB_PK_FIELDNAME",
                    Short.valueOf(UnoDialog.getListBoxLineCount()),
                    162,
                    115,
                    IPRIMEKEYSTEP,
                    Short.valueOf(curtabindex++),
                    80
                });

        chkApplyAutoValueExisting = CurUnoDialog.insertCheckBox("chkApplyAutoValueExisting", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        togglePrimeKeyFields();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //107
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CK_PK_AUTOVALUE", sApplyAutoValue, 248, 117, IPRIMEKEYSTEP, Short.valueOf(curtabindex++), 66
                });
        curPrimaryKeySelection = new FieldSelection(CurUnoDialog, IPRIMEKEYSTEP.intValue(), 116, 142, 208, 47, slblAvailableFields, slblSelPrimaryFields, 41234, false);
        curPrimaryKeySelection.addFieldSelectionListener(this);

    }

    public void initialize()
    {
        // boolean breselect;
        fieldnames = curTableDescriptor.getNonBinaryFieldNames();
        String[] skeyfieldnames = curPrimaryKeySelection.getSelectedFieldNames();
        curPrimaryKeySelection.initialize(fieldnames, false);
        if (skeyfieldnames != null && skeyfieldnames.length > 0)
        {
            String[] snewkeyfieldnames = JavaTools.removeOutdatedFields(skeyfieldnames, fieldnames);
            curPrimaryKeySelection.setSelectedFieldNames(snewkeyfieldnames);
        }
        String selfield = lstSinglePrimeKey.getSelectedItem();
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), PropertyNames.STRING_ITEM_LIST, fieldnames);
        if (selfield != null)
        {
            if (JavaTools.FieldInList(fieldnames, selfield) > -1)
            {
                lstSinglePrimeKey.selectItem(selfield, true);
            }
        }
        togglePrimeKeyFields();
    }

    public boolean iscompleted()
    {
        if (chkcreatePrimaryKey.getState() == 0)
        {
            return true;
        }
        if (this.optAddAutomatically.getState())
        {
            return true;
        }
        if (optUseExisting.getState())
        {
            fieldnames = curTableDescriptor.getNonBinaryFieldNames();
            String selfield = lstSinglePrimeKey.getSelectedItem();
            if (selfield != null)
            {
                return (JavaTools.FieldInList(fieldnames, selfield) > -1);
            }
        }
        if (optUseSeveral.getState())
        {
            fieldnames = curTableDescriptor.getNonBinaryFieldNames();
            String[] skeyfieldnames = curPrimaryKeySelection.getSelectedFieldNames();
            String[] snewkeyfieldnames = JavaTools.removeOutdatedFields(skeyfieldnames, fieldnames);
            return (snewkeyfieldnames.length > 0);
        }
        return false;
    }

    private void togglePrimeKeyFields()
    {
        boolean bdoEnable = (this.chkcreatePrimaryKey.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bAutoPrimaryKeysupportsAutoIncrementation && bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseExisting), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseSeveral), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoEnable));
        //toggle subcontrols of the radiobuttons...
        toggleAutomaticAutoValueCheckBox();
        boolean benableSinglePrimekeyControls = bdoEnable && optUseExisting.getState();
        toggleSinglePrimeKeyFields(benableSinglePrimekeyControls);
        boolean benableSeveralPrimekeyControls = bdoEnable && optUseSeveral.getState();
        curPrimaryKeySelection.toggleListboxControls(Boolean.valueOf(benableSeveralPrimekeyControls));
        // toggle the following steps of the dialog...
        if (!bdoEnable)
        {
            CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, true);
        }
        else
        {
            if (benableSeveralPrimekeyControls)
            {
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
            }
            else if (benableSinglePrimekeyControls)
            {
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, UnoDialog.isListBoxSelected(lstSinglePrimeKey)); //.getSelectedItemPos() != -1);
            }
            else if (optAddAutomatically.getState())
            {
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, true);
            }
        }
    }

    private boolean isAutoIncrementatable(String _fieldname)
    {
        try
        {
            XPropertySet xColPropertySet = curTableDescriptor.getByName(_fieldname);
            if (xColPropertySet != null && curTableDescriptor.getDBDataTypeInspector() != null)
            {
                return curTableDescriptor.getDBDataTypeInspector().isAutoIncrementable(xColPropertySet);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return false;
    }

    public boolean isAutomaticMode()
    {
        boolean bisAutomaticMode = false;
        if (chkcreatePrimaryKey.getState() == 1)
        {
            bisAutomaticMode = ((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
        }
        return bisAutomaticMode;
    }

    public String getAutomaticFieldName()
    {
        return SAUTOMATICKEYFIELDNAME;
    }

    public boolean isAutoIncremented()
    {
        boolean bischecked = false;
        if (chkcreatePrimaryKey.getState() == 1)
        {
            boolean bisAutomaticMode = ((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
            boolean bisExistingMode = ((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(optUseExisting), PropertyNames.PROPERTY_STATE)).shortValue() == (short) 1;
            if (bisAutomaticMode)
            {
                bischecked = chkApplyAutoValueAutomatic.getState() == (short) 1;
            }
            else if (bisExistingMode)
            {
                bischecked = chkApplyAutoValueExisting.getState() == (short) 1;
            }
        }
        return bischecked;
    }

    private void onPrimeKeySelected()
    {
        try
        {
            String selfieldname = lstSinglePrimeKey.getSelectedItem();
            boolean bdoenable = isAutoIncrementatable(selfieldname);
            CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenable));
            XPropertySet xColPropertySet = curTableDescriptor.getByName(selfieldname);
            boolean bIsAutoIncremented = ((Boolean) xColPropertySet.getPropertyValue("IsAutoIncrement")).booleanValue();
            if (bIsAutoIncremented)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_STATE, Short.valueOf((short) 1));
            }
            else
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_STATE, Short.valueOf((short) 0));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    private void toggleAutomaticAutoValueCheckBox()
    {
        try
        {
            boolean bisAutomaticMode = AnyConverter.toBoolean(Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_ENABLED));
            boolean bdoenable = bAutoPrimaryKeysupportsAutoIncrementation && optAddAutomatically.getState() && bisAutomaticMode;
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenable));
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.err);
        }
    }

    private void toggleSinglePrimeKeyFields(boolean _bdoenable)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblPrimeFieldName), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
        boolean bdoenableAutoValueCheckBox = (isAutoIncrementatable(lstSinglePrimeKey.getSelectedItem()) && _bdoenable);
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenableAutoValueCheckBox));
    }

    public String[] getPrimaryKeyFields()
    {
        if (chkcreatePrimaryKey.getState() == 0)
        {
            return null;
        }
        if (fieldnames == null)
        {
            initialize();
        }
        if (optUseSeveral.getState())
        {
            return curPrimaryKeySelection.getSelectedFieldNames();
        }
        else if (optUseExisting.getState())
        {
            return new String[]
                    {
                        lstSinglePrimeKey.getSelectedItem()
                    };
        }
        else if (optAddAutomatically.getState())
        {
            return new String[]
                    {
                        SAUTOMATICKEYFIELDNAME
                    };
        }
        return null;
    }

    public void moveItemDown(String Selitem)
    {
    }

    public void moveItemUp(String Selitem)
    {
    }

    public void setID(String sIncSuffix)
    {
    }

    public void shiftFromLeftToRight(String[] SelItems, String[] NewItems)
    {
        CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }

    public void shiftFromRightToLeft(String[] OldSelItems, String[] NewItems)
    {
        CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }
}
