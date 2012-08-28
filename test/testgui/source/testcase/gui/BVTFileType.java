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
package testcase.gui;

import static org.openoffice.test.common.Testspace.*;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.awt.Rectangle;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

/**
 *
 */
public class BVTFileType {

    @Rule
    public Log LOG = new Log();

    @Before
    public void setUp() throws Exception {
        app.start(true);
    }

    @After
    public void tearDown() throws Exception {

    }

    /**
     * Test New/Save a text document
     *
     * @throws Exception
     */
    @Test
    public void testSaveNewODT() throws Exception {
        saveNewDocument("helloworld_saveas.odt");
    }

    @Test
    public void testSaveNewOTT() throws Exception {
        saveNewDocument("helloworld_saveas.ott");
    }

    @Test
    public void testSaveNewSXW() throws Exception {
        saveNewDocument("helloworld_saveas.sxw");
    }

    @Test
    public void testSaveNewSTW() throws Exception {
        saveNewDocument("helloworld_saveas.stw");
    }

    @Test
    public void testSaveNewDOC() throws Exception {
        saveNewDocument("helloworld_saveas.doc");
    }

    @Test
    public void testSaveNewTXT() throws Exception {
        saveNewDocument("helloworld_saveas.txt");
    }

    private void saveNewDocument(String file) {
        String saveTo = getPath("temp/" + file);
        // Create a new text document
        app.dispatch("private:factory/swriter");
        writer.waitForExistence(10, 2);

        // Input some text by keyboard
        writer.focus();

        String text = "~!@#$%^&*()_+QWERTYUIOP{}|:LKJHGFDSAZXCVBNM<>? ";
        typeText(text);
        app.dispatch(".uno:SelectAll");
        app.setClipboard(".wrong");
        sleep(1);
        typeKeys("<$copy>");
        sleep(1);

        // Verify the text via system clip board
        Assert.assertEquals("The typed text into writer", text,
                app.getClipboard());

        // Set the text style
        writer.openContextMenu();
        // menuItem("Text Properties...").select();
        app.dispatch(".uno:FontDialog");
        EffectsPage.select();
        EffectsPage_Color.select(6);
        EffectsPage.ok();
        sleep(2);

        // Save the text document
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();

        // Close it by clicking main menu
        app.dispatch(".uno:CloseDoc");
        openStartcenter();
        // Reopen the saved file
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        writer.waitForExistence(10, 2);

        app.dispatch(".uno:SelectAll");
        app.setClipboard(".wrong");
        typeKeys("<$copy>");
        sleep(1);
        // Verify if the text still exists in the file
        Assert.assertEquals("The typed text into writer is saved!", text,
                app.getClipboard());
    }

    @Test
    public void testSaveNewODS() throws Exception {
        saveNewSpreadsheet("helloworld_saveas.ods");
    }

    @Test
    public void testSaveNewOTS() throws Exception {
        saveNewSpreadsheet("helloworld_saveas.ots");
    }

    @Test
    public void testSaveNewSXC() throws Exception {
        saveNewSpreadsheet("helloworld_saveas.sxc");
    }

    @Test
    public void testSaveNewSTC() throws Exception {
        saveNewSpreadsheet("helloworld_saveas.stc");
    }

    // @Test
    // public void testSaveNewCSV() throws Exception {
    // saveNewSpreadsheet("helloworld_saveas.csv");
    // }

    @Test
    public void testSaveNewXLS() throws Exception {
        saveNewSpreadsheet("helloworld_saveas.xls");
    }

    private void saveNewSpreadsheet(String file) {
        String saveTo = getPath("temp/" + file);
        String text = "Hello Openoffice";
        app.dispatch("private:factory/scalc");
        calc.waitForExistence(10, 2);
        CalcUtil.selectRange("A65536");
        typeKeys(text);
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        // Close it by clicking main menu
        app.dispatch(".uno:CloseDoc");
        openStartcenter();
        // Reopen the saved file
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        Assert.assertEquals("The typed text is saved!", text,
                CalcUtil.getCellText("A65536"));
    }

    @Test
    public void testSaveNewODP() throws Exception {
        saveNewPresentation("helloworld_saveas.odp");
    }

    @Test
    public void testSaveNewOTP() throws Exception {
        saveNewPresentation("helloworld_saveas.otp");
    }

    @Test
    public void testSaveNewPPT() throws Exception {
        saveNewPresentation("helloworld_saveas.ppt");
    }

    @Test
    public void testSaveNewPOT() throws Exception {
        saveNewPresentation("helloworld_saveas.pot");
    }

    @Test
    public void testSaveNewSXI() throws Exception {
        saveNewPresentation("helloworld_saveas.sxi");
    }

    @Test
    public void testSaveNewSTI() throws Exception {
        saveNewPresentation("helloworld_saveas.sti");
    }

    private void saveNewPresentation(String file) {
        String saveTo = getPath("temp/" + file);
        String text = "Hello Openoffice";
        app.dispatch("private:factory/simpress?slot=6686");
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);
        impress.click(0.01, 0.01);
        typeKeys(text);
        sleep(2);
        impress.doubleClick(0.1, 0.5);

        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        // Close it by clicking main menu
        app.dispatch(".uno:CloseDoc");
        openStartcenter();
        // Reopen the saved file
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        impress.waitForExistence(10, 2);
        impress.click(3, 3);
        typeKeys("<tab><enter>");
        app.dispatch(".uno:SelectAll");
        // app.setClipboard(".wrong");
        typeKeys("<$copy>");
        sleep(1);
        Assert.assertEquals("The typed text is saved!", text, app
                .getClipboard().trim());
    }

    // drawing

    /**
     * Test save a new drawing as .odg
     */
    @Test
    public void testSaveNewODG() throws Exception {
        saveNewDrawing("draw_saveas.odg");
    }

    /**
     * Test save a new drawing as .otg
     */
    @Test
    public void testSaveNewOTG() throws Exception {
        saveNewDrawing("draw_saveas.otg");
    }

    /**
     * Test save a new drawing as .sxd
     */
    @Test
    public void testSaveNewSXD() throws Exception {
        saveNewDrawing("draw_saveas.sxd");
    }

    /**
     * Test save a new drawing as .std
     */
    @Test
    public void testSaveNewSTD() throws Exception {
        saveNewDrawing("draw_saveas.std");
    }

    /**
     * New/Save a draw document 1. New a draw document 2. Insert a picture 3.
     * Save it as the input filename 4. Reopen the saved file 5. Check if the
     * picture is still there
     *
     * @param filename
     *            : filename to be saved
     * @throws Exception
     */
    public void saveNewDrawing(String filename) {
        String saveTo = getPath("temp/" + filename);
        String bmp_green = prepareData("pure_green_64x64.bmp");

        // Create a new drawing document
        app.dispatch("private:factory/sdraw");
        draw.waitForExistence(10, 2);

        // Insert a picture fully filled with green
        app.dispatch(".uno:InsertGraphic");
        submitOpenDlg(bmp_green);
        sleep(3);
        // Focus on edit pane
        draw.click(5, 5);
        sleep(1);

        // Verify if the picture is inserted successfully
        Rectangle rectangle = GraphicsUtil.findRectangle(
                draw.getScreenRectangle(), 0xFF00FF00);
        assertNotNull("Green rectangle: " + rectangle, rectangle);

        // Save the drawing
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        // If the format is supported by OO1.0, ask whether to change to the
        // latest format
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok(); // Keep the current format

        // Close it by clicking main menu
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen the saved file
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        draw.waitForExistence(10, 2);

        // Verify if the picture still exists in the file
        Rectangle rectangle1 = GraphicsUtil.findRectangle(
                draw.getScreenRectangle(), 0xFF00FF00);
        assertNotNull("Green rectangle: " + rectangle1, rectangle1);
    }

    // math
    /**
     * Test save a new math as .odf
     */
    @Test
    public void testSaveNewODF() throws Exception {
        saveNewMath("math_saveas.odf");
    }

    /**
     * Test save a new math as .sxm
     */
    @Test
    public void testSaveNewSXM() throws Exception {
        saveNewMath("math_saveas.sxm");
    }

    /**
     * Test save a new math as .mml
     */
    @Test
    public void testSaveNewMML() throws Exception {
        saveNewMath("math_saveas.mml");
    }

    /**
     * New/Save a math 1. New a math 2. Insert a formula 3. Save it as the input
     * filename 4. Reopen the saved file 5. Check if the formula is still there
     *
     * @param filename
     *            : filename to be saved
     * @throws Exception
     */
    public void saveNewMath(String filename) {
        String saveTo = getPath("temp/" + filename);

        // Create a new math
        app.dispatch("private:factory/smath");
        math_EditWindow.waitForExistence(10, 2);

        // Verify if the Elements window is active
        assertTrue(math_ElementsWindow.exists(3));

        // Insert a formula
        String text = "5 times 3 = 15";
        typeText(text);
        app.dispatch(".uno:Select");
        typeKeys("<$copy>");
        sleep(1);

        // Verify the text via system clip board
        assertEquals("The typed formula into math", text, app.getClipboard());

        // Save the formula
        app.dispatch(".uno:SaveAs");
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        // If the format is supported by OO1.0, ask whether to change to the
        // latest format
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok(); // Keep the current format

        // Close it by clicking main menu
        app.dispatch(".uno:CloseDoc");
        openStartcenter();

        // Reopen the saved file
        app.dispatch(".uno:Open");
        submitOpenDlg(saveTo);
        math_EditWindow.waitForExistence(10, 2);

        // Verify if the formula still exists in the file
        app.dispatch(".uno:Select");
        typeKeys("<$copy>");
        sleep(1);
        assertEquals("The typed formula into math is saved", text,
                app.getClipboard());

        // Close the file to avoid the app closing the Elements window
        // automatically
        app.dispatch(".uno:CloseDoc");
    }
}
