/*************************************************************************
 *
 *  $RCSfile: ScriptStorage.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-02-25 16:22:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


