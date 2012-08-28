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
package testcase.gui.formula.catalog;

import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import testlib.gui.Log;

/**
 *
 */
public class AddDeleteSymbolsFromCatalog {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);

        // New a formula document
        app.dispatch("private:factory/smath");
        math_EditWindow.waitForExistence(10, 2);

        // Click catalog button
        math_CatalogButton.click();

        // Select "Special", click "Edit" button
        math_SymbolsDlgListbox.select(2);
        math_SymbolsDlgEditButton.click();
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test add custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testAddSymbolFromCatalog() throws Exception{

        // Choose a symbol which is not in the list, click "Add" and "OK"
        String selectedSymbol;
        boolean bSelectSymbolNotInList;
        int nListCount;
        int nIndex = 1;
        do {
            math_EditSymbolsDlgViewControl.click(100*nIndex, 10);   // risk: after 6 or 7 circles, this will click out of ViewControl
            selectedSymbol = math_EditSymbolsDlgSymbol.getText();
            nIndex++;
            // Find if the selected symbol is already in the list
            bSelectSymbolNotInList = false;
            nListCount = math_EditSymbolsDlgSymbol.getItemCount();
            for (int i=0; i<nListCount; i++) {
                if (selectedSymbol.equals(math_EditSymbolsDlgSymbol.getItemText(i))) {
                    bSelectSymbolNotInList = true;
                    break;
                }
            }
        } while (bSelectSymbolNotInList);
        math_EditSymbolsDlgAdd.click();
        math_EditSymbolsDlg.ok();

        // Verify if the symbol is added to Symbol set
        math_SymbolsDlgEditButton.click();
        bSelectSymbolNotInList = false;
        nListCount = math_EditSymbolsDlgSymbol.getItemCount();
        for (int i=0; i<nListCount; i++) {
            if (selectedSymbol.equals(math_EditSymbolsDlgSymbol.getItemText(i))) {
                bSelectSymbolNotInList = true;
                break;
            }
        }
        assertTrue("Symbol is not added to Symbol set", bSelectSymbolNotInList);

        // Close all dialogs
        math_EditSymbolsDlg.cancel();
        math_SymbolsDlg.close();
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test modify font of custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testModifySymbolFontFromCatalog() throws Exception{

        // Modify the font of selected symbol
        //String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        int oldSymbolFontIndex = math_EditSymbolsDlgFont.getSelIndex();
        int modifiedSymbolFondIndex = (oldSymbolFontIndex+1==math_EditSymbolsDlgFont.getItemCount()) ? 0 : (oldSymbolFontIndex+1);
        math_EditSymbolsDlgFont.select(modifiedSymbolFondIndex);    // select the next font of old font
        String modifiedSymbolFont = math_EditSymbolsDlgFont.getSelText();
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlgModify.click();
        math_EditSymbolsDlg.ok();

        // Verify if the font of symbol is modified successfully
        math_SymbolsDlgEditButton.click();
        math_EditSymbolsDlgSymbol.select(selectedSymbol);
        assertEquals("Font of symbol is not modified successfully", modifiedSymbolFont, math_EditSymbolsDlgFont.getSelText());

        // Close all dialogs
        math_EditSymbolsDlg.cancel();
        math_SymbolsDlg.close();
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test modify typeface of custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testModifySymbolTypefaceFromCatalog() throws Exception{

        // Modify the typeface of selected symbol
//      String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        int oldSymbolTypefaceIndex = math_EditSymbolsDlgTypeface.getSelIndex();
        int modifiedSymbolTypefaceIndex = (oldSymbolTypefaceIndex+1==math_EditSymbolsDlgTypeface.getItemCount()) ? 0 : (oldSymbolTypefaceIndex+1);
        math_EditSymbolsDlgTypeface.select(modifiedSymbolTypefaceIndex);    // select the next typeface of old typeface
        String modifiedSymbolTypeface= math_EditSymbolsDlgTypeface.getSelText();
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlgModify.click();
        math_EditSymbolsDlg.ok();

        // Verify if the typeface of symbol is modified successfully
        math_SymbolsDlgEditButton.click();
        math_EditSymbolsDlgSymbol.select(selectedSymbol);
        assertEquals("Typeface of symbol is not modified successfully", modifiedSymbolTypeface, math_EditSymbolsDlgTypeface.getSelText());

        // Close all dialogs
        math_EditSymbolsDlg.cancel();
        math_SymbolsDlg.close();
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test delete custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testDeleteSymbolFromCatalog() throws Exception{

        // Delete the selected symbol
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlgDelete.click();
        math_EditSymbolsDlg.ok();

        // Verify if the selected symbol is deleted successfully
        math_SymbolsDlgEditButton.click();
        boolean isDeleted = true;
        for (int i=0; i<math_EditSymbolsDlgSymbol.getItemCount(); i++) {
            if (selectedSymbol.equals(math_EditSymbolsDlgSymbol.getItemText(i))){
                isDeleted = false;
                break;
            }
        }
        assertTrue("Symbol is not deleted successfully", isDeleted);

        // Close all dialogs
        math_EditSymbolsDlg.cancel();
        math_SymbolsDlg.close();
        app.dispatch(".uno:CloseDoc");
    }
}
