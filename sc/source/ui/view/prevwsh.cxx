/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/srchitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/svdview.hxx>
//CHINA001 #include <svx/zoom.hxx>
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

#ifndef _SVX_ZOOMSLIDERITEM_HXX
#include <svx/zoomslideritem.hxx>
#endif
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
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

#ifndef _SVX_ZOOM_HXX
#include <svx/zoom_def.hxx>
#endif

#include "sc.hrc" //CHINA001
#include "scabstdlg.hxx" //CHINA001
//  fuer Rad-Maus
#define SC_DELTA_ZOOM   10
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
    eZoom = SVX_ZOOM_WHOLEPAGE;

    pCorner = new ScrollBarBox( pParent, WB_SIZEABLE );

    pHorScroll = new ScrollBar(pParent, WB_HSCROLL );
    pVerScroll = new ScrollBar(pParent, WB_VSCROLL);

    // SSA: --- RTL --- no mirroring for horizontal scrollbars
    pHorScroll->EnableRTL( sal_False );

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

    pHorScroll->Show();
    pVerScroll->Show();
    pCorner->Show();
    SetHelpId( HID_SCSHELL_PREVWSH );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Preview")));
}

ScPreviewShell::ScPreviewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    SfxViewShell( pViewFrame, SFX_VIEW_CAN_PRINT | SFX_VIEW_HAS_PRINTOPTIONS ),
    pDocShell( (ScDocShell*)pViewFrame->GetObjectShell() ),
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
        ScViewData* pData = pTabViewShell->GetViewData();
        pData->WriteUserDataSequence( aSourceData );
        InitStartTable( pData->GetTabNo() );

        //  #106334# also have to store the TabView's DesignMode state
        //  (only if draw view exists)
        SdrView* pDrawView = pTabViewShell->GetSdrView();
        if ( pDrawView )
            nSourceDesignMode = pDrawView->IsDesignMode();
    }
}

__EXPORT ScPreviewShell::~ScPreviewShell()
{
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

    //  #97612# normal mode of operation is switching back to default view in the same frame,
    //  so there's no need to activate any other window here anymore
}

void ScPreviewShell::InitStartTable(SCTAB nTab)
{
    pPreview->SetPageNo( pPreview->GetFirstPage(nTab) );
}

//------------------------------------------------------------------

String __EXPORT ScPreviewShell::GetDescription() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(" ** Test ** "));
}

Size __EXPORT ScPreviewShell::GetOptimalSizePixel() const
{
    Size aOptSize(100,100);

    ScTabViewShell*     pViewSh = pDocShell->GetBestViewShell();

    if ( pViewSh )
    {
        ScViewData*         pViewData   = pViewSh->GetViewData();
        SCTAB               nCurTab     = pViewData->GetTabNo();
        ScDocument*         pDoc        = pDocShell->GetDocument();
        ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
        SfxStyleSheetBase*  pStyleSheet = pStylePool->Find(
                                            pDoc->GetPageStyle( nCurTab ),
                                            SFX_STYLE_FAMILY_PAGE );

        DBG_ASSERT( pStyleSheet, "PageStyle not found :-/" );

        if ( pStyleSheet )
        {
            const SfxItemSet&  rSet      = pStyleSheet->GetItemSet();
            const SvxSizeItem& rItem     = (const SvxSizeItem&)rSet.Get( ATTR_PAGE_SIZE );
            const Size&        rPageSize = rItem.GetSize();

            aOptSize.Width()  = (long) (rPageSize.Width()  * pViewData->GetPPTX());
            aOptSize.Height() = (long) (rPageSize.Height() * pViewData->GetPPTY());
        }
    }
    else
    {
        DBG_ERROR( "TabViewShell not found :-/" );
    }

    return aOptSize;
}

void __EXPORT ScPreviewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    long nBarW = GetViewFrame()->GetWindow().GetSettings().GetStyleSettings().GetScrollBarSize();
    long nBarH = nBarW;
//  long nBarW = pVerScroll->GetSizePixel().Width();
//  long nBarH = pHorScroll->GetSizePixel().Height();

    Size aOutSize( rSize.Width()-nBarW, rSize.Height()-nBarH );
    pPreview->SetPosSizePixel( rPos, aOutSize );
    pHorScroll->SetPosSizePixel( Point( rPos.X(), rPos.Y() + aOutSize.Height() ),
                                 Size( aOutSize.Width(), nBarH ) );
    pVerScroll->SetPosSizePixel( Point( rPos.X() + aOutSize.Width(), rPos.Y() ),
                                 Size( nBarW, aOutSize.Height() ) );
    pCorner->SetPosSizePixel( Point( rPos.X() + aOutSize.Width(), rPos.Y() + aOutSize.Height() ),
                              Size( nBarW, nBarH ) );

    if ( SVX_ZOOM_WHOLEPAGE == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(sal_False) );
    else if ( SVX_ZOOM_PAGEWIDTH == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(sal_True) );

    UpdateScrollBars();
}

void __EXPORT ScPreviewShell::InnerResizePixel( const Point &rOfs, const Size &rSize )
{
    AdjustPosSizePixel( rOfs,rSize );
}

void __EXPORT ScPreviewShell::OuterResizePixel( const Point &rOfs, const Size &rSize )
{
    AdjustPosSizePixel( rOfs,rSize );
}

void ScPreviewShell::UpdateScrollBars()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();

    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ),
                                                        SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT(pStyleSheet,"StyleSheet nicht gefunden");
    if (!pStyleSheet) return;
    const SfxItemSet* pParamSet = &pStyleSheet->GetItemSet();

    Size aPageSize = ((const SvxSizeItem&) pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
    aPageSize.Width()  = (long) (aPageSize.Width()  * HMM_PER_TWIPS );
    aPageSize.Height() = (long) (aPageSize.Height() * HMM_PER_TWIPS );

    //  for centering, page size without the shadow is used

    Size aWindowSize = pPreview->GetOutputSize();

    Point aOfs = pPreview->GetOffset();
    long nMaxPos;

    if( pHorScroll )
    {
        pHorScroll->SetRange( Range( 0, aPageSize.Width() ) );
        pHorScroll->SetLineSize( aWindowSize.Width() / 16 );
        pHorScroll->SetPageSize( aWindowSize.Width() );
        pHorScroll->SetVisibleSize( aWindowSize.Width() );
        nMaxPos = aPageSize.Width() - aWindowSize.Width();
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
            bIsDivide = sal_False;
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
            String aHelpStr;
            Rectangle aRect;
            sal_uInt16   nAlign;

            if( nDelta < 0 )
            {
                if ( nTotalPages && nPageNo > 0 && !bIsDivide )
                    pPreview->SetPageNo( nPageNo-1 );
                if( bIsDivide )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScGlobal::GetRscString( STR_PAGE );
                aHelpStr += ' ';
                aHelpStr += String::CreateFromInt32( nPageNo );

                aHelpStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " / " ));
                aHelpStr += String::CreateFromInt32( nTotalPages );
            }
            else if( nDelta > 0 )
            {
                sal_Bool bAllTested = pPreview->AllTested();
                if ( nTotalPages && ( nPageNo < nTotalPages || !bAllTested ) )
                    pPreview->SetPageNo( nPageNo );

                aHelpStr = ScGlobal::GetRscString( STR_PAGE );
                aHelpStr += ' ';
                aHelpStr += String::CreateFromInt32( nPageNo+1 );

                aHelpStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " / " ));
                aHelpStr += String::CreateFromInt32( nTotalPages );
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

sal_Bool ScPreviewShell::ScrollCommand( const CommandEvent& rCEvt )
{
    sal_Bool bDone = sal_False;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if ( pData && pData->GetMode() == COMMAND_WHEEL_ZOOM )
    {
        long nOld = pPreview->GetZoom();
        long nNew = nOld;
        if ( pData->GetDelta() < 0 )
            nNew = Max( (long) MINZOOM, (long)( nOld - SC_DELTA_ZOOM ) );
        else
            nNew = Min( (long) MAXZOOM, (long)( nOld + SC_DELTA_ZOOM ) );

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

SfxPrinter* __EXPORT ScPreviewShell::GetPrinter( sal_Bool bCreate )
{
    return pDocShell->GetPrinter(bCreate);
}

sal_uInt16 __EXPORT ScPreviewShell::SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    return pDocShell->SetPrinter( pNewPrinter, nDiffFlags );
}

SfxTabPage* ScPreviewShell::CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions )
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001
    //CHINA001 return ScTpPrintOptions::Create( pParent, rOptions );
    ::CreateTabPage ScTpPrintOptionsCreate =    pFact->GetTabPageCreatorFunc( RID_SCPAGE_PRINT );
    if ( ScTpPrintOptionsCreate )
        return  (*ScTpPrintOptionsCreate)( pParent, rOptions);
    return 0;
}

//------------------------------------------------------------------------

void __EXPORT ScPreviewShell::Activate(sal_Bool bMDI)
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

void __EXPORT ScPreviewShell::Deactivate(sal_Bool bMDI)
{
    SfxViewShell::Deactivate(bMDI);

    if (bMDI)
    {
    }
}

//------------------------------------------------------------------------

void __EXPORT ScPreviewShell::Execute( SfxRequest& rReq )
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
                sal_Bool        bCancel     = sal_False;

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
                    //CHINA001 SvxZoomDialog* pDlg = pDlg = new SvxZoomDialog( NULL, aSet );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        AbstractSvxZoomDialog* pDlg = pFact->CreateSvxZoomDialog(NULL, aSet);
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
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
                            nZoom = pPreview->GetOptimalZoom(sal_False);
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
                DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

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

            GetViewFrame()->GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_ASYNCHRON );
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

void __EXPORT ScPreviewShell::GetState( SfxItemSet& rSet )
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
                        DBG_ASSERT( pStyleSheet, "PageStyle not found! :-/" );

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

    pPreview->SetInGetState(sal_False);
}

void ScPreviewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = pPreview->GetTab();
    pDoc->GetName( nTab, rData.aTabName );

    rData.aTitle        = pDocShell->GetTitle();
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

    //  eNumType kennt der Dialog selber
}

void __EXPORT ScPreviewShell::WriteUserData(String& rData, sal_Bool /* bBrowse */)
{
    //  nZoom
    //  nPageNo

    rData =  String::CreateFromInt32(pPreview->GetZoom());
    rData += (sal_Unicode) SC_USERDATA_SEP;
    rData += String::CreateFromInt32(pPreview->GetPageNo());
}

void __EXPORT ScPreviewShell::ReadUserData(const String& rData, sal_Bool /* bBrowse */)
{
    xub_StrLen nCount = rData.GetTokenCount();
    if (nCount)
    {
        xub_StrLen nIndex = 0;
        pPreview->SetZoom((sal_uInt16)rData.GetToken( 0, SC_USERDATA_SEP, nIndex ).ToInt32());
        pPreview->SetPageNo(rData.GetToken( 0, SC_USERDATA_SEP, nIndex ).ToInt32());
        eZoom = SVX_ZOOM_PERCENT;
    }
}

void __EXPORT ScPreviewShell::WriteUserDataSequence(uno::Sequence < beans::PropertyValue >& rSeq, sal_Bool /* bBrowse */)
{
    rSeq.realloc(3);
    beans::PropertyValue* pSeq = rSeq.getArray();
    if(pSeq)
    {
        sal_uInt16 nViewID(GetViewFrame()->GetCurViewId());
        pSeq[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VIEWID));
        rtl::OUStringBuffer sBuffer(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_VIEW)));
        SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nViewID));
        pSeq[0].Value <<= sBuffer.makeStringAndClear();
        pSeq[1].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ZOOMVALUE));
        pSeq[1].Value <<= sal_Int32 (pPreview->GetZoom());
        pSeq[2].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageNumber"));
        pSeq[2].Value <<= pPreview->GetPageNo();
    }
}

void __EXPORT ScPreviewShell::ReadUserDataSequence(const uno::Sequence < beans::PropertyValue >& rSeq, sal_Bool /* bBrowse */)
{
    sal_Int32 nCount(rSeq.getLength());
    if (nCount)
    {
        sal_Int32 nTemp = 0;
        const beans::PropertyValue* pSeq = rSeq.getConstArray();
        if(pSeq)
        {
            for(sal_Int32 i = 0; i < nCount; i++, pSeq++)
            {
                rtl::OUString sName(pSeq->Name);
                if(sName.compareToAscii(SC_ZOOMVALUE) == 0)
                {
                    if (pSeq->Value >>= nTemp)
                        pPreview->SetZoom(sal_uInt16(nTemp));
                }
                else if (sName.compareToAscii("PageNumber") == 0)
                {
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

        // nHRange-nHPage kann negativ sein, deshalb Abfrage auf < 0 hinterher

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
        DBG_ERROR("kein Accessibility-Broadcaster?");
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


