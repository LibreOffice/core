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

package mod._shlibloader.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

/**
* Test for <code>com.sun.star.comp.stoc.DLLComponentLoader</code> service <p>
* Files which are used :
* <ul>
*  <li> <code>XImpReg.dll</code> : DLL which will be loaded in
*    <code>XImplementationLoader</code> interface test. </li>
* <ul>
* Multithread testing compilant.
* @see com.sun.star.loader.XImplementationLoader
* @see com.sun.star.lang.XServiceInfo
* @see ifc.loader._XImplementationLoader
* @see ifc.lang._XServiceInfo
*/
public class DLLComponentLoader extends TestCase {

    /**
    * Does nothing.
    */
    protected void initialize ( TestParameters Param, PrintWriter log) {

    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
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
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = null;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.comp.stoc.DLLComponentLoader" );
        }
        catch( Exception e ) {
            log.println("DLLComponentLoader Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for DLLComponentLoader object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding object relation for XImplementationLoader ifc test
        tEnv.addObjRelation("ImplementationLoader",
            "com.sun.star.loader.SharedLibrary") ;
        //String fs = System.getProperty("file.separator");
        String os = (String) Param.get("OS");
        if (os == null || os == "")
            throw new StatusException(
                "Couldn't determine operating system for naming convention.",
                new NullPointerException());
        String name = "javaloader.uno";
        if (!os.equals("wntmsci"))
            name = name + ".so";
        else
            name += ".dll";
        String implURL = utils.getOfficeURL(xMSF)+"/"+name;
        tEnv.addObjRelation("ImplementationUrl", implURL) ;
        log.println("looking for shared lib: " + implURL);
        tEnv.addObjRelation("ImplementationName",
            "com.sun.star.comp.stoc.JavaComponentLoader") ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class DLLComponentLoader

