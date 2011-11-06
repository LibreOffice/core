/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package mod._scripting;

import java.net.URLEncoder;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.utils;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XDesktop;


import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;



public class Dispatch extends TestCase {
    //private String scriptURL = "script://MemoryUtils.MemUsage";
    private String scriptURL = "script://HighlightText.showForm";
    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log ) throws StatusException {
    log.println("creating test environment");
        XInterface oObj = null;
        URL dispatchURL = null;
        try {

            XMultiServiceFactory xMSF = tParam.getMSF();
            SOfficeFactory SOF = null;
            SOF = SOfficeFactory.getFactory( xMSF );
            String docPath = util.utils.getFullTestURL( "ExampleSpreadSheetLatest.sxc" );
            XComponent doc = SOF.loadDocument( docPath );
            XModel model = ( XModel ) UnoRuntime.queryInterface( XModel.class,
                doc );
            XFrame frame = model.getCurrentController().getFrame();
            oObj = ( XInterface )xMSF.createInstanceWithArguments( "com.sun.star.comp.ScriptProtocolHandler", new Object[] { frame } );

            XURLTransformer xParser=(XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
                    tParam.getMSF().createInstance
                        ("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of
            // URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = scriptURL;
            xParser.parseStrict(aParseURL);
            dispatchURL = aParseURL[0];

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
            throw new StatusException("Can't create object environment", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        tEnv.addObjRelation( "XDispatchProvider.URL", scriptURL );
        tEnv.addObjRelation( "XDispatch.URL", dispatchURL );
        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


