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
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXStyleFamilies.
 */
class SwXStyleFamilies final : public UnoApiTest,
                               public apitest::XElementAccess,
                               public apitest::XIndexAccess,
                               public apitest::XNameAccess

{
public:
    SwXStyleFamilies()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<container::XNameContainer>::get())
        , XIndexAccess(7)
        , XNameAccess(u"CharacterStyles"_ustr)
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

        Reference<style::XStyleFamiliesSupplier> xStyleFamSupp(xTextDocument, UNO_QUERY_THROW);
        Reference<container::XNameAccess> xSF = xStyleFamSupp->getStyleFamilies();

        return Reference<XInterface>(xSF, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXStyleFamilies);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXStyleFamilies);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
