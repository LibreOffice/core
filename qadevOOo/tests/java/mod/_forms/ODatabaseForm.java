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
package mod._forms;

import ifc.form._XDatabaseParameterBroadcaster;
import ifc.sdb._XCompletedExecution;

import java.io.PrintWriter;
import java.util.ArrayList;
import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DBTools;
import util.DrawTools;
import util.FormTools;
import util.WriterTools;
import util.utils;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.form.DatabaseParameterEvent;
import com.sun.star.form.XForm;
import com.sun.star.form.XLoadable;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.ParametersRequest;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.form.component.DatabaseForm</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::script::XEventAttacherManager</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::sdbcx::ResultSet</code></li>
 *  <li> <code>com::sun::star::container::XChild</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSetUpdate</code></li>
 *  <li> <code>com::sun::star::sdb::XResultSetAccess</code></li>
 *  <li> <code>com::sun::star::form::FormComponent</code></li>
 *  <li> <code>com::sun::star::form::component::DataForm</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSetMetaDataSupplier</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::sdbcx::XDeleteRows</code></li>
 *  <li> <code>com::sun::star::sdb::RowSet</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::sdbc::XRowSet</code></li>
 *  <li> <code>com::sun::star::sdbc::XRowUpdate</code></li>
 *  <li> <code>com::sun::star::form::XLoadable</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::container::XIndexReplace</code></li>
 *  <li> <code>com::sun::star::io::XPersistObject</code></li>
 *  <li> <code>com::sun::star::container::XNameReplace</code></li>
 *  <li> <code>com::sun::star::container::XIndexContainer</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::sdbc::XParameters</code></li>
 *  <li> <code>com::sun::star::util::XCancellable</code></li>
 *  <li> <code>com::sun::star::form::XReset</code></li>
 *  <li> <code>com::sun::star::sdbc::XCloseable</code></li>
 *  <li> <code>com::sun::star::sdbcx::XColumnsSupplier</code></li>
 *  <li> <code>com::sun::star::sdb::XRowSetApproveBroadcaster</code></li>
 *  <li> <code>com::sun::star::sdbc::ResultSet</code></li>
 *  <li> <code>com::sun::star::sdbc::XResultSet</code></li>
 *  <li> <code>com::sun::star::sdbc::XRow</code></li>
 *  <li> <code>com::sun::star::sdbc::XColumnLocate</code></li>
 *  <li> <code>com::sun::star::awt::XTabControllerModel</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::form::XSubmit</code></li>
 *  <li> <code>com::sun::star::form::component::HTMLForm</code></li>
 *  <li> <code>com::sun::star::sdbcx::XRowLocate</code></li>
 *  <li> <code>com::sun::star::sdbc::XWarningsSupplier</code></li>
 *  <li> <code>com::sun::star::container::XNameContainer</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::sdbc::RowSet</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * The following parameters in ini-file used by this test:
 * <ul>
 *   <li><code>test.db.url</code> - URL to MySQL database.
 *   For example: <code>mysql://mercury:3306/api_current</code></li>
 *   <li><code>test.db.user</code> - user for MySQL database</li>
 *   <li><code>test.db.password</code> - password for MySQL database</li>
 * </ul><p>
 *
 * @see com.sun.star.script.XEventAttacherManager
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.sdbcx.ResultSet
 * @see com.sun.star.container.XChild
 * @see com.sun.star.sdbc.XResultSetUpdate
 * @see com.sun.star.sdb.XResultSetAccess
 * @see com.sun.star.form.FormComponent
 * @see com.sun.star.form.component.DataForm
 * @see com.sun.star.sdbc.XResultSetMetaDataSupplier
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.sdbcx.XDeleteRows
 * @see com.sun.star.sdb.RowSet
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.sdbc.XRowSet
 * @see com.sun.star.sdbc.XRowUpdate
 * @see com.sun.star.form.XLoadable
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.container.XIndexReplace
 * @see com.sun.star.io.XPersistObject
 * @see com.sun.star.container.XNameReplace
 * @see com.sun.star.container.XIndexContainer
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.sdbc.XParameters
 * @see com.sun.star.util.XCancellable
 * @see com.sun.star.form.XReset
 * @see com.sun.star.sdbc.XCloseable
 * @see com.sun.star.sdbcx.XColumnsSupplier
 * @see com.sun.star.sdb.XRowSetApproveBroadcaster
 * @see com.sun.star.sdbc.ResultSet
 * @see com.sun.star.sdbc.XResultSet
 * @see com.sun.star.sdbc.XRow
 * @see com.sun.star.sdbc.XColumnLocate
 * @see com.sun.star.awt.XTabControllerModel
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.form.XSubmit
 * @see com.sun.star.form.component.HTMLForm
 * @see com.sun.star.sdbcx.XRowLocate
 * @see com.sun.star.sdbc.XWarningsSupplier
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.sdbc.RowSet
 * @see ifc.script._XEventAttacherManager
 * @see ifc.container._XElementAccess
 * @see ifc.sdbcx._ResultSet
 * @see ifc.container._XChild
 * @see ifc.sdbc._XResultSetUpdate
 * @see ifc.sdb._XResultSetAccess
 * @see ifc.form._FormComponent
 * @see ifc.form.component._DataForm
 * @see ifc.sdbc._XResultSetMetaDataSupplier
 * @see ifc.container._XEnumerationAccess
 * @see ifc.sdbcx._XDeleteRows
 * @see ifc.sdb._RowSet
 * @see ifc.lang._XComponent
 * @see ifc.sdbc._XRowSet
 * @see ifc.sdbc._XRowUpdate
 * @see ifc.form._XLoadable
 * @see ifc.container._XNamed
 * @see ifc.container._XIndexReplace
 * @see ifc.io._XPersistObject
 * @see ifc.container._XNameReplace
 * @see ifc.container._XIndexContainer
 * @see ifc.container._XNameAccess
 * @see ifc.sdbc._XParameters
 * @see ifc.util._XCancellable
 * @see ifc.form._XReset
 * @see ifc.sdbc._XCloseable
 * @see ifc.sdbcx._XColumnsSupplier
 * @see ifc.sdb._XRowSetApproveBroadcaster
 * @see ifc.sdbc._ResultSet
 * @see ifc.sdbc._XResultSet
 * @see ifc.sdbc._XRow
 * @see ifc.sdbc._XColumnLocate
 * @see ifc.awt._XTabControllerModel
 * @see ifc.container._XIndexAccess
 * @see ifc.form._XSubmit
 * @see ifc.form.component._HTMLForm
 * @see ifc.sdbcx._XRowLocate
 * @see ifc.sdbc._XWarningsSupplier
 * @see ifc.container._XNameContainer
 * @see ifc.beans._XPropertyAccess
 * @see ifc.beans._XPropertyContainer
 * @see ifc.beans._XPropertySet
 * @see ifc.sdbc._RowSet
 */
public class ODatabaseForm extends TestCase {
    protected final static String dbSourceName = "ODatabaseFormDataSource";
    private static int uniqueSuffix = 0;
    private static String origDB = null;
    private static String tmpDir = null;
    protected XTextDocument xTextDoc = null;
    private DBTools dbTools = null;
    String tableName = null;
    DBTools.DataSourceInfo srcInf = null;
    boolean isMySQLDB = false;
    protected XConnection conn = null;
    private Object dbSrc = null;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        //log.println( "creating a draw document" );
        //xTextDoc = WriterTools.createTextDoc(t((XMultiServiceFactory) Param.getMSF));
        tmpDir = utils.getOfficeTemp(((XMultiServiceFactory) tParam.getMSF()));

        origDB = util.utils.getFullTestDocName("TestDB/testDB.dbf");

        dbTools = new DBTools( (XMultiServiceFactory)tParam.getMSF(), log );

        // creating DataSource and registering it in DatabaseContext
        String dbURL = (String) tParam.get("test.db.url");
        String dbUser = (String) tParam.get("test.db.user");
        String dbPassword = (String) tParam.get("test.db.password");

        log.println("Creating and registering DataSource ...");
        srcInf = dbTools.newDataSourceInfo();

        if ((dbURL != null) && (dbUser != null) && (dbPassword != null)) {
            isMySQLDB = true;
            log.println("dbURL = " + dbURL);
            log.println("dbUSER = " + dbUser);
            log.println("dbPASSWORD = " + dbPassword);

            //DataSource for mysql db
            try {
                tableName = "soffice_test_table";
                srcInf.URL = "jdbc:" + dbURL;
                srcInf.IsPasswordRequired = new Boolean(true);
                srcInf.Password = dbPassword;
                srcInf.User = dbUser;

                PropertyValue[] propInfo = new PropertyValue[1];
                propInfo[0] = new PropertyValue();
                propInfo[0].Name = "JavaDriverClass";
//                propInfo[0].Value = "org.gjt.mm.mysql.Driver";
                propInfo[0].Value = "util.dddriver.Driver";
                srcInf.Info = propInfo;

                dbSrc = srcInf.getDataSourceService();
                dbTools.reRegisterDB(dbSourceName, dbSrc);
            } catch (com.sun.star.uno.Exception e) {
                log.println("Error while object test initialization :");
                e.printStackTrace(log);
                throw new StatusException("Error while object test" +
                                          " initialization", e);
            }
        } else {
            //DataSource for sdbc db
            try {
                String myDbUrl = "sdbc:dbase:" + DBTools.dirToUrl(tmpDir);
                srcInf.URL = myDbUrl;

                log.println("try to register '"+myDbUrl+"' as '"+dbSourceName+"'");

                dbSrc = srcInf.getDataSourceService();
                dbTools.reRegisterDB(dbSourceName, dbSrc);
            } catch (com.sun.star.uno.Exception e) {
                log.println("Error while object test initialization :");
                e.printStackTrace(log);
                throw new StatusException(
                        "Error while object test initialization", e);
            }

            String oldF = null;
            String newF = null;

            do {
                tableName = "ODatabaseForm_tmp" + uniqueSuffix;
                oldF = utils.getFullURL(origDB);
                newF = utils.getOfficeTemp((XMultiServiceFactory) tParam.getMSF()) + tableName +
                       ".dbf";
            } while (!utils.tryOverwriteFile(((XMultiServiceFactory) tParam.getMSF()), oldF, newF) &&
                     (uniqueSuffix++ < 50));
        }
    }

    /**
     *  *    creating a Testenvironment for the interfaces to be tested
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        if (xTextDoc != null) {
            try {
                XCloseable closer = UnoRuntime.queryInterface(
                                            XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }

            log.println("Existing document disposed");
        }

        log.println("creating a text document");
        xTextDoc = WriterTools.createTextDoc(((XMultiServiceFactory) Param.getMSF()));

        //initialize test table
        if (isMySQLDB) {
            try {
                dbTools.initTestTableUsingJDBC(tableName, srcInf);
            } catch (java.sql.SQLException e) {
                e.printStackTrace(log);
                throw new StatusException(Status.failed("Couldn't " + " init test table. SQLException..."));
            } catch (java.lang.ClassNotFoundException e) {
                throw new StatusException(Status.failed("Couldn't " + "register mysql driver"));
            }
        }

        XInterface oObj = null;
        XShapes oShapes = null;
        XInterface oInstance = null;
        XConnection connection = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        XNameContainer forms = FormTools.getForms(WriterTools.getDrawPage(
                                                          xTextDoc));

        try {
            String[] formNames = forms.getElementNames();

            for (int i = 0; i < formNames.length; i++) {
                log.println("Removing form '" + formNames[i] + "' ...");
                forms.removeByName(formNames[i]);
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
        }

        String[] formNames = forms.getElementNames();
        FormTools.insertForm(xTextDoc, forms, "MyForm");
        formNames = forms.getElementNames();

        XLoadable formLoader = null;

        try {
            formLoader = FormTools.bindForm(xTextDoc, "MyForm", dbSourceName,
                                            tableName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Cann't bind the form to source '" + dbSourceName +
                        "', table '" + tableName + "' :");
            e.printStackTrace(log);
            throw new StatusException("Cann't bind a form", e);
        }


        // DEBUG
        log.println("Forms before adding controls : ");
        formNames = forms.getElementNames();

        for (int i = 0; i < formNames.length; i++) {
            log.println("    '" + formNames[i] + "'");
        }

        XControlShape shape1 = null;
        XControlShape shape2 = null;

        try {

            log.println("Elements in the 'MyForm' :");

            XIndexAccess formElements1 = UnoRuntime.queryInterface(
                                                 XIndexAccess.class,
                                                 forms.getByName("MyForm"));

            for (int i = 0; i < formElements1.getCount(); i++) {
                XNamed elemName = UnoRuntime.queryInterface(
                                          XNamed.class,
                                          formElements1.getByIndex(i));
                log.println("   '" + elemName.getName() + "'");
            }


            // END DEBUG
            //put something on the drawpage
            log.println("inserting some ControlShapes");
            oShapes = DrawTools.getShapes(WriterTools.getDrawPage(xTextDoc));
            shape1 = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                                  1000, "CommandButton");
            shape2 = FormTools.createControlShape(xTextDoc, 5000, 3500, 7500,
                                                  5000, "TextField");

            XControlShape shape3 = FormTools.createControlShape(xTextDoc, 2000,
                                                                1500, 1000,
                                                                1000,
                                                                "CheckBox");
            oShapes.add(shape1);
            oShapes.add(shape2);
            oShapes.add(shape3);
        } catch (Exception e) {
            e.printStackTrace(log);
        }

        log.println("Forms after adding controls : ");
        formNames = forms.getElementNames();

        for (int i = 0; i < formNames.length; i++) {
            log.println("    '" + formNames[i] + "'");
        }

        try {
            log.println("Elements in the 'MyForm' :");

            XIndexAccess formElements1 = UnoRuntime.queryInterface(
                                                 XIndexAccess.class,
                                                 forms.getByName("MyForm"));

            for (int i = 0; i < formElements1.getCount(); i++) {
                XNamed elemName = UnoRuntime.queryInterface(
                                          XNamed.class,
                                          formElements1.getByIndex(i));
                log.println("   '" + elemName.getName() + "'");
            }
        } catch (Exception e) {
            e.printStackTrace(log);
        }

        formLoader.load();

        try {
            oObj = (XForm) AnyConverter.toObject(new Type(XForm.class),
                                                 (FormTools.getForms(
                                                         WriterTools.getDrawPage(
                                                                 xTextDoc)))
                                                     .getByName("MyForm"));

            XPropertySet xSetProp = UnoRuntime.queryInterface(
                                            XPropertySet.class, oObj);
            connection = (XConnection) AnyConverter.toObject(
                                 new Type(XConnection.class),
                                 xSetProp.getPropertyValue("ActiveConnection"));
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get Form");
            e.printStackTrace(log);
        }


        // get a control
        oInstance = FormTools.createControl(xTextDoc, "TextField");

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        // adding relation for closing connection while environment disposing.
        this.conn = connection;

        // adding relation for XSubmit
        XControlModel the_Model = shape2.getControl();
        XControlAccess the_access = UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());
        XControl cntrl = null;

        //now get the OEditControl
        try {
            cntrl = the_access.getControl(the_Model);
            log.println(cntrl.getClass().getName());
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Couldn't get OEditControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get OEditControl", e);
        }

        XResultSet the_set = UnoRuntime.queryInterface(
                                     XResultSet.class, oObj);

        try {
            the_set.first();
        } catch (SQLException e) {
            log.println("Cann't move cursor to the first row.");
            e.printStackTrace();
            throw new StatusException("Can't move cursor to the first row.", e);
        }

        tEnv.addObjRelation("Model1", shape1.getControl());
        tEnv.addObjRelation("Model2", shape2.getControl());


        // adding an object for XNameReplace testing
        log.println("adding oInstace as obj relation to environment");
        tEnv.addObjRelation("INSTANCE", oInstance);


        // INDEX : _XNameContainer
        log.println("adding INDEX as obj relation to environment");
        tEnv.addObjRelation("INDEX", "0");


        // INDEX : _XNameReplace
        log.println("adding NameReplaceIndex as obj relation to environment");
        tEnv.addObjRelation("XNameReplaceINDEX", "2");


        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println("adding INSTANCEn as obj relation to environment");

        //XComponent xComp = (XComponent)
        //    UnoRuntime.queryInterface(XComponent.class, xDrawDoc);
        String tc = (String) Param.get("THRCNT");
        int THRCNT = 1;

        if (tc != null) {
            THRCNT = Integer.parseInt(tc);
        }

        for (int n = 1; n < (2 * (THRCNT + 1)); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");
            tEnv.addObjRelation("INSTANCE" + n,
                                FormTools.createControl(xTextDoc, "CheckBox"));
        }


        // adding relation for XNameContainer
        tEnv.addObjRelation("XNameContainer.AllowDuplicateNames", new Object());


        // adding relation for XPersistObject
        tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component.Form");

        if (the_set != null) {
            log.println("The Form has a not empty ResultSet");
        }

        // Adding obj relation for XRowSetApproveBroadcaster test
        final XResultSet xResSet = UnoRuntime.queryInterface(
                                           XResultSet.class, oObj);
        final XResultSetUpdate xResSetUpdate = UnoRuntime.queryInterface(
                                                       XResultSetUpdate.class,
                                                       oObj);
        final XRowSet xRowSet = UnoRuntime.queryInterface(
                                        XRowSet.class, oObj);
        final PrintWriter logF = log;
        tEnv.addObjRelation("XRowSetApproveBroadcaster.ApproveChecker",
                            new ifc.sdb._XRowSetApproveBroadcaster.RowSetApproveChecker() {
            public void moveCursor() {
                try {
                    xResSet.beforeFirst();
                    xResSet.afterLast();
                } catch (com.sun.star.sdbc.SQLException e) {
                    logF.println("### _XRowSetApproveBroadcaster." + "RowSetApproveChecker.moveCursor() :");
                    e.printStackTrace(logF);
                }
            }

            public RowChangeEvent changeRow() {
                try {
                    xResSet.first();

                    XRowUpdate row = UnoRuntime.queryInterface(
                                             XRowUpdate.class, xResSet);
                    row.updateString(1, "1");
                    xResSetUpdate.updateRow();
                } catch (com.sun.star.sdbc.SQLException e) {
                    logF.println("### _XRowSetApproveBroadcaster." + "RowSetApproveChecker.changeRow() :");
                    e.printStackTrace(logF);
                }

                RowChangeEvent ev = new RowChangeEvent();
                ev.Action = com.sun.star.sdb.RowChangeAction.UPDATE;
                ev.Rows = 1;

                return ev;
            }

            public void changeRowSet() {
                try {
                    xRowSet.execute();
                } catch (com.sun.star.sdbc.SQLException e) {
                    logF.println("### _XRowSetApproveBroadcaster." + "RowSetApproveChecker.changeRowSet() :");
                    e.printStackTrace(logF);
                }
            }
        });


        // Adding relation for XColumnLocate test
        tEnv.addObjRelation("XColumnLocate.ColumnName", DBTools.TST_STRING_F);

        // Adding relation for XParameters ifc test
        ArrayList<Object> params = new ArrayList<Object>();


        /*****  statement parameter types and their initial
                values must be added here as relation. */
        params.add(new String("SAU99")) ;
        params.add(new Boolean(false)) ;
        params.add(new Byte((byte) 123)) ;
        params.add(new Short((short) 234)) ;
        params.add(new Integer(12345)) ;
        params.add(new Long(23456)) ;
        params.add(new Float(1.234)) ;
        params.add(new Double(2.345)) ;
        params.add(new byte[] {1, 2, 3}) ;
        Date d = new Date();
        d.Day = 26; d.Month = 1; d.Year = 2001;
        params.add(d) ;
        Time t = new Time();
        t.Hours = 1; t.HundredthSeconds = 12; t.Minutes = 25; t.Seconds = 14;
        params.add(t) ;
        DateTime dt = new DateTime();
        dt.Day = 26; dt.Month = 1; dt.Year = 2001; dt.Hours = 1;
        dt.HundredthSeconds = 12; dt.Minutes = 25; dt.Seconds = 14;
        params.add(dt) ;
        tEnv.addObjRelation("XParameters.ParamValues", params);

        // Adding relation for XCompletedExecution
        tEnv.addObjRelation("InteractionHandlerChecker", new InteractionHandlerImpl());

        // Adding for XWarningSupplier
        tEnv.addObjRelation("CheckWarningsSupplier", new Boolean(isMySQLDB));

        // Adding relation for XDatabaseParameterBroadcaster
        tEnv.addObjRelation("ParameterListenerChecker", new ODatabaseForm.ParameterListenerImpl());
        XPropertySet xSetProp = UnoRuntime.queryInterface
            (XPropertySet.class, oObj) ;
        try {
            xSetProp.setPropertyValue("DataSourceName", dbSourceName) ;
            if(isMySQLDB) {
                xSetProp.setPropertyValue("Command", "SELECT Column0  FROM soffice_test_table  WHERE ( (  Column0 = :param1 ) )");
            }
            else {
                xSetProp.setPropertyValue("Command", "SELECT \"_TEXT\" FROM \"ODatabaseForm_tmp0\" WHERE ( ( \"_TEXT\" = :param1 ) )");
            }

            xSetProp.setPropertyValue("CommandType",
                new Integer(CommandType.COMMAND)) ;
        }
        catch(Exception e) {
        }

        // Adding relation for XResultSetUpdate
        final XRowUpdate xRowUpdate = UnoRuntime.queryInterface(
                                              XRowUpdate.class, oObj);
        final XRow xRow = UnoRuntime.queryInterface(XRow.class, oObj);

        tEnv.addObjRelation("XResultSetUpdate.UpdateTester",
                            new ifc.sdbc._XResultSetUpdate.UpdateTester() {
            String lastUpdate = null;

            public int rowCount() throws SQLException {
                int prevPos = xResSet.getRow();
                xResSet.last();

                int count = xResSet.getRow();
                xResSet.absolute(prevPos);

                return count;
            }

            public void update() throws SQLException {
                lastUpdate = xRow.getString(1);
                lastUpdate += "_";
                xRowUpdate.updateString(1, lastUpdate);
            }

            public boolean wasUpdated() throws SQLException {
                String getStr = xRow.getString(1);

                return lastUpdate.equals(getStr);
            }

            public int currentRow() throws SQLException {
                return xResSet.getRow();
            }
        });

        // Adding relations for XRow as a Vector with all data
        // of current row of RowSet.

        ArrayList<Object> rowData = new ArrayList<Object>();

        for (int i = 0; i < DBTools.TST_TABLE_VALUES[0].length; i++) {
            rowData.add(DBTools.TST_TABLE_VALUES[0][i]);
        }

        tEnv.addObjRelation("CurrentRowData", rowData);

        // Adding relation for XRowUpdate
        XRow row = UnoRuntime.queryInterface(XRow.class, oObj);
        tEnv.addObjRelation("XRowUpdate.XRow", row);


        tEnv.addObjRelation("XPropertyContainer.propertyNotRemovable", "Cycle");

        PropertyValue propVal = new PropertyValue();
        propVal.Name = "Name";
        propVal.Value = "Text since XPropertyAccess";

        tEnv.addObjRelation("XPropertyAccess.propertyToChange", propVal);

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * Closes connection of <code>RowSet</code> instance created.
    */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("closing connection...");
        try {
            conn.close();
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't close the connection");
            e.printStackTrace(log);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Connection was already closed. It's OK.");
        }


        log.println("closing data source...");
        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, dbSrc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close data source");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close data source");
        }


        log.println("closing document...");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }

        log.println("revoking data source...");
        try {
            dbTools.revokeDB(dbSourceName);
        } catch (com.sun.star.container.NoSuchElementException e){
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error while object test cleaning up :");
            e.printStackTrace(log);
            throw new StatusException("Error while object test cleaning up", e);
        }
    }

    /**
     * Implementation of interface _XDatabaseParameterBroadcaster.CheckParameterListener
     * for the XDatabaseParameterBroadcaster test
     * @see ifc.form._XDatabaseParameterBroadcaster
     */
    public class ParameterListenerImpl implements _XDatabaseParameterBroadcaster.CheckParameterListener {
        boolean listenerWasCalled = false;
        PrintWriter log = new PrintWriter(System.out);

        /**
         * Return true, if the listener was called, false otherwise.
         * @return True, if any other method of the listener was called.
         */
        public boolean checkListener() {
            return listenerWasCalled;
        }

        /**
         * Take the DataBaseParameterEvent and fill it with a meaningful value.
         * @param e The database parameter that will be filled with a value.
         * @return True, if the value could be filled.
         */
        public boolean approveParameter(DatabaseParameterEvent e) {
            log.println("### ParameterListenerImpl: approve called.");
            XIndexAccess params = e.Parameters;
            int count = params.getCount();
            try {
                for(int i=0; i<count; i++) {
                    log.println("### _XDatabaseParameterBroadcaster.ParameterListenerImpl: Parameter "+i+": "+params.getByIndex(i));
                    XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(i));
                    log.println("### _XDatabaseParameterBroadcaster.ParameterListenerImpl: Parameter Name: '"+xProp.getPropertyValue("Name") + "' is set to Value '1'");
                    xProp.setPropertyValue("Value", new Integer(1));
                    listenerWasCalled = true;
                }
            }
            catch(Exception eI) {
                log.println("### _XDatabaseParameterBroadcaster.ParameterListenerImpl: Exception!");
                eI.printStackTrace(log);
                return false;
            }
            return true;
        }

        /**
         * Dummy implemetnation. Do nothing, just log
         * @param o Ignore.
         */
        public void disposing(EventObject o) {
            log.println("### _XDatabaseParameterBroadcaster.ParameterListenerImpl: disposing");
        }

        /**
         * Set a log writer, so messages go to log instead of Standard.out
         * @param log The log messages get printed to.
         */
        public void setLog(PrintWriter log) {
            this.log = log;
        }

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
            handlerWasUsed = true;

            Object o = xInteractionRequest.getRequest();
            ParametersRequest req = (ParametersRequest)o;
            XIndexAccess params = req.Parameters;
            int count = params.getCount();
            try {
                for(int i=0; i<count; i++) {
                    params.getByIndex(i);
                    log.println("### _XCompletedExecution.InteractionHandlerImpl: Parameter "+i+": "+params.getByIndex(i));
                    XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(i));
                    log.println("### _XCompletedExecution.InteractionHandlerImpl: Parameter Name: '"+xProp.getPropertyValue("Name") + "' is set to Value '1'");
                    xProp.setPropertyValue("Value", new Integer(1));
                    handlerWasUsed = true;
                }
            }
            catch(Exception eI) {
                log.println("### _XCompletedExecution.InteractionHandlerImpl: Exception!");
                eI.printStackTrace(log);
            }
        }

        public void setLog(PrintWriter log) {
            this.log = log;
        }

    }
} // finish class ODatabaseForm
