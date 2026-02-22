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
#include <rtl/bootstrap.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>

using namespace cppu;
using namespace rtl;
using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::bridge;
using namespace css::frame;
using namespace css::text;
using namespace css::table;
using namespace css::drawing;
using namespace css::container;
using namespace css::awt;
using namespace css::uno;

Reference<XComponent> openWriter(Reference<XComponentContext> xContext);
void insertIntoCell(OUString cellName, OUString theText, Reference<XTextTable> xTextTable);

// comment: Step 1: bootstrap UNO and get the remote component context
//          Step 2: open an empty text document
//          Step 3: enter some text
//          Step 4: insert a text table
//          Step 5: insert colored text
//          Step 6: insert a text frame

int main()
{
    //oooooooooooooooooooooooooooStep 1oooooooooooooooooooooooooooooooooooooooo
    // bootstrap UNO and get the remote component context. The context can
    // be used to get the service manager

    Reference<XComponentContext> xContext = NULL;
    try
    {
        // get the remote office component context
        xContext = bootstrap();
    }
    catch (Exception& e)
    {
        std::cout << "Error: cannot do bootstrapping." << std::endl << e.Message << std::endl;
        exit(1);
    }

    //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooo
    // open an empty document. In this case it's a writer document.
    // For this purpose an instance of com.sun.star.frame.Desktop
    // is created. It's interface XDesktop provides the XComponentLoader,
    // which is used to open the document via loadComponentFromURL

    Reference<XComponent> xComponent = NULL;
    xComponent = openWriter(xContext);
    Reference<XTextDocument> xTextDocument = NULL;
    try
    {
        xTextDocument.set(xComponent, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Could not get document." << std::endl << e.Message << std::endl;
        exit(1);
    }

    //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooo
    // insert some text.
    // For this purpose get the Text-Object of the document and create the
    // cursor. Now it is possible to insert a text at the cursor-position
    // via insertString

    // get text object
    Reference<XText> xText = xTextDocument->getText();

    // create cursor object
    Reference<XTextCursor> xTextCursor = xText->createTextCursor();
    Reference<XTextRange> xTextRange = NULL;
    try
    {
        xTextRange.set(xTextCursor, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Could not get text range from cursor." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // insert some text
    xText->insertString(xTextRange, "The first line in the newly created text document.\n", false);
    xText->insertString(xTextRange, "Now we're in the second line.\n", false);

    //oooooooooooooooooooooooooooStep 4oooooooooooooooooooooooooooooooooooooooo
    // insert a text table.
    // For this purpose get MultiServiceFactory of the document, create an
    // instance of com.sun.star.text.TextTable and initialize it. Now it can
    // be inserted at the cursor position via insertTextContent.
    // After that some properties are changed and some data is inserted.

    // inserting a text table
    std::cout << "Inserting a text table" << std::endl;

    // get MSF of document
    Reference<XMultiServiceFactory> xMSF = NULL;
    try
    {
        xMSF.set(xTextDocument, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get multi-service factory." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // create text table
    Reference<XTextTable> xTextTable = NULL;
    Reference<XTextContent> xTextContent = NULL;
    try
    {
        xTextTable.set(xMSF->createInstance("com.sun.star.text.TextTable"), UNO_QUERY_THROW);
        xTextContent.set(xTextTable, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't create table." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // initialize text table with 4 columns and 4 rows
    xTextTable->initialize(4, 4);

    Reference<XPropertySet> xRowProps = NULL;

    // insert table
    try
    {
        xText->insertTextContent(xTextRange, xTextContent, false);
        // get first row properties
        Reference<XTableRows> xTextTableRows = xTextTable->getRows();
        xRowProps.set(xTextTableRows->getByIndex(0), UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't insert table." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // get table props
    Reference<XPropertySet> xTableProps = NULL;
    try
    {
        xTableProps.set(xTextTable, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get table properties." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // change back color
    try
    {
        xTableProps->setPropertyValue("BackTransparent", Any(sal_Bool(false)));
        xTableProps->setPropertyValue("BackColor", Any(sal_Int32(13421823)));
        xRowProps->setPropertyValue("BackTransparent", Any(sal_Bool(false)));
        xRowProps->setPropertyValue("BackColor", Any(sal_Int32(6710932)));
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't change table color." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // write text in the table headers
    std::cout << "Write text in the table headers" << std::endl;

    insertIntoCell("A1", "FirstColumn", xTextTable);
    insertIntoCell("B1", "SecondColumn", xTextTable);
    insertIntoCell("C1", "ThirdColumn", xTextTable);
    insertIntoCell("D1", "SUM", xTextTable);

    //Insert Something in the text table
    std::cout << "Insert something in the text table" << std::endl;

    (xTextTable->getCellByName("A2"))->setValue(22.5);
    (xTextTable->getCellByName("B2"))->setValue(5615.3);
    (xTextTable->getCellByName("C2"))->setValue(-2315.7);
    (xTextTable->getCellByName("D2"))->setFormula("sum <A2:C2>");
    (xTextTable->getCellByName("A3"))->setValue(21.5);
    (xTextTable->getCellByName("B3"))->setValue(615.3);
    (xTextTable->getCellByName("C3"))->setValue(-315.7);
    (xTextTable->getCellByName("D3"))->setFormula("sum <A3:C3>");
    (xTextTable->getCellByName("A4"))->setValue(121.5);
    (xTextTable->getCellByName("B4"))->setValue(-615.3);
    (xTextTable->getCellByName("C4"))->setValue(415.7);
    (xTextTable->getCellByName("D4"))->setFormula("sum <A4:C4>");

    //oooooooooooooooooooooooooooStep 5oooooooooooooooooooooooooooooooooooooooo
    // insert a colored text.
    // Get the propertySet of the cursor, change the CharColor and add a
    // shadow. Then insert the Text via InsertString at the cursor position.

    // get property set of the cursor
    Reference<XPropertySet> xCursorProps = NULL;
    try
    {
        xCursorProps.set(xTextCursor, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get cursor properties." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // change the color and add a shadow
    try
    {
        xCursorProps->setPropertyValue("CharColor", Any(sal_Int32(255)));
        xCursorProps->setPropertyValue("CharShadowed", Any(sal_Bool(true)));
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't change table color." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // create a paragraph break
    try
    {
        xText->insertControlCharacter(xTextRange, ControlCharacter::PARAGRAPH_BREAK, false);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't insert paragraph break." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // insert colored text
    std::cout << "Inserting colored text" << std::endl;

    xText->insertString(xTextRange, "This is a colored text - blue with shadow\n", false);

    // create a paragraph break
    try
    {
        xText->insertControlCharacter(xTextRange, ControlCharacter::PARAGRAPH_BREAK, false);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't insert paragraph break." << std::endl << e.Message << std::endl;
        exit(1);
    }

    //oooooooooooooooooooooooooooStep 6oooooooooooooooooooooooooooooooooooooooo
    // insert a text frame.
    // create an instance of com.sun.star.text.TextFrame using the MSF of the
    // document. Change some properties an insert it.
    // Now get the text-Object of the frame and the corresponding cursor.
    // Insert some text via insertString.

    // create a text frame
    Reference<XTextFrame> xTextFrame = NULL;
    Reference<XTextContent> xFrameContent = NULL;
    try
    {
        xTextFrame.set(xMSF->createInstance("com.sun.star.text.TextFrame"), UNO_QUERY_THROW);
        xFrameContent.set(xTextFrame, UNO_QUERY_THROW);
        Reference<XShape> xFrameShape(xTextFrame, UNO_QUERY_THROW);
        Size frameSize(15000, 400); //(width, height)
        xFrameShape->setSize(frameSize);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't create text frame." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // get property set of the text frame
    Reference<XPropertySet> xFrameProps = NULL;
    try
    {
        xFrameProps.set(xTextFrame, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get frame properties." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // change the anchor type
    try
    {
        xFrameProps->setPropertyValue("AnchorType", Any(TextContentAnchorType_AS_CHARACTER));
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't change frame anchor type." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // insert the text frame
    std::cout << "Inserting the text frame" << std::endl;
    try
    {
        xText->insertTextContent(xTextRange, xFrameContent, false);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't insert frame." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // get text from frame
    Reference<XText> xFrameText = xTextFrame->getText();

    // create cursor object
    Reference<XTextCursor> xFrameCursor = xFrameText->createTextCursor();
    Reference<XTextRange> xFrameRange = NULL;
    try
    {
        xFrameRange.set(xFrameCursor, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get frame text range from cursor." << std::endl
                  << e.Message << std::endl;
        exit(1);
    }

    // insert some text
    xFrameText->insertString(xFrameRange, "The first line in the newly created text frame.", false);
    xFrameText->insertString(xFrameRange, "\nWith this second line the height of the frame raises.",
                             false);

    // create a paragraph break
    try
    {
        xText->insertControlCharacter(xTextRange, ControlCharacter::PARAGRAPH_BREAK, false);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't insert paragraph break." << std::endl << e.Message << std::endl;
        exit(1);
    }

    // change the color and remove shadow
    try
    {
        xCursorProps->setPropertyValue("CharColor", Any(sal_Int32(65536)));
        xCursorProps->setPropertyValue("CharShadowed", Any(sal_Bool(false)));
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't change text color." << std::endl << e.Message << std::endl;
        exit(1);
    }

    xText->insertString(xTextRange, " That's all for now !!", false);

    return 0;
}

Reference<XComponent> openWriter(Reference<XComponentContext> xContext)
{
    Reference<XComponent> xComp = NULL;
    try
    {
        // getting the remote LibreOffice service manager
        Reference<XMultiComponentFactory> xMCF = xContext->getServiceManager();

        Reference<XInterface> oDesktop
            = xMCF->createInstanceWithContext("com.sun.star.frame.Desktop", xContext);
        Reference<XComponentLoader> xCLoader(oDesktop, UNO_QUERY_THROW);
        Sequence<PropertyValue> szEmptyArgs(0);
        OUString strDoc("private:factory/swriter");
        xComp = xCLoader->loadComponentFromURL(strDoc, "_blank", 0, szEmptyArgs);
    }
    catch (Exception& e)
    {
        std::cout << "Error opening writer." << std::endl << e.Message << std::endl;
        exit(1);
    }

    return xComp;
}

void insertIntoCell(OUString cellName, OUString theText, Reference<XTextTable> xTextTable)
{
    Reference<XText> xCellText = NULL;
    Reference<XPropertySet> xCellProps = NULL;
    Reference<XTextRange> xCellTextRange = NULL;
    try
    {
        xCellText.set(xTextTable->getCellByName(cellName), UNO_QUERY_THROW);
        // create a cursor object
        Reference<XTextCursor> xCellTextCursor = xCellText->createTextCursor();
        xCellTextRange.set(xCellTextCursor, UNO_QUERY_THROW);
        xCellProps.set(xCellTextCursor, UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't get cell text, cursor properties, or text range." << std::endl
                  << e.Message << std::endl;
    }

    // change the color
    try
    {
        xCellProps->setPropertyValue("CharColor", Any(sal_Int32(16777215)));
    }
    catch (Exception& e)
    {
        std::cout << "Couldn't change cell text color." << std::endl << e.Message << std::endl;
        exit(1);
    }

    xCellText->insertString(xCellTextRange, theText, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
