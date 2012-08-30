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
import static testlib.gui.UIMap.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.Logger;

public class ChangeTableBackgroundProperty {
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
     * Open the Table Background property dialog in Presentation
     *
     * @throws Exception
     */

    @Test
    public void OpenTableBackgroundPropertyDialog() throws Exception {

        // Create a new presentation document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();

        // Insert a table
        app.dispatch(".uno:InsertTable", 3);
        InsertTable.ok();

        // Verify if the table toolbar is active
        assertTrue(Table_Toolbar.exists(3));

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        Area_Area_page.select();
        // select Color
        Area_Fill_Type.select(1);
        assertEquals("Area_Fill_Type is Color", Area_Fill_Type.getSelText(), Area_Fill_Type.getItemText(1));
        Area_Fill_Color.select(2);
        // Area_Fill_Color.getItemCount();
        assertEquals("Area_Fill_Color is second item", Area_Fill_Color.getSelText(), Area_Fill_Color.getItemText(2));
        Area_Area_page.ok();

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        Area_Area_page.select();
        // select Gradient
        Area_Fill_Type.select(2);
        assertEquals("Area_Fill_Type is Gradient", Area_Fill_Type.getSelText(), Area_Fill_Type.getItemText(2));
        Area_Fill_Gradient.select(2);
        assertEquals("Area_Fill_Gradient is second item", Area_Fill_Gradient.getSelText(), Area_Fill_Gradient.getItemText(2));
        Area_Area_page.ok();

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        Area_Area_page.select();
        // select Hatching
        Area_Fill_Type.select(3);
        assertEquals("Area_Fill_Type is Hatching", Area_Fill_Type.getSelText(), Area_Fill_Type.getItemText(3));
        Area_Fill_Hatching.select(2);
        assertEquals("Area_Fill_Hatching is second item", Area_Fill_Hatching.getSelText(), Area_Fill_Hatching.getItemText(2));
        Area_Area_page.ok();

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        Area_Area_page.select();
        // select Bitmap
        Area_Fill_Type.select(4);
        assertEquals("Area_Fill_Type is Bitmap", Area_Fill_Type.getSelText(), Area_Fill_Type.getItemText(4));
        Area_Fill_Bitmap.select(2);
        assertEquals("Area_Fill_Bitmap is second item", Area_Fill_Bitmap.getSelText(), Area_Fill_Bitmap.getItemText(2));
        Area_Area_page.ok();

    }

}
