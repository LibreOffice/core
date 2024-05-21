/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>

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
 * Initial tests for SwXReferenceMarks.
 */
class SwXReferenceMarks final : public UnoApiTest,
                                public apitest::XElementAccess,
                                public apitest::XIndexAccess,
                                public apitest::XNameAccess
{
public:
    SwXReferenceMarks()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XTextContent>::get())
        , XIndexAccess(2)
        , XNameAccess(u"SwXReferenceMarks2"_ustr)
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
        Reference<lang::XMultiServiceFactory> xMSF(xTextDocument, UNO_QUERY_THROW);

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        //RefMark 1
        Reference<XInterface> xRefMark1
            = xMSF->createInstance(u"com.sun.star.text.ReferenceMark"_ustr);
        Reference<container::XNamed> xNamed1(xRefMark1, UNO_QUERY_THROW);
        xNamed1->setName(u"SwXReferenceMarks1"_ustr);

        Reference<text::XTextContent> xTextContent1(xRefMark1, UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xTextContent1, false);

        //RefMark 2
        Reference<XInterface> xRefMark2
            = xMSF->createInstance(u"com.sun.star.text.ReferenceMark"_ustr);
        Reference<container::XNamed> xNamed2(xRefMark2, UNO_QUERY_THROW);
        xNamed2->setName(u"SwXReferenceMarks2"_ustr);

        Reference<text::XTextContent> xTextContent2(xRefMark2, UNO_QUERY_THROW);
        xText->insertTextContent(xCursor, xTextContent2, false);

        // Getting ReferenceMarks from Text Document
        Reference<text::XReferenceMarksSupplier> xRefMarkSupp(xTextDocument, UNO_QUERY_THROW);

        return Reference<XInterface>(xRefMarkSupp->getReferenceMarks(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXReferenceMarks);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXReferenceMarks);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
