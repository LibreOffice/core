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


