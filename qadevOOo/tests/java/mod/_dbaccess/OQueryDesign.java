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

//import com.sun.star.awt.XControl;
//import com.sun.star.awt.XControlModel;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdbc.SQLException;
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
import lib.StatusException;
import util.SOfficeFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XDataSource;

public class OQueryDesign extends TestCase {

    private static XDesktop xDesk;
    private static XFrame xFrame;
    private final String sDataSourceName = "Bibliography";
    private static XConnection xConn;
    private static XTextDocument xTextDoc;


    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        xDesk = UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory)Param.getMSF()) );
    }

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        XInterface oObj = null;

        XDispatchProvider aProv = UnoRuntime.queryInterface(XDispatchProvider.class,xDesk);

        XDispatch getting = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();

              XNameAccess xNameAccess = null;

            // we use the first datasource
            XDataSource xDS = null;
        try {
            xNameAccess = UnoRuntime.queryInterface(
                        XNameAccess.class,
                        xMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
        } catch (Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Could not get Databasecontext", ex );
        }
        try {
            xDS = UnoRuntime.queryInterface(
                    XDataSource.class, xNameAccess.getByName( "Bibliography" ));
        } catch (NoSuchElementException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Could not get XDataSource", ex );
        } catch (WrappedTargetException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Could not get XDataSource", ex );
        }
        try {
            xNameAccess = UnoRuntime.queryInterface(
                        XNameAccess.class,
                        xMSF.createInstance("com.sun.star.sdb.DatabaseContext"));
        } catch (Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Could not get DatabaseConext", ex );
        }

            log.println("check XMultiServiceFactory");

        try {
            xConn = xDS.getConnection(new String(), new String());
        } catch (SQLException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Could not get XConnection", ex );
        }

        log.println( "opening QueryDesign" );
        URL the_url = new URL();
        the_url.Complete = ".component:DB/QueryDesign";
        getting = aProv.queryDispatch(the_url,"Query",12);
        PropertyValue[] Args = new PropertyValue[2];
        PropertyValue param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        Args[0] = param1;
        PropertyValue param2 = new PropertyValue();
        param2.Name = "QueryDesignView";
        param2.Value = new Boolean(false);
        Args[1] = param2;
        param1.Name = "ActiveConnection";
        param1.Value = xConn;
        Args[1] = param2;
        getting.dispatch(the_url,Args);

        shortWait();

        Object oDBC = null;

        try {
            oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );
        }
        catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate DatabaseContext", e) ;
        }

        Object oDataSource = null;
        try{
            XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oDBC);
            oDataSource = xNA.getByName(sDataSourceName);
        } catch ( com.sun.star.container.NoSuchElementException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        } catch ( com.sun.star.lang.WrappedTargetException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        }
        UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);

//        Frame = xMod.getCurrentController().getFrame();

        xFrame = DesktopTools.getCurrentFrame(xMSF);

         SOfficeFactory SOF = null;

        SOF = SOfficeFactory.getFactory( xMSF );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Could not create document", e );
        }

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
        param3.Value = new Boolean(true);
        params[2] = param3;


        oObj = xFrame.getController();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //Adding ObjRelations for XInitialization
        tEnv.addObjRelation("XInitialization.args", params);

        //Object[] ExceptionParams = new Object[3];
        //ExceptionParams = params;
        //((PropertyValue) ExceptionParams[1]).Value = Frame;
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
        ExceptionParam3.Value = new Integer(17);//new Boolean(true);
        ExceptionParams[2] = ExceptionParam3;

        tEnv.addObjRelation("XInitialization.ExceptionArgs", ExceptionParams);

        tEnv.addObjRelation("Frame", xFrame);

        tEnv.addObjRelation("XInitialization.xIni", getUnititializedObj(Param));

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

    private XInitialization getUnititializedObj(TestParameters Param){
        // creating an object which ist not initialized

        // get a model of a DataSource
        Object oDBC = null;
        XMultiServiceFactory xMSF;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oDBC = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );
        }
        catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate DatabaseContext", e) ;
        }

        Object oDataSource = null;
        try{
            XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oDBC);
            oDataSource = xNA.getByName(sDataSourceName);
        } catch ( com.sun.star.container.NoSuchElementException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        } catch ( com.sun.star.lang.WrappedTargetException e){
            throw new StatusException("could not get '" + sDataSourceName + "'" , e) ;
        }

        XDocumentDataSource xDDS = UnoRuntime.queryInterface(XDocumentDataSource.class, oDataSource);
        XModel xMod = UnoRuntime.queryInterface(XModel.class, xDDS.getDatabaseDocument ());

        // get an intaces of QueryDesign
        Object oQueryDesign = null;
        try{
            oQueryDesign = xMSF.createInstance("com.sun.star.sdb.QueryDesign");
        }catch( com.sun.star.uno.Exception e ) {
            throw new StatusException("Could not instantiate QueryDesign", e) ;
        }

        XController xCont = UnoRuntime.queryInterface(XController.class, oQueryDesign);

        // marry them all
        xCont.attachModel(xMod);
        xMod.connectController(xCont);
        try{
            xMod.setCurrentController(xCont);
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new StatusException("Could not set controller", e) ;
        }

        //xCont.attachFrame(xFrame);

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

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(5000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

}    // finish class oDatasourceBrowser

