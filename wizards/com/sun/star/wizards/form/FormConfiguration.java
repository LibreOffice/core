/*************************************************************************
 *
 *  $RCSfile: FormConfiguration.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $  $Date: 2004-10-27 13:33:48 $
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
 *  Contributor(s): Berend Cornelius
 *
 */

package com.sun.star.wizards.form;

import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.ui.CommandFieldSelection;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class FormConfiguration {
    WizardDialog CurUnoDialog;
    short curtabindex;
    XRadioButton optOnExistingRelation;
    XCheckBox chkcreateSubForm;
    XRadioButton optSelectManually;
    XFixedText lblSubFormDescription;
    XFixedText lblRelations;
    XListBox lstRelations;
    String[] sreferencedTables;
    Integer ISubFormStep;
    CommandFieldSelection CurSubFormFieldSelection;
    String SSUBFORMMODE = "toggleSubFormMode";
    String STOGGLESTEPS = "toggleSteps";
    String SONEXISTINGRELATIONSELECTION = "onexistingRelationSelection";
    boolean bsupportsRelations;


    public FormConfiguration(WizardDialog _CurUnoDialog){
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (FormWizard.SOSUBFORMPAGE * 100);
        Integer ISubFormStep = new Integer(FormWizard.SOSUBFORMPAGE);
        String sOnExistingRelation = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 5);
        String sOnManualRelation = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 7);
        String sSelectManually = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 4);
        String sSelectRelation = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 8);
        String sSubFormDescription = CurUnoDialog.oResource.getResText(UIConsts.RID_FORM + 3);

        chkcreateSubForm = CurUnoDialog.insertCheckBox("chkcreateSubForm", SSUBFORMMODE, this,
                            new String[] {"Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                              new Object[] {UIConsts.INTEGERS[8], "HID:34421", sSelectManually, new Integer(97), new Integer(26), ISubFormStep, new Short(curtabindex++), new Integer(160)});
        optOnExistingRelation = CurUnoDialog.insertRadioButton("optOnExistingRelation", STOGGLESTEPS, this,
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {Boolean.FALSE, UIConsts.INTEGERS[8], "HID:34422", sOnExistingRelation, new Integer(103), new Integer(43), ISubFormStep, new Short(curtabindex++), new Integer(160)});
        optSelectManually = CurUnoDialog.insertRadioButton("optSelectManually", STOGGLESTEPS, this,
                            new String[] {"Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "State", "Step", "TabIndex", "Width"},
                            new Object[] {Boolean.FALSE, UIConsts.INTEGERS[8], "HID:34423", sOnManualRelation, new Integer(103), new Integer(99), new Short((short)1), ISubFormStep, new Short(curtabindex++), new Integer(160)});
        lblRelations = CurUnoDialog.insertLabel("lblSelectRelation",
                            new String[] {"Enabled", "Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {Boolean.FALSE, new Integer(19), sSelectRelation, Boolean.TRUE, new Integer(119), new Integer(56), ISubFormStep, new Short(curtabindex++), new Integer(80)});
        lstRelations = CurUnoDialog.insertListBox("lstrelations", SONEXISTINGRELATIONSELECTION, SONEXISTINGRELATIONSELECTION, this,
                            new String[] {"Enabled", "Height", "HelpURL", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] {Boolean.FALSE, new Integer(37), "HID:34424", new Integer(201), new Integer(55), ISubFormStep, new Short(curtabindex++), new Integer(103)});
        lblSubFormDescription = CurUnoDialog.insertLabel("lblSubFormDescription",
                            new String[] {"Height", "Label", "MultiLine", "PositionX", "PositionY", "Step", "TabIndex", "Width"},
                            new Object[] { new Integer(59), sSubFormDescription,Boolean.TRUE, new Integer(110), new Integer(120), ISubFormStep, new Short(curtabindex++), new Integer(190)});
        CurUnoDialog.insertInfoImage(97, 120, ISubFormStep.intValue());
    }


    public boolean areexistingRelationsdefined(){
        return ((chkcreateSubForm.getState() == 1) && (optOnExistingRelation.getState()));
    }

    public void toggleSubFormMode(){
        boolean bdoEnable = (this.chkcreateSubForm.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optOnExistingRelation), "Enabled", new Boolean(bdoEnable && bsupportsRelations));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optSelectManually), "Enabled", new Boolean(bdoEnable));
        toggleSteps();
    }

    public void initialize(CommandFieldSelection _CurSubFormFieldSelection, String[] _sreferencedTables){
        sreferencedTables = _sreferencedTables;
        bsupportsRelations = (_sreferencedTables.length > 0);
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstRelations), "StringItemList", _sreferencedTables);
        this.CurSubFormFieldSelection = _CurSubFormFieldSelection;
        toggleRelationsListbox();
        Helper.setUnoPropertyValue(UnoDialog.getModel(optOnExistingRelation), "Enabled", new Boolean(bsupportsRelations && (chkcreateSubForm.getState() == 1)));
    }


    public void toggleSteps(){
        boolean bDoEnableFollowingSteps;
        if (chkcreateSubForm.getState() == 1){
            if (optOnExistingRelation.getState()){
                onexistingRelationSelection();
            }
            else if (optSelectManually.getState()){
                CurUnoDialog.enablefromStep(FormWizard.SOFIELDLINKERPAGE, (CurSubFormFieldSelection.getSelectedFieldNames().length > 0 ));
                CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDSPAGE, true);
            }
        }
        else{
            CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDSPAGE, false);
            CurUnoDialog.setStepEnabled(FormWizard.SOFIELDLINKERPAGE, false);
            CurUnoDialog.enablefromStep(FormWizard.SOCONTROLPAGE, true);
        }
        toggleRelationsListbox();
    }

    public String getreferencedTableName(){
        if (areexistingRelationsdefined()){
            short[] iselected = (short[]) Helper.getUnoArrayPropertyValue(UnoDialog.getModel(lstRelations), "SelectedItems");
            if (iselected != null){
                if (iselected.length > 0)
                    return sreferencedTables[iselected[0]];
            }
        }
        return "";
    }


    public void onexistingRelationSelection(){
        String scurreferencedTableName = getreferencedTableName();
        if (scurreferencedTableName.length() > 0){
            if (CurSubFormFieldSelection.getSelectedCommandName().equals(scurreferencedTableName)){
                CurUnoDialog.enablefromStep(FormWizard.SOSUBFORMFIELDSPAGE, true);
                CurUnoDialog.setStepEnabled(FormWizard.SOFIELDLINKERPAGE, false);
                return;
            }
            else{
                CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDSPAGE, true);
                CurUnoDialog.enablefromStep(FormWizard.SOFIELDLINKERPAGE, false);
                return;
            }
        }
        CurUnoDialog.enablefromStep(FormWizard.SOSUBFORMFIELDSPAGE, false);
    }

    private void toggleRelationsListbox(){
        boolean bdoenable = bsupportsRelations &&  this.optOnExistingRelation.getState() && (chkcreateSubForm.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblRelations), "Enabled", new Boolean(bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstRelations), "Enabled", new Boolean(bdoenable));
    }


    public boolean hasSubForm(){
        return (this.chkcreateSubForm.getState() == 1);
    }

}
