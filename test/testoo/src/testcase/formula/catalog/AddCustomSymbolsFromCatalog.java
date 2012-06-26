/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

/**
 *
 */
package testcase.formula.catalog;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 *
 */
public class AddCustomSymbolsFromCatalog {

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
     * Test add custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Ignore("Add button in Edit Symbols dialog is disabled")
    public void testAddSymbolFromCatalog() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Click catalog button
        math_CatalogButton.click();
        sleep(1);

        // Verify if the "Symbols" dialog pop up
        assertTrue("Symbols dialog does not pop up", math_SymbolsDlg.exists(3));

        // Select "Special", click "Edit" button
        math_SymbolsDlgListbox.select("Special");
        math_SymbolsDlgEditButton.click();
        sleep(1);

        // Verify if the "Edit Symbols" dialog pop up
        assertTrue("Edit Symbols dialog does not pop up", math_EditSymbolsDlg.exists(3));

        // Choose a symbol which is not in the list, click "Add" and "OK"
        String selectedSymbol;
        boolean bSelectSymbolNotInList;
        int nListCount;
        int nIndex = 1;
        do {
            math_EditSymbolsDlgViewControl.click(100*nIndex, 10);   // risk: after 6 or 7 circles, this will click out of ViewControl
            sleep(1);
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
        sleep(1);

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
    }

    /**
     * Test modify font of custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testModifySymbolFontFromCatalog() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Click catalog button
        math_CatalogButton.click();
        sleep(1);

        // Verify if the "Symbols" dialog pop up
        assertTrue("Symbols dialog does not pop up", math_SymbolsDlg.exists(3));

        // Select "Special", click "Edit" button
        math_SymbolsDlgListbox.select("Special");
        math_SymbolsDlgEditButton.click();
        sleep(1);

        // Verify if the "Edit Symbols" dialog pop up
        assertTrue("Edit Symbols dialog does not pop up", math_EditSymbolsDlg.exists(3));

        // Modify the font of selected symbol
        //String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        int oldSymbolFontIndex = math_EditSymbolsDlgFont.getSelIndex();
        int modifiedSymbolFondIndex = (oldSymbolFontIndex+1==math_EditSymbolsDlgFont.getItemCount()) ? 0 : (oldSymbolFontIndex+1);
        math_EditSymbolsDlgFont.select(modifiedSymbolFondIndex);    // select the next font of old font
        String modifiedSymbolFont = math_EditSymbolsDlgFont.getSelText();
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlgModify.click();
        math_EditSymbolsDlg.ok();
        sleep(1);

        // Verify if the font of symbol is modified successfully
        math_SymbolsDlgEditButton.click();
        math_EditSymbolsDlgSymbol.select(selectedSymbol);
        assertEquals("Font of symbol is not modified successfully", modifiedSymbolFont, math_EditSymbolsDlgFont.getSelText());
    }

    /**
     * Test modify typeface of custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testModifySymbolTypefaceFromCatalog() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Click catalog button
        math_CatalogButton.click();
        sleep(1);

        // Verify if the "Symbols" dialog pop up
        assertTrue("Symbols dialog does not pop up", math_SymbolsDlg.exists(3));

        // Select "Special", click "Edit" button
        math_SymbolsDlgListbox.select("Special");
        math_SymbolsDlgEditButton.click();
        sleep(1);

        // Verify if the "Edit Symbols" dialog pop up
        assertTrue("Edit Symbols dialog does not pop up", math_EditSymbolsDlg.exists(3));

        // Modify the typeface of selected symbol
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        int oldSymbolTypefaceIndex = math_EditSymbolsDlgTypeface.getSelIndex();
        int modifiedSymbolTypefaceIndex = (oldSymbolTypefaceIndex+1==math_EditSymbolsDlgTypeface.getItemCount()) ? 0 : (oldSymbolTypefaceIndex+1);
        math_EditSymbolsDlgTypeface.select(modifiedSymbolTypefaceIndex);    // select the next typeface of old typeface
        String modifiedSymbolTypeface= math_EditSymbolsDlgTypeface.getSelText();
        math_EditSymbolsDlgModify.click();
        math_EditSymbolsDlg.ok();
        sleep(1);

        // Verify if the typeface of symbol is modified successfully
        math_SymbolsDlgEditButton.click();
        math_EditSymbolsDlgSymbol.select(selectedSymbol);
        assertEquals("Typeface of symbol is not modified successfully", modifiedSymbolTypeface, math_EditSymbolsDlgTypeface.getSelText());
    }
}
