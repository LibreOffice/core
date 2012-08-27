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
package testcase.uno.sw.breaks;

import static org.openoffice.test.common.Testspace.*;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.Assert;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.SWUtil;

import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XText;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XTextViewCursor;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XTextViewCursorSupplier;
import com.sun.star.style.BreakType;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;

public class CheckBreaks {
    UnoApp unoApp = new UnoApp();
    XTextDocument textDocument = null;
    File temp = null;
    String tempFilePathODT = "";
    String tempFilePathDOC = "";

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        unoApp.start();

        FileUtil.deleteFile(getPath("temp"));
        temp = new File(getPath("temp"));
        temp.mkdirs();

        tempFilePathODT = temp + "/tempFilePathODT.odt";
        tempFilePathDOC = temp + "/tempFilePathDOC.doc";
    }

    @After
    public void tearDown() throws Exception {
        unoApp.close();
    }

    /**
     * test line break can be inserted and deleted.
     * when save it as odt file, close and reopen, line break can be inserted and deleted.
     * when save it as doc file, close and reopen, line break can be inserted and deleted.
     * \n represent line break
     * @throws Exception
     */
    @Test
    public void testInsertDeleteLineBreak() throws Exception
    {
        XComponent xComponent = unoApp.newDocument("swriter");
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
        this.insertNewLine(textDocument, "Line1", true);
        this.insertNewLine(textDocument, "Line2", false);
        this.insertNewLine(textDocument, "Line3", false);
        this.insertNewLine(textDocument, "Line4", false);

        int lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is inserted when new document.",4,lineCount);
        this.deleteLineBreak(textDocument);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is deleted when new document", 3,lineCount);

        //save to odt, test the line break
        SWUtil.saveAsODT(textDocument, FileUtil.getUrl(tempFilePathODT));
        unoApp.closeDocument(xComponent);
        xComponent = unoApp.loadDocument(tempFilePathODT);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,xComponent);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line breaks when open saved odt file.",3,lineCount);
        this.insertNewLine(textDocument, "Line added when open saved odt file", false);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is inserted when open saved odt file.",4,lineCount);
        this.deleteLineBreak(textDocument);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is deleted when open saved odt file.",3,lineCount);

        //save to doc, test the line break
        SWUtil.saveAs(textDocument, "MS Word 97", FileUtil.getUrl(tempFilePathDOC));
        unoApp.closeDocument(xComponent);
        xComponent = unoApp.loadDocument(tempFilePathDOC);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,xComponent);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line breaks when open saved doc file.",3,lineCount);
        this.insertNewLine(textDocument, "Line added when open saved doc file", false);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is inserted when open saved doc file.",4,lineCount);
        this.deleteLineBreak(textDocument);
        lineCount = this.getLineCount(textDocument);
        Assert.assertEquals("Line break is deleted when open saved doc file.",3,lineCount);

        unoApp.closeDocument(xComponent);
    }

    @Test
    public void testInsertDeletePageBreak() throws Exception
    {
        //new document, test page break
        XComponent xComponent = unoApp.newDocument("swriter");
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xComponent);
        this.insertNewPage(textDocument, "Page1", true);
        this.insertNewPage(textDocument, "Page2", false);
        this.insertNewPage(textDocument, "Page3", false);
        int pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is inserted when new document", 3,pageCount);

        this.deleteFirstPage(textDocument);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is deleted when new document", 2,pageCount);

        //save as odt, test page break
        SWUtil.saveAsODT(textDocument, FileUtil.getUrl(tempFilePathODT));
        unoApp.closeDocument(xComponent);
        xComponent = unoApp.loadDocument(tempFilePathODT);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,xComponent);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("Page breaks after open saved odt file", 2,pageCount);
        this.insertNewPage(textDocument, "Page4", false);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is inserted after open saved odt file", 3,pageCount);

        this.deleteFirstPage(textDocument);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is deleted after open saved odt file.", 2,pageCount);

        //save as doc, test page break
        SWUtil.saveAs(textDocument, "MS Word 97", FileUtil.getUrl(tempFilePathDOC));
        unoApp.closeDocument(xComponent);
        xComponent = unoApp.loadDocument(tempFilePathDOC);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class,xComponent);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("Page breaks after open saved doc file", 2,pageCount);

        this.deleteFirstPage(textDocument);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is deleted after open saved doc file.", 1,pageCount);

        this.insertNewPage(textDocument, "Page5", false);
        pageCount = SWUtil.getPageCount(textDocument);
        Assert.assertEquals("page break is inserted after open saved doc file", 2,pageCount);

        unoApp.closeDocument(xComponent);
    }




    /**
     * insert a new page at the end of the document
     * @param xTextDocument
     * @param pageContent
     * @param isFirstPage
     * @throws Exception
     */
    private void insertNewPage(XTextDocument document, String pageContent, Boolean isFirstPage) throws Exception
    {
        XText xText = document.getText();
        XTextCursor textCursor = xText.createTextCursor();
        textCursor.gotoEnd(false);
        System.out.println("The content before insert " + pageContent + ":" + xText.getString());
        if(!isFirstPage)
        {
            XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, textCursor);
            xCursorProps.setPropertyValue("BreakType", BreakType.PAGE_AFTER);
            document.getText().insertControlCharacter(textCursor,ControlCharacter.PARAGRAPH_BREAK,false);
        }
        document.getText().insertString(textCursor, pageContent, false);
    }

    /**
     * delete the first page of the document
     * @param document
     * @throws Exception
     */
    private void deleteFirstPage(XTextDocument document) throws Exception
    {
        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, document);
        XController xController = xModel.getCurrentController();
        XTextViewCursorSupplier xTextViewCursorSupplier =
                (XTextViewCursorSupplier) UnoRuntime.queryInterface(XTextViewCursorSupplier.class, xController);
        XTextViewCursor textViewCursor = xTextViewCursorSupplier.getViewCursor();

        XPageCursor pageCursor = (XPageCursor) UnoRuntime.queryInterface(XPageCursor.class, textViewCursor);

        // Move the cursor to the start of the document
        textViewCursor.gotoStart(false);

        pageCursor.jumpToFirstPage();
        XTextCursor textCursor = textViewCursor.getText().createTextCursorByRange(textViewCursor.getStart());

        pageCursor.jumpToEndOfPage();
        textCursor.gotoRange(textViewCursor.getEnd(), true);
        //System.out.println("deleted: " + textCursor.getString());
        textCursor.setString("");

     // Page contents cleared, now delete the page break at the start
        textCursor.collapseToStart();
        if(textCursor.goRight((short) 1, true)){
            //System.out.println("page break deleted: " + textCursor.getString());
            textCursor.setString("");
        }

    }


    /**
     * insert a new line at the end of the document.
     * @param xText
     * @param lineContent
     * @param isFirstLine
     * @throws Exception
     */
    private void insertNewLine(XTextDocument xTextDocument, String lineContent, Boolean isFirstLine) throws Exception
    {
        XText xText = xTextDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(false);
        if(!isFirstLine)
        {
            xText.insertControlCharacter(xTextCursor, ControlCharacter.LINE_BREAK, false);
        }

        xText.insertString(xTextCursor, lineContent, false);
    }

    /**
     * delete first line break
     * @param xText
     * @throws Exception
     */
    private void deleteLineBreak(XTextDocument xTextDocument) throws Exception
    {
        XText xText = xTextDocument.getText();
        String content = xText.getString();
        content = content.replaceFirst("\n", "");
        xText.setString(content);
    }

    /**
     * get line count of all text.
     * \n represent line break.
     * @param xText
     * @return count of line breaks
     * @throws Exception
     */
    private int getLineCount(XTextDocument xTextDocument) throws Exception
    {
        int count = 1;
        XText xText = xTextDocument.getText();
        String content = xText.getString();
        int index = content.indexOf("\n");
        while(index >=0)
        {
            count ++;
            content = content.substring(index+1);
            index = content.indexOf("\n");
        }
        return count;
    }

}
