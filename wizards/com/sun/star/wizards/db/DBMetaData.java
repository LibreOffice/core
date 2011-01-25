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
package com.sun.star.wizards.db;

import com.sun.star.awt.XWindow;
import com.sun.star.lang.XInitialization;
import com.sun.star.ui.dialogs.XExecutableDialog;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XHierarchicalNameContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.tools.XConnectionTools;
import com.sun.star.sdbcx.XColumnsSupplier;

import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XNumberFormatsSupplier;

import com.sun.star.task.XInteractionHandler;
import com.sun.star.sdb.XFormDocumentsSupplier;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdb.XCompletedConnection;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NamedValueCollection;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.uno.Any;
import com.sun.star.wizards.common.PropertyNames;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

public class DBMetaData
{
    private XNameAccess xQueryNames;
    public XDatabaseMetaData xDBMetaData;
    private XDataSource m_dataSource;
    private XPropertySet m_dataSourceSettings;
    private XOfficeDatabaseDocument xModel;
    private XPropertySet xDataSourcePropertySet;
    public String[] DataSourceNames;
    public String[] CommandNames;
    public java.util.Vector CommandObjects = new Vector(1);
    public Locale aLocale;
    public int[] CommandTypes;
    public String DataSourceName;
    public com.sun.star.sdbc.XConnection DBConnection;
    private com.sun.star.sdb.tools.XConnectionTools m_connectionTools;
    public com.sun.star.lang.XMultiServiceFactory xMSF;
    public XComponent xConnectionComponent;

    private XNameAccess xNameAccess;
    private XInterface xDatabaseContext;
    private XWindowPeer xWindowPeer;
    private String[] TableNames = new String[] {};
    private String[] QueryNames = new String[] {};

    protected int[][] WidthList;
    protected static final int[] NumericTypes = {
            DataType.TINYINT, // ==  -6;
            DataType.BIGINT, // ==  -5
            DataType.NUMERIC, // ==  - 2
            DataType.DECIMAL, // ==   3;
            DataType.INTEGER, // ==   4;
            DataType.SMALLINT, // ==   5;
            DataType.FLOAT, // ==   6;
            DataType.REAL, // ==   7;
            DataType.DOUBLE, // ==   8;
        };
    protected static final int[] BinaryTypes = { //new int[12];
            DataType.BINARY,
            DataType.VARBINARY,
            DataType.LONGVARBINARY,
            DataType.BLOB,
            DataType.SQLNULL,
            DataType.OBJECT,
            DataType.DISTINCT,
            DataType.STRUCT,
            DataType.ARRAY,
            DataType.CLOB,
            DataType.REF
            /* DataType.OTHER, */
        };

    private int iMaxColumnsInSelect;
    private int iMaxColumnsInGroupBy;
    private int iMaxColumnsInTable;
    private int iMaxColumnNameLength = -1;
    private int iMaxTableNameLength = -1;
    private boolean bPasswordIsRequired;
    private final static int NOLIMIT = 9999999;
    protected final static int RID_DB_COMMON = 1000;
    private final static int INVALID = 9999999;
    public TypeInspector oTypeInspector;
    private NumberFormatter oNumberFormatter = null;
    private long lDateCorrection = INVALID;
    private boolean bdisposeConnection = false;

    public XPropertySet getDataSourcePropertySet()
    {
        return xDataSourcePropertySet;
    }

    public DBMetaData(XMultiServiceFactory xMSF)
    {
        getInterfaces(xMSF);
        InitializeWidthList();
    }

    public DBMetaData(XMultiServiceFactory xMSF, Locale _aLocale, NumberFormatter _oNumberFormatter)
    {
        oNumberFormatter = _oNumberFormatter;
        aLocale = _aLocale;
        getInterfaces(xMSF);
        InitializeWidthList();
    }

    public NumberFormatter getNumberFormatter()
    {
        if (oNumberFormatter == null)
        {
            try
            {
                XNumberFormatsSupplier xNumberFormatsSupplier = (XNumberFormatsSupplier) AnyConverter.toObject(XNumberFormatsSupplier.class, xDataSourcePropertySet.getPropertyValue("NumberFormatsSupplier"));
                //TODO get the locale from the datasource
                aLocale = Configuration.getOfficeLocale(xMSF);
                oNumberFormatter = new NumberFormatter(xMSF, xNumberFormatsSupplier, aLocale);
                lDateCorrection = oNumberFormatter.getNullDateCorrection();
            }
            catch (Exception e)
            {
                e.printStackTrace(System.out);
            }
        }
        return oNumberFormatter;
    }

    public long getNullDateCorrection()
    {
        if (lDateCorrection == INVALID)
        {
            if (oNumberFormatter == null)
            {
                oNumberFormatter = getNumberFormatter();
            }
            lDateCorrection = oNumberFormatter.getNullDateCorrection();
        }
        return lDateCorrection;
    }

    private void getInterfaces(XMultiServiceFactory xMSF)
    {
        try
        {
            this.xMSF = xMSF;
            xDatabaseContext = (XInterface) xMSF.createInstance("com.sun.star.sdb.DatabaseContext");
            xNameAccess = UnoRuntime.queryInterface( XNameAccess.class, xDatabaseContext );
            DataSourceNames = xNameAccess.getElementNames();
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void setCommandTypes()
    {
        int TableCount;
        int QueryCount;
        int CommandCount;
        int i;
        int a;
        TableCount = JavaTools.getArraylength(TableNames);
        QueryCount = JavaTools.getArraylength(QueryNames);
        CommandCount = TableCount + QueryCount;
        CommandTypes = new int[CommandCount];
        if (TableCount > 0)
        {
            for (i = 0; i < TableCount; i++)
            {
                CommandTypes[i] = com.sun.star.sdb.CommandType.TABLE;
            }
            a = i;
            for (i = 0; i < QueryCount; i++)
            {
                CommandTypes[a] = com.sun.star.sdb.CommandType.QUERY;
                a += 1;
            }
        }
    }

    public boolean hasTableByName(String _stablename)
    {
        return getTableNamesAsNameAccess().hasByName(_stablename);
    }

    @SuppressWarnings("unchecked")
    public void setTableByName(String _tableName)
    {
        CommandObject oTableObject = new CommandObject(_tableName, com.sun.star.sdb.CommandType.TABLE);
        this.CommandObjects.addElement(oTableObject);
    }

    public CommandObject getTableByName(String _tablename)
    {
        return getCommandByName(_tablename, com.sun.star.sdb.CommandType.TABLE);
    }

    public CommandObject getQueryByName(String _queryname)
    {
        return getCommandByName(_queryname, com.sun.star.sdb.CommandType.QUERY);
    }

    public CommandObject getCommandByName(String _commandname, int _commandtype)
    {
        CommandObject oCommand = null;
        for (int i = 0; i < CommandObjects.size(); i++)
        {
            oCommand = (CommandObject) CommandObjects.elementAt(i);
            if ((oCommand.Name.equals(_commandname)) && (oCommand.CommandType == _commandtype))
            {
                return oCommand;
            }
        }
        if (oCommand == null)
        {
            oCommand = new CommandObject(_commandname, _commandtype);
            CommandObjects.addElement(oCommand);
        }
        return oCommand;
    }

    public void setQueryByName(String _QueryName)
    {
        CommandObject oQueryObject = new CommandObject(_QueryName, com.sun.star.sdb.CommandType.QUERY);
        this.CommandObjects.addElement(oQueryObject);
    }

    public class CommandObject
    {

        private XNameAccess xColumns;
        private XPropertySet xPropertySet;
        private String Name;
        private int CommandType;

        public CommandObject(String _CommandName, int _CommandType)
        {
            try
            {
                Object oCommand;
                this.Name = _CommandName;
                this.CommandType = _CommandType;
                // if (getTableNamesAsNameAccess() == null)
                // {
                //     initCommandNames();
                // }
                if (CommandType == com.sun.star.sdb.CommandType.TABLE)
                {
                    oCommand = getTableNamesAsNameAccess().getByName(Name);
                }
                else
                {
                    oCommand = getQueryNamesAsNameAccess().getByName(Name);
                }
                XColumnsSupplier xCommandCols = UnoRuntime.queryInterface( XColumnsSupplier.class, oCommand );
                xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, oCommand );
// TODO: Performance leak getColumns() take very long.
                xColumns = UnoRuntime.queryInterface( XNameAccess.class, xCommandCols.getColumns() );
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
        }
        public XNameAccess getColumns()
        {
            return xColumns;
        }
        public String getName()
        {
            return Name;
        }
        public XPropertySet getPropertySet()
        {
            return xPropertySet;
        }
    }

    public boolean hasEscapeProcessing(XPropertySet _xQueryPropertySet)
    {
        boolean bHasEscapeProcessing = false;
        try
        {
            if (_xQueryPropertySet.getPropertySetInfo().hasPropertyByName("EscapeProcessing"))
            {
                bHasEscapeProcessing = AnyConverter.toBoolean(_xQueryPropertySet.getPropertyValue("EscapeProcessing"));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return bHasEscapeProcessing;
    }

    public XNameAccess getQueryNamesAsNameAccess()
    {
        XQueriesSupplier xDBQueries = UnoRuntime.queryInterface( XQueriesSupplier.class, DBConnection );
        xQueryNames = xDBQueries.getQueries();
        return xQueryNames;
    }

    public XNameAccess getTableNamesAsNameAccess()
    {
        XTablesSupplier xDBTables = UnoRuntime.queryInterface( XTablesSupplier.class, DBConnection );
        XNameAccess xTableNames = xDBTables.getTables();
        return xTableNames;
    }

    public String[] getQueryNames()
    {
        if (QueryNames != null)
        {
            if (QueryNames.length > 0)
            {
                return QueryNames;
            }
        }
        QueryNames = getQueryNamesAsNameAccess().getElementNames();
        return QueryNames;
    }

    public String[] getTableNames()
    {
        if (TableNames != null)
        {
            if (TableNames.length > 0)
            {
                return TableNames;
            }
        }
        TableNames = getTableNamesAsNameAccess().getElementNames();
        return TableNames;
    }

    private void InitializeWidthList()
    {
        WidthList = new int[17][2];
        WidthList[0][0] = DataType.BIT; // ==  -7;
        WidthList[1][0] = DataType.BOOLEAN; // = 16
        WidthList[2][0] = DataType.TINYINT; // ==  -6;
        WidthList[3][0] = DataType.BIGINT; // ==  -5;
        WidthList[4][0] = DataType.LONGVARCHAR; // ==  -1;
        WidthList[5][0] = DataType.CHAR; // ==   1;
        WidthList[6][0] = DataType.NUMERIC; // ==   2;
        WidthList[7][0] = DataType.DECIMAL; // ==   3;  [mit Nachkommastellen]
        WidthList[8][0] = DataType.INTEGER; // ==   4;
        WidthList[9][0] = DataType.SMALLINT; // ==   5;
        WidthList[10][0] = DataType.FLOAT; // ==   6;
        WidthList[11][0] = DataType.REAL; // ==   7;
        WidthList[12][0] = DataType.DOUBLE; // ==   8;
        WidthList[13][0] = DataType.VARCHAR; // ==  12;
        WidthList[14][0] = DataType.DATE; // ==  91;
        WidthList[15][0] = DataType.TIME; // ==  92;
        WidthList[16][0] = DataType.TIMESTAMP; // ==  93;
        // NumericTypes are all types where aggregate functions can be performed on.
        // Similarly to a major competitor date/time/timmestamp fields are not included


    }

    public boolean isBinaryDataType(int _itype)
    {
        if (NumericTypes == null)
        {
            InitializeWidthList();
        }
        return (JavaTools.FieldInIntTable(BinaryTypes, _itype) > -1);
    }

    public int getMaxTablesInSelect()
    {
        try
        {
            int itablecount = xDBMetaData.getMaxTablesInSelect();
            if (itablecount == 0)
            {
                return DBMetaData.NOLIMIT;
            }
            else
            {
                return itablecount;
            }
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return - 1;
        }
    }

    public int getMaxColumnsInSelect()
    {
        return iMaxColumnsInSelect;
    }

    public int getMaxColumnsInGroupBy()
    {
        return iMaxColumnsInGroupBy;
    }

    private void setMaxColumnsInSelect() throws SQLException
    {
        iMaxColumnsInSelect = xDBMetaData.getMaxColumnsInSelect();
        if (iMaxColumnsInSelect == 0)
        {
            iMaxColumnsInSelect = DBMetaData.NOLIMIT;
        }
    }

    private void setMaxColumnsInGroupBy() throws SQLException
    {
        iMaxColumnsInGroupBy = xDBMetaData.getMaxColumnsInGroupBy();
        if (iMaxColumnsInGroupBy == 0)
        {
            iMaxColumnsInGroupBy = DBMetaData.NOLIMIT;
        }
    }

    public int getMaxColumnsInTable() throws SQLException
    {
        iMaxColumnsInTable = xDBMetaData.getMaxColumnsInTable();
        if (iMaxColumnsInTable == 0)
        {
            iMaxColumnsInTable = DBMetaData.NOLIMIT;
        }
        return iMaxColumnsInTable;
    }

    private void getDataSourceObjects() throws Exception
    {
        try
        {
            xDBMetaData = DBConnection.getMetaData();
            getDataSourceInterfaces();
            setMaxColumnsInGroupBy();
            setMaxColumnsInSelect();
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
    }

    private void ensureDataSourceSettings() throws UnknownPropertyException, WrappedTargetException
    {
        if ( m_dataSourceSettings != null )
            return;

        XPropertySet dataSourceProperties = UnoRuntime.queryInterface( XPropertySet.class, getDataSource() );
        m_dataSourceSettings = UnoRuntime.queryInterface( XPropertySet.class, dataSourceProperties.getPropertyValue( "Settings" ) );
    }

    public boolean isSQL92CheckEnabled()
    {
        boolean isSQL92CheckEnabled = false;
        try
        {
            ensureDataSourceSettings();
            isSQL92CheckEnabled = AnyConverter.toBoolean( m_dataSourceSettings.getPropertyValue( "EnableSQL92Check" ) );
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return isSQL92CheckEnabled;
    }

    public String verifyName(String _sname, int _maxlen)
    {
        if (_sname.length() > _maxlen)
        {
            return _sname.substring(0, _maxlen);
        }
        if (this.isSQL92CheckEnabled())
        {
            return Desktop.removeSpecialCharacters(xMSF, Configuration.getOfficeLocale(xMSF), _sname);
        }
        return _sname;
    }

    public XDataSource getDataSource()
    {
        if (m_dataSource == null)
        {
            try
            {
                    Object oDataSource = xNameAccess.getByName(DataSourceName);
                    m_dataSource = UnoRuntime.queryInterface( XDataSource.class, oDataSource );
            }
            catch (com.sun.star.container.NoSuchElementException e)
            {
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
            }
        }
        return m_dataSource;
    }

    private void setDataSourceByName(String _DataSourceName)
    {
        try
        {
            this.DataSourceName = _DataSourceName;
            getDataSourceInterfaces();
            XDocumentDataSource xDocu = UnoRuntime.queryInterface( XDocumentDataSource.class, getDataSource() );
            if (xDocu != null)
            {
                xModel = xDocu.getDatabaseDocument();
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }

    public void getDataSourceInterfaces() throws Exception
    {
        xDataSourcePropertySet = UnoRuntime.queryInterface( XPropertySet.class, getDataSource() );
        bPasswordIsRequired = ((Boolean) xDataSourcePropertySet.getPropertyValue("IsPasswordRequired")).booleanValue();
    }

    public boolean getConnection(PropertyValue[] curproperties)
    {
        try
        {
            XConnection xConnection = null;
            if (Properties.hasPropertyValue(curproperties, "ActiveConnection"))
            {
                xConnection = UnoRuntime.queryInterface( XConnection.class, Properties.getPropertyValue( curproperties, "ActiveConnection" ) );
                if (xConnection != null)
                {
                    com.sun.star.container.XChild child = UnoRuntime.queryInterface( com.sun.star.container.XChild.class, xConnection );

                    m_dataSource = UnoRuntime.queryInterface( XDataSource.class, child.getParent() );
                    XDocumentDataSource xDocu = UnoRuntime.queryInterface( XDocumentDataSource.class, m_dataSource );
                    if (xDocu != null)
                    {
                        xModel = xDocu.getDatabaseDocument();
                    }
                    XPropertySet xPSet = UnoRuntime.queryInterface( XPropertySet.class, m_dataSource );
                    if (xPSet != null)
                    {
                        DataSourceName = AnyConverter.toString(xPSet.getPropertyValue(PropertyNames.PROPERTY_NAME));
                    }
                    return getConnection(xConnection);
                }
                else
                {
                    bdisposeConnection = true;
                }
            }
            else
            {
                bdisposeConnection = true;
            }
            if (Properties.hasPropertyValue(curproperties, "DataSourceName"))
            {
                String sDataSourceName = AnyConverter.toString(Properties.getPropertyValue(curproperties, "DataSourceName"));
                return getConnection(sDataSourceName);
            }
            else if (Properties.hasPropertyValue(curproperties, "DataSource"))
            {
                m_dataSource = UnoRuntime.queryInterface( XDataSource.class, Properties.getPropertyValue( curproperties, "DataSource" ) );
                XDocumentDataSource xDocu = UnoRuntime.queryInterface( XDocumentDataSource.class, this.m_dataSource );
                if (xDocu != null)
                {
                    xModel = xDocu.getDatabaseDocument();
                }
                return getConnection(m_dataSource);
            }
            if (Properties.hasPropertyValue(curproperties, "DatabaseLocation"))
            {
                String sDataSourceName = AnyConverter.toString(Properties.getPropertyValue(curproperties, "DatabaseLocation"));
                return getConnection(sDataSourceName);
            }
            else if (xConnection != null)
            {
                bdisposeConnection = false;
                return getConnection(xConnection);
            }
        }
        catch (IllegalArgumentException e)
        {
            e.printStackTrace(System.out);
        }
        catch (UnknownPropertyException e)
        {
            e.printStackTrace(System.out);
        }
        catch (WrappedTargetException e)
        {
            e.printStackTrace(System.out);
        }

        return false;
    }

    private boolean getConnection(String _DataSourceName)
    {
        setDataSourceByName(_DataSourceName);
         return getConnection( getDataSource() );
    }

    private boolean getConnection(com.sun.star.sdbc.XConnection _DBConnection)
    {
        try
        {
            this.DBConnection = _DBConnection;
            this.m_connectionTools = UnoRuntime.queryInterface( XConnectionTools.class, this.DBConnection );
            getDataSourceObjects();
            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return false;
        }
    }

    private boolean getConnection(XDataSource _dataSource)
    {
        Resource oResource = new Resource(xMSF, "Database", "dbw");
        try
        {
            int iMsg = 0;
            boolean bgetConnection = false;
            if (DBConnection != null)
            {
                xConnectionComponent.dispose();
            }
            getDataSourceInterfaces();
            if (bPasswordIsRequired == false)
            {
                DBConnection = _dataSource.getConnection("", "");
                bgetConnection = true;
            }
            else
            {
                XInteractionHandler xInteractionHandler = UnoRuntime.queryInterface( XInteractionHandler.class, xMSF.createInstance("com.sun.star.task.InteractionHandler") );
                boolean bExitLoop = true;
                do
                {
                    XCompletedConnection xCompleted2 = UnoRuntime.queryInterface( XCompletedConnection.class, _dataSource );
                    try
                    {
                        DBConnection = xCompleted2.connectWithCompletion( xInteractionHandler );
                        bgetConnection = DBConnection != null;
                        if (bgetConnection == false)
                        {
                            bExitLoop = true;
                        }
                    }
                    catch (Exception exception)
                    {
                        // Note:  WindowAttributes from toolkit/source/awt/vclxtoolkit.cxx
                        String sMsgNoConnection = oResource.getResText(RID_DB_COMMON + 14);
                        iMsg = showMessageBox("QueryBox", VclWindowPeerAttribute.RETRY_CANCEL, sMsgNoConnection);
                        bExitLoop = iMsg == 0;
                        bgetConnection = false;
                    }
                }
                while (bExitLoop == false);
            }
            if (bgetConnection == false)
            {
                String sMsgConnectionImpossible = oResource.getResText(RID_DB_COMMON + 35);
                showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible);
            }
            else
            {
                xConnectionComponent = UnoRuntime.queryInterface( XComponent.class, DBConnection );
                m_connectionTools = UnoRuntime.queryInterface( XConnectionTools.class, DBConnection );
                getDataSourceObjects();
            }
            return bgetConnection;
        }
        catch (Exception exception)
        {
            String sMsgConnectionImpossible = oResource.getResText(RID_DB_COMMON + 35);
            showMessageBox("ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible);
            exception.printStackTrace(System.out);
            return false;
        }
    }

    public int getMaxColumnNameLength()
    {
        try
        {
            if (iMaxColumnNameLength <= 0)
            {
                iMaxColumnNameLength = xDBMetaData.getMaxColumnNameLength();
            }
            return iMaxColumnNameLength;
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return 0;
        }
    }

    public int getMaxTableNameLength()
    {
        try
        {
            if (iMaxTableNameLength <= 0)
            {
                iMaxTableNameLength = xDBMetaData.getMaxTableNameLength();
            }
            return iMaxTableNameLength;
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return 0;
        }
    }

    public boolean supportsPrimaryKeys()
    {
        boolean supportsPrimaryKeys = false;
        try
        {
            ensureDataSourceSettings();
            Any primaryKeySupport = (Any)m_dataSourceSettings.getPropertyValue( "PrimaryKeySupport" );
            if ( AnyConverter.isVoid( primaryKeySupport ) )
                supportsPrimaryKeys = supportsCoreSQLGrammar();
            else
                supportsPrimaryKeys = AnyConverter.toBoolean( primaryKeySupport );
        }
        catch ( Exception ex )
        {
            Logger.getLogger( DBMetaData.class.getName() ).log( Level.SEVERE, null, ex );
        }
        return supportsPrimaryKeys;
    }

    public boolean supportsCoreSQLGrammar()
    {
        try
        {
            return xDBMetaData.supportsCoreSQLGrammar();
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return false;
        }
    }

    public boolean supportsAutoIncrementation()
    {
        return false;
    }

    public boolean supportsQueriesInFrom()
    {
        return m_connectionTools.getDataSourceMetaData().supportsQueriesInFrom();
    }

    public String suggestName( final int i_objectType, final String i_baseName ) throws IllegalArgumentException
    {
        return m_connectionTools.getObjectNames().suggestName( i_objectType, i_baseName );
    }

    /**
     * inserts a Query to a datasource; There is no validation if the queryname is already existing in the datasource
     * @param oQuery
     * @param QueryName
     */
    public boolean createQuery(SQLQueryComposer _oSQLQueryComposer, String _QueryName)
    {
        try
        {
            XQueryDefinitionsSupplier xQueryDefinitionsSuppl = UnoRuntime.queryInterface( XQueryDefinitionsSupplier.class, m_dataSource );
            XNameAccess xQueryDefs = xQueryDefinitionsSuppl.getQueryDefinitions();
            XSingleServiceFactory xSSFQueryDefs = UnoRuntime.queryInterface( XSingleServiceFactory.class, xQueryDefs );
            Object oQuery = xSSFQueryDefs.createInstance(); //"com.sun.star.sdb.QueryDefinition"
            XPropertySet xPSet = UnoRuntime.queryInterface( XPropertySet.class, oQuery );

            String s = _oSQLQueryComposer.m_xQueryAnalyzer.getQuery();
            xPSet.setPropertyValue("Command", s);

            XNameContainer xNameCont = UnoRuntime.queryInterface( XNameContainer.class, xQueryDefs );
            m_connectionTools.getObjectNames().checkNameForCreate(com.sun.star.sdb.CommandType.QUERY, _QueryName);
            xNameCont.insertByName(_QueryName, oQuery);
            return true;
        }
        catch (WrappedTargetException exception)
        {
            SQLException sqlError = null;
            try
            {
                sqlError = (SQLException) exception.TargetException;
            }
            catch (ClassCastException castError)
            {
            }

            if (sqlError != null)
            {
                callSQLErrorMessageDialog(sqlError, null);
                return false;
            }
            exception.printStackTrace(System.out);
        }
        catch (SQLException exception)
        {
            callSQLErrorMessageDialog(exception, null);
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
        return false;
    }

    public void dispose()
    {
        if ((DBConnection != null) && (this.bdisposeConnection))
        {
            xConnectionComponent.dispose();
        }
    }

    public XHierarchicalNameAccess getReportDocuments()
    {
        XReportDocumentsSupplier xReportDocumentSuppl = UnoRuntime.queryInterface( XReportDocumentsSupplier.class, this.xModel );
        xReportDocumentSuppl.getReportDocuments();
        XHierarchicalNameAccess xReportHier = UnoRuntime.queryInterface( XHierarchicalNameAccess.class, xReportDocumentSuppl.getReportDocuments() );
        return xReportHier;
    }

    public XHierarchicalNameAccess getFormDocuments()
    {
        XFormDocumentsSupplier xFormDocumentSuppl = UnoRuntime.queryInterface( XFormDocumentsSupplier.class, xModel );
        XHierarchicalNameAccess xFormHier = UnoRuntime.queryInterface( XHierarchicalNameAccess.class, xFormDocumentSuppl.getFormDocuments() );
        return xFormHier;
    }

    public boolean hasFormDocumentByName(String _sFormName)
    {
        XFormDocumentsSupplier xFormDocumentSuppl = UnoRuntime.queryInterface( XFormDocumentsSupplier.class, xModel );
        XNameAccess xFormNameAccess = UnoRuntime.queryInterface( XNameAccess.class, xFormDocumentSuppl.getFormDocuments() );
        return xFormNameAccess.hasByName(_sFormName);
    }

    public void addFormDocument(XComponent _xComponent)
    {
        XHierarchicalNameAccess _xFormDocNameAccess = getFormDocuments();
        addDatabaseDocument(_xComponent, _xFormDocNameAccess, false);
    }

    public void addReportDocument(XComponent _xComponent, boolean _bcreatedynamicreport)
    {
        XHierarchicalNameAccess xReportDocNameAccess = getReportDocuments();
        addDatabaseDocument(_xComponent, xReportDocNameAccess, _bcreatedynamicreport);
    }

    /**
     * adds the passed document as a report or a form to the database. Afterwards the document is deleted.
     * the document may not be open
     * @param _xComponent
     * @param _xDocNameAccess
     * @param _bcreateTemplate  describes the type of the document: "form" or "report"
     */
    public void addDatabaseDocument(XComponent _xComponent, XHierarchicalNameAccess _xDocNameAccess, boolean i_createTemplate)
    {
        try
        {
            XModel xDocumentModel = UnoRuntime.queryInterface( XModel.class, _xComponent );
            String documentURL = xDocumentModel.getURL();
            String basename = FileAccess.getBasename(documentURL, "/");
            XCloseable xCloseable = UnoRuntime.queryInterface( XCloseable.class, _xComponent );
            xCloseable.close(false);

            NamedValueCollection creationArgs = new NamedValueCollection();
            creationArgs.put( PropertyNames.PROPERTY_NAME, basename );
            creationArgs.put( "URL", documentURL );
            creationArgs.put( "AsTemplate", i_createTemplate );
            XMultiServiceFactory xDocMSF = UnoRuntime.queryInterface( XMultiServiceFactory.class, _xDocNameAccess );
            Object oDBDocument = xDocMSF.createInstanceWithArguments( "com.sun.star.sdb.DocumentDefinition", creationArgs.getPropertyValues() );
            XHierarchicalNameContainer xHier = UnoRuntime.queryInterface( XHierarchicalNameContainer.class, _xDocNameAccess );
            String sdocname = Desktop.getUniqueName(_xDocNameAccess, basename);
            xHier.insertByHierarchicalName(sdocname, oDBDocument);
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface( XSimpleFileAccess.class, xInterface );
            xSimpleFileAccess.kill(documentURL);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void createTypeInspector() throws SQLException
    {
        oTypeInspector = new TypeInspector(xDBMetaData.getTypeInfo());
    }

    public TypeInspector getDBDataTypeInspector()
    {
        return oTypeInspector;
    }

    private String[] StringsFromResultSet(XResultSet _xResultSet, int _icol)
    {
        String[] sColValues = null;
        try
        {
            XRow xRow = UnoRuntime.queryInterface( XRow.class, _xResultSet );
            Vector aColVector = new Vector();
            while (_xResultSet.next())
            {
                aColVector.addElement(xRow.getString(_icol));
            }
            sColValues = new String[aColVector.size()];
            aColVector.toArray(sColValues);
        }
        catch (SQLException e)
        {
            e.printStackTrace();
        }
        return sColValues;
    }

    public String[] getCatalogNames()
    {
        try
        {
            XResultSet xResultSet = xDBMetaData.getCatalogs();
            return StringsFromResultSet(xResultSet, 1);
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return null;
        }
    }

    public String[] getSchemaNames()
    {
        try
        {
            XResultSet xResultSet = xDBMetaData.getSchemas();
            return StringsFromResultSet(xResultSet, 1);
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
            return null;
        }
    }

    public boolean storeDatabaseDocumentToTempPath(XComponent _xcomponent, String _storename)
    {
        try
        {
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = UnoRuntime.queryInterface( XSimpleFileAccess.class, xInterface );
            String storepath = FileAccess.getOfficePath(xMSF, "Temp", xSimpleFileAccess) + "/" + _storename;
            XStorable xStoreable = UnoRuntime.queryInterface( XStorable.class, _xcomponent );
            PropertyValue[] oStoreProperties = new PropertyValue[1];
            oStoreProperties[0] = Properties.createProperty("FilterName", "writer8");
            storepath += ".odt";
            xStoreable.storeAsURL(storepath, oStoreProperties);
            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return false;
        }
    }

    public int showMessageBox(String windowServiceName, int windowAttribute, String MessageText)
    {
        if (getWindowPeer() != null)
        {
            return SystemDialog.showMessageBox(xMSF, xWindowPeer, windowServiceName, windowAttribute, MessageText);
        }
        else
        {
            return SystemDialog.showMessageBox(xMSF, windowServiceName, windowAttribute, MessageText);
        }
    }

    /**
     * @return Returns the xWindowPeer.
     */
    public XWindowPeer getWindowPeer()
    {
        return xWindowPeer;
    }

    /**
     * @param windowPeer The xWindowPeer to set.
     * Should be called as soon as a Windowpeer of a wizard dialog is available
     * The windowpeer is needed to call a Messagebox
     */
    public void setWindowPeer(XWindowPeer windowPeer)
    {
        xWindowPeer = windowPeer;
    }

    public void callSQLErrorMessageDialog(SQLException oSQLException, XWindow _xWindow)
    {
        try
        {
            Object oDialog = xMSF.createInstance("com.sun.star.sdb.ErrorMessageDialog");
            XInitialization xInitialization = UnoRuntime.queryInterface( XInitialization.class, oDialog );
            PropertyValue[] aPropertyValue = new PropertyValue[2];
            aPropertyValue[0] = Properties.createProperty("SQLException", oSQLException);
            aPropertyValue[1] = Properties.createProperty("ParentWindow", _xWindow);
            xInitialization.initialize(aPropertyValue);
            XExecutableDialog xExecutableDialog = UnoRuntime.queryInterface( XExecutableDialog.class, oDialog );
            xExecutableDialog.execute();
        }
        catch (com.sun.star.uno.Exception ex)
        {
            Logger.getLogger( getClass().getName() ).log( Level.SEVERE, "error calling the error dialog", ex );
        }
    }

    public void finish()
    {
        xQueryNames = null;
        xNameAccess = null;
        xDatabaseContext = null;
        xDBMetaData = null;
        m_dataSource = null;
        xModel = null;
        xDataSourcePropertySet = null;
        xWindowPeer = null;
        DBConnection = null;
        m_connectionTools = null;
        xMSF = null;
        xConnectionComponent = null;
        CommandObjects = null;
    }
}
