/*************************************************************************
 *
 *  $RCSfile: tabview5.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 10:15:29 $
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
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/fmshell.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <tools/ref.hxx>

#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "tabsplit.hxx"
#include "colrowba.hxx"
#include "tabcont.hxx"
#include "hintwin.hxx"
#include "sc.hrc"
#include "pagedata.hxx"
#include "hiranges.hxx"
#include "drawview.hxx"
#include "drwlayer.hxx"
#include "fusel.hxx"                // Start-Function
#include "seltrans.hxx"
#include "scmod.hxx"
#include "AccessibilityHints.hxx"


// STATIC DATA -----------------------------------------------------------


void __EXPORT ScTabView::Init()
{
    //  RTL layout of the view windows is done manually, because it depends on the
    //  sheet orientation, not the UI setting
    pFrameWin->EnableRTL( FALSE );

    USHORT i;

    aScrollTimer.SetTimeout(10);
    aScrollTimer.SetTimeoutHdl( LINK( this, ScTabView, TimerHdl ) );

    for (i=0; i<4; i++)
        pGridWin[i] = NULL;
    pGridWin[SC_SPLIT_BOTTOMLEFT] = new ScGridWindow( pFrameWin, &aViewData, SC_SPLIT_BOTTOMLEFT );

    pSelEngine = new ScViewSelectionEngine( pGridWin[SC_SPLIT_BOTTOMLEFT], this,
                                                SC_SPLIT_BOTTOMLEFT );
    aFunctionSet.SetSelectionEngine( pSelEngine );

    pHdrSelEng = new ScHeaderSelectionEngine( pFrameWin, &aHdrFunc );

    pColBar[SC_SPLIT_LEFT] = new ScColBar( pFrameWin, &aViewData, SC_SPLIT_LEFT,
                                                &aHdrFunc, pHdrSelEng );
    pColBar[SC_SPLIT_RIGHT] = NULL;
    pRowBar[SC_SPLIT_BOTTOM] = new ScRowBar( pFrameWin, &aViewData, SC_SPLIT_BOTTOM,
                                                &aHdrFunc, pHdrSelEng );
    pRowBar[SC_SPLIT_TOP] = NULL;
    for (i=0; i<2; i++)
        pColOutline[i] = pRowOutline[i] = NULL;

    pHSplitter = new ScTabSplitter( pFrameWin, WinBits( WB_HSCROLL ), &aViewData );
    pVSplitter = new ScTabSplitter( pFrameWin, WinBits( WB_VSCROLL ), &aViewData );

    // SSA: override default keyboard step size to allow snap to row/column
    pHSplitter->SetKeyboardStepSize( 1 );
    pVSplitter->SetKeyboardStepSize( 1 );

    pTabControl = new ScTabControl( pFrameWin, &aViewData );
    //MI: nie! das war mal eine MUSS-Aenderung von MBA
    //if (!aViewData.IsBasicView())
    //  pTabControl->ToTop();       // ueber dem Splitter

    InitScrollBar( aHScrollLeft,    MAXCOL+1 );
    InitScrollBar( aHScrollRight,   MAXCOL+1 );
    InitScrollBar( aVScrollTop,     MAXROW+1 );
    InitScrollBar( aVScrollBottom,  MAXROW+1 );

    // SSA: --- RTL --- no mirroring for horizontal scrollbars, otherwise
    // scroll direction will be wrong
    aHScrollLeft.EnableRTL ( FALSE );
    aHScrollRight.EnableRTL ( FALSE );

    //  Mirroring is disabled for all scrollbars, completely handled manually.
    //  The other windows in the view call EnableRTL in their ctors.
    aVScrollTop.EnableRTL( FALSE );
    aVScrollBottom.EnableRTL( FALSE );
    aScrollBarBox.EnableRTL( FALSE );

    // Tabbar initially left-to-right, done via SetMirrored(), not via EnableRTL()
    pTabControl->SetMirrored( Application::GetSettings().GetLayoutRTL() );

    //  Hier noch nichts anzeigen (Show), weil noch falsch angeordnet ist
    //  Show kommt dann aus UpdateShow beim ersten Resize
    //      pTabControl, pGridWin, aHScrollLeft, aVScrollBottom,
    //      aCornerButton, aScrollBarBox, pHSplitter, pVSplitter

    //      Splitter

    pHSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );
    pVSplitter->SetSplitHdl( LINK( this, ScTabView, SplitHdl ) );

    //  UpdateShow kommt beim Resize, oder bei Kopie einer bestehenden View aus dem ctor

    pDrawActual = NULL;
    pDrawOld    = NULL;

            //  DrawView darf nicht im TabView - ctor angelegt werden,
            //  wenn die ViewShell noch nicht konstruiert ist...
            //  Das gilt auch fuer ViewOptionsHasChanged()

    TestHintWindow();
}

__EXPORT ScTabView::~ScTabView()
{
    USHORT i;

    //  remove selection object
    ScModule* pScMod = SC_MOD();
    ScSelectionTransferObj* pOld = pScMod->GetSelectionTransfer();
    if ( pOld && pOld->GetView() == this )
    {
        pOld->ForgetView();
        pScMod->SetSelectionTransfer( NULL );
        TransferableHelper::ClearSelection( GetActiveWin() );       // may delete pOld
    }

    DELETEZ(pBrushDocument);
    DELETEZ(pDrawBrushSet);

    DELETEZ(pPageBreakData);
    DELETEZ(pHighlightRanges);

    DELETEZ(pDrawOld);
    DELETEZ(pDrawActual);

    aViewData.KillEditView();           // solange GridWin's noch existieren

    DELETEZ(pInputHintWindow);

    if (pDrawView)
    {
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                pDrawView->VCRemoveWin(pGridWin[i]);
                pDrawView->DelWin(pGridWin[i]);
            }

        pDrawView->HidePagePvNum(0);
        delete pDrawView;
    }

    delete pSelEngine;

    for (i=0; i<4; i++)
        delete pGridWin[i];

    delete pHdrSelEng;

    for (i=0; i<2; i++)
    {
        delete pColBar[i];
        delete pRowBar[i];
        delete pColOutline[i];
        delete pRowOutline[i];
    }

    delete pHSplitter;
    delete pVSplitter;

    delete pTabControl;
}

void ScTabView::MakeDrawView( BYTE nForceDesignMode )
{
    if (!pDrawView)
    {
        ScDrawLayer* pLayer = aViewData.GetDocument()->GetDrawLayer();
        DBG_ASSERT(pLayer, "wo ist der DrawLayer ??");

        USHORT i;
        pDrawView = new ScDrawView( pGridWin[SC_SPLIT_BOTTOMLEFT], &aViewData );
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                if ( SC_SPLIT_BOTTOMLEFT != (ScSplitPos)i )
                    pDrawView->AddWin(pGridWin[i]);
                pDrawView->VCAddWin(pGridWin[i]);
            }
        pDrawView->RecalcScale();
        for (i=0; i<4; i++)
            if (pGridWin[i])
            {
                pGridWin[i]->SetMapMode(pGridWin[i]->GetDrawMapMode());

                pGridWin[i]->Update();      // wegen Invalidate im DrawView ctor (ShowPage),
                                            // damit gleich gezeichnet werden kann
            }
        SfxRequest aSfxRequest(SID_OBJECT_SELECT, 0,aViewData.GetViewShell()->GetPool());
        SetDrawFuncPtr(new FuSelection( aViewData.GetViewShell(), GetActiveWin(), pDrawView,
                                        pLayer,aSfxRequest));

        //  #106334# used when switching back from page preview: restore saved design mode state
        //  (otherwise, keep the default from the draw view ctor)
        if ( nForceDesignMode != SC_FORCEMODE_NONE )
            pDrawView->SetDesignMode( (BOOL)nForceDesignMode );

        //  an der FormShell anmelden
        FmFormShell* pFormSh = aViewData.GetViewShell()->GetFormShell();
        if (pFormSh)
            pFormSh->SetView(pDrawView);

        if (aViewData.GetViewShell()->HasAccessibilityObjects())
            aViewData.GetViewShell()->BroadcastAccessibility(SfxSimpleHint(SC_HINT_ACC_MAKEDRAWLAYER));

    }
}

void ScTabView::DoAddWin( ScGridWindow* pWin )
{
    if (pDrawView)
    {
        pDrawView->AddWin(pWin);
        pDrawView->VCAddWin(pWin);
    }
}

//==================================================================

void ScTabView::TabChanged()
{
    if (pDrawView)
    {
        DrawDeselectAll();      // beendet auch Text-Edit-Modus

        USHORT i;
        for (i=0; i<4; i++)
            if (pGridWin[i])
                pDrawView->VCRemoveWin(pGridWin[i]);    // fuer alte Page

        SCTAB nTab = aViewData.GetTabNo();
        pDrawView->HideAllPages();
        pDrawView->ShowPagePgNum( static_cast<sal_uInt16>(nTab), Point() );

        UpdateLayerLocks();

        pDrawView->RecalcScale();
        pDrawView->UpdateWorkArea();    // #54782# PageSize ist pro Page unterschiedlich

        for (i=0; i<4; i++)
            if (pGridWin[i])
                pDrawView->VCAddWin(pGridWin[i]);       // fuer neue Page
    }

    SfxBindings& rBindings = aViewData.GetBindings();

    //  Es gibt keine einfache Moeglichkeit, alle Slots der FormShell zu invalidieren
    //  (fuer disablete Slots auf geschuetzten Tabellen), darum hier einfach alles...
    rBindings.InvalidateAll(FALSE);

#if 0
    rBindings.Invalidate( SID_SELECT_SCENARIO );
    rBindings.Invalidate( FID_PROTECT_TABLE );
    rBindings.Invalidate( FID_DELETE_TABLE );
    rBindings.Invalidate( FID_TABLE_SHOW );
    rBindings.Invalidate( FID_TABLE_HIDE );

                // Auswirkungen von geschuetzten Tabellen.
    rBindings.Invalidate( FID_TAB_RENAME );
    rBindings.Invalidate( FID_TAB_MOVE );
    rBindings.Invalidate( SID_DEL_ROWS );
    rBindings.Invalidate( SID_DEL_COLS );
    rBindings.Invalidate( FID_INS_ROW );
    rBindings.Invalidate( FID_INS_COLUMN );
    rBindings.Invalidate( FID_INS_CELL );
    rBindings.Invalidate( FID_INS_CELLSDOWN );
    rBindings.Invalidate( FID_INS_CELLSRIGHT );
    rBindings.Invalidate( FID_DELETE_CELL );

    rBindings.Invalidate( SID_OPENDLG_CHART );
    rBindings.Invalidate( SID_INSERT_OBJECT );
    rBindings.Invalidate( SID_INSERT_DIAGRAM );
    rBindings.Invalidate( SID_INSERT_SMATH );
    rBindings.Invalidate( SID_INSERT_GRAPHIC );
#endif

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
    {
        SfxSimpleHint aAccHint(SC_HINT_ACC_TABLECHANGED);
        aViewData.GetViewShell()->BroadcastAccessibility(aAccHint);
    }
    aViewData.GetDocument()->BroadcastUno( SfxSimpleHint( SC_HINT_TABLECHANGED ) );
}

void ScTabView::UpdateLayerLocks()
{
    if (pDrawView)
    {
        SCTAB nTab = aViewData.GetTabNo();
        BOOL bEx = aViewData.GetViewShell()->IsDrawSelMode();
        BOOL bProt = aViewData.GetDocument()->IsTabProtected( nTab ) ||
                     aViewData.GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = pDrawView->GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), TRUE );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_FRONT);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
        if (pLayer)
            pDrawView->SetLayerLocked( pLayer->GetName(), bProt );
    }
}

void ScTabView::DrawDeselectAll()
{
    if (pDrawView)
    {
        ScTabViewShell* pViewSh = aViewData.GetViewShell();
        if ( pDrawActual &&
            ( pViewSh->IsDrawTextShell() || pDrawActual->GetSlotID() == SID_DRAW_NOTEEDIT ) )
        {
            // end text edit (as if escape pressed, in FuDraw)
            aViewData.GetDispatcher().Execute( pDrawActual->GetSlotID(),
                                        SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD );
        }

        pDrawView->ScEndTextEdit();
        pDrawView->UnmarkAll();

        if (!pViewSh->IsDrawSelMode())
            pViewSh->SetDrawShell( FALSE );
    }
}

BOOL ScTabView::IsDrawTextEdit() const
{
    if (pDrawView)
        return pDrawView->IsTextEdit();
    else
        return FALSE;
}

String ScTabView::GetSelectedChartName() const
{
    if (pDrawView)
        return pDrawView->GetSelectedChartName();
    else
        return EMPTY_STRING;
}

void ScTabView::SetZoom( const Fraction& rNewX, const Fraction& rNewY )
{
    aViewData.SetZoom(rNewX,rNewY);
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();              // einzeln wegen CLOOKs
}

void ScTabView::SetPagebreakMode( BOOL bSet )
{
    aViewData.SetPagebreakMode(bSet);
    if (pDrawView)
        pDrawView->RecalcScale();
    ZoomChanged();              // einzeln wegen CLOOKs
}

void ScTabView::ResetDrawDragMode()
{
    if (pDrawView)
        pDrawView->SetDragMode( SDRDRAG_MOVE );
}

void ScTabView::ViewOptionsHasChanged( BOOL bHScrollChanged, BOOL bGraphicsChanged )
{
    //  DrawView erzeugen, wenn Gitter angezeigt werden soll
    if ( !pDrawView && aViewData.GetOptions().GetGridOptions().GetGridVisible() )
        MakeDrawLayer();

    if (pDrawView)
        pDrawView->UpdateUserViewOptions();

    if (bGraphicsChanged)
        DrawEnableAnim(TRUE);   // DrawEnableAnim checks the options state

    // if TabBar is set to visible, make sure its size is not 0
    BOOL bGrow = ( aViewData.IsTabMode() && pTabControl->GetSizePixel().Width() <= 0 );

    // if ScrollBar is set to visible, TabBar must make room
    BOOL bShrink = ( bHScrollChanged && aViewData.IsTabMode() && aViewData.IsHScrollMode() &&
                        pTabControl->GetSizePixel().Width() > SC_TABBAR_DEFWIDTH );

    if ( bGrow || bShrink )
    {
        Size aSize = pTabControl->GetSizePixel();
        aSize.Width() = SC_TABBAR_DEFWIDTH;             // initial size
        pTabControl->SetSizePixel(aSize);               // DoResize is called later...
    }
}

// Helper-Funktion gegen das Include des Drawing Layers

SdrView* ScTabView::GetSdrView()
{
    return pDrawView;
}

void ScTabView::DrawMarkListHasChanged()
{
    if ( pDrawView )
        pDrawView->MarkListHasChanged();
}

void ScTabView::UpdateAnchorHandles()
{
    if ( pDrawView )
        pDrawView->AdjustMarkHdl();
}

void ScTabView::UpdateIMap( SdrObject* pObj )
{
    if ( pDrawView )
        pDrawView->UpdateIMap( pObj );
}

void ScTabView::DrawMarkRect( const Rectangle& rRect )
{
    //! store rectangle for repaint during drag

    for (USHORT i=0; i<4; i++)
    {
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
        {
            RasterOp aROp = pGridWin[i]->GetRasterOp();
            BOOL bHasLine = pGridWin[i]->IsLineColor();
            Color aLine   = pGridWin[i]->GetLineColor();
            BOOL bHasFill = pGridWin[i]->IsFillColor();
            Color aFill   = pGridWin[i]->GetFillColor();

            pGridWin[i]->SetRasterOp( ROP_INVERT );
            pGridWin[i]->SetLineColor( COL_BLACK );
            pGridWin[i]->SetFillColor();

            pGridWin[i]->DrawRect(rRect);

            pGridWin[i]->SetRasterOp(aROp);
            if (bHasLine)
                pGridWin[i]->SetLineColor(aLine);
            else
                pGridWin[i]->SetLineColor();
            if (bHasFill)
                pGridWin[i]->SetFillColor(aFill);
            else
                pGridWin[i]->SetFillColor();
        }
    }
}

void ScTabView::DrawEnableAnim(BOOL bSet)
{
    USHORT i;
    if ( pDrawView )
    {
        //  #71040# dont start animations if display of graphics is disabled
        //  graphics are controlled by VOBJ_TYPE_OLE
        if ( bSet && aViewData.GetOptions().GetObjMode(VOBJ_TYPE_OLE) == VOBJ_MODE_SHOW )
        {
            if ( !pDrawView->IsAnimationEnabled() )
            {
                pDrawView->SetAnimationEnabled(TRUE);

                //  Animierte GIFs muessen wieder gestartet werden:
                ScDocument* pDoc = aViewData.GetDocument();
                for (i=0; i<4; i++)
                    if ( pGridWin[i] && pGridWin[i]->IsVisible() )
                        pDoc->StartAnimations( aViewData.GetTabNo(), pGridWin[i] );
            }
        }
        else
        {
            pDrawView->SetAnimationEnabled(FALSE);
            //  StopAnimations am Dokument ist nicht mehr noetig
        }
    }
}

void ScTabView::DrawShowMarkHdl(BOOL bShow)
{
    if (!pDrawView)
        return;

    if (bShow)
    {
        if (!pDrawView->IsDisableHdl())
            pDrawView->ShowMarkHdl(NULL);
    }
    else
        pDrawView->HideMarkHdl(NULL);
}

void ScTabView::UpdateDrawTextOutliner()
{
    if ( pDrawView )
    {
        Outliner* pOL = pDrawView->GetTextEditOutliner();
        if (pOL)
            aViewData.UpdateOutlinerFlags( *pOL );
    }
}

void ScTabView::DigitLanguageChanged()
{
    LanguageType eNewLang = SC_MOD()->GetOptDigitLanguage();
    for (USHORT i=0; i<4; i++)
        if ( pGridWin[i] )
            pGridWin[i]->SetDigitLanguage( eNewLang );
}

//---------------------------------------------------------------

void ScTabView::ScrollToObject( SdrObject* pDrawObj )
{
    if ( pDrawObj )
        MakeVisible( pDrawObj->GetLogicRect() );
}

void ScTabView::MakeVisible( const Rectangle& rHMMRect )
{
    Window* pWin = GetActiveWin();
    Size aWinSize = pWin->GetOutputSizePixel();
    SCTAB nTab = aViewData.GetTabNo();

    Rectangle aRect = pWin->LogicToPixel( rHMMRect );

    long nScrollX=0, nScrollY=0;        // Pixel

    if ( aRect.Right() >= aWinSize.Width() )                // rechts raus
    {
        nScrollX = aRect.Right() - aWinSize.Width() + 1;    // rechter Rand sichtbar
        if ( aRect.Left() < nScrollX )
            nScrollX = aRect.Left();                        // links sichtbar (falls zu gross)
    }
    if ( aRect.Bottom() >= aWinSize.Height() )              // unten raus
    {
        nScrollY = aRect.Bottom() - aWinSize.Height() + 1;  // unterer Rand sichtbar
        if ( aRect.Top() < nScrollY )
            nScrollY = aRect.Top();                         // oben sichtbar (falls zu gross)
    }

    if ( aRect.Left() < 0 )             // links raus
        nScrollX = aRect.Left();        // linker Rand sichtbar
    if ( aRect.Top() < 0 )              // oben raus
        nScrollY = aRect.Top();         // oberer Rand sichtbar

    if (nScrollX || nScrollY)
    {
        ScDocument* pDoc = aViewData.GetDocument();
        if ( pDoc->IsNegativePage( nTab ) )
            nScrollX = -nScrollX;

        double nPPTX = aViewData.GetPPTX();
        double nPPTY = aViewData.GetPPTY();
        ScSplitPos eWhich = aViewData.GetActivePart();
        SCCOL nPosX = aViewData.GetPosX(WhichH(eWhich));
        SCROW nPosY = aViewData.GetPosY(WhichV(eWhich));

        long nLinesX=0, nLinesY=0;      // Spalten/Zeilen - um mindestens nScrollX/Y scrollen

        if (nScrollX > 0)
            while (nScrollX > 0 && nPosX < MAXCOL)
            {
                nScrollX -= (long) ( pDoc->GetColWidth(nPosX, nTab) * nPPTX );
                ++nPosX;
                ++nLinesX;
            }
        else if (nScrollX < 0)
            while (nScrollX < 0 && nPosX > 0)
            {
                --nPosX;
                nScrollX += (long) ( pDoc->GetColWidth(nPosX, nTab) * nPPTX );
                --nLinesX;
            }

        if (nScrollY > 0)
            while (nScrollY > 0 && nPosY < MAXROW)
            {
                nScrollY -= (long) ( pDoc->FastGetRowHeight(nPosY, nTab) * nPPTY );
                ++nPosY;
                ++nLinesY;
            }
        else if (nScrollY < 0)
            while (nScrollY < 0 && nPosY > 0)
            {
                --nPosY;
                nScrollY += (long) ( pDoc->FastGetRowHeight(nPosY, nTab) * nPPTY );
                --nLinesY;
            }

        ScrollLines( nLinesX, nLinesY );                    // ausfuehren
    }
}

//---------------------------------------------------------------

void ScTabView::SetBrushDocument( ScDocument* pNew, BOOL bLock )
{
    delete pBrushDocument;
    delete pDrawBrushSet;

    pBrushDocument = pNew;
    pDrawBrushSet = NULL;

    bLockPaintBrush = bLock;

    aViewData.GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void ScTabView::SetDrawBrushSet( SfxItemSet* pNew, BOOL bLock )
{
    delete pBrushDocument;
    delete pDrawBrushSet;

    pBrushDocument = NULL;
    pDrawBrushSet = pNew;

    bLockPaintBrush = bLock;

    aViewData.GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void ScTabView::ResetBrushDocument()
{
    if ( HasPaintBrush() )
    {
        SetBrushDocument( NULL, FALSE );
        SetActivePointer( Pointer( POINTER_ARROW ) );   // switch pointers also when ended with escape key
    }
}


