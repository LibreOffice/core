/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package ifc.sheet;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

import lib.MultiPropertyTest;


public class _Shape extends MultiPropertyTest {
    public void _Anchor() {
        XSpreadsheetDocument xSheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                                                 XSpreadsheetDocument.class,
                                                 tEnv.getObjRelation(
                                                         "DOCUMENT"));
        String[] sheetNames = xSheetDoc.getSheets().getElementNames();
        XSpreadsheet xSheet = null;
        XCell xCell = null;

        try {
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
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