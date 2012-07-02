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

package util;

import com.sun.star.uno.Exception;
import java.io.PrintWriter ;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.XConnection ;
import com.sun.star.sdbc.XResultSet ;
import com.sun.star.sdbc.XResultSetUpdate ;
import com.sun.star.sdbc.XStatement ;
import com.sun.star.sdbc.XRowUpdate ;
import com.sun.star.util.Date ;
import com.sun.star.uno.XNamingService ;
import com.sun.star.task.XInteractionHandler ;
import com.sun.star.sdb.XCompletedConnection ;
import com.sun.star.container.XEnumeration ;
import com.sun.star.container.XEnumerationAccess ;
import com.sun.star.io.XInputStream ;
import com.sun.star.io.XTextInputStream ;
import com.sun.star.io.XDataInputStream ;
import com.sun.star.container.XNameAccess ;
import com.sun.star.frame.XStorable;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XCloseable ;
import java.sql.Statement;
import java.sql.Connection;
import java.sql.DriverManager;

/**
* Provides useful methods for working with SOffice databases.
* Database creation, data transfering, outputting infromation.
*/
public class DBTools {

    private XMultiServiceFactory xMSF = null ;
    private XNamingService dbContext = null ;
    //JDBC driver
    public final static String TST_JDBC_DRIVER = "org.gjt.mm.mysql.Driver";

    // constants for TestDB table column indexes
    public final static int TST_STRING = 1 ;
    public final static int TST_INT = 2 ;
    public final static int TST_DOUBLE = 5 ;
    public final static int TST_DATE = 6 ;
    public final static int TST_BOOLEAN = 10 ;
    public final static int TST_CHARACTER_STREAM = 11 ;
    public final static int TST_BINARY_STREAM = 12 ;

    // constants for TestDB columns names
    public final static String TST_STRING_F = "_TEXT" ;
    public final static String TST_INT_F = "_INT" ;
    public final static String TST_DOUBLE_F = "_DOUBLE" ;
    public final static String TST_DATE_F = "_DATE" ;
    public final static String TST_BOOLEAN_F = "_BOOL" ;
    public final static String TST_CHARACTER_STREAM_F = "_MEMO1" ;
    public final static String TST_BINARY_STREAM_F = "_MEMO2" ;

    /**
    * Values for filling test table.
    */
    public final static Object[][] TST_TABLE_VALUES = new Object[][] {
        {"String1", new Integer(1), null, null, new Double(1.1),
         new Date((short) 1,(short) 1, (short) 2001), null, null, null,
         Boolean.TRUE, null, null},
        {"String2", new Integer(2), null, null, new Double(1.2),
         new Date((short) 2, (short) 1,(short)  2001), null, null, null,
         Boolean.FALSE, null, null},
        {null, null, null, null, null,
         null, null, null, null,
         null, null, null}
    } ;

    /**
    * Array of lengths of streams for each row in of the
    * <code>TST_TABLE_VALUES</code> constants.
    */
    public final static int[] TST_STREAM_LENGTHS = {0, 0, 0} ;

    /**
    * It's just a structure with some useful methods for representing
    * <code>com.sun.star.sdb.DataSource</code> service. All this
    * service's properties are stored in appropriate class fields.
    * Class also allows to construct its instances using service
    * information, and create new service instance upon class
    * fields.
    * @see com.sun.star.sdb.DataSource
    */
    public class DataSourceInfo {
        /**
        * Representation of <code>'Name'</code> property.
        */
        public String Name = null ;
        /**
        * Representation of <code>'URL'</code> property.
        */
        public String URL = null ;
        /**
        * Representation of <code>'Info'</code> property.
        */
        public PropertyValue[] Info = null ;
        /**
        * Representation of <code>'User'</code> property.
        */
        public String User = null ;
        /**
        * Representation of <code>'Password'</code> property.
        */
        public String Password = null ;
        /**
        * Representation of <code>'IsPasswordRequired'</code> property.
        */
        public Boolean IsPasswordRequired = null ;
        /**
        * Representation of <code>'SuppressVersionColumns'</code> property.
        */
        public Boolean SuppressVersionColumns = null ;
        /**
        * Representation of <code>'IsReadOnly'</code> property.
        */
        public Boolean IsReadOnly = null ;
        /**
        * Representation of <code>'TableFilter'</code> property.
        */
        public String[] TableFilter = null ;
        /**
        * Representation of <code>'TableTypeFilter'</code> property.
        */
        public String[] TableTypeFilter = null ;

        /**
        * Creates an empty instance.
        */
        public DataSourceInfo()
        {
        }

        /**
        * Creates an instance laying upon specified DataSource.
        * @param dataSource All source properties are copied into
        * class fields.
        */
        public DataSourceInfo(Object dataSource) {
            XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class, dataSource) ;

            try {
                Name = (String)xProps.getPropertyValue("Name") ;
                URL = (String)xProps.getPropertyValue("URL") ;
                Info = (PropertyValue[])xProps.getPropertyValue("Info") ;
                User = (String)xProps.getPropertyValue("User") ;
                Password = (String)xProps.getPropertyValue("Password") ;
                IsPasswordRequired = (Boolean)xProps.getPropertyValue("IsPasswordRequired") ;
                SuppressVersionColumns = (Boolean)
                    xProps.getPropertyValue("SuppressVersionColumns") ;
                IsReadOnly = (Boolean)xProps.getPropertyValue("IsReadOnly") ;
                TableFilter = (String[])xProps.getPropertyValue("TableFilter") ;
                TableTypeFilter = (String[])xProps.getPropertyValue("TableTypeFilter") ;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.err.println("util.DBTools.DataSourceInfo: Error retrieving property") ;
                e.printStackTrace(System.err) ;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.err.println("util.DBTools.DataSourceInfo: Error retrieving property") ;
                e.printStackTrace(System.err) ;
            }
        }

        /**
        * Prints datasource info.
        * @param out Stream to which information is printed.
        */
        public void printInfo(PrintWriter out) {
            out.println("Name = '" + Name + "'") ;
            out.println("  URL = '" + URL + "'") ;
            out.print("  Info = ") ;
            if (Info == null) out.println("null") ;
            else {
                out.print("{") ;
                for (int i = 0; i < Info.length; i++) {
                    out.print(Info[i].Name + " = '" + Info[i].Value + "'") ;
                    if (i + 1 < Info.length) out.print("; ") ;
                }
                out.println("}") ;
            }
            out.println("  User = '" + User + "'") ;
            out.println("  Password = '" + Password + "'") ;
            out.println("  IsPasswordRequired = '" + IsPasswordRequired + "'") ;
            out.println("  SuppressVersionColumns = '" + SuppressVersionColumns + "'") ;
            out.println("  IsReadOnly = '" + IsReadOnly + "'") ;
            out.print("  TableFilter = ") ;
            if (TableFilter == null) out.println("null") ;
            else {
                out.print("{") ;
                for (int i = 0; i < TableFilter.length; i++) {
                    out.print("'" + TableFilter[i] + "'") ;
                    if (i+1 < TableFilter.length) out.print("; ");
                }
                out.println("}") ;
            }
            out.print("  TableTypeFilter = ") ;
            if (TableTypeFilter == null) out.println("null") ;
            else {
                out.print("{") ;
                for (int i = 0; i < TableTypeFilter.length; i++) {
                    out.print("'" + TableTypeFilter[i] + "'") ;
                    if (i+1 < TableTypeFilter.length) out.print("; ");
                }
                out.println("}") ;
            }
        }

        /**
        * Creates new <code>com.sun.star.sdb.DataSource</code> service
        * instance and copies all fields (which are not null) to
        * appropriate service properties.
        * @return <code>com.sun.star.sdb.DataSource</code> service.
        */
        public Object getDataSourceService() throws Exception
        {
            Object src = src = xMSF.createInstance("com.sun.star.sdb.DataSource") ;

            XPropertySet props = UnoRuntime.queryInterface
                (XPropertySet.class, src) ;

            if (Name != null) props.setPropertyValue("Name", Name) ;
            if (URL != null) props.setPropertyValue("URL", URL) ;
            if (Info != null) props.setPropertyValue("Info", Info) ;
            if (User != null) props.setPropertyValue("User", User) ;
            if (Password != null) props.setPropertyValue("Password", Password) ;
            if (IsPasswordRequired != null) props.setPropertyValue("IsPasswordRequired", IsPasswordRequired) ;
            if (SuppressVersionColumns != null) props.setPropertyValue("SuppressVersionColumns", SuppressVersionColumns) ;
            if (IsReadOnly != null) props.setPropertyValue("IsReadOnly", IsReadOnly) ;
            if (TableFilter != null) props.setPropertyValue("TableFilter", TableFilter) ;
            if (TableTypeFilter != null) props.setPropertyValue("TableTypeFilter", TableTypeFilter) ;

            return src ;
        }
    }

    /**
    * Creates class instance.
    * @param xMSF <code>XMultiServiceFactory</code>.
    */
    public DBTools(XMultiServiceFactory xMSF, PrintWriter _logger )
    {
        this.xMSF = xMSF ;
        try {
            Object cont = xMSF.createInstance("com.sun.star.sdb.DatabaseContext") ;

            dbContext = UnoRuntime.queryInterface
                (XNamingService.class, cont) ;

        } catch (com.sun.star.uno.Exception e) {}
    }

    /**
    * Returns new instance of <code>DataSourceInfo</code> class.
    */
    public DataSourceInfo newDataSourceInfo() { return new DataSourceInfo() ;}

    /**
    * Returns new instance of <code>DataSourceInfo</code> class.
    */
    public DataSourceInfo newDataSourceInfo(Object dataSource) {
        return new DataSourceInfo(dataSource);
    }

    /**
    * Registers the datasource on the specified name in
    * <code>DatabaseContext</code> service.
    * @param name Name which dataSource will have in global context.
    * @param dataSource <code>DataSource</code> object which is to
    * be registered.
    */
    public void registerDB(String name, Object dataSource)
        throws com.sun.star.uno.Exception {

        dbContext.registerObject(name, dataSource) ;
    }


    /**
    * First tries to revoke the datasource with the specified
    * name and then registers a new one.
    * @param name Name which dataSource will have in global context.
    * @param dataSource <code>DataSource</code> object which is to
    * be registered.
    */
    public void reRegisterDB(String name, Object dataSource)
        throws com.sun.star.uno.Exception {

        try {
            revokeDB(name) ;
        } catch (com.sun.star.uno.Exception e) {}

        XDocumentDataSource xDDS = UnoRuntime.queryInterface(XDocumentDataSource.class, dataSource);
        XStorable store = UnoRuntime.queryInterface(XStorable.class,
                xDDS.getDatabaseDocument());
        String aFile = utils.getOfficeTemp(xMSF) + name + ".odb";
        store.storeAsURL(aFile, new PropertyValue[] {  });

        registerDB(name, dataSource) ;
    }

    /**
    * RESERVED. Not used.
    */
    public XConnection connectToTextDB(String contextName,
        String dbDir, String fileExtension)
                            throws com.sun.star.uno.Exception {

        try {
            XInterface newSource = (XInterface) xMSF.createInstance
                ("com.sun.star.sdb.DataSource") ;

            XPropertySet xSrcProp = UnoRuntime.queryInterface(XPropertySet.class, newSource);

            xSrcProp.setPropertyValue("URL", "sdbc:text:" + dirToUrl(dbDir));

            PropertyValue extParam = new PropertyValue() ;
            extParam.Name = "EXT" ;
            extParam.Value = fileExtension ;

            xSrcProp.setPropertyValue("Info", new PropertyValue[] {extParam}) ;

            dbContext.registerObject(contextName, newSource) ;

            Object handler = xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
            XInteractionHandler xHandler = UnoRuntime.queryInterface(XInteractionHandler.class, handler) ;

            XCompletedConnection xSrcCon = UnoRuntime.queryInterface(XCompletedConnection.class, newSource) ;

            XConnection con = xSrcCon.connectWithCompletion(xHandler) ;

            return con ;
        } finally {
            try {
                dbContext.revokeObject(contextName) ;
            } catch (Exception e) {}
        }
    }

    /**
    * Registers DBase database (directory with DBF files) in the
    * global DB context, then connects to it.
    * @param contextName Name under which DB will be registered.
    * @param dbDir The directory with DBF tables.
    * @return Connection to the DB.
    */
    public XConnection connectToDBase(String contextName,
        String dbDir)
        throws com.sun.star.uno.Exception {

        try {
            XInterface newSource = (XInterface) xMSF.createInstance
                ("com.sun.star.sdb.DataSource") ;

            XPropertySet xSrcProp = UnoRuntime.queryInterface(XPropertySet.class, newSource);
            xSrcProp.setPropertyValue("URL", "sdbc:dbase:" + dirToUrl(dbDir));

            dbContext.registerObject(contextName, newSource) ;

            XConnection con = connectToSource(newSource) ;

            return con ;
        } catch(com.sun.star.uno.Exception e) {
            try {
                dbContext.revokeObject(contextName) ;
            } catch (Exception ex) {}

            throw e ;
        }
    }

    /**
    * Performs connection to DataSource specified.
    * @param dbSource <code>com.sun.star.sdb.DataSource</code> service
    *     specified data source which must be already registered in the
    *     <code>DatabaseContext</code> service.
    * @return Connection to the data source.
    */
    public XConnection connectToSource(Object dbSource)
        throws com.sun.star.uno.Exception {

        Object handler = xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
        XInteractionHandler xHandler = UnoRuntime.queryInterface(XInteractionHandler.class, handler) ;

        XCompletedConnection xSrcCon = UnoRuntime.queryInterface(XCompletedConnection.class, dbSource) ;

        return xSrcCon.connectWithCompletion(xHandler) ;
    }

    /**
    * Registers Test data source in the <code>DatabaseContext</code> service.
    * This source always has name <code>'APITestDatabase'</code> and it
    * is registered in subdirectory <code>TestDB</code> of directory
    * <code>docPath</code> which is supposed to be a directory with test
    * documents, but can be any other (it must have subdirectory with DBF
    * tables). If such data source doesn't exists or exists with
    * different URL it is recreated and reregistered.
    * @param docPath Path to database <code>TestDB</code> directory.
    * @return <code>com.sun.star.sdb.DataSource</code> service
    * implementation which represents TestDB.
    */
    public Object registerTestDB(String docPath)
        throws com.sun.star.uno.Exception {

        String testURL = null ;
        if (docPath.endsWith("/") || docPath.endsWith("\\"))
            testURL = dirToUrl(docPath + "TestDB") ;
        else
            testURL = dirToUrl(docPath + "/" + "TestDB") ;
        testURL = "sdbc:dbase:" + testURL ;

        String existURL = null ;

        XNameAccess na = UnoRuntime.queryInterface
            (XNameAccess.class, dbContext) ;

        Object src = null ;
        if (na.hasByName("APITestDatabase")) {
            src = dbContext.getRegisteredObject("APITestDatabase") ;

            XPropertySet srcPs = UnoRuntime.queryInterface
                (XPropertySet.class, src) ;

            existURL = (String) srcPs.getPropertyValue("URL") ;
        }

        if (src == null || !testURL.equals(existURL)) {
            // test data source must be reregistered.
            DataSourceInfo info = new DataSourceInfo() ;
            info.URL = testURL ;
            src = info.getDataSourceService() ;
            reRegisterDB("APITestDatabase", src) ;
            src = dbContext.getRegisteredObject("APITestDatabase") ;
        }

        return src ;
    }

    /**
    * Connects to <code>DataSource</code> specially created for testing.
    * This source always has name <code>'APITestDatabase'</code> and it
    * is registered in subdirectory <code>TestDB</code> of directory
    * <code>docPath</code> which is supposed to be a directory with test
    * documents, but can be any other (it must have subdirectory with DBF
    * tables). If such data source doesn't exists or exists with
    * different URL it is recreated and reregistered. Finally connection
    * performed.
    * @param docPath Path to database <code>TestDB</code> directory.
    * @return Connection to test database.
    */
    public XConnection connectToTestDB(String docPath)
        throws com.sun.star.uno.Exception {

        return connectToSource(registerTestDB(docPath)) ;
    }

    /**
    * Empties the table in the specified source.
    * @param con Connection to the DataSource where appropriate
    * table exists.
    * @param table The name of the table where all rows will be deleted.
    * @return Number of rows deleted.
    */

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Currently doesn't work because of bugs 85509, 85510

    public int deleteAllRows(XConnection con, String table)
        throws com.sun.star.sdbc.SQLException {

        XStatement stat = con.createStatement() ;

        XResultSet set = stat.executeQuery("SELECT * FROM " + table) ;

        XResultSetUpdate updt = UnoRuntime.queryInterface
            (XResultSetUpdate.class, set) ;

        int count = 0 ;
        set.last() ;
        int rowNum = set.getRow() ;
        set.first() ;

        for (int i = 0; i < rowNum; i++) {
            updt.deleteRow() ;
            set.next() ;
            count ++ ;
        }

        XCloseable xClose = UnoRuntime.queryInterface
            (XCloseable.class, set) ;
        xClose.close() ;

        return count ;
    }

    /**
    * Inserts row into test table of the specified connection.
    * Test table has some predefined format which includes as much
    * field types as possible. For every column type constants
    * {@link #TST_STRING TST_STRING}, {@link #TST_INT TST_INT}, etc.
    * are declared for column index fast find.
    * @param con Connection to data source where test table exists.
    * @param table Test table name.
    * @param values Values to be inserted into test table. Values of
    * this array inserted into appropriate fields depending on their
    * types. So <code>String</code> value of the array is inserted
    * into the field of <code>CHARACTER</code> type, etc.
    * @param streamLength Is optional. It is used only if in values
    * list <code>XCharacterInputStream</code> or <code>XBinaryInputStream
    * </code> types specified. In this case the parameter specifies
    * the length of the stream for inserting.
    */
    public void addRowToTestTable(XConnection con, String table, Object[] values,
        int streamLength)
        throws com.sun.star.sdbc.SQLException {

        XStatement stat = con.createStatement() ;

        XResultSet set = stat.executeQuery("SELECT * FROM " + table) ;

        XResultSetUpdate updt = UnoRuntime.queryInterface
            (XResultSetUpdate.class, set) ;

        XRowUpdate rowUpdt = UnoRuntime.queryInterface
            (XRowUpdate.class, set) ;

        updt.moveToInsertRow() ;

        for (int i = 0; i < values.length; i++) {
            if (values[i] instanceof String) {
                rowUpdt.updateString(TST_STRING, (String) values[i]) ;
            } else
            if (values[i] instanceof Integer) {
                rowUpdt.updateInt(TST_INT, ((Integer) values[i]).intValue()) ;
            } else
            if (values[i] instanceof Double) {
                rowUpdt.updateDouble(TST_DOUBLE, ((Double) values[i]).doubleValue()) ;
            } else
            if (values[i] instanceof Date) {
                rowUpdt.updateDate(TST_DATE, (Date) values[i]) ;
            } else
            if (values[i] instanceof Boolean) {
                rowUpdt.updateBoolean(TST_BOOLEAN, ((Boolean) values[i]).booleanValue()) ;
            } else
            if (values[i] instanceof XTextInputStream) {
                rowUpdt.updateCharacterStream(TST_CHARACTER_STREAM, (XInputStream) values[i],
                    streamLength) ;
            } else
            if (values[i] instanceof XDataInputStream) {
                rowUpdt.updateBinaryStream(TST_BINARY_STREAM, (XInputStream) values[i],
                    streamLength) ;
            }
        }

        updt.insertRow() ;

        XCloseable xClose = UnoRuntime.queryInterface
            (XCloseable.class, set) ;
        xClose.close() ;
    }

    /**
    * Initializes test table specified of the connection specified.
    * Deletes all record from table, and then inserts data from
    * <code>TST_TABLE_VALUES</code> constant array. <p>
    * Test table has some predefined format which includes as much
    * field types as possible. For every column type constants
    * {@link #TST_STRING TST_STRING}, {@link #TST_INT TST_INT}, etc.
    * are declared for column index fast find.
    * @param con Connection to data source where test table exists.
    * @param table Test table name.
    */
    public void initializeTestTable(XConnection con, String table)
        throws com.sun.star.sdbc.SQLException {

        deleteAllRows(con, table) ;

        for (int i = 0; i < TST_TABLE_VALUES.length; i++) {
            addRowToTestTable(con, table, TST_TABLE_VALUES[i], TST_STREAM_LENGTHS[i]) ;
        }
    }

    /**
    * Prints full info about currently registered DataSource's.
    */
    public void printRegisteredDatabasesInfo(PrintWriter out) {
        XEnumerationAccess dbContEA = UnoRuntime.queryInterface(XEnumerationAccess.class, dbContext) ;

        XEnumeration xEnum = dbContEA.createEnumeration() ;

        out.println("DatabaseContext registered DataSource's :") ;
        while (xEnum.hasMoreElements()) {
            try {
                DataSourceInfo inf = new DataSourceInfo(xEnum.nextElement()) ;
                inf.printInfo(out) ;
            } catch (com.sun.star.container.NoSuchElementException e) {}
            catch (com.sun.star.lang.WrappedTargetException e) {}
        }
    }

    /**
    * Convert system pathname to SOffice URL string
    * (for example 'C:\Temp\DBDir\' -> 'file:///C|/Temp/DBDir/').
    * (for example '\\server\Temp\DBDir\' -> 'file://server/Temp/DBDir/').
    * Already converted string retured unchanged.
    */
    public static String dirToUrl(String dir) {
        String retVal = null;
        if (dir.startsWith("file:/")) retVal = dir;
        else {
            retVal = dir.replace(':', '|').replace('\\', '/');

            if (dir.startsWith("\\\\")) {
                retVal = "file:" + retVal;
            }

            else retVal = "file:///" + retVal ;
        }
        return retVal;
    }

    /**
    * Revokes datasource from global DB context.
    * @param name DataSource name to be revoked.
    */
    public void revokeDB(String name) throws com.sun.star.uno.Exception
    {
        dbContext.revokeObject(name) ;
    }

    /**
    * Initializes test table specified of the connection specified
    * using JDBC driver. Drops table with the name <code>tbl_name</code>,
    * creates new table with this name and then inserts data from
    * <code>TST_TABLE_VALUES</code> constant array. <p>
    * Test table has some predefined format which includes as much
    * field types as possible. For every column type constants
    * {@link #TST_STRING TST_STRING}, {@link #TST_INT TST_INT}, etc.
    * are declared for column index fast find.
    * @param tbl_name Test table name.
    */
    public void initTestTableUsingJDBC(String tbl_name, DataSourceInfo dsi)
        throws java.sql.SQLException,
               ClassNotFoundException {
        //register jdbc driver
        if ( dsi.Info[0].Name.equals("JavaDriverClass") ) {
            Class.forName((String)dsi.Info[0].Value);
        } else {
            Class.forName(TST_JDBC_DRIVER);
        }

        //getting connection
        Connection connection = null;

        connection = DriverManager.getConnection(
            dsi.URL, dsi.User, dsi.Password);
        Statement statement = connection.createStatement();

        //drop table
        dropMySQLTable(statement, tbl_name);

        //create table
        createMySQLTable(statement, tbl_name);

        //insert some content
        insertContentMySQLTable(statement, tbl_name);
    }

    /**
    * Inserts data from <code>TST_TABLE_VALUES</code> constant array
    * to test table <code>tbl_name</code>.
    * @param statement object used for executing a static SQL
    * statement and obtaining the results produced by it.
    * @param tbl_name Test table name.
    */
    protected void insertContentMySQLTable(Statement statement, String tbl_name)
        throws java.sql.SQLException {


        for(int i = 0; i < DBTools.TST_TABLE_VALUES.length; i++) {
            String query = "insert into " + tbl_name + " values (";
            int j = 0;
            while(j < DBTools.TST_TABLE_VALUES[i].length) {
                if (j > 0) {
                    query += ", ";
                }
                Object value = DBTools.TST_TABLE_VALUES[i][j];
                if (value instanceof String ||
                    value instanceof Date) {
                    query += "'";
                }
                if (value instanceof Date) {
                    Date date = (Date)value;
                    query += date.Year + "-" + date.Month +
                        "-" + date.Day;
                } else if (value instanceof Boolean) {
                    query += (((Boolean)value).booleanValue())
                        ? "1" : "0";
                } else {
                    query += value;
                }

                if (value instanceof String ||
                    value instanceof Date) {
                    query += "'";
                }
                j++;
            }
            query += ")";
            statement.executeUpdate(query);
        }
    }

    /**
     * Creates test table specified.
     * Test table has some predefined format which includes as much
     * field types as possible. For every column type constants
     * {@link #TST_STRING TST_STRING}, {@link #TST_INT TST_INT}, etc.
     * are declared for column index fast find.
     * @param statement object used for executing a static SQL
     * statement and obtaining the results produced by it.
     * @param tbl_name Test table name.
     */
    protected void createMySQLTable(Statement statement, String tbl_name)
        throws java.sql.SQLException {

        final String empty_col_name = "Column";
        int c = 0;
        String query = "create table " + tbl_name + " (";
        for (int i = 0; i < TST_TABLE_VALUES[0].length; i++) {
            if (i > 0) query += ",";

            switch(i + 1) {
                case TST_BINARY_STREAM:
                    query += TST_BINARY_STREAM_F + " BLOB";
                    break;
                case TST_BOOLEAN:
                    query += TST_BOOLEAN_F + " TINYINT";
                    break;
                case TST_CHARACTER_STREAM:
                    query += TST_CHARACTER_STREAM_F + " TEXT";
                    break;
                case TST_DATE:
                    query += TST_DATE_F + " DATE";
                    break;
                case TST_DOUBLE:
                    query += TST_DOUBLE_F + " DOUBLE";
                    break;
                case TST_INT:
                    query += TST_INT_F + " INT";
                    break;
                case TST_STRING:
                    query += TST_STRING_F + " TEXT";
                    break;
                default: query += empty_col_name + (c++) + " INT";
                         if (c == 1) {
                            query += " NOT NULL AUTO_INCREMENT";
                         }
            }
        }
        query += ", PRIMARY KEY (" + empty_col_name + "0)";
        query += ")";
        statement.execute(query);
    }

    /**
     * Drops table.
     * @param statement object used for executing a static SQL
     * statement and obtaining the results produced by it.
     * @param tbl_name Test table name.
     */
    protected void dropMySQLTable(Statement statement, String tbl_name)
        throws java.sql.SQLException {
        statement.executeUpdate("drop table if exists " + tbl_name);
    }
}
