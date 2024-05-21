/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/text/xtextrange.hxx>
#include <test/text/xtext.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTableCellText.
 */
class SwXTableCellText final : public UnoApiTest,
                               public apitest::XEnumerationAccess,
                               public apitest::XElementAccess,
                               public apitest::XSimpleText,
                               public apitest::XTextRange,
                               public apitest::XText
{
public:
    SwXTableCellText()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XTextRange>::get())
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
        mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    Reference<XInterface> init() override
    {
        Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);
        Reference<lang::XMultiServiceFactory> xMSF(mxComponent, UNO_QUERY_THROW);

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        Reference<text::XTextTable> xTable(
            xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr), UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xTable, false);

        Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
        Reference<text::XSimpleText> xCellText(xCell, UNO_QUERY_THROW);
        xCellText->setString(u"SwXTableCellText"_ustr);

        mxTextContent = Reference<text::XTextContent>(
            xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr), UNO_QUERY_THROW);

        return Reference<XInterface>(xCellText->getText(), UNO_QUERY_THROW);
    }

    Reference<text::XTextContent> getTextContent() override { return mxTextContent; };

    CPPUNIT_TEST_SUITE(SwXTableCellText);
    CPPUNIT_TEST(testCreateEnumeration);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);
    CPPUNIT_TEST(testGetEnd);
    CPPUNIT_TEST(testGetSetString);
    CPPUNIT_TEST(testGetStart);
    CPPUNIT_TEST(testGetText);
    CPPUNIT_TEST(testInsertRemoveTextContent);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextContent> mxTextContent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTableCellText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
