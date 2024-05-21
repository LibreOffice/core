/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/text/numberingrules.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXChapterNumbering.
 */
class SwXChapterNumbering final : public UnoApiTest,
                                  public apitest::XPropertySet,
                                  public apitest::XElementAccess,
                                  public apitest::XIndexAccess,
                                  public apitest::NumberingRules
{
public:
    SwXChapterNumbering()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<Sequence<beans::PropertyValue>>::get())
        , XIndexAccess(10)
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

        Reference<text::XChapterNumberingSupplier> xCNSupplier(xTextDocument, UNO_QUERY_THROW);
        Reference<container::XIndexAccess> xCNRules(xCNSupplier->getChapterNumberingRules(),
                                                    UNO_QUERY_THROW);

        try
        {
            Reference<container::XIndexReplace> xIndexReplace(xCNRules->getByIndex(1),
                                                              UNO_QUERY_THROW);
        }
        catch (Exception&)
        {
        }
        return Reference<XInterface>(xCNRules, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXChapterNumbering);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);
    CPPUNIT_TEST(testNumberingRulesProperties);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXChapterNumbering);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
