/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfdispatchvalue.cxx.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/writerfilter/qa/cppunittests/rtftok/data/";

CPPUNIT_TEST_FIXTURE(Test, testFollowStyle)
{
    // Given a file with \snext:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "follow-style.rtf";

    // When loading that file:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure we set the follow of the para style correctly:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(getComponent(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName("ParagraphStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Heading 1"),
                                               uno::UNO_QUERY);
    OUString aFollowStyle;
    xStyle->getPropertyValue("FollowStyle") >>= aFollowStyle;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Standard
    // - Actual  : Heading 1
    // i.e. \snext was ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), aFollowStyle);
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorder)
{
    // Given a document with a top margin and a border which has more spacing than the margin:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "negative-page-border.rtf";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the border distance is negative, so it can appear at the correct
    // position:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(getComponent(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("Standard"), uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue("TopMargin").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue("TopBorder").get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue("TopBorderDistance").get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -646
    // - Actual  : 342
    // i.e. the border negative distance was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
