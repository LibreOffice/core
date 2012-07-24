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
package testcase.gui.sc.sheet;

import static org.junit.Assert.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.gui.Log;

/**
 * Before running the testing class, you need specify the AOO location firstly with system property openoffice.home.
 *
 *
 */

public class InsertSheet {

    @Rule
    public Log LOG = new Log();

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
     * Insert one sheet in different place
     * @throws Exception
     */
    @Test
    public void testInsertMultipleSheet(){
        //Open Insert Sheet dialog via main menu Insert-> Sheet
        calc.menuItem("Insert->Sheet...").select();
        //Change new sheet number into 3 to insert 3 new sheet one time
        SCSheetNumber.setText("3");
        //Click OK button to create sheet with default setting
        SCInsertSheetDlg.ok();
        //Verify new sheets have been inserted before Sheet1
        calc.menuItem("Edit->Sheet->Select...").select();
        assertArrayEquals(new String[] { "Sheet4", "Sheet5", "Sheet6", "Sheet1", "Sheet2", "Sheet3" }, SCSheetsList.getItemsText());
        SCSelectSheetsDlg.ok();
    }

    /**
     * Insert one sheet in different place
     * @throws Exception
     */
    @Test
    public void testInsertOneSheet(){
        //Open Insert Sheet dialog via main menu Insert-> Sheet
        calc.menuItem("Insert->Sheet...").select();
        //Click OK button to create sheet with default setting
        SCInsertSheetDlg.ok();
        //Verify new sheet has been inserted before Sheet1
        calc.menuItem("Edit->Sheet->Select...").select();
        assertArrayEquals(new String[]{"Sheet4", "Sheet1", "Sheet2", "Sheet3"},SCSheetsList.getItemsText());
        SCSelectSheetsDlg.ok();
    }
}
