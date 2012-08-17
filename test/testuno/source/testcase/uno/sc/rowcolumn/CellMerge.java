/* Licensed to the Apache Software Foundation (ASF) under one
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


package testcase.uno.sc.rowcolumn;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XCell;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XMergeable;

/**
 * Check the content input in cell
 * @author test
 *
 */

public class CellMerge {

    UnoApp unoApp = new UnoApp();
    XSpreadsheetDocument scDocument = null;
    XComponent scComponent = null;

    @Before
    public void setUp() throws Exception {
        unoApp.start();
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(scComponent);
        unoApp.close();
        }

    @Test
    public void testCellMerge() throws Exception {

        String sheetname = "sheet1";
        scComponent = unoApp.newDocument("scalc");
        scDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, scComponent);
        XSpreadsheets spreadsheets = scDocument.getSheets();
        Object sheetObj = spreadsheets.getByName(sheetname);
        XSpreadsheet sheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, sheetObj);

        //Select A1 and input "12"
        XCell cell = sheet.getCellByPosition(0, 0);
        cell.setValue(12);

        // Get cell range A1:B1 by position - (column, row, column, row)
        XCellRange CellRange = sheet.getCellRangeByPosition( 0, 0, 1, 0 );
        //XCellRange CellRange = sheet.getCellRangeByName("A1:B1");

        //Merge cell range A1:B1 into one cell
       XMergeable xMerge = (XMergeable) UnoRuntime.queryInterface(XMergeable.class, CellRange);
        xMerge.merge(true);

        //Verify if the cell range A1:B1 is completely merged
        assertEquals("Verify if the cell range A1:B1 is completely merged",true, xMerge.getIsMerged());

       //Undo Merge cell range A1:B1 into one cell
        xMerge.merge(false);

        //Verify if the cell range A1:B1 is no longer merged
        assertEquals("Verify if the cell range A1:B1 is no longer merged",false, xMerge.getIsMerged());


    }

}
