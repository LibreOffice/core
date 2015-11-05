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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sdbc.XConnection ;
import com.sun.star.util.Date ;
import com.sun.star.uno.XNamingService ;
import com.sun.star.task.XInteractionHandler ;
import com.sun.star.sdb.XCompletedConnection ;
import com.sun.star.frame.XStorable;
import com.sun.star.sdb.XDocumentDataSource;
import java.sql.Statement;
import java.sql.Connection;
import java.sql.DriverManager;

/**
* Provides useful methods for working with SOffice databases.
* Database creation, data transferring, outputting infromation.
*/
public class DBTools {

    private final XMultiServiceFactory xMSF;
    private XNamingService dbContext;
    //JDBC driver
    public static final String TST_JDBC_DRIVER = "org.gjt.mm.mysql.Driver";

    // constants for TestDB table column indexes
    public static final int TST_STRING = 1 ;
    public static final int TST_INT = 2 ;
    private static final int TST_DOUBLE = 5 ;
    private static final int TST_DATE = 6 ;
    private static final int TST_BOOLEAN = 10 ;
    private static final int TST_CHARACTER_STREAM = 11 ;
    private static final int TST_BINARY_STREAM = 12 ;

    // constants for TestDB columns names
    public static final String TST_STRING_F = "_TEXT" ;
    public static final String TST_INT_F = "_INT" ;
    public static final String TST_DOUBLE_F = "_DOUBLE" ;
    public static final String TST_DATE_F = "_DATE" ;
    private static final String TST_BOOLEAN_F = "_BOOL" ;
    private static final String TST_CHARACTER_STREAM_F = "_MEMO1" ;
    public static final String TST_BINARY_STREAM_F = "_MEMO2" ;

    /**
    * Values for filling test table.
    */
    public static final Object[][] TST_TABLE_VALUES = new Object[][] {
        {"String1", Integer.valueOf(1), null, null, new Double(1.1),
         new Date((short) 1,(short) 1, (short) 2001), null, null, null,
         Boolean.TRUE, null, null},
        {"String2", Integer.valueOf(2), null, null, new Double(1.2),
         new Date((short) 2, (short) 1,(short)  2001), null, null, null,
         Boolean.FALSE, null, null},
        {null, null, null, null, null,
         null, null, null, null,
         null, null, null}
    } ;

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
        * Representation of <code>'IsReadOnly'</code> property.
        */
        private Boolean IsReadOnly = null ;
        /**
        * Representation of <code>'TableFilter'</code> property.
        */
        private String[] TableFilter = null ;

        /**
        * Creates new <code>com.sun.star.sdb.DataSource</code> service
        * instance and copies all fields (which are not null) to
        * appropriate service properties.
        * @return <code>com.sun.star.sdb.DataSource</code> service.
        */
        public Object getDataSourceService() throws Exception
        {
            Object src = xMSF.createInstance("com.sun.star.sdb.DataSource") ;

            XPropertySet props = UnoRuntime.queryInterface
                (XPropertySet.class, src) ;

            if (Name != null) props.setPropertyValue("Name", Name) ;
            if (URL != null) props.setPropertyValue("URL", URL) ;
            if (Info != null) props.setPropertyValue("Info", Info) ;
            if (User != null) props.setPropertyValue("User", User) ;
            if (Password != null) props.setPropertyValue("Password", Password) ;
            if (IsPasswordRequired != null) props.setPropertyValue("IsPasswordRequired", IsPasswordRequired) ;
            if (IsReadOnly != null) props.setPropertyValue("IsReadOnly", IsReadOnly) ;
            if (TableFilter != null) props.setPropertyValue("TableFilter", TableFilter) ;

            return src ;
        }
    }

    /**
    * Creates class instance.
    * @param xMSF <code>XMultiServiceFactory</code>.
    */
    public DBTools(XMultiServiceFactory xMSF )
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
    * Registers the datasource on the specified name in
    * <code>DatabaseContext</code> service.
    * @param name Name which dataSource will have in global context.
    * @param dataSource <code>DataSource</code> object which is to
    * be registered.
    */
    private void registerDB(String name, Object dataSource)
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

        Connection connection = null;
        Statement statement = null;
        try {
            //getting connection
            connection = DriverManager.getConnection(dsi.URL, dsi.User, dsi.Password);
            try {
                statement = connection.createStatement();

                //drop table
                dropMySQLTable(statement, tbl_name);

                //create table
                createMySQLTable(statement, tbl_name);

                //insert some content
                insertContentMySQLTable(statement, tbl_name);
            } finally {
                if (statement != null)
                    statement.close();
            }
        } finally {
            if (connection != null)
                connection.close();
        }
    }

    /**
    * Inserts data from <code>TST_TABLE_VALUES</code> constant array
    * to test table <code>tbl_name</code>.
    * @param statement object used for executing a static SQL
    * statement and obtaining the results produced by it.
    * @param tbl_name Test table name.
    */
    private void insertContentMySQLTable(Statement statement, String tbl_name)
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
    private void createMySQLTable(Statement statement, String tbl_name)
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
    private void dropMySQLTable(Statement statement, String tbl_name)
        throws java.sql.SQLException {
        statement.executeUpdate("drop table if exists " + tbl_name);
    }
}
