/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptInfo.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:47:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

public class ScriptInfo extends TestCase {

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
            Object storageObj = ( XInterface )xMSF.createInstanceWithArguments(
                "drafts.com.sun.star.script.framework.storage.ScriptStorage",
                new Object[]{ access, new Integer(99), docPath } );
            XScriptInfoAccess infoAccess = ( XScriptInfoAccess )UnoRuntime.queryInterface(XScriptInfoAccess.class, storageObj);
            XScriptInfo[] infos = infoAccess.getImplementations("script://MemoryUtils.MemUsage?location=document");
           oObj = infos[0];
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        TestDataLoader.setupData(tEnv, "ScriptInfo");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


