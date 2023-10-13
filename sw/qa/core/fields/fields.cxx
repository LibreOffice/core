/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <comphelper/propertyvalue.hxx>

#include <authfld.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>

namespace
{
/// Covers sw/source/core/fields/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/fields/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTooltip)
{
    // Create a document with a bibliography reference in it.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("Identifier", OUString("ARJ00")),
        comphelper::makePropertyValue("Author", OUString("Ar, J")),
        comphelper::makePropertyValue("Title", OUString("mytitle")),
        comphelper::makePropertyValue("Year", OUString("2020")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Get the tooltip of the field.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwPaM* pCursor = pWrtShell->GetCursor();
    auto pField = dynamic_cast<SwAuthorityField*>(
        SwCursorShell::GetFieldAtCursor(pCursor, /*bIncludeInputFieldAtStart=*/true));
    CPPUNIT_ASSERT(pField);
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    const SwTextAttr* pTextAttr = pTextNode->GetSwpHints().Get(0);
    const SwRootFrame* pLayout = pWrtShell->GetLayout();
    OUString aTooltip = pField->GetAuthority(pTextAttr, pLayout);

    // Without the accompanying fix in place, generating this tooltip text was not possible without
    // first inserting an empty bibliography table into the document.
    CPPUNIT_ASSERT_EQUAL(OUString("ARJ00: Ar, J, mytitle, 2020"), aTooltip);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143424)
{
    createSwDoc("tdf143424.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // TODO: I have no idea why fields are enumerated in invalid order, not like in document

    // Field: Chapter Format: Chapter name
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Another title"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number and name
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2 - Another title"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2 -"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number without separator
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(Test, testChapterFieldsFollowedBy)
{
    createSwDoc("chapter_field_followedby.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // TODO: I have no idea why fields are enumerated in invalid order, not like in document
    std::vector<OUString> aFieldValues = {
        "Followed by tab", // #1
        "I.I.I.I", // #16
        ">I.I.I.I< Followed by newline", // #15 Linefeed is replaced by space
        ">I.I.I.I<", // #14
        "Followed by newline", // #13
        "I.I.I", // #12
        ">I.I.I<Followed by nothing", // #11 Nothing between text & outline
        ">I.I.I<", // #10
        "Followed by nothing", // #9
        "I.I", // #8
        ">I.I< Followed by space", // #7 Space as is
        ">I.I<", // #6
        "Followed by space", // #5
        "I", // #4
        ">I< Followed by tab", // #3 Here is a tab, but replaced by space in field
        ">I<", // #2
    };

    for (const auto& sValue : aFieldValues)
    {
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sValue, xField->getPresentation(false));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86790)
{
    loadFromURL(u"tdf86790.docx");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // Similarly to testChapterFieldsFollowedBy, the fields are enumerated with #1 first and everything else in reverse order
    std::vector<std::pair<OUString, OUString>> aFieldValues = {
        { " Heading 1", "1" }, // #1
        { " foobar", "22.2" }, // #5
        { " foobar", "4" }, // #4
        { " Heading 2", "1.1" }, // #3
        { " Heading 2", "1.1" }, // #2
    };

    for (const auto& sValue : aFieldValues)
    {
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sValue.first, xField->getPresentation(true));
        CPPUNIT_ASSERT_EQUAL(sValue.second, xField->getPresentation(false));
    }
}

/// If there is referenced text both above and below, STYLEREF searches up
CPPUNIT_TEST_FIXTURE(Test, testStyleRefSearchUp)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorPropertySet(xCursor, uno::UNO_QUERY);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading far above field", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading above field", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.GetReference"), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xFieldPropertySet(xField, uno::UNO_QUERY);
    xFieldPropertySet->setPropertyValue("ReferenceFieldSource",
                                        uno::Any(sal_Int16(text::ReferenceFieldSource::STYLE)));
    xFieldPropertySet->setPropertyValue("ReferenceFieldPart",
                                        uno::Any(sal_Int16(text::ReferenceFieldPart::TEXT)));
    xFieldPropertySet->setPropertyValue("SourceName", uno::Any(OUString("Heading 1")));

    uno::Reference<text::XTextCursor> xFieldCursor = xText->createTextCursor();

    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading below field", false);

    xField->attach(xFieldCursor);
    CPPUNIT_ASSERT_EQUAL(OUString("Heading above field"), xField->getPresentation(false));
}

/// If there is referenced text both above and below, STYLEREF searches down
CPPUNIT_TEST_FIXTURE(Test, testStyleRefSearchDown)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorPropertySet(xCursor, uno::UNO_QUERY);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.GetReference"), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xFieldPropertySet(xField, uno::UNO_QUERY);
    xFieldPropertySet->setPropertyValue("ReferenceFieldSource",
                                        uno::Any(sal_Int16(text::ReferenceFieldSource::STYLE)));
    xFieldPropertySet->setPropertyValue("ReferenceFieldPart",
                                        uno::Any(sal_Int16(text::ReferenceFieldPart::TEXT)));
    xFieldPropertySet->setPropertyValue("SourceName", uno::Any(OUString("Heading 1")));

    uno::Reference<text::XTextCursor> xFieldCursor = xText->createTextCursor();

    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading below field", false);

    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading far below field", false);

    xField->attach(xFieldCursor);
    CPPUNIT_ASSERT_EQUAL(OUString("Heading below field"), xField->getPresentation(false));
}

/// STYLEREFs in marginals (headers or footers) should search in the page they are on first, regardless if there is anything above them
CPPUNIT_TEST_FIXTURE(Test, testMarginalStyleRef)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorPropertySet(xCursor, uno::UNO_QUERY);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Top heading on page", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Bottom heading on page", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.GetReference"), uno::UNO_QUERY);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xParagraphStylesContainer(
        xStyleFamiliesSupplier->getStyleFamilies()->getByName("PageStyles"), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xPagePropertySet(
        xParagraphStylesContainer->getByName("Standard"), uno::UNO_QUERY);

    xPagePropertySet->setPropertyValue("FooterIsOn", uno::Any(true));
    uno::Reference<text::XText> xFooterText(xPagePropertySet->getPropertyValue("FooterText"),
                                            uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xFieldPropertySet(xField, uno::UNO_QUERY);
    xFieldPropertySet->setPropertyValue("ReferenceFieldSource",
                                        uno::Any(sal_Int16(text::ReferenceFieldSource::STYLE)));
    xFieldPropertySet->setPropertyValue("ReferenceFieldPart",
                                        uno::Any(sal_Int16(text::ReferenceFieldPart::TEXT)));
    xFieldPropertySet->setPropertyValue("SourceName", uno::Any(OUString("Heading 1")));

    uno::Reference<text::XTextRange> xFooterCursor = xFooterText->createTextCursor();
    xField->attach(xFooterCursor);

    CPPUNIT_ASSERT_EQUAL(OUString("Top heading on page"), xField->getPresentation(false));
}

/// STYLEREFs in footnotes should search from the point of the reference mark
CPPUNIT_TEST_FIXTURE(Test, testFootnoteStyleRef)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();

    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorPropertySet(xCursor, uno::UNO_QUERY);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading far above reference mark", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading above reference mark", false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

    uno::Reference<text::XFootnote> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    xFootnote->setLabel("Style reference mark");
    xText->insertTextContent(xCursor, xFootnote, false);

    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    xCursorPropertySet->setPropertyValue("ParaStyleName", uno::Any(OUString("Heading 1")));
    xText->insertString(xCursor, "Heading below reference mark", false);

    uno::Reference<text::XTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.GetReference"), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xFieldPropertySet(xField, uno::UNO_QUERY);
    xFieldPropertySet->setPropertyValue("ReferenceFieldSource",
                                        uno::Any(sal_Int16(text::ReferenceFieldSource::STYLE)));
    xFieldPropertySet->setPropertyValue("ReferenceFieldPart",
                                        uno::Any(sal_Int16(text::ReferenceFieldPart::TEXT)));
    xFieldPropertySet->setPropertyValue("SourceName", uno::Any(OUString("Heading 1")));

    uno::Reference<text::XSimpleText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFootnoteCursor = xFootnoteText->createTextCursor();
    xField->attach(xFootnoteCursor);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading above reference mark"), xField->getPresentation(false));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
