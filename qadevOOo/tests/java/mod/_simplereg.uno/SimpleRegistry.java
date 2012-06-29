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

package mod._simplereg.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.registry.SimpleRegistry</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::registry::XSimpleRegistry</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> XSimpleRegistry.rdb </b> : a registry file with
 *   some key set. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.registry.XSimpleRegistry
 * @see ifc.registry._XSimpleRegistry
 */
public class SimpleRegistry extends TestCase {

    /**
    * Creates a temporary copy of file, which is deleted when VM exits.
    * @param src Source file path.
    * @param dst Destination file path.
    * @param log The log writer.
    * @throws java.io.IOException If any problems occur during copiing.
    */
    protected void copyFile(String src, String dst, PrintWriter log)
            throws java.io.IOException {
        File srcF = new File(src) ;
        File dstF = new File(dst) ;
        System.out.println("H1");

        if (dstF.exists()) dstF.delete() ;
        System.out.println("H2");
        dstF.createNewFile() ;

        dstF.deleteOnExit() ;
        System.out.println("H3");

        FileInputStream fIn = new FileInputStream(srcF) ;
        System.out.println("H4");

        FileOutputStream fOut = new FileOutputStream(dstF) ;

        byte[] buf = new byte[1024] ;
        int bytesRead = 0 ;
        while ((bytesRead = fIn.read(buf)) > 0)
            fOut.write(buf, 0, bytesRead) ;

        fIn.close() ;
        fOut.close() ;
    }
    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.registry.SimpleRegistry</code>. Then
     * makes three copies of a predefined registry file with different
     * names in a temporary SOffice directory and passes their URLs as
     * relations. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'XSimpleRegistry.open'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     *  <li> <code>'XSimpleRegistry.merge'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     *  <li> <code>'XSimpleRegistry.destroy'</code> for
     *      {@link ifc.registry._XSimpleRegistry} :
     *       URL of 'XSimpleRegistry.rbd' file copy in the
     *       temp directory. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        final String tmpDir = utils.getOfficeTempDirSys(
                                (XMultiServiceFactory)Param.getMSF()) ;
        final String openF = "XSimpleRegistry_open.rdb" ;
        final String destroyF = "XSimpleRegistry_destroy.rdb" ;
        final String mergeF = "XSimpleRegistry_merge.rdb" ;


        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.registry.SimpleRegistry" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        if (oInterface == null)
            log.println("Service wasn't created") ;

        oObj = (XInterface) oInterface;

        log.println("creating copies of the registry for XSimpleRegistry");
       try {
            String source = utils.getFullTestDocName("XSimpleRegistry.rdb");
            copyFile(source, tmpDir + openF, log);
            copyFile(source, tmpDir + destroyF, log);
            copyFile(source, tmpDir + mergeF, log);
        } catch (java.io.IOException e) {
            log.println("Exception occurred while copying files");
            e.printStackTrace(log);
        }

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XSimpleRegistry.open", tmpDir + openF);
        tEnv.addObjRelation("XSimpleRegistry.destroy", tmpDir + destroyF);
        tEnv.addObjRelation("XSimpleRegistry.merge", tmpDir + mergeF);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SimpleRegistry

