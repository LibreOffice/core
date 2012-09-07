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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XFrame;
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

            XMultiServiceFactory xMSF = (XMultiServiceFactory) tParam.getMSF();
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
                    ((XMultiServiceFactory)tParam.getMSF()).createInstance
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


