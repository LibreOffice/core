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
public class InputSpecialCharacterFromCatalogDlg {

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
     * Test input Special character from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testInputSpecialFromCatalog() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Click catalog button
        math_CatalogButton.click();
        sleep(1);

        // Verify if the "Symbols" dialog pop up
        assertTrue("Symbols dialog does not pop up", math_SymbolsDlg.exists(3));

        // Select "Special", click "Edit" button to get the selected Symbol
        math_SymbolsDlgListbox.select("Special");
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
        typeKeys("<$copy>");
        sleep(1);
        assertEquals("Symbol is not inserted succcessfully", "%".concat(selectedSymbol), app.getClipboard());   // add "%"
    }
}
