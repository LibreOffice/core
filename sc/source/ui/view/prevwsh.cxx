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

#include <sal/config.h>

#include <scitems.hxx>

#include <comphelper/SetFlagContextHelper.hxx>
#include <sfx2/app.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/svdview.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewfac.hxx>
#include <o3tl/unit_conversion.hxx>
#include <o3tl/string_view.hxx>

#include <drwlayer.hxx>
#include <prevwsh.hxx>
#include <preview.hxx>
#include <printfun.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <stlpool.hxx>
#include <editutil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <ViewSettingsSequenceDefines.hxx>
#include <viewuno.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <basegfx/utils/zoomtools.hxx>
#include <svx/zoom_def.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <scabstdlg.hxx>
#include <vcl/EnumContext.hxx>

//  for mouse wheel
#define MINZOOM_SLIDER 10
#define MAXZOOM_SLIDER 400

#define SC_USERDATA_SEP ';'

using namespace com::sun::star;

#define ShellClass_ScPreviewShell
#include <scslots.hxx>

#include <memory>


SFX_IMPL_INTERFACE(ScPreviewShell, SfxViewShell)

void ScPreviewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard|SfxVisibilityFlags::Server|SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::Objectbar_Preview);

    GetStaticInterface()->RegisterPopupMenu("preview");
}

SFX_IMPL_NAMED_VIEWFACTORY( ScPreviewShell, "PrintPreview" )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

void ScPreviewShell::Construct( vcl::Window* pParent )
{
    // Find the top-most window, and set the close window handler to intercept
    // the window close event.
    vcl::Window* pWin = pParent;
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

    eZoom = SvxZoomType::WHOLEPAGE;

    pHorScroll = VclPtr<ScrollAdaptor>::Create(pParent, true);
    pVerScroll = VclPtr<ScrollAdaptor>::Create(pParent, false);

    // RTL: no mirroring for horizontal scrollbars
    pHorScroll->EnableRTL( false );

    pHorScroll->SetScrollHdl(LINK(this, ScPreviewShell, HorzScrollHandler));
    pVerScroll->SetScrollHdl(LINK(this, ScPreviewShell, VertScrollHandler));

    pPreview = VclPtr<ScPreview>::Create( pParent, pDocShell, this );

    SetPool( &SC_MOD()->GetPool() );
    SetWindow( pPreview );
    StartListening(*pDocShell, DuplicateHandling::Prevent);
    StartListening(*SfxGetpApp(), DuplicateHandling::Prevent); // #i62045# #i62046# application is needed for Calc's own hints
    SfxBroadcaster* pDrawBC = pDocShell->GetDocument().GetDrawBroadcaster();
    if (pDrawBC)
        StartListening(*pDrawBC);

    pHorScroll->Show( false );
    pVerScroll->Show( false );
    SetName("Preview");
}

ScPreviewShell::ScPreviewShell(SfxViewFrame& rViewFrame,
                               SfxViewShell* pOldSh) :
    SfxViewShell(rViewFrame, SfxViewShellFlags::HAS_PRINTOPTIONS),
    pDocShell( static_cast<ScDocShell*>(rViewFrame.GetObjectShell()) ),
    mpFrameWindow(nullptr),
    nSourceDesignMode( TRISTATE_INDET ),
    nMaxVertPos(0),
    nPrevHThumbPos(0),
    nPrevVThumbPos(0)
{
    Construct(&rViewFrame.GetWindow());
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Printpreview));

    if ( auto pTabViewShell = dynamic_cast<ScTabViewShell*>( pOldSh) )
    {
        //  store view settings, show table from TabView
        //! store live ScViewData instead, and update on ScTablesHint?
        //! or completely forget aSourceData on ScTablesHint?

        const ScViewData& rData = pTabViewShell->GetViewData();
        pPreview->SetSelectedTabs(rData.GetMarkData());
        InitStartTable( rData.GetTabNo() );

        //  also have to store the TabView's DesignMode state
        //  (only if draw view exists)
        SdrView* pDrawView = pTabViewShell->GetScDrawView();
        if ( pDrawView )
            nSourceDesignMode
                = pDrawView->IsDesignMode() ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    new ScPreviewObj(this);
}

ScPreviewShell::~ScPreviewShell()
{
    if (mpFrameWindow)
        mpFrameWindow->SetCloseHdl(Link<SystemWindow&,void>()); // Remove close handler.

    // #108333#; notify Accessibility that Shell is dying and before destroy all
    BroadcastAccessibility( SfxHint( SfxHintId::Dying ) );
    pAccessibilityBroadcaster.reset();

    SfxBroadcaster* pDrawBC = pDocShell->GetDocument().GetDrawBroadcaster();
    if (pDrawBC)
        EndListening(*pDrawBC);
    EndListening(*SfxGetpApp());
    EndListening(*pDocShell);

    SetWindow(nullptr);
    pPreview.disposeAndClear();
    pHorScroll.disposeAndClear();
    pVerScroll.disposeAndClear();

    //  normal mode of operation is switching back to default view in the same frame,
    //  so there's no need to activate any other window here anymore
}

void ScPreviewShell::InitStartTable(SCTAB nTab)
{
    pPreview->SetPageNo( pPreview->GetFirstPage(nTab) );
}

void ScPreviewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    Size aOutSize( rSize );
    pPreview->SetPosSizePixel( rPos, aOutSize );

    if ( SvxZoomType::WHOLEPAGE == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(false) );
    else if ( SvxZoomType::PAGEWIDTH == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(true) );

    UpdateNeededScrollBars(false);
}

void ScPreviewShell::InnerResizePixel( const Point &rOfs, const Size &rSize, bool )
{
    AdjustPosSizePixel( rOfs,rSize );
}

void ScPreviewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    AdjustPosSizePixel( rOfs,rSize );
}

bool ScPreviewShell::GetPageSize( Size& aPageSize )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();

    ScStyleSheetPool*   pStylePool  = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( rDoc.GetPageStyle( nTab ),
                                                        SfxStyleFamily::Page );
    OSL_ENSURE(pStyleSheet,"No style sheet");
    if (!pStyleSheet) return false;
    const SfxItemSet* pParamSet = &pStyleSheet->GetItemSet();

    aPageSize = pParamSet->Get(ATTR_PAGE_SIZE).GetSize();
    aPageSize.setWidth(o3tl::convert(aPageSize.Width(), o3tl::Length::twip, o3tl::Length::mm100));
    aPageSize.setHeight(o3tl::convert(aPageSize.Height(), o3tl::Length::twip, o3tl::Length::mm100));
    return true;
}

void ScPreviewShell::UpdateNeededScrollBars( bool bFromZoom )
{
    Size aPageSize;
    OutputDevice* pDevice = Application::GetDefaultDevice();

    tools::Long nBarW = GetViewFrame().GetWindow().GetSettings().GetStyleSettings().GetScrollBarSize();
    tools::Long nBarH = nBarW;

    tools::Long aHeightOffSet = pDevice ? pDevice->PixelToLogic( Size( nBarW, nBarH ), pPreview->GetMapMode() ).Height() : 0;
    tools::Long aWidthOffSet = aHeightOffSet;

    if (!GetPageSize( aPageSize ))
        return;

    //  for centering, page size without the shadow is used
    bool bVert = pVerScroll->IsVisible();
    bool bHori = pHorScroll->IsVisible();
    Size aWindowSize = pPreview->GetOutDev()->GetOutputSize();
    Point aPos = pPreview->GetPosPixel();
    Size aWindowPixelSize = pPreview->GetOutputSizePixel();

    // if we are called from Zoom then we need to compensate for whatever
    // scrollbars were displayed before the zoom was called
    if ( bFromZoom )
    {
        if ( bVert )
        {
            aWindowPixelSize.AdjustWidth(nBarH );
            aWindowSize.AdjustWidth(aHeightOffSet );
        }
        if ( bHori )
        {
            aWindowPixelSize.AdjustHeight(nBarW );
            aWindowSize.AdjustHeight(aWidthOffSet );
        }
    }

    // recalculate any needed scrollbars
    tools::Long nMaxWidthPos = aPageSize.Width() - aWindowSize.Width();
    bHori = nMaxWidthPos >= 0;
    tools::Long nMaxHeightPos = aPageSize.Height() - aWindowSize.Height();
    bVert = nMaxHeightPos >= 0;

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
        aWindowPixelSize.AdjustHeight( -nBarW );
    if ( bVert )
        aWindowPixelSize.AdjustWidth( -nBarH );

    pPreview->SetSizePixel( aWindowPixelSize );
    pHorScroll->SetPosSizePixel( Point( aPos.X(), aPos.Y() + aWindowPixelSize.Height() ),
                                 Size( aWindowPixelSize.Width(), nBarH ) );
    pVerScroll->SetPosSizePixel( Point( aPos.X() + aWindowPixelSize.Width(), aPos.Y() ),
                                 Size( nBarW, aWindowPixelSize.Height() ) );
    UpdateScrollBars();
}

void ScPreviewShell::UpdateScrollBars()
{
    Size aPageSize;
    if ( !GetPageSize( aPageSize ) )
        return;

    //  for centering, page size without the shadow is used

    Size aWindowSize = pPreview->GetOutDev()->GetOutputSize();

    Point aOfs = pPreview->GetOffset();

    if( pHorScroll )
    {
        pHorScroll->SetRange( Range( 0, aPageSize.Width() ) );
        pHorScroll->SetLineSize( aWindowSize.Width() / 16 );
        pHorScroll->SetPageSize( aWindowSize.Width() );
        pHorScroll->SetVisibleSize( aWindowSize.Width() );
        tools::Long nMaxPos = aPageSize.Width() - aWindowSize.Width();
        if ( nMaxPos<0 )
        {
            //  page smaller than window -> center (but put scrollbar to 0)
            aOfs.setX( 0 );
            pPreview->SetXOffset( nMaxPos / 2 );
        }
        else if (aOfs.X() < 0)
        {
            //  page larger than window -> never use negative offset
            aOfs.setX( 0 );
            pPreview->SetXOffset( 0 );
        }
        else if (aOfs.X() > nMaxPos)
        {
            //  limit offset to align with right edge of window
            aOfs.setX( nMaxPos );
            pPreview->SetXOffset(nMaxPos);
        }
        pHorScroll->SetThumbPos( aOfs.X() );
        nPrevHThumbPos = pHorScroll->GetThumbPos();
    }

    if( !pVerScroll )
        return;

    tools::Long nPageNo     = pPreview->GetPageNo();
    tools::Long nTotalPages = pPreview->GetTotalPages();

    nMaxVertPos = aPageSize.Height() - aWindowSize.Height();
    pVerScroll->SetLineSize( aWindowSize.Height() / 16  );
    pVerScroll->SetPageSize( aWindowSize.Height() );
    pVerScroll->SetVisibleSize( aWindowSize.Height() );
    if ( nMaxVertPos < 0 )
    {
        //  page smaller than window -> center (but put scrollbar to 0)
        aOfs.setY( 0 );
        pPreview->SetYOffset( nMaxVertPos / 2 );
        pVerScroll->SetThumbPos( nPageNo * aWindowSize.Height() );
        pVerScroll->SetRange( Range( 0, aWindowSize.Height() * nTotalPages ));
    }
    else if (aOfs.Y() < 0)
    {
        //  page larger than window -> never use negative offset
        pVerScroll->SetRange( Range( 0, aPageSize.Height() ) );
        aOfs.setY( 0 );
        pPreview->SetYOffset( 0 );
        pVerScroll->SetThumbPos( aOfs.Y() );
    }
    else if (aOfs.Y() > nMaxVertPos )
    {
        //  limit offset to align with window bottom
        pVerScroll->SetRange( Range( 0, aPageSize.Height() ) );
        aOfs.setY( nMaxVertPos );
        pPreview->SetYOffset( nMaxVertPos );
        pVerScroll->SetThumbPos( aOfs.Y() );
    }
    nPrevVThumbPos = pVerScroll->GetThumbPos();
}

IMPL_LINK_NOARG(ScPreviewShell, HorzScrollHandler, weld::Scrollbar&, void)
{
    ScrollHandler(pHorScroll);
}

IMPL_LINK_NOARG(ScPreviewShell, VertScrollHandler, weld::Scrollbar&, void)
{
    ScrollHandler(pVerScroll);
}

void ScPreviewShell::ScrollHandler(ScrollAdaptor* pScroll)
{
    tools::Long nPos           = pScroll->GetThumbPos();
    tools::Long nMaxRange      = pScroll->GetRangeMax();
    tools::Long nTotalPages    = pPreview->GetTotalPages();
    tools::Long nPageNo        = 0;
    tools::Long nPerPageLength = 0;
    bool bIsDivide      = true;

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

    bool bHoriz = ( pScroll == pHorScroll );

    tools::Long nDelta = bHoriz ? (pHorScroll->GetThumbPos() - nPrevHThumbPos)
                                : (pVerScroll->GetThumbPos() - nPrevVThumbPos);

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
            tools::Rectangle aRect;
            QuickHelpFlags nAlign;

            if( nDelta < 0 )
            {
                if ( nTotalPages && nPageNo > 0 && !bIsDivide )
                    pPreview->SetPageNo( nPageNo-1 );
                if( bIsDivide )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScResId( STR_PAGE ) +
                           " " + OUString::number( nPageNo ) +
                           " / "  + OUString::number( nTotalPages );
            }
            else if( nDelta > 0 )
            {
                bool bAllTested = pPreview->AllTested();
                if ( nTotalPages && ( nPageNo < nTotalPages || !bAllTested ) )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScResId( STR_PAGE ) +
                           " " + OUString::number( nPageNo+1 ) +
                           " / "  + OUString::number( nTotalPages );
            }

            aRect.SetLeft( aPos.X() - 8 );
            aRect.SetTop( aMousePos.Y() );
            aRect.SetRight( aRect.Left() );
            aRect.SetBottom( aRect.Top() );
            nAlign          = QuickHelpFlags::Bottom|QuickHelpFlags::Center;
            Help::ShowQuickHelp( pScroll->GetParent(), aRect, aHelpStr, nAlign );
        }
    }
}

IMPL_LINK_NOARG(ScPreviewShell, CloseHdl, SystemWindow&, void)
{
    ExitPreview();
}

bool ScPreviewShell::ScrollCommand( const CommandEvent& rCEvt )
{
    bool bDone = false;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if ( pData && pData->GetMode() == CommandWheelMode::ZOOM )
    {
        sal_uInt16 nOld = pPreview->GetZoom();
        sal_uInt16 nNew;
        if ( pData->GetDelta() < 0 )
            nNew = std::max( MINZOOM, basegfx::zoomtools::zoomOut( nOld ));
        else
            nNew = std::min( MAXZOOM, basegfx::zoomtools::zoomIn( nOld ));

        if ( nNew != nOld )
        {
            eZoom = SvxZoomType::PERCENT;
            pPreview->SetZoom( nNew );
        }

        bDone = true;
    }
    else
    {
        bDone = pPreview->HandleScrollCommand( rCEvt, pHorScroll, pVerScroll );
    }

    return bDone;
}

SfxPrinter* ScPreviewShell::GetPrinter( bool bCreate )
{
    return pDocShell->GetPrinter(bCreate);
}

sal_uInt16 ScPreviewShell::SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags )
{
    return pDocShell->SetPrinter( pNewPrinter, nDiffFlags );
}

bool ScPreviewShell::HasPrintOptionsPage() const
{
    return true;
}

std::unique_ptr<SfxTabPage> ScPreviewShell::CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rOptions)
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    ::CreateTabPage ScTpPrintOptionsCreate = pFact->GetTabPageCreatorFunc(RID_SC_TP_PRINT);
    if ( ScTpPrintOptionsCreate )
        return ScTpPrintOptionsCreate(pPage, pController, &rOptions);
    return nullptr;
}

void ScPreviewShell::Activate(bool bMDI)
{
    SfxViewShell::Activate(bMDI);

    //! Basic etc. -> outsource to its own file (see tabvwsh4)

    if (bMDI)
    {
        // InputHdl is now mostly Null, no more assertion!
        ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
        if ( pInputHdl )
            pInputHdl->NotifyChange( nullptr );
    }

    SfxShell::Activate(bMDI);
}

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
                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage > 0)
                    pPreview->SetPageNo( nPage-1 );
            }
            break;
        case SID_NEXT_TABLE: // Accelerator
        case SID_PREVIEW_NEXT:
            {
                bool bAllTested = pPreview->AllTested();
                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if (nTotal && (nPage+1 < nTotal || !bAllTested))
                    pPreview->SetPageNo( nPage+1 );
            }
            break;
        case SID_CURSORTOPOFFILE: // Accelerator
        case SID_PREVIEW_FIRST:
            {
                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage != 0)
                    pPreview->SetPageNo( 0 );
            }
            break;
        case SID_CURSORENDOFFILE: // Accelerator
        case SID_PREVIEW_LAST:
            {
                if (!pPreview->AllTested())
                    pPreview->CalcAll();

                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if (nTotal && nPage+1 != nTotal)
                    pPreview->SetPageNo( nTotal-1 );
            }
            break;
        case SID_ATTR_ZOOM:
        case FID_SCALE:
            {
                sal_uInt16      nZoom       = 100;
                bool        bCancel     = false;

                eZoom = SvxZoomType::PERCENT;

                if ( pReqArgs )
                {

                    const SvxZoomItem& rZoomItem = pReqArgs->Get(SID_ATTR_ZOOM);

                    eZoom = rZoomItem.GetType();
                    nZoom = rZoomItem.GetValue();
                }
                else
                {
                    SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM>  aSet( GetPool() );
                    SvxZoomItem     aZoomItem( SvxZoomType::PERCENT, pPreview->GetZoom(), SID_ATTR_ZOOM );

                    aSet.Put( aZoomItem );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractSvxZoomDialog> pDlg(pFact->CreateSvxZoomDialog(nullptr, aSet));
                    pDlg->SetLimits( 20, 400 );
                    pDlg->HideButton( ZoomButtonId::OPTIMAL );
                    bCancel = ( RET_CANCEL == pDlg->Execute() );

                    if ( !bCancel )
                    {
                        const SvxZoomItem&  rZoomItem = pDlg->GetOutputItemSet()->
                                                    Get( SID_ATTR_ZOOM );

                        eZoom = rZoomItem.GetType();
                        nZoom = rZoomItem.GetValue();
                    }
                }

                if ( !bCancel )
                {
                    switch ( eZoom )
                    {
                        case SvxZoomType::OPTIMAL:
                        case SvxZoomType::WHOLEPAGE:
                            nZoom = pPreview->GetOptimalZoom(false);
                            break;
                        case SvxZoomType::PAGEWIDTH:
                            nZoom = pPreview->GetOptimalZoom(true);
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
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
            {
                sal_uInt16 nNewZoom;
                const sal_uInt16 nOldZoom {pPreview->GetZoom()};
                if(SID_ZOOM_OUT == nSlot)
                    nNewZoom = basegfx::zoomtools::zoomOut(nOldZoom);
                else
                    nNewZoom = basegfx::zoomtools::zoomIn(nOldZoom);

                pPreview->SetZoom(nNewZoom);
                eZoom = SvxZoomType::PERCENT;
                rReq.Done();
            }
            break;
        case SID_PREVIEW_MARGIN:
            {
                bool bMargin = pPreview->GetPageMargins();
                pPreview->SetPageMargins( !bMargin );
                pPreview->Invalidate();
                rReq.Done();
            }
            break;
        case SID_ATTR_ZOOMSLIDER:
            {
                const SvxZoomSliderItem* pItem;
                eZoom = SvxZoomType::PERCENT;
                if( pReqArgs && (pItem = pReqArgs->GetItemIfSet( SID_ATTR_ZOOMSLIDER )) )
                {
                    const sal_uInt16 nCurrentZoom = pItem->GetValue();
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
                const SvxZoomSliderItem* pItem;
                SCTAB nTab                      = pPreview->GetTab();
                OUString aOldName               = pDocShell->GetDocument().GetPageStyle( pPreview->GetTab() );
                ScStyleSheetPool* pStylePool    = pDocShell->GetDocument().GetStyleSheetPool();
                SfxStyleSheetBase* pStyleSheet  = pStylePool->Find( aOldName, SfxStyleFamily::Page );
                OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                if ( pReqArgs && pStyleSheet && (pItem = pReqArgs->GetItemIfSet( SID_PREVIEW_SCALINGFACTOR )) )
                {
                    const sal_uInt16 nCurrentZoom   = pItem->GetValue();
                    SfxItemSet& rSet            = pStyleSheet->GetItemSet();
                    rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nCurrentZoom ) );
                    ScPrintFunc aPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                    rReq.Done();
                }
                GetViewFrame().GetBindings().Invalidate( nSlot );
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
    pPreview->SetInGetState(true);

    SCTAB nTab      = pPreview->GetTab();
    tools::Long nPage      = pPreview->GetPageNo();
    tools::Long nTotal     = pPreview->GetTotalPages();
    sal_uInt16 nZoom    = pPreview->GetZoom();
    bool bAllTested = pPreview->AllTested();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_STATUS_PAGESTYLE:
            case SID_HFEDIT:
                pDocShell->GetStatePageStyle( rSet, nTab );
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
            case SID_ZOOM_IN:
                if (nZoom >= 400)
                    rSet.DisableItem(nWhich);
                break;
            case SID_ZOOM_OUT:
                if (nZoom <= 20)
                    rSet.DisableItem(nWhich);
                break;
            case SID_ATTR_ZOOM:
                {
                    SvxZoomItem aZoom( eZoom, nZoom, TypedWhichId<SvxZoomItem>(nWhich) );
                    aZoom.SetValueSet( SvxZoomEnableFlags::ALL & ~SvxZoomEnableFlags::OPTIMAL );
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
                        OUString aOldName               = pDocShell->GetDocument().GetPageStyle( pPreview->GetTab() );
                        ScStyleSheetPool* pStylePool    = pDocShell->GetDocument().GetStyleSheetPool();
                        SfxStyleSheetBase* pStyleSheet  = pStylePool->Find( aOldName, SfxStyleFamily::Page );
                        OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                        if ( pStyleSheet )
                        {
                            SfxItemSet& rStyleSet   = pStyleSheet->GetItemSet();
                            sal_uInt16 nCurrentZoom = rStyleSet.Get(ATTR_PAGE_SCALE).GetValue();
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
                rSet.Put( SfxBoolItem( nWhich, true ) );
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
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();
    OUString aTmp;
    rDoc.GetName(nTab, aTmp);
    rData.aTabName = aTmp;

    if( pDocShell->getDocProperties()->getTitle().getLength() != 0 )
        rData.aTitle = pDocShell->getDocProperties()->getTitle();
    else
        rData.aTitle = pDocShell->GetTitle();

    const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
    rData.aLongDocName  = rURLObj.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
    if ( !rData.aLongDocName.isEmpty() )
        rData.aShortDocName = rURLObj.GetLastName(INetURLObject::DecodeMechanism::Unambiguous);
    else
        rData.aShortDocName = rData.aLongDocName = rData.aTitle;
    rData.nPageNo       = pPreview->GetPageNo() + 1;

    bool bAllTested = pPreview->AllTested();
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

    rData = OUString::number(pPreview->GetZoom())
        + OUStringChar(SC_USERDATA_SEP)
        + OUString::number(pPreview->GetPageNo());
}

void ScPreviewShell::ReadUserData(const OUString& rData, bool /* bBrowse */)
{
    if (!rData.isEmpty())
    {
        sal_Int32 nIndex = 0;
        pPreview->SetZoom(static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rData, 0, SC_USERDATA_SEP, nIndex))));
        pPreview->SetPageNo(o3tl::toInt32(o3tl::getToken(rData, 0, SC_USERDATA_SEP, nIndex)));
        eZoom = SvxZoomType::PERCENT;
    }
}

void ScPreviewShell::WriteUserDataSequence(uno::Sequence < beans::PropertyValue >& rSeq)
{
    // tdf#130559: don't export preview view data if active
    if (comphelper::IsContextFlagActive("NoPreviewData"))
        return;

    rSeq.realloc(3);
    beans::PropertyValue* pSeq = rSeq.getArray();
    sal_uInt16 nViewID(GetViewFrame().GetCurViewId());
    pSeq[0].Name = SC_VIEWID;
    pSeq[0].Value <<= SC_VIEW + OUString::number(nViewID);
    pSeq[1].Name = SC_ZOOMVALUE;
    pSeq[1].Value <<= sal_Int32 (pPreview->GetZoom());
    pSeq[2].Name = "PageNumber";
    pSeq[2].Value <<= pPreview->GetPageNo();

    // Common SdrModel processing
    if (ScDrawLayer* pDrawLayer = GetDocument().GetDrawLayer())
        pDrawLayer->WriteUserDataSequence(rSeq);
}

void ScPreviewShell::ReadUserDataSequence(const uno::Sequence < beans::PropertyValue >& rSeq)
{
    for (const auto& propval : rSeq)
    {
        if (propval.Name == SC_ZOOMVALUE)
        {
            sal_Int32 nTemp = 0;
            if (propval.Value >>= nTemp)
                pPreview->SetZoom(sal_uInt16(nTemp));
        }
        else if (propval.Name == "PageNumber")
        {
            sal_Int32 nTemp = 0;
            if (propval.Value >>= nTemp)
                pPreview->SetPageNo(nTemp);
        }
        // Fallback to common SdrModel processing
        else
            pDocShell->MakeDrawLayer()->ReadUserDataSequenceValue(&propval);
    }
}

void ScPreviewShell::DoScroll( sal_uInt16 nMode )
{
    Point   aCurPos, aPrevPos;

    tools::Long nHRange    = pHorScroll->GetRange().Max();
    tools::Long nHLine     = pHorScroll->GetLineSize();
    tools::Long nHPage     = pHorScroll->GetPageSize();
    tools::Long nVRange    = pVerScroll->GetRange().Max();
    tools::Long nVLine     = pVerScroll->GetLineSize();
    tools::Long nVPage     = pVerScroll->GetPageSize();

    aCurPos.setX( pHorScroll->GetThumbPos() );
    aCurPos.setY( pVerScroll->GetThumbPos() );
    aPrevPos = aCurPos;

    tools::Long nThumbPos  = pVerScroll->GetThumbPos();
    tools::Long nRangeMax  = pVerScroll->GetRangeMax();

    switch( nMode )
    {
        case SID_CURSORUP:
            if( nMaxVertPos<0 )
            {
                tools::Long nPage = pPreview->GetPageNo();

                if( nPage>0 )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_PREVIOUS);
                    Execute( aSfxRequest );
                }
            }
            else
                aCurPos.AdjustY( -nVLine );
            break;
        case SID_CURSORDOWN:
            if( nMaxVertPos<0 )
            {
                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();

                // before testing for last page, make sure all page counts are calculated
                if ( nPage+1 == nTotal && !pPreview->AllTested() )
                {
                    pPreview->CalcAll();
                    nTotal = pPreview->GetTotalPages();
                }

                if( nPage<nTotal-1 )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_NEXT);
                    Execute( aSfxRequest );
                }
            }
            else
                aCurPos.AdjustY(nVLine );
            break;
        case SID_CURSORLEFT:
            aCurPos.AdjustX( -nHLine );
            break;
        case SID_CURSORRIGHT:
            aCurPos.AdjustX(nHLine );
            break;
        case SID_CURSORPAGEUP:
            if( nThumbPos==0 || nMaxVertPos<0 )
            {
                tools::Long nPage = pPreview->GetPageNo();

                if( nPage>0 )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_PREVIOUS);
                    Execute( aSfxRequest );
                    aCurPos.setY( nVRange );
                }
            }
            else
                aCurPos.AdjustY( -nVPage );
            break;
        case SID_CURSORPAGEDOWN:
            if( (std::abs(nVPage+nThumbPos-nRangeMax)<10) || nMaxVertPos<0 )
            {
                tools::Long nPage = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();

                // before testing for last page, make sure all page counts are calculated
                if ( nPage+1 == nTotal && !pPreview->AllTested() )
                {
                    pPreview->CalcAll();
                    nTotal = pPreview->GetTotalPages();
                }
                if( nPage<nTotal-1 )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_NEXT);
                    Execute( aSfxRequest );
                    aCurPos.setY( 0 );
                }
            }
            else
                aCurPos.AdjustY(nVPage );
            break;
        case SID_CURSORHOME:
            if( nMaxVertPos<0 )
            {
                tools::Long nPage  = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if( nTotal && nPage != 0 )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_FIRST);
                    Execute( aSfxRequest );
                }
            }
            else
            {
                aCurPos.setY( 0 );
                aCurPos.setX( 0 );
            }
            break;
        case SID_CURSOREND:
            if( nMaxVertPos<0 )
            {
                if( !pPreview->AllTested() )
                    pPreview->CalcAll();
                tools::Long nPage  = pPreview->GetPageNo();
                tools::Long nTotal = pPreview->GetTotalPages();
                if( nTotal && nPage+1 != nTotal )
                {
                    SfxViewFrame& rSfxViewFrame = GetViewFrame();
                    SfxRequest aSfxRequest(rSfxViewFrame, SID_PREVIEW_LAST);
                    Execute( aSfxRequest );
                }
            }
            else
            {
                aCurPos.setY( nVRange );
                aCurPos.setX( nHRange );
            }
            break;
    }

        // nHRange-nHPage might be negative, that's why we check for < 0 afterwards

    if( aCurPos.Y() > (nVRange-nVPage) )
        aCurPos.setY( nVRange-nVPage );
    if( aCurPos.Y() < 0 )
        aCurPos.setY( 0 );
    if( aCurPos.X() > (nHRange-nHPage) )
        aCurPos.setX( nHRange-nHPage );
    if( aCurPos.X() < 0 )
        aCurPos.setX( 0 );

    if( nMaxVertPos>=0 )
    {
        if( aCurPos.Y() != aPrevPos.Y() )
        {
            pVerScroll->SetThumbPos( aCurPos.Y() );
            nPrevVThumbPos = pVerScroll->GetThumbPos();
            pPreview->SetYOffset( aCurPos.Y() );
        }
    }

    if( aCurPos.X() != aPrevPos.X() )
    {
        pHorScroll->SetThumbPos( aCurPos.X() );
        nPrevHThumbPos = pHorScroll->GetThumbPos();
        pPreview->SetXOffset( aCurPos.X() );
    }

}

void ScPreviewShell::ExitPreview()
{
    GetViewFrame().GetDispatcher()->Execute(SID_VIEWSHELL0, SfxCallMode::ASYNCHRON);
}

void ScPreviewShell::AddAccessibilityObject( SfxListener& rObject )
{
    if (!pAccessibilityBroadcaster)
        pAccessibilityBroadcaster.reset( new SfxBroadcaster );

    rObject.StartListening( *pAccessibilityBroadcaster );
}

void ScPreviewShell::RemoveAccessibilityObject( SfxListener& rObject )
{
    if (pAccessibilityBroadcaster)
        rObject.EndListening( *pAccessibilityBroadcaster );
    else
    {
        OSL_FAIL("no accessibility broadcaster?");
    }
}

void ScPreviewShell::BroadcastAccessibility( const SfxHint &rHint )
{
    if (pAccessibilityBroadcaster)
        pAccessibilityBroadcaster->Broadcast( rHint );
}

bool ScPreviewShell::HasAccessibilityObjects() const
{
    return pAccessibilityBroadcaster && pAccessibilityBroadcaster->HasListeners();
}

const ScPreviewLocationData& ScPreviewShell::GetLocationData()
{
    return pPreview->GetLocationData();
}

ScDocument& ScPreviewShell::GetDocument()
{
    return pDocShell->GetDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
