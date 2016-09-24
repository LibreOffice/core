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
package com.sun.star.wizards.report;

import java.util.ArrayList;

import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.db.CommandMetaData;
import com.sun.star.wizards.db.QueryMetaData;
import com.sun.star.wizards.ui.FieldSelection;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.WizardDialog;

public class GroupFieldHandler extends FieldSelection
{

    private IReportDocument CurReportDocument;
    private final ArrayList<String> GroupFieldVector = new ArrayList<String>();
    private QueryMetaData CurDBMetaData;
    private static final short MAXSELFIELDS = 4;

    public GroupFieldHandler(IReportDocument _CurReportDocument, WizardDialog _CurUnoDialog)
    {
        super(_CurUnoDialog, ReportWizard.SOGROUPPAGE, 95, 27, 210, 127,
                _CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 19),
                _CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 50), 34340, false);

        try
        {
            this.CurReportDocument = _CurReportDocument;
            this.CurDBMetaData = CurReportDocument.getRecordParser();
            CurUnoDialog.setControlProperty("lstFields_2", "MultiSelection", Boolean.FALSE);
            CurUnoDialog.setControlProperty("lstSelFields_2", "MultiSelection", Boolean.FALSE);
            addFieldSelectionListener(new FieldSelectionListener());
            String sNote = ReportWizard.getBlindTextNote(_CurReportDocument, _CurUnoDialog.m_oResource);
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblBlindTextNote_1",
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_MULTILINE, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                            Boolean.FALSE, 18, sNote, Boolean.TRUE, 95, 158, Integer.valueOf(ReportWizard.SOGROUPPAGE), 209
                    });
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private boolean isGroupField(String _FieldName)
    {
        return (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, _FieldName) != -1);
    }

    public void initialize()
    {
        try
        {
            ArrayList<String> NormalFieldsVector = new ArrayList<String>();
            ArrayList<String> SelFieldsVector = new ArrayList<String>();
            String[] sFieldNames = CurDBMetaData.getFieldNames();
            for (int i = 0; i < sFieldNames.length; i++)
            {
                String sfieldtitle = sFieldNames[i]; // CurDBMetaData.getFieldTitle(sFieldNames[i]);
                if (isGroupField(sFieldNames[i]))
                {
                    SelFieldsVector.add(sfieldtitle);
                }
                else
                {
                    NormalFieldsVector.add(sfieldtitle);
                }
            }
            String[] SelFields = new String[SelFieldsVector.size()];
            SelFieldsVector.toArray(SelFields);
            String[] NormalFields = new String[NormalFieldsVector.size()];
            NormalFieldsVector.toArray(NormalFields);
            super.initialize(NormalFields, SelFields, true);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void removeGroupFieldNames()
    {
        emptyFieldsListBoxes();
        GroupFieldVector.clear();
        CurUnoDialog.setControlProperty("lblBlindTextNote_1", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
    }

    public void getGroupFieldNames(CommandMetaData CurDBMetaData)
    {
        String[] GroupFieldNames = new String[GroupFieldVector.size()];
        GroupFieldVector.toArray(GroupFieldNames);
        CurDBMetaData.GroupFieldNames = GroupFieldNames;
    }
    // @Override
    @Override
    protected void toggleListboxButtons(short iFieldsSelIndex, short iSelFieldsSelIndex)
    {
        super.toggleListboxButtons(iFieldsSelIndex, iSelFieldsSelIndex);
        int iSelCount = xSelectedFieldsListBox.getItemCount();
        if (iSelCount >= MAXSELFIELDS)
            {
            CurUnoDialog.setControlProperty("cmdMoveSelected" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        }
    }

    @Override
    public void selectFields(boolean bMoveAll)
    {
        int iSelCount = xSelectedFieldsListBox.getItemCount();
        if (iSelCount < MAXSELFIELDS)
            {
            super.selectFields(bMoveAll);
        }

    }

    /* protected */ private class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
    {

        public void moveItemDown(String Selitem)
        {
            CurReportDocument.refreshGroupFields(xSelectedFieldsListBox.getItems());
        }

        public void moveItemUp(String item)
        {
            CurReportDocument.refreshGroupFields(xSelectedFieldsListBox.getItems());
        }

        public void shiftFromLeftToRight(String[] Selitems, String[] Newitems)
        {
            String CurGroupTitle = Selitems[0];
            int iSelCount = xSelectedFieldsListBox.getItemCount();
            String[] CurGroupNames = xFieldsListBox.getItems();
            CurReportDocument.liveupdate_addGroupNametoDocument(CurGroupNames, CurGroupTitle, GroupFieldVector, CurReportDocument.getReportPath(), iSelCount);
            CurUnoDialog.setControlProperty("lblBlindTextNote_1", PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
            if (iSelCount >= MAXSELFIELDS)
            {
                toggleMoveButtons(false, false);
            }
        }

        public void shiftFromRightToLeft(String[] OldSelitems, String[] Newitems)
        {
            int iSelPos = OldSelitems.length;
            if (iSelPos > 0)
            {
                String OldGroupTitle = OldSelitems[0];
                String[] NewSelList = xSelectedFieldsListBox.getItems();
                CurReportDocument.liveupdate_removeGroupName(NewSelList, OldGroupTitle, GroupFieldVector);
                String[] NewSelGroupNames = xSelectedFieldsListBox.getItems();
                CurUnoDialog.setControlProperty("lblBlindTextNote_1", PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(NewSelGroupNames.length == 0));

            // CurReportDocument.refreshGroupFields(xSelectedFieldsListBox.getItems());
            }
        }

        public void setID(String sIncSuffix)
        {
        }
    }
}
