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

package mod._defreg;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.RegistryTools;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.registry.NestedRegistry</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::registry::XSimpleRegistry</code></li>
*  <li> <code>com::sun::star::lang::XInitialization</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> XSimpleRegistry.rdb </b> : Registry file created before. </li>
*  <li><b> XSimpleRegistry_open#.rdb </b> : Temporary registry file as copy of
*     <b> XSimpleRegistry.rdb </b> in the SOffice temp directory.
*     ('#' - is an ordinary number) </li>
*  <li><b> XSimpleRegistry_merge#.rdb </b> : Temporary registry file as copy of
*     <b> XSimpleRegistry.rdb </b> in the SOffice temp directory.
*     ('#' - is an ordinary number) </li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.registry.XSimpleRegistry
* @see com.sun.star.lang.XInitialization
* @see ifc.registry._XSimpleRegistry
* @see ifc.lang._XInitialization
*/
public class NestedRegistry extends TestCase {

    protected static int uniq = 0 ;
    XSimpleRegistry reg1;
    XSimpleRegistry reg2;

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
        TestParameters tParam ) {
        try {
            reg1.destroy();
            reg2.destroy();
        }
        catch (com.sun.star.registry.InvalidRegistryException e) {}
    }

    /**
    * Creates a temporary copy of file, which is deleted when VM exits.
    * @param src Source file path.
    * @param dst Destination file path.
    * @throws java.io.IOException If any problems occur during copiing.
    */
    protected void copyFile(String src, String dst, PrintWriter log)
            throws java.io.IOException {
        log.println("Copy File "+src+" to "+dst);
        File srcF = new File(src) ;
        File dstF = new File(dst) ;

        if (dstF.exists()) dstF.delete() ;
        dstF.createNewFile() ;

        dstF.deleteOnExit() ;

        FileInputStream fIn = new FileInputStream(srcF) ;
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
    * Creates two temporary copies of registry file created before, opens
    * them, and creates service <code>com.sun.star.comp.stoc.NestedRegistry</code>
    * with these two registries. <p>
    * Object relations created :
    * <ul>
    *  <li> <code>'NR'</code> for {@link ifc.registry._XSimpleRegistry} :
    *     Just informs interface test that <code>NestedRegistry</code>
    *     service is tested. If this relation exists, than some methods
    *     are not supported. The relation is a <code>String</code> with
    *     object name.</li>
    *  <li> <code>'XSimpleRegistry.open'</code> for
    *    {@link ifc.registry._XSimpleRegistry}
    *  </li>
    *  <li> <code>'XSimpleRegistry.destroy'</code> for
    *    {@link ifc.registry._XSimpleRegistry}
    *  </li>
    *  <li> <code>'XSimpleRegistry.merge'</code> for
    *    {@link ifc.registry._XSimpleRegistry}
    *  </li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        final String tmpDir = utils.getOfficeTempDirSys((XMultiServiceFactory)Param.getMSF()) ;
        final String openF = tmpDir + "XSimpleRegistry_open" + uniq + ".rdb" ;
        final String destroyF = tmpDir
            + "XSimpleRegistry_destroy" + uniq + ".rdb" ;
        final String mergeF = tmpDir + "XSimpleRegistry_merge" + uniq + ".rdb" ;
        uniq++ ;

        log.println("creating copies of the registry for XSimpleRegistry");
        try {
            String source = utils.getFullTestDocName("XSimpleRegistry.rdb");
            copyFile(source, openF, log) ;
            copyFile(source, mergeF, log) ;
        } catch (java.io.IOException e) {
            log.println("Exception occurred while copying files");
            e.printStackTrace(log);
            throw new StatusException("Exception occurred while copying files", e);
        }

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            reg1 = RegistryTools.
                    createRegistryService(xMSF) ;
            reg1.open(mergeF, false, true) ;
            reg2 = RegistryTools.
                    createRegistryService(xMSF) ;
            reg2.open(openF, false, true) ;
            XSimpleRegistry[] arg = new XSimpleRegistry[2];
            arg[0]=reg1;
            arg[1]=reg2;
            oInterface = xMSF.createInstanceWithArguments
                ( "com.sun.star.comp.stoc.NestedRegistry", arg );
        }
        catch( Exception e ) {
            log.println("Introspection Service not available" );
        }
        oObj = (XInterface) oInterface;



        log.println( "    creating a new environment for Introspection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("NR","NestedRegistry");

        tEnv.addObjRelation("XSimpleRegistry.open", openF) ;
        tEnv.addObjRelation("XSimpleRegistry.merge", mergeF) ;
        tEnv.addObjRelation("XSimpleRegistry.destroy", destroyF) ;

        return tEnv;
    } // finish method getTestEnvironment

    protected void cleanup( TestParameters Param, PrintWriter log) {


    }

}    // finish class NestedRegistry

