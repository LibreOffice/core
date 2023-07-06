/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XReferenceMarksSupplier.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXReferenceMark.
 */
class SwXReferenceMark final : public test::BootstrapFixture,
                               public unotest::MacrosTest,
                               public apitest::XElementAccess,
                               public apitest::XIndexAccess,
                               public apitest::XNameAccess
{
public:
    SwXReferenceMark();
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;

    CPPUNIT_TEST_SUITE(SwXReferenceMark);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<text::XTextDocument> mxTextDocument;
};

SwXReferenceMark::SwXReferenceMark()
    : XElementAccess(cppu::UnoType<text::XTextContent>::get())
    , XIndexAccess(2)
    , XNameAccess("SwXReferenceMark2")
{
}

void SwXReferenceMark::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxTextDocument = Reference<text::XTextDocument>(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"),
        uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mxTextDocument.is());
}

void SwXReferenceMark::tearDown()
{
    if (mxTextDocument.is())
        mxTextDocument->dispose();

    test::BootstrapFixture::tearDown();
}

Reference<XInterface> SwXReferenceMark::init()
{
    Reference<lang::XMultiServiceFactory> xMSF(mxTextDocument, UNO_QUERY_THROW);

    Reference<text::XText> xText = mxTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    //RefMark 1
    Reference<XInterface> xRefMark1 = xMSF->createInstance("com.sun.star.text.ReferenceMark");
    Reference<container::XNamed> xNamed1(xRefMark1, UNO_QUERY_THROW);
    xNamed1->setName("SwXReferenceMark1");

    Reference<text::XTextContent> xTextContent1(xRefMark1, UNO_QUERY_THROW);
    xText->insertTextContent(xCursor, xTextContent1, false);

    //RefMark 2
    Reference<XInterface> xRefMark2 = xMSF->createInstance("com.sun.star.text.ReferenceMark");
    Reference<container::XNamed> xNamed2(xRefMark2, UNO_QUERY_THROW);
    xNamed2->setName("SwXReferenceMark2");

    Reference<text::XTextContent> xTextContent2(xRefMark2, UNO_QUERY_THROW);
    xText->insertTextContent(xCursor, xTextContent2, false);

    // Getting ReferenceMarks from Text Document
    Reference<text::XReferenceMarksSupplier> xRefMarkSupp(mxTextDocument, UNO_QUERY_THROW);

    return Reference<XInterface>(xRefMarkSupp->getReferenceMarks(), UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXReferenceMark);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
