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
    static XSpreadsheetDocument xSheetDoc = null;

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
        XMultiServiceFactory docMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
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