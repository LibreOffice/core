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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XSingleSelectQueryAnalyzer;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sdb.DataSource</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::sdbc::RowSet</code></li>
 *  <li> <code>com::sun::star::sdbcx::XRowLocate</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSetUpdate</code></li>
 *  <li> <code>com::sun::star::util::XCancellable</code></li>
 *  <li> <code>com::sun::star::sdbc::XParameters</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSetMetaDataSupplier</code></li>
 *  <li> <code>com::sun::star::sdbcx::XDeleteRows</code></li>
 *  <li> <code>com::sun::star::sdbc::XCloseable</code></li>
 *  <li> <code>com::sun::star::sdbcx::XColumnsSupplier</code></li>
 *  <li> <code>com::sun::star::sdb::XResultSetAccess</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSet</code></li>
 *  <li> <code>com::sun::star::sdbc::XColumnLocate</code></li>
 *  <li> <code>com::sun::star::sdbc::XRowSet</code></li>
 *  <li> <code>com::sun::star::sdb::RowSet</code></li>
 *  <li> <code>com::sun::star::sdbc::XRowUpdate</code></li>
 *  <li> <code>com::sun::star::sdb::XRowSetApproveBroadcaster</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::sdbc::XRow</code></li>
 *  <li> <code>com::sun::star::sdbc::XWarningsSupplier</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::sdbcx::ResultSet</code></li>
 *  <li> <code>com::sun::star::sdbc::ResultSet</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> TestDB/TestDB.dbf </b> : the database file with some
 *    predefined fields described in <code>util.DBTools</code>.
 *    The copy of this file is always made in temp directory for
 *    testing purposes.</li>
 * </ul> <p>
 * The following parameters in ini-file used by this test:
 * <ul>
 *   <li><code>test.db.url</code> - URL to MySQL database.
 *   For example: <code>mysql://mercury:3306/api_current</code></li>
 *   <li><code>test.db.user</code> - user for MySQL database</li>
 *   <li><code>test.db.password</code> - password for MySQL database</li>
 * </ul><p>
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
public class OSingleSelectQueryComposer extends TestCase {

    String tableName = null;
    DBTools.DataSourceInfo srcInf = null;
    boolean isMySQLDB = false;
    protected final static String dbSourceName = "OSingleSelectQueryComposerDataSource";
    public XConnection conn = null;


    protected void initialize ( TestParameters Param, PrintWriter log)
        throws StatusException {

    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    *
    *     Object relations created :
    * <ul>SingleSelectQueryAnalyzer

    * <li> <code>'xComposer'</code> for
    *      {@link ifc.sdb._XSingleSelectQueryAnalyzer} interface
    * <li> <code>'xQueryAna'</code> for
    *      {@link ifc.sdb._XSingleSelectQueryComposer} interface
    * <li> <code>'xProp'</code> for
    *      {@link ifc.sdb._XSingleSelectQueryComposer} interface
    * <li> <code>'colName'</code> for
    *      {@link ifc.sdb._XSingleSelectQueryComposer} interface
    * </ul>
    *
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {

        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null ;
        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();

              XNameAccess xNameAccess = UnoRuntime.queryInterface(
                        XNameAccess.class,
                        xMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
            // we use the first datasource
            XDataSource xDS = UnoRuntime.queryInterface(
                    XDataSource.class, xNameAccess.getByName( "Bibliography" ));

            log.println("check XMultiServiceFactory");
            XMultiServiceFactory xConn = UnoRuntime.queryInterface(XMultiServiceFactory.class,
            xDS.getConnection(new String(),new String()));

            log.println("check getAvailableServiceNames");
            String[] sServiceNames = xConn.getAvailableServiceNames();
            if (! sServiceNames[0].equals("com.sun.star.sdb.SingleSelectQueryComposer"))
            {
                log.println("Service 'SingleSelectQueryComposer' not supported");
            }

            oInterface = xConn.createInstance( sServiceNames[0]);

            if (oInterface == null) {
                log.println("Service wasn't created") ;
                throw new StatusException("Service wasn't created",
                    new NullPointerException()) ;
            }

            Object oRowSet = xMSF.createInstance("com.sun.star.sdb.RowSet") ;

            XPropertySet xSetProp = UnoRuntime.queryInterface
                (XPropertySet.class, oRowSet) ;

            xSetProp.setPropertyValue("DataSourceName", "Bibliography") ;
            xSetProp.setPropertyValue("Command", "biblio") ;
            xSetProp.setPropertyValue("CommandType",
                new Integer(CommandType.TABLE)) ;

            com.sun.star.sdbc.XRowSet xORowSet = UnoRuntime.queryInterface(com.sun.star.sdbc.XRowSet.class,
            oRowSet) ;

            xORowSet.execute() ;

            XColumnsSupplier xColSup = UnoRuntime.queryInterface(XColumnsSupplier.class, oRowSet);

            XNameAccess xCols = xColSup.getColumns();

            XPropertySet xCol = (XPropertySet) AnyConverter.toObject(
                                new Type(XPropertySet.class),
                                xCols.getByName(xCols.getElementNames()[0]));

            XSingleSelectQueryAnalyzer xQueryAna = UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class,
             oInterface);

            // XSingleSelectQueryComposer
            XSingleSelectQueryComposer xComposer = UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,
              xQueryAna);
            xQueryAna.setQuery("SELECT * FROM \"biblio\"");

            oObj = (XInterface) oInterface;
            log.println("ImplementationName: " + utils.getImplName(oObj));

            log.println( "    creating a new environment for object" );
            TestEnvironment tEnv = new TestEnvironment( oObj );

            // for XSingleSelectQueryAnalyzer
            tEnv.addObjRelation("xComposer", xComposer);

            // for XSingleSelectQueryComposer
            tEnv.addObjRelation("xQueryAna", xQueryAna);

            tEnv.addObjRelation("xProp", xCol);
            tEnv.addObjRelation("colName", xCols.getElementNames()[0]);

          return tEnv;

        } catch(com.sun.star.uno.Exception e) {
            log.println("Can't create object" );
            e.printStackTrace(log) ;
            throw new StatusException("Can't create object", e) ;
        }

    } // finish method getTestEnvironment

    /**
    * Closes connection of <code>RowSet</code> instance created.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
    }
}
