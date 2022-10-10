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
#include <com/sun/star/beans/PropertyValues.hpp>

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

CPPUNIT_TEST_FIXTURE(Test, testSdtRunInPara)
{
    // Given a document with a block SDT, and inside that some content + a run SDT:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "sdt-run-in-para.docx";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure the content inside the block SDT but outside the run SDT is not lost:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: first-second
    // - Actual  : second
    // i.e. the block-SDT-only string was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("first-second"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtDropdownNoDisplayText)
{
    // Given a document with <w:listItem w:value="..."/> (no display text):
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "sdt-dropdown-no-display-text.docx";

    // When loading that document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure we create a dropdown content control, not a rich text one:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValues> aListItems;
    xContentControlProps->getPropertyValue("ListItems") >>= aListItems;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the list item was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aListItems.getLength());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
