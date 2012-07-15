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
package testcase.formula.importexport;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;
import static org.junit.Assert.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import testlib.Log;

/**
 *
 */
public class CreateFormulaInDifferentWays {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start();
    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }

    /**
     * Test elements window active and inactive
     * @throws Exception
     */
    @Test
    public void testElementsWindowActive() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Check if the "View->Elements" menu is selected
        math_EditWindow.menuItem("View").select();
        boolean viewElements = math_EditWindow.menuItem("View->Elements").isSelected();

        // If the menu is selected, the Elements window should display
        assertEquals("Elements window displays correctly", viewElements, math_ElementsWindow.exists());

        // Active or inactive the Elements window, it should display correctly
        math_EditWindow.menuItem("View->Elements").select();
        sleep(1);
        math_EditWindow.menuItem("View").select();
        viewElements = math_EditWindow.menuItem("View->Elements").isSelected();
        assertEquals("Elements window display correctly", viewElements, math_ElementsWindow.exists());
    }

    /**
     * Test create a formula from Elements window
     * @throws Exception
     */
    @Test
    public void testCreateFormulaFromElementsWindow() throws Exception{
        String saveTo = fullPath("temp/" + "FormulaFromElements.odf");

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Make Elements window pop up
        math_EditWindow.menuItem("View").select();
        if (!math_EditWindow.menuItem("View->Elements").isSelected()) {
            math_EditWindow.menuItem("View->Elements").select();
        }
        math_EditWindow.click(1, 1);

        // Click a formula in Elements window and edit the formula in the commands window
        math_ElementsRelations.click();
        math_ElementsRelationsNotEqual.click();
        sleep(0.5);
        typeKeys("a");
        math_EditWindow.menuItem("Edit->Next Marker").select();
        sleep(0.5);
        typeKeys("b");
        String insertedFormula = "a <> b";

        // Verify if the formula is correct
        app.dispatch(".uno:Select");
        app.dispatch(".uno:Copy");
        sleep(1);
        assertEquals("The inserted formula into math", insertedFormula.concat(" "), app.getClipboard());    // add " "

        // Save the formula
        math_EditWindow.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);

        // Close and reopen it
        math_EditWindow.menuItem("File->Close").select();
        openStartcenter();
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        math_EditWindow.waitForExistence(10, 2);

        // Verify if the formula still exists in the file, and correct
        math_EditWindow.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        sleep(1);
        assertEquals("The inserted formula into math", insertedFormula.concat(" "), app.getClipboard());    // add " "

        // Close all dialogs
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test create a formula from right click menu in equation editor
     * @throws Exception
     */
    @Test
    public void testCreateFormulaFromRightClickMenu() throws Exception{
        String saveTo = fullPath("temp/" + "FormulaFromRightClickMenu.odf");

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Right click in equation editor, choose "Functions->More->arcsin(x)", input a
        math_EditWindow.rightClick(5, 5);
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<down>");
        typeKeys("<enter>");
        sleep(0.5);
        typeKeys("<up>");
        typeKeys("<enter>");
        sleep(0.5);
        typeKeys("<enter>");
        sleep(0.5);
        typeKeys("a");
        String insertedFormula = "arcsin(a)";

        // Verify if the formula is correct
        math_EditWindow.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        sleep(1);
        assertEquals("The inserted formula into math", insertedFormula.concat(" "), app.getClipboard());    // add " "

        // Save the formula
        math_EditWindow.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);

        // Close and reopen it
        math_EditWindow.menuItem("File->Close").select();
        openStartcenter();
        startcenter.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        math_EditWindow.waitForExistence(10, 2);

        // Verify if the formula still exists in the file, and correct
        math_EditWindow.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        sleep(1);
        assertEquals("The inserted formula into math", insertedFormula.concat(" "), app.getClipboard());    // add " "

        // Close all dialogs
        app.dispatch(".uno:CloseDoc");
    }

    /**
     * Test undo/redo in math
     * @throws Exception
     */
    @Test
    public void testUndoRedoInMath() throws Exception{

        // New a formula document
        startcenter.menuItem("File->New->Formula").select();
        sleep(3);

        // Make Elements window pop up (For AOO3.4: View->Elements)
        math_EditWindow.menuItem("View").select();
        if (!math_EditWindow.menuItem("View->Elements").isSelected()) {
            math_EditWindow.menuItem("View->Elements").select();
        }
        math_EditWindow.click(1, 1);

        // Click a formula in Elements window and edit the formula in the commands window
        math_ElementsUnaryBinary.click();
        math_ElementsUnaryBinaryPlus.click();
        sleep(1);
        typeKeys("a");  // "+a";

        // Undo and verify if it works fine
        math_EditWindow.menuItem("Edit->Undo: Insert").select();
        math_EditWindow.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        assertEquals("The inserted formula into math", "+<?> ", app.getClipboard());    // add " "

        // Redo and verify if it works fine
        math_EditWindow.menuItem("Edit->Redo: Insert").select();
        math_EditWindow.menuItem("Edit->Select All").select();
        app.dispatch(".uno:Copy");
        assertEquals("The inserted formula into math", "+a ", app.getClipboard());  // add " "
    }
}
