/*************************************************************************
 *
 *  $RCSfile: ODatabaseSource.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:42:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._dbaccess;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;

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

    /**
    * Assigns database's name unique suffix for this object depending
    * on static field. For names differs in different object threads.
    */
    protected void initialize ( TestParameters Param, PrintWriter log) {
        uniqueSuffix = uniqueSuffixStat++ ;
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

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );

            xDBContextNameServ = (XNamingService)
                UnoRuntime.queryInterface(XNamingService.class, oInterface) ;

            // retrieving temp directory for database
            String tmpDatabaseUrl = utils.getOfficeTempDir((XMultiServiceFactory)Param.getMSF());

            tmpDatabaseUrl = "sdbc:dbase:file:///" + tmpDatabaseUrl ;

            // Creating new DBase data source in the TEMP directory
            XInterface newSource = (XInterface) xMSF.createInstance
                ("com.sun.star.sdb.DataSource") ;

            XPropertySet xSrcProp = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, newSource);

            xSrcProp.setPropertyValue("URL", tmpDatabaseUrl) ;

            databaseName = "NewDatabaseSource" + uniqueSuffix ;

            // make sure that the DatabaseContext isn't already registered
            try {
                xDBContextNameServ.revokeObject(databaseName) ;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Nothing to be removed - OK");
            }

            // registering source in DatabaseContext
            xDBContextNameServ.registerObject(databaseName, newSource) ;

            oInterface = newSource ;
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException("Service wasn't created",
                new NullPointerException()) ;
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding obj relation for interface XCompletedConnection
        Object handler = null ;
        try {
            handler = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sdb.InteractionHandler") ;
        } catch (com.sun.star.uno.Exception e) {
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

