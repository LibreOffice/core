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

package testcase.gui.sd.table;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

public class ChangeTableBorderProperty {
    @Rule
    public Logger log = Logger.getLogger(this);

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Open the table border property in Presentation
     *
     * @throws Exception
     */

    @Test
    public void OpenTableBorderPropertyDialog() throws Exception {

        // Create a new presentation document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        sleep(3);

        // Insert a table
        app.dispatch(".uno:InsertTable", 3);
        InsertTable.ok();
        sleep(3);

        // Verify if the table toolbar is active
        assertTrue(Table_Toolbar.exists(3));

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        Area_Border_page.select();

        Area_Border_Presets.click(0.01, 0.01);

        // select Line
        Area_Border_LineStyle.select(1);
        assertEquals("Area_Border_LineStyle is first item", Area_Border_LineStyle.getSelText(), Area_Border_LineStyle.getItemText(1));

        Area_Border_LineColor.select(1);
        assertEquals("Area_Border_LineColor is first item", Area_Border_LineColor.getSelText(), Area_Border_LineColor.getItemText(1));

        // select Spacing to Contents
        Area_Border_Sync.check();
        assertTrue("Area_Border_Sync is checked", Area_Border_Sync.isChecked());

        Area_Border_Mf_Left.setText("0.5");
        assertTrue("Area_Border_Mf_Left is 0.5", Area_Border_Mf_Left.getText().startsWith("0.5"));

        Area_Border_Mf_Right.setText("0.5");
        assertTrue("Area_Border_Mf_Right is 0.5", Area_Border_Mf_Right.getText().startsWith("0.5"));

        Area_Border_Mf_Top.setText("0.5");
        assertTrue("Area_Border_Mf_Top is 0.5", Area_Border_Mf_Top.getText().startsWith("0.5"));

        Area_Border_Mf_Bottom.setText("0.5");
        assertTrue("Area_Border_Mf_Bottom is 0.5", Area_Border_Mf_Bottom.getText().startsWith("0.5"));

        Area_Border_page.ok();

    }
}
