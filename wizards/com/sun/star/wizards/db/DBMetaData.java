/*************************************************************************
*
*  $RCSfile: DBMetaData.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: hr $ $Date: 2004-08-02 17:19:41 $
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

package com.sun.star.wizards.db;

import java.util.*;

import com.sun.star.io.IOException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.container.XChild;
import com.sun.star.container.XHierarchicalNameContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbcx.XColumnsSupplier;

import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.MalformedNumberFormatException;
import com.sun.star.util.NumberFormat;
import com.sun.star.util.XNumberFormatTypes;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormats;

import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.*;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.sdb.XFormDocumentsSupplier;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.sdb.XBookmarksSupplier;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdb.XCompletedConnection;
import com.sun.star.lang.Locale;
import com.sun.star.util.XFlushable;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.sdb.XReportDocumentsSupplier;

public class DBMetaData {
    public XNameAccess xTableNames;
    public XNumberFormatsSupplier xNumberFormatsSupplier;
    public XNameAccess xQueryNames;
    private XInteractionHandler oInteractionHandler;
    private XNameAccess xNameAccess;
    private XInterface xDatabaseContext;
    public XDatabaseMetaData xDBMetaData;
    private XBookmarksSupplier xBookmarksSuppl;
    private XDataSource xDataSource;
    private XCompletedConnection xCompleted;
    public XNumberFormats NumberFormats;
    private boolean bConnectionOvergiven = true;
    public String[] DataSourceNames;
    public String[] CommandNames;
    public String[] TableNames = new String[] {};
    public String[] QueryNames = new String[] {};
    public java.util.Vector CommandObjects = new Vector(1);
    public int[][] WidthList;
    public int[] NumericTypes;
    public Locale CharLocale;
    public int[] CommandTypes;
    public String DataSourceName;
    public com.sun.star.sdbc.XConnection DBConnection;
    public com.sun.star.lang.XMultiServiceFactory xMSF;
    public XComponent xComponent;
    public SQLQueryComposer oSQLQueryComposer;
    int iMaxColumnsInSelect;
    int iMaxColumnsInGroupBy;
    public int iDateFormatKey;
    public int iDateTimeFormatKey;
    public int iNumberFormatKey;
    public int iTextFormatKey;
    public int iTimeFormatKey;
    public int iLogicalFormatKey;
    public long lDateCorrection;
    private boolean bPasswordIsRequired;
    final int NOLIMIT = 9999999;
    final int RID_DB_COMMON = 1000;

    public DBMetaData(XMultiServiceFactory xMSF) {
        NumberFormats = null;
        getInterfaces(xMSF);
        InitializeWidthList();
    }

    public DBMetaData(XMultiServiceFactory xMSF, Locale CharLocale, XNumberFormats NumberFormats) {
        // Todo: getDatabaseContext
        this.NumberFormats = NumberFormats;
        this.CharLocale = CharLocale;
        getInterfaces(xMSF);
        InitializeWidthList();
        if (CharLocale != null) {
            setStandardFormatKeys(false);
        }
    }

    private void setStandardFormatKeys(boolean bgetStandardBool) {
        XNumberFormatTypes xNumberFormatTypes = (XNumberFormatTypes) UnoRuntime.queryInterface(XNumberFormatTypes.class, NumberFormats);
        if (!bgetStandardBool) {
            String FormatString = "[=1]" + '"' + (char)9745 + '"' + ";[=0]" + '"' + (char)58480 + '"' + ";";
            iLogicalFormatKey = NumberFormats.queryKey(FormatString, CharLocale, true);
            try {
                if (iLogicalFormatKey == -1)
                    iLogicalFormatKey = NumberFormats.addNew(FormatString, CharLocale);
            } catch (MalformedNumberFormatException e) {
                e.printStackTrace();
                iLogicalFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.LOGICAL, CharLocale);
            }
        } else
            iLogicalFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.LOGICAL, CharLocale);
        iDateFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.DATE, CharLocale);
        iDateTimeFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.DATETIME, CharLocale);
        iTimeFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.TIME, CharLocale);
        iNumberFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.NUMBER, CharLocale);
        iTextFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.TEXT, CharLocale);
    }

    void getInterfaces(XMultiServiceFactory xMSF) {
        try {
            this.xMSF = xMSF;
            xDatabaseContext = (XInterface) xMSF.createInstance("com.sun.star.sdb.DatabaseContext");
            xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xDatabaseContext);
            XInterface xInteractionHandler = (XInterface) xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
            oInteractionHandler = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, xInteractionHandler);
            DataSourceNames = xNameAccess.getElementNames();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void setCommandTypes() {
        int TableCount;
        int QueryCount;
        int CommandCount;
        int i;
        int a;
        TableCount = JavaTools.getArraylength(TableNames);
        QueryCount = JavaTools.getArraylength(QueryNames);
        CommandCount = TableCount + QueryCount;
        CommandTypes = new int[CommandCount];
        if (TableCount > 0) {
            for (i = 0; i < TableCount; i++) {
                CommandTypes[i] = com.sun.star.sdb.CommandType.TABLE;
            }
            a = i;
            for (i = 0; i < QueryCount; i++) {
                CommandTypes[a] = com.sun.star.sdb.CommandType.QUERY;
                a += 1;
            }
        }
    }


    public void setTableByName(String _tableName) {
        CommandObject oTableObject = new CommandObject(_tableName, com.sun.star.sdb.CommandType.TABLE);
        this.CommandObjects.addElement(oTableObject);
    }

    public CommandObject getTableByName(String _tablename) {
        return getCommandByName(_tablename, com.sun.star.sdb.CommandType.TABLE);
    }

    public CommandObject getQueryByName(String _queryname) {
        return getCommandByName(_queryname, com.sun.star.sdb.CommandType.QUERY);
    }

    public CommandObject getCommandByName(String _commandname, int _commandtype) {
        CommandObject oCommand = null;
        for (int i = 0; i < CommandObjects.size(); i++) {
            oCommand = (CommandObject) CommandObjects.elementAt(i);
            if ((oCommand.Name.equals(_commandname)) && (oCommand.CommandType == _commandtype))
                return oCommand;
        }
        if (oCommand == null){
            oCommand = new CommandObject(_commandname, _commandtype);
            CommandObjects.addElement(oCommand);
        }
        return oCommand;
    }


    public void setQueryByName(String _QueryName) {
        CommandObject oQueryObject = new CommandObject(_QueryName, com.sun.star.sdb.CommandType.QUERY);
        this.CommandObjects.addElement(oQueryObject);
    }

    public class CommandObject {
        public XNameAccess xColumns; // todo: wrap method around this property
        public String Name;
        public int CommandType;

        public CommandObject(String _CommandName, int _CommandType) {
            try {
                Object oCommand;
                this.Name = _CommandName;
                this.CommandType = _CommandType;
                if (xTableNames == null)
                    setCommandNames();
                if (CommandType == com.sun.star.sdb.CommandType.TABLE)
                    oCommand = xTableNames.getByName(Name);
                else
                    oCommand = xQueryNames.getByName(Name);
                XColumnsSupplier xCommandCols = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, oCommand);
                xColumns = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xCommandCols.getColumns());
            } catch (Exception exception) {
                exception.printStackTrace(System.out);
            }
        }
    }

    public void setCommandNames() {
        XTablesSupplier xDBTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, DBConnection);
        xTableNames = (XNameAccess) xDBTables.getTables();
        TableNames = (String[]) xTableNames.getElementNames();
        XQueriesSupplier xDBQueries = (XQueriesSupplier) UnoRuntime.queryInterface(XQueriesSupplier.class, DBConnection);
        xQueryNames = (XNameAccess) xDBQueries.getQueries();
        QueryNames = xQueryNames.getElementNames();
        java.util.Arrays.sort(QueryNames);

    }


    void InitializeWidthList() {
        WidthList = new int[16][2];
        WidthList[0][0] = DataType.BIT; // ==  -7;
        WidthList[1][0] = DataType.TINYINT; // ==  -6;
        WidthList[2][0] = DataType.BIGINT; // ==  -5;
        WidthList[3][0] = DataType.LONGVARCHAR; // ==  -1;
        WidthList[4][0] = DataType.CHAR; // ==   1;
        WidthList[5][0] = DataType.NUMERIC; // ==   2;
        WidthList[6][0] = DataType.DECIMAL; // ==   3;  [mit Nachkommastellen]
        WidthList[7][0] = DataType.INTEGER; // ==   4;
        WidthList[8][0] = DataType.SMALLINT; // ==   5;
        WidthList[9][0] = DataType.FLOAT; // ==   6;
        WidthList[10][0] = DataType.REAL; // ==   7;
        WidthList[11][0] = DataType.DOUBLE; // ==   8;
        WidthList[12][0] = DataType.VARCHAR; // ==  12;
        WidthList[13][0] = DataType.DATE; // ==  91;
        WidthList[14][0] = DataType.TIME; // ==  92;
        WidthList[15][0] = DataType.TIMESTAMP; // ==  93;
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

    }


    public int getMaxTablesInSelect(){
    try {
        int itablecount = xDBMetaData.getMaxTablesInSelect();
        if (itablecount == 0)
            return this.NOLIMIT;
        else
            return itablecount;
    } catch (SQLException e) {
        e.printStackTrace(System.out);
        return - 1;
    }}


    public int getMaxColumnsInSelect() {
        return iMaxColumnsInSelect;
    }

    public int getMaxColumnsInGroupBy() {
        return iMaxColumnsInGroupBy;
    }

    private void setMaxColumnsInSelect() throws SQLException {
        iMaxColumnsInSelect = xDBMetaData.getMaxColumnsInSelect();
        if (iMaxColumnsInSelect == 0)
            iMaxColumnsInSelect = this.NOLIMIT;
    }

    private void setMaxColumnsInGroupBy() throws SQLException {
        iMaxColumnsInGroupBy = xDBMetaData.getMaxColumnsInGroupBy();
        if (iMaxColumnsInGroupBy == 0)
            iMaxColumnsInGroupBy = this.NOLIMIT;
    }


    private void getDataSourceObjects() throws Exception{
    try {
        xBookmarksSuppl = (XBookmarksSupplier) UnoRuntime.queryInterface(XBookmarksSupplier.class, this.xDataSource);
        xDBMetaData = DBConnection.getMetaData();
        XChild xChild = (XChild) UnoRuntime.queryInterface(XChild.class, DBConnection);
        Object oDataSource = xChild.getParent();
        xNumberFormatsSupplier = (XNumberFormatsSupplier) AnyConverter.toObject(XNumberFormatsSupplier.class,
                                                                                Helper.getUnoPropertyValue(oDataSource, "NumberFormatsSupplier"));
        NumberFormats = xNumberFormatsSupplier.getNumberFormats();
        CharLocale = Configuration.getOfficeLocale(xMSF);
        this.setStandardFormatKeys(true);
        setMaxColumnsInGroupBy();
        setMaxColumnsInSelect();
    } catch (SQLException e) {
        e.printStackTrace(System.out);
    }}


    private void setDataSourceByName(String _DataSourceName, boolean bgetInterfaces) {
        try {
            this.DataSourceName = _DataSourceName;
            Object oDataSource = xNameAccess.getByName(DataSourceName);
            xDataSource = (XDataSource) UnoRuntime.queryInterface(XDataSource.class, oDataSource);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void getDataSourceInterfaces() throws Exception{
        xCompleted = (XCompletedConnection) UnoRuntime.queryInterface(XCompletedConnection.class, xDataSource);
        XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDataSource);
        bPasswordIsRequired = ((Boolean) xPSet.getPropertyValue("IsPasswordRequired")).booleanValue();
    }


    public boolean getConnection(PropertyValue[] curproperties){
    try {
        com.sun.star.sdbc.XConnection xConnection = null;
        if (Properties.hasPropertyValue(curproperties, "ActiveConnection"))
        {
            xConnection = (com.sun.star.sdbc.XConnection) AnyConverter.toObject(com.sun.star.sdbc.XConnection.class,
                Properties.getPropertyValue(curproperties, "ActiveConnection"));
            if (xConnection !=null)
            {
                com.sun.star.container.XChild child = (com.sun.star.container.XChild)UnoRuntime.queryInterface(com.sun.star.container.XChild.class, xConnection);

                xDataSource = (XDataSource) UnoRuntime.queryInterface(XDataSource.class, child.getParent());
                XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDataSource);
                if ( xPSet != null )
                    DataSourceName = AnyConverter.toString(xPSet.getPropertyValue("Name"));
                return getConnection(xConnection);
            }

        }
        if (Properties.hasPropertyValue(curproperties, "DataSourceName")){
            String sDataSourceName = AnyConverter.toString(Properties.getPropertyValue(curproperties, "DataSourceName"));
            return getConnection(sDataSourceName);
        }
    } catch (IllegalArgumentException e){
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


    public boolean getConnection(String _DataSourceName) {
        setDataSourceByName(_DataSourceName, true);
        return getConnection(xDataSource);
    }


    public boolean getConnection(com.sun.star.sdbc.XConnection _DBConnection){
    try {
        this.DBConnection = _DBConnection;
        getDataSourceObjects();
        bConnectionOvergiven = true;
        return true;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return false;
    }}


    public boolean getConnection(XDataSource xDataSource){
    bConnectionOvergiven = false;
    Resource oResource = new Resource(xMSF, "Database", "dbw");
    try {
        int iMsg = 0;
        boolean bgetConnection = false;
        if (DBConnection != null)
            xComponent.dispose();
        getDataSourceInterfaces();
        if (bPasswordIsRequired == false) {
            DBConnection = xDataSource.getConnection("", "");
            bgetConnection = true;
        } else {
            XInterface xInteractionHandler = (XInterface) xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
            XInteractionHandler oInteractionHandler = (XInteractionHandler) UnoRuntime.queryInterface(XInteractionHandler.class, xInteractionHandler);
            boolean bExitLoop = true;
            do {
                XCompletedConnection xCompleted = (XCompletedConnection) UnoRuntime.queryInterface(XCompletedConnection.class, xDataSource);
                try {
                    DBConnection = xCompleted.connectWithCompletion(oInteractionHandler);
                    bgetConnection = DBConnection != null;
                    if (bgetConnection == false)
                        bExitLoop = true;
                } catch (Exception exception) {
                    // Note:  WindowAttributes from toolkit/source/awt/vclxtoolkit.cxx
                    String sMsgNoConnection = oResource.getResText(RID_DB_COMMON + 14);
                    iMsg = SystemDialog.showMessageBox(xMSF, "QueryBox", VclWindowPeerAttribute.RETRY_CANCEL, sMsgNoConnection);
                    bExitLoop = iMsg == 0;
                    bgetConnection = false;
                }
            } while (bExitLoop == false);
        }
        if (bgetConnection == false){
            String sMsgConnectionImpossible = oResource.getResText(RID_DB_COMMON + 35);
            SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible);
        }
        else {
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, DBConnection);
            getDataSourceObjects();
        }
        return bgetConnection;
    } catch (Exception exception) {
        String sMsgConnectionImpossible = oResource.getResText(RID_DB_COMMON + 35);
        SystemDialog.showMessageBox(xMSF, "ErrorBox", VclWindowPeerAttribute.OK, sMsgConnectionImpossible);
        exception.printStackTrace(System.out);
        return false;
    }}


    /**
     * @deprecated links in datasource are no longer possible since integration of cws insight01
     * @param StorePath
     */
    public void createDBLink(String StorePath) {
        try {
            String BookmarkName = JavaTools.getFileDescription(StorePath);
            XNameAccess xBookmarks = xBookmarksSuppl.getBookmarks();
            BookmarkName = Desktop.getUniqueName(xBookmarks, BookmarkName);
            XNameContainer xNameCont = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xBookmarks);
            xNameCont.insertByName(BookmarkName, StorePath);
            XFlushable xFlush = (XFlushable) UnoRuntime.queryInterface(XFlushable.class, xBookmarks);
            xFlush.flush();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    /**
     * inserts a Query to a datasource; There is no validation if the queryname is already existing in the datasource
     * @param oQuery
     * @param QueryName
     */
    public String createQuery(SQLQueryComposer _oSQLQueryComposer, String _QueryName) {
        try {
            XQueryDefinitionsSupplier xQueryDefinitionsSuppl = (XQueryDefinitionsSupplier) UnoRuntime.queryInterface(XQueryDefinitionsSupplier.class, xDataSource);
            XNameAccess xQueryDefs = xQueryDefinitionsSuppl.getQueryDefinitions();
            XSingleServiceFactory xSSFQueryDefs = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, xQueryDefs);
            Object oQuery = xSSFQueryDefs.createInstance(); //"com.sun.star.sdb.QueryDefinition"
            XPropertySet xPSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, oQuery);
            String s = _oSQLQueryComposer.xQueryAnalyzer.getQuery();
            xPSet.setPropertyValue("Command", _oSQLQueryComposer.xQueryAnalyzer.getQuery());
            XNameContainer xNameCont = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xQueryDefs);
            XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, xQueryDefs);
            String sreturnname = Desktop.getUniqueName(xNameAccess, _QueryName);
            xNameCont.insertByName(sreturnname, oQuery);
            return sreturnname;
            //TODO was passiert mit den folgenden Zeilen????????
            //      XFlushable xFlush = (XFlushable) UnoRuntime.queryInterface(XFlushable.class, xQueryDefs);
            //      xFlush.flush();
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }


    public XNameAccess getReportDocuments(){
        XReportDocumentsSupplier xReportDocumentSuppl = (XReportDocumentsSupplier) UnoRuntime.queryInterface(XReportDocumentsSupplier.class, this.xDataSource);
        return xReportDocumentSuppl.getReportDocuments();
    }


    public XNameAccess getFormDocuments(){
        XFormDocumentsSupplier xFormDocumentSuppl = (XFormDocumentsSupplier) UnoRuntime.queryInterface(XFormDocumentsSupplier.class, this.xDataSource);
        return xFormDocumentSuppl.getFormDocuments();
    }


    /**
     * adds the passed document as a report or a form to the database. Afterwards the document is deleted.
     * the document may not be open
     * @param xComponent
     * @param _bIsForm describes the type of the document: "form" or "report"
     */
    public void addDatabaseDocument(XComponent _xComponent, boolean _bIsForm,boolean bAsTemplate){
    try {

        XModel xDocumentModel = (XModel) UnoRuntime.queryInterface(XModel.class, _xComponent);
        String sPath = xDocumentModel.getURL();
        String sFileName = FileAccess.getFilename(sPath);
        _xComponent.dispose();
        XNameAccess xDocNameAccess;
        if (_bIsForm)
            xDocNameAccess = getFormDocuments();
        else
            xDocNameAccess = getReportDocuments();
        PropertyValue[] aDocProperties = new PropertyValue[_bIsForm ? 3 : 4];
        aDocProperties[0] = Properties.createProperty("Name", sFileName);
        aDocProperties[1] = Properties.createProperty("Parent", xDocNameAccess);
        aDocProperties[2] = Properties.createProperty("URL", sPath);
                if ( !_bIsForm )
                    aDocProperties[3] = Properties.createProperty("AsTemplate", new Boolean(bAsTemplate));

        XMultiServiceFactory xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, xDocNameAccess);
        Object oDBDocument = xDocMSF.createInstanceWithArguments("com.sun.star.sdb.DocumentDefinition", aDocProperties);
        XNameContainer xNamed = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xDocNameAccess);
        xNamed.insertByName(sFileName, oDBDocument);
        XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
        XSimpleFileAccess xSimpleFileAccess = (XSimpleFileAccess) UnoRuntime.queryInterface(XSimpleFileAccess.class, xInterface);
        xSimpleFileAccess.kill(sPath);
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public void openReportDocument(String _sReportName){
    try {
        PropertyValue[] aArgs = new PropertyValue[2];
        aArgs[0] = Properties.createProperty("ActiveConnection", DBConnection);
        aArgs[1] = Properties.createProperty("OpenMode", "open");
        XNameAccess xReportDocuments = getReportDocuments();
        XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, xReportDocuments);
        XHierarchicalNameContainer xHierarchicalNameContainer = (XHierarchicalNameContainer) UnoRuntime.queryInterface(XHierarchicalNameContainer.class, xReportDocuments);
        if (xHierarchicalNameContainer.hasByHierarchicalName(_sReportName))
            xComponentLoader.loadComponentFromURL(_sReportName, "", 0, aArgs);
    } catch (Exception e) {
        e.printStackTrace(System.out);
    }}


    public boolean isConnectionOvergiven() {
        return bConnectionOvergiven;
    }


    public void disposeDBMetaData() {
        if ((xComponent != null) && (!isConnectionOvergiven()))
            xComponent.dispose();
    }
}