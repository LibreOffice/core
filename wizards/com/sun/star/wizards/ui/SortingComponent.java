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
package com.sun.star.wizards.ui;

import java.beans.PropertyChangeEvent;

import com.sun.star.wizards.common.*;
import com.sun.star.awt.*;
import java.util.ArrayList;

public class SortingComponent
{

    com.sun.star.lang.XMultiServiceFactory xMSF;
    WizardDialog CurUnoDialog;
    int MaxSortIndex = -1;
    public String[][] FieldNames;
    static String sNoSorting;
    static String sSortCriteriaisduplicate;
    static String[] sSortHeader = new String[4];
    static String[] sSortAscend = new String[4];
    static String[] sSortDescend = new String[4];
    static short[] bOldSortValues = new short[4];
    public int MAXSORTCRITERIAINDEX = 3;
    final int SOFIRSTSORTLST = 0;
    final int SOSECSORTLST = 1;
    final int SOTHIRDSORTLST = 2;
    final int SOFOURTHSORTLST = 3;
    final int[] SOSORTLST = new int[]
    {
        SOFIRSTSORTLST, SOSECSORTLST, SOTHIRDSORTLST, SOFOURTHSORTLST
    };
    XListBox[] xSortListBox = new XListBox[4];

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(ItemEvent EventObject)
        {
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, new Boolean(false));
            int ikey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            enableNextSortListBox(ikey);
            Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, new Boolean(true));
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
            xMSF = CurUnoDialog.xMSF;
            Integer IStep = new Integer(iStep);
            Integer ICompPosX = new Integer(iCompPosX);
            Integer ICompPosY = new Integer(iCompPosY);
            Integer ICompWidth = new Integer(iCompWidth);

            Integer IListBoxPosX = new Integer(iCompPosX + 6);
            int iOptButtonWidth = 65;
            Integer IOptButtonWidth = new Integer(iOptButtonWidth);
            Integer IListBoxWidth = new Integer(iCompWidth - iOptButtonWidth - 12);
            Integer IOptButtonPosX = new Integer(IListBoxPosX.intValue() + IListBoxWidth.intValue() + 6);
            getResources();
            com.sun.star.awt.XWindow[] xListBoxWindow = new com.sun.star.awt.XWindow[4];
            boolean bDoEnable;
            String HIDString;
            int iCurPosY = iCompPosY;
            for (int i = 0; i < 4; i++)
            {
                bDoEnable = (i < 2);
                CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedLineModel", "lblSort" + new Integer(i + 1).toString(), new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.ORIENTATION, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            new Boolean(bDoEnable), 8, sSortHeader[i], 0, ICompPosX, new Integer(iCurPosY), IStep, new Short(curtabindex++), ICompWidth
                        });

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex);
                xSortListBox[i] = CurUnoDialog.insertListBox("lstSort" + new Integer(i + 1).toString(), SOSORTLST[i], null, new ItemListenerImpl(), new String[]
                        {
                            "Dropdown", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            true, bDoEnable, 12, HIDString, new Short(UnoDialog.getListBoxLineCount()), "lstSort" + new Integer(i + 1), IListBoxPosX, new Integer(iCurPosY + 14), IStep, new Short(curtabindex++), IListBoxWidth
                        }); //new Short((short) (17+i*4))

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex + 1);
                XRadioButton xRadioButtonAsc = CurUnoDialog.insertRadioButton("optAscend" + Integer.toString(i + 1), 0, new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            bDoEnable, 10, HIDString, sSortAscend[i], IOptButtonPosX, new Integer(iCurPosY + 10), new Short((short) 1), IStep, new Short(curtabindex++), PropertyNames.ASC, IOptButtonWidth
                        }); //, new Short((short) (18+i*4))

                HIDString = HelpIds.getHelpIdString(FirstHelpIndex + 2);
                XRadioButton xRadioButtonDesc = CurUnoDialog.insertRadioButton("optDescend" + Integer.toString(i + 1), 0, new String[]
                        {
                            PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STATE, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, "Tag", PropertyNames.PROPERTY_WIDTH
                        }, new Object[]
                        {
                            bDoEnable, 10, HIDString, sSortDescend[i], IOptButtonPosX, new Integer(iCurPosY + 24), new Short((short) 0), IStep, new Short(curtabindex++), "DESC", IOptButtonWidth
                        }); //, new Short((short) (19+i*4))
                iCurPosY = iCurPosY + 36;
                FirstHelpIndex += 3;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
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
        for (int i = 0; i < FieldCount; i++)
        {
            ViewFieldNames[i + 1] = _FieldNames[i];
        }
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
            CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 1).toString(), PropertyNames.STRING_ITEM_LIST, ViewFieldNames);
            CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 1).toString(), PropertyNames.SELECTED_ITEMS, SelList);
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
     * @param _breadonly
     */
    public void setReadOnly(int _index, boolean _breadonly)
    {
        CurUnoDialog.setControlProperty("lstSort" + new Integer(_index + 1).toString(), PropertyNames.READ_ONLY, new Boolean(_breadonly));
    }

    /**
     *
     * @param _index the first Sorting criterion in which 'ReadOnly is set to 'false'
     * @param _bcomplete
     */
    public void setReadOnlyUntil(int _index, boolean _bcomplete)
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
            exception.printStackTrace(System.out);
        }
    }

    public String[][] getSortFieldNames()
    {
        try
        {
            short iCurState;
            String CurFieldName;
            setMaxSortIndex();
            // String[][] SortFieldNames = new String[MaxSortIndex + 1][2];
            ArrayList<String[]> SortFieldNames = new ArrayList<String[]>();
            ArrayList<String> SortDescriptions = new ArrayList<String>();
            for (int i = 0; i <= MaxSortIndex; i++)
            {
                if (!((Boolean) CurUnoDialog.getControlProperty("lstSort" + (i + 1), PropertyNames.READ_ONLY)))
                {
                    CurFieldName = xSortListBox[i].getSelectedItem();
                    SortDescriptions.add(CurFieldName);
                    iCurState = ((Short) CurUnoDialog.getControlProperty("optAscend" + new Integer(i + 1).toString(), PropertyNames.PROPERTY_STATE)).shortValue();
                    SortFieldNames.add(new String[]{CurFieldName,iCurState == 1 ? PropertyNames.ASC :"DESC" });
                }
            }
            // When searching for a duplicate entry we can neglect wether the entries are to be sorted ascending or descending
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
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public void disableListBoxesfromIndex(int CurIndex)
    {
        if (CurIndex < MAXSORTCRITERIAINDEX)
        {
            for (int i = CurIndex + 1; i <= MAXSORTCRITERIAINDEX; i++)
            {
                toggleSortListBox(i, (false));
                if (i < MaxSortIndex)
                {
                    CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 2).toString(), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                0
                            });
                }
                //          xSortListBox[i+1].selectItemPos((short)0, true);
            }
            CurUnoDialog.setFocus("lblSort" + new Integer(CurIndex + 1));
            MaxSortIndex = CurIndex - 1;
        }
    }

    //  The following code can be reactivated in a future version when task #100799 will be fixed
    private void moveupSortItems(int CurIndex, boolean bDoEnable)
    {
        short iNextItemPos;
        if ((bDoEnable == false) && (MAXSORTCRITERIAINDEX > CurIndex))
        {
            for (int i = CurIndex; i < MAXSORTCRITERIAINDEX; i++)
            {
                iNextItemPos = xSortListBox[i + 1].getSelectedItemPos();
                if (iNextItemPos != 0)
                {
                    CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 1).toString(), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                iNextItemPos
                            });
                    CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 2).toString(), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                            });
                    toggleSortListBox(i, true);
                    CurUnoDialog.setControlProperty("lstSort" + new Integer(i + 2).toString(), PropertyNames.SELECTED_ITEMS, new short[]
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
                CurUnoDialog.setControlProperty("lblSort" + new Integer(CurIndex + 1).toString(), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
                CurUnoDialog.setControlProperty("lstSort" + new Integer(CurIndex + 1).toString(), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
                CurUnoDialog.setControlProperty("optAscend" + new Integer(CurIndex + 1).toString(), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
                CurUnoDialog.setControlProperty("optDescend" + new Integer(CurIndex + 1).toString(), PropertyNames.PROPERTY_ENABLED, new Boolean(bDoEnable));
                if (bDoEnable == false)
                {
                    CurUnoDialog.setControlProperty("lstSort" + new Integer(CurIndex + 1).toString(), PropertyNames.SELECTED_ITEMS, new short[]
                            {
                                0
                            });
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
}
