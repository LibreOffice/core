/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfdispatchflag.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloatingTable)
{
    // Given a document with a floating table, when importing that document:
    loadFromURL(u"floating-table.rtf");

    // Then make sure the floating table is there & has the expected properties:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.lang.IndexOutOfBoundsException
    // i.e. the table was not floating / was not in a fly frame.
    uno::Reference<beans::XPropertySet> xFrame(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bIsSplitAllowed{};
    xFrame->getPropertyValue("IsSplitAllowed") >>= bIsSplitAllowed;
    CPPUNIT_ASSERT(bIsSplitAllowed);
    sal_Int16 nVertOrientRelation{};
    xFrame->getPropertyValue("VertOrientRelation") >>= nVertOrientRelation;
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, nVertOrientRelation);
    sal_Int16 nHoriOrientRelation{};
    xFrame->getPropertyValue("HoriOrientRelation") >>= nHoriOrientRelation;
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, nHoriOrientRelation);
    sal_Int32 nVertOrientPosition{};
    xFrame->getPropertyValue("VertOrientPosition") >>= nVertOrientPosition;
    sal_Int32 nExpected = o3tl::convert(10, o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(nExpected, nVertOrientPosition);
    sal_Int32 nHoriOrientPosition{};
    xFrame->getPropertyValue("HoriOrientPosition") >>= nHoriOrientPosition;
    nExpected = o3tl::convert(20, o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(nExpected, nHoriOrientPosition);
    sal_Int32 nLeftMargin{};
    xFrame->getPropertyValue("LeftMargin") >>= nLeftMargin;
    nExpected = o3tl::convert(30, o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(nExpected, nLeftMargin);
    sal_Int32 nRightMargin{};
    xFrame->getPropertyValue("RightMargin") >>= nRightMargin;
    nExpected = o3tl::convert(40, o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(nExpected, nRightMargin);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
