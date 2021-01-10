/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/oox/qa/unit/data/";

/// oox vml tests.
class OoxVmlTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(std::u16string_view rURL);
};

void OoxVmlTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void OoxVmlTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void OoxVmlTest::load(std::u16string_view rFileName)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFileName;
    mxComponent = loadFromDesktop(aURL);
}

CPPUNIT_TEST_FIXTURE(OoxVmlTest, testSpt202ShapeType)
{
    // Load a document with a groupshape, 2nd child is a <v:shape>, its type has o:spt set to 202
    // (TextBox).
    load(u"group-spt202.docx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: com.sun.star.drawing.TextShape
    // - Actual  : com.sun.star.drawing.CustomShape
    // and then the size of the group shape was incorrect, e.g. its right edge was outside the page
    // boundaries.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.TextShape"), xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(OoxVmlTest, testShapeNonAutosizeWithText)
{
    // Load a document which has a group shape, containing a single child.
    // 17.78 cm is the full group shape width, 19431/64008 is the child shape's relative width inside
    // that, so 5.3975 cm should be the shape width.
    load(u"shape-non-autosize-with-text.docx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Actual  : 1115
    // - Expected: 5398
    // because the width was determined using its text size, not using the explicit size.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5398), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(OoxVmlTest, testGraphicStroke)
{
    load(u"graphic-stroke.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xShape;
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xInfo(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        if (!xInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
        {
            continue;
        }

        xShape.set(xInfo, uno::UNO_QUERY);
        break;
    }
    CPPUNIT_ASSERT(xShape.is());

    drawing::LineStyle eLineStyle{};
    xShape->getPropertyValue("LineStyle") >>= eLineStyle;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. line style was NONE, not SOLID.
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, eLineStyle);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
