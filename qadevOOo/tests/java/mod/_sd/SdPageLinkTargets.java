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

package mod._sd;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XLinkTargetSupplier;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.document.LinkTargets</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.document.LinkTargets
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
*/
public class SdPageLinkTargets extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
       // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    Param.getMSF());
        log.println( "creating a draw document" );
        xDrawDoc = SOF.createDrawDoc(null);
    }

    /**
    * Disposes Drawing document.
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the collection of draw pages and take one of them.
    * Obtains the collection of possible links using the interface
    * <code>XLinkTargetSupplier</code>. The obtained collection is
    * the instance of the service <code>com.sun.star.document.LinkTargets</code>.
    * Inserts some shapes into the document.
    * @see com.sun.star.document.XLinkTargetSupplier
    * @see com.sun.star.document.LinkTargets
    */
    @Override
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here
        XDrawPage the_page = DrawTools.getDrawPage(xDrawDoc, 0);
        XLinkTargetSupplier oLTS = UnoRuntime.queryInterface(XLinkTargetSupplier.class, the_page);
        XInterface oObj = oLTS.getLinks();

        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    Param.getMSF());
        log.println( "inserting some Shapes" );
        XShapes oShapes = UnoRuntime.queryInterface(XShapes.class,the_page);
        XShape oShape =
            SOF.createShape(xDrawDoc, 15000, 13500, 5000, 5000, "OLE2");
        oShapes.add(oShape);

        XPropertySet shape_props = UnoRuntime.queryInterface(XPropertySet.class,oShape);

        log.println("Inserting a Chart");

        shape_props.
            setPropertyValue("CLSID","12DCAE26-281F-416F-a234-c3086127382e");

        log.println( "creating a new environment for LinkTargets object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method createTestEnvironment

} // finish class SdPageLinkTargets

