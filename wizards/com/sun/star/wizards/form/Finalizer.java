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

import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.ui.event.XTextListenerAdapter;

/**
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class Finalizer
{

    private WizardDialog CurUnoDialog;
    private XRadioButton optModifyForm;
    private XTextComponent txtFormName;
    private FormDocument oFormDocument;

    public Finalizer(WizardDialog _CurUnoDialog)
    {
        this.CurUnoDialog = _CurUnoDialog;
        short curtabindex = (short) (FormWizard.SOSTORE_PAGE * 100);

        String slblFormName = CurUnoDialog.m_oResource.getResText("RID_FORM_50");
        String slblProceed = CurUnoDialog.m_oResource.getResText("RID_FORM_51");
        String sWorkWithForm = CurUnoDialog.m_oResource.getResText("RID_FORM_52");
        String sModifyForm = CurUnoDialog.m_oResource.getResText("RID_FORM_53");
        CurUnoDialog.insertLabel("lblFormName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], slblFormName, 97, 25, UIConsts.INTEGERS[8], Short.valueOf(curtabindex++), 111
                });
        txtFormName = CurUnoDialog.insertTextField("txtFormName", new XTextListenerAdapter() {
                    @Override
                    public void textChanged(TextEvent event) {
                        toggleFinishButton();
                    }
                },
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGER_12, "HID:WIZARDS_HID_DLGFORM_TXTPATH", 97, 35, UIConsts.INTEGERS[8], Short.valueOf((short) 82), PropertyNames.EMPTY_STRING, 185
                });
        CurUnoDialog.insertLabel("lblProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], slblProceed, 97, 62, UIConsts.INTEGERS[8], Short.valueOf(curtabindex++), 185
                });
        CurUnoDialog.insertRadioButton("optWorkWithForm", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTWORKWITHFORM", sWorkWithForm, 101, 77, Short.valueOf((short) 1), UIConsts.INTEGERS[8], Short.valueOf(curtabindex++), 107
                });
        optModifyForm = CurUnoDialog.insertRadioButton("optModifyForm", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTMODIFYFORM", sModifyForm, 101, 89, UIConsts.INTEGERS[8], Short.valueOf(curtabindex++), 107
                });
    }

    public void initialize(String _formname, FormDocument _oFormDocument)
    {
        if (oFormDocument == null)
        {
            oFormDocument = _oFormDocument;
        }
        if (txtFormName.getText().length() == 0)
        {
            txtFormName.setText(Desktop.getUniqueName(_oFormDocument.oMainFormDBMetaData.getFormDocuments(), _formname));
        }
    }

    private void toggleFinishButton()
    {
        CurUnoDialog.enableFinishButton(txtFormName.getText().length() > 0);
    }

    public String getName()
    {
        return txtFormName.getText();
    }

    public boolean getOpenForEditing()
    {
        return optModifyForm.getState();
    }

    public boolean finish()
    {
        return oFormDocument.oMainFormDBMetaData.storeDatabaseDocumentToTempPath(this.oFormDocument.xComponent, getName());
    }
}
