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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNameAccess;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.style.StyleFamilies</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.style.StyleFamilies
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 * @see ifc.container._XIndexAccess
 */
public class SwXStyleFamilies extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested. Style families
    * are gotten from text document using <code>XStyleFamiliesSupplier</code>
    * interface and returned as a test component.
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {


        log.println( "Creating a test environment" );
        XTextDocument xArea = UnoRuntime.queryInterface(XTextDocument.class, xTextDoc);
        XStyleFamiliesSupplier oSFS = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, xArea);
        XNameAccess oSF = oSFS.getStyleFamilies();

        TestEnvironment tEnv = new TestEnvironment(oSF);
        return tEnv;
    }


}    // finish class SwXStyle

