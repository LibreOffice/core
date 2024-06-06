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
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/errinf.hxx>
#include <editeng/wghtitem.hxx>

#include <wrtsh.hxx>
#include <unotextrange.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <textlinebreak.hxx>
#include <textcontentcontrol.hxx>
#include <frmmgr.hxx>
#include <fmtcntnt.hxx>

using namespace ::com::sun::star;

/// Covers sw/source/core/unocore/ fixes.
class SwCoreUnocoreTest : public SwModelTestBase
{
public:
    SwCoreUnocoreTest()
        : SwModelTestBase(u"/sw/qa/core/unocore/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf119081)
{
    // Load a doc with a nested table in it.
    createSwDoc("tdf119081.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Enter outer A1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/3);
    // Enter inner A1.
    pWrtShell->Right(SwCursorSkipMode::Cells, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false,
                     /*bVisual=*/true);
    // Enter outer B1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);

    SwDoc* pDoc = pDocShell->GetDoc();
    SwPaM& rCursor = pWrtShell->GetCurrentShellCursor();
    rtl::Reference<SwXTextRange> xInsertPosition
        = SwXTextRange::CreateXTextRange(*pDoc, *rCursor.GetPoint(), nullptr);
    uno::Reference<text::XTextAppend> xTextAppend(xInsertPosition->getText(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.uno.RuntimeException
    xTextAppend->insertTextPortion(u"x"_ustr, {}, xInsertPosition);

    // Verify that the string is indeed inserted.
    pWrtShell->Left(SwCursorSkipMode::Cells, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false,
                    /*bVisual=*/true);
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, pWrtShell->GetCurrentShellCursor().GetText());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, selectTextRange)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> const xTD(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> const xText(xTD->getText());
    uno::Reference<text::XTextCursor> const xCursor(xText->createTextCursor());
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xCursor->getString());
    uno::Reference<lang::XMultiServiceFactory> const xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextSection> const xSection(
        xMSF->createInstance(u"com.sun.star.text.TextSection"_ustr), uno::UNO_QUERY_THROW);
    xText->insertTextContent(xCursor, xSection, true);
    uno::Reference<text::XTextRange> const xAnchor(xSection->getAnchor());
    uno::Reference<view::XSelectionSupplier> const xView(xTD->getCurrentController(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xAnchor->getString());
    CPPUNIT_ASSERT(xView->select(uno::Any(xAnchor)));
    uno::Reference<container::XIndexAccess> xSel;
    CPPUNIT_ASSERT(xView->getSelection() >>= xSel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSel->getCount());
    uno::Reference<text::XTextRange> xSelRange;
    CPPUNIT_ASSERT(xSel->getByIndex(0) >>= xSelRange);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xSelRange->getString());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, flyAtParaAnchor)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> const xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextDocument> const xTD(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextFrame> const xTextFrame(
        xMSF->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xFrameProps(xTextFrame, uno::UNO_QUERY_THROW);
    xFrameProps->setPropertyValue(u"AnchorType"_ustr,
                                  uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    auto const xText = xTD->getText();
    auto const xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextFrame, false);
    auto const xAnchor = xTextFrame->getAnchor();
    uno::Reference<text::XTextContent> const xFieldmark(
        xMSF->createInstance(u"com.sun.star.text.Fieldmark"_ustr), uno::UNO_QUERY_THROW);
    // this crashed because the anchor didn't have SwContentIndex
    xText->insertTextContent(xAnchor, xFieldmark, false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testRtlGutter)
{
    createSwDoc();
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Unknown property: RtlGutter
    auto bRtlGutter = getProperty<bool>(xPageStyle, u"RtlGutter"_ustr);
    CPPUNIT_ASSERT(!bRtlGutter);
    xPageStyle->setPropertyValue(u"RtlGutter"_ustr, uno::Any(true));
    bRtlGutter = getProperty<bool>(xPageStyle, u"RtlGutter"_ustr);
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testBiblioLocalCopy)
{
    // Given an empty document:
    createSwDoc();

    // When setting the LocalURL of a biblio field:
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"ARJ00"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Me"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"mytitle"_ustr),
        comphelper::makePropertyValue(u"Year"_ustr, u"2020"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf"_ustr),
        comphelper::makePropertyValue(u"LocalURL"_ustr, u"file:///home/me/test.pdf"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Then make sure we get that LocalURL back:
    comphelper::SequenceAsHashMap aMap(xField->getPropertyValue(u"Fields"_ustr));
    // Without the accompanying fix in place, this test would have failed, there was no LocalURL key
    // in the map.
    CPPUNIT_ASSERT(aMap.find(u"LocalURL"_ustr) != aMap.end());
    auto aActual = aMap[u"LocalURL"_ustr].get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"file:///home/me/test.pdf"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLinkedStyles)
{
    // Given an empty document:
    createSwDoc();

    // When defining a linked style for a para style:
    uno::Reference<container::XNameAccess> xParaStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference<beans::XPropertySet> xParaStyle(xParaStyles->getByName(u"Caption"_ustr),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParaStyle, u"LinkStyle"_ustr));
    xParaStyle->setPropertyValue(u"LinkStyle"_ustr, uno::Any(u"Emphasis"_ustr));
    // Then make sure we get the linked char style back:
    CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xParaStyle, u"LinkStyle"_ustr));

    // When defining a linked style for a char style:
    uno::Reference<container::XNameAccess> xCharStyles = getStyles(u"CharacterStyles"_ustr);
    uno::Reference<beans::XPropertySet> xCharStyle(xCharStyles->getByName(u"Emphasis"_ustr),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xCharStyle, u"LinkStyle"_ustr));
    xCharStyle->setPropertyValue(u"LinkStyle"_ustr, uno::Any(u"Caption"_ustr));
    // Then make sure we get the linked para style back:
    CPPUNIT_ASSERT_EQUAL(u"Caption"_ustr, getProperty<OUString>(xCharStyle, u"LinkStyle"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testViewCursorTextFrame)
{
    // Given a document with a graphic and holding a reference to that graphic frame:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aInsertArgs
        = { comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"graphic.png")) };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aInsertArgs);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xViewCursor(xTextViewCursorSupplier->getViewCursor(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame;
    xViewCursor->getPropertyValue(u"TextFrame"_ustr) >>= xFrame;

    // When saving to ODT, then make sure the store doesn't fail:
    uno::Reference<frame::XStorable> xStorable(xModel, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreArgs
        = { comphelper::makePropertyValue(u"FilterName"_ustr, u"writer8"_ustr) };
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
    createSwDoc("broken-embedded-object.odt");
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xObject(xObjects->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xEmbeddedObject;
    // Get the property first, which initializes Draw, which would overwrite our error handler.
    xObject->getPropertyValue(u"EmbeddedObject"_ustr) >>= xEmbeddedObject;
    ErrorRegistry::RegisterDisplay(&BasicDisplayErrorHandler);

    // When trying to load that embedded object:
    xObject->getPropertyValue(u"EmbeddedObject"_ustr) >>= xEmbeddedObject;

    // Then make sure we get a non-empty reference and an error popup it not shown:
    CPPUNIT_ASSERT(xEmbeddedObject.is());
    // Without the accompanying fix in place, we got this reference, but first an error popup was
    // shown to the user.
    CPPUNIT_ASSERT(
        xEmbeddedObject->supportsService(u"com.sun.star.comp.embed.OCommonEmbeddedObject"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testLineBreakInsert)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a line-break with properties:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xLineBreak(
        xMSF->createInstance(u"com.sun.star.text.LineBreak"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue(u"Clear"_ustr, uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // Then make sure that both the line break and its matching text attribute is inserted:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: "\n" (newline)
    // - Actual  : "" (empty string)
    // i.e. SwXLineBreak::attach() did not insert the newline + its text attribute.
    CPPUNIT_ASSERT_EQUAL(u"\n"_ustr, pTextNode->GetText());
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
        xMSF->createInstance(u"com.sun.star.text.LineBreak"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
    auto eClear = static_cast<sal_Int16>(SwLineBreakClear::ALL);
    xLineBreakProps->setPropertyValue(u"Clear"_ustr, uno::Any(eClear));
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertTextContent(xCursor, xLineBreak, /*bAbsorb=*/false);

    // When enumerating the text portions of the only paragraph in the document:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);

    // Then make sure that the text portion type is correct + the clear type can be read:
    auto aPortionType = getProperty<OUString>(xTextPortion, u"TextPortionType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: LineBreak
    // - Actual  : Text
    // i.e. a line break with properties was part of the normal Text portion, making it impossible
    // to get those properties.
    CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aPortionType);
    xLineBreak = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, u"LineBreak"_ustr);
    eClear = getProperty<sal_Int16>(xLineBreak, u"Clear"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserFieldTooltip)
{
    // Given a document with a user field:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.User"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance(u"com.sun.star.text.FieldMaster.User"_ustr), uno::UNO_QUERY);
    xMaster->setPropertyValue(u"Name"_ustr, uno::Any(u"a_user_field"_ustr));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue(u"Content"_ustr, uno::Any(u"42"_ustr));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xFieldProps(xField, uno::UNO_QUERY);

    // When setting a tooltip on the field:
    OUString aExpected(u"first line\nsecond line"_ustr);
    xFieldProps->setPropertyValue(u"Title"_ustr, uno::Any(aExpected));

    // Then make sure that the tooltip we read back matches the one previously specified:
    // Without the accompanying fix in place, this test would have failed with:
    // - the property is of unexpected type or void: Title
    // i.e. reading of the tooltip was broken.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(xFieldProps, u"Title"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlInsert)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a content control around one or more text portions:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    // Set a custom property on the content control:
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the text attribute is inserted:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
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

    // Also verify that setText() and getText() works:
    uno::Reference<text::XText> xContentControlText(xContentControl, uno::UNO_QUERY);
    xContentControlText->setString(u"new"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: new
    // - Actual  :
    // i.e. getString() always returned an empty string.
    CPPUNIT_ASSERT_EQUAL(u"new"_ustr, xContentControlText->getString());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testImageTooltip)
{
    // Given a document with an image and a hyperlink on it:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xImage(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xImage, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xImageProps(xImage, uno::UNO_QUERY);
    xImageProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"http://www.example.com"_ustr));

    // When setting a tooltip on the image:
    OUString aExpected(u"first line\nsecond line"_ustr);
    xImageProps->setPropertyValue(u"Tooltip"_ustr, uno::Any(aExpected));

    // Then make sure that the tooltip we read back matches the one previously specified:
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // i.e. reading/writing of the tooltip was broken.
    CPPUNIT_ASSERT_EQUAL(aExpected, getProperty<OUString>(xImageProps, u"Tooltip"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlTextPortionEnum)
{
    // Given a document with a content control around one or more text portions:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When enumerating the text portions of the only paragraph in the document:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);

    // Then make sure that the text portion type is correct + the content can be read:
    auto aPortionType = getProperty<OUString>(xTextPortion, u"TextPortionType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : Text
    // i.e. the content control text attribute was ignored.
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    xContentControl
        = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, u"ContentControl"_ustr);
    uno::Reference<text::XTextRange> xContentControlRange(xContentControl, uno::UNO_QUERY);
    xText = xContentControlRange->getText();
    uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xContent->getString());

    // Also test the generated layout:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr,
                u""_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: PortionType::ContentControl
    // - Actual  : PortionType::Text
    // i.e. SwContentControl generated a plain text portion, not a dedicated content control
    // portion.
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "type"_ostr,
                u"PortionType::ContentControl"_ustr);
    assertXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "portion"_ostr,
                u"test*"_ustr);

    // Also test the doc model, make sure that there is a dummy character at the start and end, so
    // the user can explicitly decide if they want to expand the content control or not:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aText = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ^Atest^A
    // - Actual  : ^Atest
    // i.e. there was no dummy character at the end.
    CPPUNIT_ASSERT_EQUAL(u"\x0001test\x0001"_ustr, aText);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlCheckbox)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a checkbox content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue(u"Checkbox"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"Checked"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"CheckedState"_ustr, uno::Any(u"☒"_ustr));
    xContentControlProps->setPropertyValue(u"UncheckedState"_ustr, uno::Any(u"☐"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetCheckbox());
    CPPUNIT_ASSERT(pContentControl->GetChecked());
    CPPUNIT_ASSERT_EQUAL(u"☒"_ustr, pContentControl->GetCheckedState());
    CPPUNIT_ASSERT_EQUAL(u"☐"_ustr, pContentControl->GetUncheckedState());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlDropdown)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a dropdown content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"red"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"R"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"green"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"G"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"blue"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"B"_ustr)),
            },
        };
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
        xContentControlProps->setPropertyValue(u"ListItems"_ustr, uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    std::vector<SwContentControlListItem> aListItems = pContentControl->GetListItems();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aListItems.size());
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, aListItems[0].m_aDisplayText);
    CPPUNIT_ASSERT_EQUAL(u"R"_ustr, aListItems[0].m_aValue);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testInsertFileInContentControlException)
{
    // Given a document with a content control:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Reject inserting a document inside the content control:
    xCursor->goLeft(1, false);
    OUString aURL(createFileURL(u"tdf119081.odt"));
    uno::Reference<document::XDocumentInsertable> xInsertable(xCursor, uno::UNO_QUERY);
    CPPUNIT_ASSERT_THROW(xInsertable->insertDocumentFromURL(aURL, {}), uno::RuntimeException);

    // Accept inserting a document outside the content control:
    xCursor->goRight(1, false);
    xInsertable->insertDocumentFromURL(aURL, {});
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlPicture)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a picture content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue(u"Picture"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a date content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    xContentControlProps->setPropertyValue(u"Date"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"DateFormat"_ustr, uno::Any(u"M/d/yyyy"_ustr));
    xContentControlProps->setPropertyValue(u"DateLanguage"_ustr, uno::Any(u"en-US"_ustr));
    xContentControlProps->setPropertyValue(u"CurrentDate"_ustr,
                                           uno::Any(u"2022-05-25T00:00:00Z"_ustr));
    xContentControlProps->setPropertyValue(u"PlaceholderDocPart"_ustr,
                                           uno::Any(u"DefaultPlaceholder_-1854013437"_ustr));
    xContentControlProps->setPropertyValue(
        u"DataBindingPrefixMappings"_ustr,
        uno::Any(u"xmlns:ns0='http://schemas.microsoft.com/vsto/samples' "_ustr));
    xContentControlProps->setPropertyValue(
        u"DataBindingXpath"_ustr,
        uno::Any(u"/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]"_ustr));
    xContentControlProps->setPropertyValue(
        u"DataBindingStoreItemID"_ustr, uno::Any(u"{241A8A02-7FFD-488D-8827-63FBE74E8BC9}"_ustr));
    xContentControlProps->setPropertyValue(u"Color"_ustr, uno::Any(u"008000"_ustr));
    xContentControlProps->setPropertyValue(u"Alias"_ustr, uno::Any(u"myalias"_ustr));
    xContentControlProps->setPropertyValue(u"Tag"_ustr, uno::Any(u"mytag"_ustr));
    xContentControlProps->setPropertyValue(u"Id"_ustr, uno::Any(static_cast<sal_Int32>(123)));
    xContentControlProps->setPropertyValue(u"TabIndex"_ustr, uno::Any(sal_uInt32(1)));
    xContentControlProps->setPropertyValue(u"Lock"_ustr, uno::Any(u"sdtContentLocked"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetDate());
    CPPUNIT_ASSERT_EQUAL(u"M/d/yyyy"_ustr, pContentControl->GetDateFormat());
    CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, pContentControl->GetDateLanguage());
    CPPUNIT_ASSERT_EQUAL(u"2022-05-25T00:00:00Z"_ustr, pContentControl->GetCurrentDate());
    CPPUNIT_ASSERT_EQUAL(u"DefaultPlaceholder_-1854013437"_ustr,
                         pContentControl->GetPlaceholderDocPart());
    CPPUNIT_ASSERT_EQUAL(u"xmlns:ns0='http://schemas.microsoft.com/vsto/samples' "_ustr,
                         pContentControl->GetDataBindingPrefixMappings());
    CPPUNIT_ASSERT_EQUAL(u"/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]"_ustr,
                         pContentControl->GetDataBindingXpath());
    CPPUNIT_ASSERT_EQUAL(u"{241A8A02-7FFD-488D-8827-63FBE74E8BC9}"_ustr,
                         pContentControl->GetDataBindingStoreItemID());
    CPPUNIT_ASSERT_EQUAL(u"008000"_ustr, pContentControl->GetColor());
    CPPUNIT_ASSERT_EQUAL(u"myalias"_ustr, pContentControl->GetAlias());
    CPPUNIT_ASSERT_EQUAL(u"mytag"_ustr, pContentControl->GetTag());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(123), pContentControl->GetId());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(1), pContentControl->GetTabIndex());
    CPPUNIT_ASSERT_EQUAL(u"sdtContentLocked"_ustr, pContentControl->GetLock());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlPlainText)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a plain text content control around a text portion:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"PlainText"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the text attribute is inserted:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    CPPUNIT_ASSERT(pAttr);
    // Also verify that the type if plain text:
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetPlainText());

    // Now check if the char index range 2-4 is extended to 0-6 when we apply formatting:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Select "es" from "<dummy>test<dummy>".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    SfxItemSetFixed<RES_CHRATR_WEIGHT, RES_CHRATR_WEIGHT> aSet(pWrtShell->GetAttrPool());
    SvxWeightItem aItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT);
    aSet.Put(aItem);
    pWrtShell->SetAttrSet(aSet);
    pAttr = pTextNode->GetTextAttrAt(2, RES_TXTATR_AUTOFMT);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the plain text content control now had 3 portions (<dummy>t<b>es</b>t<dummy>), instead
    // of one (<b><dummy>test<dummy></b>).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pAttr->GetStart());
    CPPUNIT_ASSERT(pAttr->End());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), *pAttr->End());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControlComboBox)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a combobox content control:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"red"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"R"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"green"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"G"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, uno::Any(u"blue"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, uno::Any(u"B"_ustr)),
            },
        };
        xContentControlProps->setPropertyValue(u"ListItems"_ustr, uno::Any(aListItems));
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
        xContentControlProps->setPropertyValue(u"ComboBox"_ustr, uno::Any(true));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // Then make sure that the specified properties are set:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    std::vector<SwContentControlListItem> aListItems = pContentControl->GetListItems();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aListItems.size());
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, aListItems[0].m_aDisplayText);
    CPPUNIT_ASSERT_EQUAL(u"R"_ustr, aListItems[0].m_aValue);
    CPPUNIT_ASSERT(pContentControl->GetComboBox());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testContentControls)
{
    // Given an empty document:
    createSwDoc();
    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    uno::Reference<container::XIndexAccess> xContentControls = pXTextDocument->getContentControls();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xContentControls->getCount());

    // When inserting content controls:
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    // First tag1.
    xText->insertString(xCursor, u"test1"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    {
        uno::Reference<text::XTextContent> xContentControl(
            xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        xContentControlProps->setPropertyValue(u"Tag"_ustr, uno::Any(u"tag1"_ustr));
        xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);
    }
    xCursor->gotoStart(/*bExpand=*/false);
    // Then tag2 before tag1.
    xText->insertString(xCursor, u"test2"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->goRight(5, /*bExpand=*/true);
    {
        uno::Reference<text::XTextContent> xContentControl(
            xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        xContentControlProps->setPropertyValue(u"Tag"_ustr, uno::Any(u"tag2"_ustr));
        xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);
    }

    // Then make sure that XContentControls contains the items in a correct order:
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xContentControls->getCount());
    uno::Reference<beans::XPropertySet> xContentControl;
    xContentControls->getByIndex(0) >>= xContentControl;
    OUString aTag;
    xContentControl->getPropertyValue(u"Tag"_ustr) >>= aTag;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: tag2
    // - Actual  : tag1
    // i.e. the order of the items was sorted by insert time, not by their doc model position.
    CPPUNIT_ASSERT_EQUAL(u"tag2"_ustr, aTag);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testParagraphMarkerODFExport)
{
    // Given a document with a red numbering portion, from the paragraph marker's format:
    createSwDoc("paragraph-marker.docx");

    // When saving that as ODT + reload:
    saveAndReload(u"writer8"_ustr);

    // Then make sure that it still has the correct color:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 00ff0000 (COL_LIGHTRED)
    // - Actual  : ffffffff (COL_AUTO)
    // i.e. the custom "red" color was lost as RES_PARATR_LIST_AUTOFMT was not serialized to ODT.
    CPPUNIT_ASSERT_EQUAL(
        u"00ff0000"_ustr,
        getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr, "color"_ostr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testParagraphMarkerFormattedRun)
{
    // Given a document with a bold run and non-bold paragraph marker:
    createSwDoc("paragraph-marker-formatted-run.docx");

    // When saving that as ODT + reload:
    saveAndReload(u"writer8"_ustr);

    // Then make sure that the numbering portion is still non-bold, matching Word:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: normal
    // - Actual  : bold
    // i.e. the numbering portion was bold, while its weight should be normal.
    CPPUNIT_ASSERT_EQUAL(
        u"normal"_ustr, getXPath(pXmlDoc, "//SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
                                 "weight"_ostr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testFlySplit)
{
    // Given a document with a fly frame:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    uno::Reference<text::XTextFramesSupplier> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xDocument->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    bool bIsSplitAllowed{};
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // - Unknown property: IsSplitAllowed
    // i.e. the property was missing.
    xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
    CPPUNIT_ASSERT(!bIsSplitAllowed);

    // When marking it as IsSplitAllowed=true:
    xFrame->setPropertyValue(u"IsSplitAllowed"_ustr, uno::Any(true));

    // Then make sure that IsSplitAllowed is true when asking back:
    xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
    CPPUNIT_ASSERT(bIsSplitAllowed);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testConvertToTextFrame)
{
    // Given a document with 2 non-interesting frames, an inner frame and an outer frame:
    createSwDoc("floattable-outer-nonsplit-inner.docx");

    // When checking the anchor of the inner frame:
    SwDoc* pDoc = getSwDoc();
    const sw::FrameFormats<sw::SpzFrameFormat*>& rFrames = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFrame3 = rFrames.FindFrameFormatByName(u"Frame3"_ustr);
    SwNodeIndex aFrame3Anchor = pFrame3->GetAnchor().GetContentAnchor()->nNode;

    // Then make sure it's anchored in the outer frame's last content node:
    sw::SpzFrameFormat* pFrame4 = rFrames.FindFrameFormatByName(u"Frame4"_ustr);
    SwPaM aPaM(*pFrame4->GetContent().GetContentIdx()->GetNode().EndOfSectionNode());
    aPaM.Move(fnMoveBackward, GoInContent);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: SwNodeIndex (node 27)
    // - Actual  : SwNodeIndex (node 49)
    // i.e. Frame3 was anchored much later, in the body text, not in Frame4.
    CPPUNIT_ASSERT_EQUAL(aPaM.GetPoint()->nNode, aFrame3Anchor);
}

namespace
{
/// This selection listener calls XTextRange::getString() on a selection change, which triggered
/// a new selection change event by accident, resulting infinite recursion and crash
struct SelectionChangeListener : public cppu::WeakImplHelper<view::XSelectionChangeListener>
{
public:
    SelectionChangeListener();
    // view::XSelectionChangeListener
    void SAL_CALL selectionChanged(const lang::EventObject& rEvent) override;

    // lang::XEventListener
    void SAL_CALL disposing(const lang::EventObject& rSource) override;
};
}

SelectionChangeListener::SelectionChangeListener() {}

void SelectionChangeListener::selectionChanged(const lang::EventObject& rEvent)
{
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(rEvent.Source, uno::UNO_QUERY);
    css::uno::Reference<css::container::XIndexAccess> xSelection(xSelectionSupplier->getSelection(),
                                                                 css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelection->getCount());
    css::uno::Reference<css::text::XTextRange> xTextRange(xSelection->getByIndex(0),
                                                          css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xTextRange->getString().startsWith("test"));
}

void SelectionChangeListener::disposing(const lang::EventObject& /*rSource*/) {}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf155951)
{
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->addSelectionChangeListener(new SelectionChangeListener());

    // This crashed here because of infinite recursion
    dispatchCommand(mxComponent, u".uno:WordLeftSel"_ustr, {});

    // this needs to wait for dispatching (trigger also a second selection change)
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testCollectFrameAtNodeWithLayout)
{
    // Given a document with a floating table on 2 pages, with a calculated layout:
    createSwDoc("floattable-split.docx");
    calcLayout();

    // When saving to ODT:
    save(u"writer8"_ustr);

    // Then make sure the output is valid and hasa 1 <draw:frame>:
    // Without the accompanying fix in place, this test would have failed with:
    // Error: uncompleted content model.
    // i.e. the output was not valid, the second <draw:frame> has an empty <table:table> as a child
    // element.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Also make sure that we don't have multiple <draw:frame> elements in the first place.
    assertXPath(pXmlDoc, "//draw:frame"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf149555)
{
    createSwDoc("tdf149555.docx");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    xCursor->jumpToFirstPage();
    OUString sPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"HEADER 1"_ustr, xHeaderText->getString());

    // Without the fix in place, this test would have failed with
    // - Expected: HEADER 2
    // - Actual: HEADER 1
    xCursor->jumpToPage(2);
    sPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"HEADER 2"_ustr, xHeaderText->getString());

    xCursor->jumpToPage(3);
    sPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"HEADER 2"_ustr, xHeaderText->getString());

    xCursor->jumpToPage(4);
    sPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"HEADER 2"_ustr, xHeaderText->getString());
}

// just care that it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf108272Crash)
{
    createSwDoc("tdf108272-1-minimal.docx");
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testWrapTextAtFlyStart)
{
    // Given a document with a fly frame:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    uno::Reference<text::XTextFramesSupplier> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xDocument->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    bool bWrapTextAtFlyStart{};
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // - Unknown property: WrapTextAtFlyStart
    // i.e. the property was missing.
    xFrame->getPropertyValue(u"WrapTextAtFlyStart"_ustr) >>= bWrapTextAtFlyStart;
    CPPUNIT_ASSERT(!bWrapTextAtFlyStart);

    // When marking it as WrapTextAtFlyStart=true:
    xFrame->setPropertyValue(u"WrapTextAtFlyStart"_ustr, uno::Any(true));

    // Then make sure that WrapTextAtFlyStart is true when asking back:
    xFrame->getPropertyValue(u"WrapTextAtFlyStart"_ustr) >>= bWrapTextAtFlyStart;
    CPPUNIT_ASSERT(bWrapTextAtFlyStart);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
