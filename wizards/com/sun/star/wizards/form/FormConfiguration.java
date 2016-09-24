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
package com.sun.star.wizards.form;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.RelationController;
import com.sun.star.wizards.ui.CommandFieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.event.XActionListenerAdapter;
import com.sun.star.wizards.ui.event.XItemListenerAdapter;

/**
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class FormConfiguration
{

    private WizardDialog CurUnoDialog;
    private XRadioButton optOnExistingRelation;
    private XCheckBox chkcreateSubForm;
    private XRadioButton optSelectManually;
    private XFixedText lblRelations;
    private XListBox lstRelations;
    private String[] sreferencedTables;
    private CommandFieldSelection CurSubFormFieldSelection;
    private boolean bsupportsRelations;
    private RelationController oRelationController = null;

    public FormConfiguration(WizardDialog _CurUnoDialog)
    {
        this.CurUnoDialog = _CurUnoDialog;
        short curtabindex = (short) (FormWizard.SOSUBFORM_PAGE * 100);
        Integer ISubFormStep = Integer.valueOf(FormWizard.SOSUBFORM_PAGE);
        String sOnExistingRelation = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 5);
        String sOnManualRelation = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 7);
        String sSelectManually = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 4);
        String sSelectRelation = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 8);
        String sSubFormDescription = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 3);

        // CheckBox 'Add sub form'
        chkcreateSubForm = CurUnoDialog.insertCheckBox("chkcreateSubForm", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        toggleSubFormMode();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_CHKCREATESUBFORM", sSelectManually, 97, 26, ISubFormStep, Short.valueOf(curtabindex++), 160
                });
        optOnExistingRelation = CurUnoDialog.insertRadioButton("optOnExistingRelation", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        toggleSteps();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTONEXISTINGRELATION", sOnExistingRelation, 107, 43, ISubFormStep, Short.valueOf(curtabindex++), 160
                });
        optSelectManually = CurUnoDialog.insertRadioButton("optSelectManually", new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        toggleSteps();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTSELECTMANUALLY", sOnManualRelation, 107, 99, Short.valueOf((short) 1), ISubFormStep, Short.valueOf(curtabindex++), 160
                });
        lblRelations = CurUnoDialog.insertLabel("lblSelectRelation",
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, 19, sSelectRelation, Boolean.TRUE, 119, 56, ISubFormStep, Short.valueOf(curtabindex++), 80
                });
        lstRelations = CurUnoDialog.insertListBox("lstrelations", new XActionListenerAdapter() {
                    @Override
                    public void actionPerformed(ActionEvent event) {
                        onexistingRelationSelection();
                    }
                }, new XItemListenerAdapter() {
                    @Override
                    public void itemStateChanged(ItemEvent event) {
                        onexistingRelationSelection();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, 37, "HID:WIZARDS_HID_DLGFORM_lstRELATIONS", 201, 55, ISubFormStep, Short.valueOf(curtabindex++), 103
                });
        CurUnoDialog.insertLabel("lblSubFormDescription",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    59, sSubFormDescription, Boolean.TRUE, 110, 120, ISubFormStep, Short.valueOf(curtabindex++), 190
                });
        CurUnoDialog.insertInfoImage(97, 120, ISubFormStep.intValue());
    }


    public RelationController getRelationController()
    {
        return oRelationController;
    }

    public boolean areexistingRelationsdefined()
    {
        return ((chkcreateSubForm.getState() == 1) && (optOnExistingRelation.getState()));
    }

    private void toggleSubFormMode()
    {
        boolean bdoEnable = (this.chkcreateSubForm.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(optOnExistingRelation), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoEnable && bsupportsRelations));
        Helper.setUnoPropertyValue(UnoDialog.getModel(optSelectManually), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoEnable));
        toggleSteps();
    }

    public void initialize(CommandFieldSelection _CurSubFormFieldSelection, RelationController _oRelationController)
    {
        oRelationController = _oRelationController;
        sreferencedTables = oRelationController.getExportedKeys();
        bsupportsRelations = (sreferencedTables.length > 0);
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstRelations), PropertyNames.STRING_ITEM_LIST, sreferencedTables);
        this.CurSubFormFieldSelection = _CurSubFormFieldSelection;
        toggleRelationsListbox();
        Helper.setUnoPropertyValue(UnoDialog.getModel(optOnExistingRelation), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bsupportsRelations && (chkcreateSubForm.getState() == 1)));
    }

    private void toggleSteps()
    {
        if (chkcreateSubForm.getState() == 1)
        {
            if (optOnExistingRelation.getState())
            {
                onexistingRelationSelection();
            }
            else if (optSelectManually.getState())
            {
                CurUnoDialog.enablefromStep(FormWizard.SOFIELDLINKER_PAGE, (CurSubFormFieldSelection.getSelectedFieldNames().length > 0));
                CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDS_PAGE, true);
            }
        }
        else
        {
            CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDS_PAGE, false);
            CurUnoDialog.setStepEnabled(FormWizard.SOFIELDLINKER_PAGE, false);
            CurUnoDialog.enablefromStep(FormWizard.SOCONTROL_PAGE, true);
        }
        toggleRelationsListbox();
    }

    public String getreferencedTableName()
    {
        if (areexistingRelationsdefined())
        {
            short[] iselected = (short[]) Helper.getUnoArrayPropertyValue(UnoDialog.getModel(lstRelations), PropertyNames.SELECTED_ITEMS);
            if (iselected != null && iselected.length > 0)
            {
                return sreferencedTables[iselected[0]];
            }
        }
        return PropertyNames.EMPTY_STRING;
    }

    private void onexistingRelationSelection()
    {
        String scurreferencedTableName = getreferencedTableName();
        if (scurreferencedTableName.length() > 0)
        {
            if (CurSubFormFieldSelection.getSelectedCommandName().equals(scurreferencedTableName))
            {
                CurUnoDialog.enablefromStep(FormWizard.SOSUBFORMFIELDS_PAGE, true);
                CurUnoDialog.setStepEnabled(FormWizard.SOFIELDLINKER_PAGE, false);
                return;
            }
            else
            {
                CurUnoDialog.setStepEnabled(FormWizard.SOSUBFORMFIELDS_PAGE, true);
                CurUnoDialog.enablefromStep(FormWizard.SOFIELDLINKER_PAGE, false);
                return;
            }
        }
        CurUnoDialog.enablefromStep(FormWizard.SOSUBFORMFIELDS_PAGE, false);
    }

    private void toggleRelationsListbox()
    {
        boolean bdoenable = bsupportsRelations && this.optOnExistingRelation.getState() && (chkcreateSubForm.getState() == 1);
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblRelations), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(lstRelations), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bdoenable));
    }

    public boolean hasSubForm()
    {
        return (this.chkcreateSubForm.getState() == 1);
    }
}
