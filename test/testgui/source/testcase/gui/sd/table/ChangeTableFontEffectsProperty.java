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

public class ChangeTableFontEffectsProperty {
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
     * Open the Font Effect property dialog in Presentation
     *
     * @throws Exception
     */

    @Test
    public void OpenTableFontEffectPropertyDialog() throws Exception {

        // Create a new presentation document
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();

        // Insert a table
        app.dispatch(".uno:InsertTable", 3);
        InsertTable.ok();

        // Verify if the table toolbar is active
        assertTrue("Table Toolbar exist", Table_Toolbar.exists(3));

        // open Table Properties Dialog
        app.dispatch(".uno:TableDialog", 3);
        EffectsPage.select();
        // select Font color
        EffectsPage_Color.select(1);
        assertEquals("EffectsPage_Color is black", EffectsPage_Color.getSelText(), "Black");

        // select Relief
        EffectsPage_Relief.select(0);
        assertEquals("EffectsPage_Relief is no", EffectsPage_Relief.getSelText(), "(Without)");
        EffectsPage_Outline.check();
        assertTrue("EffectsPage_Outline is checked", EffectsPage_Outline.isChecked());
        EffectsPage_Shadow.check();
        assertTrue("EffectsPage_Shadow is checked", EffectsPage_Shadow.isChecked());

        // select Overlining
        EffectsPage_Overline.select(1);
        assertEquals("EffectsPage_Overline is first item", EffectsPage_Overline.getSelText(), EffectsPage_Overline.getItemText(1));
        EffectsPage_Overline_Color.select(1);
        assertEquals("EffectsPage_Overline_Color is first item", EffectsPage_Overline_Color.getSelText(), EffectsPage_Overline_Color.getItemText(1));

        // select Strikethrough
        EffectsPage_Strikeout.select(1);
        assertEquals("EffectsPage_Strikeout is first item", EffectsPage_Strikeout.getSelText(), EffectsPage_Strikeout.getItemText(1));

        // select Underlining
        EffectsPage_Underline.select(1);
        assertEquals("EffectsPage_Underline is first item", EffectsPage_Underline.getSelText(), EffectsPage_Underline.getItemText(1));
        EffectsPage_Underline_Color.select(1);
        assertEquals("EffectsPage_Underline_Color is first item", EffectsPage_Underline_Color.getSelText(), EffectsPage_Underline_Color.getItemText(1));

        // select individual workds
        EffectsPage_InvidiualWords.check();
        assertTrue("EffectsPage_InvidiualWords is checked", EffectsPage_InvidiualWords.isChecked());

        // select Emphasis Mark
        // EffectsPage_Emphasis.select(1);
        // assertEquals(EffectsPage_Emphasis.getSelText(), "Dot");
        // EffectsPage_Position.select(1);
        // assertEquals(EffectsPage_Position.getSelText(), "Below text");
        EffectsPage.ok();

    }
}
