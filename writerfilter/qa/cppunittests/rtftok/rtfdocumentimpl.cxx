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
#include <com/sun/star/graphic/XGraphic.hpp>

#include <vcl/graph.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfdocumentimpl.cxx.
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

CPPUNIT_TEST_FIXTURE(Test, testPicwPich)
{
    // Given a document with a WMF file where picwgoal and picscalex is provided, so picw is not
    // relevant:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "picw-pich.rtf";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure the graphic's preferred size is correct:
    uno::Reference<drawing::XDrawPageSupplier> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xTextDocument->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("Graphic") >>= xGraphic;
    Graphic aGraphic(xGraphic);
    Size aPrefSize = aGraphic.GetPrefSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2619
    // - Actual  : 132
    // i.e. the graphic width didn't match 2.62 cm from the Word UI.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(2619), aPrefSize.Width());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
