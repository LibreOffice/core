/*************************************************************************
*
*  $RCSfile: PrimaryKeyHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: pjunck $ $Date: 2004-10-27 13:38:36 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*/


package com.sun.star.wizards.table;

import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.db.TableDescriptor;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.XFieldSelectionListener;


public class PrimaryKeyHandler implements XFieldSelectionListener {

    WizardDialog CurUnoDialog;
    short curtabindex;
    String SPRIMEKEYMODE = "togglePrimeKeyFields";
    String SSINGLEKEYMODE = "toggleSinglePrimeKeyFields";
    String SSEVERALKEYMODE = "toggleSeveralPrimeKeyFields";
    XRadioButton optAddAutomatically;
    XRadioButton optUseExisting;
    XRadioButton optUseSeveral;
    XCheckBox chkUsePrimaryKey;
    XCheckBox chkcreatePrimaryKey;
    XCheckBox chkApplyAutoValue;
    XListBox lstSinglePrimeKey;
    XFixedText lblPrimeFieldName;
    FieldSelection curPrimaryKeySelection;
    String[] fieldnames;

    public PrimaryKeyHandler(WizardDialog _CurUnoDialog){
        this.CurUnoDialog = _CurUnoDialog;
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
              new Object[] { new Integer(40), sExplanations,  Boolean.TRUE, new Integer(97), new Integer(18),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(207)}
        );

        chkcreatePrimaryKey = CurUnoDialog.insertCheckBox("chkcreatePrimaryKey", SPRIMEKEYMODE, this,
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
            new Object[] {UIConsts.INTEGERS[8], "HID:41229", screatePrimaryKey, new Integer(97), new Integer(58), new Short((short)1), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(160)});

        optAddAutomatically = CurUnoDialog.insertRadioButton("optAddAutomatically", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
              new Object[] { UIConsts.INTEGERS[8], "HID:41231", sAddAutomatically, new Integer(103), new Integer(70), new Short((short)1),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        optUseExisting = CurUnoDialog.insertRadioButton("optUseExisting", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
              new Object[] { UIConsts.INTEGERS[8], "HID:41230", sUseExisting, new Integer(103), new Integer(82),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        optUseSeveral = CurUnoDialog.insertRadioButton("optUseSeveral", SPRIMEKEYMODE, this,
              new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
              new Object[] { UIConsts.INTEGERS[8], "HID:41234", sUseSeveral, new Integer(103), new Integer(110),IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(200)}
        );

        lblPrimeFieldName = CurUnoDialog.insertLabel("lbPrimeFieldName",
            new String[] {"Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] { Boolean.FALSE, UIConsts.INTEGERS[8], slblPrimeFieldName, new Integer(110), new Integer(95), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(49)}
        );

        lstSinglePrimeKey = CurUnoDialog.insertListBox("lstSinglePrimeKey", "onPrimeKeySelected", null, this,
            new String[] {"Dropdown", "Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {Boolean.TRUE, Boolean.FALSE, new Integer(12), "HID:41232", new Integer(160), new Integer(93), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(70)});

        chkApplyAutoValue = CurUnoDialog.insertCheckBox("chkApplyAutoValue", SPRIMEKEYMODE, this,
            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
            new Object[] {UIConsts.INTEGERS[8], "HID:41233", sApplyAutoValue, new Integer(232), new Integer(95), IPRIMEKEYSTEP, new Short(curtabindex++), new Integer(68)});
        curPrimaryKeySelection = new FieldSelection(CurUnoDialog, IPRIMEKEYSTEP.intValue(), 114, 122, 190, 56, slblAvailableFields, slblSelPrimaryFields, 41235, false);
        curPrimaryKeySelection.addFieldSelectionListener(this);
    }


    public void initialize(TableDescriptor curTableDescriptor){
        boolean breselect;
        fieldnames = curTableDescriptor.getFieldnames();
//      fieldnames = setNumericFields();
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


    public void togglePrimeKeyFields(){
        boolean bdoEnable = (this.chkcreatePrimaryKey.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optAddAutomatically), "Enabled", new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseExisting), "Enabled", new Boolean(bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optUseSeveral), "Enabled", new Boolean(bdoEnable));
        boolean benableSinglePrimekeyControls = bdoEnable && optUseExisting.getState();
        toggleSinglePrimeKeyFields(benableSinglePrimekeyControls);
        boolean benableSeveralPrimekeyControls = bdoEnable && optUseSeveral.getState();
        curPrimaryKeySelection.toggleListboxControls(new Boolean(benableSeveralPrimekeyControls));
        if (!bdoEnable)
            CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, true);
        else{
            if (benableSeveralPrimekeyControls)
                CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
            else if (benableSinglePrimekeyControls)
                CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
            else if (optAddAutomatically.getState())
                CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, true);
        }
    }


    public void onPrimeKeySelected(){
        CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, lstSinglePrimeKey.getSelectedItemPos() != -1);
    }


    public void toggleSinglePrimeKeyFields(boolean _bdoEnable){
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblPrimeFieldName), "Enabled", new Boolean(_bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chkApplyAutoValue), "Enabled", new Boolean(_bdoEnable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstSinglePrimeKey), "Enabled", new Boolean(_bdoEnable));
    }


    public void toggleSeveralPrimeKeyFields(){
        boolean bdoEnable = (this.optUseSeveral.getState());
        curPrimaryKeySelection.toggleListboxControls(new Boolean(bdoEnable));
    }


    public boolean IsAutoIncrement(){
        return false;
    }


    public String[] getPrimaryKeyFields(TableDescriptor _curtabledescriptor){
        if (fieldnames == null)
            initialize(_curtabledescriptor);
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
        CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }

    public void shiftFromRightToLeft(String[] OldSelItems, String[] NewItems) {
        CurUnoDialog.enablefromStep(TableWizard.SOFINALPAGE, (curPrimaryKeySelection.getSelectedFieldNames().length > 0));
    }

}
