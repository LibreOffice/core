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

#include "scitems.hxx"
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/srchitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/svdview.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>

#include "prevwsh.hxx"
#include "preview.hxx"
#include "printfun.hxx"
#include "attrib.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "ViewSettingsSequenceDefines.hxx"
#include "tpprint.hxx"
#include "printopt.hxx"
#include "viewuno.hxx"
#include <sax/tools/converter.hxx>
#include <rtl/ustrbuf.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <basegfx/tools/zoomtools.hxx>
#include <svx/zoom_def.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "scabstdlg.hxx"
//  for mouse wheel
#define MINZOOM_SLIDER 10
#define MAXZOOM_SLIDER 400

#define SC_USERDATA_SEP ';'

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define ScPreviewShell
#include "scslots.hxx"

TYPEINIT1( ScPreviewShell, SfxViewShell );

SFX_IMPL_INTERFACE( ScPreviewShell, SfxViewShell, ScResId(SCSTR_PREVIEWSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|
                                SFX_VISIBILITY_SERVER|SFX_VISIBILITY_READONLYDOC,
                                ScResId(RID_OBJECTBAR_PREVIEW));
    SFX_POPUPMENU_REGISTRATION(ScResId(RID_POPUP_PREVIEW));
}

SFX_IMPL_NAMED_VIEWFACTORY( ScPreviewShell, "PrintPreview" )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

//------------------------------------------------------------------

void ScPreviewShell::Construct( Window* pParent )
{
    // Find the top-most window, and set the close window handler to intercept
    // the window close event.
    Window* pWin = pParent;
    while (!pWin->IsSystemWindow())
    {
        if (pWin->GetParent())
            pWin = pWin->GetParent();
        else
            break;
    }

    mpFrameWindow = dynamic_cast<SystemWindow*>(pWin);
    if (mpFrameWindow)
        mpFrameWindow->SetCloseHdl(LINK(this, ScPreviewShell, CloseHdl));

    eZoom = SVX_ZOOM_WHOLEPAGE;

    pCorner = new ScrollBarBox( pParent, WB_SIZEABLE );

    pHorScroll = new ScrollBar(pParent, WB_HSCROLL );
    pVerScroll = new ScrollBar(pParent, WB_VSCROLL);

    // SSA: --- RTL --- no mirroring for horizontal scrollbars
    pHorScroll->EnableRTL( false );

    pHorScroll->SetEndScrollHdl( LINK( this, ScPreviewShell, ScrollHandler ) );
    pVerScroll->SetEndScrollHdl( LINK( this, ScPreviewShell, ScrollHandler ) );

    pPreview = new ScPreview( pParent, pDocShell, this );

    SetPool( &SC_MOD()->GetPool() );
    SetWindow( pPreview );
    StartListening(*pDocShell,sal_True);
    StartListening(*SFX_APP(),sal_True);        // #i62045# #i62046# application is needed for Calc's own hints
    SfxBroadcaster* pDrawBC = pDocShell->GetDocument()->GetDrawBroadcaster();
    if (pDrawBC)
        StartListening(*pDrawBC);

    pHorScroll->Show( false );
    pVerScroll->Show( false );
    pCorner->Show();
    SetHelpId( HID_SCSHELL_PREVWSH );
    SetName(OUString("Preview"));
}

ScPreviewShell::ScPreviewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    SfxViewShell( pViewFrame, SFX_VIEW_CAN_PRINT | SFX_VIEW_HAS_PRINTOPTIONS ),
    pDocShell( (ScDocShell*)pViewFrame->GetObjectShell() ),
    mpFrameWindow(NULL),
    nSourceDesignMode( SC_FORCEMODE_NONE ),
    pAccessibilityBroadcaster( NULL )
{
    Construct( &pViewFrame->GetWindow() );

    if ( pOldSh && pOldSh->ISA( ScTabViewShell ) )
    {
        //  store view settings, show table from TabView
        //! store live ScViewData instead, and update on ScTablesHint?
        //! or completely forget aSourceData on ScTablesHint?

        ScTabViewShell* pTabViewShell = ((ScTabViewShell*)pOldSh);
        const ScViewData* pData = pTabViewShell->GetViewData();
        pData->WriteUserDataSequence( aSourceData );
        pPreview->SetSelectedTabs(pData->GetMarkData());
        InitStartTable( pData->GetTabNo() );

        //  also have to store the TabView's DesignMode state
        //  (only if draw view exists)
        SdrView* pDrawView = pTabViewShell->GetSdrView();
        if ( pDrawView )
            nSourceDesignMode = pDrawView->IsDesignMode();
    }

    new ScPreviewObj(this);
}

ScPreviewShell::~ScPreviewShell()
{
    if (mpFrameWindow)
        mpFrameWindow->SetCloseHdl(Link()); // Remove close handler.

    // #108333#; notify Accessibility that Shell is dying and before destroy all
    BroadcastAccessibility( SfxSimpleHint( SFX_HINT_DYING ) );
    DELETEZ(pAccessibilityBroadcaster);

    SfxBroadcaster* pDrawBC = pDocShell->GetDocument()->GetDrawBroadcaster();
    if (pDrawBC)
        EndListening(*pDrawBC);
    EndListening(*SFX_APP());
    EndListening(*pDocShell);

    SetWindow(0);
    delete pPreview;
    delete pHorScroll;
    delete pVerScroll;
    delete pCorner;

    //  normal mode of operation is switching back to default view in the same frame,
    //  so there's no need to activate any other window here anymore
}

void ScPreviewShell::InitStartTable(SCTAB nTab)
{
    pPreview->SetPageNo( pPreview->GetFirstPage(nTab) );
}

//------------------------------------------------------------------

String ScPreviewShell::GetDescription() const
{
    return OUString(" ** Test ** ");
}

void ScPreviewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    Size aOutSize( rSize );
    pPreview->SetPosSizePixel( rPos, aOutSize );

    if ( SVX_ZOOM_WHOLEPAGE == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(false) );
    else if ( SVX_ZOOM_PAGEWIDTH == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(true) );

    UpdateNeededScrollBars();
}

void ScPreviewShell::InnerResizePixel( const Point &rOfs, const Size &rSize )
{
    AdjustPosSizePixel( rOfs,rSize );
}

void ScPreviewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    AdjustPosSizePixel( rOfs,rSize );
}

bool ScPreviewShell::GetPageSize( Size& aPageSize )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();

    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ),
                                                        SFX_STYLE_FAMILY_PAGE );
    OSL_ENSURE(pStyleSheet,"No style sheet");
    if (!pStyleSheet) return false;
    const SfxItemSet* pParamSet = &pStyleSheet->GetItemSet();

    aPageSize = ((const SvxSizeItem&) pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
    aPageSize.Width()  = (long) (aPageSize.Width()  * HMM_PER_TWIPS );
    aPageSize.Height() = (long) (aPageSize.Height() * HMM_PER_TWIPS );
    return true;
}

void ScPreviewShell::UpdateNeededScrollBars( bool bFromZoom )
{
    Size aPageSize;
    OutputDevice* pDevice = Application::GetDefaultDevice();

    long nBarW = GetViewFrame()->GetWindow().GetSettings().GetStyleSettings().GetScrollBarSize();
    long nBarH = nBarW;

    long aHeightOffSet = pDevice ? pDevice->PixelToLogic( Size( nBarW, nBarH ), pPreview->GetMapMode() ).Height() : 0;
    long aWidthOffSet = aHeightOffSet;


    if (!GetPageSize( aPageSize ))
        return;

    //  for centering, page size without the shadow is used
    bool bVert = pVerScroll ? pVerScroll->IsVisible() : false;
    bool bHori = pHorScroll ? pHorScroll->IsVisible() : false;
    Size aWindowSize = pPreview->GetOutputSize();
    Point aPos = pPreview->GetPosPixel();
    Size aWindowPixelSize = pPreview->GetOutputSizePixel();

    // if we are called from Zoom then we need to compensate for whatever
    // scrollbars were displayed before the zoom was called
    if ( bFromZoom )
    {
        if ( bVert )
        {
            aWindowPixelSize.Width() += nBarH;
            aWindowSize.Width() += aHeightOffSet;
        }
        if ( bHori )
        {
            aWindowPixelSize.Height() += nBarW;
            aWindowSize.Height() += aWidthOffSet;
        }
    }
    // recalculate any needed scrollbars
    bHori = false;
    bVert = false;

    long nMaxWidthPos = aPageSize.Width() - aWindowSize.Width();
    if ( nMaxWidthPos<0 )
        bHori = false;
    else
        bHori = true;

    long nMaxHeightPos = aPageSize.Height() - aWindowSize.Height();

    if ( nMaxHeightPos < 0 )
        bVert = false;
    else
        bVert = true;

    // see if having a scroll bar requires the other
    if ( bVert != bHori && ( bVert || bHori ) )
    {
        if ( bVert && ( (nMaxWidthPos + aWidthOffSet  ) > 0 ) )
            bHori = true;
        else if ( (nMaxHeightPos + aHeightOffSet ) > 0 )
            bVert = true;
    }
    pHorScroll->Show( bHori );
    pVerScroll->Show( bVert );

    // make room for needed scrollbars ( and reduce the size
    // of the preview appropriately )
    if ( bHori )
        aWindowPixelSize.Height() -= nBarW;
    if ( bVert )
        aWindowPixelSize.Width() -= nBarH;

    pPreview->SetSizePixel( aWindowPixelSize );
    pHorScroll->SetPosSizePixel( Point( aPos.X(), aPos.Y() + aWindowPixelSize.Height() ),
                                 Size( aWindowPixelSize.Width(), nBarH ) );
    pVerScroll->SetPosSizePixel( Point( aPos.X() + aWindowPixelSize.Width(), aPos.Y() ),
                                 Size( nBarW, aWindowPixelSize.Height() ) );
    pCorner->SetPosSizePixel( Point( aPos.X() + aWindowPixelSize.Width(), aPos.Y() + aWindowPixelSize.Height() ),
                              Size( nBarW, nBarH ) );
    UpdateScrollBars();
}

void ScPreviewShell::UpdateScrollBars()
{
    Size aPageSize;
    if ( !GetPageSize( aPageSize ) )
        return;

    //  for centering, page size without the shadow is used

    Size aWindowSize = pPreview->GetOutputSize();

    Point aOfs = pPreview->GetOffset();

    if( pHorScroll )
    {
        pHorScroll->SetRange( Range( 0, aPageSize.Width() ) );
        pHorScroll->SetLineSize( aWindowSize.Width() / 16 );
        pHorScroll->SetPageSize( aWindowSize.Width() );
        pHorScroll->SetVisibleSize( aWindowSize.Width() );
        long nMaxPos = aPageSize.Width() - aWindowSize.Width();
        if ( nMaxPos<0 )
        {
            //  page smaller than window -> center (but put scrollbar to 0)
            aOfs.X() = 0;
            pPreview->SetXOffset( nMaxPos / 2 );
        }
        else if (aOfs.X() < 0)
        {
            //  page larger than window -> never use negative offset
            aOfs.X() = 0;
            pPreview->SetXOffset( 0 );
        }
        else if (aOfs.X() > nMaxPos)
        {
            //  limit offset to align with right edge of window
            aOfs.X() = nMaxPos;
            pPreview->SetXOffset(nMaxPos);
        }
        pHorScroll->SetThumbPos( aOfs.X() );
    }

    if( pVerScroll )
    {
        long nPageNo     = pPreview->GetPageNo();
        long nTotalPages = pPreview->GetTotalPages();

        nMaxVertPos = aPageSize.Height() - aWindowSize.Height();
        pVerScroll->SetLineSize( aWindowSize.Height() / 16  );
        pVerScroll->SetPageSize( aWindowSize.Height() );
        pVerScroll->SetVisibleSize( aWindowSize.Height() );
        if ( nMaxVertPos < 0 )
        {
            //  page smaller than window -> center (but put scrollbar to 0)
            aOfs.Y() = 0;
            pPreview->SetYOffset( nMaxVertPos / 2 );
            pVerScroll->SetThumbPos( nPageNo * aWindowSize.Height() );
            pVerScroll->SetRange( Range( 0, aWindowSize.Height() * nTotalPages ));
        }
        else if (aOfs.Y() < 0)
        {
            //  page larger than window -> never use negative offset
            pVerScroll->SetRange( Range( 0, aPageSize.Height() ) );
            aOfs.Y() = 0;
            pPreview->SetYOffset( 0 );
            pVerScroll->SetThumbPos( aOfs.Y() );
        }
        else if (aOfs.Y() > nMaxVertPos )
        {
            //  limit offset to align with window bottom
            pVerScroll->SetRange( Range( 0, aPageSize.Height() ) );
            aOfs.Y() = nMaxVertPos;
            pPreview->SetYOffset( nMaxVertPos );
            pVerScroll->SetThumbPos( aOfs.Y() );
        }
    }
}

IMPL_LINK (ScPreviewShell,ScrollHandler, ScrollBar* ,pScroll )
{
    long nPos           = pScroll->GetThumbPos();
    long nDelta         = pScroll->GetDelta();
    long nMaxRange      = pScroll->GetRangeMax();
    long nTotalPages    = pPreview->GetTotalPages();
    long nPageNo        = 0;
    long nPerPageLength = 0;
    sal_Bool bIsDivide      = sal_True;

    if( nTotalPages )
        nPerPageLength = nMaxRange / nTotalPages;

    if( nPerPageLength )
    {
        nPageNo = nPos / nPerPageLength;
        if( nPos % nPerPageLength )
        {
            bIsDivide = false;
            nPageNo ++;
        }
    }

    sal_Bool bHoriz = ( pScroll == pHorScroll );

    if( bHoriz )
        pPreview->SetXOffset( nPos );
    else
    {
        if( nMaxVertPos > 0 )
            pPreview->SetYOffset( nPos );
        else
        {
            Point  aMousePos = pScroll->OutputToNormalizedScreenPixel( pScroll->GetPointerPosPixel() );
            Point  aPos      = pScroll->GetParent()->OutputToNormalizedScreenPixel( pScroll->GetPosPixel() );
            OUString aHelpStr;
            Rectangle aRect;
            sal_uInt16   nAlign;

            if( nDelta < 0 )
            {
                if ( nTotalPages && nPageNo > 0 && !bIsDivide )
                    pPreview->SetPageNo( nPageNo-1 );
                if( bIsDivide )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScGlobal::GetRscString( STR_PAGE ) +
                           " " + OUString::number( nPageNo ) +
                           " / "  + OUString::number( nTotalPages );
            }
            else if( nDelta > 0 )
            {
                sal_Bool bAllTested = pPreview->AllTested();
                if ( nTotalPages && ( nPageNo < nTotalPages || !bAllTested ) )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScGlobal::GetRscString( STR_PAGE ) +
                           " " + OUString::number( nPageNo+1 ) +
                           " / "  + OUString::number( nTotalPages );
            }

            aRect.Left()    = aPos.X() - 8;
            aRect.Top()     = aMousePos.Y();
            aRect.Right()   = aRect.Left();
            aRect.Top()     = aRect.Top();
            nAlign          = QUICKHELP_BOTTOM|QUICKHELP_CENTER;
            Help::ShowQuickHelp( pScroll->GetParent(), aRect, aHelpStr, nAlign );
        }
    }

    return 0;
}

IMPL_LINK (ScPreviewShell, CloseHdl, SystemWindow*, EMPTYARG)
{
    ExitPreview();
    return 0;
}

sal_Bool ScPreviewShell::ScrollCommand( const CommandEvent& rCEvt )
{
    sal_Bool bDone = false;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if ( pData && pData->GetMode() == COMMAND_WHEEL_ZOOM )
    {
        long nOld = pPreview->GetZoom();
        long nNew = nOld;
        if ( pData->GetDelta() < 0 )
            nNew = std::max( (long) MINZOOM, basegfx::zoomtools::zoomOut( nOld ));
        else
            nNew = std::min( (long) MAXZOOM, basegfx::zoomtools::zoomIn( nOld ));

        if ( nNew != nOld )
        {
            eZoom = SVX_ZOOM_PERCENT;
            pPreview->SetZoom( (sal_uInt16)nNew );
        }

        bDone = sal_True;
    }
    else
    {
        bDone = pPreview->HandleScrollCommand( rCEvt, pHorScroll, pVerScroll );
    }

    return bDone;
}

SfxPrinter* ScPreviewShell::GetPrinter( sal_Bool bCreate )
{
    return pDocShell->GetPrinter(bCreate);
}

sal_uInt16 ScPreviewShell::SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    return pDocShell->SetPrinter( pNewPrinter, nDiffFlags );
}

bool ScPreviewShell::HasPrintOptionsPage() const
{
    return true;
}

SfxTabPage* ScPreviewShell::CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions )
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
    ::CreateTabPage ScTpPrintOptionsCreate =    pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
    if ( ScTpPrintOptionsCreate )
        return  (*ScTpPrintOptionsCreate)( pParent, rOptions);
    return 0;
}

void ScPreviewShell::Activate(sal_Bool bMDI)
{
    SfxViewShell::Activate(bMDI);

    //! Basic etc. -> auslagern in eigene Datei (s. tabvwsh4)

    if (bMDI)
    {
        // InputHdl ist jetzt meistens Null, keine Assertion mehr!
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl )
            pInputHdl->NotifyChange( NULL );
    }
}

void ScPreviewShell::Deactivate(sal_Bool bMDI)
{
    SfxViewShell::Deactivate(bMDI);

    if (bMDI)
    {
    }
}

//------------------------------------------------------------------------

void ScPreviewShell::Execute( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    switch ( nSlot )
    {
        case SID_FORMATPAGE:
        case SID_STATUS_PAGESTYLE:
        case SID_HFEDIT:
            pDocShell->ExecutePageStyle( *this, rReq, pPreview->GetTab() );
            break;
        case SID_REPAINT:
            pPreview->Invalidate();
            rReq.Done();
            break;
        case SID_PREV_TABLE: // Accelerator
        case SID_PREVIEW_PREVIOUS:
            {
                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage > 0)
                    pPreview->SetPageNo( nPage-1 );
            }
            break;
        case SID_NEXT_TABLE: // Accelerator
        case SID_PREVIEW_NEXT:
            {
                sal_Bool bAllTested = pPreview->AllTested();
                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if (nTotal && (nPage+1 < nTotal || !bAllTested))
                    pPreview->SetPageNo( nPage+1 );
            }
            break;
        case SID_CURSORTOPOFFILE: // Accelerator
        case SID_PREVIEW_FIRST:
            {
                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage != 0)
                    pPreview->SetPageNo( 0 );
            }
            break;
        case SID_CURSORENDOFFILE: // Accelerator
        case SID_PREVIEW_LAST:
            {
                if (!pPreview->AllTested())
                    pPreview->CalcAll();

                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage+1 != nTotal)
                    pPreview->SetPageNo( nTotal-1 );
            }
            break;
        case SID_ATTR_ZOOM:
        case FID_SCALE:
            {
                sal_uInt16      nZoom       = 100;
                sal_Bool        bCancel     = false;

                eZoom = SVX_ZOOM_PERCENT;

                if ( pReqArgs )
                {

                    const SvxZoomItem& rZoomItem = (const SvxZoomItem&)
                                                   pReqArgs->Get(SID_ATTR_ZOOM);

                    eZoom = rZoomItem.GetType();
                    nZoom = rZoomItem.GetValue();
                }
                else
                {
                    SfxItemSet      aSet     ( GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
                    SvxZoomItem     aZoomItem( SVX_ZOOM_PERCENT, pPreview->GetZoom(), SID_ATTR_ZOOM );

                    aSet.Put( aZoomItem );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        AbstractSvxZoomDialog* pDlg = pFact->CreateSvxZoomDialog(NULL, aSet);
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");
                        pDlg->SetLimits( 20, 400 );
                        pDlg->HideButton( ZOOMBTN_OPTIMAL );
                        bCancel = ( RET_CANCEL == pDlg->Execute() );

                        if ( !bCancel )
                        {
                            const SvxZoomItem&  rZoomItem = (const SvxZoomItem&)
                                                    pDlg->GetOutputItemSet()->
                                                        Get( SID_ATTR_ZOOM );

                            eZoom = rZoomItem.GetType();
                            nZoom = rZoomItem.GetValue();
                        }

                        delete pDlg;
                    }
                }

                if ( !bCancel )
                {
                    switch ( eZoom )
                    {
                        case SVX_ZOOM_OPTIMAL:
                        case SVX_ZOOM_WHOLEPAGE:
                            nZoom = pPreview->GetOptimalZoom(false);
                            break;
                        case SVX_ZOOM_PAGEWIDTH:
                            nZoom = pPreview->GetOptimalZoom(sal_True);
                            break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }

                    pPreview->SetZoom( nZoom );
                    rReq.Done();
                }
            }
            break;
        case SID_PREVIEW_ZOOMIN:
            {
                sal_uInt16 nNew = pPreview->GetZoom() + 20 ;
                nNew -= nNew % 20;
                pPreview->SetZoom( nNew );
                eZoom = SVX_ZOOM_PERCENT;
                rReq.Done();
            }
            break;
        case SID_PREVIEW_ZOOMOUT:
            {
                sal_uInt16 nNew = pPreview->GetZoom() - 1;
                nNew -= nNew % 20;
                pPreview->SetZoom( nNew );
                eZoom = SVX_ZOOM_PERCENT;
                rReq.Done();
            }
            break;
        case SID_PREVIEW_MARGIN:
            {
                sal_Bool bMargin = pPreview->GetPageMargins();
                pPreview->SetPageMargins( !bMargin );
                pPreview->Invalidate();
                rReq.Done();
            }
            break;
        case SID_ATTR_ZOOMSLIDER:
            {
                const SfxPoolItem* pItem;
                eZoom = SVX_ZOOM_PERCENT;
                if( pReqArgs && SFX_ITEM_SET == pReqArgs->GetItemState( SID_ATTR_ZOOMSLIDER, sal_True, &pItem ) )
                {
                    const sal_uInt16 nCurrentZoom = ((const SvxZoomSliderItem*)pItem)->GetValue();
                    if( nCurrentZoom )
                    {
                        pPreview->SetZoom( nCurrentZoom );
                        rReq.Done();
                    }
                }
            }
            break;
        case SID_PREVIEW_SCALINGFACTOR:
            {
                const SfxPoolItem* pItem;
                SCTAB nTab                      = pPreview->GetTab();
                String aOldName                 = pDocShell->GetDocument()->GetPageStyle( pPreview->GetTab() );
                ScStyleSheetPool* pStylePool    = pDocShell->GetDocument()->GetStyleSheetPool();
                SfxStyleSheetBase* pStyleSheet  = pStylePool->Find( aOldName, SFX_STYLE_FAMILY_PAGE );
                OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                if ( pReqArgs && pStyleSheet && SFX_ITEM_SET == pReqArgs->GetItemState( SID_PREVIEW_SCALINGFACTOR, sal_True, &pItem ) )
                {
                    const sal_uInt16 nCurrentZoom   = ((const SvxZoomSliderItem *)pItem)->GetValue();
                    SfxItemSet& rSet            = pStyleSheet->GetItemSet();
                    rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nCurrentZoom ) );
                    ScPrintFunc aPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                    rReq.Done();
                }
                GetViewFrame()->GetBindings().Invalidate( nSlot );
            }
            break;
        case SID_PRINTPREVIEW:
        case SID_PREVIEW_CLOSE:
            //  print preview is now always in the same frame as the tab view
            //  -> always switch this frame back to normal view
            //  (ScTabViewShell ctor reads stored view data)

            ExitPreview();
            break;
        case SID_CURSORPAGEUP:
        case SID_CURSORPAGEDOWN:
        case SID_CURSORHOME:
        case SID_CURSOREND:
        case SID_CURSORUP:
        case SID_CURSORDOWN:
        case SID_CURSORLEFT:
        case SID_CURSORRIGHT:
            DoScroll( nSlot );
            break;
        case SID_CANCEL:
            if( ScViewUtil::IsFullScreen( *this ) )
                ScViewUtil::SetFullScreen( *this, false );
            break;

        default:
            break;
    }
}

void ScPreviewShell::GetState( SfxItemSet& rSet )
{
    pPreview->SetInGetState(sal_True);

    SCTAB nTab      = pPreview->GetTab();
    long nPage      = pPreview->GetPageNo();
    long nTotal     = pPreview->GetTotalPages();
    sal_uInt16 nZoom    = pPreview->GetZoom();
    sal_Bool bAllTested = pPreview->AllTested();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_STATUS_PAGESTYLE:
            case SID_HFEDIT:
                pDocShell->GetStatePageStyle( *this, rSet, nTab );
                break;
            case SID_UNDO:
            case SID_REDO:
            case SID_REPEAT:
            case SID_SAVEDOC:
            case SID_SAVEASDOC:
            case SID_MAIL_SENDDOC:
            case SID_VIEW_DATA_SOURCE_BROWSER:
            case SID_QUITAPP:
                rSet.DisableItem(nWhich);
                break;
            case SID_PREVIEW_PREVIOUS:
            case SID_PREVIEW_FIRST:
                if (!nTotal || nPage==0)
                    rSet.DisableItem(nWhich);
                break;
            case SID_PREVIEW_NEXT:
            case SID_PREVIEW_LAST:
                if (bAllTested)
                    if (!nTotal || nPage==nTotal-1)
                        rSet.DisableItem(nWhich);
                break;
            case SID_PREVIEW_ZOOMIN:
                if (nZoom >= 400)
                    rSet.DisableItem(nWhich);
                break;
            case SID_PREVIEW_ZOOMOUT:
                if (nZoom <= 20)
                    rSet.DisableItem(nWhich);
                break;
            case SID_ATTR_ZOOM:
                {
                    SvxZoomItem aZoom( eZoom, nZoom, nWhich );
                    aZoom.SetValueSet( SVX_ZOOM_ENABLE_ALL & ~SVX_ZOOM_ENABLE_OPTIMAL );
                    rSet.Put( aZoom );
                }
                break;
            case SID_ATTR_ZOOMSLIDER:
                {
                    SvxZoomSliderItem aZoomSliderItem( nZoom, MINZOOM, MAXZOOM, SID_ATTR_ZOOMSLIDER );
                    aZoomSliderItem.AddSnappingPoint( 100 );
                    rSet.Put( aZoomSliderItem );
                }
                break;
            case SID_PREVIEW_SCALINGFACTOR:
                {
                    if( pDocShell->IsReadOnly() )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        String aOldName                 = pDocShell->GetDocument()->GetPageStyle( pPreview->GetTab() );
                        ScStyleSheetPool* pStylePool    = pDocShell->GetDocument()->GetStyleSheetPool();
                        SfxStyleSheetBase* pStyleSheet  = pStylePool->Find( aOldName, SFX_STYLE_FAMILY_PAGE );
                        OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                        if ( pStyleSheet )
                        {
                            SfxItemSet& rStyleSet   = pStyleSheet->GetItemSet();
                            sal_uInt16 nCurrentZoom     = ((const SfxUInt16Item&)rStyleSet.Get(ATTR_PAGE_SCALE)).GetValue();
                            if( nCurrentZoom )
                            {
                                SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM_SLIDER, MAXZOOM_SLIDER, SID_PREVIEW_SCALINGFACTOR );
                                aZoomSliderItem.AddSnappingPoint( 100 );
                                rSet.Put( aZoomSliderItem );
                            }
                            else
                                rSet.DisableItem( nWhich );
                        }
                    }
                }
                break;
            case SID_STATUS_DOCPOS:
                rSet.Put( SfxStringItem( nWhich, pPreview->GetPosString() ) );
                break;
            case SID_PRINTPREVIEW:
                rSet.Put( SfxBoolItem( nWhich, sal_True ) );
                break;
            case SID_FORMATPAGE:
            case SID_PREVIEW_MARGIN:
                if( pDocShell->IsReadOnly() )
                    rSet.DisableItem( nWhich );
                break;
        }

        nWhich = aIter.NextWhich();
    }

    pPreview->SetInGetState(false);
}

void ScPreviewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();
    OUString aTmp;
    pDoc->GetName(nTab, aTmp);
    rData.aTabName = aTmp;

    if( pDocShell->getDocProperties()->getTitle().getLength() != 0 )
        rData.aTitle = pDocShell->getDocProperties()->getTitle();
    else
        rData.aTitle = pDocShell->GetTitle();

    const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
    rData.aLongDocName  = rURLObj.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
    if ( rData.aLongDocName.Len() )
        rData.aShortDocName = rURLObj.GetName( INetURLObject::DECODE_UNAMBIGUOUS );
    else
        rData.aShortDocName = rData.aLongDocName = rData.aTitle;
    rData.nPageNo       = pPreview->GetPageNo() + 1;

    sal_Bool bAllTested = pPreview->AllTested();
    if (bAllTested)
        rData.nTotalPages = pPreview->GetTotalPages();
    else
        rData.nTotalPages = 99;

    //  the dialog knows eNumType
}

void ScPreviewShell::WriteUserData(OUString& rData, bool /* bBrowse */)
{
    //  nZoom
    //  nPageNo

    rData =  OUString::number(pPreview->GetZoom());
    rData += OUString(SC_USERDATA_SEP);
    rData += OUString::number(pPreview->GetPageNo());
}

void ScPreviewShell::ReadUserData(const OUString& rData, bool /* bBrowse */)
{
    if (!rData.isEmpty())
    {
        sal_Int32 nIndex = 0;
        pPreview->SetZoom((sal_uInt16)rData.getToken(0, SC_USERDATA_SEP, nIndex).toInt32());
        pPreview->SetPageNo(rData.getToken(0, SC_USERDATA_SEP, nIndex).toInt32());
        eZoom = SVX_ZOOM_PERCENT;
    }
}

void ScPreviewShell::WriteUserDataSequence(uno::Sequence < beans::PropertyValue >& rSeq, sal_Bool /* bBrowse */)
{
    rSeq.realloc(3);
    beans::PropertyValue* pSeq = rSeq.getArray();
    if(pSeq)
    {
        sal_uInt16 nViewID(GetViewFrame()->GetCurViewId());
        pSeq[0].Name = OUString(SC_VIEWID);
        OUStringBuffer sBuffer(OUString(SC_VIEW));
        ::sax::Converter::convertNumber(sBuffer,
                static_cast<sal_Int32>(nViewID));
        pSeq[0].Value <<= sBuffer.makeStringAndClear();
        pSeq[1].Name = OUString(SC_ZOOMVALUE);
        pSeq[1].Value <<= sal_Int32 (pPreview->GetZoom());
        pSeq[2].Name = OUString("PageNumber");
        pSeq[2].Value <<= pPreview->GetPageNo();
    }
}

void ScPreviewShell::ReadUserDataSequence(const uno::Sequence < beans::PropertyValue >& rSeq, sal_Bool /* bBrowse */)
{
    sal_Int32 nCount(rSeq.getLength());
    if (nCount)
    {
        const beans::PropertyValue* pSeq = rSeq.getConstArray();
        if(pSeq)
        {
            for(sal_Int32 i = 0; i < nCount; i++, pSeq++)
            {
                OUString sName(pSeq->Name);
                if(sName.compareToAscii(SC_ZOOMVALUE) == 0)
                {
                    sal_Int32 nTemp = 0;
                    if (pSeq->Value >>= nTemp)
                        pPreview->SetZoom(sal_uInt16(nTemp));
                }
                else if (sName.compareToAscii("PageNumber") == 0)
                {
                    sal_Int32 nTemp = 0;
                    if (pSeq->Value >>= nTemp)
                        pPreview->SetPageNo(nTemp);
                }
            }
        }
    }
}

void ScPreviewShell::DoScroll( sal_uInt16 nMode )
{
    Point   aCurPos, aPrevPos;

    long nHRange    = pHorScroll->GetRange().Max();
    long nHLine     = pHorScroll->GetLineSize();
    long nHPage     = pHorScroll->GetPageSize();
    long nVRange    = pVerScroll->GetRange().Max();
    long nVLine     = pVerScroll->GetLineSize();
    long nVPage     = pVerScroll->GetPageSize();

    aCurPos.X() = pHorScroll->GetThumbPos();
    aCurPos.Y() = pVerScroll->GetThumbPos();
    aPrevPos = aCurPos;

    long nThumbPos  = pVerScroll->GetThumbPos();
    long nRangeMax  = pVerScroll->GetRangeMax();

    switch( nMode )
    {
        case SID_CURSORUP:
            if( nMaxVertPos<0 )
            {
                long nPage = pPreview->GetPageNo();

                if( nPage>0 )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_PREVIOUS );
                    Execute( aSfxRequest );
                }
            }
            else
                aCurPos.Y() -= nVLine;
            break;
        case SID_CURSORDOWN:
            if( nMaxVertPos<0 )
            {
                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();

                // before testing for last page, make sure all page counts are calculated
                if ( nPage+1 == nTotal && !pPreview->AllTested() )
                {
                    pPreview->CalcAll();
                    nTotal = pPreview->GetTotalPages();
                }

                if( nPage<nTotal-1 )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_NEXT );
                    Execute( aSfxRequest );
                }
            }
            else
                aCurPos.Y() += nVLine;
            break;
        case SID_CURSORLEFT:
            aCurPos.X() -= nHLine;
            break;
        case SID_CURSORRIGHT:
            aCurPos.X() += nHLine;
            break;
        case SID_CURSORPAGEUP:
            if( nThumbPos==0 || nMaxVertPos<0 )
            {
                long nPage = pPreview->GetPageNo();

                if( nPage>0 )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_PREVIOUS );
                    Execute( aSfxRequest );
                    aCurPos.Y() = nVRange;
                }
            }
            else
                aCurPos.Y() -= nVPage;
            break;
        case SID_CURSORPAGEDOWN:
            if( (abs(nVPage+nThumbPos-nRangeMax)<10) || nMaxVertPos<0 )
            {
                long nPage = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();

                // before testing for last page, make sure all page counts are calculated
                if ( nPage+1 == nTotal && !pPreview->AllTested() )
                {
                    pPreview->CalcAll();
                    nTotal = pPreview->GetTotalPages();
                }
                if( nPage<nTotal-1 )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_NEXT );
                    Execute( aSfxRequest );
                    aCurPos.Y() = 0;
                }
            }
            else
                aCurPos.Y() += nVPage;
            break;
        case SID_CURSORHOME:
            if( nMaxVertPos<0 )
            {
                long nPage  = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if( nTotal && nPage != 0 )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_FIRST );
                    Execute( aSfxRequest );
                }
            }
            else
            {
                aCurPos.Y() = 0;
                aCurPos.X() = 0;
            }
            break;
        case SID_CURSOREND:
            if( nMaxVertPos<0 )
            {
                if( !pPreview->AllTested() )
                    pPreview->CalcAll();
                long nPage  = pPreview->GetPageNo();
                long nTotal = pPreview->GetTotalPages();
                if( nTotal && nPage+1 != nTotal )
                {
                    SfxViewFrame* pSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest( pSfxViewFrame, SID_PREVIEW_LAST );
                    Execute( aSfxRequest );
                }
            }
            else
            {
                aCurPos.Y() = nVRange;
                aCurPos.X() = nHRange;
            }
            break;
    }

        // nHRange-nHPage might be negative, that's why we check for < 0 afterwards

    if( aCurPos.Y() > (nVRange-nVPage) )
        aCurPos.Y() = (nVRange-nVPage);
    if( aCurPos.Y() < 0 )
        aCurPos.Y() = 0;
    if( aCurPos.X() > (nHRange-nHPage) )
        aCurPos.X() = (nHRange-nHPage);
    if( aCurPos.X() < 0 )
        aCurPos.X() = 0;

    if( nMaxVertPos>=0 )
    {
        if( aCurPos.Y() != aPrevPos.Y() )
        {
            pVerScroll->SetThumbPos( aCurPos.Y() );
            pPreview->SetYOffset( aCurPos.Y() );
        }
    }

    if( aCurPos.X() != aPrevPos.X() )
    {
        pHorScroll->SetThumbPos( aCurPos.X() );
        pPreview->SetXOffset( aCurPos.X() );
    }

}

void ScPreviewShell::ExitPreview()
{
    GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0, SFX_CALLMODE_ASYNCHRON);
}

void ScPreviewShell::AddAccessibilityObject( SfxListener& rObject )
{
    if (!pAccessibilityBroadcaster)
        pAccessibilityBroadcaster = new SfxBroadcaster;

    rObject.StartListening( *pAccessibilityBroadcaster );
}

void ScPreviewShell::RemoveAccessibilityObject( SfxListener& rObject )
{
    if (pAccessibilityBroadcaster)
        rObject.EndListening( *pAccessibilityBroadcaster );
    else
    {
        OSL_FAIL("kein Accessibility-Broadcaster?");
    }
}

void ScPreviewShell::BroadcastAccessibility( const SfxHint &rHint )
{
    if (pAccessibilityBroadcaster)
        pAccessibilityBroadcaster->Broadcast( rHint );
}

sal_Bool ScPreviewShell::HasAccessibilityObjects()
{
    return pAccessibilityBroadcaster && pAccessibilityBroadcaster->HasListeners();
}

const ScPreviewLocationData& ScPreviewShell::GetLocationData()
{
    return pPreview->GetLocationData();
}

ScDocument* ScPreviewShell::GetDocument()
{
    return pDocShell->GetDocument();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
