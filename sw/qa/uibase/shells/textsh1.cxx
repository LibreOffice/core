/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/fontitem.hxx>

#include <docary.hxx>
#include <docsh.hxx>
#include <frmmgr.hxx>
#include <wrtsh.hxx>
#include <formatflysplit.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <formatcontentcontrol.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <poolfmt.hxx>

namespace
{
/// Covers sw/source/uibase/shells/textsh1.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/uibase/shells/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDeleteRichTextContentControl)
{
    // Given an empty document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // When inserting a content control:
    pWrtShell->InsertContentControl(SwContentControlType::RICH_TEXT);

    // Then verify that the content control has been inserted
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT(pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));

    // And verify that there is text in the document
    size_t nTextNodeLength = pTextNode->GetText().getLength();
    CPPUNIT_ASSERT(nTextNodeLength > 0);

    // And the content control manager has one content control
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rManager.GetCount());

    // When deleting the content control
    dispatchCommand(mxComponent, ".uno:DeleteContentControl", {});

    // Then verify that there are no content controls
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rManager.GetCount());
    CPPUNIT_ASSERT(!pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));
    CPPUNIT_ASSERT_EQUAL(nTextNodeLength, static_cast<size_t>(pTextNode->GetText().getLength()));
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteCheckboxContentControl)
{
    // Given an empty document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // The default Liberation Serif doesn't have a checkmark glyph, avoid font fallback.
    SwView& rView = pWrtShell->GetView();
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END> aSet(rView.GetPool());
    SvxFontItem aFont(FAMILY_DONTKNOW, u"DejaVu Sans"_ustr, OUString(), PITCH_DONTKNOW,
                      RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT);
    aSet.Put(aFont);
    SwTextFormatColl* pStyle
        = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);
    pStyle->SetFormatAttr(aSet);

    // When inserting a content control:
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);

    // Then verify that a checkbox content control exists in the document
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    CPPUNIT_ASSERT(pContentControl->GetCheckbox());

    // And the content control manager has one content control
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rManager.GetCount());

    // When deleting the content control
    dispatchCommand(mxComponent, ".uno:DeleteContentControl", {});

    // Then verify that there are no content controls
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rManager.GetCount());
    CPPUNIT_ASSERT(!pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));

    // And verify that text remains in the document
    size_t nTextNodeLength = pTextNode->GetText().getLength();
    CPPUNIT_ASSERT(nTextNodeLength > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteDropdownContentControl)
{
    // Create a document with a dropdown content control
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // Then verify that the content control manager has one content control
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rManager.GetCount());

    // When deleting the content control
    dispatchCommand(mxComponent, ".uno:DeleteContentControl", {});

    // Then verify that there are no content controls
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rManager.GetCount());
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT(!pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));

    // And verify that text remains in the document
    size_t nTextNodeLength = pTextNode->GetText().getLength();
    CPPUNIT_ASSERT(nTextNodeLength > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteDateContentControl)
{
    // Create a document with a date content control
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);

    // Then verify that the content control manager has one content control
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rManager.GetCount());

    // When deleting the content control
    dispatchCommand(mxComponent, ".uno:DeleteContentControl", {});

    // Then verify that there are no content controls
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rManager.GetCount());
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT(!pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL));

    // And verify that text remains in the document
    size_t nTextNodeLength = pTextNode->GetText().getLength();
    CPPUNIT_ASSERT(nTextNodeLength > 0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
