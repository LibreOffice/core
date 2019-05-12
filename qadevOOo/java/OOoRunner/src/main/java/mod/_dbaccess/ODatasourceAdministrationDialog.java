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

