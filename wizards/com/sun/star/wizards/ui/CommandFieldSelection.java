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

import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
// import com.sun.star.awt.XWindow;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.CommandType;
// import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.AnyConverter;
// import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.*;
import com.sun.star.beans.PropertyValue;
import java.text.Collator;
import java.util.Comparator;

public class CommandFieldSelection extends FieldSelection implements Comparator
{

    private CommandMetaData CurDBMetaData;
    private XListBox xTableListBox;
    private XFixedText xlblTable;
    // private String SFILLUPFIELDSLISTBOX = "fillUpFieldsListbox";
    private String sTableListBoxName;
    private String sTableLabelName;
    private String sQueryPrefix;
    private String sTablePrefix;
    private short m_iSelPos = -1;
    private short iOldSelPos = -1;
    private boolean bpreselectCommand = true;
    private boolean bgetQueries;
    // boolean AppendMode;
    private WizardDialog oWizardDialog;
    private Collator aCollator = null;

    class ItemListenerImpl implements com.sun.star.awt.XItemListener
    {

        public void itemStateChanged(com.sun.star.awt.ItemEvent EventObject)
        {
            short[] SelItems = (short[]) CurUnoDialog.getControlProperty(sTableListBoxName, PropertyNames.SELECTED_ITEMS);
            if (SelItems.length > 0)
            {
                iOldSelPos = m_iSelPos;
                m_iSelPos = SelItems[0];
                if ((m_iSelPos > -1) && (m_iSelPos != iOldSelPos))
                {
                    if (!AppendMode)
                    {
                        oWizardDialog.enablefromStep(IStep.intValue() + 1, false);
                    }
                    fillUpFieldsListbox();
                }
            }
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }
    }

    /**
     * instantiates a CommandFieldSelection with a preselected command
     * @param _CurUnoDialog
     * @param _CurDBMetaData
     * @param iStep
     * @param _iHeight
     * @param _reslblFields
     * @param _reslblSelFields
     * @param _reslblTables
     * @param _bgetQueries
     * @param _ifirstHID
     */
    public CommandFieldSelection(WizardDialog _CurUnoDialog, CommandMetaData _CurDBMetaData, int iStep, int _iHeight, String _reslblFields, String _reslblSelFields, String _reslblTables, boolean _bgetQueries, int _ifirstHID)
    {
        super(_CurUnoDialog, iStep, 95, 57, 210, _iHeight, _reslblFields, _reslblSelFields, (_ifirstHID + 1), true);
        insertControls(_CurDBMetaData, _bgetQueries, _reslblTables);
        oWizardDialog = (WizardDialog) CurUnoDialog;
    }

    /**
     * instantiates a CommandFieldSelection with a preselected command
     * @param _CurUnoDialog
     * @param _CurDBMetaData
     * @param _iHeight
     * @param _reslblFields
     * @param _reslblSelFields
     * @param _reslblTables
     * @param _bgetQueries
     * @param _ifirstHID
     */
    public CommandFieldSelection(UnoDialog _CurUnoDialog, CommandMetaData _CurDBMetaData, int _iHeight, String _reslblFields, String _reslblSelFields, String _reslblTables, boolean _bgetQueries, int _ifirstHID)
    {
        super(_CurUnoDialog, 1, 95, 57, 210, _iHeight, _reslblFields, _reslblSelFields, (_ifirstHID + 1), true);
        insertControls(_CurDBMetaData, _bgetQueries, _reslblTables);
        oWizardDialog = (WizardDialog) CurUnoDialog;
    }

    private void insertControls(CommandMetaData _CurDBMetaData, boolean _bgetQueries, String _reslblTables)
    {
        try
        {
            this.AppendMode = !_bgetQueries;
            this.bgetQueries = _bgetQueries;
            this.CurDBMetaData = _CurDBMetaData;
            toggleListboxControls(Boolean.FALSE);
            sTableLabelName = "lblTables_" + super.sIncSuffix;
            sTableListBoxName = "lstTables_" + super.sIncSuffix;
            sTablePrefix = getTablePrefix();
            sQueryPrefix = getQueryPrefix();
            Integer LabelWidth = new Integer(getListboxWidth().intValue() + 6);
            // Label 'Tables or Queries'
            xlblTable = CurUnoDialog.insertLabel(sTableLabelName,
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, 8, _reslblTables, 95, 27, IStep, new Short((short) 3), LabelWidth
                    });
            // DropDown Listbox TableNames
            xTableListBox = CurUnoDialog.insertListBox(sTableListBoxName, 0, null, new ItemListenerImpl(),
                    new String[]
                    {
                        "Dropdown", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.TRUE, Boolean.FALSE, 12, HelpIds.getHelpIdString(super.FirstHelpIndex - 1), new Short(UnoDialog.getListBoxLineCount()), 95, 37, IStep, new Short((short) 4), getListboxWidth()
                    });
            // XWindow xTableListBoxWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xTableListBox);
            fillupCommandListBox();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    /**
     * @return Returns the sQueryPrefix.
     */
    public String getQueryPrefix()
    {
        if (sQueryPrefix == null)
        {
            sQueryPrefix = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 22);
        }
        return sQueryPrefix;
    }

    private String getCommandPrefix(int _nCommandType)
    {
        if (_nCommandType == CommandType.TABLE)
        {
            return getTablePrefix();
        }
        else if (_nCommandType == CommandType.QUERY)
        {
            return getQueryPrefix();
        }
        else
        {
            return PropertyNames.EMPTY_STRING;
        }
    }

    /**
     * @return Returns the sTablePrefix.
     */
    public String getTablePrefix()
    {
        if (sTablePrefix == null)
        {
            sTablePrefix = CurUnoDialog.m_oResource.getResText(UIConsts.RID_QUERY + 21);
        }
        return sTablePrefix;
    }

    private short getselectedItemPos()
    {
        short[] iSelPoses = ((short[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS));
        if (iSelPoses.length > 0)
        {
            return iSelPoses[0];
        }
        else
        {
            if (this.bpreselectCommand)
            {
                String[] sItemList = ((String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.STRING_ITEM_LIST));
                if (sItemList.length > 0)
                {
                    return (short) 0;
                }
            }
            return (short) -1;
        }
    }

    public void fillUpFieldsListbox()
    {
        try
        {
            boolean binitialize = false;
            String curCommandName = PropertyNames.EMPTY_STRING;
            //As the peer of the control might not yet exist we have to query the model for the SelectedItems
            short iSelPos = getselectedItemPos();
            // String[] sLocList = (String[]) CurUnoDialog.getControlProperty(sTableListBoxName, PropertyNames.STRING_ITEM_LIST);
            final String sSelectedTableName = xTableListBox.getItem(iSelPos);
            if (!bgetQueries)
            {
                curCommandName = sSelectedTableName; // sLocList[iSelPos];
                CurDBMetaData.setTableByName(curCommandName);
                binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE, AppendMode);
            }
            else
            {
                if (sSelectedTableName.startsWith(sTablePrefix))
                {
                    CurDBMetaData.setCommandType(CommandType.TABLE);
                    curCommandName = JavaTools.replaceSubString(sSelectedTableName, PropertyNames.EMPTY_STRING, sTablePrefix);
                    CurDBMetaData.setTableByName(curCommandName);
                    binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE, AppendMode);
                }
                else
                {
                    CurDBMetaData.setCommandType(CommandType.QUERY);
                    curCommandName = JavaTools.replaceSubString(sSelectedTableName, PropertyNames.EMPTY_STRING, sQueryPrefix);
                    CurDBMetaData.setQueryByName(curCommandName);
                    binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.QUERY, AppendMode);
                }
            }
            if (binitialize)
            {
                CurDBMetaData.setCommandName(curCommandName);
                if (CurDBMetaData.m_aAllFieldNames != null)
                {
                    if (CurDBMetaData.m_aAllFieldNames.length > 0)
                    {
                        initialize(CurDBMetaData.m_aAllFieldNames, AppendMode, CurDBMetaData.getMaxColumnsInSelect());
                        return;
                    }
                }
            }
            emptyFieldsListBoxes();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    /** returns the selected entry index in the commandListbox
     *
     * @return
     * @throws com.sun.star.wizards.common.TerminateWizardException
     */
    private short fillupCommandListBox() throws com.sun.star.wizards.common.TerminateWizardException
    {
        short[] iSelArray = new short[0];
        boolean bgetFields = false;
        String[] ContentList = new String[0];
        // CurDBMetaData.initCommandNames();
        if (bgetQueries)
        {
            ContentList = new String[CurDBMetaData.getTableNames().length + CurDBMetaData.getQueryNames().length];
            System.arraycopy(CurDBMetaData.getQueryNames(), 0, ContentList, CurDBMetaData.getTableNames().length, CurDBMetaData.getQueryNames().length);
            ContentList = setPrefixinArray(ContentList, sQueryPrefix, CurDBMetaData.getTableNames().length, CurDBMetaData.getQueryNames().length);
        }
        else
        {
            ContentList = new String[CurDBMetaData.getTableNames().length];
        }
        System.arraycopy(CurDBMetaData.getTableNames(), 0, ContentList, 0, CurDBMetaData.getTableNames().length);
        if (bgetQueries)
        {
            ContentList = setPrefixinArray(ContentList, sTablePrefix, 0, CurDBMetaData.getTableNames().length);
        }
        java.util.Arrays.sort(ContentList, this);
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.STRING_ITEM_LIST, ContentList);
        short iSelPos = getselectedItemPos();
        if (bpreselectCommand)
        {
            if (iSelPos > -1)
            {
                bgetFields = true;
                iSelArray = new short[]
                        {
                            iSelPos
                        };
            }
        }
        else
        {
            emptyFieldsListBoxes();
            iSelArray = new short[] {iSelPos};
        }
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, iSelArray);
        toggleCommandListBox(true);
        if (bgetFields)
        {
            fillUpFieldsListbox();
        }
        return iSelPos;
    }

    private Collator getCollator()
    {
        if (this.aCollator == null)
        {
            com.sun.star.lang.Locale aOfficeLocale = Configuration.getOfficeLocale(this.CurDBMetaData.xMSF);
            java.util.Locale aJavaLocale = new java.util.Locale(aOfficeLocale.Language, aOfficeLocale.Country, aOfficeLocale.Variant);
            //Get the Collator for US English and set its strength to PRIMARY
            this.aCollator = Collator.getInstance(aJavaLocale);
            aCollator.setStrength(Collator.TERTIARY);
        }
        return aCollator;
    }

    public int compare(Object _oObject1, Object _oObject2)
    {
        return this.getCollator().compare(_oObject1, _oObject2);
    }

    private String[] setPrefixinArray(String[] _ContentList, String _sprefix, int _startindex, int _nlen)
    {
        for (int i = _startindex; i < _startindex + _nlen; i++)
        {
            _ContentList[i] = _sprefix + _ContentList[i];
        }
        return _ContentList;
    }

    public void toggleCommandListBox(String[] _NewItems)
    {
        boolean bdoenable = !(QueryMetaData.getIncludedCommandNames(_NewItems).length >= CurDBMetaData.getMaxTablesInSelect());
        toggleCommandListBox(bdoenable);
    }

    public void toggleCommandListBox(boolean _bdoenable)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
        Helper.setUnoPropertyValue(UnoDialog.getModel(xlblTable), PropertyNames.PROPERTY_ENABLED, Boolean.valueOf(_bdoenable));
    }

    public String getSelectedCommandName()
    {
        String sCommandname = xTableListBox.getSelectedItem();
        if (sCommandname.startsWith(this.sTablePrefix))
        {
            return sCommandname.substring(sTablePrefix.length());
        }
        else if ((sCommandname.startsWith(this.sQueryPrefix)))
        {
            return sCommandname.substring(sQueryPrefix.length());
        }
        else
        {
            return sCommandname;
        }
    }

    public int getSelectedCommandType()
    {
        String sCommandname = xTableListBox.getSelectedItem();
        if (sCommandname.startsWith(this.sTablePrefix))
        {
            return CommandType.TABLE;
        }
        else
        {
            return CommandType.QUERY;
        }
    }

    public void preselectCommand(PropertyValue[] _aPropertyValue, boolean _bReadOnly)
    {
        try
        {
            if (Properties.hasPropertyValue(_aPropertyValue, PropertyNames.COMMAND_TYPE))
            {
                int nCommandType = AnyConverter.toInt(Properties.getPropertyValue(_aPropertyValue, PropertyNames.COMMAND_TYPE));
                String sCommand = AnyConverter.toString(Properties.getPropertyValue(_aPropertyValue, PropertyNames.COMMAND));
                if (sCommand != null)
                {
                    preselectCommand(sCommand, nCommandType, _bReadOnly);
                }
            }
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace();
        }
    }

    public void preselectCommand(String _selitem, int _nCommandType, boolean _bReadOnly)
    {
        if (_selitem.length() > 0)
        {
            String[] sitems = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.STRING_ITEM_LIST);
            String sPrefix = getCommandPrefix(_nCommandType);
            short iselpos = (short) JavaTools.FieldInList(sitems, sPrefix + _selitem);
            if (iselpos > -1)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, new short[]
                        {
                            iselpos
                        });
            }
            this.fillUpFieldsListbox();
        }
        else
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, new short[]
                    {
                    });
        }
        if (_bReadOnly)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.READ_ONLY, Boolean.valueOf(_selitem.length() > 0));
        }
    }

    public void preselectCommand(String _selitem, boolean _bReadOnly)
    {
        if (_selitem.length() > 0)
        {
            String[] sitems = (String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.STRING_ITEM_LIST);
            short iselpos = (short) JavaTools.FieldInList(sitems, getTablePrefix() + _selitem);
            if (iselpos > -1)
            {
                Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, new short[] { iselpos });
            }
            this.fillUpFieldsListbox();
        }
        else
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.SELECTED_ITEMS, new short[] {} );
            this.fillUpFieldsListbox();
        }
        if (_bReadOnly)
        {
            Helper.setUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.READ_ONLY, Boolean.valueOf(_selitem.length() > 0));
        }
        toggleListboxButtons((short)-1,(short)-1);
    }
}
