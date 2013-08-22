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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DesktopTools;
import util.SOfficeFactory;


public class ScDocumentConfiguration extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    protected void initialize(TestParameters tParam, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a sheetdocument");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");
        DesktopTools.closeDoc(xSheetDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XMultiServiceFactory docMSF = UnoRuntime.queryInterface(
                                              XMultiServiceFactory.class,
                                              xSheetDoc);
        Object oObj = null;

        try {
            oObj = docMSF.createInstance("com.sun.star.sheet.DocumentSettings");
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Couldn't create document settings", e);
        }

        log.println("Implementationname: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment((XInterface) oObj);

        return tEnv;
    }
}