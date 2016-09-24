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

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XQueryDefinitionsSupplier;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XIsolatedConnection;
import com.sun.star.sdbc.XStatement;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.awt.Robot;
import java.awt.event.InputEvent;
import util.DesktopTools;
import util.utils;

/**
 * Object implements the following interfaces :
 * <ul>
 * <li><code>::com::sun::star::accessibility::XAccessible</code></li>
 * <li><code>::com::sun::star::accessibility::XAccessibleContext
 *   </code></li>
 * <li><code>::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *   </code></li>
 * </ul>
 * <p>
 *
 * @see com.sun.star.accessibility.XAccessible
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessible
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 */
public class TableWindowAccessibility extends TestCase {
    XWindow xWindow = null;
    String aFile = "";
    XIsolatedConnection isolConnection = null;
    XComponent QueryComponent = null;
    String user = "";
    String password = "";

    /**
     * Creates a new DataSource and stores it. Creates a connection and using it
     * creates two tables in database. Creates a new query and adds it to
     * DefinitionContainer. Opens the QueryComponent.with loadComponentFromURL
     * and gets the object with the role PANEL and the implementation name that
     * contains TabelViewAccessibility
     *
     * @param Param
     *            test parameters
     * @param log
     *            writer to log information while testing
     * @return
     * @throws StatusException
     * @see TestEnvironment
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) throws Exception {
        XInterface oObj = null;

        Param.getMSF().createInstance("com.sun.star.sdb.DatabaseContext");
        Object oDBSource = Param.getMSF()
                .createInstance("com.sun.star.sdb.DataSource");
        Object newQuery = Param.getMSF().createInstance(
                "com.sun.star.sdb.QueryDefinition");
        Param.getMSF().createInstance("com.sun.star.awt.Toolkit");

        String mysqlURL = (String) Param.get("mysql.url");

        if (mysqlURL == null) {
            throw new StatusException(
                    Status.failed("Couldn't get 'mysql.url' from ini-file"));
        }

        user = (String) Param.get("jdbc.user");
        password = (String) Param.get("jdbc.password");

        if ((user == null) || (password == null)) {
            throw new StatusException(
                    Status.failed("Couldn't get 'jdbc.user' or 'jdbc.password' from ini-file"));
        }

        PropertyValue[] info = new PropertyValue[2];
        info[0] = new PropertyValue();
        info[0].Name = "user";
        info[0].Value = user;
        info[1] = new PropertyValue();
        info[1].Name = "password";
        info[1].Value = password;

        XPropertySet propSetDBSource = UnoRuntime.queryInterface(
                XPropertySet.class, oDBSource);

        propSetDBSource.setPropertyValue("URL", mysqlURL);
        propSetDBSource.setPropertyValue("Info", info);

        log.println("writing database file ...");
        XDocumentDataSource xDDS = UnoRuntime.queryInterface(
                XDocumentDataSource.class, oDBSource);
        XStorable store = UnoRuntime.queryInterface(XStorable.class,
                xDDS.getDatabaseDocument());
        aFile = utils.getOfficeTemp(Param.getMSF()) + "TableWindow.odb";
        log.println("... filename will be " + aFile);
        store.storeAsURL(aFile, new PropertyValue[] {});
        log.println("... done");

        isolConnection = UnoRuntime.queryInterface(XIsolatedConnection.class,
                oDBSource);

        final String tbl_name1 = "tst_table1";
        final String tbl_name2 = "tst_table2";
        final String col_name1 = "id1";
        final String col_name2 = "id2";

        util.utils.waitForEventIdle(Param.getMSF());
        XConnection connection = isolConnection.getIsolatedConnection(user, password);
        XStatement statement = connection.createStatement();
        statement.executeUpdate("drop table if exists " + tbl_name1);
        statement.executeUpdate("drop table if exists " + tbl_name2);
        statement.executeUpdate("create table " + tbl_name1 + " ("
                + col_name1 + " int)");
        statement.executeUpdate("create table " + tbl_name2 + " ("
                + col_name2 + " int)");

        XQueryDefinitionsSupplier querySuppl = UnoRuntime.queryInterface(
                XQueryDefinitionsSupplier.class, oDBSource);

        XNameAccess defContainer = querySuppl.getQueryDefinitions();

        XPropertySet queryProp = UnoRuntime.queryInterface(XPropertySet.class,
                newQuery);

        final String query = "select * from " + tbl_name1 + ", " + tbl_name2
                + " where " + tbl_name1 + "." + col_name1 + "=" + tbl_name2
                + "." + col_name2;
        queryProp.setPropertyValue("Command", query);

        XNameContainer queryContainer = UnoRuntime.queryInterface(
                XNameContainer.class, defContainer);

        queryContainer.insertByName("Query1", newQuery);
        store.store();
        connection.close();

        PropertyValue[] loadProps = new PropertyValue[3];
        loadProps[0] = new PropertyValue();
        loadProps[0].Name = "QueryDesignView";
        loadProps[0].Value = Boolean.TRUE;

        loadProps[1] = new PropertyValue();
        loadProps[1].Name = "CurrentQuery";
        loadProps[1].Value = "Query1";

        loadProps[2] = new PropertyValue();
        loadProps[2].Name = "DataSource";
        loadProps[2].Value = oDBSource;

        QueryComponent = DesktopTools.loadDoc(Param.getMSF(),
                ".component:DB/QueryDesign", loadProps);

        xWindow = UnoRuntime.queryInterface(XModel.class, QueryComponent)
                .getCurrentController().getFrame().getContainerWindow();

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot,
                Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                AccessibleRole.PANEL, "", "TableWindowAccessibility");

        log.println("ImplementationName " + util.utils.getImplName(oObj));

        log.println("creating TestEnvironment ... done");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        util.utils.waitForEventIdle(Param.getMSF());

        XAccessibleComponent accComp = UnoRuntime.queryInterface(
                XAccessibleComponent.class, oObj);

        final Point point = accComp.getLocationOnScreen();

        tEnv.addObjRelation(
                "EventProducer",
                new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                    public void fireEvent() {
                        try {
                            Robot rob = new Robot();
                            rob.mouseMove(point.X + 2, point.Y + 7);
                            rob.mousePress(InputEvent.BUTTON1_MASK);
                            rob.mouseMove(point.X + 400, point.Y);
                            rob.mouseRelease(InputEvent.BUTTON1_MASK);
                        } catch (java.awt.AWTException e) {
                            System.out.println("desired child doesn't exist");
                        }
                    }
                });

        return tEnv;
    } // finish method getTestEnvironment

    /**
     * Closes all open documents.
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        try {

            log.println("closing QueryComponent ...");
            DesktopTools.closeDoc(QueryComponent);
            log.println("... done");
            XMultiServiceFactory xMSF = Param.getMSF();
            Object sfa = xMSF
                    .createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess xSFA = UnoRuntime.queryInterface(
                    XSimpleFileAccess.class, sfa);
            log.println("deleting database file");
            xSFA.kill(aFile);
            log.println("Could delete file " + aFile + ": "
                    + !xSFA.exists(aFile));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
