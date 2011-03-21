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
package com.sun.star.wizards.report;

import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.*;
import com.sun.star.wizards.db.*;

import java.util.Vector;

public class GroupFieldHandler extends FieldSelection
{

    private IReportDocument CurReportDocument;
    private Vector GroupFieldVector = new Vector();
    private QueryMetaData CurDBMetaData;
    private WizardDialog oWizardDialog;
    private static final short MAXSELFIELDS = 4;

    public GroupFieldHandler(IReportDocument _CurReportDocument, WizardDialog _CurUnoDialog)
    {
        super(_CurUnoDialog, ReportWizard.SOGROUPPAGE, 95, 27, 210, 127,
                _CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 19),
                _CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 50), 34340, false);

        try
        {
            this.oWizardDialog = _CurUnoDialog;
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
                            Boolean.FALSE, 18, sNote, Boolean.TRUE, 95, 158, new Integer(ReportWizard.SOGROUPPAGE), 209
                    });
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public boolean isGroupField(String _FieldName)
    {
        return (JavaTools.FieldInList(CurDBMetaData.GroupFieldNames, _FieldName) != -1);
    }

    public void initialize()
    {
        try
        {
            Vector NormalFieldsVector = new Vector();
            Vector SelFieldsVector = new Vector();
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
            exception.printStackTrace(System.out);
        }
    }

    public void removeGroupFieldNames()
    {
        emptyFieldsListBoxes();
        GroupFieldVector.removeAllElements();
        CurUnoDialog.setControlProperty("lblBlindTextNote_1", PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
    }

    public void getGroupFieldNames(CommandMetaData CurDBMetaData)
    {
        String[] GroupFieldNames = new String[GroupFieldVector.size()];
        GroupFieldVector.copyInto(GroupFieldNames);
        CurDBMetaData.GroupFieldNames = GroupFieldNames;
    }
    // @Override
    protected void toggleListboxButtons(short iFieldsSelIndex, short iSelFieldsSelIndex)
    {
        super.toggleListboxButtons(iFieldsSelIndex, iSelFieldsSelIndex);
        int iSelCount = xSelectedFieldsListBox.getItemCount();
        if (iSelCount >= MAXSELFIELDS)
            {
            CurUnoDialog.setControlProperty("cmdMoveSelected" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
        }
    }

    public void selectFields(boolean bMoveAll)
    {
        int iSelCount = xSelectedFieldsListBox.getItemCount();
        if (iSelCount < MAXSELFIELDS)
            {
            super.selectFields(bMoveAll);
        }

    }

    /* protected */ class FieldSelectionListener implements com.sun.star.wizards.ui.XFieldSelectionListener
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

        public int getID()
        {
            // TODO: here is a good place for a comment, isn't it?
            return 2;
        }

        public void setID(String sIncSuffix)
        {
        }
    }
}
