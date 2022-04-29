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
#include <com/sun/star/text/XTextDocument.hpp>

using namespace com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/SdtHelper.cxx.
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

CPPUNIT_TEST_FIXTURE(Test, testSdtRunRichText)
{
    // Given a document with a rich text inline/run SDT:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "sdt-run-rich-text.docx";

    // When loading the document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that formatting of the text inside the SDT is not lost:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue("TextPortionType") >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : TextField
    // i.e. the SDT was imported as a text field, and the whole SDT had 12pt font size.
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aTextPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    float fCharheight{};
    xContent->getPropertyValue("CharHeight") >>= fCharheight;
    CPPUNIT_ASSERT_EQUAL(12.f, fCharheight);
    xContent.set(xContentEnum->nextElement(), uno::UNO_QUERY);
    xContent->getPropertyValue("CharHeight") >>= fCharheight;
    CPPUNIT_ASSERT_EQUAL(24.f, fCharheight);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunCheckbox)
{
    // Given a document with a checkbox inline/run SDT:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "sdt-run-checkbox.docx";

    // When loading the document:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure that the doc model has a clickable checkbox content control:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue("TextPortionType") >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the SDT was imported as plain text, making it hard to fill in checkboxes.
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aTextPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bCheckbox{};
    xContentControlProps->getPropertyValue("Checkbox") >>= bCheckbox;
    CPPUNIT_ASSERT(bCheckbox);
    bool bChecked{};
    xContentControlProps->getPropertyValue("Checked") >>= bChecked;
    CPPUNIT_ASSERT(bChecked);
    OUString aCheckedState;
    xContentControlProps->getPropertyValue("CheckedState") >>= aCheckedState;
    CPPUNIT_ASSERT_EQUAL(OUString(u"☒"), aCheckedState);
    OUString aUncheckedState;
    xContentControlProps->getPropertyValue("UncheckedState") >>= aUncheckedState;
    CPPUNIT_ASSERT_EQUAL(OUString(u"☐"), aUncheckedState);
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u"☒"), xContent->getString());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
