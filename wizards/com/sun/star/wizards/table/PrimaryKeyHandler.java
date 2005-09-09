/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PrimaryKeyHandler.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:40:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.wizards.table;

import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.DataType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.db.DBMetaData;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.db.TypeInspector;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;


public class PrimaryKeyHandler implements XFieldSelectionListener {

    TableWizard CurUnoDialog;
    short curtabindex;
    String SPRIMEKEYMODE = "togglePrimeKeyFields";
    String SSINGLEKEYMODE = "toggleSinglePrimeKeyFields";
    String SSEVERALKEYMODE = "toggleSeveralPrimeKeyFields";
    XRadioButton optAddAutomatically;
    XRadioButton optUseExisting;
    XRadioButton optUseSeveral;
    XCheckBox chkUsePrimaryKey;
    XCheckBox chkcreatePrimaryKey;
    XCheckBox chkApplyAutoValueExisting;
    XCheckBox chkApplyAutoValueAutomatic;

    XListBox lstSinglePrimeKey;
    XFixedText lblPrimeFieldName;
    FieldSelection curPrimaryKeySelection;
    String[] fieldnames;
    TableDescriptor curTableDescriptor;
    int nAutoPrimeKeyDataType;
    boolean bAutoPrimaryKeysupportsAutoIncrmentation;

    public PrimaryKeyHandler(TableWizard _CurUnoDialog, TableDescriptor _curTableDescriptor){
        this.CurUnoDialog = _CurUnoDialog;
        curTableDescriptor = _curTableDescriptor;
        bAutoPrimaryKeysupportsAutoIncrmentation = isAutoPrimeKeyAutoIncrementationsupported();
        curtabindex = (short) ((TableWizard.SOPRIMARYKEYPAGE * 100) - 20);
        Integer IPRIMEKEYSTEP = new Integer(TableWizard.SOPRIMARYKEYPAGE);
        String sExplanations  = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 26);
        String screatePrimaryKey = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 27);
        String slblPrimeFieldName = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 31);
        String sApplyAutoValue = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 33);
        String sAddAutomatically = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 28);
        String sUseExisting = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 29);
        String sUseSeveral = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 30);
        String slblAvailableFields = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 4);
        String slblSelPrimaryFields = CurUnoDialog.oResource.getResText(UIConsts.RID_TABLE + 32);
        CurUnoDialog.insertLabel("lblExplanation",
              new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
              new Object[] { new Integer(40), sExplanations,  Boolean.TRUE, new Integer(91), new Integer(27),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(213)}
        );

        chkcreatePrimaryKey = CurUnoDialog.insertCheckBox("chkcreatePrimaryKey", SPRIMEKEYMODE, this,
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
            new Object[] {UIConsts.INTEGERS[8], "HID:41227", screatePrimaryKey, new Integer(97), new Integer(70), new Short((short)1), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(160)});

        optAddAutomatically = CurUnoDialog.insertRadioButton("optAddAutomatically", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
              new Object[] { UIConsts.INTEGERS[8], "HID:41228", sAddAutomatically, new Integer(106), new Integer(82), new Short((short)1),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        optUseExisting = CurUnoDialog.insertRadioButton("optUseExisting", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"}, //94
              new Object[] { UIConsts.INTEGERS[8], "HID:41230", sUseExisting, new Integer(106), new Integer(104),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        optUseSeveral = CurUnoDialog.insertRadioButton("optUseSeveral", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
              new Object[] { UIConsts.INTEGERS[8], "HID:41233", sUseSeveral, new Integer(106), new Integer(132),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        chkApplyAutoValueAutomatic = CurUnoDialog.insertCheckBox("chkApplyAutoValueAutomatic", SPRIMEKEYMODE, this,
                new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"}, //107
                new Object[] {UIConsts.INTEGERS[8], "HID:41229", sApplyAutoValue, new Integer(116), new Integer(92), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(68)});

        lblPrimeFieldName = CurUnoDialog.insertLabel("lblPrimeFieldName",
            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] { Boolean.FALSE, UIConsts.INTEGERS[8], slblPrimeFieldName, new Integer(116), new Integer(117), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(46)}
        );

        lstSinglePrimeKey = CurUnoDialog.insertListBox("lstSinglePrimeKey", "onPrimeKeySelected", null, this,
            new String[] {"Dropdown", "Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {Boolean.TRUE, Boolean.FALSE, new Integer(12), "HID:41231", new Integer(162), new Integer(115), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(70)});

        chkApplyAutoValueExisting = CurUnoDialog.insertCheckBox("chkApplyAutoValueExisting", SPRIMEKEYMODE, this,
                new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"}, //107
                new Object[] {UIConsts.INTEGERS[8], "HID:41232", sApplyAutoValue, new Integer(238), new Integer(117), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(66)});
        curPrimaryKeySelection = new FieldSelection(CurUnoDialog, IPRIMEKEYSTEP.intValue(), 116, 142, 188, 39, slblAvailableFields, slblSelPrimaryFields, 41234, false);
        curPrimaryKeySelection.addFieldSelectionListener(this);

    }


    public void initialize(){
        boolean breselect;
        fieldnames = curTableDescriptor.getNonBinaryFieldNames();
        String[] skeyfieldnames = curPrimaryKeySelection.getSelectedFieldNames();
        curPrimaryKeySelection.initialize(fieldnames, false);
        if (skeyfieldnames != null){
            if (skeyfieldnames.length > 0){
                String[] snewkeyfieldnames = JavaTools.removeOutdatedFields(skeyfieldnames, fieldnames);
                curPrimaryKeySelection.setSelectedFieldNames(snewkeyfieldnames);
            }
        }
        String selfield = lstSinglePrimeKey.getSelectedItem();
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), "StringItemList", fieldnames);
        if ( selfield != null){
            if (JavaTools.FieldInList(fieldnames, selfield) > -1)
                lstSinglePrimeKey.selectItem(selfield, true);
        }
        togglePrimeKeyFields();
    }


    private boolean isAutoPrimeKeyAutoIncrementationsupported(){
        TypeInspector.TypeInfo aAutoPrimeTypeInfo;
        aAutoPrimeTypeInfo = curTableDescriptor.oTypeInspector.findAutomaticPrimaryKeyType();
        return aAutoPrimeTypeInfo.bisAutoIncrementable;
    }

    public boolean iscompleted(){
        if (chkcreatePrimaryKey.getState() == 0)
            return true;
        if (this.optAddAutomatically.getState())
            return true;
        if (optUseExisting.getState()){
            fieldnames = curTableDescriptor.getNonBinaryFieldNames();
            String selfield = lstSinglePrimeKey.getSelectedItem();
            if ( selfield != null){
                return (JavaTools.FieldInList(fieldnames, selfield) > -1);
            }
        }
        if (optUseSeveral.getState()){
            fieldnames = curTableDescriptor.getNonBinaryFieldNames();
            String[] skeyfieldnames = curPrimaryKeySelection.getSelectedFieldNames();
            String[] snewkeyfieldnames = JavaTools.removeOutdatedFields(skeyfieldnames, fieldnames);
            return (snewkeyfieldnames.length > 0);
        }
        return false;
    }

    public void togglePrimeKeyFields(){
        boolean bdoEnable = (this.chkcreatePrimaryKey.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), "Enabled", new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), "Enabled", new Boolean(bAutoPrimaryKeysupportsAutoIncrmentation && bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseExisting), "Enabled", new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseSeveral), "Enabled", new Boolean(bdoEnable));
        //toggle subcontrols of the radiobuttons...
        toggleAutomaticAutoValueCheckBox();
        boolean benableSinglePrimekeyControls = bdoEnable && optUseExisting.getState();
        toggleSinglePrimeKeyFields(benableSinglePrimekeyControls);
        boolean benableSeveralPrimekeyControls = bdoEnable && optUseSeveral.getState();
        curPrimaryKeySelection.toggleListboxControls(new Boolean(benableSeveralPrimekeyControls));
        // toggle the following steps of the dialog...
        if (!bdoEnable)
            CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, true);
        else{
            if (benableSeveralPrimekeyControls)
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
            else if (benableSinglePrimekeyControls)
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
            else if (optAddAutomatically.getState())
                CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, true);
        }
    }


    private boolean isAutoIncrementatable(String _fieldname){
    boolean bisAutoIncrementable = false;
    try {
        XPropertySet xColPropertySet = curTableDescriptor.getByName(_fieldname);
        if (xColPropertySet != null){
            if (curTableDescriptor.getDBDataTypeInspector() != null)
                return curTableDescriptor.getDBDataTypeInspector().isAutoIncrementable(xColPropertySet);
        }
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }
    return false;
    }


    public boolean isAutoIncremented(){
    boolean bischecked = false;
    if (chkcreatePrimaryKey.getState() == 1){
        boolean bisAutomaticMode = ((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), "State")).shortValue() == (short) 1;
        boolean bisExistingMode = ((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(optUseExisting), "State")).shortValue() == (short) 1;
        if (bisAutomaticMode)
            bischecked = chkApplyAutoValueAutomatic.getState() == (short)1;
        else if (bisExistingMode)
            bischecked = chkApplyAutoValueExisting.getState() == (short)1;
    }
    return bischecked;
    }


    public void onPrimeKeySelected(){
    try {
        String selfieldname = lstSinglePrimeKey.getSelectedItem();
        boolean bdoenable = isAutoIncrementatable(selfieldname);
        CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), "Enabled", new Boolean(bdoenable));
        XPropertySet xColPropertySet = curTableDescriptor.getByName(selfieldname);
        boolean bIsAutoIncremented = ((Boolean) xColPropertySet.getPropertyValue("IsAutoIncrement")).booleanValue();
        if (bIsAutoIncremented)
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), "State", new Short((short) 1));
        else
            Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), "State", new Short((short) 0));
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    private void toggleAutomaticAutoValueCheckBox(){
    try {
        boolean bisAutomaticMode = AnyConverter.toBoolean(Helper.getUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), "Enabled"));
        boolean bdoenable = bAutoPrimaryKeysupportsAutoIncrmentation && optAddAutomatically.getState() && bisAutomaticMode;
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueAutomatic), "Enabled", new Boolean(bdoenable));
    } catch (IllegalArgumentException e) {
        e.printStackTrace(System.out);
    }}

    private void toggleSinglePrimeKeyFields(boolean _bdoenable){
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblPrimeFieldName), "Enabled", new Boolean(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), "Enabled", new Boolean(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), "Enabled", new Boolean(_bdoenable));
        boolean bdoenableAutoValueCheckBox = (isAutoIncrementatable(lstSinglePrimeKey.getSelectedItem()) && _bdoenable);
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValueExisting), "Enabled", new Boolean(bdoenableAutoValueCheckBox));
    }


    private void toggleSeveralPrimeKeyFields(){
        boolean bdoEnable = (this.optUseSeveral.getState());
        curPrimaryKeySelection.toggleListboxControls(new Boolean(bdoEnable));
    }


    public String[] getPrimaryKeyFields(TableDescriptor _curtabledescriptor){
        if (chkcreatePrimaryKey.getState() == 0){
            return null;
        }
        if (fieldnames == null)
            initialize();
        if (optUseSeveral.getState())
            return curPrimaryKeySelection.getSelectedFieldNames();
        else if (optUseExisting.getState())
            return (new String[]{lstSinglePrimeKey.getSelectedItem()});
        else if (optAddAutomatically.getState()){
            return (new String[]{"ID"});
        }
        return null;
    }


    public int getID() {
        return 0;
    }

    public void moveItemDown(String Selitem) {

    }

    public void moveItemUp(String Selitem) {

    }

    public void setID(String sIncSuffix) {

    }

    public void shiftFromLeftToRight(String[] SelItems, String[] NewItems) {
        CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }

    public void shiftFromRightToLeft(String[] OldSelItems, String[] NewItems) {
        CurUnoDialog.setcompleted(TableWizard.SOPRIMARYKEYPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }
}
