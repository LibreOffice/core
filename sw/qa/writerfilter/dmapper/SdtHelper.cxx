/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/SdtHelper.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSdtRunRichText)
{
    // Given a document with a rich text inline/run SDT:
    loadFromFile(u"sdt-run-rich-text.docx");

    // Then make sure that formatting of the text inside the SDT is not lost:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : TextField
    // i.e. the SDT was imported as a text field, and the whole SDT had 12pt font size.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aTextPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    float fCharheight{};
    xContent->getPropertyValue(u"CharHeight"_ustr) >>= fCharheight;
    CPPUNIT_ASSERT_EQUAL(12.f, fCharheight);
    xContent.set(xContentEnum->nextElement(), uno::UNO_QUERY);
    xContent->getPropertyValue(u"CharHeight"_ustr) >>= fCharheight;
    CPPUNIT_ASSERT_EQUAL(24.f, fCharheight);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    OUString aAlias;
    xContentControlProps->getPropertyValue(u"Alias"_ustr) >>= aAlias;
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(u"myalias"_ustr, aAlias);
    OUString aTag;
    xContentControlProps->getPropertyValue(u"Tag"_ustr) >>= aTag;
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(u"mytag"_ustr, aTag);
    sal_Int32 nId = 0;
    xContentControlProps->getPropertyValue(u"Id"_ustr) >>= nId;
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2147483647), nId);
    sal_uInt32 nTabIndex = 0;
    xContentControlProps->getPropertyValue(u"TabIndex"_ustr) >>= nTabIndex;
    // This was 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(4294967295), nTabIndex);
    OUString aLock;
    xContentControlProps->getPropertyValue(u"Lock"_ustr) >>= aLock;
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(u"contentLocked"_ustr, aLock);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunPlainText)
{
    // Given a document with a plain text inline/run SDT:
    loadFromFile(u"sdt-run-plain-text.docx");

    // Then make sure that the text inside the SDT is not rich:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : TextField
    // i.e. the SDT was imported as a text field, not as a content control.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aTextPortionType);
    uno::Reference<beans::XPropertySet> xContentControl;
    xPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    bool bPlainText{};
    xContentControl->getPropertyValue(u"PlainText"_ustr) >>= bPlainText;
    CPPUNIT_ASSERT(bPlainText);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunCheckbox)
{
    // Given a document with a checkbox inline/run SDT:
    loadFromFile(u"sdt-run-checkbox.docx");

    // Then make sure that the doc model has a clickable checkbox content control:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the SDT was imported as plain text, making it hard to fill in checkboxes.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aTextPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bCheckbox{};
    xContentControlProps->getPropertyValue(u"Checkbox"_ustr) >>= bCheckbox;
    CPPUNIT_ASSERT(bCheckbox);
    bool bChecked{};
    xContentControlProps->getPropertyValue(u"Checked"_ustr) >>= bChecked;
    CPPUNIT_ASSERT(bChecked);
    OUString aCheckedState;
    xContentControlProps->getPropertyValue(u"CheckedState"_ustr) >>= aCheckedState;
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, aCheckedState);
    OUString aUncheckedState;
    xContentControlProps->getPropertyValue(u"UncheckedState"_ustr) >>= aUncheckedState;
    CPPUNIT_ASSERT_EQUAL(u"☐"_ustr, aUncheckedState);
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, xContent->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunDropdown)
{
    // Given a document with a dropdown inline/run SDT:
    loadFromFile(u"sdt-run-dropdown.docx");

    // Then make sure that the doc model has a clickable dropdown content control:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    // Without the accompanying fix in place, this failed with:
    // - Expected: ContentControl
    // - Actual  : TextField
    // i.e. the SDT was imported as a dropdown field, which does not support display-text + value
    // pairs.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValues> aListItems;
    xContentControlProps->getPropertyValue(u"ListItems"_ustr) >>= aListItems;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aListItems.getLength());
    comphelper::SequenceAsHashMap aMap0(aListItems[0]);
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, aMap0[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"R"_ustr, aMap0[u"Value"_ustr].get<OUString>());
    comphelper::SequenceAsHashMap aMap1(aListItems[1]);
    CPPUNIT_ASSERT_EQUAL(u"green"_ustr, aMap1[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"G"_ustr, aMap1[u"Value"_ustr].get<OUString>());
    comphelper::SequenceAsHashMap aMap2(aListItems[2]);
    CPPUNIT_ASSERT_EQUAL(u"blue"_ustr, aMap2[u"DisplayText"_ustr].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, aMap2[u"Value"_ustr].get<OUString>());
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"choose a color"_ustr, xContent->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunComboBox)
{
    // Given a document with a combo box inline/run SDT:
    loadFromFile(u"sdt-run-combobox.docx");

    // Then make sure that the doc model has a clickable combo box content control:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bComboBox{};
    xContentControlProps->getPropertyValue(u"ComboBox"_ustr) >>= bComboBox;
    // Without the accompanying fix in place, this failed as the content control was a drop-down,
    // not a combo box.
    CPPUNIT_ASSERT(bComboBox);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunPicture)
{
    // Given a document with a dropdown inline/run SDT:
    loadFromFile(u"sdt-run-picture.docx");

    // Then make sure that the doc model has a clickable picture content control:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    // Without the accompanying fix in place, this failed with:
    // - Expected: ContentControl
    // - Actual  : Frame
    // i.e. the SDT was imported as a plain image, not as a clickable placeholder in a content
    // control.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bPicture{};
    xContentControlProps->getPropertyValue(u"Picture"_ustr) >>= bPicture;
    CPPUNIT_ASSERT(bPicture);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
