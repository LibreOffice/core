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
package com.sun.star.wizards.table;

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
import com.sun.star.wizards.db.TypeInspector;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;

public class PrimaryKeyHandler implements XFieldSelectionListener
{

    private TableWizard CurUnoDialog;
    private short curtabindex;
    private final static String SPRIMEKEYMODE = "togglePrimeKeyFields";
    private final static String SSINGLEKEYMODE = "toggleSinglePrimeKeyFields";
    private final static String SSEVERALKEYMODE = "toggleSeveralPrimeKeyFields";
    private XRadioButton optAddAutomatically;
    private XRadioButton optUseExisting;
    private XRadioButton optUseSeveral;
    private XCheckBox chkUsePrimaryKey;
    private XCheckBox chkcreatePrimaryKey;
    private XCheckBox chkApplyAutoValueExisting;
    private XCheckBox chkApplyAutoValueAutomatic;
    private XListBox lstSinglePrimeKey;
    private XFixedText lblPrimeFieldName;
    private FieldSelection curPrimaryKeySelection;
    private String[] fieldnames;
    private TableDescriptor curTableDescriptor;
    private int nAutoPrimeKeyDataType;
    private boolean bAutoPrimaryKeysupportsAutoIncrmentation;
    private final static String SAUTOMATICKEYFIELDNAME = "ID";

    public PrimaryKeyHandler(TableWizard _CurUnoDialog, TableDescriptor _curTableDescriptor)
    {
        this.CurUnoDialog = _CurUnoDialog;
        curTableDescriptor = _curTableDescriptor;
        bAutoPrimaryKeysupportsAutoIncrmentation = isAutoPrimeKeyAutoIncrementationsupported();
        curtabindex = (short) ((TableWizard.SOPRIMARYKEYPAGE * 100) - 20);
        Integer IPRIMEKEYSTEP = new Integer(TableWizard.SOPRIMARYKEYPAGE);
        final String sExplanations = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 26);
        final String screatePrimaryKey = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 27);
        final String slblPrimeFieldName = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 31);
        final String sApplyAutoValue = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 33);
        final String sAddAutomatically = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 28);
        final String sUseExisting = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 29);
        final String sUseSeveral = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 30);
        final String slblAvailableFields = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 4);
        final String slblSelPrimaryFields = CurUnoDialog.m_oResource.getResText(UIConsts.RID_TABLE + 32);
        CurUnoDialog.insertLabel("lblExplanation",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    40, sExplanations, Boolean.TRUE, 91, 27, IPRIMEKEYSTEP, new Short(curtabindex++), 233
                });

        chkcreatePrimaryKey = CurUnoDialog.insertCheckBox("chkcreatePrimaryKey", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CHK_USEPRIMEKEY", screatePrimaryKey, 97, 70, new Short((short) 1), IPRIMEKEYSTEP, new Short(curtabindex++), 160
                });

        optAddAutomatically = CurUnoDialog.insertRadioButton("optAddAutomatically", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_AUTOMATIC", sAddAutomatically, 106, 82, new Short((short) 1), IPRIMEKEYSTEP, new Short(curtabindex++), 200
                });

        optUseExisting = CurUnoDialog.insertRadioButton("optUseExisting", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //94
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_SINGLE", sUseExisting, 106, 104, IPRIMEKEYSTEP, new Short(curtabindex++), 200
                });

        optUseSeveral = CurUnoDialog.insertRadioButton("optUseSeveral", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_OPT_PK_SEVERAL", sUseSeveral, 106, 132, IPRIMEKEYSTEP, new Short(curtabindex++), 200
                });

        chkApplyAutoValueAutomatic = CurUnoDialog.insertCheckBox("chkApplyAutoValueAutomatic", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //107
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CK_PK_AUTOVALUE_AUTOMATIC", sApplyAutoValue, 116, 92, IPRIMEKEYSTEP, new Short(curtabindex++), 68
                });

        lblPrimeFieldName = CurUnoDialog.insertLabel("lblPrimeFieldName",
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, UIConsts.INTEGERS[8], slblPrimeFieldName, 116, 117, IPRIMEKEYSTEP, new Short(curtabindex++), 46
                });

        lstSinglePrimeKey = CurUnoDialog.insertListBox("lstSinglePrimeKey", "onPrimeKeySelected", null, this,
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
                    new Short(curtabindex++),
                    80
                });

        chkApplyAutoValueExisting = CurUnoDialog.insertCheckBox("chkApplyAutoValueExisting", SPRIMEKEYMODE, this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                }, //107
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGTABLE_CK_PK_AUTOVALUE", sApplyAutoValue, 248, 117, IPRIMEKEYSTEP, new Short(curtabindex++), 66
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
        if (skeyfieldnames != null)
        {
            if (skeyfieldnames.length > 0)
            {
                String[] snewkeyfieldnames = JavaTools.removeOutdatedFields(skeyfieldnames, fieldnames);
                curPrimaryKeySelection.setSelectedFieldNames(snewkeyfieldnames);
            }
        }
        String selfield = lstSinglePrimeKey.getSelectedItem();
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), "StringItemList", fieldnames);
        if (selfield != null)
        {
            if (JavaTools.FieldInList(fieldnames, selfield) > -1)
            {
                lstSinglePrimeKey.selectItem(selfield, true);
            }
        }
        togglePrimeKeyFields();
    }

    private boolean isAutoPrimeKeyAutoIncrementationsupported()
    {
        TypeInspector.TypeInfo aAutoPrimeTypeInfo;
        aAutoPrimeTypeInfo = curTableDescriptor.oTypeInspector.findAutomaticPrimaryKeyType();
        return aAutoPrimeTypeInfo.bisAutoIncrementable;
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

    public void togglePrimeKeyFields()
    {
        boolean bdoEnable = (this.chkcreatePrimaryKey.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), PropertyNames.PROPERTY_ENABLED, new Boolean(bAutoPrimaryKeysupportsAutoIncrmentation && bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseExisting), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseSeveral), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoEnable));
        //toggle subcontrols of the radiobuttons...
        toggleAutomaticAutoValueCheckBox();
        boolean benableSinglePrimekeyControls = bdoEnable && optUseExisting.getState();
        toggleSinglePrimeKeyFields(benableSinglePrimekeyControls);
        boolean benableSeveralPrimekeyControls = bdoEnable && optUseSeveral.getState();
        curPrimaryKeySelection.toggleListboxControls(new Boolean(benableSeveralPrimekeyControls));
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
        boolean bisAutoIncrementable = false;
        try
        {
            XPropertySet xColPropertySet = curTableDescriptor.getByName(_fieldname);
            if (xColPropertySet != null)
            {
                if (curTableDescriptor.getDBDataTypeInspector() != null)
                {
                    return curTableDescriptor.getDBDataTypeInspector().isAutoIncrementable(xColPropertySet);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
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

    public void onPrimeKeySelected()
    {
        try
        {
            String selfieldname = lstSinglePrimeKey.getSelectedItem();
            boolean bdoenable = isAutoIncrementatable(selfieldname);
            CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoenable));
            XPropertySet xColPropertySet = curTableDescriptor.getByName(selfieldname);
            boolean bIsAutoIncremented = ((Boolean) xColPropertySet.getPropertyValue("IsAutoIncrement")).booleanValue();
            if (bIsAutoIncremented)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_STATE, new Short((short) 1));
            }
            else
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_STATE, new Short((short) 0));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void toggleAutomaticAutoValueCheckBox()
    {
        try
        {
            boolean bisAutomaticMode = AnyConverter.toBoolean(Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), PropertyNames.PROPERTY_ENABLED));
            boolean bdoenable = bAutoPrimaryKeysupportsAutoIncrmentation && optAddAutomatically.getState() && bisAutomaticMode;
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoenable));
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void toggleSinglePrimeKeyFields(boolean _bdoenable)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblPrimeFieldName), PropertyNames.PROPERTY_ENABLED, new Boolean(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), PropertyNames.PROPERTY_ENABLED, new Boolean(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, new Boolean(_bdoenable));
        boolean bdoenableAutoValueCheckBox = (isAutoIncrementatable(lstSinglePrimeKey.getSelectedItem()) && _bdoenable);
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), PropertyNames.PROPERTY_ENABLED, new Boolean(bdoenableAutoValueCheckBox));
    }

    private void toggleSeveralPrimeKeyFields()
    {
        boolean bdoEnable = (this.optUseSeveral.getState());
        curPrimaryKeySelection.toggleListboxControls(new Boolean(bdoEnable));
    }

    public String[] getPrimaryKeyFields(TableDescriptor _curtabledescriptor)
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
            return (new String[]
                    {
                        lstSinglePrimeKey.getSelectedItem()
                    });
        }
        else if (optAddAutomatically.getState())
        {
            return (new String[]
                    {
                        SAUTOMATICKEYFIELDNAME
                    });
        }
        return null;
    }

    public int getID()
    {
        return 0;
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
