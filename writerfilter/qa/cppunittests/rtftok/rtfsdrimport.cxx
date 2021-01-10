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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfsdrimport.cxx.
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

CPPUNIT_TEST_FIXTURE(Test, testPictureInTextframe)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "picture-in-textframe.rtf";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xTextDocument->getDrawPage();
    uno::Reference<beans::XPropertySet> xInnerShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    text::TextContentAnchorType eAnchorType = text::TextContentAnchorType_AT_PARAGRAPH;
    xInnerShape->getPropertyValue("AnchorType") >>= eAnchorType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 4
    // i.e. the properties of the inner shape (including its anchor type and bitmap fill) were lost
    // on import.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, eAnchorType);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
