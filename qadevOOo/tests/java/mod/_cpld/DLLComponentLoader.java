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

package mod._cpld;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for <code>com.sun.star.comp.stoc.DLLComponentLoader</code> service <p>
* Files which are used :
* <ul>
*  <li> <code>XImpReg.dll</code> : DLL which will be loaded in
*    <code>XImplementationLoader</code> interface test. </li>
* <ul>
* Multithread testing compliant.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class DLLComponentLoader extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.comp.stoc.DLLComponentLoader</code>
    * service. <p>
    * The following object relations created :
    * <ul>
    *  <li> <code>'ImplementationLoader'</code> : service which is
    *    responsible for loading DLL implementations. </li>
    *  <li> <code>'ImplementationUrl'</code> : DLL file location. </li>
    *  <li> <code>'ImplementationName'</code> : Name of the implementation.</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        XMultiServiceFactory xMSF = Param.getMSF();
        Object oInterface = xMSF.createInstance
            ( "com.sun.star.comp.stoc.DLLComponentLoader" );
        XInterface oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for DLLComponentLoader object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding object relation for XImplementationLoader ifc test
        tEnv.addObjRelation("ImplementationLoader",
            "com.sun.star.loader.SharedLibrary") ;
        String os = (String) Param.get("OS");
        if (os == null || os.equals(""))
            throw new StatusException(
                "Couldn't determine operating system for naming convention.",
                new NullPointerException());
        String name = "javaloader";
        if (!os.equals("wntmsci"))
            name = "lib" + name + ".so";
        else
            name += ".dll";
        String implURL = utils.getFullTestURL(os + "/" + name);
        tEnv.addObjRelation("ImplementationUrl", implURL) ;
        log.println("looking for shared lib: " + implURL);
        tEnv.addObjRelation("ImplementationName",
            "com.sun.star.comp.stoc.JavaComponentLoader") ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class DLLComponentLoader

