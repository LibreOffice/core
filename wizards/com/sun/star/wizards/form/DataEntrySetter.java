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

import com.sun.star.awt.XCheckBox;
import com.sun.star.awt.XRadioButton;
import com.sun.star.beans.PropertyValue;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.common.PropertyNames;

public class DataEntrySetter
{

    private WizardDialog CurUnoDialog;
    private short curtabindex;
    private XRadioButton optNewDataOnly;
    private XRadioButton optDisplayAllData;
    private XCheckBox chknomodification;
    private XCheckBox chknodeletion;
    private XCheckBox chknoaddition;

    public DataEntrySetter(WizardDialog _CurUnoDialog)
    {
        this.CurUnoDialog = _CurUnoDialog;
        curtabindex = (short) (FormWizard.SODATA_PAGE * 100);
        Integer IDataStep = new Integer(FormWizard.SODATA_PAGE);
        String sNewDataOnly = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 44);    //
        String sDisplayAllData = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 46); //
        String sNoModification = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 47); // AllowUpdates
        String sNoDeletion = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 48);     // AllowDeletes
        String sNoAddition = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 49);     // AlowInserts
        String sdontdisplayExistingData = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 45);

        optNewDataOnly = CurUnoDialog.insertRadioButton("optNewDataOnly", "toggleCheckBoxes", this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTNEWDATAONLY", sNewDataOnly, 98, 25, IDataStep, new Short(curtabindex++), 195
                });

        optDisplayAllData = CurUnoDialog.insertRadioButton("optDisplayAllData", "toggleCheckBoxes", this,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_OPTDISPLAYALLDATA", sDisplayAllData, 98, 50, new Short((short) 1), IDataStep, new Short(curtabindex++), 197
                });
        chknomodification = CurUnoDialog.insertCheckBox("chknomodification", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_CHKNOMODIFICATION", sNoModification, 108, 62, new Short((short) 0), IDataStep, new Short(curtabindex++), 189
                });
        chknodeletion = CurUnoDialog.insertCheckBox("chknodeletion", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_CHKNODELETION", sNoDeletion, 108, 74, new Short((short) 0), IDataStep, new Short(curtabindex++), 189
                });
        chknoaddition = CurUnoDialog.insertCheckBox("chknoaddition", null,
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    UIConsts.INTEGERS[8], "HID:WIZARDS_HID_DLGFORM_CHKNOADDITION", sNoAddition, 108, 86, new Short((short) 0), IDataStep, new Short(curtabindex++), 191
                });
        CurUnoDialog.insertLabel("lbldontdisplayExistingData",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, sdontdisplayExistingData, 108, 33, IDataStep, new Short(curtabindex++), 134
                });
    }

    public PropertyValue[] getFormProperties()
    {
        PropertyValue[] retProperties;
        if (optDisplayAllData.getState())
        {
            retProperties = new PropertyValue[3];
            boolean bAllowUpdates = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknomodification), PropertyNames.PROPERTY_STATE)).shortValue()) != 1;
            boolean bAllowDeletes = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknodeletion), PropertyNames.PROPERTY_STATE)).shortValue()) != 1;
            boolean bAllowInserts = (((Short) Helper.getUnoPropertyValue(UnoDialog.getModel(chknoaddition), PropertyNames.PROPERTY_STATE)).shortValue()) != 1;
            retProperties[0] = Properties.createProperty("AllowUpdates", new Boolean(bAllowUpdates));
            retProperties[1] = Properties.createProperty("AllowDeletes", new Boolean(bAllowDeletes));
            retProperties[2] = Properties.createProperty("AllowInserts", new Boolean(bAllowInserts));
        }
        else
        {
            retProperties = new PropertyValue[1];
            retProperties[0] = Properties.createProperty("IgnoreResult", new Boolean(true));
        }
        return retProperties;

    }

    public void toggleCheckBoxes()
    {
        boolean bdisplayalldata = optDisplayAllData.getState();
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknomodification), PropertyNames.PROPERTY_ENABLED, new Boolean(bdisplayalldata));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknodeletion), PropertyNames.PROPERTY_ENABLED, new Boolean(bdisplayalldata));
        Helper.setUnoPropertyValue(UnoDialog.getModel(chknoaddition), PropertyNames.PROPERTY_ENABLED, new Boolean(bdisplayalldata));
    }
}
