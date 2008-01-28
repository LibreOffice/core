/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FieldSelection.java,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:32:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.ui;
import com.sun.star.awt.FontDescriptor;
import com.sun.star.awt.XListBox;
import com.sun.star.wizards.common.*;

import java.util.*;

/**
 *
 * @author  bc93774
 */
public class FieldSelection {
    public UnoDialog CurUnoDialog;
    public XListBox xFieldsListBox;
    public XListBox xSelFieldsListBox;
    public XFieldSelectionListener xFieldSelection;
    public int maxfieldcount = 10000000;
    public String sIncSuffix;
    protected Integer IStep;
    protected int FirstHelpIndex;
    protected short curtabindex;
    String[] AllFieldNames;
    public Integer ListBoxWidth;
    public Integer SelListBoxPosX;
    boolean bisModified = false;
    boolean AppendMode = false;
    final int SOCMDMOVESEL = 1;
    final int SOCMDMOVEALL = 2;
    final int SOCMDREMOVESEL = 3;
    final int SOCMDREMOVEALL = 4;
    final int SOCMDMOVEUP = 5;
    final int SOCMDMOVEDOWN = 6;
    final int SOFLDSLST = 7;
    final int SOSELFLDSLST = 8;

    final int cmdButtonWidth = 16;
    final int cmdButtonHoriDist = 4;
    final int cmdButtonHeight = 14;
    final int cmdButtonVertiDist = 2;
    final int lblHeight = 8;
    final int lblVertiDist = 2;
    int CompPosX;
    int CompPosY;
    int CompHeight;
    int CompWidth;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject) {
            int iPos;
            com.sun.star.wizards.common.Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", Boolean.FALSE);
            int iKey = CurUnoDialog.getControlKey(EventObject.Source, CurUnoDialog.ControlList);
            switch (iKey) {

                case SOFLDSLST :
                    toggleListboxButtons((short) - 1, (short) - 1);
                    break;

                case SOSELFLDSLST :
                    toggleListboxButtons((short) - 1, (short) - 1);
                    break;
                default :
                    break;
            }
            com.sun.star.wizards.common.Helper.setUnoPropertyValue(CurUnoDialog.xDialogModel, "Enabled", Boolean.TRUE);
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }

    class ActionListenerImpl implements com.sun.star.awt.XActionListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent) {
            try {
                int iKey = CurUnoDialog.getControlKey(actionEvent.Source, CurUnoDialog.ControlList);
                switch (iKey) {
                    case SOFLDSLST :
                        selectFields(false);
                        break;

                    case SOSELFLDSLST :
                        deselectFields(false);
                        break;

                    case SOCMDMOVESEL :
                        selectFields(false);
                        break;

                    case SOCMDMOVEALL :
                        selectFields(true);
                        break;

                    case SOCMDREMOVESEL :
                        deselectFields(false);
                        break;

                    case SOCMDREMOVEALL :
                        deselectFields(true);
                        break;

                    case SOCMDMOVEUP :
                        changeSelectionOrder(-1);
                        break;

                    case SOCMDMOVEDOWN :
                        changeSelectionOrder(1);
                        break;

                    default :
                        // System.err.println( exception);
                        break;
                }
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }
    }

    public void addFieldSelectionListener(XFieldSelectionListener xFieldSelection) {
        this.xFieldSelection = xFieldSelection;
        this.xFieldSelection.setID(sIncSuffix);
    }

    public void setAppendMode(boolean _AppendMode) {
        AppendMode = _AppendMode;
    }

    public boolean getAppendMode() {
        return AppendMode;
    }

    public FieldSelection(UnoDialog CurUnoDialog, int iStep, int CompPosX, int CompPosY, int CompWidth, int CompHeight, String slblFields, String slblSelFields, int _FirstHelpIndex, boolean bshowFourButtons) {
        try {
            String AccessTextMoveSelected = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 39);
            String AccessTextRemoveSelected = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 40);
            String AccessTextMoveAll = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 41);
            String AccessTextRemoveAll = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 42);
            String AccessMoveFieldUp = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 43);
            String AccessMoveFieldDown = CurUnoDialog.oResource.getResText(UIConsts.RID_DB_COMMON + 44);
            FirstHelpIndex = _FirstHelpIndex;
            curtabindex = UnoDialog.setInitialTabindex(iStep);
            int ShiftButtonCount = 2;
            int a = 0;
            this.CurUnoDialog = CurUnoDialog;
            this.CompPosX = CompPosX;
            this.CompPosY = CompPosY;
            this.CompHeight = CompHeight;
            this.CompWidth = CompWidth;
            Object btnmoveall = null;
            Object btnremoveall = null;

            ListBoxWidth = new Integer((int) ((CompWidth - 3 * cmdButtonHoriDist - 2 * cmdButtonWidth) / 2));
            Integer cmdShiftButtonPosX = new Integer((int) (CompPosX + ListBoxWidth.intValue() + cmdButtonHoriDist));
            Integer ListBoxPosY = new Integer(CompPosY + lblVertiDist + lblHeight);
            Integer ListBoxHeight = new Integer(CompHeight - 8 - 2);
            SelListBoxPosX = new Integer(cmdShiftButtonPosX.intValue() + cmdButtonWidth + cmdButtonHoriDist);
            IStep = new Integer(iStep);
            if (bshowFourButtons == true) {
                ShiftButtonCount = 4;
            }
            Integer[] ShiftButtonPosY = getYButtonPositions(ShiftButtonCount);
            Integer[] MoveButtonPosY = getYButtonPositions(2);
            Integer cmdMoveButtonPosX = new Integer(SelListBoxPosX.intValue() + ListBoxWidth.intValue() + cmdButtonHoriDist);

            Integer CmdButtonWidth = new Integer(cmdButtonWidth);

            sIncSuffix = "_" + com.sun.star.wizards.common.Desktop.getIncrementSuffix(CurUnoDialog.xDlgNameAccess, "lblFields_");

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblFields" + sIncSuffix,
                                            new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(8), slblFields, new Integer(CompPosX), new Integer(CompPosY), IStep, new Short(curtabindex), new Integer(109)});

            xFieldsListBox = CurUnoDialog.insertListBox("lstFields" + sIncSuffix, SOFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                                            new String[] { "Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { ListBoxHeight, "HID:" + Integer.toString(_FirstHelpIndex++), Boolean.TRUE, new Integer(CompPosX), ListBoxPosY, IStep, new Short((curtabindex++)), ListBoxWidth });

            Object btnmoveselected = CurUnoDialog.insertButton("cmdMoveSelected" + sIncSuffix, SOCMDMOVESEL, new ActionListenerImpl(),
                                            new String[] { "Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { Boolean.FALSE, new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), ">", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth });

            if (bshowFourButtons == true)
                btnmoveall = CurUnoDialog.insertButton("cmdMoveAll" + sIncSuffix, SOCMDMOVEALL, new ActionListenerImpl(),
                                            new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), ">>", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth });

            Object btnremoveselected = CurUnoDialog.insertButton("cmdRemoveSelected" + sIncSuffix, SOCMDREMOVESEL, new ActionListenerImpl(),
                                            new String[] { "Enabled", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { Boolean.FALSE, new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), "<", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth });

            if (bshowFourButtons == true)
                btnremoveall = CurUnoDialog.insertButton("cmdRemoveAll" + sIncSuffix, SOCMDREMOVEALL, new ActionListenerImpl(),
                                            new String[] { "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                            new Object[] { new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), "<<", cmdShiftButtonPosX, ShiftButtonPosY[a++], IStep, new Short(curtabindex++), CmdButtonWidth });

            FontDescriptor oFontDesc = new FontDescriptor();
            oFontDesc.Name = "StarSymbol";

            CurUnoDialog.insertControlModel("com.sun.star.awt.UnoControlFixedTextModel", "lblSelFields" + sIncSuffix,
                                        new String[] { "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { new Integer(8), slblSelFields, SelListBoxPosX, new Integer(CompPosY), IStep, new Short(curtabindex++), ListBoxWidth });

            xSelFieldsListBox = CurUnoDialog.insertListBox("lstSelFields" + sIncSuffix, SOSELFLDSLST, new ActionListenerImpl(), new ItemListenerImpl(),
                                        new String[] { "Height", "HelpURL", "MultiSelection", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                        new Object[] { ListBoxHeight, "HID:" + Integer.toString(_FirstHelpIndex++), Boolean.TRUE, SelListBoxPosX, ListBoxPosY, IStep, new Short(curtabindex++), ListBoxWidth });

            Object btnmoveup = CurUnoDialog.insertButton("cmdMoveUp" + sIncSuffix, SOCMDMOVEUP, new ActionListenerImpl(),
                                    new String[] { "Enabled", "FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                    new Object[] { Boolean.FALSE, oFontDesc, new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), String.valueOf((char) 8743), cmdMoveButtonPosX, MoveButtonPosY[0], IStep, new Short(curtabindex++), CmdButtonWidth });

            Object btnmovedown = CurUnoDialog.insertButton("cmdMoveDown" + sIncSuffix, SOCMDMOVEDOWN, new ActionListenerImpl(),
                                    new String[] { "Enabled", "FontDescriptor", "Height", "HelpURL", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                                    new Object[] { Boolean.FALSE, oFontDesc, new Integer(14), "HID:" + Integer.toString(_FirstHelpIndex++), String.valueOf((char) 8744), cmdMoveButtonPosX, MoveButtonPosY[1], IStep, new Short(curtabindex++), CmdButtonWidth });
            CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnmoveselected, AccessTextMoveSelected);
            CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnremoveselected, AccessTextMoveSelected);
            CurUnoDialog.getPeerConfiguration().setAccessiblityName(xFieldsListBox, JavaTools.replaceSubString(slblFields, "", "~"));
            CurUnoDialog.getPeerConfiguration().setAccessiblityName(xSelFieldsListBox, JavaTools.replaceSubString(slblSelFields, "", "~"));
            if (btnmoveall != null)
                CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnmoveall, AccessTextMoveAll);
            if (btnremoveall != null)
                CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnremoveall, AccessTextRemoveAll);
            if (btnmoveup != null)
                CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnmoveup, AccessMoveFieldUp);
            if (btnmovedown != null)
                CurUnoDialog.getPeerConfiguration().setAccessiblityName(btnmovedown, AccessMoveFieldDown );

        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    // Todo: If Value is getting smaller than zero -> throw exception
    private Integer[] getYButtonPositions(int ButtonCount) {
        Integer[] YPosArray;
        if (ButtonCount > 0) {
            YPosArray = new Integer[ButtonCount];
            YPosArray[0] = new Integer((int) (CompPosY + 10 + (((CompHeight - 10) - (ButtonCount * cmdButtonHeight) - ((ButtonCount - 1) * cmdButtonVertiDist)) / 2)));
            if (ButtonCount > 1) {
                for (int i = 1; i < ButtonCount; i++) {
                    YPosArray[i] = new Integer(YPosArray[i - 1].intValue() + cmdButtonHeight + cmdButtonVertiDist);
                }
            }
            return YPosArray;
        }
        return null;
    }


    public Integer getListboxWidth() {
        return this.ListBoxWidth;
    }


    private void changeSelectionOrder(int iNeighbor) {
        short[] iSelIndices = xSelFieldsListBox.getSelectedItemsPos();
        // Todo: we are assuming that the array starts with the lowest index. Verfy this assumption!!!!!
        if (iSelIndices.length == 1) {
            short iSelIndex = iSelIndices[0];
            String[] NewItemList = xSelFieldsListBox.getItems();
            String CurItem = NewItemList[iSelIndex];
            String NeighborItem = NewItemList[iSelIndex + iNeighbor];
            NewItemList[iSelIndex + iNeighbor] = CurItem;
            NewItemList[iSelIndex] = NeighborItem;
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, "StringItemList", NewItemList);
            xSelFieldsListBox.selectItem(CurItem, true);
            if (xFieldSelection != null){
                if (iNeighbor < 0)
                    xFieldSelection.moveItemUp(CurItem);
                else
                    xFieldSelection.moveItemDown(CurItem);
            }
        }
    }

    public void toggleListboxControls(Boolean BDoEnable) {
        try {
            CurUnoDialog.setControlProperty("lblFields" + sIncSuffix, "Enabled", BDoEnable);
            CurUnoDialog.setControlProperty("lblSelFields" + sIncSuffix, "Enabled", BDoEnable);
            CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, "Enabled", BDoEnable);
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, "Enabled", BDoEnable);

            if (BDoEnable.booleanValue() == true)
                toggleListboxButtons((short) - 1, (short) - 1);
            else {
                CurUnoDialog.setControlProperty("cmdRemoveAll" + sIncSuffix, "Enabled", BDoEnable);
                CurUnoDialog.setControlProperty("cmdRemoveSelected" + sIncSuffix, "Enabled", BDoEnable);
                toggleMoveButtons(BDoEnable.booleanValue(), BDoEnable.booleanValue());
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    // Enable or disable the buttons used for moving the available
    // fields between the two list boxes.
    protected void toggleListboxButtons(short iFieldsSelIndex, short iSelFieldsSelIndex) {
        try {
            boolean bmoveUpenabled = false;
            boolean bmoveDownenabled = false;
            CurUnoDialog.selectListBoxItem(xFieldsListBox, iFieldsSelIndex);
            CurUnoDialog.selectListBoxItem(xSelFieldsListBox, iSelFieldsSelIndex);
            int SelListBoxSelLength = xSelFieldsListBox.getSelectedItems().length;
            int ListBoxSelLength = xFieldsListBox.getSelectedItems().length;
            boolean bIsFieldSelected = (ListBoxSelLength > 0);
            int FieldCount = xFieldsListBox.getItemCount();
            boolean bSelectSelected = (SelListBoxSelLength > 0);
            int SelectCount = xSelFieldsListBox.getItemCount();
            if (bSelectSelected) {
                short[] iSelIndices = xSelFieldsListBox.getSelectedItemsPos();
                bmoveUpenabled = ((iSelIndices[0] > 0) && (iSelIndices.length == 1));
                bmoveDownenabled = (((iSelIndices[SelListBoxSelLength - 1]) < (short) (SelectCount - 1)) && (iSelIndices.length == 1));
            }
            CurUnoDialog.setControlProperty("cmdRemoveAll" + sIncSuffix, "Enabled", new Boolean(SelectCount >= 1));
            CurUnoDialog.setControlProperty("cmdRemoveSelected" + sIncSuffix, "Enabled", new Boolean(bSelectSelected));
            toggleMoveButtons((FieldCount >= 1), bIsFieldSelected);
            CurUnoDialog.setControlProperty("cmdMoveUp" + sIncSuffix, "Enabled", new Boolean(bmoveUpenabled));
            CurUnoDialog.setControlProperty("cmdMoveDown" + sIncSuffix, "Enabled", new Boolean(bmoveDownenabled));
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    protected void toggleMoveButtons(boolean _btoggleMoveAll, boolean _btoggleMoveSelected) {
        boolean btoggleMoveAll = (((xFieldsListBox.getItemCount() + xSelFieldsListBox.getItemCount()) < maxfieldcount) && (_btoggleMoveAll));
        boolean btoggleMoveSelected = (((xFieldsListBox.getSelectedItems().length + xSelFieldsListBox.getItemCount()) < maxfieldcount) && (_btoggleMoveSelected));
        CurUnoDialog.setControlProperty("cmdMoveAll" + sIncSuffix, "Enabled", new Boolean(btoggleMoveAll));
        CurUnoDialog.setControlProperty("cmdMoveSelected" + sIncSuffix, "Enabled", new Boolean(btoggleMoveSelected));
    }


    public void setMultipleMode(boolean _bisMultiple) {
        xFieldsListBox.setMultipleMode(_bisMultiple);
        xSelFieldsListBox.setMultipleMode(_bisMultiple);
    }

    public void emptyFieldsListBoxes() {
        try {
            toggleListboxControls(Boolean.FALSE);
            CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, "StringItemList", new String[]{});
            CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, "StringItemList", new String[]{});
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void mergeList(String[] AllFieldNames, String[] SecondList) {
        int MaxIndex = SecondList.length;
        xFieldsListBox.addItems(AllFieldNames, (short) 0);
        toggleListboxButtons((short) - 1, (short) - 1);
    }

    public void intializeSelectedFields(String[] _SelectedFieldNames) {
        xSelFieldsListBox.addItems(_SelectedFieldNames, xSelFieldsListBox.getItemCount());
    }


    // Note Boolean Parameter
    public void initialize(String[] _AllFieldNames, boolean _AppendMode) {
        AppendMode = _AppendMode;
        xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
        xFieldsListBox.addItems(_AllFieldNames, (short) 0);
        this.AllFieldNames = xFieldsListBox.getItems();
        if ((xSelFieldsListBox.getItemCount() > 0) && (!AppendMode))
            xSelFieldsListBox.removeItems((short) 0, xSelFieldsListBox.getItemCount());
        toggleListboxControls(Boolean.TRUE);
    }


    public void initialize(String[][] _AllFieldNamesTable, boolean _AppendMode, int _maxfieldcount) {
        String[] AllFieldNames = new String[_AllFieldNamesTable.length];
        for (int i = 0; i < _AllFieldNamesTable.length; i++)
            AllFieldNames[i] = _AllFieldNamesTable[i][0];
        initialize(AllFieldNames, _AppendMode, _maxfieldcount);
    }


    public void initialize(String[] _AllFieldNames, boolean _AppendMode, int _maxfieldcount) {
        maxfieldcount = _maxfieldcount;
        initialize(_AllFieldNames, _AppendMode);
    }


    public void initialize(String[] _AllFieldNames, String[] _SelFieldNames,  boolean _AppendMode){
        xSelFieldsListBox.removeItems((short) 0, xSelFieldsListBox.getItemCount());
        xSelFieldsListBox.addItems(_SelFieldNames, (short) 0);
        initialize( _AllFieldNames,_AppendMode);
    }


    public void selectFields(boolean bMoveAll) {
        int CurIndex;
        short iFieldSelected = (short) - 1;
        short iSelFieldSelected = (short) - 1;
        int MaxCurTarget = xSelFieldsListBox.getItemCount();
        String[] SelFieldItems;
        if (bMoveAll) {
            SelFieldItems = xFieldsListBox.getItems();
            xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
            if (!AppendMode){
                xSelFieldsListBox.removeItems((short) 0, xSelFieldsListBox.getItemCount());
                xSelFieldsListBox.addItems(AllFieldNames, (short) 0);
            }
            else
                xSelFieldsListBox.addItems(SelFieldItems, xSelFieldsListBox.getItemCount());
        } else {
            SelFieldItems = xFieldsListBox.getSelectedItems();
            int MaxSourceSelected = SelFieldItems.length;
            if (MaxSourceSelected > 0) {
                iFieldSelected = xFieldsListBox.getSelectedItemPos();
                iSelFieldSelected = xSelFieldsListBox.getSelectedItemPos();
                short[] SourceSelList = new short[xFieldsListBox.getSelectedItemsPos().length];
                SourceSelList = xFieldsListBox.getSelectedItemsPos();
                xSelFieldsListBox.addItems(SelFieldItems, xSelFieldsListBox.getItemCount());
                CurUnoDialog.removeSelectedItems(xFieldsListBox);
                xSelFieldsListBox.selectItemPos((short) 0, xSelFieldsListBox.getSelectedItems().length > 0);
            }
        }
        toggleListboxButtons(iFieldSelected, iSelFieldSelected);
        if (xFieldSelection != null)
            xFieldSelection.shiftFromLeftToRight(SelFieldItems, xSelFieldsListBox.getItems());
    }

    public void deselectFields(boolean bMoveAll) {
        int m = 0;
        String SearchString;
        short iOldFieldSelected = xFieldsListBox.getSelectedItemPos();
        short iOldSelFieldSelected = xSelFieldsListBox.getSelectedItemPos();
        String[] OldSelFieldItems = xSelFieldsListBox.getSelectedItems();
        if (bMoveAll) {
            OldSelFieldItems = xSelFieldsListBox.getItems();
            xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
            xFieldsListBox.addItems(AllFieldNames, (short) 0);
            xSelFieldsListBox.removeItems((short) 0, xSelFieldsListBox.getItemCount());
        } else {
            int MaxOriginalCount = AllFieldNames.length;
            int MaxSelected = OldSelFieldItems.length;
            String[] SelList = xFieldsListBox.getItems();
            Vector NewSourceVector = new Vector();
            for (int i = 0; i < MaxOriginalCount; i++) {
                SearchString = AllFieldNames[i];
                if (JavaTools.FieldInList(SelList, SearchString) != -1)
                    NewSourceVector.addElement(SearchString);
                else if (JavaTools.FieldInList(OldSelFieldItems, SearchString) != -1)
                    NewSourceVector.addElement(SearchString);
            }
            xFieldsListBox.removeItems((short) 0, xFieldsListBox.getItemCount());
            if (NewSourceVector.size() > 0) {
                String[] NewSourceList = new String[NewSourceVector.size()];
                NewSourceVector.toArray(NewSourceList);
                xFieldsListBox.addItems(NewSourceList, (short) 0);
            }
            CurUnoDialog.removeSelectedItems(xSelFieldsListBox);
        }
        toggleListboxButtons(iOldFieldSelected, iOldSelFieldSelected);
        String[] NewSelFieldItems = xSelFieldsListBox.getItems();
        if (xFieldSelection != null)
            xFieldSelection.shiftFromRightToLeft(OldSelFieldItems, NewSelFieldItems);
    }

    public void addItemsToFieldsListbox(String[] _sItems) {
        String[] sOldList = xFieldsListBox.getItems();
        for (int i = 0; i < _sItems.length; i++) {
            if (JavaTools.FieldInList(sOldList, _sItems[i]) < 0) {
                xFieldsListBox.addItem(_sItems[i], xFieldsListBox.getItemCount());
            }
        }
    }

    public String[] getSelectedFieldNames() {
        return (String[]) CurUnoDialog.getControlProperty("lstSelFields" + sIncSuffix, "StringItemList");
    }

    public void setSelectedFieldNames(String[] _sfieldnames){
        CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, "StringItemList", _sfieldnames);
        String[] sleftboxfieldnames = JavaTools.removefromList(xFieldsListBox.getItems(), _sfieldnames);
        CurUnoDialog.setControlProperty("lstFields" + sIncSuffix, "StringItemList", sleftboxfieldnames);
    }

    public void setModified(boolean _bModified){
        bisModified = _bModified;
    }

    public boolean isModified(){
        return bisModified;
    }

    public void changeSelectedFieldNames(String[] _sfieldnames){
        CurUnoDialog.setControlProperty("lstSelFields" + sIncSuffix, "StringItemList", _sfieldnames);
    }


}
