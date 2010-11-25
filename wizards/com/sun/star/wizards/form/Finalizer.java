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
package com.sun.star.wizards.form;

import com.sun.star.awt.XRadioButton;
import com.sun.star.awt.XTextComponent;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.*;

/**
 * @author Administrator
 *
 * To change the template for this generated type comment go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
public class Finalizer
{

    WizardDialog CurUnoDialog;
    short curtabindex;
    XRadioButton optModifyForm;
    XRadioButton optWorkWithForm;
    XTextComponent txtFormName;
    FormDocument oFormDocument;

    public Finalizer(WizardDialog _CurUnoDialog)
    {
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (FormWizard.SOSTORE_PAGE * 100);

        String slblFormName = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 50);
        String slblProceed = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 51);
        String sWorkWithForm = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 52);
        String sModifyForm = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 53);
        CurUnoDialog.insertLabel("lblFormName",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], slblFormName, new Integer(97), new Integer(25), UIConsts.INTEGERS[8], new Short(curtabindex++), new Integer(111)
                });
        txtFormName = CurUnoDialog.insertTextField("txtFormName", "toggleFinishButton", this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGER_12, "HID:WIZARDS_HID_DLGFORM_TXTPATH", new Integer(97), new Integer(35), UIConsts.INTEGERS[8], new Short((short) 82), "", new Integer(185)
                });
        CurUnoDialog.insertLabel("lblProceed",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], slblProceed, new Integer(97), new Integer(62), UIConsts.INTEGERS[8], new Short(curtabindex++), new Integer(185)
                });
        CurUnoDialog.insertRadioButton("optWorkWithForm", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTWORKWITHFORM", sWorkWithForm, new Integer(101), new Integer(77), new Short((short) 1), UIConsts.INTEGERS[8], new Short(curtabindex++), new Integer(107)
                });
        optModifyForm = CurUnoDialog.insertRadioButton("optModifyForm", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTMODIFYFORM", sModifyForm, new Integer(101), new Integer(89), UIConsts.INTEGERS[8], new Short(curtabindex++), new Integer(107)
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

    public void toggleFinishButton()
    {
        CurUnoDialog.enableFinishButton(txtFormName.getText().length() > 0);
    }

    public String getName()
    {
        return txtFormName.getText();
    }

    public boolean getOpenForEditing()
    {
        return optModifyForm.getState() ? true : false;
    }

    public boolean finish()
    {
//        if (!oFormDocument.oMainFormDBMetaData.hasFormDocumentByName(sFormName)){
        return oFormDocument.oMainFormDBMetaData.storeDatabaseDocumentToTempPath(this.oFormDocument.xComponent, getName());
//        }
    }
}
