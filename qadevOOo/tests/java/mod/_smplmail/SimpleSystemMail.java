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

package mod._smplmail;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.system.SimpleSystemMail</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::system::XSimpleMailClientSupplier</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.system.XSimpleMailClientSupplier
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.lang.XTypeProvider
 * @see ifc.system._XSimpleMailClientSupplier
 * @see ifc.lang._XServiceInfo
 * @see ifc.lang._XTypeProvider
 */
public class SimpleSystemMail extends TestCase {

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * In case if OS is Win or not specified tries to create an instance
     * of the service <code>com.sun.star.system.SimpleSystemMail</code>. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'SystemMailExist'</code> for
     *      {@link ifc.system._XSimpleMailClientSupplier} :
     *      the value depends on tested platform and
     *      <code>test.system.mail.isExist</code> INI
     *      parameter.</li>
     * </ul>
     */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws Exception {
        XInterface oObj = null;
        Object oInterface = null;
        boolean isWinOS = false ;
        boolean isSystemMailExist = false ;

        // if 'OS' parameter is not defined then by default
        // Win platform is used. If really not the error will occurs.
        isWinOS = Param.get("OS") == null ||
            "wntmsci".equals(Param.get("OS")) ;
        isSystemMailExist = "true".equalsIgnoreCase
            ((String) Param.get("test.system.mail.isExist")) ;

        XMultiServiceFactory xMSF = Param.getMSF();
        if (isWinOS) {
            log.println("The OS is Win : trying to create service");
            oInterface = xMSF.createInstance
                ( "com.sun.star.system.SimpleSystemMail" );
        } else {
            log.println("The OS is NOT Win, SKIPPING this component");
            log.println("Creating Dummy object ...");
            oInterface = xMSF.createInstance( "com.sun.star.io.Pipe" );
        }

        oObj = (XInterface) oInterface;

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("SystemMailExist", Boolean.valueOf(isSystemMailExist));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SimpleSystemMail

