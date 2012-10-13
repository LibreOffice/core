/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.lang.XComponent;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XCell;
import com.sun.star.table.TableBorder;
import com.sun.star.table.BorderLine;
import com.sun.star.drawing.XShape;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPage;

public class HelloTextTableShape {

    private XComponentContext xRemoteContext = null;
    private XMultiComponentFactory xRemoteServiceManager = null;

    /** Creates a new instance of HelloTextTableShape */
    public HelloTextTableShape() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        HelloTextTableShape helloTextTableShape1 = new HelloTextTableShape();
        try {
            helloTextTableShape1.useDocuments();
        }
        catch (java.lang.Exception e){
            System.err.println(e.getMessage());
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }

    }

    protected void useDocuments() throws java.lang.Exception  {
        useWriter();
        useCalc();
        useDraw();
    }

    protected void useWriter() throws java.lang.Exception {
        try {
            // create new writer document and get text, then manipulate text
            XComponent xWriterComponent = newDocComponent("swriter");
            XTextDocument xTextDocument = (XTextDocument)UnoRuntime.queryInterface(
                XTextDocument.class, xWriterComponent);
            XText xText = xTextDocument.getText();

            manipulateText(xText);

            // get internal service factory of the document
            XMultiServiceFactory xWriterFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xWriterComponent);

            // insert TextTable and get cell text, then manipulate text in cell
            Object table = xWriterFactory.createInstance("com.sun.star.text.TextTable");
            XTextContent xTextContentTable = (XTextContent)UnoRuntime.queryInterface(
                XTextContent.class, table);

            xText.insertTextContent(xText.getEnd(), xTextContentTable, false);

            XCellRange xCellRange = (XCellRange)UnoRuntime.queryInterface(
                XCellRange.class, table);
            XCell xCell = xCellRange.getCellByPosition(0, 1);
            XText xCellText = (XText)UnoRuntime.queryInterface(XText.class, xCell);

            manipulateText(xCellText);
            manipulateTable(xCellRange);

            // insert RectangleShape and get shape text, then manipulate text
            Object writerShape = xWriterFactory.createInstance(
                "com.sun.star.drawing.RectangleShape");
            XShape xWriterShape = (XShape)UnoRuntime.queryInterface(
                XShape.class, writerShape);
            xWriterShape.setSize(new Size(10000, 10000));
            XTextContent xTextContentShape = (XTextContent)UnoRuntime.queryInterface(
                XTextContent.class, writerShape);

            xText.insertTextContent(xText.getEnd(), xTextContentShape, false);

            XPropertySet xShapeProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, writerShape);
            // wrap text inside shape
            xShapeProps.setPropertyValue("TextContourFrame", new Boolean(true));


            XText xShapeText = (XText)UnoRuntime.queryInterface(XText.class, writerShape);

            manipulateText(xShapeText);
            manipulateShape(xWriterShape);

/* more code snippets used in the manual:

            Object bookmark = xWriterFactory.createInstance ( "com.sun.star.text.Bookmark" );
            // name the bookmark
            XNamed xNamed = (XNamed) UnoRuntime.queryInterface (
                    XNamed.class, bookmark );
            xNamed.setName("MyUniqueBookmarkName");

            // get XTextContent interface and insert it at the end of the document
            XTextContent xTextContent = (XTextContent) UnoRuntime.queryInterface (
                    XTextContent.class, bookmark );
            //mxDocText.insertTextContent ( mxDocText.getEnd(), xTextContent, false );
            xText.insertTextContent ( xText.getEnd(), xTextContent, false );

                        //query BookmarksSupplier
                        XBookmarksSupplier xBookmarksSupplier = (XBookmarksSupplier)UnoRuntime.queryInterface(
                            XBookmarksSupplier.class, xWriterComponent);
                        XNameAccess xNamedBookmarks = xBookmarksSupplier.getBookmarks();
                        Object foundBookmark = xNamedBookmarks.getByName("MyUniqueBookmarkName");
                        XTextContent xFoundBookmark = (XTextContent)UnoRuntime.queryInterface(XTextContent.class, foundBookmark);
                        XTextRange xFound = xFoundBookmark.getAnchor();
                        xFound.setString(" The throat mike, glued to her neck, "
                            + "looked as much as possible like an analgesic dermadisk.");




            // first query the XTextTablesSupplier interface from our document
            XTextTablesSupplier xTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(
                    XTextTablesSupplier.class, xWriterComponent);
            // get the tables collection
            XNameAccess xNamedTables = xTablesSupplier.getTextTables();

            // now query the XIndexAccess from the tables collection
            XIndexAccess xIndexedTables = (XIndexAccess) UnoRuntime.queryInterface(
                    XIndexAccess.class, xNamedTables);

            // we need properties
            XPropertySet xTableProps = null;

            // get the tables
            for (int i = 0; i < xIndexedTables.getCount(); i++) {
                    //Object table = xIndexedTables.getByIndex(i);
                    table = xIndexedTables.getByIndex(i);
                    xTableProps = (XPropertySet) UnoRuntime.queryInterface(
                            XPropertySet.class, table);
                    xTableProps.setPropertyValue("BackColor", new Integer(0xC8FFB9));
            }
 */
        }
        catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
            xRemoteContext = null;
            throw e;
        }

    }

    protected void useCalc() throws java.lang.Exception {
        try {
            // create new calc document and manipulate cell text
            XComponent xCalcComponent = newDocComponent("scalc");
            XSpreadsheetDocument  xSpreadsheetDocument  =
                (XSpreadsheetDocument)UnoRuntime.queryInterface(
                    XSpreadsheetDocument .class, xCalcComponent);
            Object sheets = xSpreadsheetDocument.getSheets();
            XIndexAccess xIndexedSheets = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class, sheets);
            Object sheet =  xIndexedSheets.getByIndex(0);

            //get cell A2 in first sheet
            XCellRange xSpreadsheetCells = (XCellRange)UnoRuntime.queryInterface(
                XCellRange.class, sheet);
            XCell xCell = xSpreadsheetCells.getCellByPosition(0,1);
            XPropertySet xCellProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xCell);
            xCellProps.setPropertyValue("IsTextWrapped", new Boolean(true));

            XText xCellText = (XText)UnoRuntime.queryInterface(XText.class, xCell);

            manipulateText(xCellText);
            manipulateTable(xSpreadsheetCells);

            // get internal service factory of the document
            XMultiServiceFactory xCalcFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xCalcComponent);
            // get Drawpage
            XDrawPageSupplier xDrawPageSupplier = (XDrawPageSupplier)UnoRuntime.queryInterface(XDrawPageSupplier.class, sheet);
            XDrawPage xDrawPage = xDrawPageSupplier.getDrawPage();

            // create and insert RectangleShape and get shape text, then manipulate text
            Object calcShape = xCalcFactory.createInstance(
                "com.sun.star.drawing.RectangleShape");
            XShape xCalcShape = (XShape)UnoRuntime.queryInterface(
                XShape.class, calcShape);
            xCalcShape.setSize(new Size(10000, 10000));
            xCalcShape.setPosition(new Point(7000, 3000));

            xDrawPage.add(xCalcShape);

            XPropertySet xShapeProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, calcShape);
            // wrap text inside shape
            xShapeProps.setPropertyValue("TextContourFrame", new Boolean(true));


            XText xShapeText = (XText)UnoRuntime.queryInterface(XText.class, calcShape);

            manipulateText(xShapeText);
            manipulateShape(xCalcShape);

        }
        catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
            xRemoteContext = null;
            throw e;
        }

    }

    protected void useDraw() throws java.lang.Exception {
        try {
            //create new draw document and insert ractangle shape
            XComponent xDrawComponent = newDocComponent("sdraw");
            XDrawPagesSupplier xDrawPagesSupplier =
                (XDrawPagesSupplier)UnoRuntime.queryInterface(
                    XDrawPagesSupplier.class, xDrawComponent);

            Object drawPages = xDrawPagesSupplier.getDrawPages();
            XIndexAccess xIndexedDrawPages = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class, drawPages);
            Object drawPage = xIndexedDrawPages.getByIndex(0);
            XDrawPage xDrawPage = (XDrawPage)UnoRuntime.queryInterface(XDrawPage.class, drawPage);

            // get internal service factory of the document
            XMultiServiceFactory xDrawFactory =
                (XMultiServiceFactory)UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, xDrawComponent);

            Object drawShape = xDrawFactory.createInstance(
                "com.sun.star.drawing.RectangleShape");
            XShape xDrawShape = (XShape)UnoRuntime.queryInterface(XShape.class, drawShape);
            xDrawShape.setSize(new Size(10000, 20000));
            xDrawShape.setPosition(new Point(5000, 5000));
            xDrawPage.add(xDrawShape);

            XText xShapeText = (XText)UnoRuntime.queryInterface(XText.class, drawShape);
            XPropertySet xShapeProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, drawShape);

            // wrap text inside shape
            xShapeProps.setPropertyValue("TextContourFrame", new Boolean(true));

            manipulateText(xShapeText);
            manipulateShape(xDrawShape);
        }
        catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
            xRemoteContext = null;
            throw e;
        }


    }

    protected void manipulateText(XText xText) throws com.sun.star.uno.Exception {
        // simply set whole text as one string
        xText.setString("He lay flat on the brown, pine-needled floor of the forest, "
            + "his chin on his folded arms, and high overhead the wind blew in the tops "
            + "of the pine trees.");

        // create text cursor for selecting and formatting
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, xTextCursor);

        // use cursor to select "He lay" and apply bold italic
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short)6, true);
        // from CharacterProperties
        xCursorProps.setPropertyValue("CharPosture",
            com.sun.star.awt.FontSlant.ITALIC);
        xCursorProps.setPropertyValue("CharWeight",
            new Float(com.sun.star.awt.FontWeight.BOLD));

        // add more text at the end of the text using insertString
        xTextCursor.gotoEnd(false);
        xText.insertString(xTextCursor, " The mountainside sloped gently where he lay; "
            + "but below it was steep and he could see the dark of the oiled road "
            + "winding through the pass. There was a stream alongside the road "
            + "and far down the pass he saw a mill beside the stream and the falling water "
            + "of the dam, white in the summer sunlight.", false);
        // after insertString the cursor is behind the inserted text, insert more text
        xText.insertString(xTextCursor, "\n  \"Is that the mill?\" he asked.", false);
    }

    protected void manipulateTable(XCellRange xCellRange) throws com.sun.star.uno.Exception {

        String backColorPropertyName = "";
        XPropertySet xTableProps = null;

        // enter column titles and a cell value
        XCell xCell = xCellRange.getCellByPosition(0,0);
        XText xCellText = (XText)UnoRuntime.queryInterface(XText.class, xCell);
        xCellText.setString("Quotation");
        xCell = xCellRange.getCellByPosition(1,0);
        xCellText = (XText)UnoRuntime.queryInterface(XText.class, xCell);
        xCellText.setString("Year");
        xCell = xCellRange.getCellByPosition(1,1);
        xCell.setValue(1940);
        XCellRange xSelectedCells = xCellRange.getCellRangeByName("A1:B1");
        XPropertySet xCellProps = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, xSelectedCells);

        // format table headers and table borders
        // we need to distinguish text and sheet tables:
        // property name for cell colors is different in text and sheet cells
        // we want to apply TableBorder to whole text table, but only to sheet cells with content
        XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(
            XServiceInfo.class, xCellRange);
        if (xServiceInfo.supportsService("com.sun.star.sheet.Spreadsheet")) {
            backColorPropertyName = "CellBackColor";
            xSelectedCells = xCellRange.getCellRangeByName("A1:B2");
            xTableProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xSelectedCells);
        }
        else if (xServiceInfo.supportsService("com.sun.star.text.TextTable")) {
            backColorPropertyName = "BackColor";
            xTableProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xCellRange);
        }
        // set cell background color
        xCellProps.setPropertyValue(backColorPropertyName, new Integer(0x99CCFF));

        // set table borders
        // create description for blue line, width 10
        BorderLine theLine = new BorderLine();
        theLine.Color = 0x000099;
        theLine.OuterLineWidth = 10;
        // apply line description to all border lines and make them valid
        TableBorder bord = new TableBorder();
        bord.VerticalLine = bord.HorizontalLine =
            bord.LeftLine = bord.RightLine =
            bord.TopLine = bord.BottomLine =
                theLine;
        bord.IsVerticalLineValid = bord.IsHorizontalLineValid =
            bord.IsLeftLineValid = bord.IsRightLineValid =
            bord.IsTopLineValid = bord.IsBottomLineValid =
                true;

        xTableProps.setPropertyValue("TableBorder", bord);

        bord = (TableBorder)xTableProps.getPropertyValue("TableBorder");
        theLine = bord.TopLine;
        int col = theLine.Color;
        System.out.println(col);
    }

    protected void manipulateShape(XShape xShape) throws com.sun.star.uno.Exception {
        XPropertySet xShapeProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShape);
        xShapeProps.setPropertyValue("FillColor", new Integer(0x99CCFF));
        xShapeProps.setPropertyValue("LineColor", new Integer(0x000099));
        xShapeProps.setPropertyValue("RotateAngle", new Integer(3000));

        xShapeProps.setPropertyValue("TextLeftDistance", new Integer(0));
        xShapeProps.setPropertyValue("TextRightDistance", new Integer(0));
        xShapeProps.setPropertyValue("TextUpperDistance", new Integer(0));
        xShapeProps.setPropertyValue("TextLowerDistance", new Integer(0));
    }


    protected XComponent newDocComponent(String docType) throws java.lang.Exception {
        String loadUrl = "private:factory/" + docType;
        xRemoteServiceManager = this.getRemoteServiceManager();
        Object desktop = xRemoteServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", xRemoteContext);
        XComponentLoader xComponentLoader = (XComponentLoader)UnoRuntime.queryInterface(
            XComponentLoader.class, desktop);
        PropertyValue[] loadProps = new PropertyValue[0];
        return xComponentLoader.loadComponentFromURL(loadUrl, "_blank", 0, loadProps);
    }

    protected XMultiComponentFactory getRemoteServiceManager() throws java.lang.Exception {
        if (xRemoteContext == null && xRemoteServiceManager == null) {
            try {
                // First step: get the remote office component context
                xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
                System.out.println("Connected to a running office ...");

                xRemoteServiceManager = xRemoteContext.getServiceManager();
            }
            catch( Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
        }
        return xRemoteServiceManager;
    }
}
