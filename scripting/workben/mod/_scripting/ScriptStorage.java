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
            XMultiServiceFactory xMSF = (XMultiServiceFactory) tParam.getMSF();
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


