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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.CellAreaLink</code>.
* This object reflects some cell range (this range
* can also be from another saved document) in
* any range (of the same size) of the current
* document.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XAreaLink</code></li>
*  <li> <code>com::sun::star::util::XRefreshable</code></li>
*  <li> <code>com::sun::star::sheet::CellAreaLink</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.sheet.XAreaLink
* @see com.sun.star.util.XRefreshable
* @see com.sun.star.sheet.CellAreaLink
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._XAreaLink
* @see ifc.util._XRefreshable
* @see ifc.sheet._CellAreaLink
* @see ifc.beans._XPropertySet
*/
public class ScAreaLinkObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a Spreadsheet document" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of Area Links using the 'AreaLinks'
    * property of the Spreadsheet document. Adds a new link to this
    * collection, which has a source in the same document. This
    * link is passed as a tested object.
    */
    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws Exception {

        XInterface oObj = null;

        // creation of testobject here
        XPropertySet props = UnoRuntime.queryInterface
            (XPropertySet.class, xSheetDoc);
        XAreaLinks links = (XAreaLinks) AnyConverter.toObject(
            new Type(XAreaLinks.class),props.getPropertyValue("AreaLinks")) ;
        CellAddress addr = new CellAddress ((short) 1,2,3) ;
        String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
        links.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

        oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class), links.getByIndex(0)) ;

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }
}
