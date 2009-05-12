/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSingleSelectQueryComposer.java,v $
 * $Revision: 1.4 $
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

    private static int uniqueSuffix = 0 ;
    private DBTools dbTools     = null ;
    private static String origDB = null ;
    private PrintWriter log = null ;
    private static String tmpDir = null ;
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
    *      {@link ifc.sdb.XSingleSelectQueryAnalyzer} interface
    * <li> <code>'xQueryAna'</code> for
    *      {@link ifc.sdb.XSingleSelectQueryComposer} interface
    * <li> <code>'xProp'</code> for
    *      {@link ifc.sdb.XSingleSelectQueryComposer} interface
    * <li> <code>'colName'</code> for
    *      {@link ifc.sdb.XSingleSelectQueryComposer} interface
    * </ul>
    *
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {

        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null ;
        boolean envCreatedOK = false ;


        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();

              XNameAccess xNameAccess = (XNameAccess)UnoRuntime.queryInterface(
                        XNameAccess.class,
                        xMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
            // we use the first datasource
            XDataSource xDS = (XDataSource)UnoRuntime.queryInterface(
                    XDataSource.class, xNameAccess.getByName( "Bibliography" ));

            log.println("check XMultiServiceFactory");
            XMultiServiceFactory xConn = (XMultiServiceFactory)
                        UnoRuntime.queryInterface(XMultiServiceFactory.class,
                        xDS.getConnection(new String(),new String()));

            log.println("check getAvailableServiceNames");
            String[] sServiceNames = xConn.getAvailableServiceNames();
            if (! sServiceNames[0].equals("com.sun.star.sdb.SingleSelectQueryComposer"))
            {
                log.println("Service 'SingleSelectQueryComposer' not supported");
            }

            oInterface = (XInterface) xConn.createInstance( sServiceNames[0]);

            if (oInterface == null) {
                log.println("Service wasn't created") ;
                throw new StatusException("Service wasn't created",
                    new NullPointerException()) ;
            }

            Object oRowSet = xMSF.createInstance("com.sun.star.sdb.RowSet") ;

            XPropertySet xSetProp = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class, oRowSet) ;

            xSetProp.setPropertyValue("DataSourceName", "Bibliography") ;
            xSetProp.setPropertyValue("Command", "biblio") ;
            xSetProp.setPropertyValue("CommandType",
                new Integer(CommandType.TABLE)) ;

            com.sun.star.sdbc.XRowSet xORowSet = (com.sun.star.sdbc.XRowSet)
                UnoRuntime.queryInterface(com.sun.star.sdbc.XRowSet.class,
                oRowSet) ;

            xORowSet.execute() ;

            XColumnsSupplier xColSup = (XColumnsSupplier)
                    UnoRuntime.queryInterface(XColumnsSupplier.class, oRowSet);

            XNameAccess xCols = xColSup.getColumns();

            XPropertySet xCol = (XPropertySet) AnyConverter.toObject(
                                new Type(XPropertySet.class),
                                xCols.getByName(xCols.getElementNames()[0]));

            XSingleSelectQueryAnalyzer xQueryAna = (XSingleSelectQueryAnalyzer)
                     UnoRuntime.queryInterface(XSingleSelectQueryAnalyzer.class,
                     oInterface);

            // XSingleSelectQueryComposer
            XSingleSelectQueryComposer xComposer = (XSingleSelectQueryComposer)
                      UnoRuntime.queryInterface(XSingleSelectQueryComposer.class,
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

          envCreatedOK = true ;
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
