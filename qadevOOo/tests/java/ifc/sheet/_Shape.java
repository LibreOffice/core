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
package ifc.sheet;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

import lib.MultiPropertyTest;


public class _Shape extends MultiPropertyTest {
    public void _Anchor() {
        XSpreadsheetDocument xSheetDoc = UnoRuntime.queryInterface(
                                                 XSpreadsheetDocument.class,
                                                 tEnv.getObjRelation(
                                                         "DOCUMENT"));
        String[] sheetNames = xSheetDoc.getSheets().getElementNames();
        XSpreadsheet xSheet = null;
        XCell xCell = null;

        try {
            xSheet = UnoRuntime.queryInterface(
                             XSpreadsheet.class,
                             xSheetDoc.getSheets().getByName(sheetNames[0]));
            xCell = xSheet.getCellByPosition(0, 0);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
        }

        testProperty("Anchor", xSheet, xCell);
    }
}