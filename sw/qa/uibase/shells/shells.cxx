/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/processfactory.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <docsh.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/uibase/shells/data/";

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testTdf130179)
{
    SwDoc* pDoc = createSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testShapeTextAlignment)
{
// FIXME find out why this fails on macOS/Windows
#if !defined(MACOSX) && !defined(_WIN32)
    // Create a document with a rectangle in it.
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aStartPos(1000, 1000);
    pWrtShell->BeginCreate(static_cast<sal_uInt16>(OBJ_RECT), aStartPos);
    Point aMovePos(2000, 2000);
    pWrtShell->MoveCreate(aMovePos);
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Start shape text edit.
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Select the shape.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    pView->StopShellTimer();
    // Start the actual text edit.
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
    SdrObject* pObject = pPage->GetObj(0);
    pView->EnterShapeDrawTextMode(pObject);
    pView->AttrChangedNotify(nullptr);

    // Change paragraph adjustment to center.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_PARA_ADJUST_CENTER,
                                                    SfxCallMode::SYNCHRON);

    // End shape text edit.
    pWrtShell->EndTextEdit();

    const OutlinerParaObject* pOutliner = pObject->GetOutlinerParaObject();
    // Without the accompanying fix in place, this test would have failed, because the shape had no
    // text or text formatting. In other words the paragraph adjustment command was ignored.
    CPPUNIT_ASSERT(pOutliner);
    const SfxItemSet& rParaAttribs = pOutliner->GetTextObject().GetParaAttribs(0);
    SvxAdjust eAdjust = rParaAttribs.GetItem(EE_PARA_JUST)->GetAdjust();
    CPPUNIT_ASSERT_EQUAL(SvxAdjust::Center, eAdjust);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testOleSavePreviewUpdate)
{
    // Load a document with 2 charts in it. The second is down enough that you have to scroll to
    // trigger its rendering. Previews are missing for both.
    load(DATA_DIRECTORY, "ole-save-preview-update.odt");

    // Explicitly update OLE previews, etc.
    dispatchCommand(mxComponent, ".uno:UpdateAll", {});

    // Save the document and see if we get the previews.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // Without the accompanying fix in place, this test would have failed, because the object
    // replacements were not generated, even after UpdateAll.
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 2"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
