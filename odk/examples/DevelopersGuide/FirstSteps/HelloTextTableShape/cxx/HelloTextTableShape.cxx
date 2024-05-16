/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

using namespace css;
using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::beans;
using namespace css::sheet;
using namespace css::table;
using namespace css::text;
using namespace css::drawing;
using namespace css::awt;
using namespace css::container;
using namespace rtl;
using namespace awt;

void manipulateText(const Reference<XText>& xText);
void manipulateTable(const Reference<XCellRange>& xCellRange);
void manipulateShape(const Reference<XShape>& xShape);
void useDocuments();
void useWriter();
void useCalc();
void useDraw();
Reference<XComponentContext> getComponentContext();
Reference<XMultiComponentFactory> getRemoteServiceManager();
Reference<XComponent> newDocComponent(const OUString docType);

Reference<XComponentContext> xRemoteContext = NULL;
Reference<XMultiComponentFactory> xRemoteServiceManager = NULL;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        useDocuments();
    }
    catch (RuntimeException& e)
    {
        std::cerr << e.Message << "\n";
        return 1;
    }
    return 0;
}

void useDocuments()
{
    useWriter();
    useCalc();
    useDraw();
}

void useWriter()
{
    // create new writer document and get text, then manipulate text
    Reference<XComponent> xWriterComponent = newDocComponent("swriter");
    Reference<XTextDocument> xTextDocument(xWriterComponent, UNO_QUERY_THROW);
    Reference<XText> xText = xTextDocument->getText();

    manipulateText(xText);

    // get internal service factory of the document
    Reference<lang::XMultiServiceFactory> xWriterFactory
        = Reference<XMultiServiceFactory>(xTextDocument, UNO_QUERY_THROW);

    // insert TextTable and get cell text, then manipulate text in cell
    Reference<XTextContent> xTextContentTable = Reference<XTextContent>(
        xWriterFactory->createInstance("com.sun.star.text.TextTable"), UNO_QUERY_THROW);

    xText->insertTextContent(xText->getEnd(), xTextContentTable, false);

    Reference<XCellRange> xCellRange = Reference<XCellRange>(xTextContentTable, UNO_QUERY_THROW);
    Reference<XCell> xCell = xCellRange->getCellByPosition(0, 1);
    Reference<XText> xCellText = Reference<XText>(xCell, UNO_QUERY_THROW);

    manipulateText(xCellText);
    manipulateTable(xCellRange);

    Reference<XShape> xWriterShape = Reference<XShape>(
        xWriterFactory->createInstance("com.sun.star.drawing.RectangleShape"), UNO_QUERY_THROW);
    xWriterShape->setSize(Size(10000, 10000));
    Reference<XTextContent> xTextContentShape(xWriterShape, UNO_QUERY_THROW);

    xText->insertTextContent(xText->getEnd(), xTextContentShape, false);

    Reference<XPropertySet> xShapeProps(xWriterShape, UNO_QUERY_THROW);
    // wrap text inside shape
    xShapeProps->setPropertyValue("TextContourFrame", Any(true));

    Reference<XText> xShapeText = Reference<XText>(xWriterShape, UNO_QUERY_THROW);

    manipulateText(xShapeText);
    manipulateShape(xWriterShape);

    // more code snippets used in the manual:

    Reference<XInterface> bookmark = xWriterFactory->createInstance("com.sun.star.text.Bookmark");
    // name the bookmark
    Reference<XNamed> xNamed(bookmark, UNO_QUERY_THROW);
    xNamed->setName("MyUniqueBookmarkName");

    // get XTextContent interface and insert it at the end of the document
    Reference<XTextContent> xTextContent(bookmark, UNO_QUERY_THROW);
    xText->insertTextContent(xText->getEnd(), xTextContent, false);

    //query BookmarksSupplier
    Reference<XBookmarksSupplier> xBookmarksSupplier(xWriterComponent, UNO_QUERY_THROW);
    Reference<XNameAccess> xNamedBookmarks = xBookmarksSupplier->getBookmarks();
    Any foundBookmark = xNamedBookmarks->getByName("MyUniqueBookmarkName");
    Reference<XTextContent> xFoundBookmark(foundBookmark, UNO_QUERY_THROW);
    Reference<XTextRange> xFound = xFoundBookmark->getAnchor();
    xFound->setString(" The throat mike, glued to her neck, "
                      "looked as much as possible like an analgesic dermadisk.");

    // first query the XTextTablesSupplier interface from our document
    Reference<XTextTablesSupplier> xTablesSupplier(xWriterComponent, UNO_QUERY_THROW);
    // get the tables collection
    Reference<XNameAccess> xNamedTables = xTablesSupplier->getTextTables();

    // now query the XIndexAccess from the tables collection
    Reference<XIndexAccess> xIndexedTables(xNamedTables, UNO_QUERY_THROW);

    // we need properties
    Reference<XPropertySet> xTableProps = NULL;

    // get the tables
    for (int i = 0; i < xIndexedTables->getCount(); i++)
    {
        //Object table = xIndexedTables.getByIndex(i);
        Any table = xIndexedTables->getByIndex(i);
        xTableProps = Reference<XPropertySet>(table, UNO_QUERY_THROW);
        xTableProps->setPropertyValue("BackColor", Any(static_cast<sal_Int32>(0xC8FFB9)));
    }
}

void useCalc()
{
    // create new calc document and manipulate cell text
    Reference<XComponent> xCalcComponent = newDocComponent("scalc");
    Reference<XSpreadsheetDocument> xCalcDocument(xCalcComponent, UNO_QUERY_THROW);
    Reference<XSpreadsheetDocument> xSpreadsheetDocument
        = Reference<XSpreadsheetDocument>(xCalcDocument, UNO_QUERY);

    Reference<XIndexAccess> xIndexedSheets(xSpreadsheetDocument->getSheets(), UNO_QUERY);
    Reference<XCellRange> xSpreadsheetCells(xIndexedSheets->getByIndex(0), UNO_QUERY_THROW);

    //get cell A2 in first sheet
    Reference<XCell> xCell = xSpreadsheetCells->getCellByPosition(0, 1);
    Reference<XText> xCellText = Reference<XText>(xCell, UNO_QUERY_THROW);

    Reference<XPropertySet> xCellProps(xCell, UNO_QUERY_THROW);
    xCellProps->setPropertyValue("IsTextWrapped", Any(true));

    manipulateText(xCellText);
    manipulateTable(xSpreadsheetCells);

    // get internal service factory of the document
    Reference<XMultiServiceFactory> xCalcFactory
        = Reference<XMultiServiceFactory>(xCalcDocument, UNO_QUERY_THROW);
    // get Drawpage
    Reference<XDrawPageSupplier> xDrawPageSupplier(xIndexedSheets->getByIndex(0), UNO_QUERY_THROW);
    Reference<XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);

    // create and insert RectangleShape and get shape text, then manipulate text
    Reference<XShape> xCalcShape(
        xCalcFactory->createInstance("com.sun.star.drawing.RectangleShape"), UNO_QUERY_THROW);
    xCalcShape->setSize(Size(10000, 10000));
    xCalcShape->setPosition(Point(7000, 3000));
    xDrawPage->add(xCalcShape);

    Reference<XPropertySet> xShapeProps(xCalcShape, UNO_QUERY_THROW);
    // wrap text inside shape
    xShapeProps->setPropertyValue("TextContourFrame", Any(true));

    Reference<XText> xShapeText = Reference<XText>(xCalcShape, UNO_QUERY_THROW);
    manipulateText(xShapeText);
    manipulateShape(xCalcShape);
}

void useDraw()
{
    //create new draw document and insert ractangle shape
    Reference<XComponent> xDrawComponent = newDocComponent("sdraw");
    Reference<XDrawPagesSupplier> xDrawPagesSupplier(xDrawComponent, UNO_QUERY_THROW);

    Reference<XIndexAccess> xIndexedDrawPages(xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW);
    Reference<XDrawPage> xDrawPage(xIndexedDrawPages->getByIndex(0), UNO_QUERY_THROW);

    // get internal service factory of the document
    Reference<lang::XMultiServiceFactory> xDrawFactory
        = Reference<lang::XMultiServiceFactory>(xDrawComponent, UNO_QUERY_THROW);
    Reference<XShape> xDrawShape(
        xDrawFactory->createInstance("com.sun.star.drawing.RectangleShape"), UNO_QUERY_THROW);
    xDrawShape->setSize(Size(10000, 20000));
    xDrawShape->setPosition(Point(5000, 5000));
    xDrawPage->add(xDrawShape);

    Reference<XPropertySet> xShapeProps(xDrawShape, UNO_QUERY_THROW);
    // wrap text inside shape
    xShapeProps->setPropertyValue("TextContourFrame", Any(true));

    Reference<XText> xShapeText = Reference<XText>(xDrawShape, UNO_QUERY_THROW);
    manipulateText(xShapeText);
    manipulateShape(xDrawShape);
}

void manipulateText(const Reference<XText>& xText)
{
    // simply set whole text as one string
    xText->setString("He lay flat on the brown, pine-needled floor of the forest, "
                     "his chin on his folded arms, and high overhead the wind blew in the tops "
                     "of the pine trees.");

    // create text cursor for selecting and formatting
    Reference<XTextCursor> xTextCursor = xText->createTextCursor();
    Reference<XPropertySet> xCursorProps(xTextCursor, UNO_QUERY_THROW);

    // use cursor to select "He lay" and apply bold italic
    xTextCursor->gotoStart(false);
    xTextCursor->goRight(6, true);

    // from CharacterProperties
    xCursorProps->setPropertyValue("CharPosture", Any(FontSlant_ITALIC));
    xCursorProps->setPropertyValue("CharWeight", Any(FontWeight::BOLD));

    xTextCursor->gotoEnd(false);
    xText->insertString(
        xTextCursor->getStart(),
        " The mountainside sloped gently where he lay; "
        "but below it was steep and he could see the dark of the oiled road "
        "winding through the pass. There was a stream alongside the road "
        "and far down the pass he saw a mill beside the stream and the falling water "
        "of the dam, white in the summer sunlight.",
        false);
    xText->insertString(xTextCursor->getStart(), "\n  \"Is that the mill?\" he asked.", false);
}

void manipulateTable(const Reference<XCellRange>& xCellRange)
{
    OUString backColorPropertyName;
    Reference<XPropertySet> xTableProps = NULL;

    // enter column titles and a cell value
    Reference<XCell> xCell = xCellRange->getCellByPosition(0, 0);
    Reference<XText> xCellText = Reference<XText>(xCell, UNO_QUERY_THROW);
    xCellText->setString("Quotation");
    xCell = xCellRange->getCellByPosition(1, 0);
    xCellText = Reference<XText>(xCell, UNO_QUERY_THROW);
    xCellText->setString("Year");
    xCell = xCellRange->getCellByPosition(1, 1);
    xCell->setValue(1940);
    Reference<XCellRange> xSelectedCells = xCellRange->getCellRangeByName("A1:B1");
    Reference<XPropertySet> xCellProps(xSelectedCells, UNO_QUERY_THROW);

    // format table headers and table borders
    // we need to distinguish text and sheet tables:
    // property name for cell colors is different in text and sheet cells
    // we want to apply TableBorder to whole text table, but only to sheet cells with content
    Reference<XServiceInfo> xServiceInfo(xCellRange, UNO_QUERY_THROW);
    if (xServiceInfo->supportsService("com.sun.star.sheet.Spreadsheet"))
    {
        backColorPropertyName = "CellBackColor";
        xSelectedCells = xCellRange->getCellRangeByName("A1:B2");
        xTableProps = Reference<XPropertySet>(xSelectedCells, UNO_QUERY_THROW);
    }
    else if (xServiceInfo->supportsService("com.sun.star.text.TextTable"))
    {
        backColorPropertyName = "BackColor";
        xTableProps = Reference<XPropertySet>(xCellRange, UNO_QUERY_THROW);
    }
    // set cell background color
    xCellProps->setPropertyValue(backColorPropertyName, Any(static_cast<sal_Int32>(0x99CCFF)));

    // set table borders
    // create description for blue line, width 10
    BorderLine theLine;
    theLine.Color = 0x000099;
    theLine.OuterLineWidth = 10;

    // apply line description to all border lines and make them valid
    TableBorder bord;
    bord.VerticalLine = bord.HorizontalLine = bord.LeftLine = bord.RightLine = bord.TopLine
        = bord.BottomLine = theLine;
    bord.IsVerticalLineValid = bord.IsHorizontalLineValid = bord.IsLeftLineValid
        = bord.IsRightLineValid = bord.IsTopLineValid = bord.IsBottomLineValid = true;
    xTableProps->setPropertyValue("TableBorder", Any(bord));

    xTableProps->getPropertyValue("TableBorder") >>= bord;
    theLine = bord.TopLine;
    int col = theLine.Color;
    std::cout << col << "\n";
}

void manipulateShape(const Reference<XShape>& xShape)
{
    Reference<XPropertySet> xShapeProps = Reference<XPropertySet>(xShape, UNO_QUERY_THROW);
    xShapeProps->setPropertyValue("FillColor", Any(static_cast<sal_Int32>(0x99CCFF)));
    xShapeProps->setPropertyValue("LineColor", Any(static_cast<sal_Int32>(0x000099)));
    xShapeProps->setPropertyValue("RotateAngle", Any(static_cast<sal_Int32>(3000)));

    xShapeProps->setPropertyValue("TextLeftDistance", Any(static_cast<sal_Int32>(0)));
    xShapeProps->setPropertyValue("TextRightDistance", Any(static_cast<sal_Int32>(0)));
    xShapeProps->setPropertyValue("TextUpperDistance", Any(static_cast<sal_Int32>(0)));
    xShapeProps->setPropertyValue("TextLowerDistance", Any(static_cast<sal_Int32>(0)));
}

Reference<XComponent> newDocComponent(const OUString docType)
{
    OUString loadUrl = "private:factory/" + docType;
    Reference<XMultiComponentFactory> xRemoteServiceManager = getRemoteServiceManager();
    Reference<XInterface> desktop = xRemoteServiceManager->createInstanceWithContext(
        "com.sun.star.frame.Desktop", xRemoteContext);
    Reference<XComponentLoader> xComponentLoader
        = Reference<XComponentLoader>(desktop, UNO_QUERY_THROW);
    Sequence<PropertyValue> loadProps(0);
    Reference<XComponent> xDocument
        = xComponentLoader->loadComponentFromURL(loadUrl, "_blank", 0, loadProps);
    return xDocument;
}

Reference<XMultiComponentFactory> getRemoteServiceManager()
{
    if (xRemoteContext == NULL && xRemoteServiceManager == NULL)
    {
        try
        {
            // First step: get the remote office component context
            xRemoteContext = bootstrap();
            std::cout << "Connected to a running office ...\n";
            xRemoteServiceManager = xRemoteContext->getServiceManager();
        }
        catch (Exception e)
        {
            std::cerr << e.Message << "\n";
            exit(1);
        }
    }
    return xRemoteServiceManager;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
