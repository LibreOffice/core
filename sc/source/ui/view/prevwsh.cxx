/*************************************************************************
 *
 *  $RCSfile: prevwsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/sizeitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/zoom.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/topfrm.hxx>
#include <svtools/printdlg.hxx>
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>

#include "prevwsh.hxx"
#include "preview.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "sc.hrc"

//  fuer Rad-Maus
#define SC_DELTA_ZOOM   10

#define SC_USERDATA_SEP ';'

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

SFX_IMPL_VIEWFACTORY( ScPreviewShell, ScResId(STR_NONAME) )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

//------------------------------------------------------------------

void ScPreviewShell::Construct( Window* pParent )
{
    eZoom = SVX_ZOOM_WHOLEPAGE;

    pCorner = new Window(pParent, WinBits( WB_BORDER ));
    pCorner->SetBackground( Color( COL_LIGHTGRAY ) );

    pHorScroll = new ScrollBar(pParent, WB_HSCROLL );
    pVerScroll = new ScrollBar(pParent, WB_VSCROLL);

    pHorScroll->SetEndScrollHdl( LINK( this, ScPreviewShell, ScrollHandler ) );
    pVerScroll->SetEndScrollHdl( LINK( this, ScPreviewShell, ScrollHandler ) );

    pPreview = new ScPreview( pParent, pDocShell, this );

    SetPool( &SC_MOD()->GetPool() );
    SetWindow( pPreview );
    StartListening(*pDocShell);
    StartListening(*SFX_APP());
    SfxBroadcaster* pDrawBC = pDocShell->GetDocument()->GetDrawBroadcaster();
    if (pDrawBC)
        StartListening(*pDrawBC);

//  pPreview->Show();           // wird vom Sfx angezeigt
    pHorScroll->Show();
    pVerScroll->Show();
    pCorner->Show();
    SetHelpId( HID_SCSHELL_PREVWSH );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Preview")));
}

ScPreviewShell::ScPreviewShell( SfxViewFrame* pViewFrame,
                                const ScPreviewShell& rWin ) :
    SfxViewShell( pViewFrame, SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_CAN_PRINT ),
    pDocShell( rWin.pDocShell ),
    aSourceData( rWin.aSourceData )
{
    Construct( &pViewFrame->GetWindow() );
}

ScPreviewShell::ScPreviewShell( SfxViewFrame* pViewFrame,
                                Window *pParent ) :
    SfxViewShell( pViewFrame, SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_CAN_PRINT ),
    pDocShell( (ScDocShell*)pViewFrame->GetObjectShell() )
{
    Construct( pParent );
}

ScPreviewShell::ScPreviewShell( SfxViewFrame* pViewFrame,
                                SfxViewShell* pOldSh ) :
    SfxViewShell( pViewFrame, SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_CAN_PRINT ),
    pDocShell( (ScDocShell*)pViewFrame->GetObjectShell() )
{
    Construct( &pViewFrame->GetWindow() );

    if ( pOldSh && pOldSh->ISA( ScTabViewShell ) )
    {
        //  store view settings, show table from TabView
        //! store live ScViewData instead, and update on ScTablesHint?
        //! or completely forget aSourceData on ScTablesHint?

        ScViewData* pData = ((ScTabViewShell*)pOldSh)->GetViewData();
        pData->WriteUserData( aSourceData );
        InitStartTable( pData->GetTabNo() );
    }
}

__EXPORT ScPreviewShell::~ScPreviewShell()
{
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

    //  #47435# wie im Writer (SwPagePreView::~SwPagePreView):
    //  Wenn die Preview zugemacht wird, eine andere View des Dokuments suchen und aktivieren

    TypeId aType = TYPE( SfxTopViewFrame );
    for( SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pDocShell, aType );
        pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell, aType ) )
        if( pFrame != GetViewFrame() )
        {
            // es gibt noch eine weitere Sicht auf unser Dokument, also
            // aktiviere dieses
            pFrame->GetFrame()->Appear();
            break;
        }
}

void ScPreviewShell::InitStartTable(USHORT nTab)
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
        USHORT              nCurTab     = pViewData->GetTabNo();
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
        DBG_ERROR( "TabViewShell not found :-/" );

    return aOptSize;
}

void __EXPORT ScPreviewShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    long nBarW = pVerScroll->GetSizePixel().Width();
    long nBarH = pHorScroll->GetSizePixel().Height();
    Size aOutSize( rSize.Width()-nBarW, rSize.Height()-nBarH );
    pPreview->SetPosSizePixel( rPos, aOutSize );
    pHorScroll->SetPosSizePixel( Point( rPos.X(), rPos.Y() + aOutSize.Height() ),
                                 Size( aOutSize.Width(), nBarH ) );
    pVerScroll->SetPosSizePixel( Point( rPos.X() + aOutSize.Width(), rPos.Y() ),
                                 Size( nBarW, aOutSize.Height() ) );
    pCorner->SetPosSizePixel( Point( rPos.X() + aOutSize.Width(), rPos.Y() + aOutSize.Height() ),
                              Size( nBarW, nBarH ) );

    if ( SVX_ZOOM_WHOLEPAGE == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(FALSE) );
    else if ( SVX_ZOOM_PAGEWIDTH == eZoom )
        pPreview->SetZoom( pPreview->GetOptimalZoom(TRUE) );
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
    USHORT nTab = pPreview->GetTab();

    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ),
                                                        SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT(pStyleSheet,"StyleSheet nicht gefunden");
    if (!pStyleSheet) return;
    const SfxItemSet* pParamSet = &pStyleSheet->GetItemSet();

    Size aPageSize = ((const SvxSizeItem&) pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
    aPageSize.Width()  = (long) (aPageSize.Width()  * HMM_PER_TWIPS );
    aPageSize.Height() = (long) (aPageSize.Height() * HMM_PER_TWIPS );
    Size aWindowSize = pPreview->GetOutputSize();

    Point aOfs = pPreview->GetOffset();
    long nMaxPos;

    pHorScroll->SetRange( Range( 0, aPageSize.Width() ) );
    pHorScroll->SetLineSize( aWindowSize.Width() / 16 );
    pHorScroll->SetPageSize( aWindowSize.Width() );
    pHorScroll->SetVisibleSize( aWindowSize.Width() );
    nMaxPos = aPageSize.Width() - aWindowSize.Width();
    if (nMaxPos<0) nMaxPos = 0;
    if (aOfs.X() > nMaxPos)
    {
        aOfs.X() = nMaxPos;
        pPreview->SetXOffset(nMaxPos);
    }
    pHorScroll->SetThumbPos( aOfs.X() );

    pVerScroll->SetRange( Range( 0, aPageSize.Height() ) );
    pVerScroll->SetLineSize( aWindowSize.Height() / 16 );
    pVerScroll->SetPageSize( aWindowSize.Height() );
    pVerScroll->SetVisibleSize( aWindowSize.Height() );
    nMaxPos = aPageSize.Height() - aWindowSize.Height();
    if (nMaxPos<0) nMaxPos = 0;
    if (aOfs.Y() > nMaxPos)
    {
        aOfs.Y() = nMaxPos;
        pPreview->SetYOffset(nMaxPos);
    }
    pVerScroll->SetThumbPos( aOfs.Y() );
}

IMPL_LINK (ScPreviewShell,ScrollHandler, ScrollBar* ,pScroll )
{
    long nDelta = pScroll->GetDelta();
    long nPos = pScroll->GetThumbPos();

    BOOL bHoriz = ( pScroll == pHorScroll );
    if (bHoriz)
        pPreview->SetXOffset( nPos );
    else
        pPreview->SetYOffset( nPos );

    return 0;
}

BOOL ScPreviewShell::ScrollCommand( const CommandEvent& rCEvt )
{
    BOOL bDone = FALSE;
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
            pPreview->SetZoom( nNew );
        }

        bDone = TRUE;
    }
    else
    {
        bDone = pPreview->HandleScrollCommand( rCEvt, pHorScroll, pVerScroll );

        //! ist das noetig ????
        if ( bDone )
        {
            Point aOld = pPreview->GetOffset();
            long nHorThunb = pHorScroll->GetThumbPos();
            if ( nHorThunb != aOld.X() )
                pPreview->SetXOffset( nHorThunb );
            long nVerThunb = pVerScroll->GetThumbPos();
            if ( nVerThunb != aOld.Y() )
                pPreview->SetYOffset( nVerThunb );
        }
    }

    return bDone;
}

SfxPrinter* __EXPORT ScPreviewShell::GetPrinter( BOOL bCreate )
{
    //  Drucker ist immer da (wird fuer die FontListe schon beim Starten angelegt)
    return pDocShell->GetPrinter();
}

USHORT __EXPORT ScPreviewShell::SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags )
{
    return pDocShell->SetPrinter( pNewPrinter, nDiffFlags );
}

PrintDialog* __EXPORT ScPreviewShell::CreatePrintDialog( Window* pParent )
{
    pDocShell->GetDocument()->SetPrintOptions();    // Optionen aus OFA am Printer setzen
    SfxPrinter* pPrinter = GetPrinter();

    const long   nCurPage    = pPreview->GetPageNo()+1;
    const long   nDocPageMax = pPreview->GetTotalPages();
#ifndef VCL
    PrintDialog* pDlg        = new PrintDialog( pParent,
                                                WinBits(WB_SVLOOK|WB_STDMODAL) );
#else
    PrintDialog* pDlg        = new PrintDialog( pParent );
#endif
// wenn zu langsam wieder einbauen
//  if ( pPreview->AllTested() )
//      nPageMax = pPreview->GetTotalPages();

    if ( nDocPageMax > 0 )
        pDlg->SetRangeText( String::CreateFromInt32( nCurPage ) );

    pDlg->EnableRange   ( PRINTDIALOG_ALL );
    pDlg->EnableRange   ( PRINTDIALOG_RANGE );
    pDlg->SetFirstPage  ( 1 );
    pDlg->SetMinPage    ( 1 );
    pDlg->SetLastPage   ( (USHORT)nDocPageMax );
    pDlg->SetMaxPage    ( (USHORT)nDocPageMax );
    pDlg->EnableCollate ();

    // Selektion hier nicht

    return pDlg;
}

void __EXPORT ScPreviewShell::PreparePrint( PrintDialog* pPrintDialog )
{
    SfxViewShell::PreparePrint( pPrintDialog );
    pDocShell->PreparePrint( pPrintDialog, NULL );
}

USHORT __EXPORT ScPreviewShell::Print( SfxProgress& rProgress, PrintDialog* pPrintDialog )
{
    pDocShell->GetDocument()->SetPrintOptions();    // Optionen aus OFA am Printer setzen

    SfxViewShell::Print( rProgress, pPrintDialog );
    pDocShell->Print( rProgress, pPrintDialog, NULL );

    return 0;
}

//------------------------------------------------------------------------

void __EXPORT ScPreviewShell::Activate(BOOL bMDI)
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

void __EXPORT ScPreviewShell::Deactivate(BOOL bMDI)
{
    SfxViewShell::Deactivate(bMDI);

    if (bMDI)
    {
    }
}

//------------------------------------------------------------------------

void __EXPORT ScPreviewShell::Execute( SfxRequest& rReq )
{
    USHORT nSlot = rReq.GetSlot();
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
                BOOL bAllTested = pPreview->AllTested();
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
                USHORT      nZoom       = 100;
                BOOL        bCancel     = FALSE;

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
                    SvxZoomDialog* pDlg = pDlg = new SvxZoomDialog( NULL, aSet );
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

                if ( !bCancel )
                {
                    switch ( eZoom )
                    {
                        case SVX_ZOOM_OPTIMAL:
                        case SVX_ZOOM_WHOLEPAGE:
                            nZoom = pPreview->GetOptimalZoom(FALSE);
                            break;
                        case SVX_ZOOM_PAGEWIDTH:
                            nZoom = pPreview->GetOptimalZoom(TRUE);
                            break;
                    }

                    pPreview->SetZoom( nZoom );
                    rReq.Done();
                }
            }
            break;
        case SID_PREVIEW_ZOOMIN:
            {
                USHORT nNew = pPreview->GetZoom() + 20 ;
                nNew -= nNew % 20;
                pPreview->SetZoom( nNew );
                eZoom = SVX_ZOOM_PERCENT;
                rReq.Done();
            }
            break;
        case SID_PREVIEW_ZOOMOUT:
            {
                USHORT nNew = pPreview->GetZoom() - 1;
                nNew -= nNew % 20;
                pPreview->SetZoom( nNew );
                eZoom = SVX_ZOOM_PERCENT;
                rReq.Done();
            }
            break;
        case SID_PRINTPREVIEW:
            //  print preview is now always in the same frame as the tab view
            //  -> always switch this frame back to normal view
            //  (ScTabViewShell ctor reads stored view data)

            GetViewFrame()->GetBindings().Execute( SID_VIEWSHELL0, NULL, 0, SFX_CALLMODE_ASYNCHRON );
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

        default:
            break;
    }
}

void __EXPORT ScPreviewShell::GetState( SfxItemSet& rSet )
{
    pPreview->SetInGetState(TRUE);

    USHORT nTab     = pPreview->GetTab();
    long nPage      = pPreview->GetPageNo();
    long nTotal     = pPreview->GetTotalPages();
    USHORT nZoom    = pPreview->GetZoom();
    BOOL bAllTested = pPreview->AllTested();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
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
            case SID_STATUS_DOCPOS:
                rSet.Put( SfxStringItem( nWhich, pPreview->GetPosString() ) );
                break;
            case SID_PRINTPREVIEW:
                rSet.Put( SfxBoolItem( nWhich, TRUE ) );
                break;
            case SID_FORMATPAGE:
                //! bei geschuetzten Tabellen ???
                if (pDocShell->IsReadOnly())
                    rSet.DisableItem( nWhich );
                break;
        }

        nWhich = aIter.NextWhich();
    }

    pPreview->SetInGetState(FALSE);
}

void ScPreviewShell::FillFieldData( ScHeaderFieldData& rData )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nTab = pPreview->GetTab();
    pDoc->GetName( nTab, rData.aTabName );

    rData.aTitle        = pDocShell->GetTitle();
    rData.aLongDocName  = pDocShell->GetMedium()->GetName();
    if ( !rData.aLongDocName.Len() )
        rData.aLongDocName = rData.aTitle;
    rData.aShortDocName = INetURLObject( rData.aLongDocName ).GetName();
    rData.nPageNo       = pPreview->GetPageNo() + 1;

    BOOL bAllTested = pPreview->AllTested();
    if (bAllTested)
        rData.nTotalPages = pPreview->GetTotalPages();
    else
        rData.nTotalPages = 99;

    //  eNumType kennt der Dialog selber
}

void __EXPORT ScPreviewShell::WriteUserData(String& rData, BOOL bBrowse)
{
    //  nZoom
    //  nPageNo

    rData =  String::CreateFromInt32(pPreview->GetZoom());
    rData += (sal_Unicode) SC_USERDATA_SEP;
    rData += String::CreateFromInt32(pPreview->GetPageNo());

    if ( aSourceData.Len() )
    {
        //  add settings from tabview in parentheses

        rData += (sal_Unicode) SC_USERDATA_SEP;
        rData += (sal_Unicode) '(';
        rData += aSourceData;
        rData += (sal_Unicode) ')';
    }
}

void __EXPORT ScPreviewShell::ReadUserData(const String& rData, BOOL bBrowse)
{
    xub_StrLen nCount = rData.GetTokenCount();
    if (nCount)
    {
        xub_StrLen nIndex = 0;
        pPreview->SetZoom(rData.GetToken( 0, SC_USERDATA_SEP, nIndex ).ToInt32());
        pPreview->SetPageNo(rData.GetToken( 0, SC_USERDATA_SEP, nIndex ).ToInt32());
        eZoom = SVX_ZOOM_PERCENT;

        //  tabview data is enclosed in parentheses

        String aTabStr = rData.Copy( nIndex );
        if ( aTabStr.GetChar(0) == '(' )
        {
            xub_StrLen nParPos = aTabStr.Search( ')' );
            if ( nParPos != STRING_NOTFOUND )
            {
                //  get the string in the parentheses
                aSourceData = aTabStr.Copy( 1, nParPos - 1 );
            }
        }
    }
}

void ScPreviewShell::DoScroll( USHORT nMode )
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

    switch( nMode )
    {
        case SID_CURSORUP:
            aCurPos.Y() -= nVLine;
            break;
        case SID_CURSORDOWN:
            aCurPos.Y() += nVLine;
            break;
        case SID_CURSORLEFT:
            aCurPos.X() -= nHLine;
            break;
        case SID_CURSORRIGHT:
            aCurPos.X() += nHLine;
            break;
        case SID_CURSORPAGEUP:
            aCurPos.Y() -= nVPage;
            break;
        case SID_CURSORPAGEDOWN:
            aCurPos.Y() += nVPage;
            break;
        case SID_CURSORHOME:
            aCurPos.Y() = 0;
            aCurPos.X() = 0;
            break;
        case SID_CURSOREND:
            aCurPos.Y() = nVRange;
            aCurPos.X() = nHRange;
            break;
    }

        // nHRange-nHPage kann negativ sein, deshalb Abfrage auf < 0 hinterher

    if( aCurPos.Y() > (nVRange-nVPage) )    aCurPos.Y() = (nVRange-nVPage);
    if( aCurPos.Y() < 0 )                   aCurPos.Y() = 0;
    if( aCurPos.X() > (nHRange-nHPage) )    aCurPos.X() = (nHRange-nHPage);
    if( aCurPos.X() < 0 )                   aCurPos.X() = 0;

    if( aCurPos.Y() != aPrevPos.Y() )
    {
        pVerScroll->SetThumbPos( aCurPos.Y() );
        pPreview->SetYOffset( aCurPos.Y() );
    }

    if( aCurPos.X() != aPrevPos.X() )
    {
        pHorScroll->SetThumbPos( aCurPos.X() );
        pPreview->SetXOffset( aCurPos.X() );
    }

}



