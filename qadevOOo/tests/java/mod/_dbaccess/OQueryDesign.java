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

import com.sun.star.sdbc.XConnection;
import com.sun.star.uno.Exception;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import util.SOfficeFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XDataSource;

public class OQueryDesign extends TestCase {

    private static XDesktop xDesk;
    private static XFrame xFrame;
    private static final String sDataSourceName = "Bibliography";
    private static XConnection xConn;
    private static XTextDocument xTextDoc;


    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        xDesk = UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop(Param.getMSF()) );
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        log.println( "creating a test environment" );

        XDispatchProvider aProv = UnoRuntime.queryInterface(XDispatchProvider.class,xDesk);
        XMultiServiceFactory xMSF = Param.getMSF();

        // we use the first datasource
        XNameAccess xNameAccess = UnoRuntime.queryInterface(
                    XNameAccess.class,
                    xMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
        XDataSource xDS = UnoRuntime.queryInterface(
                XDataSource.class, xNameAccess.getByName( "Bibliography" ));

        log.println("check XMultiServiceFactory");

        xConn = xDS.getConnection("", "");

        log.println( "opening QueryDesign" );
        URL the_url = new URL();
        the_url.Complete = ".component:DB/QueryDesign";
        XDispatch getting = aProv.queryDispatch(the_url,"Query",12);
        PropertyValue[] Args = new PropertyValue[2];
        PropertyValue param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        Args[0] = param1;
        PropertyValue param2 = new PropertyValue();
        param2.Name = "QueryDesignView";
        param2.Value = Boolean.FALSE;
        Args[1] = param2;
        param1.Name = "ActiveConnection";
        param1.Value = xConn;
        Args[1] = param2;
        getting.dispatch(the_url,Args);

        util.utils.waitForEventIdle(Param.getMSF());

        Object oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );

        Object oDataSource = null;
        XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oDBC);
        oDataSource = xNA.getByName(sDataSourceName);
        UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);

        xFrame = DesktopTools.getCurrentFrame(xMSF);

        SOfficeFactory SOF = null;

        SOF = SOfficeFactory.getFactory( xMSF );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );

        XModel xDocMod = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XFrame xTextFrame  = xDocMod.getCurrentController().getFrame();

        Object[] params = new Object[3];
        param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        params[0] = param1;
        param2 = new PropertyValue();
        param2.Name = "Frame";
        param2.Value = xTextFrame;
        params[1] = param2;
        PropertyValue param3 = new PropertyValue();
        param3.Name = "QueryDesignView";
        param3.Value = Boolean.TRUE;
        params[2] = param3;


        XInterface oObj = xFrame.getController();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //Adding ObjRelations for XInitialization
        tEnv.addObjRelation("XInitialization.args", params);

        Object[] ExceptionParams = new Object[3];
        PropertyValue ExceptionParam1 = new PropertyValue();
        ExceptionParam1.Name = "DataSourceName";
        ExceptionParam1.Value = "Bibliography2";
        ExceptionParams[0] = ExceptionParam1;
        PropertyValue ExceptionParam2 = new PropertyValue();
        ExceptionParam2.Name = "Frame";
        ExceptionParam2.Value = null;
        ExceptionParams[1] = ExceptionParam2;
        PropertyValue ExceptionParam3 = new PropertyValue();
        ExceptionParam3.Name = "QueryDesignView";
        ExceptionParam3.Value = Integer.valueOf(17);//Boolean.TRUE;
        ExceptionParams[2] = ExceptionParam3;

        tEnv.addObjRelation("XInitialization.ExceptionArgs", ExceptionParams);

        tEnv.addObjRelation("Frame", xFrame);

        tEnv.addObjRelation("XInitialization.xIni", getUnititializedObj(Param));

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

    private XInitialization getUnititializedObj(TestParameters Param) throws Exception {
        // creating an object which ist not initialized

        // get a model of a DataSource
        Object oDBC = null;
        XMultiServiceFactory xMSF;

        xMSF = Param.getMSF();
        oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );

        Object oDataSource = null;
        XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oDBC);
        oDataSource = xNA.getByName(sDataSourceName);

        XDocumentDataSource xDDS = UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);
        XModel xMod = UnoRuntime.queryInterface(XModel.class, xDDS.getDatabaseDocument ());

        // get an instance of QueryDesign
        Object oQueryDesign = xMSF.createInstance("com.sun.star.sdb.QueryDesign");

        XController xCont = UnoRuntime.queryInterface(XController.class, oQueryDesign);

        // marry them all
        xCont.attachModel(xMod);
        xMod.connectController(xCont);
        xMod.setCurrentController(xCont);

        return UnoRuntime.queryInterface(XInitialization.class, oQueryDesign);
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        try {
            xConn.close() ;
            DesktopTools.closeDoc(xFrame);
            DesktopTools.closeDoc(xTextDoc);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't close the connection") ;
            e.printStackTrace(log) ;
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Connection was already closed. It's OK.") ;
        }

    }

}    // finish class oDatasourceBrowser

