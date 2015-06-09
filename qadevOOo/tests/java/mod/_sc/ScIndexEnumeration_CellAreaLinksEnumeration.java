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
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_CellAreaLinksEnumeration extends TestCase {
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


    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        TestEnvironment tEnv = null;

        // creation of testobject here
        XPropertySet props = UnoRuntime.queryInterface(XPropertySet.class,
                xSheetDoc);
        oObj = (XInterface) AnyConverter.toObject(new Type(XInterface.class),
                props.getPropertyValue("AreaLinks"));
        XAreaLinks links = null;

        // adding one link into collection (for best testing)
        links = UnoRuntime.queryInterface(XAreaLinks.class, oObj);
        CellAddress addr = new CellAddress((short) 1, 2, 3);
        String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
        links.insertAtPosition(addr, aSourceArea, "a2:b5", "", "");

        XEnumerationAccess ea = UnoRuntime.queryInterface(
                XEnumerationAccess.class, oObj);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: " + util.utils.getImplName(oObj));
        // creating test environment
        tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("ENUM", ea);

        return tEnv;
    }

}

