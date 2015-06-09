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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

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
* threads concurrently.
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
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sdb.DatasourceAdministrationDialog</code>.
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws Exception {
        Object oInterface = Param.getMSF().createInstance
            ("com.sun.star.sdb.DatasourceAdministrationDialog" );
        XInterface oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

} // finish class ODataSourceAdministrationDialog

