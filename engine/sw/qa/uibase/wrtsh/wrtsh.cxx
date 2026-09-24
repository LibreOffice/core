/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <wrtsh.hxx>

#include <optional>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <cpo/uno/Sequence.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/svxenum.hxx>
#include <officecfg/Office/Common.hxx>
#include <numrule.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/viewfrm.hxx>

#include <swmodeltestbase.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <formatlinebreak.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <unotxdoc.hxx>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <frmatr.hxx>

#include <vcl/scheduler.hxx>

#include <svx/fontworkbar.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <swdtflvr.hxx>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
/// Covers sw/source/uibase/wrtsh/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/uibase/wrtsh/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testInsertLineBreak)
{
    // Given an empty document:
    createSwDoc();

    // When inserting a clearing break:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, cpo::uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When going to that content control in placeholder mode:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // The default Liberation Serif doesn't have a checkmark glyph, avoid font fallback.
    SwView& rView = pWrtShell->GetView();
    SfxItemSet aSet(
        SfxItemSet::makeFixedSfxItemSet<RES_CHRATR_BEGIN, RES_CHRATR_END>(rView.GetPool()));
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
    xContentControlProps->setPropertyValue(u"Checkbox"_ustr, cpo::uno::Any(true));
    xContentControlProps->setPropertyValue(u"Checked"_ustr, cpo::uno::Any(true));
    xContentControlProps->setPropertyValue(u"CheckedState"_ustr, cpo::uno::Any(u"☒"_ustr));
    xContentControlProps->setPropertyValue(u"UncheckedState"_ustr, cpo::uno::Any(u"☐"_ustr));
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

    // When inserting a content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // The default Liberation Serif doesn't have a checkmark glyph, avoid font fallback.
    SwView& rView = pWrtShell->GetView();
    SfxItemSet aSet(
        SfxItemSet::makeFixedSfxItemSet<RES_CHRATR_BEGIN, RES_CHRATR_END>(rView.GetPool()));
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
        cpo::uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, cpo::uno::Any(u"red"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, cpo::uno::Any(u"R"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, cpo::uno::Any(u"green"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, cpo::uno::Any(u"G"_ustr)),
            },
            {
                comphelper::makePropertyValue(u"DisplayText"_ustr, cpo::uno::Any(u"blue"_ustr)),
                comphelper::makePropertyValue(u"Value"_ustr, cpo::uno::Any(u"B"_ustr)),
            },
        };
        xContentControlProps->setPropertyValue(u"ListItems"_ustr, cpo::uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   cpo::uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"ShowingPlaceHolder"_ustr, cpo::uno::Any(true));
    xContentControlProps->setPropertyValue(u"Picture"_ustr, cpo::uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    xContentControlProps->setPropertyValue(u"Date"_ustr, cpo::uno::Any(true));
    xContentControlProps->setPropertyValue(u"DateFormat"_ustr, cpo::uno::Any(u"YYYY-MM-DD"_ustr));
    xContentControlProps->setPropertyValue(u"DateLanguage"_ustr, cpo::uno::Any(u"en-US"_ustr));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When clicking on that content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a date content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a plain text content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

    // When inserting a combo box content control:
    dispatchCommand(mxComponent, u".uno:InsertComboBoxContentControl"_ustr, {});

    // Then make sure that the matching text attribute is added to the document model:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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

void InsertSplitFly(SwWrtShell* pWrtShell)
{
    SwPosition aInsertPos = *pWrtShell->GetCursor()->GetPoint();
    // Insert a table:
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->GoPrevCell();
    pWrtShell->Insert(u"A1"_ustr);
    // Select cell:
    pWrtShell->SelAll();
    // Select table:
    pWrtShell->SelAll();
    // Wrap the table in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    // Set fly properties:
    pWrtShell->StartAllAction();
    SwFrameFormat* pFly = pWrtShell->GetFlyFrameFormat();
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
    aAnchor.SetAnchor(&aInsertPos);
    aSet.Put(aAnchor);
    SwDoc* pDoc = pWrtShell->GetDoc();
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();
    pWrtShell->EnterStdMode();
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlysAnchorJoin)
{
    // Given a document with two paragraphs, each serving as an anchor of a split fly:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first para"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second para"_ustr);
    pWrtShell->SttEndDoc(/*bStt=*/true);
    InsertSplitFly(pWrtShell);
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SttPara();
    InsertSplitFly(pWrtShell);

    // When trying to delete at the end of the first para:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->EndPara();
    pWrtShell->DelRight();

    // Then make sure the join doesn't happen till a text node can only be an anchor for one split
    // fly:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: first para
    // - Actual  : first parasecond para
    // i.e. we did join the 2 anchors and for complex enough documents the layout never finished.
    CPPUNIT_ASSERT_EQUAL(u"first para"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());
    pWrtShell->SttEndDoc(/*bStt=*/false);
    CPPUNIT_ASSERT_EQUAL(u"second para"_ustr, pCursor->GetPointNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testBulletCharChangeOnIndent)
{
    // Given an empty document:
    createSwDoc();

    // When adding 2 bullets, A is level 1, B is level 2:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->BulletOn();
    pWrtShell->Insert(u"A"_ustr);
    pWrtShell->SplitNode();
    // Increase indent: downgrade to level 2.
    pWrtShell->NumUpDown(/*bDown=*/true);
    pWrtShell->Insert(u"B"_ustr);

    // Then make sure the bullet characters are different:
    pWrtShell->Up(/*bSelect=*/false);
    SwCursor* pCursor = pWrtShell->GetCursor();
    sal_UCS4 nBullet1 = 0;
    {
        SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
        SwNumRule* pNumRule = pTextNode->GetNumRule();
        const SwNumFormat& rNumFormat = pNumRule->Get(pTextNode->GetActualListLevel());
        nBullet1 = rNumFormat.GetBulletChar();
    }
    pWrtShell->Down(/*bSelect=*/false);
    sal_UCS4 nBullet2 = 0;
    {
        SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
        SwNumRule* pNumRule = pTextNode->GetNumRule();
        const SwNumFormat& rNumFormat = pNumRule->Get(pTextNode->GetActualListLevel());
        nBullet2 = rNumFormat.GetBulletChar();
    }
    // Without the accompanying fix in place, this test would have failed, while nBullet1 should be
    // • and nBullet2 should be ◦.
    CPPUNIT_ASSERT(nBullet1 != nBullet2);
}

CPPUNIT_TEST_FIXTURE(Test, testRemoveIndent)
{
    // Given a document with an empty, bulleted paragraph at the document end:
    createSwDoc("remove-indent.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    // Press backspace once to make it not numbered:
    bool bOnlyBackspaceKey = true;
    pWrtShell->NumOrNoNum(!bOnlyBackspaceKey);

    // When pressing backspace again to try to decrease its indent to change from left margin to
    // first line margin:
    pWrtShell->TryRemoveIndent();

    // Then make sure we actually decrease the indent:
    SwPaM* pCursor = pWrtShell->GetCursor();
    SwTextNode* pTextNode = pCursor->GetPointNode().GetTextNode();
    SwTwips nLeftMargin = pTextNode->GetSwAttrSet().GetTextLeftMargin().GetTextLeft().m_dValue;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1135
    // - Actual  : 1418
    // i.e. there was no decrease of the left text margin on pressing backspace.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(1135), nLeftMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testCutFontworkObject)
{
    // Given a document with a fontwork object named "fontwork1":
    createSwDoc("tdf58511.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwDoc* pDoc = getSwDoc();

    // First, verify that the fontwork object exists
    SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    SdrPage* pPage = pDrawModel->GetPage(0);
    SdrObject* pFontworkObj = nullptr;

    // Search for the fontwork object by name
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        if (pObj && pObj->GetName() == u"fontwork1")
        {
            // Check if it's actually a fontwork object using the helper function
            if (svx::checkForFontWork(pObj))
            {
                pFontworkObj = pObj;
                break;
            }
        }
    }

    // Make sure the fontwork object was found initially
    CPPUNIT_ASSERT(pFontworkObj != nullptr);

    // Select the fontwork object
    SdrView* pSdrView = pWrtShell->GetDrawView();
    CPPUNIT_ASSERT(pSdrView != nullptr);

    // Clear any existing selection
    pSdrView->UnmarkAll();

    // Mark the fontwork object
    pSdrView->MarkObj(pFontworkObj, pSdrView->GetSdrPageView());

    // Verify the object is selected
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rMarkList.GetMarkCount());
    CPPUNIT_ASSERT_EQUAL(pFontworkObj, rMarkList.GetMark(0)->GetMarkedSdrObj());

    // When cutting the fontwork object - use SwTransferable::Copy with bIsCut = true:
    rtl::Reference<SwTransferable> xTransfer(new SwTransferable(*pWrtShell));
    xTransfer->Cut();

    // Then make sure the fontwork object is deleted from the document:
    pFontworkObj = nullptr;
    for (size_t i = 0; i < pPage->GetObjCount(); ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        if (pObj && pObj->GetName() == u"fontwork1")
        {
            if (svx::checkForFontWork(pObj))
            {
                pFontworkObj = pObj;
                break;
            }
        }
    }

    // Without the accompanying fix in place, this test would have failed because
    // the fontwork object was not properly deleted when cut.
    CPPUNIT_ASSERT(pFontworkObj == nullptr);
}
}

CPPUNIT_TEST_FIXTURE(Test, testMultiSelectionTextSelectionCallback)
{
    // Given a document with "ABC" and COKit active:
    comphelper::COKit::setActive(true);
    createSwDoc();
    SwXTextDocument* pTextDocument = getSwTextDoc();
    pTextDocument->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    int nViewId = KitHelper::getView(*pWrtShell->GetSfxViewShell());
    pWrtShell->Insert(u"ABC"_ustr);
    pWrtShell->SttEndDoc(/*bStt=*/true);

    // When having a multi-selection: "A" is selected and also the cursor is after "B":
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    // Move past "B" without selecting (ctrl+click equivalent):
    pWrtShell->EnterAddMode();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Then the COKit text selection payload should not be empty:
    std::optional<OString> aPayload
        = pWrtShell->getKitPayload(COKitCallbackType::TEXT_SELECTION, nViewId);
    CPPUNIT_ASSERT(aPayload.has_value());
    // Without the fix in place, this test would have failed, only the non-range "after B" selection
    // was part of the payload.
    CPPUNIT_ASSERT(!aPayload->isEmpty());

    // Tear down COKit:
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::COKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(Test, testSetAsDefaultNumberingListFormat)
{
    // A saved numbered default keeps its per-level type and list format, here
    // upper roman wrapped as "(I)", when the numbered-list toggle runs.
    {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultListNumbering::set(
            cpo::uno::Sequence<sal_Int32>{ SVX_NUM_ROMAN_UPPER }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultListFormats::set(
            cpo::uno::Sequence<OUString>{ u"(%1%)"_ustr }, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard aReset([] {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultListNumbering::set(
            cpo::uno::Sequence<sal_Int32>{ SVX_NUM_ARABIC }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultListFormats::set(
            cpo::uno::Sequence<OUString>(), pBatch);
        pBatch->commit();
    });

    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    const SwNumRule* pNumRule = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetNumRule();
    CPPUNIT_ASSERT(pNumRule);
    const SwNumFormat& rFormat = pNumRule->Get(0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(SVX_NUM_ROMAN_UPPER),
                         static_cast<sal_uInt16>(rFormat.GetNumberingType()));
    CPPUNIT_ASSERT(rFormat.HasListFormat());
    CPPUNIT_ASSERT_EQUAL(u"(%1%)"_ustr, rFormat.GetListFormat());
}

CPPUNIT_TEST_FIXTURE(Test, testSetAsDefaultMixedOutline)
{
    // A saved mixed outline default (level 1 numbered as "1.", level 2 a
    // bullet) is rebuilt by the "Select Outline Format" button.
    {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineNumbering::set(
            cpo::uno::Sequence<sal_Int32>{ SVX_NUM_ARABIC, SVX_NUM_NUMBER_NONE }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineBullets::set(
            cpo::uno::Sequence<OUString>{ u"•"_ustr, u"▪"_ustr }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineBulletsFonts::set(
            cpo::uno::Sequence<OUString>{ u"OpenSymbol"_ustr, u"OpenSymbol"_ustr }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineListFormats::set(
            cpo::uno::Sequence<OUString>{ u"%1%."_ustr, u""_ustr }, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard aReset([] {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineNumbering::set(
            cpo::uno::Sequence<sal_Int32>(), pBatch);
        pBatch->commit();
    });

    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    dispatchCommand(mxComponent, u".uno:SetOutline"_ustr, {});

    const SwNumRule* pNumRule = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetNumRule();
    CPPUNIT_ASSERT(pNumRule);
    // Level 1 keeps the numbered type and its "1." format.
    const SwNumFormat& rLevel0 = pNumRule->Get(0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(SVX_NUM_ARABIC),
                         static_cast<sal_uInt16>(rLevel0.GetNumberingType()));
    CPPUNIT_ASSERT(rLevel0.HasListFormat());
    CPPUNIT_ASSERT_EQUAL(u"%1%."_ustr, rLevel0.GetListFormat());
    // Level 2 is the saved bullet.
    const SwNumFormat& rLevel1 = pNumRule->Get(1);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(SVX_NUM_CHAR_SPECIAL),
                         static_cast<sal_uInt16>(rLevel1.GetNumberingType()));
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x25AA), rLevel1.GetBulletChar());
}

CPPUNIT_TEST_FIXTURE(Test, testOutlineToggleOff)
{
    // A second click on "Select Outline Format" removes the mixed outline.
    {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineNumbering::set(
            cpo::uno::Sequence<sal_Int32>{ SVX_NUM_ARABIC, SVX_NUM_NUMBER_NONE }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineBullets::set(
            cpo::uno::Sequence<OUString>{ u"•"_ustr, u"▪"_ustr }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineBulletsFonts::set(
            cpo::uno::Sequence<OUString>{ u"OpenSymbol"_ustr, u"OpenSymbol"_ustr }, pBatch);
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineListFormats::set(
            cpo::uno::Sequence<OUString>{ u"%1%."_ustr, u""_ustr }, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard aReset([] {
        auto pBatch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::BulletsNumbering::DefaultOutlineNumbering::set(
            cpo::uno::Sequence<sal_Int32>(), pBatch);
        pBatch->commit();
    });

    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);

    // First click applies the mixed outline.
    dispatchCommand(mxComponent, u".uno:SetOutline"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetNumRule());

    // Second click removes it, so the paragraph is no longer in a list.
    dispatchCommand(mxComponent, u".uno:SetOutline"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetNumRule());
}

CPPUNIT_TEST_FIXTURE(Test, testSeveralShapesGetIndividualHandles)
{
    // Selecting several drawing objects shows the handles of each one, the way Toggle Point Edit
    // Mode does, and the command still toggles that back and forth.
    createSwDoc();

    auto xFactory(mxComponent.queryThrow<lang::XMultiServiceFactory>());
    auto xDrawPage(mxComponent.queryThrow<drawing::XDrawPageSupplier>()->getDrawPage());
    std::vector<SdrObject*> aShapes;
    for (sal_Int32 nShape = 0; nShape < 3; ++nShape)
    {
        auto xShape(xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr)
                        .queryThrow<drawing::XShape>());
        xDrawPage->add(xShape);
        xShape->setPosition(awt::Point(1000 + nShape * 3000, 1000));
        xShape->setSize(awt::Size(2000, 2000));
        SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
        CPPUNIT_ASSERT(pObject);
        aShapes.push_back(pObject);
    }

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();
    SdrView* pDrawView = pWrtShell->GetDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Toggle Point Edit Mode is handled by the drawing object shell. StopShellTimer brings that
    // shell up at once instead of waiting for the timer the selection started.
    auto togglePointEditMode = [&rView] {
        rView.StopShellTimer();
        rView.GetViewFrame().GetDispatcher()->Execute(SID_BEZIER_EDIT, SfxCallMode::SYNCHRON);
        Scheduler::ProcessEventsToIdle();
    };

    // One selected object keeps the single surrounding frame.
    pWrtShell->SelectObj(Point(), 0, aShapes[0]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT(rView.IsDrawSelMode());

    // A second selected object brings up the handles of both.
    pWrtShell->SelectObj(Point(), SW_ADD_SELECT, aShapes[1]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pDrawView->IsFrameDragSingles());
    // The view keeps its own copy of the mode, so the command shows the right state.
    CPPUNIT_ASSERT(!rView.IsDrawSelMode());

    // The command turns the individual handles off and on again.
    togglePointEditMode();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());
    togglePointEditMode();
    CPPUNIT_ASSERT(!pDrawView->IsFrameDragSingles());

    // Turning them off and then selecting one more object keeps them off, so the choice the user
    // made holds while the same objects stay selected.
    togglePointEditMode();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());
    pWrtShell->SelectObj(Point(), SW_ADD_SELECT, aShapes[2]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());

    // Falling back to one selected object restores the single frame and arms the default again.
    pWrtShell->SelectObj(Point(), 0, aShapes[0]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT(rView.IsDrawSelMode());
    pWrtShell->SelectObj(Point(), SW_ADD_SELECT, aShapes[1]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT(!rView.IsDrawSelMode());
}

namespace
{
/// Puts two differently sized rectangles into the document, anchored to the paragraph.
std::vector<SdrObject*> lcl_addTwoShapes(const uno::Reference<lang::XComponent>& xComponent)
{
    auto xFactory(xComponent.queryThrow<lang::XMultiServiceFactory>());
    auto xDrawPage(xComponent.queryThrow<drawing::XDrawPageSupplier>()->getDrawPage());
    const awt::Size aSizes[] = { awt::Size(4000, 2000), awt::Size(2000, 4000) };
    std::vector<SdrObject*> aShapes;
    for (size_t nShape = 0; nShape < std::size(aSizes); ++nShape)
    {
        auto xShape(xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr)
                        .queryThrow<drawing::XShape>());
        xDrawPage->add(xShape);
        // A shape anchored as character makes Writer protect the whole selection against
        // resizing, so anchor these the way the drawing toolbar does.
        xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(
            u"AnchorType"_ustr, uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
        xShape->setPosition(awt::Point(1000 + nShape * 6000, 1000));
        xShape->setSize(aSizes[nShape]);
        SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
        CPPUNIT_ASSERT(pObject);
        aShapes.push_back(pObject);
    }
    return aShapes;
}

/// Drags the given handle of the first shape by the given distance, with both shapes selected.
void lcl_dragHandleOfFirstShape(SwWrtShell* pWrtShell, const std::vector<SdrObject*>& rShapes,
                                SdrHdlKind eKind, const Point& rDistance)
{
    pWrtShell->SelectObj(Point(), 0, rShapes[0]);
    pWrtShell->SelectObj(Point(), SW_ADD_SELECT, rShapes[1]);
    Scheduler::ProcessEventsToIdle();

    SdrView* pDrawView = pWrtShell->GetDrawView();
    SdrHdl* pDragHandle = nullptr;
    const SdrHdlList& rHandles = pDrawView->GetHdlList();
    for (size_t i = 0; i < rHandles.GetHdlCount(); ++i)
    {
        if (rHandles.GetHdl(i)->GetKind() == eKind && rHandles.GetHdl(i)->GetObj() == rShapes[0])
            pDragHandle = rHandles.GetHdl(i);
    }
    CPPUNIT_ASSERT_MESSAGE("the first shape has no handle of that kind", pDragHandle);

    const Point aStart = pDragHandle->GetPos();
    CPPUNIT_ASSERT(pDrawView->BegDragObj(aStart, nullptr, pDragHandle, 0));
    pDrawView->MovDragObj(aStart + rDistance);
    pDrawView->EndDragObj();
    Scheduler::ProcessEventsToIdle();
}
}

CPPUNIT_TEST_FIXTURE(Test, testOneResizeHandleResizesEverySelectedShape)
{
    // Dragging the resize handle of one selected shape grows every selected shape by the same
    // factor, each around its own opposite corner, and a single undo puts them all back.
    createSwDoc();
    std::vector<SdrObject*> aShapes = lcl_addTwoShapes(mxComponent);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    // Snapping would round the drag, and these assertions are about the exact geometry.
    pWrtShell->GetDrawView()->SetSnapEnabled(false);

    const tools::Rectangle aWide(aShapes[0]->GetSnapRect());
    const tools::Rectangle aTall(aShapes[1]->GetSnapRect());
    lcl_dragHandleOfFirstShape(pWrtShell, aShapes, SdrHdlKind::LowerRight, Point(1000, 500));

    // The dragged corner ends up where the pointer left it, and the opposite corner stays.
    const tools::Rectangle aNewWide(aShapes[0]->GetSnapRect());
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aWide.TopLeft(), aNewWide.TopLeft(), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aWide.BottomRight() + Point(1000, 500),
                                              aNewWide.BottomRight(), 1);

    // The other shape grows by that same factor around its own top left corner, which is what
    // makes this different from moving both corners by the same distance.
    const double fHorizontal = double(aNewWide.GetWidth()) / aWide.GetWidth();
    const double fVertical = double(aNewWide.GetHeight()) / aWide.GetHeight();
    const tools::Rectangle aNewTall(aShapes[1]->GetSnapRect());
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aTall.TopLeft(), aNewTall.TopLeft(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aTall.GetWidth() * fHorizontal, aNewTall.GetWidth(), 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aTall.GetHeight() * fVertical, aNewTall.GetHeight(), 2.0);

    // One undo brings both shapes back.
    pWrtShell->Do(SwWrtShell::UNDO, 1, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aWide, aShapes[0]->GetSnapRect(), 1);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aTall, aShapes[1]->GetSnapRect(), 1);
}

CPPUNIT_TEST_FIXTURE(Test, testOneRotateHandleTurnsEverySelectedShape)
{
    // Dragging the rotate handle of one selected shape turns every selected shape by the same
    // angle, each around its own centre, and a single undo puts them all back.
    createSwDoc();
    std::vector<SdrObject*> aShapes = lcl_addTwoShapes(mxComponent);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GetDrawView()->SetSnapEnabled(false);

    const tools::Rectangle aWide(aShapes[0]->GetSnapRect());
    const tools::Rectangle aTall(aShapes[1]->GetSnapRect());
    lcl_dragHandleOfFirstShape(pWrtShell, aShapes, SdrHdlKind::Rotate, Point(1000, 1000));

    // Both shapes end up turned by the same angle, and neither is left unturned.
    const Degree100 nWideAngle = aShapes[0]->GetRotateAngle();
    CPPUNIT_ASSERT(nWideAngle.get() != 0);
    CPPUNIT_ASSERT_EQUAL(nWideAngle.get(), aShapes[1]->GetRotateAngle().get());

    // Each shape turned around its own centre, so no shape travelled across the page.
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aWide.Center(), aShapes[0]->GetSnapRect().Center(),
                                              2);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aTall.Center(), aShapes[1]->GetSnapRect().Center(),
                                              2);

    // One undo brings both shapes back.
    pWrtShell->Do(SwWrtShell::UNDO, 1, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aShapes[0]->GetRotateAngle().get());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aShapes[1]->GetRotateAngle().get());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aWide, aShapes[0]->GetSnapRect(), 1);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aTall, aShapes[1]->GetSnapRect(), 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTogglePointEditModeReportsTheDrawingViewState)
{
    // Toggle Point Edit Mode shows the mode the drawing view is in, also for a line, which
    // draws its point handles whatever the mode says. One press then changes the mode.
    createSwDoc();
    std::vector<SdrObject*> aShapes = lcl_addTwoShapes(mxComponent);

    auto xFactory(mxComponent.queryThrow<lang::XMultiServiceFactory>());
    auto xDrawPage(mxComponent.queryThrow<drawing::XDrawPageSupplier>()->getDrawPage());
    auto xLine(xFactory->createInstance(u"com.sun.star.drawing.LineShape"_ustr)
                   .queryThrow<drawing::XShape>());
    xDrawPage->add(xLine);
    xLine.queryThrow<beans::XPropertySet>()->setPropertyValue(
        u"AnchorType"_ustr, uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    xLine->setPosition(awt::Point(1000, 6000));
    xLine->setSize(awt::Size(3000, 0));
    SdrObject* pLine = SdrObject::getSdrObjectFromXShape(xLine);
    CPPUNIT_ASSERT(pLine);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwView& rView = pWrtShell->GetView();
    SdrView* pDrawView = pWrtShell->GetDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Two selected shapes turn Point Edit Mode on.
    pWrtShell->SelectObj(Point(), 0, aShapes[0]);
    pWrtShell->SelectObj(Point(), SW_ADD_SELECT, aShapes[1]);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT(!rView.IsDrawSelMode());

    // Selecting the line alone turns Point Edit Mode off, and the view says the same.
    pWrtShell->SelectObj(Point(), 0, pLine);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT_EQUAL(pDrawView->IsFrameDragSingles(), rView.IsDrawSelMode());

    // The first press of the command turns Point Edit Mode back on.
    rView.StopShellTimer();
    rView.GetViewFrame().GetDispatcher()->Execute(SID_BEZIER_EDIT, SfxCallMode::SYNCHRON);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!pDrawView->IsFrameDragSingles());
    CPPUNIT_ASSERT_EQUAL(pDrawView->IsFrameDragSingles(), rView.IsDrawSelMode());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
