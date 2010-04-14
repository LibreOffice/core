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
import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.frame.XStorable;
import com.sun.star.sdb.XDocumentDataSource;
import util.utils;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdb.DatabaseContext</code>. <p>
*
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::uno::XNamingService</code></li>
* </ul>
*
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.uno.XNamingService
* @see com.sun.star.sdb.DatabaseContext
* @see ifc.container.XNameAccess
* @see ifc.container.XEnumerationAccess
* @see ifc.container.XElementAccess
* @see ifc.uno.XNamingService
*/
public class ODatabaseContext extends TestCase {

    /**
    * Does nothing.
    */
    protected void initialize ( TestParameters Param, PrintWriter log) {

    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.sdb.DatabaseContext</code>.
    *     Needed object relations :
    * <ul>
    * <li> <code>'XNamingService.RegisterObject'</code> for
    *      {@link ifc.namingservice.XNamingService} as an
    *      instance of <code>com.sun.star.sdb.DataSource</code>
    *      service. </li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null ;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance( "com.sun.star.sdb.DatabaseContext" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding obj relation for XNamingService
        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance( "com.sun.star.sdb.DataSource" );

            XPropertySet xDSProps = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oInterface) ;

            xDSProps.setPropertyValue("URL", "sdbc:dbase:file:///.") ;

            XDocumentDataSource xDDS = (XDocumentDataSource)
            UnoRuntime.queryInterface(XDocumentDataSource.class, oInterface);
            XStorable store = (XStorable) UnoRuntime.queryInterface(XStorable.class,
            xDDS.getDatabaseDocument ());
            String aFile = utils.getOfficeTemp ((XMultiServiceFactory) Param.getMSF ())+"DatabaseContext.odb";
            log.println("store to '" + aFile + "'");
            store.storeAsURL(aFile,new PropertyValue[]{});

            tEnv.addObjRelation("XNamingService.RegisterObject", oInterface) ;

            tEnv.addObjRelation("INSTANCE", oInterface);

            tEnv.addObjRelation("XContainer.Container",
                (XNamingService) UnoRuntime.queryInterface(
                                    XNamingService.class, oObj));

        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object relation", e) ;
        } catch (NullPointerException e) {
            throw new StatusException("Can't create object relation", e) ;
        }

        return tEnv;
    } // finish method getTestEnvironment

}
