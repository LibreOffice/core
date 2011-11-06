/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
