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



package testcase.gui.sc.validity;

import static testlib.gui.UIMap.*;

import org.junit.After;
import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.gui.Log;


public class ValidityDialogDefaultValue {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);

        // New a spreadsheet, open Validity dialog
        app.dispatch("private:factory/scalc");
        app.dispatch(".uno:Validation");
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * test Data -> Validity - UI(Criteria: Cell range).
     */
    @Test
    public void testValidityUICellRange() {

        SC_ValidityCriteriaTabpage.select();
        SC_ValidityCriteriaAllowList.select(5); // "Cell range"

        assertEquals(true,SC_ValidityAllowBlankCells.isChecked());
        assertEquals(true,SC_ValidityShowSelectionList.isChecked());
        assertEquals(false,SC_ValiditySortEntriesAscending.isChecked());
        assertEquals(true,SC_ValidityAllowBlankCells.isEnabled());
        assertEquals(true,SC_ValiditySortEntriesAscending.isEnabled());
        assertEquals(true,SC_ValiditySortEntriesAscending.isEnabled());
        assertEquals(true,SC_ValiditySourcePicker.isEnabled());
    }

    /**
     * test Data -> Validity - UI(Error Alert: Default status)
     */
    @Test
    public void testValidityUIErrorAlert() {

        SC_ValidityErrorAlertTabPage.select();
        assertEquals(true,SC_ValidityErrorAlertActionList.isEnabled());
//      assertEquals("Stop",SC_ValidityErrorAlertActionList.getSelText());  // Do not test this. Do it in GVT.
        assertEquals(true,SC_ValidityErrorMessageTitle.isEnabled());
        assertEquals("",SC_ValidityErrorMessageTitle.getText());
        assertEquals(true,SC_ValidityErrorMessage.isEnabled());
        assertEquals("",SC_ValidityErrorMessage.getText());
        SC_ValidityErrorAlertActionList.select(3);  // "Macro"
        assertEquals(true,SC_ValidityErrorBrowseButton.isEnabled());
    }

}

