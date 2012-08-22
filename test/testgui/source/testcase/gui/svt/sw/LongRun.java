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

package testcase.gui.svt.sw;
import static org.openoffice.test.common.Testspace.*;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppUtil.*;
import static testlib.gui.UIMap.*;

import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.Condition;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

import testlib.gui.CalcUtil;
import testlib.gui.Log;

public class LongRun {

    @Rule
    public Log LOG = new Log();

    private PrintStream result = null;

    private String pid = null;

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        app.start(true);
        result = new PrintStream(new FileOutputStream(Testspace.getFile("output/longrun.csv")));
        HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
        pid = (String)proccessInfo.get("pid");
        result.println("Iterator,Time,Memory(KB),CPU(%)");
        LOG.info("Result will be saved to " + Testspace.getPath("output/longrun.csv"));
    }

    @After
    public void tearDown() throws Exception {
        app.close();
        result.close();
    }

    Rectangle rect = new Rectangle(400, 200, 60, 60);
    Condition condition = new Condition() {
        @Override
        public boolean value() {
            BufferedImage image = GraphicsUtil.screenshot(rect);
            for (int x = 0; x < image.getWidth();x++) {
                for (int y = 0; y < image.getHeight();y++) {
                    if (image.getRGB(x, y) == 0xFF000000) {
                        return true;
                    }
                }
            }
            return false;
        }

    };

    /**
     * Test New/Save a text document
     *
     * @throws Exception
     */
    @Test
    public void testLongRun() throws Exception {
        startcenter.menuItem("File->New->Text Document").select();
        writer.typeKeys("Long-running test...");
        for(int i = 0 ; i < 1000; i++){
            saveNewDocument("helloworld_saveas.odt");
            saveNewDocument("helloworld_saveas.ott");
            saveNewDocument("helloworld_saveas.sxw");
            saveNewDocument("helloworld_saveas.stw");
            saveNewDocument("helloworld_saveas.doc");
            saveNewDocument("helloworld_saveas.txt");
            saveNewSpreadsheet("helloworld_saveas.ods");
            saveNewSpreadsheet("helloworld_saveas.ots");
            saveNewSpreadsheet("helloworld_saveas.sxc");
            saveNewSpreadsheet("helloworld_saveas.stc");
            saveNewSpreadsheet("helloworld_saveas.xls");
            saveNewPresentation("helloworld_saveas.odp");
            saveNewPresentation("helloworld_saveas.otp");
            saveNewPresentation("helloworld_saveas.ppt");
            saveNewPresentation("helloworld_saveas.pot");
            saveNewPresentation("helloworld_saveas.sxi");
            saveNewPresentation("helloworld_saveas.sti");
            saveNewMath("math_saveas.sxm");
            saveNewMath("math_saveas.mml");
            saveNewDrawing("draw_saveas.odg");
            saveNewDrawing("draw_saveas.otg");
            saveNewDrawing("draw_saveas.sxd");
            saveNewDrawing("draw_saveas.std");
            HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
            String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
            LOG.info("Record: " + record);
            result.println(record);
            result.flush();
        }
    }

    private void saveNewDocument(String file) {
        String saveTo = getPath("temp/" + file);
        // Create a new text document
        writer.menuItem("File->New->Text Document").select();
        sleep(3);

        // Input some text by keyboard
        writer.focus();

        String text = "~!@#$%^&*()_+QWERTYUIOP{}|:LKJHGFDSAZXCVBNM<>? ";
        typeText(text);
        writer.menuItem("Edit->Select All").select();
        app.setClipboard(".wrong");
        sleep(1);
        typeKeys("<$copy>");
        sleep(1);

        // Verify the text via system clip board
        Assert.assertEquals("The typed text into writer", text,
                app.getClipboard());

        // Set the text style
        writer.openContextMenu();
        menuItem("Character...").select();
        EffectsPage.select();
        EffectsPage_Color.select("Magenta");
        EffectsPage.ok();
        sleep(2);

        // Save the text document
        writer.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();

        // Close it by clicking main menu
        writer.menuItem("File->Close").select();
//      openStartcenter();
        // Reopen the saved file
        writer.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        writer.waitForExistence(10, 2);

        writer.menuItem("Edit->Select All").select();
        app.setClipboard(".wrong");
        typeKeys("<$copy>");
        sleep(1);
        // Verify if the text still exists in the file
        Assert.assertEquals("The typed text into writer is saved!", text,
                app.getClipboard());
        writer.menuItem("File->Close").select();
    }

    private void saveNewSpreadsheet(String file) {
        String saveTo = getPath("temp/" + file);
        String text = "Hello Openoffice";
        writer.menuItem("File->New->Spreadsheet").select();
        calc.waitForExistence(10, 2);
        CalcUtil.selectRange("A65536");
        typeKeys(text);
        calc.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        // Close it by clicking main menu
        calc.menuItem("File->Close").select();
//      openStartcenter();
        // Reopen the saved file
        writer.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        calc.waitForExistence(10, 2);
        Assert.assertEquals("The typed text is saved!", text,
                CalcUtil.getCellText("A65536"));
        calc.menuItem("File->Close").select();
    }

    private void saveNewPresentation(String file) {
        String saveTo = getPath("temp/" + file);
        String text = "Hello Openoffice";
        writer.menuItem("File->New->Presentation").select();
        PresentationWizard.ok();
        impress.waitForExistence(10, 2);
        sleep(2);
        impress.click(0.01, 0.01);
        typeKeys(text);
        sleep(2);
        impress.doubleClick(0.1, 0.5);

        impress.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok();
        // Close it by clicking main menu
        impress.menuItem("File->Close").select();
//      openStartcenter();
        // Reopen the saved file
        sleep(2);
        writer.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        impress.waitForExistence(10, 2);
        sleep(2);
        impress.click(3, 3);
        typeKeys("<tab><enter>");
        impress.menuItem("Edit->Select All").select();
        // app.setClipboard(".wrong");
        typeKeys("<$copy>");
        sleep(1);
        Assert.assertEquals("The typed text is saved!", text,
                app.getClipboard());
        impress.menuItem("File->Close").select();
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
        writer.menuItem("File->New->Drawing").select();
        sleep(3);

        // Insert a picture fully filled with green
        draw.menuItem("Insert->Picture->From File...").select();
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
        draw.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        // If the format is supported by OO1.0, ask whether to change to the
        // latest format
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok(); // Keep the current format

        // Close it by clicking main menu
        draw.menuItem("File->Close").select();
//      openStartcenter();

        // Reopen the saved file
        writer.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        draw.waitForExistence(10, 2);
        draw.click(5, 5);
        sleep(1);
        // Verify if the picture still exists in the file
        Rectangle rectangle1 = GraphicsUtil.findRectangle(
                draw.getScreenRectangle(), 0xFF00FF00);
        assertNotNull("Green rectangle: " + rectangle1, rectangle1);
        draw.menuItem("File->Close").select();
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
        writer.menuItem("File->New->Formula").select();
        sleep(3);

        // Verify if the Elements window is active
        assertTrue(math_ElementsWindow.exists(3));

        // Insert a formula
        String text = "5 times 3 = 15";
        typeText(text);
        math_EditWindow.menuItem("Edit->Select All").select();
        typeKeys("<$copy>");
        sleep(1);

        // Verify the text via system clip board
        assertEquals("The typed formula into math", text, app.getClipboard());

        // Save the formula
        math_EditWindow.menuItem("File->Save As...").select();
        FileUtil.deleteFile(saveTo);
        submitSaveDlg(saveTo);
        // If the format is supported by OO1.0, ask whether to change to the
        // latest format
        if (AlienFormatDlg.exists(3))
            AlienFormatDlg.ok(); // Keep the current format

        // Close it by clicking main menu
        math_EditWindow.menuItem("File->Close").select();
//      openStartcenter();

        // Reopen the saved file
        writer.menuItem("File->Open...").select();
        submitOpenDlg(saveTo);
        math_EditWindow.waitForExistence(10, 2);

        // Verify if the formula still exists in the file
        math_EditWindow.menuItem("Edit->Select All").select();
        typeKeys("<$copy>");
        sleep(1);
        assertEquals("The typed formula into math is saved", text,
                app.getClipboard());

        // Close the file to avoid the app closing the Elements window
        // automatically
        math_EditWindow.menuItem("File->Close").select();
    }

}
