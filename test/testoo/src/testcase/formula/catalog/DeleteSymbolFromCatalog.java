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
public class DeleteSymbolFromCatalog {

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
     * Test delete custom symbols from Catalog->Symbols
     * @throws Exception
     */
    @Test
    public void testDeleteSymbolFromCatalog() throws Exception{

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

        // Delete the selected symbol
        String selectedSymbol = math_EditSymbolsDlgSymbol.getText();
        math_EditSymbolsDlgDelete.click();
        math_EditSymbolsDlg.ok();
        sleep(1);

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
    }
}
