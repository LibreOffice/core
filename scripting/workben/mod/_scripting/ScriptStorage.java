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

package mod._scripting;

import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;
import java.net.URLEncoder;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;

public class ScriptStorage extends TestCase {

    String docPath = null;
    public void initialize( TestParameters tParam, PrintWriter log ) {
        // Get path to test documents
        String rootDocPath = ( String )tParam.get( "DOCPTH" );
        System.out.println( "DOCPTH is " + rootDocPath );
        rootDocPath = util.utils.getFullTestURL( "ExampleSpreadSheetLatest.sxc" );
        if ( rootDocPath != null && rootDocPath.length() > 1 ){
            // convert all "\\" to "/", necessary for UCB
            if ( rootDocPath.indexOf( "\\" ) > 0 ){
                rootDocPath = rootDocPath.replace( '\\','/' );
        }
            System.out.println("After processing the path is " + rootDocPath);
        // encode the ulr (for UCB)
            String encodedPath = URLEncoder.encode( rootDocPath );
            System.out.println("The encoded path is " + encodedPath );
            docPath = "vnd.sun.star.pkg://" + encodedPath;
            System.out.println( "docPath path is " + docPath );
        }

    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log ) throws StatusException {

    log.println("creating test environment");
        if ( docPath == null ){
            log.println("Testdata not set up, docPath is null");
            throw new StatusException(
                "Can't create object environment, no test document available",
                new Exception() ) ;
        }

        XInterface oObj = null;
        XSimpleFileAccess access = null;
        try {
            XMultiServiceFactory xMSF = tParam.getMSF();
            Object xInterface =
                xMSF.createInstance( "com.sun.star.ucb.SimpleFileAccess" );
            access = ( XSimpleFileAccess )
                UnoRuntime.queryInterface( XSimpleFileAccess.class, xInterface );
            oObj = ( XInterface )xMSF.createInstanceWithArguments(
                "drafts.com.sun.star.script.framework.storage.ScriptStorage",
                new Object[]{ access, new Integer(99), docPath } );

        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        TestDataLoader.setupData(tEnv, "ScriptStorage");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


