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

package mod._simreg;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

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
        log.println("H1");

        if (dstF.exists()) dstF.delete() ;
        log.println("H2");
        dstF.createNewFile() ;

        dstF.deleteOnExit() ;
        log.println("H3");

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
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        final String tmpDir = utils.getOfficeTempDirSys((XMultiServiceFactory)Param.getMSF()) ;
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

