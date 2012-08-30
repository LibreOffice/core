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
package testcase.gui.sc.rowcolumn;

import static org.junit.Assert.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

import testlib.gui.CalcUtil;

public class InsertRowAndColumn {

    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start();
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 3);
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Insert new entire row and column
     *
     * @throws Exception
     */

    @Test
    public void testInsertEntireRowColumn() {

        // insert data in cell A2 and B2
        CalcUtil.selectRange("Sheet1.A2");
        typeKeys("123");
        CalcUtil.selectRange("Sheet1.B2");
        typeKeys("456");

        // Set expected result after executing insert one row
        String[][] expectedInsertRowResult = new String[][] { { "", "" }, { "", "" }, { "123", "456" }, };

        // Select Cell A2
        CalcUtil.selectRange("Sheet1.A2");

        // Insert one entire Row via menu
        calc.menuItem("Insert->Rows").select();

        // Verify results after inserting one row
        assertArrayEquals("Verify results after inserting one row", expectedInsertRowResult, CalcUtil.getCellTexts("A1:B3"));

        // Set expected result after executing insert column
        String[][] expectedInsertColumnResult = new String[][] { { "", "", "" }, { "", "", "" }, { "", "123", "456" }, };
        // Select Cell A3
        CalcUtil.selectRange("Sheet1.A3");

        // Insert one entire Column via menu
        calc.menuItem("Insert->Columns").select();

        // Verify results after inserting one column
        assertArrayEquals("Verify results after inserting one column", expectedInsertColumnResult, CalcUtil.getCellTexts("A1:C3"));

    }

}
