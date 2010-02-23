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

package mod._ucprmt;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ProxyProvider extends TestCase {

    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) {

        XInterface oObj = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XContentIdentifierFactory cntIDFac = null ;

        try {
            oObj = (XInterface) xMSF.createInstance
                ("com.sun.star.ucb.RemoteProxyContentProvider");
            cntIDFac = (XContentIdentifierFactory) UnoRuntime.queryInterface
                (XContentIdentifierFactory.class, oObj) ;
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        if (oObj == null) {
            log.println("!!! Object wasn't created !!!") ;
            throw new StatusException(Status.failed
                ("!!! Object wasn't created !!!")) ;
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XContentProvider
        tEnv.addObjRelation("FACTORY", cntIDFac) ;
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.pkg:///user/work");
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.wfs://");

        tEnv.addObjRelation("NoCONTENT",new Boolean(true));

        return tEnv;
    } // finish method getTestEnvironment

}

