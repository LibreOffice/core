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
package testcase.sc.sheet;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.sleep;
import static testlib.AppUtil.initApp;
import static testlib.UIMap.*;
import static testlib.UIMap.startcenter;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.Log;

/**
 * Before running the testing class, you need specify the AOO location firstly with system property openoffice.home.
 *
 *
 */

public class InsertOneSheet {

    /**
     * TestCapture helps us to do
     * 1. Take a screenshot when failure occurs.
     * 2. Collect extra data when OpenOffice crashes.
     */
    @Rule
    public Log LOG = new Log();

    /**
     * initApp helps us to do
     * 1. Patch the OpenOffice to enable automation if necessary.
     * 2. Start OpenOffice with automation enabled if necessary.
     * 3. Reset OpenOffice to startcenter.
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        initApp();
    }

    /**
     * Insert one sheet in different place
     * @throws Exception
     */
    @Test
    public void testInsertOneSheet(){
        //Create a new Spreadsheet
        startcenter.menuItem("File->New->Spreadsheet").select();
        calc.waitForExistence(10, 3);
        sleep(3);

        //Open Insert Sheet dialog via main menu Insert-> Sheet
        calc.menuItem("Insert->Sheet...").select();
        sleep(2);

        //Click OK button to create sheet with default setting
        SCInsertSheetDlg.ok();
        sleep(5);

        //Verify new sheet has been inserted before Sheet1
        calc.menuItem("Edit->Sheet->Select...").select();
        sleep(2);
        assertEquals("Sheet4",SCSheetsList.getItemText(0,0));

    }
}
