/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/errinf.hxx>

#include <wrtsh.hxx>
#include <unotextrange.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <textlinebreak.hxx>
#include <textcontentcontrol.hxx>

using namespace ::com::sun::star;

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/unocore/data/";
}

/// Covers sw/source/core/unocore/ fixes.
class SwCoreUnocoreTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf119081)
{
    // Load a doc with a nested table in it.
    load(DATA_DIRECTORY, "tdf119081.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Enter outer A1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/3);
    // Enter inner A1.
    pWrtShell->Right(CRSR_SKIP_CELLS, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false,
                     /*bVisual=*/true);
    // Enter outer B1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);

    SwDoc* pDoc = pDocShell->GetDoc();
    SwPaM& rCursor = pWrtShell->GetCurrentShellCursor();
    uno::Reference<text::XTextRange> xInsertPosition
        = SwXTextRange::CreateXTextRange(*pDoc, *rCursor.GetPoint(), nullptr);
    uno::Reference<text::XTextAppend> xTextAppend(xInsertPosition->getText(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.uno.RuntimeException
    xTextAppend->insertTextPortion("x", {}, xInsertPosition);

    // Verify that the string is indeed inserted.
    pWrtShell->Left(CRSR_SKIP_CELLS, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false,
                    /*bVisual=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pWrtShell->GetCurrentShellCursor().GetText());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, flyAtParaAnchor)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<lang::XMultiServiceFactory> const xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextDocument> const xTD(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextFrame> const xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xFrameProps(xTextFrame, uno::UNO_QUERY_THROW);
    xFrameProps->setPropertyValue("AnchorType", uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    auto const xText = xTD->getText();
    auto const xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextFrame, false);
    auto const xAnchor = xTextFrame->getAnchor();
    uno::Reference<text::XTextContent> const xFieldmark(
        xMSF->createInstance("com.sun.star.text.Fieldmark"), uno::UNO_QUERY_THROW);
    // this crashed because the anchor didn't have SwIndex
    xText->insertTextContent(xAnchor, xFieldmark, false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testRtlGutter)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Unknown property: RtlGutter
    auto bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(!bRtlGutter);
    xPageStyle->setPropertyValue("RtlGutter", uno::Any(true));
    bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testBiblioLocalCopy)
{
    // Given an empty document:
    createSwDoc();

    // When setting the LocalURL of a biblio field:
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("ARJ00")),
        comphelper::makePropertyValue("Author", OUString("Me")),
        comphelper::makePropertyValue("Title", OUString("mytitle")),
        comphelper::makePropertyValue("Year", OUString("2020")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf")),
        comphelper::makePropertyValue("LocalURL", OUString("file:///home/me/test.pdf")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Then make sure we get that LocalURL back:
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue("Fields"));
    // Without the accompanying fix in place, this test would have failed, there was no LocalURL key
    // in the map.
    CPPUNIT_ASSERT(aMap.find("LocalURL") != aMap.end());
    auto aActual = aMap["LocalURL"].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("file:///home/me/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLinkedStyles)
{
    // Given an empty document:
    createSwDoc();

    // When defining a linked style for a para style:
    uno::Reference<container::XNameAccess> xParaStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xParaStyle(xParaStyles->getByName("Caption"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParaStyle, "LinkStyle"));
    xParaStyle->setPropertyValue("LinkStyle", uno::Any(OUString("Emphasis")));
    // Then make sure we get the linked char style back:
    CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xParaStyle, "LinkStyle"));

    // When defining a linked style for a char style:
    uno::Reference<container::XNameAccess> xCharStyles = getStyles("CharacterStyles");
    uno::Reference<beans::XPropertySet> xCharStyle(xCharStyles->getByName("Emphasis"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xCharStyle, "LinkStyle"));
    xCharStyle->setPropertyValue("LinkStyle", uno::Any(OUString("Caption")));
    // Then make sure we get the linked para style back:
    CPPUNIT_ASSERT_EQUAL(OUString("Caption"), getProperty<OUString>(xCharStyle, "LinkStyle"));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testViewCursorTextFrame)
{
    // Given a document with a graphic and holding a reference to that graphic frame:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aInsertArgs = { comphelper::makePropertyValue(
        "FileName", m_directories.getURLFromSrc(DATA_DIRECTORY) + "graphic.png") };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aInsertArgs);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xViewCursor(xTextViewCursorSupplier->getViewCursor(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame;
    xViewCursor->getPropertyValue("TextFrame") >>= xFrame;

    // When saving to ODT, then make sure the store doesn't fail:
    uno::Reference<frame::XStorable> xStorable(xModel, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreArgs
        = { comphelper::makePropertyValue("FilterName", OUString("writer8")) };
    // Without the accompanying fix in place, this test would have failed with:
    // uno.RuntimeException: "SwXParagraph: disposed or invalid ..."
    xStorable->storeToURL(maTempFile.GetURL(), aStoreArgs);
}

/// Fails the test if an error popup would be presented.
static void BasicDisplayErrorHandler(const OUString& /*rErr*/, const OUString& /*rAction*/)
{
    CPPUNIT_ASSERT(false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testBrokenEmbeddedObject)
{
    // Given a document with a broken embedded object (the XML markup is not well-formed):
    load(DATA_DIRECTORY, "broken-embedded-object.odt");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xObject(xObjects->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xEmbeddedObject;
    // Get the property first, which initializes Draw, which would overwrite our error handler.
    xObject->getPropertyValue("EmbeddedObject") >>= xEmbeddedObject;
    ErrorRegistry::RegisterDisplay(&BasicDisplayErrorHandler);

    // When trying to load that embedded object:
    xObject->getPropertyValue("EmbeddedObject") >>= xEmbeddedObject;

    // Then make sure we get a non-empty reference and an error popup it not shown:
    CPPUNIT_ASSERT(xEmbeddedObject.is());
    // Without the accompanying fix in place, we got this reference, but first an error popup was
    // shown to the user.
    CPPUNIT_ASSERT(
        xEmbeddedObject->supportsService("com.sun.star.comp.embed.OCommonEmbeddedObject"));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLineBreakInsert)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a line-break with properties:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xMSF->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // Then make sure that both the line break and its matching text attribute is inserted:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: "\n" (newline)
    // - Actual  : "" (empty string)
    // i.e. SwXLineBreak::attach() did not insert the newline + its text attribute.
    CPPUNIT_ASSERT_EQUAL(OUString("\n"), pTextNode->GetText());
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_LINEBREAK);
    CPPUNIT_ASSERT(pAttr);
    auto pTextLineBreak = static_cast<SwTextLineBreak*>(pAttr);
    auto& rFormatLineBreak = static_cast<SwFormatLineBreak&>(pTextLineBreak->GetAttr());
    CPPUNIT_ASSERT_EQUAL(SwLineBreakClear::ALL, rFormatLineBreak.GetValue());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLineBreakTextPortionEnum)
{
    // Given a document with a clearing break:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xMSF->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue("Clear", uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When enumerating the text portions of the only paragraph in the document:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);

    // Then make sure that the text portion type is correct + the clear type can be read:
    auto aPortionType = getProperty<OUString>(xTextPortion, "TextPortionType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: LineBreak
    // - Actual  : Text
    // i.e. a line break with properties was part of the normal Text portion, making it impossible
    // to get those properties.
    CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aPortionType);
    xLineBreak = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, "LineBreak");
    eClear = getProperty<sal_Int16>(xLineBreak, "Clear");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserFieldTooltip)
{
    // Given a document with a user field:
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::Any(OUString("a_user_field")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::Any(OUString("42")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xFieldProps(xField, uno::UNO_QUERY);

    // When setting a tooltip on the field:
    OUString aExpected("first line\nsecond line");
    xFieldProps->setPropertyValue("Title", uno::Any(aExpected));

    // Then make sure that the tooltip we read back matches the one previously specified:
    // Without the accompanying fix in place, this test would have failed with:
    // - the property is of unexpected type or void: Title
    // i.e. reading of the tooltip was broken.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(xFieldProps, "Title"));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlInsert)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a content control around one or more text portions:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    // Set a custom property on the content control:
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("ShowingPlaceHolder", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the text attribute is inserted:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    // Without the accompanying fix in place, this test would have failed, as the
    // SwXContentControl::attach() implementation was missing.
    CPPUNIT_ASSERT(pAttr);
    // Also verify that the custom property was set:
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetShowingPlaceHolder());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testImageTooltip)
{
    // Given a document with an image and a hyperlink on it:
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xImage(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xImage, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xImageProps(xImage, uno::UNO_QUERY);
    xImageProps->setPropertyValue("HyperLinkURL", uno::Any(OUString("http://www.example.com")));

    // When setting a tooltip on the image:
    OUString aExpected("first line\nsecond line");
    xImageProps->setPropertyValue("Tooltip", uno::Any(aExpected));

    // Then make sure that the tooltip we read back matches the one previously specified:
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // i.e. reading/writing of the tooltip was broken.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(xImageProps, "Tooltip"));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlTextPortionEnum)
{
    // Given a document with a content control around one or more text portions:
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When enumerating the text portions of the only paragraph in the document:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);

    // Then make sure that the text portion type is correct + the content can be read:
    auto aPortionType = getProperty<OUString>(xTextPortion, "TextPortionType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the content control text attribute was ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    xContentControl
        = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, "ContentControl");
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xContent->getString());

    // Also test the generated layout:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwFieldPortion", "expand", "");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: PortionType::ContentControl
    // - Actual  : PortionType::Text
    // i.e. SwContentControl generated a plain text portion, not a dedicated content control
    // portion.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwLinePortion", "type",
                "PortionType::ContentControl");
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwLinePortion", "portion", "test*");

    // Also test the doc model, make sure that there is a dummy character at the start and end, so
    // the user can explicitly decide if they want to expand the content control or not:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aText = pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ^Atest^A
    // - Actual  : ^Atest
    // i.e. there was no dummy character at the end.
    CPPUNIT_ASSERT_EQUAL(OUString("\x0001test\x0001"), aText);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlCheckbox)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a checkbox content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue("Checkbox", uno::Any(true));
    xContentControlProps->setPropertyValue("Checked", uno::Any(true));
    xContentControlProps->setPropertyValue("CheckedState", uno::Any(OUString(u"☒")));
    xContentControlProps->setPropertyValue("UncheckedState", uno::Any(OUString(u"☐")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetCheckbox());
    CPPUNIT_ASSERT(pContentControl->GetChecked());
    CPPUNIT_ASSERT_EQUAL(OUString(u"☒"), pContentControl->GetCheckedState());
    CPPUNIT_ASSERT_EQUAL(OUString(u"☐"), pContentControl->GetUncheckedState());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlDropdown)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a dropdown content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("red"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("R"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("green"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("G"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("blue"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("B"))),
            },
        };
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
        xContentControlProps->setPropertyValue("ListItems", uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    std::vector<SwContentControlListItem> aListItems = pContentControl->GetListItems();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aListItems.size());
    CPPUNIT_ASSERT_EQUAL(OUString("red"), aListItems[0].m_aDisplayText);
    CPPUNIT_ASSERT_EQUAL(OUString("R"), aListItems[0].m_aValue);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testInsertFileInContentControlException)
{
    // Given a document with a content control:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Reject inserting a document inside the content control:
    xCursor->goLeft(1, false);
    OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf119081.odt");
    uno::Reference<document::XDocumentInsertable> xInsertable(xCursor, uno::UNO_QUERY);
    CPPUNIT_ASSERT_THROW(xInsertable->insertDocumentFromURL(aURL, {}), uno::RuntimeException);

    // Accept inserting a document outside the content control:
    xCursor->goRight(1, false);
    xInsertable->insertDocumentFromURL(aURL, {});
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlPicture)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a picture content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue("Picture", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetPicture());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlDate)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a date content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue("Date", uno::Any(true));
    xContentControlProps->setPropertyValue("DateFormat", uno::Any(OUString("M/d/yyyy")));
    xContentControlProps->setPropertyValue("DateLanguage", uno::Any(OUString("en-US")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetDate());
    CPPUNIT_ASSERT_EQUAL(OUString("M/d/yyyy"), pContentControl->GetDateFormat());
    CPPUNIT_ASSERT_EQUAL(OUString("en-US"), pContentControl->GetDateLanguage());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
