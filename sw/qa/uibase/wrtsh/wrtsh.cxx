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

#include <swmodeltestbase.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <formatlinebreak.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <fmtanchr.hxx>

namespace
{
/// Covers sw/source/uibase/wrtsh/ fixes.
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testInsertLineBreak)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a clearing break:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    std::optional<SwLineBreakClear> oClear = SwLineBreakClear::ALL;
    pWrtShell->InsertLineBreak(oClear);

    // Then make sure it's not just a plain linebreak:
    uno::Reference<css::text::XTextRange> xTextPortion = getRun(getParagraph(1), 1);
    auto aPortionType = getProperty<OUString>(xTextPortion, "TextPortionType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: LineBreak
    // - Actual  : Text
    // i.e. the line break lost its "clear" property.
    CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aPortionType);
    auto xLineBreak = getProperty<uno::Reference<text::XTextContent>>(xTextPortion, "LineBreak");
    auto eClear = getProperty<sal_Int16>(xLineBreak, "Clear");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
}

CPPUNIT_TEST_FIXTURE(Test, testGotoContentControl)
{
    // Given a document with a content control:
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
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("ShowingPlaceHolder", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When going to that content control in placeholder mode:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNode = pDoc->GetNodes()[nIndex]->GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    pWrtShell->GotoContentControl(rFormatContentControl);

    // Then make sure that the content control is selected (without the dummy character):
    // Without the accompanying fix in place, this test would have failed, the user had to manually
    // select the placeholder text.
    sal_Int32 nStart = pWrtShell->GetCursor()->Start()->nContent.GetIndex();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nStart);
    sal_Int32 nEnd = pWrtShell->GetCursor()->End()->nContent.GetIndex();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), nEnd);
}

CPPUNIT_TEST_FIXTURE(Test, testTickCheckboxContentControl)
{
    // Given a document with a checkbox (checked) content control:
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, OUString(u"☒"), /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Checkbox", uno::Any(true));
    xContentControlProps->setPropertyValue("Checked", uno::Any(true));
    xContentControlProps->setPropertyValue("CheckedState", uno::Any(OUString(u"☒")));
    xContentControlProps->setPropertyValue("UncheckedState", uno::Any(OUString(u"☐")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    CPPUNIT_ASSERT_EQUAL(OUString(u"☐"), pTextNode->GetExpandText(pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertContentControl)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed, nothing happened on
    // InsertContentControl().
    CPPUNIT_ASSERT(pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertCheckboxContentControl)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "choose an item", /*bAbsorb=*/false);
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
        xContentControlProps->setPropertyValue("ListItems", uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    CPPUNIT_ASSERT_EQUAL(OUString("red"), pTextNode->GetExpandText(pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertDropdownContentControl)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    SwDoc* pDoc = createSwDoc();
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
    xContentControlProps->setPropertyValue("ShowingPlaceHolder", uno::Any(true));
    xContentControlProps->setPropertyValue("Picture", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);
    pWrtShell->EnterSelFrameMode();
    const SwFrameFormat* pFlyFormat = pWrtShell->GetFlyFrameFormat();
    const SwFormatAnchor& rFormatAnchor = pFlyFormat->GetAnchor();
    const SwPosition* pAnchorPos = rFormatAnchor.GetContentAnchor();
    SwTextNode* pTextNode = pAnchorPos->nNode.GetNode().GetTextNode();
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
    SwDoc* pDoc = createSwDoc();

    // When inserting a content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::PICTURE);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Date", uno::Any(true));
    xContentControlProps->setPropertyValue("DateFormat", uno::Any(OUString("YYYY-MM-DD")));
    xContentControlProps->setPropertyValue("DateLanguage", uno::Any(OUString("en-US")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
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
    CPPUNIT_ASSERT_EQUAL(OUString("2022-05-24"), pTextNode->GetExpandText(pWrtShell->GetLayout()));
    CPPUNIT_ASSERT_EQUAL(OUString("2022-05-24T00:00:00Z"),
                         rFormatContentControl.GetContentControl()->GetCurrentDate());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertDateContentControl)
{
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();

    // When inserting a date content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);

    // Then make sure that the matching text attribute is added to the document model:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, there was no special
    // handling for date content control.
    CPPUNIT_ASSERT(pContentControl->GetDate());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
