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
* @see ifc.container._XNameAccess
* @see ifc.container._XEnumerationAccess
* @see ifc.container._XElementAccess
* @see ifc.uno._XNamingService
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
    *      {@link ifc.uno._XNamingService} as an
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

            XPropertySet xDSProps = UnoRuntime.queryInterface(XPropertySet.class, oInterface) ;

            xDSProps.setPropertyValue("URL", "sdbc:dbase:file:///.") ;

            XDocumentDataSource xDDS = UnoRuntime.queryInterface(XDocumentDataSource.class, oInterface);
            XStorable store = UnoRuntime.queryInterface(XStorable.class,
            xDDS.getDatabaseDocument ());
            String aFile = utils.getOfficeTemp ((XMultiServiceFactory) Param.getMSF ())+"DatabaseContext.odb";
            log.println("store to '" + aFile + "'");
            store.storeAsURL(aFile,new PropertyValue[]{});

            tEnv.addObjRelation("XNamingService.RegisterObject", oInterface) ;

            tEnv.addObjRelation("INSTANCE", oInterface);

            tEnv.addObjRelation("XContainer.Container",
                UnoRuntime.queryInterface(
                                    XNamingService.class, oObj));

        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object relation", e) ;
        } catch (NullPointerException e) {
            throw new StatusException("Can't create object relation", e) ;
        }

        return tEnv;
    } // finish method getTestEnvironment

}
