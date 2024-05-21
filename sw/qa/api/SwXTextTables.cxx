/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xelementaccess.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextTables.
 */
class SwXTextTables final : public UnoApiTest,
                            public apitest::XElementAccess,
                            public apitest::XIndexAccess,
                            public apitest::XNameAccess
{
public:
    SwXTextTables()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XTextTable>::get())
        , XIndexAccess(1)
        , XNameAccess(u"Table1"_ustr)
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

        try
        {
            Reference<text::XTextTable> xTable(
                xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr), UNO_QUERY_THROW);
            xTable->initialize(4, 3);
            xText->insertTextContent(xCursor, xTable, false);

            Reference<text::XTextTable> xTable2(
                xMSF->createInstance(u"com.sun.star.text.TextTable"_ustr), UNO_QUERY_THROW);
            xTable->initialize(4, 3);
            xText->insertTextContent(xCursor, xTable2, false);
        }
        catch (Exception&)
        {
        }

        Reference<text::XTextTablesSupplier> xSupplier(xTextDocument, UNO_QUERY_THROW);

        return Reference<XInterface>(xSupplier->getTextTables(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXTextTables);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextTables);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
