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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.GlobalSheetSettings</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::GlobalSheetSettings</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.sheet.GlobalSheetSettings
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._GlobalSheetSettings
* @see ifc.beans._XPropertySet
*/
public class ScSpreadsheetSettings extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sheet.GlobalSheetSettings</code>.
    */
    public TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) throws StatusException {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here
        try {
            oObj = (XInterface)((XMultiServiceFactory)tParam.getMSF()).
                createInstance("com.sun.star.sheet.GlobalSheetSettings");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScSpreadsheetSettings

