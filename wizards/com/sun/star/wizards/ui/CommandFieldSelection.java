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

import com.sun.star.wizards.common.*;
import com.sun.star.wizards.db.*;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdb.CommandType;
import com.sun.star.uno.AnyConverter;
import com.sun.star.awt.*;
import com.sun.star.beans.PropertyValue;

import java.text.Collator;
import java.util.Comparator;

public class CommandFieldSelection extends FieldSelection implements Comparator<String>
{

    private CommandMetaData CurDBMetaData;
    private XListBox xTableListBox;
    private XFixedText xlblTable;
    private String sTableListBoxName;
    private String sQueryPrefix;
    private String sTablePrefix;
    private short m_iSelPos = -1;
    private short iOldSelPos = -1;
    private boolean bgetQueries;
    private final WizardDialog oWizardDialog;
    private Collator aCollator = null;

    private class ItemListenerImpl implements com.sun.star.awt.XItemListener
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
     */
    public CommandFieldSelection(WizardDialog _CurUnoDialog, CommandMetaData _CurDBMetaData, int iStep, int _iHeight, String _reslblFields, String _reslblSelFields, String _reslblTables, boolean _bgetQueries, int _ifirstHID)
    {
        super(_CurUnoDialog, iStep, 95, 57, 210, _iHeight, _reslblFields, _reslblSelFields, (_ifirstHID + 1), true);
        insertControls(_CurDBMetaData, _bgetQueries, _reslblTables);
        oWizardDialog = (WizardDialog) CurUnoDialog;
    }

    /**
     * instantiates a CommandFieldSelection with a preselected command
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
            String sTableLabelName = "lblTables_" + super.sIncSuffix;
            sTableListBoxName = "lstTables_" + super.sIncSuffix;
            sTablePrefix = getTablePrefix();
            sQueryPrefix = getQueryPrefix();
            Integer LabelWidth = Integer.valueOf(getListboxWidth().intValue() + 6);
            // Label 'Tables or Queries'
            xlblTable = CurUnoDialog.insertLabel(sTableLabelName,
                    new String[]
                    {
                        PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.FALSE, 8, _reslblTables, 95, 27, IStep, Short.valueOf((short) 3), LabelWidth
                    });
            // DropDown Listbox TableNames
            xTableListBox = CurUnoDialog.insertListBox(sTableListBoxName, 0, null, new ItemListenerImpl(),
                    new String[]
                    {
                        "Dropdown", PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, "LineCount", PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        Boolean.TRUE, Boolean.FALSE, 12, HelpIds.getHelpIdString(super.FirstHelpIndex - 1), Short.valueOf(UnoDialog.getListBoxLineCount()), 95, 37, IStep, Short.valueOf((short) 4), getListboxWidth()
                    });
            fillupCommandListBox();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    /**
     * @return Returns the sQueryPrefix.
     */
    private String getQueryPrefix()
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
    private String getTablePrefix()
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
            String[] sItemList = ((String[]) Helper.getUnoPropertyValue(UnoDialog.getModel(xTableListBox), PropertyNames.STRING_ITEM_LIST));
            if (sItemList.length > 0)
            {
                return (short) 0;
            }
            return (short) -1;
        }
    }

    private void fillUpFieldsListbox()
    {
        try
        {
            boolean binitialize = false;
            String curCommandName = PropertyNames.EMPTY_STRING;
            //As the peer of the control might not yet exist we have to query the model for the SelectedItems
            short iSelPos = getselectedItemPos();
            final String sSelectedTableName = xTableListBox.getItem(iSelPos);
            if (!bgetQueries)
            {
                curCommandName = sSelectedTableName; // sLocList[iSelPos];
                CurDBMetaData.setTableByName(curCommandName);
                binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE);
            }
            else
            {
                if (sSelectedTableName.startsWith(sTablePrefix))
                {
                    CurDBMetaData.setCommandType(CommandType.TABLE);
                    curCommandName = JavaTools.replaceSubString(sSelectedTableName, PropertyNames.EMPTY_STRING, sTablePrefix);
                    CurDBMetaData.setTableByName(curCommandName);
                    binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.TABLE);
                }
                else
                {
                    CurDBMetaData.setCommandType(CommandType.QUERY);
                    curCommandName = JavaTools.replaceSubString(sSelectedTableName, PropertyNames.EMPTY_STRING, sQueryPrefix);
                    CurDBMetaData.setQueryByName(curCommandName);
                    binitialize = CurDBMetaData.getFieldNamesOfCommand(curCommandName, CommandType.QUERY);
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
            exception.printStackTrace(System.err);
        }
    }

    /** returns the selected entry index in the commandListbox
     *
     * @return
     * @throws com.sun.star.wizards.common.TerminateWizardException
     */
    private short fillupCommandListBox()
    {
        short[] iSelArray = new short[0];
        boolean bgetFields = false;
        String[] ContentList = new String[0];
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
        if (iSelPos > -1)
        {
            bgetFields = true;
            iSelArray = new short[]
                    {
                        iSelPos
                    };
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
            com.sun.star.lang.Locale aOfficeLocale = Configuration.getLocale(this.CurDBMetaData.xMSF);
            java.util.Locale aJavaLocale = new java.util.Locale(aOfficeLocale.Language, aOfficeLocale.Country, aOfficeLocale.Variant);
            //Get the Collator for US English and set its strength to PRIMARY
            this.aCollator = Collator.getInstance(aJavaLocale);
            aCollator.setStrength(Collator.TERTIARY);
        }
        return aCollator;
    }

    public int compare(String _oObject1, String _oObject2)
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
        boolean bdoenable = QueryMetaData.getIncludedCommandNames(_NewItems).length < CurDBMetaData.getMaxTablesInSelect();
        toggleCommandListBox(bdoenable);
    }

    private void toggleCommandListBox(boolean _bdoenable)
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
        else if (sCommandname.startsWith(this.sQueryPrefix))
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

    private void preselectCommand(String _selitem, int _nCommandType, boolean _bReadOnly)
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
