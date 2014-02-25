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
package com.sun.star.wizards.db;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.Locale;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.Resource;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class CommandMetaData extends DBMetaData
{

    public Map<String, String> FieldTitleSet = new HashMap<String, String>();
    public String[] m_aAllFieldNames = new String[]
    {
    };
    public FieldColumn[] FieldColumns = new FieldColumn[]
    {
    };
    public String[] GroupFieldNames = new String[]
    {
    };
    private String[][] SortFieldNames = new String[][]
    {
    };
    private String[] RecordFieldNames = new String[]
    {
    };
    public String[][] AggregateFieldNames = new String[][]
    {
    };
    public String[] NumericFieldNames = new String[]
    {
    };
    public String[] NonAggregateFieldNames;
    private int CommandType;
    private String Command;
    boolean bCatalogAtStart = true;
    String sCatalogSep = PropertyNames.EMPTY_STRING;
    String sIdentifierQuote = PropertyNames.EMPTY_STRING;
    boolean bCommandComposerAttributesalreadyRetrieved = false;
    private XIndexAccess xIndexKeys;

    public CommandMetaData(XMultiServiceFactory xMSF, Locale _aLocale, NumberFormatter oNumberFormatter)
    {
        super(xMSF, _aLocale, oNumberFormatter);
    }

    public CommandMetaData(XMultiServiceFactory xMSF)
    {
        super(xMSF);
    }

    public void initializeFieldColumns(boolean _bgetDefaultValue, String _CommandName, String[] _FieldNames)
    {
        this.setCommandName(_CommandName);
        FieldColumns = new FieldColumn[_FieldNames.length];
        for (int i = 0; i < _FieldNames.length; i++)
        {
            FieldColumns[i] = new FieldColumn(this, _FieldNames[i], this.getCommandName(), false);
//                if (_bgetDefaultValue)
//                {
//                    FieldColumns[i].getDefaultValue();
//                }
        }
    }

    public void initializeFieldColumns(String[] _FieldNames, XNameAccess _xColumns)
    {
        FieldColumns = new FieldColumn[_FieldNames.length];
        for (int i = 0; i < _FieldNames.length; i++)
        {
            FieldColumns[i] = new FieldColumn(this, _xColumns, _FieldNames[i]);
        }
    }

    public void initializeFieldColumns(String[] _FieldNames, String _CommandName)
    {
        this.setCommandName(_CommandName);
        FieldColumns = new FieldColumn[_FieldNames.length];
        for (int i = 0; i < _FieldNames.length; i++)
        {
            FieldColumns[i] = new FieldColumn(this, _FieldNames[i], _CommandName, false);
            if (FieldTitleSet != null && FieldTitleSet.containsKey(_FieldNames[i]))
            {
                FieldColumns[i].setFieldTitle(FieldTitleSet.get(_FieldNames[i]));
                if (FieldColumns[i].getFieldTitle() == null)
                {
                    FieldColumns[i].setFieldTitle(_FieldNames[i]);
                    FieldTitleSet.put(_FieldNames[i], _FieldNames[i]);
                }
            }
        }
    }

    public Map<String, String> getFieldTitleSet()
    {
        return FieldTitleSet;
    }

    public XPropertySet getColumnObjectByFieldName(String _FieldName, boolean _bgetByDisplayName)
    {
        try
        {
            FieldColumn CurFieldColumn = null;
            if (_bgetByDisplayName)
            {
                CurFieldColumn = this.getFieldColumnByDisplayName(_FieldName);
            }
            else
            {
                CurFieldColumn = this.getFieldColumnByFieldName(_FieldName);
            }
            String CurCommandName = CurFieldColumn.getCommandName();
            CommandObject oCommand = getTableByName(CurCommandName);
            Object oColumn = oCommand.getColumns().getByName(CurFieldColumn.getFieldName());
            return UnoRuntime.queryInterface(XPropertySet.class, oColumn);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    // @SuppressWarnings("unchecked")
    public void prependSortFieldNames(String[] _fieldnames)
    {
        ArrayList<String[]> aSortFields = new ArrayList<String[]>();
        for (int i = 0; i < _fieldnames.length; i++)
        {
            String[] sSortFieldName = new String[2];
            sSortFieldName[0] = _fieldnames[i];
            int index = JavaTools.FieldInTable(SortFieldNames, _fieldnames[i]);
            if (index > -1)
            {
                sSortFieldName[1] = SortFieldNames[index][1];
            }
            else
            {
                sSortFieldName[1] = PropertyNames.ASC;
            }
            aSortFields.add(sSortFieldName);
        }
        for (int i = 0; i < SortFieldNames.length; i++)
        {
            if (JavaTools.FieldInList(_fieldnames, SortFieldNames[i][0]) == -1)
            {
                aSortFields.add(SortFieldNames[i]);
            }
        }
        SortFieldNames = new String[aSortFields.size()][2];
        aSortFields.toArray(SortFieldNames);
    }

    public String[][] getSortFieldNames()
    {
        return SortFieldNames;
    }

    public void setSortFieldNames(String[][] aNewListList)
    {
        SortFieldNames = aNewListList;
    }

    public FieldColumn getFieldColumn(String _FieldName, String _CommandName)
    {
        for (int i = 0; i < FieldColumns.length; i++)
        {
            if (FieldColumns[i].getFieldName().equals(_FieldName) && FieldColumns[i].getCommandName().equals(_CommandName))
            {
                return FieldColumns[i];
            }
        }
        return null;
    }

    public FieldColumn getFieldColumnByFieldName(String _FieldName)
    {
        for (int i = 0; i < FieldColumns.length; i++)
        {
            String sFieldName = FieldColumns[i].getFieldName();
            if (sFieldName.equals(_FieldName))
            {
                return FieldColumns[i];
            }
            if (_FieldName.indexOf('.') == -1)
            {
                String sCompound = Command + "." + _FieldName;
                if (sFieldName.equals(sCompound))
                {
                    return FieldColumns[i];
                }
            }
        }
        throw new com.sun.star.uno.RuntimeException();
    }

    public FieldColumn getFieldColumnByDisplayName(String _DisplayName)
    {
        String identifierQuote = getIdentifierQuote();
        for (int i = 0; i < FieldColumns.length; i++)
        {
            String sDisplayName = FieldColumns[i].getDisplayFieldName();
            if (sDisplayName.equals(_DisplayName))
            {
                return FieldColumns[i];
            }
            if (_DisplayName.indexOf('.') == -1)
            {
                String sCompound = Command + "." + _DisplayName;
                if (sDisplayName.equals(sCompound))
                {
                    return FieldColumns[i];
                }
            }
            String quotedName = new StringBuilder(CommandName.quoteName(FieldColumns[i].getCommandName(), identifierQuote)).append('.').append(CommandName.quoteName(FieldColumns[i].getFieldName(), identifierQuote)).toString();
            if (quotedName.equals(_DisplayName))
            {
                return FieldColumns[i];
            }
        }
        throw new com.sun.star.uno.RuntimeException();
    }

    public FieldColumn getFieldColumnByTitle(String _FieldTitle)
    {
        for (int i = 0; i < FieldColumns.length; i++)
        {
            if (FieldColumns[i].getFieldTitle().equals(_FieldTitle))
            {
                return FieldColumns[i];
            }
        }
        // throw new com.sun.star.uno.RuntimeException();
        // LLA: Group works with fields direct
        for (int i = 0; i < FieldColumns.length; i++)
        {
            if (FieldColumns[i].getFieldName().equals(_FieldTitle))
            {
                return FieldColumns[i];
            }
        }
        throw new com.sun.star.uno.RuntimeException();
    }

    public boolean getFieldNamesOfCommand(String _commandname, int _commandtype, boolean _bAppendMode)
    {
        try
        {
            // Object oField;
            java.util.ArrayList<String> ResultFieldNames = new java.util.ArrayList<String>(10);
            String[] FieldNames;
            CommandObject oCommand = this.getCommandByName(_commandname, _commandtype);
            FieldNames = oCommand.getColumns().getElementNames();
            if (FieldNames.length > 0)
            {
                for (int n = 0; n < FieldNames.length; n++)
                {
                    final String sFieldName = FieldNames[n];
                    Object oField = oCommand.getColumns().getByName(sFieldName);
                    int iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
                    // BinaryFieldTypes are not included in the WidthList
                    if (JavaTools.FieldInIntTable(WidthList, iType) >= 0)
                    {
//                        if (_bAppendMode)
//                            ResultFieldNames.addElement(_commandname + "." + FieldNames[n]);
//                        else
                        ResultFieldNames.add(sFieldName);
                    }
                    else if (JavaTools.FieldInIntTable(BinaryTypes, iType) >= 0)
                    {
                        ResultFieldNames.add(sFieldName);
                    }
                }
                // FieldNames = new String[FieldNames.length];
                // FieldTypes = new int[FieldNames.length];
                m_aAllFieldNames = new String[ResultFieldNames.size()];
                m_aAllFieldNames = ResultFieldNames.toArray(m_aAllFieldNames);
                return true;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        Resource oResource = new Resource(xMSF, "Database", "dbw");
        String sMsgNoFieldsFromCommand = oResource.getResText(RID_DB_COMMON + 45);
        sMsgNoFieldsFromCommand = JavaTools.replaceSubString(sMsgNoFieldsFromCommand, _commandname, "%NAME");
        showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgNoFieldsFromCommand);
        return false;
    }

    public String[] getOrderableColumns(String[] _fieldnames)
    {
        ArrayList<String> aOrderableColumns = new ArrayList<String>();
        for (int i = 0; i < _fieldnames.length; i++)
        {
            FieldColumn ofieldcolumn = getFieldColumnByFieldName(_fieldnames[i]);
            if (getDBDataTypeInspector().isColumnOrderable(ofieldcolumn.getXColumnPropertySet()))
            {
                aOrderableColumns.add(_fieldnames[i]);
            }
        }
        String[] sretfieldnames = new String[aOrderableColumns.size()];
        return aOrderableColumns.toArray(sretfieldnames);
    }

    /**
     * @return Returns the command.
     */
    public String getCommandName()
    {
        return Command;
    }

    /**
     * @param _command The command to set.
     */
    public void setCommandName(String _command)
    {
        Command = _command;
    }

    /**
     * @return Returns the commandType.
     */
    public int getCommandType()
    {
        return CommandType;
    }

    /**
     * @param _commandType The commandType to set.
     */
    public void setCommandType(int _commandType)
    {
        CommandType = _commandType;
    }

    public boolean isnumeric(FieldColumn _oFieldColumn)
    {
        try
        {
            CommandObject oTable = super.getTableByName(_oFieldColumn.getCommandName());
            Object oField = oTable.getColumns().getByName(_oFieldColumn.getFieldName());
            int iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
            int ifound = java.util.Arrays.binarySearch(NumericTypes, iType);
            if ((ifound < NumericTypes.length) && (ifound > 0))
            {
                return (NumericTypes[ifound] == iType);
            }
            else
            {
                return false;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return false;
        }
    }

    public String[] setNumericFields()
    {
        try
        {
            ArrayList<String> numericfieldsvector = new java.util.ArrayList<String>();
            for (int i = 0; i < FieldColumns.length; i++)
            {
                if (isnumeric(FieldColumns[i]))
                {
                    numericfieldsvector.add(FieldColumns[i].getDisplayFieldName());
                }
            }
            NumericFieldNames = new String[numericfieldsvector.size()];
            numericfieldsvector.toArray(NumericFieldNames);
            return NumericFieldNames;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return new String[]
                    {
                    };
        }
    }

    public String[] getFieldNames(String[] _sDisplayFieldNames, String _sCommandName)
    {
        ArrayList<String> sFieldNamesVector = new java.util.ArrayList<String>();
        for (int i = 0; i < FieldColumns.length; i++)
        {
            if (_sCommandName.equals(FieldColumns[i].getCommandName()) && JavaTools.FieldInList(_sDisplayFieldNames, FieldColumns[i].getDisplayFieldName()) > -1)
            {
                sFieldNamesVector.add(FieldColumns[i].getFieldName());
            }
        }
        String[] sFieldNames = new String[sFieldNamesVector.size()];
        sFieldNamesVector.toArray(sFieldNames);
        return sFieldNames;
    }

    public String[] getFieldNames()
    {
        String[] sFieldNames = new String[FieldColumns.length];
        for (int i = 0; i < FieldColumns.length; i++)
        {
            sFieldNames[i] = FieldColumns[i].getFieldName();
        }
        return sFieldNames;
    }

    public String[] getDisplayFieldNames()
    {
        String[] sDisplayFieldNames = new String[FieldColumns.length];
        for (int i = 0; i < FieldColumns.length; i++)
        {
            sDisplayFieldNames[i] = FieldColumns[i].getDisplayFieldName();
        }
        return sDisplayFieldNames;
    }

    public String[] setNonAggregateFieldNames()
    {
        try
        {
            ArrayList<String> nonaggregatefieldsvector = new java.util.ArrayList<String>();
            for (int i = 0; i < FieldColumns.length; i++)
            {
                if (JavaTools.FieldInTable(AggregateFieldNames, FieldColumns[i].getDisplayFieldName()) == -1)
                {
                    nonaggregatefieldsvector.add(FieldColumns[i].getDisplayFieldName());
                }
            }
            NonAggregateFieldNames = new String[nonaggregatefieldsvector.size()];
            nonaggregatefieldsvector.toArray(NonAggregateFieldNames);
            return NonAggregateFieldNames;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return new String[]
                    {
                    };
        }
    }

    /**
     * the fieldnames passed over are not necessarily the ones that are defined in the class
     * @param _DisplayFieldNames
     * @return
     */
    public boolean hasNumericalFields(String[] _DisplayFieldNames)
    {
        if (_DisplayFieldNames != null && _DisplayFieldNames.length > 0)
        {
            for (int i = 0; i < _DisplayFieldNames.length; i++)
            {
                if (isnumeric(getFieldColumnByDisplayName(_DisplayFieldNames[i])))
                {
                    return true;
                }
            }
        }
        return false;
    }

    public String getFieldTitle(String FieldName)
    {
        String FieldTitle = FieldName;
        if (this.FieldTitleSet != null)
        {
            FieldTitle = this.FieldTitleSet.get(FieldName); //FieldTitles[TitleIndex];
            if (FieldTitle == null)
            {
                return FieldName;
            }
        }
        return FieldTitle;
    }

    public void setFieldTitles(String[] sFieldTitles)
    {
        int nFieldColLength = FieldColumns.length;
        for (int i = 0; i < sFieldTitles.length; i++)
        {
            if (i < nFieldColLength)
            {
                FieldColumns[i].setFieldTitle(sFieldTitles[i]);
            }

        }
    }

    public String[] getFieldTitles()
    {
        String[] sFieldTitles = new String[FieldColumns.length];
        for (int i = 0; i < FieldColumns.length; i++)
        {
            sFieldTitles[i] = FieldColumns[i].getFieldTitle();
        }
        return sFieldTitles;
    }

    public void setGroupFieldNames(String[] GroupFieldNames)
    {
        this.GroupFieldNames = GroupFieldNames;
    }

    public String[] getGroupFieldNames()
    {
        return GroupFieldNames;
    }

    public void createRecordFieldNames()
    {
        String CurFieldName;
        int GroupFieldCount;
        int TotFieldCount = FieldColumns.length;
        //    int SortFieldCount = SortFieldNames[0].length;
        GroupFieldCount = JavaTools.getArraylength(GroupFieldNames);
        RecordFieldNames = new String[TotFieldCount - GroupFieldCount];

        int a = 0;
        for (int i = 0; i < TotFieldCount; i++)
        {
            CurFieldName = FieldColumns[i].getFieldName();
            if (JavaTools.FieldInList(GroupFieldNames, CurFieldName) < 0)
            {
                RecordFieldNames[a] = CurFieldName;
                // a += 1;
                ++a;
            }
        }
    }

    public void setRecordFieldNames(String[] _aNewList)
    {
        RecordFieldNames = _aNewList;
    }

    public String[] getRecordFieldNames()
    {
        return RecordFieldNames;
    }

    public String getRecordFieldName(int i)
    {
        return RecordFieldNames[i];
    }

    /**@deprecated use 'RelationController' class instead
     *
     * @param _stablename
     * @param _ncommandtype
     * @return
     */
    public String[] getReferencedTables(String _stablename, int _ncommandtype)
    {
        String[] sTotReferencedTables = new String[]
        {
        };
        try
        {
            if (_ncommandtype == com.sun.star.sdb.CommandType.TABLE && xDBMetaData.supportsIntegrityEnhancementFacility())
            {
                java.util.ArrayList<String> TableVector = new java.util.ArrayList<String>();
                Object oTable = getTableNamesAsNameAccess().getByName(_stablename);
                XKeysSupplier xKeysSupplier = UnoRuntime.queryInterface(XKeysSupplier.class, oTable);
                xIndexKeys = xKeysSupplier.getKeys();
                for (int i = 0; i < xIndexKeys.getCount(); i++)
                {
                    XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xIndexKeys.getByIndex(i));
                    int curtype = AnyConverter.toInt(xPropertySet.getPropertyValue("Type"));
                    if (curtype == KeyType.FOREIGN)
                    {
                        // getImportedKeys (RelationController.cxx /source/ui/relationdesign) /Zeile 475
                        String sreftablename = AnyConverter.toString(xPropertySet.getPropertyValue("ReferencedTable"));
                        if (getTableNamesAsNameAccess().hasByName(sreftablename))
                        {
                            TableVector.add(sreftablename);
                        }
                    }
                }
                if (TableVector.size() > 0)
                {
                    sTotReferencedTables = new String[TableVector.size()];
                    TableVector.toArray(sTotReferencedTables);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return sTotReferencedTables;
    }

    /**@deprecated use 'RelationController' class instead
     *
     * @param _sreferencedtablename
     * @return
     */
    public String[][] getKeyColumns(String _sreferencedtablename)
    {
        String[][] skeycolumnnames = null;
        try
        {
            for (int i = 0; i < xIndexKeys.getCount(); i++)
            {
                XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xIndexKeys.getByIndex(i));
                int curtype = AnyConverter.toInt(xPropertySet.getPropertyValue("Type"));
                if (curtype == KeyType.FOREIGN)
                {
                    String scurreftablename = AnyConverter.toString(xPropertySet.getPropertyValue("ReferencedTable"));
                    if (getTableNamesAsNameAccess().hasByName(scurreftablename))
                    {
                        if (scurreftablename.equals(_sreferencedtablename))
                        {
                            XColumnsSupplier xColumnsSupplier = UnoRuntime.queryInterface(XColumnsSupplier.class, xPropertySet);
                            String[] smastercolnames = xColumnsSupplier.getColumns().getElementNames();
                            skeycolumnnames = new String[2][smastercolnames.length];
                            skeycolumnnames[0] = smastercolnames;
                            skeycolumnnames[1] = new String[smastercolnames.length];
                            for (int n = 0; n < smastercolnames.length; n++)
                            {
                                XPropertySet xcolPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xColumnsSupplier.getColumns().getByName(smastercolnames[n]));
                                skeycolumnnames[1][n] = AnyConverter.toString(xcolPropertySet.getPropertyValue("RelatedColumn"));
                            }
                            return skeycolumnnames;
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return skeycolumnnames;
    }

    public void openFormDocument(boolean _bReadOnly)
    {
    }

    public void setCommandComposingAttributes()
    {
        try
        {
            sCatalogSep = xDBMetaData.getCatalogSeparator();
            sIdentifierQuote = xDBMetaData.getIdentifierQuoteString();
            bCommandComposerAttributesalreadyRetrieved = true;
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.err);
        }
    }

    /**
     * @return Returns the bCatalogAtStart.
     */
    public boolean isCatalogAtStart()
    {
        if (!bCommandComposerAttributesalreadyRetrieved)
        {
            setCommandComposingAttributes();
        }
        return bCatalogAtStart;
    }

    /**
     * @return Returns the sCatalogSep.
     */
    public String getCatalogSeparator()
    {
        if (!bCommandComposerAttributesalreadyRetrieved)
        {
            setCommandComposingAttributes();
        }
        return sCatalogSep;
    }

    /**
     * @return Returns the sIdentifierQuote.
     */
    public String getIdentifierQuote()
    {
        if (!bCommandComposerAttributesalreadyRetrieved)
        {
            setCommandComposingAttributes();
        }
        return sIdentifierQuote;
    }
}
