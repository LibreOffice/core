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

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XListBox;
import com.sun.star.uno.Exception;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.HelpIds;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.db.RelationController;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.WizardDialog;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.DBLimitedFieldSelection;
import com.sun.star.wizards.common.PropertyNames;

public class FieldLinker extends DBLimitedFieldSelection
{

    private XFixedText[] lblSlaveFields;
    private XFixedText[] lblMasterFields;
    private XListBox[] lstSlaveFields;
    private XListBox[] lstMasterFields;
    private int[] SOLINKLST = null;
    private String[] sSlaveListHeader;
    private String[] sMasterListHeader; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);

    public FieldLinker(WizardDialog _CurUnoDialog, int iStep, int iCompPosY, int _firsthelpid)
    {
        super(_CurUnoDialog, iStep, iCompPosY, _firsthelpid);
    }

    protected void insertControlGroup(int i)
    {
        try
        {
            if (i == 0)
            {
                lblSlaveFields = new XFixedText[rowcount];
                lblMasterFields = new XFixedText[rowcount];
                lstSlaveFields = new XListBox[rowcount];
                lstMasterFields = new XListBox[rowcount];
                int SOFIRSTLINKLST = 0;
                int SOSECLINKLST = 1;
                int SOTHIRDLINKLST = 2;
                int SOFOURTHLINKLST = 3;
                sSlaveListHeader = CurUnoDialog.m_oResource.getResArray(UIConsts.RID_FORM + 20, 4); //new String[rowcount];PropertyNames.EMPTY_STRING; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);
                sMasterListHeader = CurUnoDialog.m_oResource.getResArray(UIConsts.RID_FORM + 24, 4);// new String[rowcount];PropertyNames.EMPTY_STRING; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);
                SOLINKLST = new int[]
                        {
                                SOFIRSTLINKLST, SOSECLINKLST, SOTHIRDLINKLST, SOFOURTHLINKLST
                        };
            }
            String sSlaveHidString = HelpIds.getHelpIdString(FirstHelpIndex + (i * 2));
            String sMasterHidString = HelpIds.getHelpIdString(FirstHelpIndex + (i * 2) + 1);
            boolean bDoEnable = (i < 2);
            lblSlaveFields[i] = CurUnoDialog.insertLabel("lblSlaveFieldLink" + Integer.toString(i + 1),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                            Boolean.valueOf(bDoEnable), 8, sSlaveListHeader[i], 97, Integer.valueOf(iCurPosY), IStep, Short.valueOf(curtabindex++), 97
                    });
            lstSlaveFields[i] = CurUnoDialog.insertListBox("lstSlaveFieldLink" + (i + 1), SOLINKLST[i], null, new ItemListenerImpl(),
                    new String[]
                    {
                        "Dropdown",
                        PropertyNames.PROPERTY_ENABLED,
                        PropertyNames.PROPERTY_HEIGHT,
                        PropertyNames.PROPERTY_HELPURL,
                        "LineCount",
                        PropertyNames.PROPERTY_POSITION_X,
                        PropertyNames.PROPERTY_POSITION_Y,
                        PropertyNames.PROPERTY_STEP,
                        PropertyNames.PROPERTY_TABINDEX,
                        PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.TRUE,
                            Boolean.valueOf(bDoEnable),
                        UIConsts.INTEGER_12,
                            sSlaveHidString,
                        Short.valueOf(UnoDialog.getListBoxLineCount()),
                        97,
                        Integer.valueOf(iCurPosY + 10),
                        IStep,
                        Short.valueOf(curtabindex++),
                        97
                    });

            lblMasterFields[i] = CurUnoDialog.insertLabel("lblMasterFieldLink" + Integer.toString(i + 1),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                            Boolean.valueOf(bDoEnable), 8, sMasterListHeader[i], 206, Integer.valueOf(iCurPosY), IStep, Short.valueOf(curtabindex++), 97
                    });

            lstMasterFields[i] = CurUnoDialog.insertListBox("lstMasterFieldLink" + Integer.toString(i + 1), SOLINKLST[i], null, new ItemListenerImpl(),
                    new String[]
                    {
                        "Dropdown",
                        PropertyNames.PROPERTY_ENABLED,
                        PropertyNames.PROPERTY_HEIGHT,
                        PropertyNames.PROPERTY_HELPURL,
                        "LineCount",
                        PropertyNames.PROPERTY_POSITION_X,
                        PropertyNames.PROPERTY_POSITION_Y,
                        PropertyNames.PROPERTY_STEP,
                        PropertyNames.PROPERTY_TABINDEX,
                        PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.TRUE,
                            Boolean.valueOf(bDoEnable),
                        UIConsts.INTEGER_12,
                            sMasterHidString,
                        Short.valueOf(UnoDialog.getListBoxLineCount()),
                        206,
                        Integer.valueOf(iCurPosY + 10),
                        IStep,
                        Short.valueOf(curtabindex++),
                        97
                    });
            iCurPosY = iCurPosY + 38;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    protected void enableNextControlRow(int curindex)
    {
        boolean bSlaveField = lstSlaveFields[curindex].getSelectedItemPos() > 0;
        boolean bMasterField = lstMasterFields[curindex].getSelectedItemPos() > 0;
        boolean bDoEnable = (bSlaveField && bMasterField);
        if (!bDoEnable)
        {
            moveupSelectedItems(curindex, bDoEnable);
        }
        else
        {
            toggleControlRow(curindex + 1, true);
        }
    }

    protected int getMaxSelIndex()
    {
        int MaxSelIndex = -1;
        for (int i = 0; i < rowcount; i++)
        {
            if ((lstSlaveFields[i].getSelectedItemPos() > 0) && (lstMasterFields[i].getSelectedItemPos() > 0))
            {
                MaxSelIndex += 1;
            }
        }
        return MaxSelIndex;
    }

    protected void toggleControlRow(int i, boolean bDoEnable)
    {
        if (i < rowcount)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(lblSlaveFields[i]), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[i]), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lblMasterFields[i]), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[i]), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
            if (!bDoEnable)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[i]), PropertyNames.SELECTED_ITEMS, new short[] { 0 });
                Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[i]), PropertyNames.SELECTED_ITEMS, new short[] { 0 });
            }
        }
    }

    protected void updateFromNextControlRow(int curindex)
    {
        short iNextMasterItemPos = lstMasterFields[curindex + 1].getSelectedItemPos();
        short iNextSlaveItemPos = lstSlaveFields[curindex + 1].getSelectedItemPos();

        if ((iNextMasterItemPos != 0) && (iNextSlaveItemPos != 0))
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[curindex]), PropertyNames.SELECTED_ITEMS, new short[] {iNextMasterItemPos });
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[curindex]), PropertyNames.SELECTED_ITEMS, new short[]  {iNextSlaveItemPos});

            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[curindex + 1]), PropertyNames.SELECTED_ITEMS, new short[] { 0 });
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[curindex + 1]), PropertyNames.SELECTED_ITEMS, new short[] { 0 });
            toggleControlRow(curindex, true);
        }
    }

    public void initialize(String[] _AllMasterFieldNames, String[] _AllSlaveFieldNames, String[][] _LinkFieldNames)
    {
        int SOMASTERINDEX = 1;
        String[] MasterLinkNames = JavaTools.ArrayOutOfMultiDimArray(_LinkFieldNames, SOMASTERINDEX);
        int SOSLAVEINDEX = 0;
        String[] SlaveLinkNames = JavaTools.ArrayOutOfMultiDimArray(_LinkFieldNames, SOSLAVEINDEX);
        String[] ViewMasterFieldNames = addNoneFieldItemToList(_AllMasterFieldNames); // add '-undefined-'
        String[] ViewSlaveFieldNames = addNoneFieldItemToList(_AllSlaveFieldNames);
        for (int i = 0; i < rowcount; i++)
        {
            super.initializeListBox(lstMasterFields[i], ViewMasterFieldNames, MasterLinkNames, i);
            super.initializeListBox(lstSlaveFields[i], ViewSlaveFieldNames, SlaveLinkNames, i);
            if (_LinkFieldNames != null)
            {
                toggleControlRow(i, (i <= _LinkFieldNames.length));
            }
            else
            {
                toggleControlRow(i, i == 0);
            }
        }
    }

    public String[][] getLinkFieldNames(RelationController _oRelationController, String _sReferencedTableName)
    {
        return _oRelationController.getImportedKeyColumns(_sReferencedTableName);
    }

    /**
     * @return the LinkFieldnames of the joins. When no LinkFieldNames were selected the returned Array is empty.
     * When Joins were assigned duplicate a null value is returned
     *
     */
    public String[][] getLinkFieldNames()
    {
        int nSelectedIndex = getMaxSelIndex();
        String[][] LinkFieldNames = new String[2][nSelectedIndex + 1];
        for (int i = 0; i <= nSelectedIndex; i++)
        {
            LinkFieldNames[0][i] = lstSlaveFields[i].getSelectedItem();
            LinkFieldNames[1][i] = lstMasterFields[i].getSelectedItem();
        }
        int iduplicate = JavaTools.getDuplicateFieldIndex(LinkFieldNames);
        if (iduplicate != -1)
        {
            String sLinkFieldsAreDuplicate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 19);
            String sLocLinkFieldsAreDuplicate = JavaTools.replaceSubString(sLinkFieldsAreDuplicate, LinkFieldNames[0][iduplicate], "<FIELDNAME1>");
            sLocLinkFieldsAreDuplicate = JavaTools.replaceSubString(sLocLinkFieldsAreDuplicate, LinkFieldNames[1][iduplicate], "<FIELDNAME2>");
            CurUnoDialog.setCurrentStep(FormWizard.SOFIELDLINKER_PAGE);
            CurUnoDialog.enableNavigationButtons(true, true, true);
            CurUnoDialog.showMessageBox("WarningBox", VclWindowPeerAttribute.OK, sLocLinkFieldsAreDuplicate);
            CurUnoDialog.setFocus("lstSlaveFieldLink" + (iduplicate + 1));
            return null;
        }
            return LinkFieldNames;

    }

    public void enable(boolean _bdoenable)
    {
        CurUnoDialog.setStepEnabled(IStep.intValue(), _bdoenable);
    }

    private class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(ItemEvent EventObject)
        {
            if (EventObject != null) {
                int ikey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
                enableNextControlRow(ikey);
            }
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }
}
