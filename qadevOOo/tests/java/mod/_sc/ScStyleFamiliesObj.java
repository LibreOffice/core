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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.style.StyleFamilies</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
* </ul>
* @see com.sun.star.style.StyleFamilies
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexAccess
*/
public class ScStyleFamiliesObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a Spreadsheet document" );
        xSpreadsheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the collection of style families from the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * This collection is the instance of the service
    * <code>com.sun.star.style.StyleFamilies</code>.
    * @see com.sun.star.style.StyleFamilies
    * @see com.sun.star.style.XStyleFamiliesSupplier
    */
    @Override
    public TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) throws StatusException {

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XNameAccess oStyleFamilies=null;
        // create testobject here
        log.println("getting style families");
        XStyleFamiliesSupplier oStyleFamiliesSupplier =UnoRuntime.queryInterface(
            XStyleFamiliesSupplier.class, xSpreadsheetDoc);

        oStyleFamilies = oStyleFamiliesSupplier.getStyleFamilies();

        TestEnvironment tEnv = new TestEnvironment(oStyleFamilies);
        return tEnv;

    } // finish method getTestEnvironment
}    // finish class ScStyleFamiliesObj

