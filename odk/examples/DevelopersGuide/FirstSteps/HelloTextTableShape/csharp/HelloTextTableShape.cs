/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.awt;
using com.sun.star.beans;
using com.sun.star.container;
using com.sun.star.drawing;
using com.sun.star.frame;
using com.sun.star.lang;
using com.sun.star.sheet;
using com.sun.star.table;
using com.sun.star.text;
using com.sun.star.uno;

XComponentContext xRemoteContext = null;
XMultiComponentFactory xRemoteServiceManager = null;

try
{
    UseWriter();
    UseCalc();
    UseDraw();
    return 0;
}
catch (UnoException e)
{
    Console.Error.WriteLine(e.Message);
    return 1;
}

void UseWriter()
{
    // Create a new Writer document, and manipulate the text content
    XComponent xWriterComponent = NewDocComponent("swriter");
    XTextDocument xTextDocument = xWriterComponent.query<XTextDocument>();
    XText xText = xTextDocument.getText();

    ManipulateText(xText);

    // Get the internal service factory of the document
    XMultiServiceFactory xWriterFactory = xTextDocument.query<XMultiServiceFactory>();

    // Insert a TextTable, and manipulate the text content of the cell
    XTextContent xTextContentTable = xWriterFactory.createInstance("com.sun.star.text.TextTable").query<XTextContent>();
    xText.insertTextContent(xText.getEnd(), xTextContentTable, false);

    XCellRange xCellRange = xTextContentTable.query<XCellRange>();
    XCell xCell = xCellRange.getCellByPosition(0, 1);
    XText xCellText = xCell.query<XText>();

    ManipulateText(xCellText);
    ManipulateTable(xCellRange);

    XShape xWriterShape = xWriterFactory.createInstance("com.sun.star.drawing.RectangleShape").query<XShape>();
    xWriterShape.setSize(new Size(10000, 10000));
    XTextContent xTextContentShape = xWriterShape.query<XTextContent>();

    xText.insertTextContent(xText.getEnd(), xTextContentShape, false);

    // Wrap the text inside the shape
    XPropertySet xShapeProps = xWriterShape.query<XPropertySet>();
    xShapeProps.setPropertyValue("TextContourFrame", new Any(true));

    XText xShapeText = xWriterShape.query<XText>();

    ManipulateText(xShapeText);
    ManipulateShape(xWriterShape);

    // Create and name a bookmark
    IQueryInterface bookmark = xWriterFactory.createInstance("com.sun.star.text.Bookmark");
    XNamed xNamed = bookmark.query<XNamed>();
    xNamed.setName("MyUniqueBookmarkName");

    // Insert the bookmark at the end of the document
    XTextContent xTextContent = bookmark.query<XTextContent>();
    xText.insertTextContent(xText.getEnd(), xTextContent, false);

    // Get all bookmarks from the XBookmarksSupplier
    XBookmarksSupplier xBookmarksSupplier = xWriterComponent.query<XBookmarksSupplier>();
    XNameAccess xNamedBookmarks = xBookmarksSupplier.getBookmarks();
    XTextContent xFoundBookmark = xNamedBookmarks.getByName("MyUniqueBookmarkName").cast<XTextContent>();
    XTextRange xFound = xFoundBookmark.getAnchor();
    xFound.setString(" The throat mike, glued to her neck, "
        + "looked as much as possible like an analgesic dermadisk.");

    // Get all tables from the XTextTablesSupplier
    XTextTablesSupplier xTablesSupplier = xWriterComponent.query<XTextTablesSupplier>();
    XNameAccess xNamedTables = xTablesSupplier.getTextTables();
    XIndexAccess xIndexedTables = xNamedTables.query<XIndexAccess>();

    // Get the tables
    XPropertySet xTableProps = null;
    for (int i = 0; i < xIndexedTables.getCount(); i++)
    {
        xTableProps = xIndexedTables.getByIndex(i).cast<XPropertySet>();
        xTableProps.setPropertyValue("BackColor", new Any(0xC8FFB9));
    }
}

void UseCalc()
{
    // Create a new Calc document, and manipulate text in a cell
    XComponent xCalcComponent = NewDocComponent("scalc");
    XSpreadsheetDocument xCalcDocument = xCalcComponent.query<XSpreadsheetDocument>();
    XSpreadsheetDocument xSpreadsheetDocument = xCalcDocument.query<XSpreadsheetDocument>();

    XIndexAccess xIndexedSheets = xSpreadsheetDocument.getSheets().query<XIndexAccess>();
    XCellRange xSpreadsheetCells = xIndexedSheets.getByIndex(0).cast<XCellRange>();

    // Get cell A2 from the first sheet
    XCell xCell = xSpreadsheetCells.getCellByPosition(0, 1);
    XText xCellText = xCell.query<XText>();

    XPropertySet xCellProps = xCell.query<XPropertySet>();
    xCellProps.setPropertyValue("IsTextWrapped", new Any(true));

    ManipulateText(xCellText);
    ManipulateTable(xSpreadsheetCells);

    // Get the internal service factory of the document
    XMultiServiceFactory xCalcFactory = xCalcDocument.query<XMultiServiceFactory>();
    XDrawPageSupplier xDrawPageSupplier = xIndexedSheets.getByIndex(0).cast<XDrawPageSupplier>();
    XDrawPage xDrawPage = xDrawPageSupplier.getDrawPage();

    // Create and insert a RectangleShape, and manipulate its shape text
    XShape xCalcShape = xCalcFactory.createInstance("com.sun.star.drawing.RectangleShape").query<XShape>();
    xCalcShape.setSize(new Size(10000, 10000));
    xCalcShape.setPosition(new Point(7000, 3000));
    xDrawPage.add(xCalcShape);

    // Wrap the text inside the shape
    XPropertySet xShapeProps = xCalcShape.query<XPropertySet>();
    xShapeProps.setPropertyValue("TextContourFrame", new Any(true));

    XText xShapeText = xCalcShape.query<XText>();
    ManipulateText(xShapeText);
    ManipulateShape(xCalcShape);
}

void UseDraw()
{
    // Create a new Draw document, and insert a rectangle
    XComponent xDrawComponent = NewDocComponent("sdraw");
    XDrawPagesSupplier xDrawPagesSupplier = xDrawComponent.query<XDrawPagesSupplier>();

    XIndexAccess xIndexedDrawPages = xDrawPagesSupplier.getDrawPages();
    XDrawPage xDrawPage = xIndexedDrawPages.getByIndex(0).cast<XDrawPage>();

    // Get the internal service factory of the document
    XMultiServiceFactory xDrawFactory = xDrawComponent.query<XMultiServiceFactory>();
    XShape xDrawShape = xDrawFactory.createInstance("com.sun.star.drawing.RectangleShape").query<XShape>();
    xDrawShape.setSize(new Size(10000, 20000));
    xDrawShape.setPosition(new Point(5000, 5000));
    xDrawPage.add(xDrawShape);

    // Wrap the text inside the shape
    XPropertySet xShapeProps = xDrawShape.query<XPropertySet>();
    xShapeProps.setPropertyValue("TextContourFrame", new Any(true));

    XText xShapeText = xDrawShape.query<XText>();
    ManipulateText(xShapeText);
    ManipulateShape(xDrawShape);
}

void ManipulateText(XText xText)
{
    // Set the entire text at once
    xText.setString("He lay flat on the brown, pine-needled floor of the forest, "
        + "his chin on his folded arms, and high overhead the wind blew in the tops "
        + "of the pine trees.");

    // Create a text cursor for selecting and formatting
    XTextCursor xTextCursor = xText.createTextCursor();
    XPropertySet xCursorProps = xTextCursor.query<XPropertySet>();

    // Use the cursor to select "He lay", and apply bold and italic formatting
    xTextCursor.gotoStart(false);
    xTextCursor.goRight(6, true);

    xCursorProps.setPropertyValue("CharPosture", new Any(FontSlant.ITALIC));
    xCursorProps.setPropertyValue("CharWeight", new Any(FontWeight.BOLD));

    xTextCursor.gotoEnd(false);
    xText.insertString(xTextCursor.getStart(),
        " The mountainside sloped gently where he lay; "
        + "but below it was steep and he could see the dark of the oiled road "
        + "winding through the pass. There was a stream alongside the road "
        + "and far down the pass he saw a mill beside the stream and the falling water "
        + "of the dam, white in the summer sunlight.", false);
    xText.insertString(xTextCursor.getStart(), "\n  \"Is that the mill?\" he asked.", false);
}

void ManipulateTable(XCellRange xCellRange)
{
    // Set column titles and a cell value
    XCell xCell = xCellRange.getCellByPosition(0, 0);
    XText xCellText = xCell.query<XText>();
    xCellText.setString("Quotation");

    xCell = xCellRange.getCellByPosition(1, 0);
    xCellText = xCell.query<XText>();
    xCellText.setString("Year");

    xCell = xCellRange.getCellByPosition(1, 1);
    xCell.setValue(1940);

    XCellRange xSelectedCells = xCellRange.getCellRangeByName("A1:B1");
    XPropertySet xCellProps = xSelectedCells.query<XPropertySet>();

    string backColorPropertyName = null;
    XPropertySet xTableProps = null;

    // Format the table headers and borders
    XServiceInfo xServiceInfo = xCellRange.query<XServiceInfo>();
    if (xServiceInfo.supportsService("com.sun.star.sheet.Spreadsheet"))
    {
        backColorPropertyName = "CellBackColor";
        xSelectedCells = xCellRange.getCellRangeByName("A1:B2");
        xTableProps = xSelectedCells.query<XPropertySet>();
    }
    else if (xServiceInfo.supportsService("com.sun.star.text.TextTable"))
    {
        backColorPropertyName = "BackColor";
        xTableProps = xCellRange.query<XPropertySet>();
    }
    // Set cell background color
    xCellProps.setPropertyValue(backColorPropertyName, new Any(0x99CCFF));

    // Set table borders (blue line, width 10)
    BorderLine theLine = new BorderLine()
    {
        Color = 0x000099,
        OuterLineWidth = 10,
    };
    
    // Use the above defined line for all borders and enable them
    TableBorder bord = new TableBorder()
    {
        VerticalLine = theLine,
        HorizontalLine = theLine,
        LeftLine = theLine,
        RightLine = theLine,
        TopLine = theLine,
        BottomLine = theLine,

        IsVerticalLineValid = true,
        IsHorizontalLineValid = true,
        IsLeftLineValid = true,
        IsRightLineValid = true,
        IsTopLineValid = true,
        IsBottomLineValid = true,
    };
    xTableProps.setPropertyValue("TableBorder", new Any(bord));

    bord = xTableProps.getPropertyValue("TableBorder").cast<TableBorder>();
    Console.WriteLine(bord.TopLine.Color);
}

void ManipulateShape(XShape xShape)
{
    XPropertySet xShapeProps = xShape.query<XPropertySet>();
    xShapeProps.setPropertyValue("FillColor", new Any(0x99CCFF));
    xShapeProps.setPropertyValue("LineColor", new Any(0x000099));
    xShapeProps.setPropertyValue("RotateAngle", new Any(3000));
    xShapeProps.setPropertyValue("TextLeftDistance", new Any(0));
    xShapeProps.setPropertyValue("TextRightDistance", new Any(0));
    xShapeProps.setPropertyValue("TextUpperDistance", new Any(0));
    xShapeProps.setPropertyValue("TextLowerDistance", new Any(0));
}

XComponent NewDocComponent(string docType)
{
    XMultiComponentFactory xRemoteServiceManager = GetRemoteServiceManager();

    XComponentLoader xComponentLoader = xRemoteServiceManager
        .createInstanceWithContext("com.sun.star.frame.Desktop", xRemoteContext)
        .query<XComponentLoader>();

    string loadUrl = $"private:factory/{docType}";
    return xComponentLoader.loadComponentFromURL(loadUrl, "_blank", 0, new PropertyValue[0]);
}

XMultiComponentFactory GetRemoteServiceManager()
{
    if (xRemoteContext == null && xRemoteServiceManager == null)
    {
        xRemoteContext = NativeBootstrap.bootstrap();
        Console.WriteLine("Connected to a running office ...");

        xRemoteServiceManager = xRemoteContext.getServiceManager();
    }
    return xRemoteServiceManager;
}