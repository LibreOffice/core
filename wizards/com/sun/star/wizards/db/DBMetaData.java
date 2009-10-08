/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DBMetaData.java,v $
 * $Revision: 1.20 $
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
import java.util.*;

// import com.sun.star.io.IOException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XChild;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XHierarchicalNameContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
// import com.sun.star.container.XNamed;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.tools.XConnectionTools;
// import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XColumnsSupplier;

import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XNumberFormatsSupplier;

import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.*;
// import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.sdb.XFormDocumentsSupplier;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.sdb.XReportDocumentsSupplier;
// import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdb.XCompletedConnection;
import com.sun.star.lang.Locale;
// import com.sun.star.util.XFlushable;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbcx.XTablesSupplier;

public class DBMetaData
{

    public XNameAccess xTableNames;
    public XNameAccess xQueryNames;
    private XInteractionHandler oInteractionHandler;
    private XNameAccess xNameAccess;
    private XInterface xDatabaseContext;
    public XDatabaseMetaData xDBMetaData;
    public XDataSource xDataSource;
    public XOfficeDatabaseDocument xModel;
    private XCompletedConnection xCompleted;
    public XPropertySet xDataSourcePropertySet;
    // private int[] nDataTypes = null;
    private XWindowPeer xWindowPeer;
    public String[] DataSourceNames;
    public String[] CommandNames;
    public String[] TableNames = new String[]
    {
    };
    public String[] QueryNames = new String[]
    {
    };
    public java.util.Vector CommandObjects = new Vector(1);
    public int[][] WidthList;
    public int[] NumericTypes;
    public int[] BinaryTypes;
    public Locale aLocale;
    public int[] CommandTypes;
    public String DataSourceName;
    public com.sun.star.sdbc.XConnection DBConnection;
    public com.sun.star.sdb.tools.XConnectionTools ConnectionTools;
    public com.sun.star.lang.XMultiServiceFactory xMSF;
    public XComponent xConnectionComponent;
    public SQLQueryComposer oSQLQueryComposer;
    int iMaxColumnsInSelect;
    int iMaxColumnsInGroupBy;
    int iMaxColumnsInTable;
    int iMaxColumnNameLength = -1;
    int iMaxTableNameLength = -1;
    private boolean bPasswordIsRequired;
    // private boolean bFormatKeysareset = false;
    final int NOLIMIT = 9999999;
    final int RID_DB_COMMON = 1000;
    final int INVALID = 9999999;
    public TypeInspector oTypeInspector;
    private PropertyValue[] aInfoPropertyValues = null;
    private boolean bisSQL92CheckEnabled = false;
    private NumberFormatter oNumberFormatter = null;
    private long lDateCorrection = INVALID;
    private boolean bdisposeConnection = false;

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

    void getInterfaces(XMultiServiceFactory xMSF)
    {
        try
        {
            this.xMSF = xMSF;
            xDatabaseContext = (XInterface) xMSF.createInstance("com.sun.star.sdb.DatabaseContext");
            xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDatabaseContext);
            XInterface xInteractionHandler = (XInterface) xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
            oInteractionHandler = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, xInteractionHandler);
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
        getTableNames();
        return xTableNames.hasByName(_stablename);
    }

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

        public XNameAccess xColumns;
        public XPropertySet xPropertySet;
        public String Name;
        public int CommandType;

        public CommandObject(String _CommandName, int _CommandType)
        {
            try
            {
                Object oCommand;
                this.Name = _CommandName;
                this.CommandType = _CommandType;
                if (xTableNames == null)
                {
                    setCommandNames();
                }
                if (CommandType == com.sun.star.sdb.CommandType.TABLE)
                {
                    oCommand = xTableNames.getByName(Name);
                }
                else
                {
                    oCommand = xQueryNames.getByName(Name);
                }
                XColumnsSupplier xCommandCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, oCommand);
                xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oCommand);
                xColumns = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xCommandCols.getColumns());
            }
            catch (Exception exception)
            {
                exception.printStackTrace(System.out);
            }
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

    public void setCommandNames()
    {
        getTableNames();
        XQueriesSupplier xDBQueries = (XQueriesSupplier) UnoRuntime.queryInterface(XQueriesSupplier.class, DBConnection);
        xQueryNames = (XNameAccess) xDBQueries.getQueries();
        QueryNames = xQueryNames.getElementNames();

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
        XTablesSupplier xDBTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, DBConnection);
        xTableNames = (XNameAccess) xDBTables.getTables();
        TableNames = (String[]) xTableNames.getElementNames();
        return TableNames;
    }

    void InitializeWidthList()
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
        NumericTypes = new int[9];
        NumericTypes[0] = DataType.TINYINT; // ==  -6;
        NumericTypes[1] = DataType.BIGINT; // ==  -5
        NumericTypes[2] = DataType.NUMERIC; // ==  - 2
        NumericTypes[3] = DataType.DECIMAL; // ==   3;
        NumericTypes[4] = DataType.INTEGER; // ==   4;
        NumericTypes[5] = DataType.SMALLINT; // ==   5;
        NumericTypes[6] = DataType.FLOAT; // ==   6;
        NumericTypes[7] = DataType.REAL; // ==   7;
        NumericTypes[8] = DataType.DOUBLE; // ==   8;

        BinaryTypes = new int[12];
        BinaryTypes[0] = DataType.BINARY;
        BinaryTypes[1] = DataType.VARBINARY;
        BinaryTypes[2] = DataType.LONGVARBINARY;
        BinaryTypes[3] = DataType.BLOB;
        BinaryTypes[4] = DataType.SQLNULL;
        BinaryTypes[5] = DataType.OBJECT;
        BinaryTypes[6] = DataType.DISTINCT;
        BinaryTypes[7] = DataType.STRUCT;
        BinaryTypes[8] = DataType.ARRAY;
        BinaryTypes[9] = DataType.CLOB;
        BinaryTypes[10] = DataType.REF;
        BinaryTypes[11] = DataType.OTHER;
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
                return this.NOLIMIT;
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
            iMaxColumnsInSelect = this.NOLIMIT;
        }
    }

    private void setMaxColumnsInGroupBy() throws SQLException
    {
        iMaxColumnsInGroupBy = xDBMetaData.getMaxColumnsInGroupBy();
        if (iMaxColumnsInGroupBy == 0)
        {
            iMaxColumnsInGroupBy = this.NOLIMIT;
        }
    }

    public int getMaxColumnsInTable() throws SQLException
    {
        iMaxColumnsInTable = xDBMetaData.getMaxColumnsInTable();
        if (iMaxColumnsInTable == 0)
        {
            iMaxColumnsInTable = this.NOLIMIT;
        }
        return iMaxColumnsInTable;
    }

    private void getDataSourceObjects() throws Exception
    {
        try
        {
            xDBMetaData = DBConnection.getMetaData();
            XChild xChild = (XChild) UnoRuntime.queryInterface(XChild.class, DBConnection);
            Object oDataSource = xChild.getParent();
            getDataSourceInterfaces();
            setMaxColumnsInGroupBy();
            setMaxColumnsInSelect();
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
    }

    public boolean isSQL92CheckEnabled()
    {
        try
        {
            if (aInfoPropertyValues == null)
            {
                aInfoPropertyValues = (PropertyValue[]) AnyConverter.toArray(this.xDataSourcePropertySet.getPropertyValue("Info"));
                if (Properties.hasPropertyValue(aInfoPropertyValues, "EnableSQL92Check"))
                {
                    bisSQL92CheckEnabled = AnyConverter.toBoolean(Properties.getPropertyValue(aInfoPropertyValues, "EnableSQL92Check"));
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return bisSQL92CheckEnabled;
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

    private void setDataSourceByName(String _DataSourceName, boolean bgetInterfaces)
    {
        try
        {
            this.DataSourceName = _DataSourceName;
            Object oDataSource = xNameAccess.getByName(DataSourceName);
            xDataSource = (XDataSource) UnoRuntime.queryInterface(XDataSource.class, oDataSource);
            getDataSourceInterfaces();
            XDocumentDataSource xDocu = (XDocumentDataSource) UnoRuntime.queryInterface(XDocumentDataSource.class, this.xDataSource);
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
        xCompleted = (XCompletedConnection) UnoRuntime.queryInterface(XCompletedConnection.class, xDataSource);
        xDataSourcePropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDataSource);
        bPasswordIsRequired = ((Boolean) xDataSourcePropertySet.getPropertyValue("IsPasswordRequired")).booleanValue();
    }

    public boolean getConnection(PropertyValue[] curproperties)
    {
        try
        {
            com.sun.star.sdbc.XConnection xConnection = null;
            if (Properties.hasPropertyValue(curproperties, "ActiveConnection"))
            {
                xConnection = (com.sun.star.sdbc.XConnection) AnyConverter.toObject(com.sun.star.sdbc.XConnection.class,
                        Properties.getPropertyValue(curproperties, "ActiveConnection"));
                if (xConnection != null)
                {
                    com.sun.star.container.XChild child = (com.sun.star.container.XChild) UnoRuntime.queryInterface(com.sun.star.container.XChild.class, xConnection);

                    xDataSource = (XDataSource) UnoRuntime.queryInterface(XDataSource.class, child.getParent());
                    XDocumentDataSource xDocu = (XDocumentDataSource) UnoRuntime.queryInterface(XDocumentDataSource.class, this.xDataSource);
                    if (xDocu != null)
                    {
                        xModel = xDocu.getDatabaseDocument();
                    }
                    XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDataSource);
                    if (xPSet != null)
                    {
                        DataSourceName = AnyConverter.toString(xPSet.getPropertyValue("Name"));
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
                xDataSource = (XDataSource) UnoRuntime.queryInterface(XDataSource.class, Properties.getPropertyValue(curproperties, "DataSource"));
                XDocumentDataSource xDocu = (XDocumentDataSource) UnoRuntime.queryInterface(XDocumentDataSource.class, this.xDataSource);
                if (xDocu != null)
                {
                    xModel = xDocu.getDatabaseDocument();
                }
                return getConnection(xDataSource);
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
        setDataSourceByName(_DataSourceName, true);
        return getConnection(xDataSource);
    }

    private boolean getConnection(com.sun.star.sdbc.XConnection _DBConnection)
    {
        try
        {
            this.DBConnection = _DBConnection;
            this.ConnectionTools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, this.DBConnection);
            getDataSourceObjects();
            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
            return false;
        }
    }

    private boolean getConnection(XDataSource xDataSource)
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
                DBConnection = xDataSource.getConnection("", "");
                bgetConnection = true;
            }
            else
            {
                XInterface xInteractionHandler = (XInterface) xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
                XInteractionHandler oInteractionHandler2 = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, xInteractionHandler);
                boolean bExitLoop = true;
                do
                {
                    XCompletedConnection xCompleted2 = (XCompletedConnection) UnoRuntime.queryInterface(XCompletedConnection.class, xDataSource);
                    try
                    {
                        DBConnection = xCompleted2.connectWithCompletion(oInteractionHandler2);
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
                xConnectionComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, DBConnection);
                ConnectionTools = (XConnectionTools) UnoRuntime.queryInterface(XConnectionTools.class, DBConnection);
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

    /**
     * inserts a Query to a datasource; There is no validation if the queryname is already existing in the datasource
     * @param oQuery
     * @param QueryName
     */
    public boolean createQuery(SQLQueryComposer _oSQLQueryComposer, String _QueryName)
    {
        try
        {
            XQueryDefinitionsSupplier xQueryDefinitionsSuppl = (XQueryDefinitionsSupplier) UnoRuntime.queryInterface(XQueryDefinitionsSupplier.class, xDataSource);
            XNameAccess xQueryDefs = xQueryDefinitionsSuppl.getQueryDefinitions();
            XSingleServiceFactory xSSFQueryDefs = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, xQueryDefs);
            Object oQuery = xSSFQueryDefs.createInstance(); //"com.sun.star.sdb.QueryDefinition"
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oQuery);

            String s = _oSQLQueryComposer.m_xQueryAnalyzer.getQuery();
            xPSet.setPropertyValue("Command", s);

            XNameContainer xNameCont = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xQueryDefs);
            XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xQueryDefs);
            ConnectionTools.getObjectNames().checkNameForCreate(com.sun.star.sdb.CommandType.QUERY, _QueryName);
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
        XReportDocumentsSupplier xReportDocumentSuppl = (XReportDocumentsSupplier) UnoRuntime.queryInterface(XReportDocumentsSupplier.class, this.xModel);
        xReportDocumentSuppl.getReportDocuments();
        XHierarchicalNameAccess xReportHier = (XHierarchicalNameAccess) UnoRuntime.queryInterface(XHierarchicalNameAccess.class, xReportDocumentSuppl.getReportDocuments());
        return xReportHier;
    }

    public XHierarchicalNameAccess getFormDocuments()
    {
        XFormDocumentsSupplier xFormDocumentSuppl = (XFormDocumentsSupplier) UnoRuntime.queryInterface(XFormDocumentsSupplier.class, xModel);
        XHierarchicalNameAccess xFormHier = (XHierarchicalNameAccess) UnoRuntime.queryInterface(XHierarchicalNameAccess.class, xFormDocumentSuppl.getFormDocuments());
        return xFormHier;
    }

    public boolean hasFormDocumentByName(String _sFormName)
    {
        XFormDocumentsSupplier xFormDocumentSuppl = (XFormDocumentsSupplier) UnoRuntime.queryInterface(XFormDocumentsSupplier.class, xModel);
        XNameAccess xFormNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xFormDocumentSuppl.getFormDocuments());
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
    public void addDatabaseDocument(XComponent _xComponent, XHierarchicalNameAccess _xDocNameAccess, boolean _bcreateTemplate)
    {
        try
        {
            PropertyValue[] aDocProperties;
            XModel xDocumentModel = (XModel) UnoRuntime.queryInterface(XModel.class, _xComponent);
            String sPath = xDocumentModel.getURL();
            String basename = FileAccess.getBasename(sPath, "/");
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, _xComponent);
            _xComponent.dispose();
            xCloseable.close(false);
            if (_bcreateTemplate)
            {
                aDocProperties = new PropertyValue[5];
            }
            else
            {
                aDocProperties = new PropertyValue[4];
            }
            aDocProperties[0] = Properties.createProperty("Name", basename);
            aDocProperties[1] = Properties.createProperty("Parent", _xDocNameAccess);
            aDocProperties[2] = Properties.createProperty("URL", sPath);
            aDocProperties[3] = Properties.createProperty("DocumentTitle", basename);
            if (_bcreateTemplate)
            {
                aDocProperties[4] = Properties.createProperty("AsTemplate", new Boolean(_bcreateTemplate));
            }
            XMultiServiceFactory xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, _xDocNameAccess);
            Object oDBDocument = xDocMSF.createInstanceWithArguments("com.sun.star.sdb.DocumentDefinition", aDocProperties);
            XHierarchicalNameContainer xHier = (XHierarchicalNameContainer) UnoRuntime.queryInterface(XHierarchicalNameContainer.class, _xDocNameAccess);
            String sdocname = Desktop.getUniqueName(_xDocNameAccess, basename);
            xHier.insertByHierarchicalName(sdocname, oDBDocument);
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
            xSimpleFileAccess.kill(sPath);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    public XComponent[] openDatabaseDocument(String _docname, boolean _bAsTemplate, boolean _bOpenInDesign, XHierarchicalNameAccess _xDocuments)
    {
        XComponent[] xRetComponent = new XComponent[2];
        try
        {
            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, _xDocuments);
            PropertyValue[] aPropertyValues = new PropertyValue[4];
            aPropertyValues[0] = Properties.createProperty("OpenMode", _bOpenInDesign ? "openDesign" : "open");
            aPropertyValues[1] = Properties.createProperty("ActiveConnection", this.DBConnection);
            aPropertyValues[2] = Properties.createProperty("DocumentTitle", _docname);
            aPropertyValues[3] = Properties.createProperty("AsTemplate", new Boolean(_bAsTemplate));
            XHierarchicalNameContainer xHier = (XHierarchicalNameContainer) UnoRuntime.queryInterface(XHierarchicalNameContainer.class, _xDocuments);
            if (xHier.hasByHierarchicalName(_docname))
            {
                xRetComponent[0] = (XComponent) UnoRuntime.queryInterface(XComponent.class, xHier.getByHierarchicalName(_docname));
                xRetComponent[1] = xComponentLoader.loadComponentFromURL(_docname, "", 0, aPropertyValues);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return xRetComponent;
    }

    public XComponent[] openFormDocument(String _sformname, boolean _bOpenInDesign)
    {
        XHierarchicalNameAccess xFormDocuments = getFormDocuments();
        return openDatabaseDocument(_sformname, false, _bOpenInDesign, xFormDocuments);
    }

    public XComponent[] openReportDocument(String _sreportname, boolean _bAsTemplate, boolean _bOpenInDesign)
    {
        XHierarchicalNameAccess xReportDocuments = getReportDocuments();
        return openDatabaseDocument(_sreportname, _bAsTemplate, _bOpenInDesign, xReportDocuments);
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
            XRow xRow = (XRow) UnoRuntime.queryInterface(XRow.class, _xResultSet);
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
            XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
            String storepath = FileAccess.getOfficePath(xMSF, "Temp", xSimpleFileAccess) + "/" + _storename;
            XStorable xStoreable = (XStorable) UnoRuntime.queryInterface(XStorable.class, _xcomponent);
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
            XInitialization xInitialization = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, oDialog);
            PropertyValue[] aPropertyValue = new PropertyValue[2];
            aPropertyValue[0] = Properties.createProperty("SQLException", oSQLException);
            aPropertyValue[1] = Properties.createProperty("ParentWindow", _xWindow);
            xInitialization.initialize(aPropertyValue);
            XExecutableDialog xExecutableDialog = (XExecutableDialog) UnoRuntime.queryInterface(XExecutableDialog.class, oDialog);
            xExecutableDialog.execute();
        }
        catch (com.sun.star.uno.Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public void finish()
    {
        xTableNames = null;
        xQueryNames = null;
        oInteractionHandler = null;
        xNameAccess = null;
        xDatabaseContext = null;
        xDBMetaData = null;
        xDataSource = null;
        xModel = null;
        xCompleted = null;
        xDataSourcePropertySet = null;
        xWindowPeer = null;
        DBConnection = null;
        ConnectionTools = null;
        xMSF = null;
        xConnectionComponent = null;
        oSQLQueryComposer = null;
        CommandObjects = null;
    }
}
