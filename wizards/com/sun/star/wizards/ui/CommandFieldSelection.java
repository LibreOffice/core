/*************************************************************************
*
*  $RCSfile: CommandFieldSelection.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 13:04:06 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*/

package com.sun.star.wizards.ui;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
import com.sun.star.awt.XWindow;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.*;

public class CommandFieldSelection extends FieldSelection {
    CommandMetaData CurDBMetaData;
    XListBox xTableListBox;
    XFixedText xlblTable;
    String SFILLUPFIELDSLISTBOX = "fillUpFieldsListbox";
    String sTableListBoxName;
    String sTableLabelName;
    String sQueryPrefix;
    String sTablePrefix;
    short iSelPos = -1;
    short iOldSelPos = -1;



    boolean bgetQueries;
    boolean AppendMode;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject) {
            short[] SelItems = (short[]) CurUnoDialog.getControlProperty(sTableListBoxName, "SelectedItems");
            if (SelItems.length > 0) {
                iOldSelPos = iSelPos;
                iSelPos = SelItems[0];
                if ((iSelPos > -1) && (iSelPos != iOldSelPos)){
                    if (!AppendMode)
                        emptyFieldsListBoxes();
                    fillUpFieldsListbox();
                }
            }
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }

    public CommandFieldSelection(UnoDialog _CurUnoDialog, CommandMetaData _CurDBMetaData, int iStep, int _iHeight, String _reslblFields, String _reslblSelFields, String _reslblTables, boolean _bgetQueries, int _ifirstHID) {
        super(_CurUnoDialog, iStep, 95, 57, 210, _iHeight, _reslblFields, _reslblSelFields, (_ifirstHID + 1), true);
        insertControls(_CurDBMetaData, _bgetQueries, _reslblTables);
    }


    public CommandFieldSelection(UnoDialog _CurUnoDialog, CommandMetaData _CurDBMetaData, int _iHeight, String _reslblFields, String _reslblSelFields, String _reslblTables, boolean _bgetQueries, int _ifirstHID) {
        super(_CurUnoDialog, 1, 95, 57, 210, _iHeight, _reslblFields, _reslblSelFields, (_ifirstHID + 1), true);
        insertControls(_CurDBMetaData, _bgetQueries, _reslblTables);
    }


    private void insertControls(CommandMetaData _CurDBMetaData, boolean _bgetQueries, String _reslblTables){
        try {
            this.AppendMode = !_bgetQueries;
            this.bgetQueries = _bgetQueries;
            this.CurDBMetaData = _CurDBMetaData;
            toggleListboxControls(Boolean.FALSE);
            sTableLabelName = "lblTables_" + super.sIncSuffix;
            sTableListBoxName = "lstTables_" + super.sIncSuffix;
            sTablePrefix = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 21);
            sQueryPrefix = CurUnoDialog.oResource.getResText(UIConsts.RID_QUERY + 22);
            Integer LabelWidth = new Integer(getListboxWidth().intValue() + 6);
            xlblTable = CurUnoDialog.insertLabel(sTableLabelName, new String[] { "Enabled", "Height", "Label", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                        new Object[] { Boolean.FALSE, new Integer(8), _reslblTables, new Integer(95), new Integer(27), IStep, new Short((short) 3), LabelWidth });
            xTableListBox = CurUnoDialog.insertListBox(sTableListBoxName, 0, null, new ItemListenerImpl(),
                        new String[] { "Dropdown", "Enabled", "Height", "HelpURL", "LineCount", "PositionX", "PositionY", "Step", "TabIndex", "Width" },
                        new Object[] { Boolean.TRUE, Boolean.FALSE, new Integer(12), "HID:" + (super.FirstHelpIndex-1), new Short("7"), new Integer(95), new Integer(37), IStep, new Short((short) 4), getListboxWidth()});
            XWindow xTableListBoxWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xTableListBox);
            fillupCommandListBox(false);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }




    public void fillUpFieldsListbox() {
    try {
        String curCommandName = "";
        //As the peer of the control might not yet exist we have to query the model for the SelectedItems
        String[] sLocList = (String[]) CurUnoDialog.getControlProperty(sTableListBoxName, "StringItemList");
        if (!bgetQueries){
            curCommandName = sLocList[iSelPos];
            CurDBMetaData.setTableByName(curCommandName);
            CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE);
        }
        else {
            if (sLocList[iSelPos].startsWith(sTablePrefix)){
                CurDBMetaData.CommandType = CommandType.TABLE;
                curCommandName = JavaTools.replaceSubString(sLocList[iSelPos], "", sTablePrefix);
                CurDBMetaData.setTableByName(curCommandName);
                CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE);
            }
            else {
                CurDBMetaData.CommandType = CommandType.QUERY;
                curCommandName = JavaTools.replaceSubString(sLocList[iSelPos], "", sQueryPrefix);
                CurDBMetaData.setQueryByName(curCommandName);
                CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.QUERY);
            }
        }
        if (CurDBMetaData.AllFieldNames.length > 0) {
            initialize(CurDBMetaData.AllFieldNames, AppendMode, CurDBMetaData.getMaxColumnsInSelect());
            return;
        }
        emptyFieldsListBoxes();
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }}

    // returns the selected entry index in the commandListbox
    private short fillupCommandListBox(boolean bpreselectCommand) throws com.sun.star.wizards.common.TerminateWizardException {
        short[] iSelArray = new short[0];
        boolean bgetFields = false;
        String[] ContentList = new String[0];
        CurDBMetaData.setCommandNames();
        if (bgetQueries) {
            ContentList = new String[CurDBMetaData.TableNames.length + CurDBMetaData.QueryNames.length];
            System.arraycopy(CurDBMetaData.QueryNames, 0, ContentList, CurDBMetaData.TableNames.length, CurDBMetaData.QueryNames.length);
            ContentList = setPrefixinArray(ContentList, sQueryPrefix, CurDBMetaData.TableNames.length, CurDBMetaData.QueryNames.length);
        } else
            ContentList = new String[CurDBMetaData.TableNames.length];
        System.arraycopy(CurDBMetaData.TableNames, 0, ContentList, 0, CurDBMetaData.TableNames.length);
        if (bgetQueries)
            ContentList = setPrefixinArray(ContentList, sTablePrefix, 0, CurDBMetaData.TableNames.length);
        if (bpreselectCommand) {
            //          if (CurDBMetaData.MainCommandName != null)
            //              iSelPos = (short) CurDBMetaData.getCommandTypeByName(CurDBMetaData.MainCommandName);
            if (iSelPos > -1) {
                bgetFields = true;
                iSelArray = new short[] { iSelPos };
            }
        } else {
            emptyFieldsListBoxes();
            iSelArray = new short[] {(short) iSelPos };
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), "StringItemList", ContentList);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), "SelectedItems", iSelArray);
        toggleCommandListBox(true);
        if (bgetFields)
            fillUpFieldsListbox();
        return iSelPos;
    }

    private String[] setPrefixinArray(String[] _ContentList, String _sprefix, int _startindex, int _nlen) {
        for (int i = _startindex; i < _startindex + _nlen; i++)
            _ContentList[i] = _sprefix + _ContentList[i];
        return _ContentList;
    }


    public void toggleCommandListBox(String[] _NewItems) {
        boolean bdoenable = !(QueryMetaData.getIncludedCommandNames(_NewItems).length >= CurDBMetaData.getMaxTablesInSelect());
        toggleCommandListBox(bdoenable);
    }


    public void toggleCommandListBox(boolean _bdoenable) {
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), "Enabled", new Boolean(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlblTable), "Enabled", new Boolean(_bdoenable));
    }

}
