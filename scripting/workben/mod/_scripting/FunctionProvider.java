/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FunctionProvider.java,v $
 * $Revision: 1.4 $
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
import com.sun.star.uno.UnoRuntime;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;

public class FunctionProvider extends TestCase {

    String docPath = null;
    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
    log.println("creating test environment");
        try {

            XMultiServiceFactory xMSF = tParam.getMSF();

            Object xInterface = xMSF.createInstance( "com.sun.star.frame.Desktop" );
            XDesktop dtop = ( XDesktop )UnoRuntime.queryInterface( XDesktop.class,
                xInterface );

            XModel model = dtop.getCurrentFrame().getController().getModel();
            oObj  =
                (XInterface)xMSF.createInstanceWithArguments( "drafts.com.sun.star.script.framework.provider.FunctionProvider", new Object[]{ model } );


        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create object environment", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        TestDataLoader.setupData(tEnv, "FunctionProvider");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


