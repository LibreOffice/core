/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
* @see com.sun.star.sdb.XQueryDefinitionsSupplier
* @see com.sun.star.sdb.XFormDocumentsSupplier
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.sdb.DataSource
* @see com.sun.star.sdb.XCompletedConnection
* @see com.sun.star.util.XFlushable
* @see com.sun.star.sdb.XReportDocumentsSupplier
* @see ifc.sdb._XQueryDefinitionsSupplier
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
