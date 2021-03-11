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
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/UnitConversion.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/DomainMapper.cxx.
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

constexpr OUStringLiteral DATA_DIRECTORY = u"/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, testLargeParaTopMargin)
{
    // Given a document with a paragraph with a large "before" spacing.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "large-para-top-margin.docx";
    getComponent() = loadFromDesktop(aURL);

    // When checking the first paragraph.
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    // Then assert its top margin.
    sal_Int32 nParaTopMargin{};
    xPara->getPropertyValue("ParaTopMargin") >>= nParaTopMargin;
    // <w:spacing w:before="37050"/> in the document.
    sal_Int32 nExpected = convertTwipToMm100(37050);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65352
    // - Actual  : 0
    // i.e. the paragraph margin was lost, which shifted the paragraph to the right (no top margin
    // -> wrap around a TextBox), which shifted the triangle shape out of the page frame.
    CPPUNIT_ASSERT_EQUAL(nExpected, nParaTopMargin);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
