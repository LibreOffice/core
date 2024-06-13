/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <wrtsh.hxx>

#include <optional>

#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/propertyvalue.hxx>
#include <editeng/fontitem.hxx>

#include <swmodeltestbase.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <formatlinebreak.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <fmtanchr.hxx>
#include <view.hxx>

namespace
{
/// Covers sw/source/uibase/wrtsh/ fixes.
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testInsertLineBreak)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a clearing break:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    std::optional<SwLineBreakClear> oClear = SwLineBreakClear::ALL;
    pWrtShell->InsertLineBreak(oClear);

    // Then make sure it's not just a plain linebreak:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);
    auto aPortionType = getProperty<OUString>(xTextPortion, u"TextPortionType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: LineBreak
    // - Actual  : Text
    // i.e. the line break lost its "clear" property.
    CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aPortionType);
    auto xLineBreak
        = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, u"LineBreak"_ustr);
    auto eClear = getProperty<sal_Int16>(xLineBreak, u"Clear"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
}

CPPUNIT_TEST_FIXTURE(Test, testGotoContentControl)
{
    // Given a document with a content control:
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
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When going to that content control in placeholder mode:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the content control is selected (without the dummy character):
    // Without the accompanying fix in place, this test would have failed, the user had to manually
    // select the placeholder text.
    sal_Int32 nStart = pWrtShell->GetCursor()->Start()->GetContentIndex();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nStart);
    sal_Int32 nEnd = pWrtShell->GetCursor()->End()->GetContentIndex();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), nEnd);
}

CPPUNIT_TEST_FIXTURE(Test, testTickCheckboxContentControl)
{
    // Given a document with a checkbox (checked) content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // The default Liberation Serif doesn't have a checkmark glyph, avoid font fallback.
    SwView& rView = pWrtShell->GetView();
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
    SvxFontItem aFont(FAMILY_DONTKNOW, u"DejaVu Sans"_ustr, OUString(), PITCH_DONTKNOW,
                      RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
    aSet.Put(aFont);
    pWrtShell->SetAttrSet(aSet);

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"☒"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Checkbox"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"Checked"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"CheckedState"_ustr, uno::Any(u"☒"_ustr));
    xContentControlProps->setPropertyValue(u"UncheckedState"_ustr, uno::Any(u"☐"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the checkbox is no longer checked:
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: ☐
    // - Actual  : ☒
    // i.e. the text node's text was "Ballot Box with X", not just "Ballot Box".
    CPPUNIT_ASSERT_EQUAL(u"☐"_ustr, pTextNode->GetExpandText(pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed, nothing happened on
    // InsertContentControl().
    CPPUNIT_ASSERT(pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertCheckboxContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // The default Liberation Serif doesn't have a checkmark glyph, avoid font fallback.
    SwView& rView = pWrtShell->GetView();
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
    SvxFontItem aFont(FAMILY_DONTKNOW, u"DejaVu Sans"_ustr, OUString(), PITCH_DONTKNOW,
                      RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
    aSet.Put(aFont);
    pWrtShell->SetAttrSet(aSet);

    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, the inserted content
    // control wasn't a checkbox one.
    CPPUNIT_ASSERT(pContentControl->GetCheckbox());
}

CPPUNIT_TEST_FIXTURE(Test, testSelectDropdownContentControl)
{
    // Given a document with a dropdown content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"choose an item"_ustr, /*bAbsorb=*/false);
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
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    rFormatContentControl.GetContentControl()->SetSelectedListItem(0);
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the document text is updated:
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: red
    // - Actual  : choose an item
    // i.e. the document text was unchanged instead of display text of the first list item.
    CPPUNIT_ASSERT_EQUAL(u"red"_ustr, pTextNode->GetExpandText(pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertDropdownContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the inserted content control was a default (rich text) one, not a dropdown.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pContentControl->GetListItems().size());
}

CPPUNIT_TEST_FIXTURE(Test, testReplacePictureContentControl)
{
    // Given a document with a picture content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"Picture"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);
    pWrtShell->EnterSelFrameMode();
    const SwFrameFormat* pFlyFormat = pWrtShell->GetFlyFrameFormat();
    const SwFormatAnchor& rFormatAnchor = pFlyFormat->GetAnchor();
    SwNode* pAnchorNode = rFormatAnchor.GetAnchorNode();
    SwTextNode* pTextNode = pAnchorNode->GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the picture is replaced:
    CPPUNIT_ASSERT(!rFormatContentControl.GetContentControl()->GetShowingPlaceHolder());
    // Without the accompanying fix in place, this test would have failed, there was no special
    // handling for picture content control (how to interact with them), and the default handler
    // killed the image selection.
    CPPUNIT_ASSERT(pWrtShell->IsFrameSelected());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertPictureContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::PICTURE);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, there was no special
    // handling for picture content control, no placeholder fly content was inserted.
    CPPUNIT_ASSERT(pContentControl->GetPicture());
    CPPUNIT_ASSERT(pTextNode->GetTextAttrForCharAt(1, RES_TXTATR_FLYCNT));
}

CPPUNIT_TEST_FIXTURE(Test, testSelectDateContentControl)
{
    // Given a document with a date content control:
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
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Date"_ustr, uno::Any(true));
    xContentControlProps->setPropertyValue(u"DateFormat"_ustr, uno::Any(u"YYYY-MM-DD"_ustr));
    xContentControlProps->setPropertyValue(u"DateLanguage"_ustr, uno::Any(u"en-US"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    rFormatContentControl.GetContentControl()->SetSelectedDate(44705);
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the document text is updated:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2022-05-24
    // - Actual  : test
    // i.e. the content control was not updated.
    CPPUNIT_ASSERT_EQUAL(u"2022-05-24"_ustr, pTextNode->GetExpandText(pWrtShell->GetLayout()));
    CPPUNIT_ASSERT_EQUAL(u"2022-05-24T00:00:00Z"_ustr,
                         rFormatContentControl.GetContentControl()->GetCurrentDate());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertDateContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a date content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, there was no special
    // handling for date content control.
    CPPUNIT_ASSERT(pContentControl->GetDate());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertPlainTextContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a plain text content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::PLAIN_TEXT);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, there was no special
    // handling for plain text content controls.
    CPPUNIT_ASSERT(pContentControl->GetPlainText());

    CPPUNIT_ASSERT(pContentControl->GetShowingPlaceHolder());
    pWrtShell->GotoContentControl(rFormatContentControl);
    CPPUNIT_ASSERT(pContentControl->GetShowingPlaceHolder());
    pWrtShell->Insert(u"Foo"_ustr);
    // No longer showing placeholder text, as it has been changed
    CPPUNIT_ASSERT(!pContentControl->GetShowingPlaceHolder());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertComboBoxContentControl)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a combo box content control:
    dispatchCommand(mxComponent, u".uno:InsertComboBoxContentControl"_ustr, {});

    // Then make sure that the matching text attribute is added to the document model:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed, no content control was
    // inserted.
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    CPPUNIT_ASSERT(pAttr);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetComboBox());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
