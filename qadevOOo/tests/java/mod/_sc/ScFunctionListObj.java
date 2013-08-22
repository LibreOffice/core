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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.FunctionDescriptions</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::sheet::XFunctionDescriptions</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.sheet.FunctionDescriptions
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.sheet.XFunctionDescriptions
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XIndexAccess
* @see ifc.sheet._XFunctionDescriptions
* @see ifc.container._XElementAccess
*/
public class ScFunctionListObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sheet.FunctionDescriptions</code>.
    * @see com.sun.star.sheet.FunctionDescriptions
    */
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)Param.getMSF();
        try {
            oObj =  (XInterface)oDocMSF.createInstance(
                "com.sun.star.sheet.FunctionDescriptions");
            log.println("Creating object - " +
                                        ((oObj == null) ? "FAILED" : "OK"));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e);
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    }

}


