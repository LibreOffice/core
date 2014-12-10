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
package com.sun.star.wizards.ui;

import java.beans.PropertyChangeEvent;

import com.sun.star.wizards.common.*;
import com.sun.star.awt.*;

import java.util.ArrayList;

public class SortingComponent
{

    private WizardDialog CurUnoDialog;
    private int MaxSortIndex = -1;
    private static String sNoSorting;
    private static String sSortCriteriaisduplicate;
    private static String[] sSortHeader = new String[4];
    private static String[] sSortAscend = new String[4];
    private static String[] sSortDescend = new String[4];
    private int MAXSORTCRITERIAINDEX = 3;
    private static final int SOFIRSTSORTLST = 0;
    private static final int SOSECSORTLST = 1;
    private static final int SOTHIRDSORTLST = 2;
    private static final int SOFOURTHSORTLST = 3;
    private static final int[] SOSORTLST = new int[]
    {
        SOFIRSTSORTLST, SOSECSORTLST, SOTHIRDSORTLST, SOFOURTHSORTLST
    };
    private XListBox[] xSortListBox = new XListBox[4];

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(ItemEvent EventObject)
        {
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            int ikey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            enableNextSortListBox(ikey);
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }

    public SortingComponent(WizardDialog CurUnoDialog, int iStep, int iCompPosX, int iCompPosY, int iCompWidth, int FirstHelpIndex)
    {
        try
        {
            this.CurUnoDialog = CurUnoDialog;
            short curtabindex = UnoDialog.setInitialTabindex(iStep);
            Integer IStep = Integer.valueOf(iStep);
            Integer ICompPosX = Integer.valueOf(iCompPosX);
            Integer ICompWidth = Integer.valueOf(iCompWidth);

            Integer IListBoxPosX = Integer.valueOf(iCompPosX + 6);
            int iOptButtonWidth = 65;
            Integer IOptButtonWidth = Integer.valueOf(iOptButtonWidth);
            Integer IListBoxWidth = Integer.valueOf(iCompWidth - iOptButtonWidth - 12);
            Integer IOptButtonPosX = Integer.valueOf(IListBoxPosX.intValue() + IListBoxWidth.intValue() + 6);
            getResources();
            boolean bDoEnable;
            String HIDString;
            int iCurPosY = iCompPosY;
            for (int i = 0; i < 4; i++)
            {
                bDoEnable = (i < 2);
                CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lblSort" + Integer.toString(i + 1), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                                Boolean.valueOf(bDoEnable), 8, sSortHeader[i], 0, ICompPosX, Integer.valueOf(iCurPosY), IStep, Short.valueOf(curtabindex++), ICompWidth
                        });

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex);
                xSortListBox[i] = CurUnoDialog.insertListBox("lstSort" + Integer.toString(i + 1), SOSORTLST[i], null, new ItemListenerImpl(), new String[]
                        {
                            "Dropdown", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            true, bDoEnable, 12, HIDString, Short.valueOf(UnoDialog.getListBoxLineCount()), "lstSort" + Integer.valueOf(i + 1), IListBoxPosX, Integer.valueOf(iCurPosY + 14), IStep, Short.valueOf(curtabindex++), IListBoxWidth
                        }); //Short.valueOf((short) (17+i*4))

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex + 1);
                CurUnoDialog.insertRadioButton("optAscend" + Integer.toString(i + 1), 0, new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            bDoEnable, 10, HIDString, sSortAscend[i], IOptButtonPosX, Integer.valueOf(iCurPosY + 10), Short.valueOf((short) 1), IStep, Short.valueOf(curtabindex++), PropertyNames.ASC, IOptButtonWidth
                        }); //, Short.valueOf((short) (18+i*4))

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex + 2);
                CurUnoDialog.insertRadioButton("optDescend" + Integer.toString(i + 1), 0, new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            bDoEnable, 10, HIDString, sSortDescend[i], IOptButtonPosX, Integer.valueOf(iCurPosY + 24), Short.valueOf((short) 0), IStep, Short.valueOf(curtabindex++), "DESC", IOptButtonWidth
                        }); //, Short.valueOf((short) (19+i*4))
                iCurPosY = iCurPosY + 36;
                FirstHelpIndex += 3;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    private boolean getResources()
    {
        sSortHeader[0] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 20);
        sSortHeader[1] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 21);
        sSortHeader[2] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 51);
        sSortHeader[3] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 52);
        sSortAscend[0] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 36);
        sSortAscend[1] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 53);
        sSortAscend[2] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 54);
        sSortAscend[3] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 55);
        sSortDescend[0] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 37);
        sSortDescend[1] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 56);
        sSortDescend[2] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 57);
        sSortDescend[3] = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 58);
        sSortCriteriaisduplicate = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 74);
        sNoSorting = CurUnoDialog.m_oResource.getResText(UIConsts.RID_REPORT + 8);
        return true;
    }

    public void initialize(String[] _FieldNames, String[][] _SortFieldNames)
    {
        int FieldCount = _FieldNames.length;
        String[] ViewFieldNames = new String[FieldCount + 1];
        ViewFieldNames[0] = sNoSorting;
        System.arraycopy(_FieldNames, 0, ViewFieldNames, 1, FieldCount);
        short[] SelList = null;
        for (int i = 0; i < 4; i++)
        {
            if (i < _SortFieldNames.length)
            {
                SelList = new short[]
                        {
                            (short) (JavaTools.FieldInList(_FieldNames, _SortFieldNames[i][0]) + 1)
                        };
            }
            else
            {
                SelList = new short[]
                        {
                            (short) 0
                        };
            }
            CurUnoDialog.setControlProperty("lstSort" + Integer.toString(i + 1), PropertyNames.STRING_ITEM_LIST, ViewFieldNames);
            CurUnoDialog.setControlProperty("lstSort" + Integer.toString(i + 1), PropertyNames.SELECTED_ITEMS, SelList);
            toggleSortListBox(i, (i <= _SortFieldNames.length));
        }
    }

    private void setMaxSortIndex()
    {
        MaxSortIndex = -1;
        for (int i = 0; i <= MAXSORTCRITERIAINDEX; i++)
        {
            if (xSortListBox[i].getSelectedItemPos() > 0)
            {
                MaxSortIndex += 1;
            }
        }
    }

    /**
     * sets the controls of a Sorting criterion to readonly or not.
     * @param _index index of the Sorting criterion
     */
    private void setReadOnly(int _index, boolean _breadonly)
    {
        CurUnoDialog.setControlProperty("lstSort" + Integer.toString(_index + 1), PropertyNames.READ_ONLY, Boolean.valueOf(_breadonly));
    }

    /**
     *
     * @param _index the first Sorting criterion in which 'ReadOnly is set to 'false'
     */
    public void setReadOnlyUntil(int _index)
    {
        for (int i = 0; i <= 4; i++)
        {
            boolean breadonly = i < _index;
            setReadOnly(i, breadonly);
        }
    }

    private void enableNextSortListBox(int CurIndex)
    {
        try
        {
            setMaxSortIndex();
            boolean bDoEnable = (xSortListBox[CurIndex].getSelectedItemPos() != 0);
            if (!bDoEnable)
            {
                moveupSortItems(CurIndex, bDoEnable); //disableListBoxesfromIndex(CurIndex);
            }
            else
            {
                toggleSortListBox(CurIndex + 1, true);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public String[][] getSortFieldNames()
    {
        try
        {
            short iCurState;
            String CurFieldName;
            setMaxSortIndex();
            ArrayList<String[]> SortFieldNames = new ArrayList<String[]>();
            ArrayList<String> SortDescriptions = new ArrayList<String>();
            for (int i = 0; i <= MaxSortIndex; i++)
            {
		CurFieldName = xSortListBox[i].getSelectedItem();
		SortDescriptions.add(CurFieldName);
		iCurState = ((Short) CurUnoDialog.getControlProperty("optAscend" + Integer.toString(i + 1), PropertyNames.PROPERTY_STATE)).shortValue();
		SortFieldNames.add(new String[]{CurFieldName,iCurState == 1 ? PropertyNames.ASC :"DESC" });
            }
            // When searching for a duplicate entry we can neglect whether the entries are to be sorted ascending or descending
            // TODO for the future we should deliver a messagebox when two different sorting modes have been applied to one field
            int iduplicate = JavaTools.getDuplicateFieldIndex(SortDescriptions.toArray(new String[SortDescriptions.size()]));
            if (iduplicate != -1)
            {
                String sLocSortCriteriaisduplicate = JavaTools.replaceSubString(sSortCriteriaisduplicate, SortFieldNames.get(iduplicate)[0], "<FIELDNAME>");
                CurUnoDialog.showMessageBox("WarningBox", VclWindowPeerAttribute.OK, sLocSortCriteriaisduplicate);
                CurUnoDialog.vetoableChange(new PropertyChangeEvent(CurUnoDialog, "Steps", 1, 2));
                CurUnoDialog.setFocus("lstSort" + (iduplicate + 1));
                return new String[][]
                        {
                        };
            }
            else
            {
                return SortFieldNames.toArray(new String[SortFieldNames.size()][2]);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }



    //  The following code can be reactivated in a future version when task #100799 will be fixed
    private void moveupSortItems(int CurIndex, boolean bDoEnable)
    {
        short iNextItemPos;
        if ((!bDoEnable) && (MAXSORTCRITERIAINDEX > CurIndex))
        {
            for (int i = CurIndex; i < MAXSORTCRITERIAINDEX; i++)
            {
                iNextItemPos = xSortListBox[i + 1].getSelectedItemPos();
                if (iNextItemPos != 0)
                {
                    CurUnoDialog.setControlProperty("lstSort" + Integer.toString(i + 1), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                iNextItemPos
                            });
                    CurUnoDialog.setControlProperty("lstSort" + Integer.toString(i + 2), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                            });
                    toggleSortListBox(i, true);
                    CurUnoDialog.setControlProperty("lstSort" + Integer.toString(i + 2), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                0
                            });
                }
            }
            if (MaxSortIndex < xSortListBox.length - 2)
            {
                toggleSortListBox(MaxSortIndex + 2, false);
            }
        }
        else
        {
            toggleSortListBox(CurIndex + 1, bDoEnable);
        }
    }

    private void toggleSortListBox(int CurIndex, boolean bDoEnable)
    {
        try
        {
            if (CurIndex < xSortListBox.length)
            {
                CurUnoDialog.setControlProperty("lblSort" + Integer.toString(CurIndex + 1), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
                CurUnoDialog.setControlProperty("lstSort" + Integer.toString(CurIndex + 1), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
                CurUnoDialog.setControlProperty("optAscend" + Integer.toString(CurIndex + 1), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
                CurUnoDialog.setControlProperty("optDescend" + Integer.toString(CurIndex + 1), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bDoEnable));
                if (!bDoEnable)
                {
                    CurUnoDialog.setControlProperty("lstSort" + Integer.toString(CurIndex + 1), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                0
                            });
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }
}
