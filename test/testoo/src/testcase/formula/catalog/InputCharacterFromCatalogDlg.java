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
package testcase.formula.catalog;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import testlib.Log;

/**
 *
 */
public class InputCharacterFromCatalogDlg {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Click catalog button
        math_CatalogButton.click();
        sleep(1);

        // Verify if the "Symbols" dialog pop up
        assertTrue("Symbols dialog does not pop up", math_SymbolsDlg.exists(3));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Test input Greek character from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testInputGreekFromCatalog() throws Exception{

        // Select "Greek", click "Edit" button to get the selected Symbol
        math_SymbolsDlgListbox.select("Greek");
        math_SymbolsDlgEditButton.click();
        sleep(1);
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlg.ok();
        sleep(1);

        // Insert the selected symbol
        math_SymbolsDlgInsertButton.click();
        math_SymbolsDlgCloseButton.click();

        // Verify if the symbol is inserted successfully
        math_EditWindow.activate();
        math_EditWindow.menuItem("Edit->Select All").select();
        math_EditWindow.menuItem("Edit->Copy").select();
        sleep(1);
        assertEquals("Symbol is not inserted succcessfully", "%".concat(selectedSymbol).concat(" "), app.getClipboard());   // add "%" in the front, add " " in the end

        // Close all dialogs
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test input iGreek character from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testInputIGreekFromCatalog() throws Exception{

        // Select "iGreek", click "Edit" button to get the selected Symbol
        math_SymbolsDlgListbox.select("iGreek");
        math_SymbolsDlgEditButton.click();
        sleep(1);
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlg.ok();
        sleep(1);

        // Insert the selected symbol
        math_SymbolsDlgInsertButton.click();
        math_SymbolsDlgCloseButton.click();

        // Verify if the symbol is inserted successfully
        math_EditWindow.click(5, 5);
        math_EditWindow.menuItem("Edit->Select All").select();
        math_EditWindow.menuItem("Edit->Copy").select();
        sleep(1);
        assertEquals("Symbol is not inserted succcessfully", "%".concat(selectedSymbol).concat(" "), app.getClipboard());   // add "%" in the front, add " " in the end

        // Close all dialogs
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test input Special character from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testInputSpecialFromCatalog() throws Exception{

        // Select "Special", "Insert" the default first symbol
        math_SymbolsDlgListbox.select("Special");
        math_SymbolsDlgInsertButton.click();

        // Click "Edit" button to get the selected Symbol
        math_SymbolsDlgEditButton.click();
        sleep(1);
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlg.ok();
        math_SymbolsDlgCloseButton.click();

        // Verify if the symbol is inserted successfully
        math_EditWindow.click(5, 5);
        math_EditWindow.menuItem("Edit->Select All").select();
        math_EditWindow.menuItem("Edit->Copy").select();
        sleep(1);
        assertEquals("Symbol is not inserted succcessfully", "%".concat(selectedSymbol).concat(" "), app.getClipboard());   // add "%" in the front, add " " in the end

        // Close all dialogs
        app.dispatch(".uno:CloseDoc");
    }
}
