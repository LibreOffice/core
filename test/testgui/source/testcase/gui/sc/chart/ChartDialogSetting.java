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

/**
 *
 */
package testcase.gui.sc.chart;

import static org.junit.Assert.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

/**
 * Test the setting about chart dialog in spreadsheet
 */
public class ChartDialogSetting {

    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start(true);

        // Create a new spreadsheet document
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);
        app.dispatch(".uno:InsertObjectChart");
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test cancel and back button in chart wizard dialog
     *
     * @throws java.lang.Exception
     */
    @Test
    public void testChartDialogCancelBack() {
        WizardNextButton.click();
        assertTrue(ChartRangeChooseTabPage.isEnabled());
        WizardBackButton.click();
        assertTrue(ChartTypeChooseTabPage.isEnabled());
        Chart_Wizard.cancel();
        assertFalse(Chart_Wizard.exists());
    }
}
