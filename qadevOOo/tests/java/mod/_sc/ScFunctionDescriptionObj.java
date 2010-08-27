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

package mod._sc;

import java.io.PrintWriter;
import java.util.Random;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScFunctionDescriptionObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.
                        queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment(
                                    TestParameters Param, PrintWriter log )
                                                       throws StatusException {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        try {
            log.println("Getting test object ") ;

            XMultiServiceFactory oDocMSF =  (XMultiServiceFactory) Param.getMSF();

            XInterface FDs = (XInterface)oDocMSF.
                    createInstance("com.sun.star.sheet.FunctionDescriptions");
            XNameAccess NA = (XNameAccess)UnoRuntime.queryInterface
                (XNameAccess.class, FDs);

            String names[] = NA.getElementNames();
            Random rnd = new Random();
            int idx = rnd.nextInt(names.length);

            oObj = (XInterface)NA.getByName(names[idx]);

            log.println("Creating object - " +
                                        ((oObj == null) ? "FAILED" : "OK"));

        } catch (Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException
                ("Error getting test object from spreadsheet document",e) ;
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests

        return tEnv;
    }

}


