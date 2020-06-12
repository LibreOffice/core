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
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/GraphicImport.cxx.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, testGroupShapeRotation)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "group-shape-rotation.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nVertPosition = 0;
    xShape->getPropertyValue("VertOrientPosition") >>= nVertPosition;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1221
    // - Actual  : -2048
    // i.e. the group shape had a so low vertical position that the line shape did not point into
    // it.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1221), nVertPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testRelfromhInsidemargin)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "relfromh-insidemargin.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nRelation = 0;
    xShape->getPropertyValue("HoriOrientRelation") >>= nRelation;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7 (PAGE_FRAME)
    // - Actual  : 0 (FRAME)
    // i.e. the horizontal position was relative to the paragraph area, not to the entire page.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nRelation);
    bool bPageToggle = false;
    xShape->getPropertyValue("PageToggle") >>= bPageToggle;
    CPPUNIT_ASSERT(bPageToggle);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawShapeInlineEffect)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "draw-shape-inline-effect.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nBottomMargin = 0;
    xShape->getPropertyValue("BottomMargin") >>= nBottomMargin;
    // 273 in mm100 is 98425 EMUs from the file.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 273
    // - Actual  : 0
    // i.e. the layout result had less pages than expected (compared to Word).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(273), nBottomMargin);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
