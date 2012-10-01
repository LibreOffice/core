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

import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.XListBox;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.common.JavaTools;

import java.util.*;

public class FieldSelection
{
    public XListBox xFieldsListBox;                 // Left ListBox
    public XListBox xSelectedFieldsListBox;         // right (selected) ListBox

    protected UnoDialog CurUnoDialog;
    protected String sIncSuffix;
    protected int FirstHelpIndex;
    protected boolean AppendMode = false;
    protected Integer IStep;

    protected int CompPosX;
    protected int CompPosY;
    protected int CompHeight;
    protected int CompWidth;

    private XFieldSelectionListener xFieldSelection;
    private int maxfieldcount = 10000000;
    private String[] AllFieldNames;
    private Integer ListBoxWidth;

    private Integer SelListBoxPosX;

    private boolean bisModified = false;

    private final static int SOCMDMOVESEL = 1;
    private final static int SOCMDMOVEALL = 2;
    private final static int SOCMDREMOVESEL = 3;
    private final static int SOCMDREMOVEALL = 4;
    private final static int SOCMDMOVEUP = 5;
    private final static int SOCMDMOVEDOWN = 6;
    private final static int SOFLDSLST = 7;
    private final static int SOSELFLDSLST = 8;


    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
        {
            com.sun.star.wizards.common.Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.FALSE);
            int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            switch (iKey)
            {

                case SOFLDSLST:
                    toggleListboxButtons((short) - 1, (short) - 1);
                    break;

                case SOSELFLDSLST:
                    toggleListboxButtons((short) - 1, (short) - 1);
                    break;
                default:
                    break;
            }
            com.sun.star.wizards.common.Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, PropertyNames.PROPERTY_ENABLED, Boolean.TRUE);
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener
    {

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
        {
            try
            {
                int iKey = CurUnoDialog.getControlKey(actionEvent.Source, CurUnoDialog.ControlList);
                switch (iKey)
                {
                    case SOFLDSLST:
                        selectFields(false);
                        break;

                    case SOSELFLDSLST:
                        deselectFields(false);
                        break;

                    case SOCMDMOVESEL:
                        selectFields(false);
                        break;

                    case SOCMDMOVEALL:
                        selectFields(true);
                        break;

                    case SOCMDREMOVESEL:
                        deselectFields(false);
                        break;

                    case SOCMDREMOVEALL:
                        deselectFields(true);
                        break;

                    case SOCMDMOVEUP:
                        changeSelectionOrder(-1);
                        break;

                    case SOCMDMOVEDOWN:
                        changeSelectionOrder(1);
                        break;

                }
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.err);
            }
        }
    }

    public void addFieldSelectionListener(XFieldSelectionListener xFieldSelection)
    {
        this.xFieldSelection = xFieldSelection;
        this.xFieldSelection.setID(sIncSuffix);
    }

    public void setAppendMode(boolean _AppendMode)
    {
        AppendMode = _AppendMode;
    }

    public boolean getAppendMode()
    {
        return AppendMode;
    }

    public FieldSelection(UnoDialog CurUnoDialog, int _iStep, int CompPosX, int CompPosY, int CompWidth, int CompHeight, String slblFields, String slblSelFields, int _FirstHelpIndex, boolean bshowFourButtons)
    {
        try
        {
            final String AccessTextMoveSelected = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 39);
            final String AccessTextRemoveSelected = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 40);
            final String AccessTextMoveAll = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 41);
            final String AccessTextRemoveAll = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 42);
            final String AccessMoveFieldUp = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 43);
            final String AccessMoveFieldDown = CurUnoDialog.m_oResource.getResText(UIConsts.RID_DB_COMMON + 44);

            FirstHelpIndex = _FirstHelpIndex;
            short curtabindex = UnoDialog.setInitialTabindex(_iStep);
            int ShiftButtonCount = 2;
            int a = 0;
            this.CurUnoDialog = CurUnoDialog;
            this.CompPosX = CompPosX;
            this.CompPosY = CompPosY;
            this.CompHeight = CompHeight;
            this.CompWidth = CompWidth;
            Object btnmoveall = null;
            Object btnremoveall = null;

            final int cmdButtonWidth = 16;
            final int cmdButtonHoriDist = 4;
            final int lblHeight = 8;
            final int lblVertiDist = 2;

            ListBoxWidth = new Integer(((CompWidth - 3 * cmdButtonHoriDist - 2 * cmdButtonWidth) / 2));
            Integer cmdShiftButtonPosX = new Integer((CompPosX + ListBoxWidth.intValue() + cmdButtonHoriDist));
            Integer ListBoxPosY = new Integer(CompPosY + lblVertiDist + lblHeight);
            Integer ListBoxHeight = new Integer(CompHeight - 8 - 2);
            SelListBoxPosX = new Integer(cmdShiftButtonPosX.intValue() + cmdButtonWidth + cmdButtonHoriDist);

            IStep = new Integer(_iStep);
            if (bshowFourButtons)
            {
                ShiftButtonCount = 4;
            }
            Integer[] ShiftButtonPosY = getYButtonPositions(ShiftButtonCount);
            Integer[] MoveButtonPosY = getYButtonPositions(2);
            Integer cmdMoveButtonPosX = new Integer(SelListBoxPosX.intValue() + ListBoxWidth.intValue() + cmdButtonHoriDist);

            Integer CmdButtonWidth = new Integer(cmdButtonWidth);

            sIncSuffix = "_" + com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.getDlgNameAccess(), "lblFields_");

            // Label
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblFields" + sIncSuffix,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblFields, new Integer(CompPosX), new Integer(CompPosY), IStep, new Short(curtabindex), 109
                    });

            // Listbox 'Available fields'
            xFieldsListBox = CurUnoDialog.insertListBox("lstFields" + sIncSuffix, SOFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "MultiSelection", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        ListBoxHeight, HelpIds.getHelpIdString(_FirstHelpIndex), Boolean.TRUE, new Integer(CompPosX), ListBoxPosY, IStep, new Short((curtabindex++)), ListBoxWidth
                    });

            Object btnmoveselected = CurUnoDialog.insertButton("cmdMoveSelected" + sIncSuffix, SOCMDMOVESEL, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, 14, HelpIds.getHelpIdString(_FirstHelpIndex + 1), ">", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth
                    });

            if (bshowFourButtons)
            {
                btnmoveall = CurUnoDialog.insertButton("cmdMoveAll" + sIncSuffix, SOCMDMOVEALL, new ActionListenerImpl(),
                        new String[]
                        {
                            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            14, HelpIds.getHelpIdString(_FirstHelpIndex + 2), ">>", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth
                        });
            }
            Object btnremoveselected = CurUnoDialog.insertButton("cmdRemoveSelected" + sIncSuffix, SOCMDREMOVESEL, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, 14, HelpIds.getHelpIdString(_FirstHelpIndex + 3), "<", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth
                    });

            if (bshowFourButtons)
            {
                btnremoveall = CurUnoDialog.insertButton("cmdRemoveAll" + sIncSuffix, SOCMDREMOVEALL, new ActionListenerImpl(),
                        new String[]
                        {
                            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                        },
                        new Object[]
                        {
                            14, HelpIds.getHelpIdString(_FirstHelpIndex + 4), "<<", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth
                        });
            }

            FontDescriptor oFontDesc = new FontDescriptor();
            oFontDesc.Name = "StarSymbol";

            // Label 'Fields in the form'
            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSelFields" + sIncSuffix,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        8, slblSelFields, SelListBoxPosX, new Integer(CompPosY), IStep, new Short(curtabindex++), ListBoxWidth
                    });

            // ListBox 'Fields in the form'
            xSelectedFieldsListBox = CurUnoDialog.insertListBox("lstSelFields" + sIncSuffix, SOSELFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "MultiSelection", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        ListBoxHeight, HelpIds.getHelpIdString(_FirstHelpIndex + 5), Boolean.TRUE, SelListBoxPosX, ListBoxPosY, IStep, new Short(curtabindex++), ListBoxWidth
                    });

            Object btnmoveup = CurUnoDialog.insertButton("cmdMoveUp" + sIncSuffix, SOCMDMOVEUP, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, oFontDesc, 14, HelpIds.getHelpIdString(_FirstHelpIndex + 6), String.valueOf((char) 8743), cmdMoveButtonPosX, MoveButtonPosY[0], IStep, new Short(curtabindex++), CmdButtonWidth
                    });

            Object btnmovedown = CurUnoDialog.insertButton("cmdMoveDown" + sIncSuffix, SOCMDMOVEDOWN, new ActionListenerImpl(),
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.FONT_DESCRIPTOR, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, oFontDesc, 14, HelpIds.getHelpIdString(_FirstHelpIndex + 7), String.valueOf((char) 8744), cmdMoveButtonPosX, MoveButtonPosY[1], IStep, new Short(curtabindex++), CmdButtonWidth
                    });

            CurUnoDialog.getPeerConfiguration().setAccessibleName(btnmoveselected, AccessTextMoveSelected);
            CurUnoDialog.getPeerConfiguration().setAccessibleName(btnremoveselected, AccessTextRemoveSelected);
            CurUnoDialog.getPeerConfiguration().setAccessibleName(xFieldsListBox, JavaTools.replaceSubString(slblFields, PropertyNames.EMPTY_STRING, "~"));
            CurUnoDialog.getPeerConfiguration().setAccessibleName(xSelectedFieldsListBox, JavaTools.replaceSubString(slblSelFields, PropertyNames.EMPTY_STRING, "~"));
            if (btnmoveall != null)
            {
                CurUnoDialog.getPeerConfiguration().setAccessibleName(btnmoveall, AccessTextMoveAll);
            }
            if (btnremoveall != null)
            {
                CurUnoDialog.getPeerConfiguration().setAccessibleName(btnremoveall, AccessTextRemoveAll);
            }
            if (btnmoveup != null)
            {
                CurUnoDialog.getPeerConfiguration().setAccessibleName(btnmoveup, AccessMoveFieldUp);
            }
            if (btnmovedown != null)
            {
                CurUnoDialog.getPeerConfiguration().setAccessibleName(btnmovedown, AccessMoveFieldDown);
            }

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    // TODO: If Value is getting smaller than zero -> throw exception
    private Integer[] getYButtonPositions(int ButtonCount)
    {
        Integer[] YPosArray;
        if (ButtonCount > 0)
        {
            YPosArray = new Integer[ButtonCount];
            final int cmdButtonHeight = 14;
            final int cmdButtonVertiDist = 2;

            YPosArray[0] = new Integer( (CompPosY + 10 + (((CompHeight - 10) - (ButtonCount * cmdButtonHeight) - ((ButtonCount - 1) * cmdButtonVertiDist)) / 2)));
            if (ButtonCount > 1)
            {
                for (int i = 1; i < ButtonCount; i++)
                {
                    YPosArray[i] = new Integer(YPosArray[i - 1].intValue() + cmdButtonHeight + cmdButtonVertiDist);
                }
            }
            return YPosArray;
        }
        return null;
    }

    public Integer getListboxWidth()
    {
        return this.ListBoxWidth;
    }

    private void changeSelectionOrder(int iNeighbor)
    {
        short[] iSelIndices = xSelectedFieldsListBox.getSelectedItemsPos();
        // TODO: we are assuming that the array starts with the lowest index. Verfy this assumption!!!!!
        if (iSelIndices.length == 1)
        {
            short iSelIndex = iSelIndices[0];
            String[] NewItemList = xSelectedFieldsListBox.getItems();
            String CurItem = NewItemList[iSelIndex];
            String NeighborItem = NewItemList[iSelIndex + iNeighbor];
            NewItemList[iSelIndex + iNeighbor] = CurItem;
            NewItemList[iSelIndex] = NeighborItem;
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, NewItemList);
            xSelectedFieldsListBox.selectItem(CurItem, true);
            if (xFieldSelection != null)
            {
                if (iNeighbor < 0)
                {
                    xFieldSelection.moveItemUp(CurItem);
                }
                else
                {
                    xFieldSelection.moveItemDown(CurItem);
                }
            }
        }
    }

    public void toggleListboxControls(Boolean BDoEnable)
    {
        try
        {
            CurUnoDialog.setControlProperty("lblFields" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);
            CurUnoDialog.setControlProperty("lblSelFields" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);
            CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);

            if (BDoEnable.booleanValue())
            {
                toggleListboxButtons((short) - 1, (short) - 1);
            }
            else
            {
                CurUnoDialog.setControlProperty("cmdRemoveAll" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);
                CurUnoDialog.setControlProperty("cmdRemoveSelected" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, BDoEnable);
                toggleMoveButtons(BDoEnable.booleanValue(), BDoEnable.booleanValue());
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    // Enable or disable the buttons used for moving the available
    // fields between the two list boxes.
    protected void toggleListboxButtons(short iFieldsSelIndex, short iSelFieldsSelIndex)
    {
        try
        {
            boolean bmoveUpenabled = false;
            boolean bmoveDownenabled = false;
            CurUnoDialog.selectListBoxItem(xFieldsListBox, iFieldsSelIndex);
            CurUnoDialog.selectListBoxItem(xSelectedFieldsListBox, iSelFieldsSelIndex);
            int SelListBoxSelLength = xSelectedFieldsListBox.getSelectedItems().length;
            int ListBoxSelLength = xFieldsListBox.getSelectedItems().length;
            boolean bIsFieldSelected = (ListBoxSelLength > 0);
            int FieldCount = xFieldsListBox.getItemCount();
            boolean bSelectSelected = (SelListBoxSelLength > 0);
            int SelectCount = xSelectedFieldsListBox.getItemCount();
            if (bSelectSelected)
            {
                short[] iSelIndices = xSelectedFieldsListBox.getSelectedItemsPos();
                bmoveUpenabled = ((iSelIndices[0] > 0) && (iSelIndices.length == 1));
                bmoveDownenabled = (((iSelIndices[SelListBoxSelLength - 1]) < (short) (SelectCount - 1)) && (iSelIndices.length == 1));
            }
            CurUnoDialog.setControlProperty("cmdRemoveAll" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(SelectCount >= 1));
            CurUnoDialog.setControlProperty("cmdRemoveSelected" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bSelectSelected));
            toggleMoveButtons((FieldCount >= 1), bIsFieldSelected);
            CurUnoDialog.setControlProperty("cmdMoveUp" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bmoveUpenabled));
            CurUnoDialog.setControlProperty("cmdMoveDown" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(bmoveDownenabled));
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    protected void toggleMoveButtons(boolean _btoggleMoveAll, boolean _btoggleMoveSelected)
    {
        boolean btoggleMoveAll = (((xFieldsListBox.getItemCount() + xSelectedFieldsListBox.getItemCount()) < maxfieldcount) && (_btoggleMoveAll));
        boolean btoggleMoveSelected = (((xFieldsListBox.getSelectedItems().length + xSelectedFieldsListBox.getItemCount()) < maxfieldcount) && (_btoggleMoveSelected));
        CurUnoDialog.setControlProperty("cmdMoveAll" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(btoggleMoveAll));
        CurUnoDialog.setControlProperty("cmdMoveSelected" + sIncSuffix, PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(btoggleMoveSelected));
    }

    public void setMultipleMode(boolean _bisMultiple)
    {
        xFieldsListBox.setMultipleMode(_bisMultiple);
        xSelectedFieldsListBox.setMultipleMode(_bisMultiple);
    }

    public void emptyFieldsListBoxes()
    {
        try
        {
            toggleListboxControls(Boolean.FALSE);
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, new String[]
                    {
                    });
            CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, new String[]
                    {
                    });
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public void mergeList(String[] AllFieldNames, String[] SecondList)
    {
        xFieldsListBox.addItems(AllFieldNames, (short) 0);
        toggleListboxButtons((short) - 1, (short) - 1);
    }

    public void intializeSelectedFields(String[] _SelectedFieldNames)
    {
        xSelectedFieldsListBox.addItems(_SelectedFieldNames, xSelectedFieldsListBox.getItemCount());
    }

    private void removeAllItems(XListBox _xListBox)
    {
        _xListBox.removeItems((short) 0, _xListBox.getItemCount());
    }

    // Note Boolean Parameter
    public void initialize(String[] _AllFieldNames, boolean _AppendMode)
    {
        AppendMode = _AppendMode;
        removeAllItems(xFieldsListBox);
        xFieldsListBox.addItems(_AllFieldNames, (short) 0);
        AllFieldNames = xFieldsListBox.getItems();
        if ((xSelectedFieldsListBox.getItemCount() > 0) && (!AppendMode))
        {
            removeAllItems(xSelectedFieldsListBox);
        }
        toggleListboxControls(Boolean.TRUE);
    }

    public void initialize(String[][] _AllFieldNamesTable, boolean _AppendMode, int _maxfieldcount)
    {
        String[] AllFieldNames_ = new String[_AllFieldNamesTable.length];
        for (int i = 0; i < _AllFieldNamesTable.length; i++)
        {
            AllFieldNames_[i] = _AllFieldNamesTable[i][0];
        }
        initialize(AllFieldNames_, _AppendMode, _maxfieldcount);
    }

    public void initialize(String[] _AllFieldNames, boolean _AppendMode, int _maxfieldcount)
    {
        maxfieldcount = _maxfieldcount;
        initialize(_AllFieldNames, _AppendMode);
    }

    public void initialize(String[] _AllFieldNames, String[] _SelFieldNames, boolean _AppendMode)
    {
        removeAllItems(xSelectedFieldsListBox);
        xSelectedFieldsListBox.addItems(_SelFieldNames, (short) 0);
        initialize(_AllFieldNames, _AppendMode);
    }

    public void selectFields(boolean bMoveAll)
    {
        short iFieldSelected = (short) - 1;
        short iSelFieldSelected = (short) - 1;
        String[] SelFieldItems;
        if (bMoveAll)
        {
            SelFieldItems = xFieldsListBox.getItems();
            removeAllItems(xFieldsListBox);
            if (!AppendMode)
            {
                removeAllItems(xSelectedFieldsListBox);
                xSelectedFieldsListBox.addItems(AllFieldNames, (short) 0);
            }
            else
            {
                xSelectedFieldsListBox.addItems(SelFieldItems, xSelectedFieldsListBox.getItemCount());
            }
        }
        else
        {
            SelFieldItems = xFieldsListBox.getSelectedItems();
            int MaxSourceSelected = SelFieldItems.length;
            if (MaxSourceSelected > 0)
            {
                iFieldSelected = xFieldsListBox.getSelectedItemPos();
                iSelFieldSelected = xSelectedFieldsListBox.getSelectedItemPos();
                short[] SourceSelList = new short[xFieldsListBox.getSelectedItemsPos().length];
                SourceSelList = xFieldsListBox.getSelectedItemsPos();
                xSelectedFieldsListBox.addItems(SelFieldItems, xSelectedFieldsListBox.getItemCount());
                CurUnoDialog.removeSelectedItems(xFieldsListBox);
                xSelectedFieldsListBox.selectItemPos((short) 0, xSelectedFieldsListBox.getSelectedItems().length > 0);
            }
        }
        toggleListboxButtons(iFieldSelected, iSelFieldSelected);
        if (xFieldSelection != null)
        {
            xFieldSelection.shiftFromLeftToRight(SelFieldItems, xSelectedFieldsListBox.getItems());
        }
    }

    public void deselectFields(boolean bMoveAll)
    {
        String SearchString;
        short iOldFieldSelected = xFieldsListBox.getSelectedItemPos();
        short iOldSelFieldSelected = xSelectedFieldsListBox.getSelectedItemPos();
        String[] OldSelFieldItems = xSelectedFieldsListBox.getSelectedItems();
        if (bMoveAll)
        {
            OldSelFieldItems = xSelectedFieldsListBox.getItems();
            removeAllItems(xFieldsListBox);
            xFieldsListBox.addItems(AllFieldNames, (short) 0);
            removeAllItems(xSelectedFieldsListBox);
        }
        else
        {
            int MaxOriginalCount = AllFieldNames.length;
            String[] SelList = xFieldsListBox.getItems();
            Vector<String> NewSourceVector = new Vector<String>();
            for (int i = 0; i < MaxOriginalCount; i++)
            {
                SearchString = AllFieldNames[i];
                if (JavaTools.FieldInList(SelList, SearchString) != -1)
                {
                    NewSourceVector.addElement(SearchString);
                }
                else if (JavaTools.FieldInList(OldSelFieldItems, SearchString) != -1)
                {
                    NewSourceVector.addElement(SearchString);
                }
            }
            xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
            if (NewSourceVector.size() > 0)
            {
                String[] NewSourceList = new String[NewSourceVector.size()];
                NewSourceVector.toArray(NewSourceList);
                xFieldsListBox.addItems(NewSourceList, (short) 0);
            }
            CurUnoDialog.removeSelectedItems(xSelectedFieldsListBox);
        }
        toggleListboxButtons(iOldFieldSelected, iOldSelFieldSelected);
        String[] NewSelFieldItems = xSelectedFieldsListBox.getItems();
        if (xFieldSelection != null)
        {
            xFieldSelection.shiftFromRightToLeft(OldSelFieldItems, NewSelFieldItems);
        }
    }

    public void addItemsToFieldsListbox(String[] _sItems)
    {
        String[] sOldList = xFieldsListBox.getItems();
        for (int i = 0; i < _sItems.length; i++)
        {
            if (JavaTools.FieldInList(sOldList, _sItems[i]) < 0)
            {
                xFieldsListBox.addItem(_sItems[i], xFieldsListBox.getItemCount());
            }
        }
    }

    public String[] getSelectedFieldNames()
    {
        return (String[]) CurUnoDialog.getControlProperty("lstSelFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST);
    }

    public void setSelectedFieldNames(String[] _sfieldnames)
    {
        CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, _sfieldnames);
        String[] sleftboxfieldnames = JavaTools.removefromList(xFieldsListBox.getItems(), _sfieldnames);
        CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, sleftboxfieldnames);
    }

    public void setModified(boolean _bModified)
    {
        bisModified = _bModified;
    }

    public boolean isModified()
    {
        return bisModified;
    }

    public void changeSelectedFieldNames(String[] _sfieldnames)
    {
        CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, PropertyNames.STRING_ITEM_LIST, _sfieldnames);
    }
}
