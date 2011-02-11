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
    private final int SOMASTERINDEX = 1;
    private final int SOSLAVEINDEX = 0;
    private int SOFIRSTLINKLST = 0;
    private int SOSECLINKLST = 1;
    private int SOTHIRDLINKLST = 2;
    private int SOFOURTHLINKLST = 3;
    private int[] SOLINKLST = null;
    private String[] sSlaveListHeader;
    private String[] sMasterListHeader; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);
    private String sSlaveHidString;
    private String sMasterHidString;
    private Integer IListBoxPosX;

    public FieldLinker(WizardDialog _CurUnoDialog, int iStep, int iCompPosX, int iCompPosY, int iCompWidth, int _firsthelpid)
    {
        super(_CurUnoDialog, iStep, iCompPosX, iCompPosY, iCompWidth, _firsthelpid);
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
                SOFIRSTLINKLST = 0;
                SOSECLINKLST = 1;
                SOTHIRDLINKLST = 2;
                SOFOURTHLINKLST = 3;
                IListBoxPosX = new Integer(iCompPosX + 6);
                sSlaveListHeader = CurUnoDialog.m_oResource.getResArray(UIConsts.RID_FORM + 20, 4); //new String[rowcount];""; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);
                sMasterListHeader = CurUnoDialog.m_oResource.getResArray(UIConsts.RID_FORM + 24, 4);// new String[rowcount];""; //CurUnoDialog.m_oResource.getResText(UIConsts.RID_FORM + 40);
                SOLINKLST = new int[]
                        {
                            SOFIRSTLINKLST, SOSECLINKLST, SOTHIRDLINKLST, SOFOURTHLINKLST
                        };
            }
            sSlaveHidString = HelpIds.getHelpIdString(FirstHelpIndex + (i * 2));
            sMasterHidString = HelpIds.getHelpIdString(FirstHelpIndex + (i * 2) + 1);
            boolean bDoEnable = (i < 2);
            lblSlaveFields[i] = CurUnoDialog.insertLabel("lblSlaveFieldLink" + new Integer(i + 1).toString(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Boolean(bDoEnable), new Integer(8), sSlaveListHeader[i], new Integer(97), new Integer(iCurPosY), IStep, new Short(curtabindex++), new Integer(97)
                    });
            lstSlaveFields[i] = CurUnoDialog.insertListBox("lstSlaveFieldLink" + new Integer(i + 1).toString(), SOLINKLST[i], null, new ItemListenerImpl(),
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
                        new Boolean(bDoEnable),
                        UIConsts.INTEGER_12,
                        sSlaveHidString,
                        Short.valueOf(UnoDialog.getListBoxLineCount()),
                        new Integer(97),
                        new Integer(iCurPosY + 10),
                        IStep,
                        new Short(curtabindex++),
                        new Integer(97)
                    });

            lblMasterFields[i] = CurUnoDialog.insertLabel("lblMasterFieldLink" + new Integer(i + 1).toString(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        new Boolean(bDoEnable), new Integer(8), sMasterListHeader[i], new Integer(206), new Integer(iCurPosY), IStep, new Short(curtabindex++), new Integer(97)
                    });

            lstMasterFields[i] = CurUnoDialog.insertListBox("lstMasterFieldLink" + new Integer(i + 1).toString(), SOLINKLST[i], null, new ItemListenerImpl(),
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
                        new Boolean(bDoEnable),
                        UIConsts.INTEGER_12,
                        sMasterHidString,
                        Short.valueOf(UnoDialog.getListBoxLineCount()),
                        new Integer(206),
                        new Integer(iCurPosY + 10),
                        IStep,
                        new Short(curtabindex++),
                        new Integer(97)
                    });
            iCurPosY = iCurPosY + 38;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    protected void enableNextControlRow(int curindex)
    {
        // setMaxSelIndex();
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
            Helper.setUnoPropertyValue(UnoDialog.getModel(lblSlaveFields[i]), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[i]), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lblMasterFields[i]), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[i]), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
            if (bDoEnable == false)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[i]), "SelectedItems", new short[] { 0 });
                Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[i]), "SelectedItems", new short[] { 0 });
            }
        }
    }

    protected void updateFromNextControlRow(int curindex)
    {
        short iNextMasterItemPos = lstMasterFields[curindex + 1].getSelectedItemPos();
        short iNextSlaveItemPos = lstSlaveFields[curindex + 1].getSelectedItemPos();

        if ((iNextMasterItemPos != 0) && (iNextSlaveItemPos != 0))
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[curindex]), "SelectedItems", new short[] {iNextMasterItemPos });
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[curindex]), "SelectedItems", new short[]  {iNextSlaveItemPos});

            Helper.setUnoPropertyValue(UnoDialog.getModel(lstMasterFields[curindex + 1]), "SelectedItems", new short[] { 0 });
            Helper.setUnoPropertyValue(UnoDialog.getModel(lstSlaveFields[curindex + 1]), "SelectedItems", new short[] { 0 });
            toggleControlRow(curindex, true);
        }
    }

    public void initialize(String[] _AllMasterFieldNames, String[] _AllSlaveFieldNames, String[][] _LinkFieldNames)
    {
        // short[] MasterSelList = null;
        // short[] SlaveSelList = null;
        String[] MasterLinkNames = JavaTools.ArrayOutOfMultiDimArray(_LinkFieldNames, SOMASTERINDEX);
        String[] SlaveLinkNames = JavaTools.ArrayOutOfMultiDimArray(_LinkFieldNames, SOSLAVEINDEX);
        String[] ViewMasterFieldNames = addNoneFieldItemToList(_AllMasterFieldNames); // add '-undefiened-'
        String[] ViewSlaveFieldNames = addNoneFieldItemToList(_AllSlaveFieldNames);
        for (int i = 0; i < super.rowcount; i++)
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
        // setMaxSelIndex();
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

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(ItemEvent EventObject)
        {
            int ikey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            enableNextControlRow(ikey);
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }
}
