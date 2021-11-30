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
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/styles/ code.
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

constexpr OUStringLiteral DATA_DIRECTORY = u"/svx/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testThemeChange)
{
    // Given a document, with a first slide and blue shape text from theme:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "theme.pptx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xDrawPageShapes(xDrawPage, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPageShapes->getByIndex(0), uno::UNO_QUERY);
    {
        uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xPara(
            xText->createEnumeration()->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                     uno::UNO_QUERY);
        sal_Int32 nColor{};
        xPortion->getPropertyValue("CharColor") >>= nColor;
        // Blue.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x4472c4), nColor);
    }

    // When changing the master slide of slide 1 to use the theme of the second master slide:
    uno::Reference<drawing::XMasterPageTarget> xDrawPage2(
        xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage2(xDrawPage2->getMasterPage(), uno::UNO_QUERY);
    uno::Any aTheme = xMasterPage2->getPropertyValue("Theme");
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    xMasterPage->setPropertyValue("Theme", aTheme);

    // Then make sure the shape text color is now green:
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    sal_Int32 nColor{};
    xPortion->getPropertyValue("CharColor") >>= nColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9486886 (#90c226, green)
    // - Actual  : 4485828 (#4472c4, blue)
    // i.e. shape text was not updated on theme change.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90c226), nColor);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
