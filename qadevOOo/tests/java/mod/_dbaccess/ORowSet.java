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

package mod._dbaccess;

import ifc.sdb._XCompletedExecution;

import java.io.PrintWriter;
import java.util.ArrayList;
import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;
import util.utils;
import util.db.DataSource;
import util.db.DataSourceDescriptor;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.ParametersRequest;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.sdb.XInteractionSupplyParameters;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.task.XInteractionAbort;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.ucb.AuthenticationRequest;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sdb.RowSet</code>. <p>
 *
 * The following files used by this test :
 * <ul>
 *  <li><b> TestDB/TestDB.dbf </b> : the database file with some
 *    predefined fields described in <code>util.DBTools</code>.
 *    The copy of this file is always made in temp directory for
 *    testing purposes.</li>
 * </ul>
 * The following parameters in ini-file used by this test:
 * <ul>
 *   <li><code>test.db.url</code> - URL to MySQL database.
 *   For example: <code>mysql://mercury:3306/api_current</code></li>
 *   <li><code>test.db.user</code> - user for MySQL database</li>
 *   <li><code>test.db.password</code> - password for MySQL database</li>
 * </ul>
 *
 * @see com.sun.star.sdbc.RowSet
 * @see com.sun.star.sdbcx.XRowLocate
 * @see com.sun.star.sdbc.XResultSetUpdate
 * @see com.sun.star.util.XCancellable
 * @see com.sun.star.sdbc.XParameters
 * @see com.sun.star.sdbc.XResultSetMetaDataSupplier
 * @see com.sun.star.sdbcx.XDeleteRows
 * @see com.sun.star.sdbc.XCloseable
 * @see com.sun.star.sdbcx.XColumnsSupplier
 * @see com.sun.star.sdb.XResultSetAccess
 * @see com.sun.star.sdbc.XResultSet
 * @see com.sun.star.sdbc.XColumnLocate
 * @see com.sun.star.sdbc.XRowSet
 * @see com.sun.star.sdb.RowSet
 * @see com.sun.star.sdbc.XRowUpdate
 * @see com.sun.star.sdb.XRowSetApproveBroadcaster
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.sdbc.XRow
 * @see com.sun.star.sdbc.XWarningsSupplier
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.sdbcx.ResultSet
 * @see com.sun.star.sdbc.ResultSet
 * @see ifc.sdbc._RowSet
 * @see ifc.sdbcx._XRowLocate
 * @see ifc.sdbc._XResultSetUpdate
 * @see ifc.util._XCancellable
 * @see ifc.sdbc._XParameters
 * @see ifc.sdbc._XResultSetMetaDataSupplier
 * @see ifc.sdbcx._XDeleteRows
 * @see ifc.sdbc._XCloseable
 * @see ifc.sdbcx._XColumnsSupplier
 * @see ifc.sdb._XResultSetAccess
 * @see ifc.sdbc._XResultSet
 * @see ifc.sdbc._XColumnLocate
 * @see ifc.sdbc._XRowSet
 * @see ifc.sdb._RowSet
 * @see ifc.sdbc._XRowUpdate
 * @see ifc.sdb._XRowSetApproveBroadcaster
 * @see ifc.beans._XPropertySet
 * @see ifc.sdbc._XRow
 * @see ifc.sdbc._XWarningsSupplier
 * @see ifc.lang._XComponent
 * @see ifc.sdbcx._ResultSet
 * @see ifc.sdbc._ResultSet
 */
public class ORowSet extends TestCase {

    private static int uniqueSuffix = 0 ;
    private DBTools dbTools     = null ;
    private static String origDB = null ;
    String tableName = null;
    DataSourceDescriptor srcInf = null;
    boolean isMySQLDB = false;
    protected final static String dbSourceName = "ORowSetDataSource";
    public XConnection m_connection = null;
    private Object m_rowSet = null;
    private DataSource m_dataSource;
    private String m_tableFile;
    private XMultiServiceFactory m_orb = null;

    /**
    * Initializes some class fields. Then creates DataSource, which serves
    * as a single source for all tables created in the test.
    * This DataSource then registered in the global
    * <code>DatabaseContext</code> service. This data source's URL
    * points to SOffice temp directory where tables are copied from
    * <code>TestDocuments</code> directory on every environment
    * creation.
    * To create DataSource for MySQL database next parameters required
    * in ini-file:
    * <ul>
    *   <li><code>test.db.url</code> - URL to MySQL database.
    *   For example: <code>mysql://mercury:3306/api_current</code></li>
    *   <li><code>test.db.user</code> - user for MySQL database</li>
    *   <li><code>test.db.password</code> - password for MySQL database</li>
    * </ul>
    *
    * @throws StatusException if DataSource can not be created or
    * registered.
    */
    protected void initialize ( TestParameters Param, PrintWriter _log)
        throws StatusException
    {
        m_orb = (XMultiServiceFactory)Param.getMSF();

        String tmpDir = utils.getOfficeTemp( m_orb );

        origDB = util.utils.getFullTestDocName("TestDB/testDB.dbf");

        dbTools = new DBTools( m_orb, _log );

        // creating DataSource and registering it in DatabaseContext
        String dbURL = (String) Param.get("test.db.url");
        String dbUser = (String) Param.get("test.db.user");
        String dbPassword = (String) Param.get("test.db.password");

        log.println("Creating and registering DataSource ...");
        srcInf = new DataSourceDescriptor( m_orb );
        if (dbURL != null && dbUser != null && dbPassword != null)
        {
            isMySQLDB = true;
            log.println("dbURL = " + dbURL);
            log.println("dbUSER = " + dbUser);
            log.println("dbPASSWORD = " + dbPassword);
            //DataSource for mysql db
            tableName = "soffice_test_table";
            srcInf.URL = "jdbc:" + dbURL;
            srcInf.IsPasswordRequired = new Boolean(true);
            srcInf.Password = dbPassword;
            srcInf.User = dbUser;
            PropertyValue[] propInfo = new PropertyValue[1];
            propInfo[0] = new PropertyValue();
            propInfo[0].Name = "JavaDriverClass";
            propInfo[0].Value = "org.gjt.mm.mysql.Driver";
            srcInf.Info = propInfo;
        }
        else
        {
            srcInf.URL = "sdbc:dbase:" + DBTools.dirToUrl(tmpDir);
        }
        m_dataSource = srcInf.createDataSource();
        m_dataSource.registerAs( dbSourceName, true );
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * The database (DBF) file is copied from test document directory
    * into SOffice temp dir with unique name for each enviroment
    * creation. If the file cann't be copied (is not released)
    * then another unique name is used (file name suffix incremented
    * by 1).<p>
    *
    * <code>com.sun.star.sdb.RowSet</code> service created and its
    * source is all rows from the current copy of the table. Then
    * row set command ("select all rows from a table") is executed
    * and cursor is positioned to the first row. <p>
    *
    *     Object relations created :
    * <ul>
    *  <li> <code>'ORowSet.Connection'</code> for
    *      internal component test usage. Is used for
    *      closing connection when cleaning up environment. </li>
    *  <li> <code>'XRowSetApproveBroadcaster.ApproveChecker'</code> for
    *      {@link ifc.sdb._XRowSetApproveBroadcaster} interface
    *      implementation which made actions required </li>
    *  <li> <code>'CurrentRowData'</code> for
    *      {@link ifc.sdbc._XRow}, {@link ifc.sdbc._XRowUpdate} :
    *      exports types and values of the current row data.</li>
    *  <li> <code>'XColumnLocate.ColumnName'</code> for
    *      {@link ifc.sdbc._XColumnLocate} :
    *      the name of the first column of the table.</li>
    *  <li> <code>'XParameters.ParamValues'</code> for
    *      {@link ifc.sdbc._XParameters} :
    *      Collection of parameter types presented in the query. </li>
    *  <li> <code>'XRowUpdate.XRow'</code> for
    *      {@link ifc.sdbc._XRowUpdate} :
    *      <code>XRow</code> interface of the current component.</li>
    *  <li> <code>'XResultSetUpdate.UpdateTester'</code> for
    *      {@link ifc.sdbc._XResultSetUpdate} </li>
    * </ul>
    *
    * @see com.sun.star.sdb.DatabaseContext
    * @see com.sun.star.sdb.DataSource
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log)
    {
        XMultiServiceFactory orb = (XMultiServiceFactory)Param.getMSF();
        uniqueSuffix++;
        boolean envCreatedOK = false ;

        //initialize test table
        if (isMySQLDB)
        {
            try
            {
                DBTools.DataSourceInfo legacyDescriptor = dbTools.newDataSourceInfo();
                legacyDescriptor.Name = srcInf.Name;
                legacyDescriptor.User = srcInf.User;
                legacyDescriptor.Password = srcInf.Password;
                legacyDescriptor.Info = srcInf.Info;
                legacyDescriptor.URL = srcInf.URL;
                legacyDescriptor.IsPasswordRequired = srcInf.IsPasswordRequired;
                dbTools.initTestTableUsingJDBC(tableName, legacyDescriptor);
            }
            catch(java.sql.SQLException e)
            {
                e.printStackTrace(log);
                throw new StatusException(Status.failed("Couldn't " +
                    " init test table. SQLException..."));
            }
            catch(java.lang.ClassNotFoundException e)
            {
                throw new StatusException(Status.failed("Couldn't " +
                    "register mysql driver"));
            }
        }
        else
        {
            String oldF = null ;
            String newF = null ;
            String tempFolder = utils.getOfficeTemp( orb );
            do
            {
                tableName = "ORowSet_tmp" + uniqueSuffix ;
                oldF = utils.getFullURL(origDB);
                newF = tempFolder + tableName + ".dbf";
            }
            while ( !utils.tryOverwriteFile( orb, oldF, newF ) );
            m_tableFile = newF;
        }

        try
        {
            m_rowSet = orb.createInstance("com.sun.star.sdb.RowSet");

            XPropertySet rowSetProps = UnoRuntime.queryInterface( XPropertySet.class, m_rowSet );

            log.println("Trying to open: " + tableName);

            rowSetProps.setPropertyValue("DataSourceName", dbSourceName);
            rowSetProps.setPropertyValue("Command", tableName);
            rowSetProps.setPropertyValue("CommandType",
                new Integer(CommandType.TABLE));

            final XRowSet rowSet = UnoRuntime.queryInterface( XRowSet.class, m_rowSet);
            rowSet.execute();
            m_connection = UnoRuntime.queryInterface( XConnection.class, rowSetProps.getPropertyValue("ActiveConnection") );

            XResultSet xRes = UnoRuntime.queryInterface( XResultSet.class, m_rowSet );
            xRes.first();

            log.println( "creating a new environment for object" );
            TestEnvironment tEnv = new TestEnvironment( (XInterface)m_rowSet );

            // Adding obj relation for XRowSetApproveBroadcaster test
            {
                final XResultSet resultSet = UnoRuntime.queryInterface( XResultSet.class, m_rowSet );
                final XResultSetUpdate resultSetUpdate = UnoRuntime.queryInterface( XResultSetUpdate.class, m_rowSet );
                final XRowUpdate rowUpdate = UnoRuntime.queryInterface(XRowUpdate.class, m_rowSet );
                final PrintWriter logF = log ;
                tEnv.addObjRelation( "XRowSetApproveBroadcaster.ApproveChecker",
                    new ifc.sdb._XRowSetApproveBroadcaster.RowSetApproveChecker()
                    {
                        public void moveCursor()
                        {
                            try
                            {
                                resultSet.beforeFirst();
                                resultSet.afterLast();
                                resultSet.first();
                            }
                            catch (com.sun.star.sdbc.SQLException e)
                            {
                                logF.println("### _XRowSetApproveBroadcaster.RowSetApproveChecker.moveCursor() :");
                                e.printStackTrace(logF);
                                throw new StatusException( "RowSetApproveChecker.moveCursor failed", e );
                            }
                        }
                        public RowChangeEvent changeRow()
                        {
                            try
                            {
                                resultSet.first();
                                rowUpdate.updateString(1, "ORowSetTest2");
                                resultSetUpdate.updateRow();
                            }
                            catch (com.sun.star.sdbc.SQLException e)
                            {
                                logF.println("### _XRowSetApproveBroadcaster.RowSetApproveChecker.changeRow() :");
                                e.printStackTrace(logF);
                                throw new StatusException( "RowSetApproveChecker.changeRow failed", e );
                            }
                            RowChangeEvent ev = new RowChangeEvent();
                            ev.Action = com.sun.star.sdb.RowChangeAction.UPDATE ;
                            ev.Rows = 1 ;

                            return ev ;
                        }
                        public void changeRowSet()
                        {
                            try
                            {
                                // since we gave the row set a parametrized statement, we need to ensure the
                                // parameter is actually filled, otherwise we would get an empty result set,
                                // which would imply some further tests failing
                                XParameters rowSetParams = UnoRuntime.queryInterface( XParameters.class, resultSet );
                                rowSetParams.setString( 1, "String2" );
                                rowSet.execute();
                                resultSet.first();
                            }
                            catch (com.sun.star.sdbc.SQLException e)
                            {
                                logF.println("### _XRowSetApproveBroadcaster.RowSetApproveChecker.changeRowSet() :");
                                e.printStackTrace(logF);
                                throw new StatusException( "RowSetApproveChecker.changeRowSet failed", e );
                            }
                        }
                    }
                );
            }
            // Adding relations for XRow as a Vector with all data
            // of current row of RowSet.

            ArrayList<Object> rowData = new ArrayList<Object>();

            for (int i = 0; i < DBTools.TST_TABLE_VALUES[0].length; i++) {
                rowData.add(DBTools.TST_TABLE_VALUES[0][i]);
            }

            // here XRef must be added
            // here XBlob must be added
            // here XClob must be added
            // here XArray must be added

            tEnv.addObjRelation("CurrentRowData", rowData);

            // Adding relation for XColumnLocate ifc test
            tEnv.addObjRelation( "XColumnLocate.ColumnName", DBTools.TST_STRING_F );

            // Adding relation for XCompletedExecution
            tEnv.addObjRelation( "InteractionHandlerChecker", new InteractionHandlerImpl() );
            try
            {
                String sqlCommand = isMySQLDB
                    ?   "SELECT Column0  FROM soffice_test_table  WHERE ( (  Column0 = :param1 ) )"
                    :   "SELECT \"_TEXT\" FROM \"" + tableName + "\" WHERE ( ( \"_TEXT\" = :param1 ) )";
                rowSetProps.setPropertyValue( "DataSourceName", dbSourceName );
                rowSetProps.setPropertyValue( "Command", sqlCommand );
                rowSetProps.setPropertyValue( "CommandType", new Integer(CommandType.COMMAND) );
            }
            catch(Exception e)
            {
                throw new StatusException( "setting up the RowSet with a parametrized command failed", e );
            }

            // Adding relation for XParameters ifc test
            tEnv.addObjRelation( "XParameters.ParamValues", new ArrayList<String>() );

            // Adding relation for XRowUpdate
            final XRow row = UnoRuntime.queryInterface( XRow.class, m_rowSet );
            tEnv.addObjRelation("XRowUpdate.XRow", row);

            // Adding relation for XResultSetUpdate
            {
                final XResultSet resultSet = UnoRuntime.queryInterface( XResultSet.class, m_rowSet );
                final XRowUpdate rowUpdate = UnoRuntime.queryInterface( XRowUpdate.class, m_rowSet );

                tEnv.addObjRelation("XResultSetUpdate.UpdateTester",
                    new ifc.sdbc._XResultSetUpdate.UpdateTester()
                    {
                        String lastUpdate = null ;

                        public int rowCount() throws SQLException
                        {
                            int prevPos = resultSet.getRow();
                            resultSet.last();
                            int count = resultSet.getRow();
                            resultSet.absolute(prevPos);

                            return count ;
                        }

                        public void update() throws SQLException
                        {
                            lastUpdate = row.getString(1);
                            lastUpdate += "_" ;
                            rowUpdate.updateString(1, lastUpdate);
                        }

                        public boolean wasUpdated() throws SQLException
                        {
                            String getStr = row.getString(1);
                            return lastUpdate.equals(getStr);
                        }

                        public int currentRow() throws SQLException
                        {
                            return resultSet.getRow();
                        }
                    }
                );
            }

            envCreatedOK = true ;
            return tEnv;

        }
        catch(com.sun.star.uno.Exception e)
        {
            log.println( "couldn't set up tes tenvironment:" );
            e.printStackTrace(log);
            try
            {
                if ( m_connection != null )
                    m_connection.close();
            }
            catch(Exception ex)
            {
            }
            throw new StatusException( "couldn't set up tes tenvironment", e );
        }
        finally
        {
            if (!envCreatedOK)
            {
                try
                {
                    m_connection.close();
                }
                catch(Exception ex)
                {
                }
            }
        }

    } // finish method getTestEnvironment

    /**
    * Closes connection of <code>RowSet</code> instance created.
    */
    protected void cleanup( TestParameters Param, PrintWriter log)
    {
        String doing = null;
        try
        {
            doing = "revoking data source registration";
            log.println( doing );
            m_dataSource.revokeRegistration();

            doing = "closing database document";
            log.println( doing );
            XModel databaseDocModel = UnoRuntime.queryInterface( XModel.class,
                m_dataSource.getDatabaseDocument().getDatabaseDocument() );
            String documentFile = databaseDocModel.getURL();

            XCloseable closeModel = UnoRuntime.queryInterface( XCloseable.class,
                m_dataSource.getDatabaseDocument().getDatabaseDocument() );
            closeModel.close( true );

            if ( m_rowSet != null )
            {
                doing = "disposing row set";
                log.println( doing );
                XComponent rowSetComponent = UnoRuntime.queryInterface( XComponent.class, m_rowSet );
                rowSetComponent.dispose();
            }

            try
            {
                doing = "closing connection";
                log.println( doing );
                m_connection.close();
            }
            catch (com.sun.star.lang.DisposedException e)
            {
                log.println( "already closed - okay." );
            }

            doing = "deleting database file ("  + documentFile + ")";
            log.println( doing );
            impl_deleteFile( documentFile );

            if ( m_tableFile != null )
            {
                doing = "deleting dBase table file (" + m_tableFile + ")";
                log.println( doing );
                impl_deleteFile( m_tableFile );
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            log.println( "error: ");
            e.printStackTrace(log);
        }
    }

    private final void impl_deleteFile( final String _file )
    {
        java.io.File file = new java.io.File( _file );
        file.delete();
        if ( file.exists() )
            file.deleteOnExit();
    }

    /**
     * Implementation of interface _XCompletedExecution.CheckInteractionHandler
     * for the XCompletedExecution test
     * @see ifc.sdb._XCompletedExecution
     */
    public class InteractionHandlerImpl implements _XCompletedExecution.CheckInteractionHandler {
        private boolean handlerWasUsed = false;
        private PrintWriter log = new PrintWriter(System.out);

        public boolean checkInteractionHandler() {
            return handlerWasUsed;
        }

        public void handle(XInteractionRequest xInteractionRequest) {
            log.println("### _XCompletedExecution.InteractionHandlerImpl: handle called.");
            boolean abort = false;

            Object o = xInteractionRequest.getRequest();
            if (o instanceof ParametersRequest) {
            }
            else if (o instanceof AuthenticationRequest) {
                log.println("### The request in XCompletedExecution is of type 'AuthenticationRequest'");
                log.println("### This is not implemented in ORowSet.InteractionHandlerImpl test -> abort.");
                abort = true;
            }
            else {
                log.println("### Unknown request:" + o.toString());
                log.println("### This is not implemented in ORowSet.InteractionHandlerImpl test -> abort.");
                abort = true;
            }

            XInteractionContinuation[]xCont = xInteractionRequest.getContinuations();
            XInteractionSupplyParameters xParamCallback = null;
            for(int i=0; i<xCont.length; i++) {
                if (abort) {
                    XInteractionAbort xAbort = null;
                    xAbort = UnoRuntime.queryInterface(XInteractionAbort.class, xCont[i]);
                    if (xAbort != null)
                        xAbort.select();
                        return;
                }
                else {
                    xParamCallback = UnoRuntime.queryInterface(XInteractionSupplyParameters.class, xCont[i]);
                    if (xParamCallback != null)
                        break;
                }
            }
            if (xParamCallback != null) {
                log.println("### _XCompletedExecution.InteractionHandlerImpl: supplying parameters.");
                handlerWasUsed = true;
                PropertyValue[] prop = new PropertyValue[1];
                prop[0] = new PropertyValue();
                prop[0].Name = "param1";
                prop[0].Value = "Hi.";

                xParamCallback.setParameters(prop);
                xParamCallback.select();
            }
            else { // we should never reach this: abort has to be true first.
                log.println("### _XCompletedExecution.InteractionHandlerImpl: Got no " +
                            "'XInteractionSupplyParameters' and no 'XInteractionAbort'.");
            }
        }

        public void setLog(PrintWriter log) {
            this.log = log;
        }

    }
}
