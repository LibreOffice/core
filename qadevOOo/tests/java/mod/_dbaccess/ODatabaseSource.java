/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XStorable;


import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.utils;

import com.sun.star.io.IOException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.task.XInteractionHandler;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdb.DataSource</code>. <p>
*
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sdb::XQueryDefinitionsSupplier</code></li>
*  <li> <code>com::sun::star::sdb::DataSource</code></li>
*  <li> <code>com::sun::star::sdb::XCompletedConnection</code></li>
*  <li> <code>com::sun::star::util::XFlushable</code></li>
*  <li> <code>com::sun::star::sdb::XBookmarksSupplier</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
*
* @see com.sun.star.sdb.XQueryDefinitionSupplier
* @see com.sun.star.sdb.XFormDocumentsSupplier
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.sdb.DataSource
* @see com.sun.star.sdb.XCompletedConnection
* @see com.sun.star.util.XFlushable
* @see com.sun.star.sdb.XReportDocumentsSupplier
* @see ifc.sdb._XQueryDefinitionSupplier
* @see ifc.sdb._XFormDocumentsSupplier
* @see ifc.beans._XPropertySet
* @see ifc.sdb._DataSource
* @see ifc.sdb._XCompletedConnection
* @see ifc.util._XFlushable
* @see ifc.sdb._XReportDocumentsSupplier
*/
public class ODatabaseSource extends TestCase {

    private static int uniqueSuffixStat = 0 ;

    private int uniqueSuffix = 0 ;
    private XNamingService xDBContextNameServ = null ;
    private String databaseName = null ;
    private XOfficeDatabaseDocument xDBDoc = null;

    /**
    * Assigns database's name unique suffix for this object depending
    * on static field. For names differs in different object threads.
    */
    protected void initialize ( TestParameters Param, PrintWriter log) {
        uniqueSuffix = uniqueSuffixStat++ ;
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing not longer needed docs... ");
        DesktopTools.closeDoc(xDBDoc);
    }



    /**
    * Creating a Testenvironment for the interfaces to be tested.
    *
    * Creates new <code>DataSource</code> which represents DBase database
    * located in temporary directory, and then registers it in service
    * <code>com.sun.star.sdb.DatabaseContext</code>.
    *
    *    Created object relations :
    * <ul>
    * <li> <code>'XCompletedConnection.Handler'</code> for interface test
    *      <code>XCompletedConnection</code> </li>
    * </ul>
    * <li> <code>'UserAndPassword'</code> for interface test
    *      <code>XIsolatedConnection</code> </li>
    * </ul>
    * @see com.sun.star.sdb.DatabaseContext
    * @see com.sun.star.sdb.DataSource
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null ;

        xMSF = (XMultiServiceFactory)Param.getMSF();
        try{
            oInterface = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );

            if (oInterface == null)
                throw new StatusException("Could not get service 'com.sun.star.sdb.DatabaseContext'", new Exception());

        }catch( Exception e ) {
            log.println("Could not get service 'com.sun.star.sdb.DatabaseContext'" );
            e.printStackTrace ();
            throw new StatusException("Service not available", e) ;
        }

        xDBContextNameServ = (XNamingService)
            UnoRuntime.queryInterface(XNamingService.class, oInterface) ;

        // retrieving temp directory for database
        String tmpDatabaseUrl = utils.getOfficeTempDir((XMultiServiceFactory)Param.getMSF());

        tmpDatabaseUrl = "sdbc:dbase:file:///" + tmpDatabaseUrl ;

        try{
            XInterface oDatabaseDoc = (XInterface) xMSF.createInstance
                                    ("com.sun.star.sdb.OfficeDatabaseDocument") ;

            if (oDatabaseDoc == null)
                throw new StatusException("Could not get service 'com.sun.star.sdb.OfficeDatabaseDocument'", new Exception());

            xDBDoc = (XOfficeDatabaseDocument) UnoRuntime.queryInterface(
                                                XOfficeDatabaseDocument.class,
                                                oDatabaseDoc);
        }
        catch( Exception e ) {
            log.println("Could not get service 'com.sun.star.sdb.OfficeDatabaseDocument'" );
            e.printStackTrace ();
            throw new StatusException("Service not available", e) ;
        }

        oObj = (XInterface) xDBDoc.getDataSource();
        log.println("ImplementationName: " + utils.getImplName(oObj));

        // Creating new DBase data source in the TEMP directory

        XPropertySet xSrcProp = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oObj);

        try{
            xSrcProp.setPropertyValue("URL", tmpDatabaseUrl) ;
        } catch ( UnknownPropertyException e){
            throw new StatusException("Could not set property 'URL' ", e) ;
        } catch ( PropertyVetoException e){
            throw new StatusException("Could not set property 'URL' ", e) ;
        } catch (  IllegalArgumentException e){
            throw new StatusException("Could not set property 'URL' ", e) ;
        } catch ( WrappedTargetException e){
            throw new StatusException("Could not set property 'URL' ", e) ;
        }

        databaseName = "NewDatabaseSource" + uniqueSuffix ;

        // make sure that the DatabaseContext isn't already registered
        try {
            xDBContextNameServ.revokeObject(databaseName) ;
        } catch (Exception e) {
            log.println("Nothing to be removed - OK");
        }

        // registering source in DatabaseContext
        XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class, xDBDoc);
        String aFile = utils.getOfficeTemp ((XMultiServiceFactory) Param.getMSF ())+"DataSource.odb";
        try{
            store.storeAsURL(aFile,new PropertyValue[]{});
        } catch (IOException e){
            log.println("Could not store datasource 'aFile'" );
            e.printStackTrace ();
            throw new StatusException("Could not save ", e) ;
        } catch (Exception e){
            log.println("Could not store datasource 'aFile'" );
            e.printStackTrace ();
            throw new StatusException("Could not save ", e) ;
        }


        try{
            xDBContextNameServ.registerObject(databaseName, oObj) ;
        } catch (Exception e){
            log.println("Could not register data source" );
            e.printStackTrace ();
            throw new StatusException("Could not register ", e) ;
        }

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding obj relation for interface XCompletedConnection
        Object handler = null ;
        try {
            handler = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sdb.InteractionHandler") ;
        } catch (Exception e) {
            log.println("Relation for XCompletedConnection wasn't created") ;
            e.printStackTrace(log) ;
        }

        // dbase does not need user and password
        tEnv.addObjRelation("UserAndPassword", new String[]{"",""}) ;

        tEnv.addObjRelation("XCompletedConnection.Handler",
            UnoRuntime.queryInterface(XInteractionHandler.class, handler)) ;

        return tEnv;
    } // finish method getTestEnvironment

}
