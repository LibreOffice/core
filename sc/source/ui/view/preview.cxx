/*************************************************************************
 *
 *  $RCSfile: preview.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:17:41 $
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
#include <tools/pstm.hxx>
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svtools/colorcfg.hxx>
#include <svx/fmview.hxx>
#include <svx/sizeitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/itemset.hxx>
#include <tools/multisel.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/sound.hxx>

#include "preview.hxx"
#include "prevwsh.hxx"
#include "prevloc.hxx"
#include "docsh.hxx"
#include "printfun.hxx"
#include "printopt.hxx"
#include "stlpool.hxx"
#include "drwlayer.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "sc.hrc"           // fuer ShellInvalidate
#include "AccessibleDocumentPagePreview.hxx"
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif

// STATIC DATA -----------------------------------------------------------

//==================================================================

#define SC_PREVIEW_SHADOWSIZE   2

long lcl_GetDisplayStart( USHORT nTab, ScDocument* pDoc, long* pPages )
{
    long nDisplayStart = 0;
    for (USHORT i=0; i<nTab; i++)
    {
        if ( pDoc->NeedPageResetAfterTab(i) )
            nDisplayStart = 0;
        else
            nDisplayStart += pPages[i];
    }
    return nDisplayStart;
}


ScPreview::ScPreview( Window* pParent, ScDocShell* pDocSh, ScPreviewShell* pViewSh ) :
    Window( pParent ),
    pDocShell( pDocSh ),
    pViewShell( pViewSh ),
    bInPaint( FALSE ),
    bValid( FALSE ),
    bStateValid( FALSE ),
    bLocationValid( FALSE ),
    bInGetState( FALSE ),
    pLocationData( NULL ),
    pDrawView( NULL ),
    nTabsTested( 0 ),
    nPageNo( 0 ),
    nZoom( 100 ),
    nTab( 0 ),
    nTotalPages( 0 ),
    nTabStart( 0 ),
    nDisplayStart( 0 )
{
    SetOutDevViewType( OUTDEV_VIEWTYPE_PRINTPREVIEW ); //#106611#
    SetBackground();

    SetHelpId( HID_SC_WIN_PREVIEW );
    SetUniqueId( HID_SC_WIN_PREVIEW );
}


__EXPORT ScPreview::~ScPreview()
{
    delete pDrawView;
    delete pLocationData;
}


void ScPreview::UpdateDrawView()        // nTab muss richtig sein
{
    ScDocument* pDoc = pDocShell->GetDocument();

    Rectangle aFull( 0, 0, LONG_MAX, LONG_MAX );
    if ( pDoc->HasControl( nTab, aFull ) )
    {
        if ( pDrawView && !pDrawView->GetPageViewPgNum(nTab) )
        {
            //  die angezeigte Page der DrawView umzustellen (s.u.) funktioniert nicht ?!?
            delete pDrawView;
            pDrawView = NULL;
        }

        if ( !pDrawView )                                   // neu anlegen?
        {
            ScDrawLayer* pModel = pDoc->GetDrawLayer();     // ist nicht 0
            pDrawView = new FmFormView( pModel, this );
            // #55259# die DrawView uebernimmt den Design-Modus vom Model
            // (Einstellung "Im Entwurfsmodus oeffnen"), darum hier zuruecksetzen
            pDrawView->SetDesignMode( TRUE );
            pDrawView->SetPrintPreview( TRUE );
            pDrawView->ShowPagePgNum( nTab, Point() );
        }
#if 0
        else if ( !pDrawView->GetPageViewPgNum(nTab) )      // angezeigte Page umstellen
        {
            pDrawView->HideAllPages();
            pDrawView->ShowPagePgNum( nTab, Point() );
        }
#endif
    }
    else if ( pDrawView )
    {
        delete pDrawView;           // fuer diese Tabelle nicht gebraucht
        pDrawView = NULL;
    }
}


void ScPreview::TestLastPage()
{
    if (nPageNo >= nTotalPages)
    {
        if (nTotalPages)
        {
            nPageNo = nTotalPages - 1;
            nTab = nTabCount - 1;
            while (nTab && !nPages[nTab])       // letzte nicht leere Tabelle
                --nTab;
            DBG_ASSERT(nPages[nTab],"alle Tabellen leer?");
            nTabPage = nPages[nTab] - 1;
            nTabStart = 0;
            for (USHORT i=0; i<nTab; i++)
                nTabStart += nPages[i];

            ScDocument* pDoc = pDocShell->GetDocument();
            nDisplayStart = lcl_GetDisplayStart( nTab, pDoc, nPages );
        }
        else        // leeres Dokument
        {
            nTab = 0;
            nPageNo = nTabPage = nTabStart = nDisplayStart = 0;
            aState.nPrintTab = aState.nStartCol = aState.nStartRow =
            aState.nEndCol = aState.nEndRow = aState.nZoom =
            aState.nPagesX = aState.nPagesY = 0;
            aState.nTabPages = aState.nTotalPages =
            aState.nPageStart = aState.nDocPages = 0;
        }
    }
}


void ScPreview::CalcPages( USHORT nToWhichTab )
{
    WaitObject( this );

    ScDocument* pDoc = pDocShell->GetDocument();
    nTabCount = pDoc->GetTableCount();
    USHORT i;

    USHORT nAnz = Min( nTabCount, USHORT(nToWhichTab+1) );
    USHORT nStart = nTabsTested;
    if (!bValid)
    {
        nStart = 0;
        nTotalPages = 0;
        nTabsTested = 0;
    }

    //  PrintOptions is passed to PrintFunc for SkipEmpty flag,
    //  but always all sheets are used (there is no selected sheet)
    ScPrintOptions aOptions = SC_MOD()->GetPrintOptions();

    for (i=nStart; i<nAnz; i++)
    {
        long nAttrPage = i ? nFirstAttr[i-1] : 1;

        long nThisStart = nTotalPages;
        ScPrintFunc aPrintFunc( this, pDocShell, i, nAttrPage, 0, NULL, &aOptions );
        long nThisTab = aPrintFunc.GetTotalPages();
        nPages[i] = nThisTab;
        nTotalPages += nThisTab;
        nFirstAttr[i] = aPrintFunc.GetFirstPageNo();    // behalten oder aus Vorlage

        if (nPageNo>=nThisStart && nPageNo<nTotalPages)
        {
            nTab = i;
            nTabPage = nPageNo - nThisStart;
            nTabStart = nThisStart;

            aPrintFunc.GetPrintState( aState );
            aPageSize = aPrintFunc.GetPageSize();
        }
    }

    nDisplayStart = lcl_GetDisplayStart( nTab, pDoc, nPages );

    if (nAnz > nTabsTested)
        nTabsTested = nAnz;

    //  testen, ob hinter letzter Seite

    if ( nTabsTested >= nTabCount )
        TestLastPage();

    aState.nDocPages = nTotalPages;

    bValid = TRUE;
    bStateValid = TRUE;
    DoInvalidate();
}


void ScPreview::RecalcPages()                   // nur nPageNo geaendert
{
    if (!bValid)
        return;                         // dann wird CalcPages aufgerufen

    USHORT nOldTab = nTab;
    USHORT i;

    BOOL bDone = FALSE;
    while (nPageNo >= nTotalPages && nTabsTested < nTabCount)
    {
        CalcPages( nTabsTested );
        bDone = TRUE;
    }

    if (!bDone)
    {
        long nPartPages = 0;
        for (i=0; i<nTabsTested; i++)
        {
            long nThisStart = nPartPages;
            nPartPages += nPages[i];

            if (nPageNo>=nThisStart && nPageNo<nPartPages)
            {
                nTab = i;
                nTabPage = nPageNo - nThisStart;
                nTabStart = nThisStart;

//              aPageSize = aPrintFunc.GetPageSize();
            }
        }

        ScDocument* pDoc = pDocShell->GetDocument();
        nDisplayStart = lcl_GetDisplayStart( nTab, pDoc, nPages );
    }

    TestLastPage();         // testen, ob hinter letzter Seite

    if ( nTab != nOldTab )
        bStateValid = FALSE;

    DoInvalidate();
}


void ScPreview::DoPrint( ScPreviewLocationData* pFillLocation )
{
    if (!bValid)
    {
        CalcPages(0);
        RecalcPages();
        UpdateDrawView();       // Tabelle evtl. geaendert
    }

    Fraction aPreviewZoom( nZoom, 100 );
    Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
    MapMode aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );

    BOOL bDoPrint = ( pFillLocation == NULL );
    BOOL bValidPage = ( nPageNo < nTotalPages );

    ScModule* pScMod = SC_MOD();
    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aBackColor( rColorCfg.GetColorValue(svtools::APPBACKGROUND).nColor );

    if ( bDoPrint && ( aOffset.X() < 0 || aOffset.Y() < 0 ) && bValidPage )
    {
        SetMapMode( aMMMode );
        SetLineColor();
        SetFillColor(aBackColor);

        Size aWinSize = GetOutputSize();
        if ( aOffset.X() < 0 )
            DrawRect(Rectangle( 0, 0, -aOffset.X(), aWinSize.Height() ));
        if ( aOffset.Y() < 0 )
            DrawRect(Rectangle( 0, 0, aWinSize.Width(), -aOffset.Y() ));
    }

    Size aPageSize;
    if ( bValidPage )
    {
        ScPrintOptions aOptions = pScMod->GetPrintOptions();

        ScPrintFunc* pPrintFunc;
        if (bStateValid)
            pPrintFunc = new ScPrintFunc( this, pDocShell, aState, &aOptions );
        else
            pPrintFunc = new ScPrintFunc( this, pDocShell, nTab, nFirstAttr[nTab], nTotalPages, NULL, &aOptions );

        pPrintFunc->SetOffset(aOffset);
        pPrintFunc->SetManualZoom(nZoom);
        pPrintFunc->SetDateTime(aDate,aTime);
        pPrintFunc->SetClearFlag(TRUE);
        pPrintFunc->SetUseStyleColor( pScMod->GetAccessOptions().GetIsForPagePreviews() );

        pPrintFunc->SetDrawView( pDrawView );

        // MultiSelection fuer die eine Seite muss etwas umstaendlich erzeugt werden...
        Range aPageRange( nPageNo+1, nPageNo+1 );
        MultiSelection aPage( aPageRange );
        aPage.SetTotalRange( Range(0,RANGE_MAX) );
        aPage.Select( aPageRange );

        long nPrinted = pPrintFunc->DoPrint( aPage, nTabStart, nDisplayStart, bDoPrint, NULL, pFillLocation );
        DBG_ASSERT(nPrinted<=1, "was'n nu los?");

        SetMapMode(aMMMode);
//      USHORT nPrintZoom = pPrintFunc->GetZoom();

        if (nPrinted)   // wenn nichts, alles grau zeichnen
        {
            aPageSize = pPrintFunc->GetPageSize();
            aPageSize.Width()  = (long) (aPageSize.Width()  * HMM_PER_TWIPS );
            aPageSize.Height() = (long) (aPageSize.Height() * HMM_PER_TWIPS );
        }

        if (!bStateValid)
        {
            pPrintFunc->GetPrintState( aState );
            aState.nDocPages = nTotalPages;
            bStateValid = TRUE;
        }
        delete pPrintFunc;
    }

    if ( bDoPrint )
    {
        long nPageEndX = aPageSize.Width()  - aOffset.X();
        long nPageEndY = aPageSize.Height() - aOffset.Y();
        if ( !bValidPage )
            nPageEndX = nPageEndY = 0;

        Size aWinSize = GetOutputSize();
        Point aWinEnd( aWinSize.Width(), aWinSize.Height() );
        BOOL bRight  = nPageEndX <= aWinEnd.X();
        BOOL bBottom = nPageEndY <= aWinEnd.Y();
        if (bRight || bBottom)
        {
            SetLineColor();
            SetFillColor(aBackColor);
            if (bRight)
                DrawRect(Rectangle(nPageEndX,0, aWinEnd.X(),aWinEnd.Y()));
            if (bBottom)
            {
                if (bRight)
                    DrawRect(Rectangle(0,nPageEndY, nPageEndX,aWinEnd.Y()));    // Ecke nicht doppelt
                else
                    DrawRect(Rectangle(0,nPageEndY, aWinEnd.X(),aWinEnd.Y()));
            }
        }

        if ( bValidPage )
        {
            Color aBorderColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

            //  draw border

            if ( aOffset.X() <= 0 || aOffset.Y() <= 0 || bRight || bBottom )
            {
                SetLineColor( aBorderColor );
                SetFillColor();

                Rectangle aPixel( LogicToPixel( Rectangle( -aOffset.X(), -aOffset.Y(), nPageEndX, nPageEndY ) ) );
                --aPixel.Right();
                --aPixel.Bottom();
                DrawRect( PixelToLogic( aPixel ) );
            }

            //  draw shadow

            SetLineColor();
            SetFillColor( aBorderColor );

            Rectangle aPixel;

            aPixel = LogicToPixel( Rectangle( nPageEndX, -aOffset.Y(), nPageEndX, nPageEndY ) );
            aPixel.Top() += SC_PREVIEW_SHADOWSIZE;
            aPixel.Right() += SC_PREVIEW_SHADOWSIZE - 1;
            aPixel.Bottom() += SC_PREVIEW_SHADOWSIZE - 1;
            DrawRect( PixelToLogic( aPixel ) );

            aPixel = LogicToPixel( Rectangle( -aOffset.X(), nPageEndY, nPageEndX, nPageEndY ) );
            aPixel.Left() += SC_PREVIEW_SHADOWSIZE;
            aPixel.Right() += SC_PREVIEW_SHADOWSIZE - 1;
            aPixel.Bottom() += SC_PREVIEW_SHADOWSIZE - 1;
            DrawRect( PixelToLogic( aPixel ) );
        }
    }
}


void __EXPORT ScPreview::Paint( const Rectangle& rRect )
{
    DoPrint( NULL );
    pViewShell->UpdateScrollBars();
}


void __EXPORT ScPreview::Command( const CommandEvent& rCEvt )
{
    USHORT nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_WHEEL || nCmd == COMMAND_STARTAUTOSCROLL || nCmd == COMMAND_AUTOSCROLL )
    {
        BOOL bDone = pViewShell->ScrollCommand( rCEvt );
        if (!bDone)
            Window::Command(rCEvt);
    }
    else if ( nCmd == COMMAND_CONTEXTMENU )
        SfxDispatcher::ExecutePopup();
    else
        Window::Command( rCEvt );
}


void __EXPORT ScPreview::KeyInput( const KeyEvent& rKEvt )
{
    //  The + and - keys can't be configured as accelerator entries, so they must be handled directly
    //  (in ScPreview, not ScPreviewShell -> only if the preview window has the focus)

    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    USHORT nKey = rKeyCode.GetCode();
    BOOL bHandled = FALSE;
    if(!rKeyCode.GetModifier())
    {
        USHORT nSlot = 0;
        switch(nKey)
        {
            case KEY_ADD:      nSlot = SID_PREVIEW_ZOOMIN;  break;
            case KEY_SUBTRACT: nSlot = SID_PREVIEW_ZOOMOUT; break;
        }
        if(nSlot)
        {
            bHandled = TRUE;
            pViewShell->GetViewFrame()->GetDispatcher()->Execute( nSlot, SFX_CALLMODE_ASYNCHRON );
        }
    }

    if ( !bHandled && !pViewShell->KeyInput(rKEvt) )
        Window::KeyInput(rKEvt);
}


const ScPreviewLocationData& ScPreview::GetLocationData()
{
    if ( !pLocationData )
    {
        pLocationData = new ScPreviewLocationData( pDocShell->GetDocument(), this );
        bLocationValid = FALSE;
    }
    if ( !bLocationValid )
    {
        pLocationData->Clear();
        DoPrint( pLocationData );
        bLocationValid = TRUE;
    }
    return *pLocationData;
}


void ScPreview::DataChanged(BOOL bNewTime)
{
    if (bNewTime)
    {
        aDate = Date();
        aTime = Time();
    }

    bValid = FALSE;
    InvalidateLocationData( SC_HINT_DATACHANGED );
    Invalidate();
}


String ScPreview::GetPosString()
{
    if (!bValid)
    {
        CalcPages(nTab);
        UpdateDrawView();       // Tabelle evtl. geaendert
    }

    String aString( ScGlobal::GetRscString( STR_PAGE ) );
    aString += ' ';
    aString += String::CreateFromInt32(nPageNo+1);

    if (nTabsTested >= nTabCount)
    {
        aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " / " ));
        aString += String::CreateFromInt32(nTotalPages);
    }

    return aString;
}


void ScPreview::SetZoom(USHORT nNewZoom)
{
    if (nNewZoom < 20)
        nNewZoom = 20;
    if (nNewZoom > 400)
        nNewZoom = 400;
    if (nNewZoom != nZoom)
    {
        double nFact = nNewZoom / (double) nZoom;
        nZoom = nNewZoom;

        //  apply new MapMode and call UpdateScrollBars to update aOffset

        Fraction aPreviewZoom( nZoom, 100 );
        Fraction aHorPrevZoom( (long)( 100 * nZoom / pDocShell->GetOutputFactor() ), 10000 );
        MapMode aMMMode( MAP_100TH_MM, Point(), aHorPrevZoom, aPreviewZoom );
        SetMapMode( aMMMode );

        bInPaint = TRUE;                // don't scroll during SetYOffset in UpdateScrollBars
        pViewShell->UpdateScrollBars();
        bInPaint = FALSE;

        bStateValid = FALSE;
        InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
        DoInvalidate();
        Invalidate();
    }
}


void ScPreview::SetPageNo( long nPage )
{
    nPageNo = nPage;
    RecalcPages();
    UpdateDrawView();       // Tabelle evtl. geaendert
    InvalidateLocationData( SC_HINT_DATACHANGED );
    Invalidate();
}


long ScPreview::GetFirstPage(USHORT nTab)
{
    USHORT nTabCount = pDocShell->GetDocument()->GetTableCount();
    if (nTab >= nTabCount)
        nTab = nTabCount-1;

    long nPage = 0;
    if (nTab>0)
    {
        CalcPages( nTab );
        UpdateDrawView();       // Tabelle evtl. geaendert

        for (USHORT i=0; i<nTab; i++)
            nPage += nPages[i];

        // bei leerer Tabelle vorhergehende Seite

        if ( nPages[nTab]==0 && nPage > 0 )
            --nPage;
    }

    return nPage;
}


Size lcl_GetDocPageSize( ScDocument* pDoc, USHORT nTab )
{
    String aName = pDoc->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aName, SFX_STYLE_FAMILY_PAGE );
    if ( pStyleSheet )
    {
        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
        return ((const SvxSizeItem&) rStyleSet.Get(ATTR_PAGE_SIZE)).GetSize();
    }
    else
    {
        DBG_ERROR( "PageStyle not found" );
        return Size();
    }
}


USHORT ScPreview::GetOptimalZoom(BOOL bWidthOnly)
{
    double nWinScaleX = ScGlobal::nScreenPPTX / pDocShell->GetOutputFactor();
    double nWinScaleY = ScGlobal::nScreenPPTY;
    Size aWinSize = GetOutputSizePixel();

    //  desired margin is 0.25cm in default MapMode (like Writer),
    //  but some additional margin is introduced by integer scale values
    //  -> add only 0.10cm, so there is some margin in all cases.
    Size aMarginSize( LogicToPixel( Size( 100, 100 ), MAP_100TH_MM ) );
    aWinSize.Width()  -= 2 * aMarginSize.Width();
    aWinSize.Height() -= 2 * aMarginSize.Height();

    Size aPageSize = lcl_GetDocPageSize( pDocShell->GetDocument(), nTab );
    if ( aPageSize.Width() && aPageSize.Height() )
    {
        long nZoomX = (long) ( aWinSize.Width() * 100  / ( aPageSize.Width() * nWinScaleX ));
        long nZoomY = (long) ( aWinSize.Height() * 100 / ( aPageSize.Height() * nWinScaleY ));

        long nOptimal = nZoomX;
        if (!bWidthOnly && nZoomY<nOptimal)
            nOptimal = nZoomY;

        if (nOptimal<20)
            nOptimal = 20;
        if (nOptimal>400)
            nOptimal = 400;

        return (USHORT) nOptimal;
    }
    else
        return nZoom;
}


void ScPreview::SetXOffset( long nX )
{
    if ( aOffset.X() == nX )
        return;

    if (bValid)
    {
        long nDif = LogicToPixel(aOffset).X() - LogicToPixel(Point(nX,0)).X();
        aOffset.X() = nX;
        if (nDif && !bInPaint)
        {
            MapMode aOldMode = GetMapMode(); SetMapMode(MAP_PIXEL);
            Scroll( nDif, 0 );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.X() = nX;
        if (!bInPaint)
            Invalidate();
    }
    InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
}


void ScPreview::SetYOffset( long nY )
{
    if ( aOffset.Y() == nY )
        return;

    if (bValid)
    {
        long nDif = LogicToPixel(aOffset).Y() - LogicToPixel(Point(0,nY)).Y();
        aOffset.Y() = nY;
        if (nDif && !bInPaint)
        {
            MapMode aOldMode = GetMapMode(); SetMapMode(MAP_PIXEL);
            Scroll( 0, nDif );
            SetMapMode(aOldMode);
        }
    }
    else
    {
        aOffset.Y() = nY;
        if (!bInPaint)
            Invalidate();
    }
    InvalidateLocationData( SC_HINT_ACC_VISAREACHANGED );
}


void ScPreview::DoInvalidate()
{
    //  Wenn das ganze aus dem GetState der Shell gerufen wird,
    //  muss das Invalidate hinterher asynchron kommen...

    if (bInGetState)
        Application::PostUserEvent( STATIC_LINK( this, ScPreview, InvalidateHdl ) );
    else
        StaticInvalidate();     // sofort
}

void ScPreview::StaticInvalidate()
{
    //  static method, because it's called asynchronously
    //  -> must use current viewframe

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    SfxBindings& rBindings = pViewFrm->GetBindings();
    rBindings.Invalidate(SID_STATUS_DOCPOS);
    rBindings.Invalidate(SID_STATUS_PAGESTYLE);
    rBindings.Invalidate(SID_PREVIEW_PREVIOUS);
    rBindings.Invalidate(SID_PREVIEW_NEXT);
    rBindings.Invalidate(SID_PREVIEW_FIRST);
    rBindings.Invalidate(SID_PREVIEW_LAST);
    rBindings.Invalidate(SID_ATTR_ZOOM);
    rBindings.Invalidate(SID_PREVIEW_ZOOMIN);
    rBindings.Invalidate(SID_PREVIEW_ZOOMOUT);
}

IMPL_STATIC_LINK( ScPreview, InvalidateHdl, void*, EMPTYARG )
{
    StaticInvalidate();
    return 0;
}

void ScPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( (rDCEvt.GetType() == DATACHANGED_PRINTER) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( rDCEvt.GetType() == DATACHANGED_FONTS )
            pDocShell->UpdateFontList();

        if ( rDCEvt.GetType() == DATACHANGED_SETTINGS &&
              (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        {
            //  scroll bar size may have changed
            pViewShell->InvalidateBorder();     // calls OuterResizePixel
        }

        Invalidate();
        InvalidateLocationData( SC_HINT_DATACHANGED );
    }
}

void ScPreview::InvalidateLocationData(ULONG nId)
{
    bLocationValid = FALSE;
    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( SfxSimpleHint( nId ) );
}

void ScPreview::GetFocus()
{
    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusGotHint(GetAccessible()) );
}

void ScPreview::LoseFocus()
{
    if (pViewShell->HasAccessibilityObjects())
        pViewShell->BroadcastAccessibility( ScAccWinFocusLostHint(GetAccessible()) );
}

com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible> ScPreview::CreateAccessible()
{
    ScAccessibleDocumentPagePreview* pAccessible =
        new ScAccessibleDocumentPagePreview( GetAccessibleParentWindow()->GetAccessible(), pViewShell );
    com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xAccessible = pAccessible;
    pAccessible->Init();
    return xAccessible;
}


