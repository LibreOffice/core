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

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdb.DatasourceAdministrationDialog</code>. <p>
*
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XInitialization</code></li>
*  <li> <code>com::sun::star::ui::dialogs::XExecutableDialog</code></li>
*  <li> <code>com::sun::star::sdb::DatasourceAdministrationDialog</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul> <p>
*
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
*
* @see com.sun.star.lang.XInitialization
* @see com.sun.star.ui.dialogs.XExecutableDialog
* @see com.sun.star.sdb.DatasourceAdministrationDialog
* @see com.sun.star.beans.XPropertySet
* @see ifc.lang._XInitialization
* @see ifc.ui.dialogs._XExecutableDialog
* @see ifc.sdb._DatasourceAdministrationDialog
* @see ifc.beans._XPropertySet
*/
public class ODatasourceAdministrationDialog extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sdb.DatasourceAdministrationDialog</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sdb.DatasourceAdministrationDialog" );
        } catch( com.sun.star.uno.Exception e ) {
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

        return tEnv;
    } // finish method getTestEnvironment

} // finish class ODataSourceAdministrationDialog

