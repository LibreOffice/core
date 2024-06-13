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
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFollowStyle)
{
    // Given a file with \snext:
    loadFromFile(u"follow-style.rtf");

    // Then make sure we set the follow of the para style correctly:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"ParagraphStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Heading 1"_ustr),
                                               uno::UNO_QUERY);
    OUString aFollowStyle;
    xStyle->getPropertyValue(u"FollowStyle"_ustr) >>= aFollowStyle;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Standard
    // - Actual  : Heading 1
    // i.e. \snext was ignored.
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, aFollowStyle);
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorder)
{
    // Given a document with a top margin and a border which has more spacing than the margin:
    loadFromFile(u"negative-page-border.rtf");

    // Then make sure that the border distance is negative, so it can appear at the correct
    // position:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue(u"TopBorder"_ustr).get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue(u"TopBorderDistance"_ustr).get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -646
    // - Actual  : 342
    // i.e. the border negative distance was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
