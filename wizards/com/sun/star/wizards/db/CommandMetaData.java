/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CommandMetaData.java,v $
 * $Revision: 1.16 $
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

package com.sun.star.wizards.db;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.KeyType;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XKeysSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.beans.*;
import com.sun.star.uno.UnoRuntime;
import java.util.*;
import com.sun.star.lang.Locale;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.embed.EntryInitModes;
import com.sun.star.frame.*;

public class CommandMetaData extends DBMetaData
{
    public Map FieldTitleSet = new HashMap();
    public String[] AllFieldNames = new String[]{};
    public FieldColumn[] FieldColumns = new FieldColumn[]{};
//  public String[] FieldNames = new String[] {};
    public String[] GroupFieldNames = new String[] {};
    private String[][] SortFieldNames = new String[][] {};
    private String[] RecordFieldNames = new String[] {};
    public String[][] AggregateFieldNames = new String[][] {};
    public String[] NumericFieldNames = new String[] {};
    public String[] NonAggregateFieldNames;
    public int[] FieldTypes;
    private int CommandType;
    private String Command;
    boolean bCatalogAtStart = true;
    String sCatalogSep = "";
    String sIdentifierQuote = "";
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
                if (_bgetDefaultValue)
                {
                    FieldColumns[i].getDefaultValue();
                }
            }
        }


    public void initializeFieldColumns(String[] _FieldNames, XNameAccess _xColumns)
        {
            FieldColumns = new FieldColumn[_FieldNames.length];
            for (int i = 0; i < _FieldNames.length; i++)
            {
                FieldColumns[i] = new FieldColumn(this,_xColumns, _FieldNames[i] );
            }
        }


    public void initializeFieldColumns(String[] _FieldNames, String _CommandName)
        {
            this.setCommandName(_CommandName);
            FieldColumns = new FieldColumn[_FieldNames.length];
            for (int i = 0; i < _FieldNames.length; i++)
            {
                FieldColumns[i] = new FieldColumn(this, _FieldNames[i], _CommandName, false);
                if (FieldTitleSet != null)
                {
                    if (FieldTitleSet.containsKey(_FieldNames[i]))
                    {
                        FieldColumns[i].setFieldTitle( (String) FieldTitleSet.get(_FieldNames[i]) );
                        if (FieldColumns[i].getFieldTitle() == null)
                        {
                            FieldColumns[i].setFieldTitle( _FieldNames[i] );
                            FieldTitleSet.put(_FieldNames[i], _FieldNames[i]);
                        }
                    }
                }
            }
        }

    public Map getFieldTitleSet()
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
                Object oColumn = oCommand.xColumns.getByName(CurFieldColumn.m_sFieldName);
                XPropertySet xColumn = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oColumn);
                return xColumn;
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
                return null;
            }
        }


    // @SuppressWarnings("unchecked")
    public void prependSortFieldNames(String[] _fieldnames)
        {
            Vector aSortFields = new Vector();
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
                    sSortFieldName[1] = "ASC";
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
            if (FieldColumns.length > 0)
            {
                for (int i = 0; i < FieldColumns.length; i++)
                {
                    if (FieldColumns[i].m_sFieldName.equals(_FieldName))

                    {
                        if (FieldColumns[i].getCommandName().equals(_CommandName))
                        {
                            return FieldColumns[i];
                        }
                    }
                }
            }
            return null;
        }



    public FieldColumn getFieldColumnByFieldName(String _FieldName)
        {
            for (int i = 0; i < FieldColumns.length; i++)
            {
                String sFieldName = FieldColumns[i].m_sFieldName;
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
                if (FieldColumns[i].m_sFieldName.equals(_FieldTitle))
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
                Object oField;
                java.util.Vector ResultFieldNames = new java.util.Vector(10);
                String[] FieldNames;
                CommandObject oCommand = this.getCommandByName(_commandname, _commandtype);
                FieldNames = oCommand.xColumns.getElementNames();
                if (FieldNames.length > 0)
                {
                    for (int n = 0; n < FieldNames.length; n++)
                    {
                        oField = oCommand.xColumns.getByName(FieldNames[n]);
                        int iType = AnyConverter.toInt(Helper.getUnoPropertyValue(oField, "Type"));
                        // BinaryFieldTypes are not included in the WidthList
                        if (JavaTools.FieldInIntTable(WidthList, iType) >= 0)
                        {
//                        if (_bAppendMode)
//                            ResultFieldNames.addElement(_commandname + "." + FieldNames[n]);
//                        else
                            ResultFieldNames.addElement(FieldNames[n]);
                        }
                    }
                    FieldNames = new String[FieldNames.length];
                    FieldTypes = new int[FieldNames.length];
                    AllFieldNames = new String[ResultFieldNames.size()];
                    ResultFieldNames.copyInto(AllFieldNames);
                    return true;
                }
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            Resource oResource = new Resource(xMSF, "Database", "dbw");
            String sMsgNoFieldsFromCommand = oResource.getResText(RID_DB_COMMON + 45);
            sMsgNoFieldsFromCommand = JavaTools.replaceSubString(sMsgNoFieldsFromCommand, _commandname, "%NAME");
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgNoFieldsFromCommand);
            return false;
        }






    public String[] getOrderableColumns(String[] _fieldnames)
        {
            Vector aOrderableColumns = new Vector();
            int ncount = 0;
            for (int i = 0; i < _fieldnames.length; i++)
            {
                FieldColumn ofieldcolumn = getFieldColumnByFieldName(_fieldnames[i]);
                if (getDBDataTypeInspector().isColumnOrderable(ofieldcolumn.getXColumnPropertySet()))
                {
                    aOrderableColumns.addElement(_fieldnames[i]);
                    ncount++;
                }
            }
            String[] sretfieldnames = new String[ncount];
            aOrderableColumns.toArray(sretfieldnames);
            return sretfieldnames;
        }


    /**
     * @return Returns the command.
     */
    public String getCommandName()
        {
            return Command;
        }
    /**
     * @param command The command to set.
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
     * @param commandType The commandType to set.
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
                Object oField = oTable.xColumns.getByName(_oFieldColumn.getFieldName());
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
                exception.printStackTrace(System.out);
                return false;
            }
        }

    public String[] setNumericFields()
        {
            try
            {
                Vector numericfieldsvector = new java.util.Vector();
                for (int i = 0; i < FieldColumns.length; i++)
                {
                    if (isnumeric(FieldColumns[i]))
                    {
                        numericfieldsvector.addElement(FieldColumns[i].getDisplayFieldName());
                    }
                }
                NumericFieldNames = new String[numericfieldsvector.size()];
                numericfieldsvector.toArray(NumericFieldNames);
                return NumericFieldNames;
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
                return new String[]{};
            }
        }

    public String[] getFieldNames(String[] _sDisplayFieldNames, String _sCommandName)
        {
            Vector sFieldNamesVector = new java.util.Vector();
            for (int i = 0; i < FieldColumns.length; i++)
            {
                if (_sCommandName.equals(FieldColumns[i].getCommandName()))
                {
                    if (JavaTools.FieldInList(_sDisplayFieldNames, FieldColumns[i].getDisplayFieldName()) > -1)
                    {
                        sFieldNamesVector.addElement(FieldColumns[i].m_sFieldName);
                    }
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
                sFieldNames[i] = FieldColumns[i].m_sFieldName;
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
                Vector nonaggregatefieldsvector = new java.util.Vector();
                for (int i = 0; i < FieldColumns.length; i++)
                {
                    if (JavaTools.FieldInTable(AggregateFieldNames, FieldColumns[i].getDisplayFieldName()) == -1)
                    {
                        nonaggregatefieldsvector.addElement(FieldColumns[i].getDisplayFieldName());
                    }
                }
                NonAggregateFieldNames = new String[nonaggregatefieldsvector.size()];
                nonaggregatefieldsvector.toArray(NonAggregateFieldNames);
                return NonAggregateFieldNames;
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
                return new String[]{};
            }
        }

    /**
     * the fieldnames passed over are not necessarily the ones that are defined in the class
     * @param FieldNames
     * @return
     */
    public boolean hasNumericalFields(String[] _DisplayFieldNames)

        {
            if (_DisplayFieldNames != null)
            {
                if (_DisplayFieldNames.length > 0)
                {
                    for (int i = 0; i < _DisplayFieldNames.length; i++)
                    {
                        if (isnumeric(getFieldColumnByDisplayName(_DisplayFieldNames[i])))
                        {
                            return true;
                        }
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
                FieldTitle = (String) this.FieldTitleSet.get(FieldName); //FieldTitles[TitleIndex];
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
                CurFieldName = FieldColumns[i].m_sFieldName;
                if (JavaTools.FieldInList(GroupFieldNames, CurFieldName) < 0)
                {
                    RecordFieldNames[a] = CurFieldName;
                    // a += 1;
                    ++a;
                }
            }
        }
        public void setRecordFieldNames(String [] _aNewList)
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

    public XComponent[] switchtoDesignmode(String _commandname, int _commandtype,XFrame parentFrame)
        {
            XComponent[] ret = null;
            PropertyValue[] rDispatchArguments = new PropertyValue[_commandtype == com.sun.star.sdb.CommandType.QUERY ? 5 : 3];
            rDispatchArguments[0] = Properties.createProperty("DataSourceName", this.DataSourceName);
            rDispatchArguments[1] = Properties.createProperty("ActiveConnection", this.DBConnection);
            if (_commandtype == com.sun.star.sdb.CommandType.QUERY)
            {
                rDispatchArguments[2] = Properties.createProperty("GraphicalDesign", Boolean.TRUE);
                rDispatchArguments[3] = Properties.createProperty("Command", _commandname);
                rDispatchArguments[4] = Properties.createProperty("CommandType", new Integer(_commandtype));
                ret = showCommandView(".component:DB/QueryDesign", rDispatchArguments,parentFrame);
            }
            else
            {
                rDispatchArguments[2] = Properties.createProperty("CurrentTable", _commandname);
                ret = showCommandView(".component:DB/TableDesign", rDispatchArguments,parentFrame);
            }
            return ret;
        }

    public XComponent[] switchtoDataViewmode(String _commandname, int _commandtype,XFrame parentFrame)
        {
            PropertyValue[] rDispatchArguments = new PropertyValue[7];
            rDispatchArguments[0] = Properties.createProperty("DataSourceName", this.DataSourceName);
            rDispatchArguments[1] = Properties.createProperty("ActiveConnection", this.DBConnection);
            rDispatchArguments[2] = Properties.createProperty("Command", _commandname);
            rDispatchArguments[3] = Properties.createProperty("CommandType", new Integer(_commandtype));
            rDispatchArguments[4] = Properties.createProperty("ShowTreeView", Boolean.FALSE);
            rDispatchArguments[5] = Properties.createProperty("ShowTreeViewButton", Boolean.FALSE);
            rDispatchArguments[6] = Properties.createProperty("ShowMenu", Boolean.TRUE);
            return showCommandView(".component:DB/DataSourceBrowser", rDispatchArguments,parentFrame);
        }

    //
    public XComponent[] showCommandView(String surl, PropertyValue[] _rArgs,XFrame parentFrame)
        {
            XComponent[] ret = new XComponent[2];
            try
            {
                XSingleServiceFactory xFac = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class,xMSF.createInstance("com.sun.star.frame.TaskCreator"));
                Object[] args = new Object[2];
                args[0] = Properties.createProperty("ParentFrame",parentFrame);
                args[1] = Properties.createProperty("TopWindow",Boolean.TRUE);

                XComponentLoader xLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class,xFac.createInstanceWithArguments(args));
                ret[0] = xLoader.loadComponentFromURL(surl, "_self", 0, _rArgs);
                if ( ret[0] != null)
                    ret[0] = (XComponent)UnoRuntime.queryInterface(XComponent.class,xLoader);
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
            return ret;
        }
    /**@deprecated use 'RelationController' class instead
     *
     * @param _stablename
     * @param _ncommandtype
     * @return
     */
    public String[] getReferencedTables(String _stablename, int _ncommandtype)
        {
            String[] sTotReferencedTables = new String[]{};
            try
            {
                if (_ncommandtype == com.sun.star.sdb.CommandType.TABLE)
                {
                    if (xDBMetaData.supportsIntegrityEnhancementFacility())
                    {
                        java.util.Vector TableVector = new java.util.Vector();
                        Object oTable = xTableNames.getByName(_stablename);
                        XKeysSupplier xKeysSupplier = (XKeysSupplier) UnoRuntime.queryInterface(XKeysSupplier.class, oTable);
                        xIndexKeys = xKeysSupplier.getKeys();
                        for (int i = 0; i < xIndexKeys.getCount(); i++)
                        {
                            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xIndexKeys.getByIndex(i) );
                            int curtype = AnyConverter.toInt(xPropertySet.getPropertyValue("Type"));
                            if (curtype == KeyType.FOREIGN)
                            {
                                // getImportedKeys (RelationController.cxx /source/ui/relationdesign) /Zeile 475
                                String sreftablename = AnyConverter.toString(xPropertySet.getPropertyValue("ReferencedTable"));
                                if (xTableNames.hasByName(sreftablename))
                                {
                                    TableVector.addElement(sreftablename);
                                }
                            }
                        }
                        if (TableVector.size() > 0)
                        {
                            sTotReferencedTables = new String[TableVector.size()];
                            TableVector.copyInto(sTotReferencedTables);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
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
                    XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xIndexKeys.getByIndex(i) );
                    int curtype = AnyConverter.toInt(xPropertySet.getPropertyValue("Type"));
                    if (curtype == KeyType.FOREIGN)
                    {
                        String scurreftablename = AnyConverter.toString(xPropertySet.getPropertyValue("ReferencedTable"));
                        if (xTableNames.hasByName(scurreftablename))
                        {
                            if (scurreftablename.equals(_sreferencedtablename))
                            {
                                XColumnsSupplier xColumnsSupplier = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, xPropertySet);
                                String[] smastercolnames = xColumnsSupplier.getColumns().getElementNames();
                                skeycolumnnames = new String[2][smastercolnames.length];
                                skeycolumnnames[0] = smastercolnames;
                                skeycolumnnames[1] = new String[smastercolnames.length];
                                for (int n = 0; n < smastercolnames.length; n++)
                                {
                                    XPropertySet xcolPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xColumnsSupplier.getColumns().getByName(smastercolnames[n]));
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
            try
            {
                Object oEmbeddedFactory = super.xMSF.createInstance("com.sun.star.embed.OOoEmbeddedObjectFactory");
                int iEntryInitMode = EntryInitModes.DEFAULT_INIT;       //TRUNCATE_INIT???
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }


    public void setCommandComposingAttributes()
        {
            try
            {
                boolean bCatalogAtStart = xDBMetaData.isCatalogAtStart();
                sCatalogSep = xDBMetaData.getCatalogSeparator();
                sIdentifierQuote = xDBMetaData.getIdentifierQuoteString();
                bCommandComposerAttributesalreadyRetrieved = true;
            }
            catch (SQLException e)
            {
                e.printStackTrace(System.out);
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
