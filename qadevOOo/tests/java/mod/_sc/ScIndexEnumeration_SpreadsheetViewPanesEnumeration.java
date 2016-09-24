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

import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_SpreadsheetViewPanesEnumeration extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc;
    private static SOfficeFactory SOF;
    private static XInterface oObj;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    public void initialize( TestParameters Param, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOF = SOfficeFactory.getFactory( Param.getMSF());

        log.println("creating a spreadsheetdocument");
        xSpreadsheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println("disposing xSpreadsheetDocument");
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {

        XModel xm = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);
        XController xc = xm.getCurrentController();
        XIndexAccess xIA = UnoRuntime.queryInterface(XIndexAccess.class, xc);
        oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),xIA.getByIndex(0));

        XEnumerationAccess ea = UnoRuntime.queryInterface(XEnumerationAccess.class,xIA);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: "+util.utils.getImplName(oObj));
        // creating test environment
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM",ea);

        return tEnv;
    }
}

