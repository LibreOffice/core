/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "OutlineView.hxx"
#include <memory>
#include <editeng/forbiddencharacterstable.hxx>
#include <sfx2/progress.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svxids.hrc>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <svl/itempool.hxx>
#include <svl/style.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdundo.hxx>
#include <svl/brdcst.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editeng.hxx>

#include <editeng/editobj.hxx>
#include <editeng/editund2.hxx>

#include <editeng/editview.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/fhgtitem.hxx>

#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "OutlineViewShell.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "sdresid.hxx"
#include "Outliner.hxx"
#include "strings.hrc"
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "undo/undoobjects.hxx"
#include "undo/undomanager.hxx"
#include "stlsheet.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace sd {

// width: DIN A4, two margins at 1 cm each
#define OUTLINE_PAPERWIDTH 19000

// a progress bar gets displayed when more than
// PROCESS_WITH_PROGRESS_THRESHOLD pages are concerned
#define PROCESS_WITH_PROGRESS_THRESHOLD  5

struct SdParaAndPos
{
    Paragraph* pPara;
    sal_uInt16     nPos;
};

TYPEINIT1( OutlineView, ::sd::View );


OutlineView::OutlineView( DrawDocShell& rDocSh, ::Window* pWindow, OutlineViewShell& rOutlineViewSh)
: ::sd::View(*rDocSh.GetDoc(), pWindow, &rOutlineViewSh)
, mrOutlineViewShell(rOutlineViewSh)
, mrOutliner(*mrDoc.GetOutliner(sal_True))
, mnPagesToProcess(0)
, mnPagesProcessed(0)
, mbFirstPaint(sal_True)
, mpProgress(NULL)
, maDocColor( COL_WHITE )
, maLRSpaceItem( 0, 0, 2000, 0, EE_PARA_OUTLLRSPACE )
{
    sal_Bool bInitOutliner = sal_False;

    if (mrOutliner.GetViewCount() == 0)
    {
        // initialize Outliner: set Reference Device
        bInitOutliner = sal_True;
        mrOutliner.Init( OUTLINERMODE_OUTLINEVIEW );
        mrOutliner.SetRefDevice( SD_MOD()->GetRefDevice( rDocSh ) );
        sal_uLong nWidth = OUTLINE_PAPERWIDTH;
        mrOutliner.SetPaperSize(Size(nWidth, 400000000));
    }

    // insert View into Outliner
    for (sal_uInt16 nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        mpOutlinerView[nView] = NULL;
    }

    mpOutlinerView[0] = new OutlinerView(&mrOutliner, pWindow);
    Rectangle aNullRect;
    mpOutlinerView[0]->SetOutputArea(aNullRect);
    mrOutliner.SetUpdateMode(sal_False);
    mrOutliner.InsertView(mpOutlinerView[0], LIST_APPEND);

    onUpdateStyleSettings( true );

    if (bInitOutliner)
    {
        // fill Outliner with contents
        FillOutliner();
    }

    Link aLink( LINK(this,OutlineView,EventMultiplexerListener) );
    mrOutlineViewShell.GetViewShellBase().GetEventMultiplexer()->AddEventListener(
        aLink,
        tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_PAGE_ORDER);

    LanguageType eLang = mrOutliner.GetDefaultLanguage();
    maPageNumberFont = OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE, eLang, 0 );
    maPageNumberFont.SetHeight( 500 );

    maBulletFont.SetColor( COL_AUTO );
    maBulletFont.SetHeight( 1000 );
    maBulletFont.SetCharSet(RTL_TEXTENCODING_MS_1252);   // and replacing other values by standard
    maBulletFont.SetName( OUString( "StarSymbol" ) );
    maBulletFont.SetWeight(WEIGHT_NORMAL);
    maBulletFont.SetUnderline(UNDERLINE_NONE);
    maBulletFont.SetStrikeout(STRIKEOUT_NONE);
    maBulletFont.SetItalic(ITALIC_NONE);
    maBulletFont.SetOutline(sal_False);
    maBulletFont.SetShadow(sal_False);


    Reference<XFrame> xFrame (mrOutlineViewShell.GetViewShellBase().GetFrame()->GetTopFrame().GetFrameInterface(), UNO_QUERY);

    const OUString aSlotURL( ".uno:ShowSlide" );
    maSlideImage = GetImage( xFrame, aSlotURL, true );

    // Tell undo manager of the document about the undo manager of the
    // outliner, so that the former can synchronize with the later.
    sd::UndoManager* pDocUndoMgr = dynamic_cast<sd::UndoManager*>(mpDocSh->GetUndoManager());
    if (pDocUndoMgr != NULL)
        pDocUndoMgr->SetLinkedUndoManager(&mrOutliner.GetUndoManager());
}

/**
 * Destructor, restore Links, clear Oultiner
 */
OutlineView::~OutlineView()
{
    DBG_ASSERT(maDragAndDropModelGuard.get() == 0, "sd::OutlineView::~OutlineView(), prior drag operation not finished correctly!" );

    Link aLink( LINK(this,OutlineView,EventMultiplexerListener) );
    mrOutlineViewShell.GetViewShellBase().GetEventMultiplexer()->RemoveEventListener( aLink );
    DisconnectFromApplication();

    if( mpProgress )
        delete mpProgress;

    // unregister OutlinerViews and destroy them
    for (sal_uInt16 nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        if (mpOutlinerView[nView] != NULL)
        {
            mrOutliner.RemoveView( mpOutlinerView[nView] );
            delete mpOutlinerView[nView];
            mpOutlinerView[nView] = NULL;
        }
    }

    if (mrOutliner.GetViewCount() == 0)
    {
        // uninitialize Outliner: enable color display
        ResetLinks();
        sal_uLong nCntrl = mrOutliner.GetControlWord();
        mrOutliner.SetUpdateMode(sal_False); // otherwise there will be drawn on SetControlWord
        mrOutliner.SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);
        SvtAccessibilityOptions aOptions;
        mrOutliner.ForceAutoColor( aOptions.GetIsAutomaticFontColor() );
        mrOutliner.Clear();
    }
}




void OutlineView::ConnectToApplication (void)
{
    mrOutlineViewShell.GetActiveWindow()->GrabFocus();
    Application::AddEventListener(LINK(this, OutlineView, AppEventListenerHdl));
}




void OutlineView::DisconnectFromApplication (void)
{
    Application::RemoveEventListener(LINK(this, OutlineView, AppEventListenerHdl));
}





void OutlineView::Paint(const Rectangle& rRect, ::sd::Window* pWin)
{
    OutlinerView* pOlView = GetViewByWindow(pWin);

    if (pOlView)
    {
        pOlView->HideCursor();
        pOlView->Paint(rRect);

        pOlView->ShowCursor(mbFirstPaint);

        mbFirstPaint = sal_False;
    }
}

void OutlineView::InvalidateSlideNumberArea()
{
}

/**
 * Window size was changed
 */

void OutlineView::AdjustPosSizePixel(const Point &,const Size &,::sd::Window*)
{
}


void OutlineView::AddWindowToPaintView(OutputDevice* pWin)
{
    sal_Bool bAdded = sal_False;
    sal_Bool bValidArea = sal_False;
    Rectangle aOutputArea;
    const Color aWhiteColor( COL_WHITE );
    sal_uInt16 nView = 0;

    while (nView < MAX_OUTLINERVIEWS && !bAdded)
    {
        if (mpOutlinerView[nView] == NULL)
        {
            mpOutlinerView[nView] = new OutlinerView(&mrOutliner, dynamic_cast< ::sd::Window* >(pWin));
            mpOutlinerView[nView]->SetBackgroundColor( aWhiteColor );
            mrOutliner.InsertView(mpOutlinerView[nView], LIST_APPEND);
            bAdded = sal_True;

            if (bValidArea)
            {
                mpOutlinerView[nView]->SetOutputArea(aOutputArea);
            }
        }
        else if (!bValidArea)
        {
            aOutputArea = mpOutlinerView[nView]->GetOutputArea();
            bValidArea = sal_True;
        }

        nView++;
    }

    // white background in Outliner
    pWin->SetBackground( Wallpaper( aWhiteColor ) );

    ::sd::View::AddWindowToPaintView(pWin);
}


void OutlineView::DeleteWindowFromPaintView(OutputDevice* pWin)
{
    sal_Bool bRemoved = sal_False;
    sal_uInt16 nView = 0;
    ::Window* pWindow;

    while (nView < MAX_OUTLINERVIEWS && !bRemoved)
    {
        if (mpOutlinerView[nView] != NULL)
        {
            pWindow = mpOutlinerView[nView]->GetWindow();

            if (pWindow == pWin)
            {
                mrOutliner.RemoveView( mpOutlinerView[nView] );
                delete mpOutlinerView[nView];
                mpOutlinerView[nView] = NULL;
                bRemoved = sal_True;
            }
        }

        nView++;
    }

    ::sd::View::DeleteWindowFromPaintView(pWin);
}

/**
 * Return a pointer to the OutlinerView corresponding to the window
 */
OutlinerView* OutlineView::GetViewByWindow (::Window* pWin) const
{
    OutlinerView* pOlView = NULL;
    for (sal_uInt16 nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        if (mpOutlinerView[nView] != NULL)
        {
            if ( pWin == mpOutlinerView[nView]->GetWindow() )
            {
                pOlView = mpOutlinerView[nView];
            }
        }
    }
    return (pOlView);
}


/**
 * Return the title before a random paragraph
 */
Paragraph* OutlineView::GetPrevTitle(const Paragraph* pPara)
{
    sal_Int32 nPos = mrOutliner.GetAbsPos(const_cast<Paragraph*>(pPara));

    if (nPos > 0)
    {
        while(nPos)
        {
            pPara = mrOutliner.GetParagraph(--nPos);
            if( mrOutliner.HasParaFlag(pPara, PARAFLAG_ISPAGE) )
            {
                return const_cast< Paragraph* >( pPara );
            }
        }

    }
    return NULL;
}

/**
 * Return the title after a random paragraph
 */
Paragraph* OutlineView::GetNextTitle(const Paragraph* pPara)
{
    Paragraph* pResult = const_cast< Paragraph* >( pPara );

    sal_Int32 nPos = mrOutliner.GetAbsPos(pResult);

    do
    {
        pResult = mrOutliner.GetParagraph(++nPos);
        if( pResult && mrOutliner.HasParaFlag(pResult, PARAFLAG_ISPAGE) )
            return pResult;
    }
    while( pResult );

    return NULL;
}

/**
 * Handler for inserting pages (paragraphs)
 */
IMPL_LINK( OutlineView, ParagraphInsertedHdl, ::Outliner *, pOutliner )
{
    // we get calls to this handler during binary insert of drag and drop contents but
    // we ignore it here and handle it later in OnEndPasteOrDrop()
    if( maDragAndDropModelGuard.get() == 0 )
    {
        OutlineViewPageChangesGuard aGuard(this);

        Paragraph* pPara = pOutliner->GetHdlParagraph();

        sal_Int32 nAbsPos = mrOutliner.GetAbsPos( pPara );

        UpdateParagraph( nAbsPos );

        if( (nAbsPos == 0) || mrOutliner.HasParaFlag(pPara,PARAFLAG_ISPAGE) || mrOutliner.HasParaFlag(mrOutliner.GetParagraph( nAbsPos-1 ), PARAFLAG_ISPAGE) )
        {
            InsertSlideForParagraph( pPara );
            InvalidateSlideNumberArea();
        }
    }

    return 0;
}

/** creates and inserts an empty slide for the given paragraph */
SdPage* OutlineView::InsertSlideForParagraph( Paragraph* pPara )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::InsertSlideForParagraph(), model change without undo?!" );

    OutlineViewPageChangesGuard aGuard(this);

    mrOutliner.SetParaFlag( pPara, PARAFLAG_ISPAGE );
    // how many titles are there before the new title paragraph?
    sal_uLong nExample = 0L;            // position of the "example" page
    sal_uLong nTarget  = 0L;            // position of insertion
    while(pPara)
    {
        pPara = GetPrevTitle(pPara);
        if (pPara)
            nTarget++;
    }

    // if a new paragraph is created via RETURN before the first paragraph, the
    // Outliner reports the old paragraph (which was moved down) as a new
    // paragraph
    if (nTarget == 1)
    {
        OUString aTest = mrOutliner.GetText(mrOutliner.GetParagraph(0));
        if (aTest.isEmpty())
        {
            nTarget = 0;
        }
    }


    // the "example" page is the previous page - if it is available
    if (nTarget > 0)
    {
        nExample = nTarget - 1;

        sal_uInt16 nPageCount = mrDoc.GetSdPageCount( PK_STANDARD );
        if( nExample >= nPageCount )
            nExample = nPageCount - 1;
    }

    /**********************************************************************
    * All the time, a standard page is created before a notes page.
    * It is ensured that after each standard page the corresponding notes page
    * follows. A handout page is exactly one handout page.
    **********************************************************************/

    // this page is exemplary
    SdPage* pExample = (SdPage*)mrDoc.GetSdPage((sal_uInt16)nExample, PK_STANDARD);
    SdPage* pPage = (SdPage*)mrDoc.AllocPage(sal_False);

    pPage->SetLayoutName(pExample->GetLayoutName());

    // insert (page)
    mrDoc.InsertPage(pPage, (sal_uInt16)(nTarget) * 2 + 1);
    if( isRecordingUndo() )
        AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoNewPage(*pPage));

    // assign a master page to the standard page
    pPage->TRG_SetMasterPage(pExample->TRG_GetMasterPage());

    // set page size
    pPage->SetSize(pExample->GetSize());
    pPage->SetBorder( pExample->GetLftBorder(),
                      pExample->GetUppBorder(),
                      pExample->GetRgtBorder(),
                      pExample->GetLwrBorder() );

    // create new presentation objects (after <Title> or <Title with subtitle>
    // follows <Title with outline>, otherwise apply the layout of the previous
    // page
    AutoLayout eAutoLayout = pExample->GetAutoLayout();
    if (eAutoLayout == AUTOLAYOUT_TITLE ||
        eAutoLayout == AUTOLAYOUT_ONLY_TITLE)
    {
        pPage->SetAutoLayout(AUTOLAYOUT_ENUM, sal_True);
    }
    else
    {
        pPage->SetAutoLayout(pExample->GetAutoLayout(), sal_True);
    }

    /**********************************************************************
    |* now the notes page
    \*********************************************************************/
    pExample = (SdPage*)mrDoc.GetSdPage((sal_uInt16)nExample, PK_NOTES);
    SdPage* pNotesPage = (SdPage*)mrDoc.AllocPage(sal_False);

    pNotesPage->SetLayoutName(pExample->GetLayoutName());

    pNotesPage->SetPageKind(PK_NOTES);

    // insert (notes page)
    mrDoc.InsertPage(pNotesPage, (sal_uInt16)(nTarget) * 2 + 2);
    if( isRecordingUndo() )
        AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

    // assign a master page to the notes page
    pNotesPage->TRG_SetMasterPage(pExample->TRG_GetMasterPage());

    // set page size, there must be already one page available
    pNotesPage->SetSize(pExample->GetSize());
    pNotesPage->SetBorder( pExample->GetLftBorder(),
                           pExample->GetUppBorder(),
                           pExample->GetRgtBorder(),
                           pExample->GetLwrBorder() );

    // create presentation objects
    pNotesPage->SetAutoLayout(pExample->GetAutoLayout(), sal_True);

    mrOutliner.UpdateFields();

    return pPage;
}

/**
 * Handler for deleting pages (paragraphs)
 */
IMPL_LINK( OutlineView, ParagraphRemovingHdl, ::Outliner *, pOutliner )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::ParagraphRemovingHdl(), model change without undo?!" );

    OutlineViewPageChangesGuard aGuard(this);

    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if( pOutliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
    {
        // how many titles are in front of the title paragraph in question?
        sal_uLong nPos = 0L;
        while(pPara)
        {
            pPara = GetPrevTitle(pPara);
            if (pPara) nPos++;
        }

        // delete page and notes page
        sal_uInt16 nAbsPos = (sal_uInt16)nPos * 2 + 1;
        SdrPage* pPage = mrDoc.GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        mrDoc.RemovePage(nAbsPos);

        nAbsPos = (sal_uInt16)nPos * 2 + 1;
        pPage = mrDoc.GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        mrDoc.RemovePage(nAbsPos);

        // progress display if necessary
        if (mnPagesToProcess)
        {
            mnPagesProcessed++;

            if(mpProgress)
                mpProgress->SetState(mnPagesProcessed);

            if (mnPagesProcessed == mnPagesToProcess)
            {
                if(mpProgress)
                {
                    delete mpProgress;
                    mpProgress = NULL;
                }
                mnPagesToProcess = 0;
                mnPagesProcessed = 0;
            }
        }
        pOutliner->UpdateFields();
    }

    InvalidateSlideNumberArea();

    return 0;
}

/**
 * Handler for changing the indentation depth of paragraphs (requires inserting
 * or deleting of pages in some cases)
 */
IMPL_LINK( OutlineView, DepthChangedHdl, ::Outliner *, pOutliner )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::DepthChangedHdl(), no undo for model change?!" );

    OutlineViewPageChangesGuard aGuard(this);

    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if( pOutliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) && ((pOutliner->GetPrevFlags() & PARAFLAG_ISPAGE) == 0) )
    {
        // the current paragraph is transformed into a slide

        mrOutliner.SetDepth( pPara, -1 );

        // are multiple level 1 paragraphs being brought to level 0 and we
        // should start a progress view or a timer and didn't already?
        if (mnPagesToProcess == 0)
        {
            Window*       pActWin = mrOutlineViewShell.GetActiveWindow();
            OutlinerView* pOlView = GetViewByWindow(pActWin);

            std::vector<Paragraph*> aSelList;
            pOlView->CreateSelectionList(aSelList);

            Paragraph *pParagraph = NULL;
            for (std::vector<Paragraph*>::const_iterator iter = aSelList.begin(); iter != aSelList.end(); ++iter)
            {
                pParagraph = *iter;

                if( !pOutliner->HasParaFlag( pParagraph, PARAFLAG_ISPAGE ) &&
                    (pOutliner->GetDepth( pOutliner->GetAbsPos( pParagraph ) ) <= 0) )
                    mnPagesToProcess++;
            }

            mnPagesToProcess++; // the paragraph being in level 0 already
                                // should be included
            mnPagesProcessed = 0;

            if (mnPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD)
            {
                if( mpProgress )
                    delete mpProgress;

                mpProgress = new SfxProgress( GetDocSh(), SD_RESSTR(STR_CREATE_PAGES), mnPagesToProcess );
            }
            else
            {
                mpDocSh->SetWaitCursor( sal_True );
            }
        }

        ParagraphInsertedHdl(pOutliner);

        mnPagesProcessed++;

        // should there be a progress display?
        if (mnPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD)
        {
            if (mpProgress)
                mpProgress->SetState(mnPagesProcessed);
        }

        // was this the last page?
        if (mnPagesProcessed == mnPagesToProcess)
        {
            if (mnPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD && mpProgress)
            {
                delete mpProgress;
                mpProgress = NULL;
            }
            else
                mpDocSh->SetWaitCursor( sal_False );

            mnPagesToProcess = 0;
            mnPagesProcessed = 0;
        }
        pOutliner->UpdateFields();
    }
    else if( !pOutliner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) && ((pOutliner->GetPrevFlags() & PARAFLAG_ISPAGE) != 0) )
    {
        // the paragraph was a page but now becomes a normal paragraph

        // how many titles are before the title paragraph in question?
        sal_uLong nPos = 0L;
        Paragraph* pParagraph = pPara;
        while(pParagraph)
        {
            pParagraph = GetPrevTitle(pParagraph);
            if (pParagraph)
                nPos++;
        }
        // delete page and notes page

        sal_uInt16 nAbsPos = (sal_uInt16)nPos * 2 + 1;
        SdrPage* pPage = mrDoc.GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        mrDoc.RemovePage(nAbsPos);

        nAbsPos = (sal_uInt16)nPos * 2 + 1;
        pPage = mrDoc.GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        mrDoc.RemovePage(nAbsPos);

        pPage = GetPageForParagraph( pPara );

        mrOutliner.SetDepth( pPara, (pPage && (static_cast<SdPage*>(pPage)->GetAutoLayout() == AUTOLAYOUT_TITLE)) ?  -1 : 0 );

        // progress display if necessary
        if (mnPagesToProcess)
        {
            mnPagesProcessed++;
            if (mpProgress)
                mpProgress->SetState(mnPagesProcessed);

            if (mnPagesProcessed == mnPagesToProcess)
            {
                if(mpProgress)
                {
                    delete mpProgress;
                    mpProgress = NULL;
                }
                mnPagesToProcess = 0;
                mnPagesProcessed = 0;
            }
        }
        pOutliner->UpdateFields();
    }
    else if ( (pOutliner->GetPrevDepth() == 1) && ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 2 ) )
    {
        // how many titles are in front of the title paragraph in question?
        sal_Int32 nPos = -1L;

        Paragraph* pParagraph = pPara;
        while(pParagraph)
        {
            pParagraph = GetPrevTitle(pParagraph);
            if (pParagraph)
                nPos++;
        }

        if(nPos >= 0)
        {
            SdPage*pPage = (SdPage*)mrDoc.GetSdPage( (sal_uInt16) nPos, PK_STANDARD);

            if(pPage && pPage->GetPresObj(PRESOBJ_TEXT))
                pOutliner->SetDepth( pPara, 0 );
        }

    }
    // how many titles are in front of the title paragraph in question?
    sal_Int32 nPos = -1L;

    Paragraph* pTempPara = pPara;
    while(pTempPara)
    {
        pTempPara = GetPrevTitle(pTempPara);
        if (pTempPara)
            nPos++;
    }

    if( nPos >= 0 )
    {
        SdPage* pPage = (SdPage*) mrDoc.GetSdPage( (sal_uInt16) nPos, PK_STANDARD );

        if( pPage )
        {
            SfxStyleSheet* pStyleSheet = NULL;
            sal_Int32 nPara = pOutliner->GetAbsPos( pPara );
            sal_Int16 nDepth = pOutliner->GetDepth( nPara );
            bool bSubTitle = pPage->GetPresObj(PRESOBJ_TEXT) != NULL;

            if( pOutliner->HasParaFlag(pPara, PARAFLAG_ISPAGE) )
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
            }
            else if( bSubTitle )
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
            }
            else
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );

                if( nDepth > 0 )
                {
                    OUString aNewStyleSheetName = pStyleSheet->GetName();
                    if (!aNewStyleSheetName.isEmpty())
                        aNewStyleSheetName = aNewStyleSheetName.copy(0, aNewStyleSheetName.getLength() - 1);
                    aNewStyleSheetName += OUString::number( nDepth+1 );
                    SfxStyleSheetBasePool* pStylePool = mrDoc.GetStyleSheetPool();
                    pStyleSheet = (SfxStyleSheet*) pStylePool->Find( aNewStyleSheetName, pStyleSheet->GetFamily() );
                }
            }

            // before we set the style sheet we need to preserve the bullet item
            // since all items will be deleted while setting a new style sheet
             SfxItemSet aOldAttrs( pOutliner->GetParaAttribs( nPara ) );

            pOutliner->SetStyleSheet( nPara, pStyleSheet );

            // restore the old bullet item but not if the style changed
            if ( pOutliner->GetPrevDepth() != -1 && nDepth != -1 &&
                 aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
            {
                SfxItemSet aAttrs( pOutliner->GetParaAttribs( nPara ) );
                aAttrs.Put( *aOldAttrs.GetItem( EE_PARA_NUMBULLET ) );
                pOutliner->SetParaAttribs( nPara, aAttrs );
            }
        }
    }

    InvalidateSlideNumberArea();

    return 0;
}

/**
 * Handler for StatusEvents
 */
IMPL_LINK_NOARG(OutlineView, StatusEventHdl)
{
    ::sd::Window*   pWin = mrOutlineViewShell.GetActiveWindow();
    OutlinerView*   pOutlinerView = GetViewByWindow(pWin);
    Rectangle     aVis          = pOutlinerView->GetVisArea();
    sal_uLong nWidth = OUTLINE_PAPERWIDTH;
    Rectangle aText = Rectangle(Point(0,0),
                                   Size(nWidth,
                                        mrOutliner.GetTextHeight()));
    Rectangle aWin(Point(0,0), pWin->GetOutputSizePixel());
    aWin = pWin->PixelToLogic(aWin);

    if (!aVis.IsEmpty())        // not when opening
    {
        aText.Bottom() += aWin.GetHeight();

        mrOutlineViewShell.InitWindows(Point(0,0), aText.GetSize(),
                                       Point(aVis.TopLeft()));
        mrOutlineViewShell.UpdateScrollBars();
    }

    InvalidateSlideNumberArea();
    return 0;
}

IMPL_LINK_NOARG(OutlineView, BeginDropHdl)
{
    DBG_ASSERT(maDragAndDropModelGuard.get() == 0, "sd::OutlineView::BeginDropHdl(), prior drag operation not finished correctly!" );

    maDragAndDropModelGuard.reset( new OutlineViewModelChangeGuard( *this ) );
    return 0;
}

IMPL_LINK_NOARG(OutlineView, EndDropHdl)
{
    maDragAndDropModelGuard.reset(0);
    InvalidateSlideNumberArea();
    return 0;
}

/**
 * Handler for the start of a paragraph movement
 */
IMPL_LINK( OutlineView, BeginMovingHdl, ::Outliner *, pOutliner )
{
    OutlineViewPageChangesGuard aGuard(this);

    // list of selected title paragraphs
    mpOutlinerView[0]->CreateSelectionList(maSelectedParas);

    for (std::vector<Paragraph*>::iterator it = maSelectedParas.begin(); it != maSelectedParas.end();)
    {
        if (!pOutliner->HasParaFlag(*it, PARAFLAG_ISPAGE))
            it = maSelectedParas.erase(it);
        else
            ++it;
    }

    // select the pages belonging to the paragraphs on level 0 to select
    sal_uInt16 nPos = 0;
    sal_Int32 nParaPos = 0;
    Paragraph* pPara = pOutliner->GetParagraph( 0 );
    std::vector<Paragraph*>::const_iterator fiter;

    while(pPara)
    {
        if( pOutliner->HasParaFlag(pPara, PARAFLAG_ISPAGE) )                     // one page?
        {
            maOldParaOrder.push_back(pPara);
            SdPage* pPage = mrDoc.GetSdPage(nPos, PK_STANDARD);

            fiter = std::find(maSelectedParas.begin(),maSelectedParas.end(),pPara);

            pPage->SetSelected(fiter != maSelectedParas.end());

            ++nPos;
        }
        pPara = pOutliner->GetParagraph( ++nParaPos );
    }

    return 0;
}

/**
 * Handler for the end of a paragraph movement
 */
IMPL_LINK( OutlineView, EndMovingHdl, ::Outliner *, pOutliner )
{
    OutlineViewPageChangesGuard aGuard(this);

    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::EndMovingHdl(), model change without undo?!" );

    // look for insertion position via the first paragraph
    Paragraph* pSearchIt = maSelectedParas.empty() ? NULL : *(maSelectedParas.begin());

    // look for the first of the selected paragraphs in the new ordering
    sal_uInt16 nPosNewOrder = 0;
    sal_Int32 nParaPos = 0;
    Paragraph*  pPara = pOutliner->GetParagraph( 0 );
    Paragraph*  pPrev = NULL;
    while (pPara && pPara != pSearchIt)
    {
        if( pOutliner->HasParaFlag(pPara, PARAFLAG_ISPAGE) )
        {
            nPosNewOrder++;
            pPrev = pPara;
        }
        pPara = pOutliner->GetParagraph( ++nParaPos );
    }

    sal_uInt16 nPos = nPosNewOrder;     // don't change nPosNewOrder
    if (nPos == 0)
    {
        nPos = (sal_uInt16)-1;          // insert before the first page
    }
    else
    {
        // look for the predecessor in the old ordering
        std::vector<Paragraph*>::const_iterator it = std::find(maOldParaOrder.begin(),
                                                               maOldParaOrder.end(),
                                                               pPrev);

        if (it != maOldParaOrder.end())
            nPos = static_cast<sal_uInt16>(it-maOldParaOrder.begin());
        else
            nPos = 0xffff;

        DBG_ASSERT(nPos != 0xffff, "Paragraph not found");
    }

    mrDoc.MovePages(nPos);

    // deselect the pages again
    sal_uInt16 nPageCount = (sal_uInt16)maSelectedParas.size();
    while (nPageCount)
    {
        SdPage* pPage = mrDoc.GetSdPage(nPosNewOrder, PK_STANDARD);
        pPage->SetSelected(sal_False);
        nPosNewOrder++;
        nPageCount--;
    }

    pOutliner->UpdateFields();

    maSelectedParas.clear();
    maOldParaOrder.clear();

    InvalidateSlideNumberArea();

    return 0;
}

/**
 * Look for the title text object in one page of the model
 */
SdrTextObj* OutlineView::GetTitleTextObject(SdrPage* pPage)
{
    sal_uLong           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (sal_uLong nObject = 0; nObject < nObjectCount; nObject++)
    {
        pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor &&
            pObject->GetObjIdentifier() == OBJ_TITLETEXT)
        {
            pResult = (SdrTextObj*)pObject;
            break;
        }
    }
    return pResult;
}


/**
 * Look for the outline text object in one page of the model
 */
SdrTextObj* OutlineView::GetOutlineTextObject(SdrPage* pPage)
{
    sal_uLong           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (sal_uLong nObject = 0; nObject < nObjectCount; nObject++)
    {
        pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor &&
            pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            pResult = (SdrTextObj*)pObject;
            break;
        }
    }
    return pResult;
}

SdrTextObj* OutlineView::CreateTitleTextObject(SdPage* pPage)
{
    DBG_ASSERT( GetTitleTextObject(pPage) == 0, "sd::OutlineView::CreateTitleTextObject(), there is already a title text object!" );

    if( pPage->GetAutoLayout() == AUTOLAYOUT_NONE )
    {
        // simple case
        pPage->SetAutoLayout( AUTOLAYOUT_ONLY_TITLE, true );
    }
    else
    {
        // we already have a layout with a title but the title
        // object was deleted, create a new one
        pPage->InsertAutoLayoutShape( 0, PRESOBJ_TITLE, false, pPage->GetTitleRect(), true );
    }

    return GetTitleTextObject(pPage);
}

SdrTextObj* OutlineView::CreateOutlineTextObject(SdPage* pPage)
{
    DBG_ASSERT( GetOutlineTextObject(pPage) == 0, "sd::OutlineView::CreateOutlineTextObject(), there is already a layout text object!" );

    AutoLayout eNewLayout = pPage->GetAutoLayout();
    switch( eNewLayout )
    {
    case AUTOLAYOUT_NONE:
    case AUTOLAYOUT_ONLY_TITLE:
    case AUTOLAYOUT_TITLE:  eNewLayout = AUTOLAYOUT_ENUM; break;

    case AUTOLAYOUT_CHART:  eNewLayout = AUTOLAYOUT_CHARTTEXT; break;

    case AUTOLAYOUT_ORG:
    case AUTOLAYOUT_TAB:
    case AUTOLAYOUT_OBJ:    eNewLayout = AUTOLAYOUT_OBJTEXT; break;
    default:
        break;
    }

    if( eNewLayout != pPage->GetAutoLayout() )
    {
        pPage->SetAutoLayout( eNewLayout, true );
    }
    else
    {
        // we already have a layout with a text but the text
        // object was deleted, create a new one
        pPage->InsertAutoLayoutShape( 0,
                                      (eNewLayout == AUTOLAYOUT_TITLE) ? PRESOBJ_TEXT : PRESOBJ_OUTLINE,
                                      false, pPage->GetLayoutRect(), true );
    }

    return GetOutlineTextObject(pPage);
}

/** updates draw model with all changes from outliner model */
sal_Bool OutlineView::PrepareClose(sal_Bool)
{
    ::sd::UndoManager* pDocUndoMgr = dynamic_cast<sd::UndoManager*>(mpDocSh->GetUndoManager());
    if (pDocUndoMgr != NULL)
        pDocUndoMgr->SetLinkedUndoManager(NULL);

    mrOutliner.GetUndoManager().Clear();

    BegUndo(SD_RESSTR(STR_UNDO_CHANGE_TITLE_AND_LAYOUT));
    UpdateDocument();
    EndUndo();
    mrDoc.SetSelected(GetActualPage(), sal_True);
    return sal_True;
}


/**
 * Set attributes of the selected text
 */
sal_Bool OutlineView::SetAttributes(const SfxItemSet& rSet, sal_Bool )
{
    sal_Bool bOk = sal_False;

    OutlinerView* pOlView = GetViewByWindow(mrOutlineViewShell.GetActiveWindow());

    if (pOlView)
    {
        pOlView->SetAttribs(rSet);
        bOk = sal_True;
    }

    mrOutlineViewShell.Invalidate (SID_PREVIEW_STATE);

    return (bOk);
}

/**
 * Get attributes of the selected text
 */
sal_Bool OutlineView::GetAttributes( SfxItemSet& rTargetSet, sal_Bool ) const
{
    OutlinerView* pOlView = GetViewByWindow(
                                mrOutlineViewShell.GetActiveWindow());
    DBG_ASSERT(pOlView, "keine OutlinerView gefunden");

    rTargetSet.Put( pOlView->GetAttribs(), sal_False );
    return sal_True;
}

/** creates outliner model from draw model */
void OutlineView::FillOutliner()
{
    mrOutliner.GetUndoManager().Clear();
    mrOutliner.EnableUndo(sal_False);
    ResetLinks();
    mrOutliner.SetUpdateMode(false);

    Paragraph* pTitleToSelect = NULL;
    sal_uLong nPageCount = mrDoc.GetSdPageCount(PK_STANDARD);

    // fill outliner with paragraphs from slides title & (outlines|subtitles)
    for (sal_uInt16 nPage = 0; nPage < nPageCount; nPage++)
    {
        SdPage*     pPage = (SdPage*)mrDoc.GetSdPage(nPage, PK_STANDARD);
        Paragraph * pPara = NULL;

        // take text from title shape
        SdrTextObj* pTO = GetTitleTextObject(pPage);
        if(pTO && !(pTO->IsEmptyPresObj()))
        {
            OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
            if (pOPO)
            {
                sal_Bool bVertical = pOPO->IsVertical();
                pOPO->SetVertical( sal_False );
                mrOutliner.AddText(*pOPO);
                pOPO->SetVertical( bVertical );
                pPara = mrOutliner.GetParagraph( mrOutliner.GetParagraphCount()-1 );
            }
        }

        if( pPara == 0 ) // no title, insert an empty paragraph
        {
            pPara = mrOutliner.Insert(OUString());
            mrOutliner.SetDepth(pPara, -1);

            // do not apply hard attributes from the previous paragraph
            mrOutliner.SetParaAttribs( mrOutliner.GetAbsPos(pPara),
                                       mrOutliner.GetEmptyItemSet() );

            mrOutliner.SetStyleSheet( mrOutliner.GetAbsPos( pPara ), pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ) );
        }

        mrOutliner.SetParaFlag( pPara, PARAFLAG_ISPAGE );

        sal_Int32 nPara = mrOutliner.GetAbsPos( pPara );

        UpdateParagraph( nPara );

        // remember paragraph of currently selected page
        if (pPage->IsSelected())
            pTitleToSelect = pPara;

        // take text from subtitle or outline
        pTO = static_cast<SdrTextObj*>(pPage->GetPresObj(PRESOBJ_TEXT));
        const bool bSubTitle = pTO != 0;

        if (!pTO) // if no subtile found, try outline
            pTO = GetOutlineTextObject(pPage);

        if(pTO && !(pTO->IsEmptyPresObj())) // found some text
        {
            OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
            if (pOPO)
            {
                sal_Int32 nParaCount1 = mrOutliner.GetParagraphCount();
                sal_Bool bVertical = pOPO->IsVertical();
                pOPO->SetVertical( sal_False );
                mrOutliner.AddText(*pOPO);
                pOPO->SetVertical( bVertical );

                sal_Int32 nParaCount2 = mrOutliner.GetParagraphCount();
                for (sal_Int32 n = nParaCount1; n < nParaCount2; n++)
                {
                    if( bSubTitle )
                    {
                        Paragraph* p = mrOutliner.GetParagraph(n);
                        if(p && mrOutliner.GetDepth( n ) > 0 )
                            mrOutliner.SetDepth(p, 0);
                    }

                    UpdateParagraph( n );
                }
            }
        }
    }

    // place cursor at the start
    Paragraph* pFirstPara = mrOutliner.GetParagraph( 0 );
    mpOutlinerView[0]->Select( pFirstPara, sal_True, sal_False );
    mpOutlinerView[0]->Select( pFirstPara, sal_False, sal_False );

    // select title of slide that was selected
    if (pTitleToSelect)
        mpOutlinerView[0]->Select(pTitleToSelect, sal_True, sal_False);

    SetLinks();

    mrOutliner.EnableUndo(sal_True);

    mrOutliner.SetUpdateMode(true);
}

/**
 * Handler for deleting of level 0 paragraphs (pages): Warning
 */
IMPL_LINK_NOARG(OutlineView, RemovingPagesHdl)
{
    sal_Int32 nNumOfPages = mrOutliner.GetSelPageCount();

    if (nNumOfPages > PROCESS_WITH_PROGRESS_THRESHOLD)
    {
        mnPagesToProcess = nNumOfPages;
        mnPagesProcessed  = 0;
    }

    if (mnPagesToProcess)
    {
        if( mpProgress )
            delete mpProgress;

        mpProgress = new SfxProgress( GetDocSh(), SD_RESSTR(STR_DELETE_PAGES), mnPagesToProcess );
    }
    mrOutliner.UpdateFields();

    InvalidateSlideNumberArea();

    return 1;
}

/**
 * Handler for indenting level 0 paragraphs (pages): Warning
 */
IMPL_LINK_INLINE_START( OutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )
{
    return RemovingPagesHdl(pOutlinerView);
}
IMPL_LINK_INLINE_END( OutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )


/** returns the first slide that is selected in the outliner or where
    the cursor is located */
SdPage* OutlineView::GetActualPage()
{
    ::sd::Window* pWin = mrOutlineViewShell.GetActiveWindow();
    OutlinerView* pActiveView = GetViewByWindow(pWin);

    std::vector<Paragraph*> aSelList;
    pActiveView->CreateSelectionList(aSelList);

    Paragraph *pPar = aSelList.empty() ? NULL : *(aSelList.begin());
    SdPage* pCurrent = GetPageForParagraph(pPar);

    DBG_ASSERT( pCurrent ||
                (mpDocSh->GetUndoManager() && static_cast< sd::UndoManager *>(mpDocSh->GetUndoManager())->IsDoing()) ||
                maDragAndDropModelGuard.get(),
                "sd::OutlineView::GetActualPage(), no current page?" );

    if( pCurrent )
        return pCurrent;

    return mrDoc.GetSdPage( 0, PK_STANDARD );
}

SdPage* OutlineView::GetPageForParagraph( Paragraph* pPara )
{
    if( !mrOutliner.HasParaFlag(pPara,PARAFLAG_ISPAGE) )
        pPara = GetPrevTitle(pPara);

    sal_uInt32 nPageToSelect = 0;
    while(pPara)
    {
        pPara = GetPrevTitle(pPara);
        if(pPara)
            nPageToSelect++;
    }

    if( nPageToSelect < (sal_uInt32)mrDoc.GetSdPageCount( PK_STANDARD ) )
        return static_cast< SdPage* >( mrDoc.GetSdPage( (sal_uInt16)nPageToSelect, PK_STANDARD) );

    return 0;
}

Paragraph* OutlineView::GetParagraphForPage( ::Outliner& rOutl, SdPage* pPage )
{
    // get the number of paragraphs with ident 0 we need to skip before
    // we finde the actual page
    sal_uInt32 nPagesToSkip = (pPage->GetPageNum() - 1) >> 1;

    sal_Int32 nParaPos = 0;
    Paragraph* pPara = rOutl.GetParagraph( 0 );
    while( pPara )
    {
        // if this paragraph is a page ...
        if( mrOutliner.HasParaFlag(pPara,PARAFLAG_ISPAGE) )
        {
            // see if we already skiped enough pages
            if( 0 == nPagesToSkip )
                break;  // and if so, end the loop

            // we skiped another page
            nPagesToSkip--;
        }

        // get next paragraph
        pPara = mrOutliner.GetParagraph( ++nParaPos );
    }

    return pPara;
}

/** selects the paragraph for the given page at the outliner view*/
void OutlineView::SetActualPage( SdPage* pActual )
{
    if( pActual && dynamic_cast<Outliner&>(mrOutliner).GetIgnoreCurrentPageChangesLevel()==0 && !mbFirstPaint)
    {
        // if we found a paragraph, select its text at the outliner view
        Paragraph* pPara = GetParagraphForPage( mrOutliner, pActual );
        if( pPara )
            mpOutlinerView[0]->Select( pPara, sal_True, sal_False );
    }
}

/**
 * Get StyleSheet from the selection
 */
SfxStyleSheet* OutlineView::GetStyleSheet() const
{
     ::sd::Window* pActWin = mrOutlineViewShell.GetActiveWindow();
    OutlinerView* pOlView = GetViewByWindow(pActWin);
    SfxStyleSheet* pResult = pOlView->GetStyleSheet();
    return pResult;
}



/**
 * Mark pages as selected / not selected
 */
void OutlineView::SetSelectedPages()
{
    // list of selected title paragraphs
    std::vector<Paragraph*> aSelParas;
    mpOutlinerView[0]->CreateSelectionList(aSelParas);

    for (std::vector<Paragraph*>::iterator it = aSelParas.begin(); it != aSelParas.end();)
    {
        if (!mrOutliner.HasParaFlag(*it, PARAFLAG_ISPAGE))
            it = aSelParas.erase(it);
        else
            ++it;
    }

    // select the pages belonging to the paragraphs on level 0 to select
    sal_uInt16 nPos = 0;
    sal_Int32 nParaPos = 0;
    Paragraph *pPara = mrOutliner.GetParagraph( 0 );
    std::vector<Paragraph*>::const_iterator fiter;

    while(pPara)
    {
        if( mrOutliner.HasParaFlag(pPara, PARAFLAG_ISPAGE) )                     // one page
        {
            SdPage* pPage = mrDoc.GetSdPage(nPos, PK_STANDARD);
            DBG_ASSERT(pPage!=NULL,
                "Trying to select non-existing page OutlineView::SetSelectedPages()");

            if (pPage)
            {
                fiter = std::find(aSelParas.begin(),aSelParas.end(),pPara);
                pPage->SetSelected(fiter != aSelParas.end());
            }

            nPos++;
        }

        pPara = mrOutliner.GetParagraph( ++nParaPos );
    }
}


/**
 * Set new links
 */
void OutlineView::SetLinks()
{
    // set notification links
    mrOutliner.SetParaInsertedHdl(LINK(this, OutlineView, ParagraphInsertedHdl));
    mrOutliner.SetParaRemovingHdl(LINK(this, OutlineView, ParagraphRemovingHdl));
    mrOutliner.SetDepthChangedHdl(LINK(this, OutlineView, DepthChangedHdl));
    mrOutliner.SetBeginMovingHdl(LINK(this, OutlineView, BeginMovingHdl));
    mrOutliner.SetEndMovingHdl(LINK(this, OutlineView, EndMovingHdl));
    mrOutliner.SetRemovingPagesHdl(LINK(this, OutlineView, RemovingPagesHdl));
    mrOutliner.SetIndentingPagesHdl(LINK(this, OutlineView, IndentingPagesHdl));
    mrOutliner.SetStatusEventHdl(LINK(this, OutlineView, StatusEventHdl));
    mrOutliner.SetBeginDropHdl(LINK(this,OutlineView, BeginDropHdl));
    mrOutliner.SetEndDropHdl(LINK(this,OutlineView, EndDropHdl));
    mrOutliner.SetPaintFirstLineHdl(LINK(this,OutlineView,PaintingFirstLineHdl));
    mrOutliner.SetBeginPasteOrDropHdl(LINK(this,OutlineView, BeginPasteOrDropHdl));
    mrOutliner.SetEndPasteOrDropHdl(LINK(this,OutlineView, EndPasteOrDropHdl));
}



/**
 * Restore old links
 */
void OutlineView::ResetLinks() const
{
    Link aEmptyLink;
    mrOutliner.SetParaInsertedHdl(aEmptyLink);
    mrOutliner.SetParaRemovingHdl(aEmptyLink);
    mrOutliner.SetDepthChangedHdl(aEmptyLink);
    mrOutliner.SetBeginMovingHdl(aEmptyLink);
    mrOutliner.SetEndMovingHdl(aEmptyLink);
    mrOutliner.SetStatusEventHdl(aEmptyLink);
    mrOutliner.SetRemovingPagesHdl(aEmptyLink);
    mrOutliner.SetIndentingPagesHdl(aEmptyLink);
    mrOutliner.SetDrawPortionHdl(aEmptyLink);
    mrOutliner.SetBeginPasteOrDropHdl(aEmptyLink);
    mrOutliner.SetEndPasteOrDropHdl(aEmptyLink);
}


sal_Int8 OutlineView::AcceptDrop( const AcceptDropEvent&, DropTargetHelper&, ::sd::Window*, sal_uInt16, sal_uInt16)
{
    return DND_ACTION_NONE;
}


sal_Int8 OutlineView::ExecuteDrop( const ExecuteDropEvent&, DropTargetHelper&, ::sd::Window*, sal_uInt16, sal_uInt16)
{
    return DND_ACTION_NONE;
}

// Re-implement GetScriptType for this view to get correct results
sal_uInt16 OutlineView::GetScriptType() const
{
    sal_uInt16 nScriptType = ::sd::View::GetScriptType();

    OutlinerParaObject* pTempOPObj = mrOutliner.CreateParaObject();
    if(pTempOPObj)
    {
        nScriptType = pTempOPObj->GetTextObject().GetScriptType();
        delete pTempOPObj;
    }

    return nScriptType;
}

void OutlineView::onUpdateStyleSettings( bool bForceUpdate /* = false */ )
{
    svtools::ColorConfig aColorConfig;
    const Color aDocColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    if( bForceUpdate || (maDocColor != aDocColor) )
    {
        sal_uInt16 nView;
        for( nView = 0; nView < MAX_OUTLINERVIEWS; nView++ )
        {
            if (mpOutlinerView[nView] != NULL)
            {
                mpOutlinerView[nView]->SetBackgroundColor( aDocColor );

                ::Window* pWindow = mpOutlinerView[nView]->GetWindow();

                if( pWindow )
                    pWindow->SetBackground( Wallpaper( aDocColor ) );

            }
        }

        mrOutliner.SetBackgroundColor( aDocColor );

        maDocColor = aDocColor;
    }
}

IMPL_LINK_NOARG(OutlineView, AppEventListenerHdl)
{
    onUpdateStyleSettings();
    return 0;
}




IMPL_LINK(OutlineView, EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->meEventId)
        {
            case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
                SetActualPage(mrOutlineViewShell.GetActualPage());
                InvalidateSlideNumberArea();
                break;

            case tools::EventMultiplexerEvent::EID_PAGE_ORDER:
                if (dynamic_cast<Outliner&>(mrOutliner).GetIgnoreCurrentPageChangesLevel()==0)
                {
                    if (((mrDoc.GetPageCount()-1)%2) == 0)
                    {
                        mrOutliner.Clear();
                        FillOutliner();
                        ::sd::Window* pWindow = mrOutlineViewShell.GetActiveWindow();
                        if (pWindow != NULL)
                            pWindow->Invalidate();
                    }
                }
                break;
        }
    }
    return 0;
}

void OutlineView::IgnoreCurrentPageChanges (bool bIgnoreChanges)
{
    if (bIgnoreChanges)
        dynamic_cast<Outliner&>(mrOutliner).IncreIgnoreCurrentPageChangesLevel();
    else
        dynamic_cast<Outliner&>(mrOutliner).DecreIgnoreCurrentPageChangesLevel();
}

/** call this method before you do anything that can modify the outliner
    and or the drawing document model. It will create needed undo actions */
void OutlineView::BeginModelChange()
{
    mrOutliner.GetUndoManager().EnterListAction("", "");
    BegUndo(SD_RESSTR(STR_UNDO_CHANGE_TITLE_AND_LAYOUT));
}

/** call this method after BeginModelChange(), when all possible model
    changes are done. */
void OutlineView::EndModelChange()
{
    UpdateDocument();

    ::svl::IUndoManager* pDocUndoMgr = mpDocSh->GetUndoManager();

    bool bHasUndoActions = pDocUndoMgr->GetUndoActionCount() != 0;

    EndUndo();

    DBG_ASSERT( bHasUndoActions == (mrOutliner.GetUndoManager().GetUndoActionCount() != 0), "sd::OutlineView::EndModelChange(), undo actions not in sync!" );

    mrOutliner.GetUndoManager().LeaveListAction();

    if( bHasUndoActions && mrOutliner.GetEditEngine().HasTriedMergeOnLastAddUndo() )
        TryToMergeUndoActions();

    mrOutlineViewShell.Invalidate( SID_UNDO );
    mrOutlineViewShell.Invalidate( SID_REDO );
}

/** updates all changes in the outliner model to the draw model */
void OutlineView::UpdateDocument()
{
    const sal_uInt32 nPageCount = mrDoc.GetSdPageCount(PK_STANDARD);
    Paragraph* pPara = mrOutliner.GetParagraph( 0 );
    sal_uInt32 nPage;
    for (nPage = 0; nPage < nPageCount; nPage++)
    {
        SdPage* pPage = mrDoc.GetSdPage( (sal_uInt16)nPage, PK_STANDARD);
        mrDoc.SetSelected(pPage, sal_False);

        mrOutlineViewShell.UpdateTitleObject( pPage, pPara );
        mrOutlineViewShell.UpdateOutlineObject( pPage, pPara );

        if( pPara )
            pPara = GetNextTitle(pPara);
    }

    DBG_ASSERT( pPara == 0, "sd::OutlineView::UpdateDocument(), slides are out of sync, creating missing ones" );
    while( pPara )
    {
        SdPage* pPage = InsertSlideForParagraph( pPara );
        mrDoc.SetSelected(pPage, sal_False);

        mrOutlineViewShell.UpdateTitleObject( pPage, pPara );
        mrOutlineViewShell.UpdateOutlineObject( pPage, pPara );

        if( pPara )
            pPara = GetNextTitle(pPara);
    }
}

/** merge edit engine undo actions if possible */
void OutlineView::TryToMergeUndoActions()
{
    ::svl::IUndoManager& rOutlineUndo = mrOutliner.GetUndoManager();
    if( rOutlineUndo.GetUndoActionCount() > 1 )
    {
        SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction* >( rOutlineUndo.GetUndoAction(0) );
        SfxListUndoAction* pPrevListAction = dynamic_cast< SfxListUndoAction* >( rOutlineUndo.GetUndoAction(1) );
        if( pListAction && pPrevListAction )
        {
            // find the top EditUndo action in the top undo action list
            size_t nAction = pListAction->aUndoActions.size();
            EditUndo* pEditUndo = 0;
            while( !pEditUndo && nAction )
            {
                pEditUndo = dynamic_cast< EditUndo* >(pListAction->aUndoActions[--nAction].pAction);
            }

            sal_uInt16 nEditPos = nAction; // we need this later to remove the merged undo actions

            // make sure it is the only EditUndo action in the top undo list
            while( pEditUndo && nAction )
            {
                if( dynamic_cast< EditUndo* >(pListAction->aUndoActions[--nAction].pAction) )
                    pEditUndo = 0;
            }

            // do we have one and only one EditUndo action in the top undo list?
            if( pEditUndo )
            {
                // yes, see if we can merge it with the prev undo list

                nAction = pPrevListAction->aUndoActions.size();
                EditUndo* pPrevEditUndo = 0;
                while( !pPrevEditUndo && nAction )
                    pPrevEditUndo = dynamic_cast< EditUndo* >(pPrevListAction->aUndoActions[--nAction].pAction);

                if( pPrevEditUndo && pPrevEditUndo->Merge( pEditUndo ) )
                {
                    // ok we merged the only EditUndo of the top undo list with
                    // the top EditUndo of the previous undo list

                    // first remove the merged undo action
                    DBG_ASSERT( pListAction->aUndoActions[nEditPos].pAction == pEditUndo,
                        "sd::OutlineView::TryToMergeUndoActions(), wrong edit pos!" );
                    pListAction->aUndoActions.Remove(nEditPos);
                    delete pEditUndo;

                    // now check if we also can merge the draw undo actions
                    ::svl::IUndoManager* pDocUndoManager = mpDocSh->GetUndoManager();
                    if( pDocUndoManager && ( pListAction->aUndoActions.size() == 1 ))
                    {
                        SfxLinkUndoAction* pLinkAction = dynamic_cast< SfxLinkUndoAction* >( pListAction->aUndoActions[0].pAction );
                        SfxLinkUndoAction* pPrevLinkAction = 0;

                        if( pLinkAction )
                        {
                            nAction = pPrevListAction->aUndoActions.size();
                            while( !pPrevLinkAction && nAction )
                                pPrevLinkAction = dynamic_cast< SfxLinkUndoAction* >(pPrevListAction->aUndoActions[--nAction].pAction);
                        }

                        if( pLinkAction && pPrevLinkAction &&
                            ( pLinkAction->GetAction() == pDocUndoManager->GetUndoAction(0) ) &&
                            ( pPrevLinkAction->GetAction() == pDocUndoManager->GetUndoAction(1) ) )
                        {
                            SfxListUndoAction* pSourceList = dynamic_cast< SfxListUndoAction* >(pLinkAction->GetAction());
                            SfxListUndoAction* pDestinationList = dynamic_cast< SfxListUndoAction* >(pPrevLinkAction->GetAction());

                            if( pSourceList && pDestinationList )
                            {
                                sal_uInt16 nCount = pSourceList->aUndoActions.size();
                                sal_uInt16 nDestAction = pDestinationList->aUndoActions.size();
                                while( nCount-- )
                                {
                                    SfxUndoAction* pTemp = pSourceList->aUndoActions[0].pAction;
                                    pSourceList->aUndoActions.Remove(0);
                                    pDestinationList->aUndoActions.Insert( pTemp, nDestAction++ );
                                }
                                pDestinationList->nCurUndoAction = pDestinationList->aUndoActions.size();

                                pListAction->aUndoActions.Remove(0);
                                delete pLinkAction;

                                pDocUndoManager->RemoveLastUndoAction();
                            }
                        }
                    }

                    if ( !pListAction->aUndoActions.empty() )
                    {
                        // now we have to move all remaining doc undo actions from the top undo
                        // list to the previous undo list and remove the top undo list

                        size_t nCount = pListAction->aUndoActions.size();
                        size_t nDestAction = pPrevListAction->aUndoActions.size();
                        while( nCount-- )
                        {
                            SfxUndoAction* pTemp = pListAction->aUndoActions[0].pAction;
                            pListAction->aUndoActions.Remove(0);
                            if( pTemp )
                                pPrevListAction->aUndoActions.Insert( pTemp, nDestAction++ );
                        }
                        pPrevListAction->nCurUndoAction = pPrevListAction->aUndoActions.size();
                    }

                    rOutlineUndo.RemoveLastUndoAction();
                }
            }
        }
    }
}

IMPL_LINK(OutlineView, PaintingFirstLineHdl, PaintFirstLineInfo*, pInfo)
{
    if( pInfo )
    {
        Paragraph* pPara = mrOutliner.GetParagraph( pInfo->mnPara );
        EditEngine& rEditEngine = const_cast< EditEngine& >( mrOutliner.GetEditEngine() );

        Size aImageSize( pInfo->mpOutDev->PixelToLogic( maSlideImage.GetSizePixel()  ) );
        Size aOffset( 100, 100 );

        // paint slide number
        if( pPara && mrOutliner.HasParaFlag(pPara,PARAFLAG_ISPAGE) )
        {
            long nPage = 0; // todo, printing??
            for ( sal_Int32 n = 0; n <= pInfo->mnPara; n++ )
            {
                Paragraph* p = mrOutliner.GetParagraph( n );
                if ( mrOutliner.HasParaFlag(p,PARAFLAG_ISPAGE) )
                    nPage++;
            }

            long nBulletHeight = (long)mrOutliner.GetLineHeight( pInfo->mnPara );
            long nFontHeight = 0;
            if ( !rEditEngine.IsFlatMode() )
            {
                nFontHeight = nBulletHeight / 5;
            }
            else
            {
                nFontHeight = (nBulletHeight * 10) / 25;
            }

            Size aFontSz( 0, nFontHeight );

            Size aOutSize( 2000, nBulletHeight );

            const float fImageHeight = ((float)aOutSize.Height() * (float)4) / (float)7;
            const float fImageRatio  = (float)aImageSize.Height() / (float)aImageSize.Width();
            aImageSize.Width() = (long)( fImageRatio * fImageHeight );
            aImageSize.Height() = (long)( fImageHeight );

            Point aImagePos( pInfo->mrStartPos );
            aImagePos.X() += aOutSize.Width() - aImageSize.Width() - aOffset.Width() ;
            aImagePos.Y() += (aOutSize.Height() - aImageSize.Height()) / 2;

            pInfo->mpOutDev->DrawImage( aImagePos, aImageSize, maSlideImage );

            const bool bVertical = mrOutliner.IsVertical();
            const bool bRightToLeftPara = rEditEngine.IsRightToLeft( pInfo->mnPara );

            LanguageType eLang = rEditEngine.GetDefaultLanguage();

            Point aTextPos( aImagePos.X() - aOffset.Width(), pInfo->mrStartPos.Y() );
            Font aNewFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SANS_UNICODE, eLang, 0 ) );
            aNewFont.SetSize( aFontSz );
            aNewFont.SetVertical( bVertical );
            aNewFont.SetOrientation( bVertical ? 2700 : 0 );
            aNewFont.SetColor( COL_AUTO );
            pInfo->mpOutDev->SetFont( aNewFont );
            OUString aPageText = OUString::number( nPage );
            Size aTextSz;
            aTextSz.Width() = pInfo->mpOutDev->GetTextWidth( aPageText );
            aTextSz.Height() = pInfo->mpOutDev->GetTextHeight();
            if ( !bVertical )
            {
                aTextPos.Y() += (aOutSize.Height() - aTextSz.Height()) / 2;
                if ( !bRightToLeftPara )
                {
                    aTextPos.X() -= aTextSz.Width();
                }
                else
                {
                    aTextPos.X() += aTextSz.Width();
                }
            }
            else
            {
                aTextPos.Y() -= aTextSz.Width();
                aTextPos.X() += nBulletHeight / 2;
            }
            pInfo->mpOutDev->DrawText( aTextPos, aPageText );
        }
    }

    return 0;
}

// --------------------------------------------------------------------

void OutlineView::UpdateParagraph( sal_Int32 nPara )
{
    SfxItemSet aNewAttrs2( mrOutliner.GetParaAttribs( nPara ) );
    aNewAttrs2.Put( maLRSpaceItem );
    mrOutliner.SetParaAttribs( nPara, aNewAttrs2 );
}

// --------------------------------------------------------------------

void OutlineView::OnBeginPasteOrDrop( PasteOrDropInfos* /*pInfos*/ )
{
}

/** this is called after a paste or drop operation, make sure that the newly inserted paragraphs
    get the correct style sheet and new slides are inserted. */
void OutlineView::OnEndPasteOrDrop( PasteOrDropInfos* pInfos )
{
    SdPage* pPage = 0;
    SfxStyleSheetBasePool* pStylePool = GetDoc().GetStyleSheetPool();

    for( sal_Int32 nPara = pInfos->nStartPara; nPara <= pInfos->nEndPara; nPara++ )
    {
        Paragraph* pPara = mrOutliner.GetParagraph( nPara );

        bool bPage = mrOutliner.HasParaFlag( pPara, PARAFLAG_ISPAGE  );

        if( !bPage )
        {
            SdStyleSheet* pStyleSheet = dynamic_cast< SdStyleSheet* >( mrOutliner.GetStyleSheet( nPara ) );
            if( pStyleSheet )
            {
                const OUString aName( pStyleSheet->GetApiName() );
                if ( aName == "title" )
                    bPage = true;
            }
        }

        if( !pPara )
            continue; // fatality!?

        if( bPage && (nPara != pInfos->nStartPara) )
        {
            // insert new slide for this paragraph
            pPage = InsertSlideForParagraph( pPara );
        }
        else
        {
            // newly inserted non page paragraphs get the outline style
            if( !pPage )
                pPage = GetPageForParagraph( pPara );

            if( pPage )
            {
                SfxStyleSheet* pStyle = pPage->GetStyleSheetForPresObj( bPage ? PRESOBJ_TITLE : PRESOBJ_OUTLINE );

                if( !bPage )
                {
                    const sal_Int16 nDepth = mrOutliner.GetDepth( nPara );
                    if( nDepth > 0 )
                    {
                        OUString aStyleSheetName = pStyle->GetName();
                        if (!aStyleSheetName.isEmpty())
                            aStyleSheetName = aStyleSheetName.copy(0, aStyleSheetName.getLength() - 1);
                        aStyleSheetName += OUString::number( nDepth );
                        pStyle = static_cast<SfxStyleSheet*>( pStylePool->Find( aStyleSheetName, pStyle->GetFamily() ) );
                        DBG_ASSERT( pStyle, "sd::OutlineView::OnEndPasteOrDrop(), Style not found!" );
                    }
                }

                mrOutliner.SetStyleSheet( nPara, pStyle );
            }

            UpdateParagraph( nPara );
        }
    }
}

// ====================================================================


OutlineViewModelChangeGuard::OutlineViewModelChangeGuard( OutlineView& rView )
: mrView( rView )
{
    mrView.BeginModelChange();
}

OutlineViewModelChangeGuard::~OutlineViewModelChangeGuard()
{
    mrView.EndModelChange();
}

OutlineViewPageChangesGuard::OutlineViewPageChangesGuard( OutlineView* pView )
: mpView( pView )
{
    if( mpView )
        mpView->IgnoreCurrentPageChanges( true );
}

OutlineViewPageChangesGuard::~OutlineViewPageChangesGuard()
{
    if( mpView )
        mpView->IgnoreCurrentPageChanges( false );
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
