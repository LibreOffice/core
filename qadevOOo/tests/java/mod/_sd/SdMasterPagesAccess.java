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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XMasterPagesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.MasterPages</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::drawing::XDrawPages</code></li>
* </ul>
* @see com.sun.star.drawing.MasterPages
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.drawing.XDrawPages
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.drawing._XDrawPages
*/
public class SdMasterPagesAccess extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xDrawDoc = SOF.createDrawDoc(null);
         } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
         }
    }

    /**
    * Disposes Drawing document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        util.DesktopTools.closeDoc(xDrawDoc);;
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the master pages from the document using the
    * interface <code>XMasterPagesSupplier</code>. Inserts two new draw pages.
    * The retrieved collection is the instance of the service
    * <code>com.sun.star.drawing.MasterPages</code>.
    * @see com.sun.star.drawing.XMasterPagesSupplier
    * @see com.sun.star.drawing.MasterPages
    */
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        log.println( "creating a test environment" );

        // get the MasterPages here
        log.println( "getting MasterPages" );
        XMasterPagesSupplier oMPS = (XMasterPagesSupplier)
            UnoRuntime.queryInterface( XMasterPagesSupplier.class, xDrawDoc);
        XDrawPages oMP = oMPS.getMasterPages();
        log.println( "insert MasterPages" );
        oMP.insertNewByIndex(1);
        oMP.insertNewByIndex(2);
        XInterface oObj = oMP;

        log.println( "creating a new environment for MasterPagesAccess object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SdMasterPagesAccess

