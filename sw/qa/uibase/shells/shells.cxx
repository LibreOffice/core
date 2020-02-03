/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/gdimtf.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/uibase/shells/data/";

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwUibaseShellsTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testTdf130179)
{
    SwDoc* pDoc = createDoc();
    IDocumentContentOperations& rIDCO = pDoc->getIDocumentContentOperations();
    SwCursorShell* pShell(pDoc->GetEditShell());
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>{});
    SfxItemSet aGrfSet(pDoc->GetAttrPool(), svl::Items<RES_GRFATR_BEGIN, RES_GRFATR_END - 1>{});
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
    aFrameSet.Put(aAnchor);
    GraphicObject aGrf;
    CPPUNIT_ASSERT(rIDCO.InsertGraphicObject(*pShell->GetCursor(), aGrf, &aFrameSet, &aGrfSet));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));

    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame()->GetBindings().QueryState(FN_POSTIT, pItem);
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: !pItem
    // i.e. comment insertion was enabled for an at-para anchored image.
    CPPUNIT_ASSERT(!pItem);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
