/*************************************************************************
 *
 *  $RCSfile: gridwin4.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-22 19:34:13 $
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
#include <svx/editview.hxx>
#include <so3/ipenv.hxx>

#ifdef MAC
#include <svx/brshitem.hxx>
#endif

#include "gridwin.hxx"
#include "viewdata.hxx"
#include "output.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "patattr.hxx"          // InvertSimple
#include "dbcolect.hxx"
#include "docoptio.hxx"
#include "notemark.hxx"
#include "dbfunc.hxx"           // oder GetPageBreakData an die ViewData
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "rfindlst.hxx"
#include "hiranges.hxx"
#include "pagedata.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "docsh.hxx"            // oder GetSfxInPlaceObject
#include "cbutton.hxx"
#include "invmerge.hxx"

//#include "tabvwsh.hxx"            //! Test !!!!

//------------------------------------------------------------------------

void lcl_LimitRect( Rectangle& rRect, const Rectangle& rVisible )
{
    if ( rRect.Top()    < rVisible.Top()-1 )    rRect.Top()    = rVisible.Top()-1;
//  if ( rRect.Left()   < rVisible.Left()-1 )   rRect.Left()   = rVisible.Left()-1;
    if ( rRect.Bottom() > rVisible.Bottom()+1 ) rRect.Bottom() = rVisible.Bottom()+1;
//  if ( rRect.Right()  > rVisible.Right()+1 )  rRect.Right()  = rVisible.Right()+1;

    // #51122# auch wenn das inner-Rectangle nicht sichtbar ist, muss evtl.
    // die Titelzeile gezeichnet werden, darum kein Rueckgabewert mehr.
    // Wenn's weit daneben liegt, wird lcl_DrawOneFrame erst gar nicht gerufen.
}

void lcl_DrawOneFrame( OutputDevice* pDev, const Rectangle& rInnerPixel,
                        const String& rTitle, const Color& rColor, BOOL bTextBelow,
                        double nPPTX, double nPPTY, const Fraction& rZoomY,
                        ScDocument* pDoc, ScViewData* pButtonViewData )
{
    //  pButtonViewData wird nur benutzt, um die Button-Groesse zu setzen,
    //  darf ansonsten NULL sein!

    Rectangle aInner = rInnerPixel;
    Rectangle aVisible( Point(0,0), pDev->GetOutputSizePixel() );
    lcl_LimitRect( aInner, aVisible );

    Rectangle aOuter = aInner;
    long nHor = (long) ( SC_SCENARIO_HSPACE * nPPTX );
    long nVer = (long) ( SC_SCENARIO_VSPACE * nPPTY );
    aOuter.Left()   -= nHor;
    aOuter.Right()  += nHor;
    aOuter.Top()    -= nVer;
    aOuter.Bottom() += nVer;

    //  use ScPatternAttr::GetFont only for font size
    Font aAttrFont;
    ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
                                    GetFont(aAttrFont,pDev,&rZoomY);

    //  everything else from application font
    Font aAppFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
    aAppFont.SetSize( aAttrFont.GetSize() );

    aAppFont.SetAlign( ALIGN_TOP );
    pDev->SetFont( aAppFont );

    Size aTextSize( pDev->GetTextWidth( rTitle ), pDev->GetTextHeight() );

    if ( bTextBelow )
        aOuter.Bottom() += aTextSize.Height();
    else
        aOuter.Top()    -= aTextSize.Height();

    pDev->SetLineColor();
    pDev->SetFillColor( rColor );
    //  links, oben, rechts, unten
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aInner.Left(),  aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aOuter.Right(), aInner.Top()    ) );
    pDev->DrawRect( Rectangle( aInner.Right(), aOuter.Top(),    aOuter.Right(), aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aInner.Bottom(), aOuter.Right(), aOuter.Bottom() ) );

    long nButtonY = bTextBelow ? aInner.Bottom() : aOuter.Top();

    ScDDComboBoxButton aComboButton((Window*)pDev);
    aComboButton.SetOptSizePixel();
    long nBWidth  = ( aComboButton.GetSizePixel().Width() * rZoomY.GetNumerator() )
                        / rZoomY.GetDenominator();
    long nBHeight = nVer + aTextSize.Height() + 1;
    Size aButSize( nBWidth, nBHeight );
    aComboButton.Draw( Point(aOuter.Right()-nBWidth+1, nButtonY), aButSize, Color( COL_BLACK ) );
    if (pButtonViewData)
        pButtonViewData->SetScenButSize( aButSize );

    BOOL bWasClip = FALSE;
    Region aOldClip;
    BOOL bClip = ( aTextSize.Width() > aOuter.Right() - nBWidth - aInner.Left() );
    if ( bClip )
    {
        if (pDev->IsClipRegion())
        {
            bWasClip = TRUE;
            aOldClip = pDev->GetActiveClipRegion();
        }
        pDev->SetClipRegion( Rectangle( aInner.Left(), nButtonY + nVer/2,
                        aOuter.Right() - nBWidth, nButtonY + nVer/2 + aTextSize.Height() ) );
    }

    pDev->DrawText( Point( aInner.Left(), nButtonY + nVer/2 ), rTitle );

    if ( bClip )
    {
        if ( bWasClip )
            pDev->SetClipRegion(aOldClip);
        else
            pDev->SetClipRegion();
    }

    pDev->SetFillColor();
    pDev->SetLineColor( COL_BLACK );
    pDev->DrawRect( aInner );
    pDev->DrawRect( aOuter );
}

void lcl_DrawScenarioFrames( OutputDevice* pDev, ScViewData* pViewData, ScSplitPos eWhich,
                            USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();
    USHORT nTabCount = pDoc->GetTableCount();
    if ( nTab+1<nTabCount && pDoc->IsScenario(nTab+1) && !pDoc->IsScenario(nTab) )
    {
        if ( nX1 ) --nX1;
        if ( nY1>=2 ) nY1 -= 2;             // Hack: Titelzeile beruehrt zwei Zellen
        else if ( nY1 ) --nY1;
        if ( nX2 < MAXCOL ) ++nX2;
        if ( nY2 < MAXROW-1 ) nY2 += 2;     // Hack: Titelzeile beruehrt zwei Zellen
        else if ( nY2 < MAXROW ) ++nY2;
        ScRange aViewRange( nX1,nY1,nTab, nX2,nY2,nTab );

        //! Ranges an der Table cachen!!!!

        USHORT i;
        ScMarkData aMarks;
        for (i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            pDoc->MarkScenario( i, nTab, aMarks, FALSE, SC_SCENARIO_SHOWFRAME );
        ScRangeListRef xRanges = new ScRangeList;
        aMarks.FillRangeListWithMarks( xRanges, FALSE );

        USHORT nRangeCount = (USHORT)xRanges->Count();
        for (i=0; i<nRangeCount; i++)
        {
            ScRange aRange = *xRanges->GetObject(i);
            //  Szenario-Rahmen immer dann auf zusammengefasste Zellen erweitern, wenn
            //  dadurch keine neuen nicht-ueberdeckten Zellen mit umrandet werden
            pDoc->ExtendTotalMerge( aRange );

            //! -> Repaint beim Zusammenfassen erweitern !!!

            if ( aRange.Intersects( aViewRange ) )          //! Platz fuer Text/Button?
            {
                Point aStartPos = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, TRUE );
                Point aEndPos = pViewData->GetScrPos(
                                    aRange.aEnd.Col()+1, aRange.aEnd.Row()+1, eWhich, TRUE );
                //  auf dem Gitter:
                aStartPos.X() -= 1;
                aStartPos.Y() -= 1;
                aEndPos.X() -= 1;
                aEndPos.Y() -= 1;

                BOOL bTextBelow = ( aRange.aStart.Row() == 0 );

                String aCurrent;
                Color aColor( COL_LIGHTGRAY );
                for (USHORT nAct=nTab+1; nAct<nTabCount && pDoc->IsScenario(nAct); nAct++)
                    if ( pDoc->IsActiveScenario(nAct) && pDoc->HasScenarioRange(nAct,aRange) )
                    {
                        String aDummyComment;
                        USHORT nDummyFlags;
                        pDoc->GetName( nAct, aCurrent );
                        pDoc->GetScenarioData( nAct, aDummyComment, aColor, nDummyFlags );
                    }

                if (!aCurrent.Len())
                    aCurrent = ScGlobal::GetRscString( STR_EMPTYDATA );

                //! eigener Text "(keins)" statt "(leer)" ???

                lcl_DrawOneFrame( pDev, Rectangle( aStartPos, aEndPos ),
                                    aCurrent, aColor, bTextBelow,
                                    pViewData->GetPPTX(), pViewData->GetPPTY(), pViewData->GetZoomY(),
                                    pDoc, pViewData );
            }
        }
    }
}

//------------------------------------------------------------------------

void lcl_DrawHighlight( ScOutputData& rOutputData, ScViewData* pViewData,
                        ScHighlightRanges& rHighlightRanges )
{
    USHORT nTab = pViewData->GetTabNo();
    ULONG nCount = rHighlightRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScHighlightEntry* pEntry = rHighlightRanges.GetObject( i );
        if (pEntry)
        {
            ScRange aRange = pEntry->aRef;
            if ( nTab >= aRange.aStart.Tab() && nTab <= aRange.aEnd.Tab() )
            {
                rOutputData.DrawRefMark(
                                    aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(),
                                    pEntry->aColor, FALSE );
            }
        }
    }
}

//------------------------------------------------------------------------

void ScGridWindow::DoInvertRect( const Rectangle& rPixel )
{
    Invert( PixelToLogic(rPixel) );

    if ( rPixel == aInvertRect )
        aInvertRect = Rectangle();      // aufheben
    else
    {
        DBG_ASSERT( aInvertRect.IsEmpty(), "DoInvertRect nicht paarig" );

        aInvertRect = rPixel;           // neues Rechteck merken
    }
}

//------------------------------------------------------------------------

void __EXPORT ScGridWindow::Paint( const Rectangle& rRect )
{
    ScDocShell* pDocSh = pViewData->GetDocShell();
    SvInPlaceEnvironment* pEnv = pDocSh->GetIPEnv();
    if (pEnv && pEnv->GetRectsChangedLockCount())
    {
        Invalidate(rRect);
        return;
    }

    ScDocument* pDoc = pViewData->GetDocument();
    if ( pDoc->IsInInterpreter() )
    {
        //  via Reschedule, interpretierende Zellen nicht nochmal anstossen
        //  hier kein Invalidate, sonst kommt z.B. eine Error-Box nie an die Reihe
        //  (Bug 36381). Durch bNeedsRepaint wird spaeter alles nochmal gemalt.

        if ( bNeedsRepaint )
        {
            //! Rechtecke zusammenfassen?
            aRepaintPixel = Rectangle();            // mehrfach -> alles painten
        }
        else
        {
            bNeedsRepaint = TRUE;
            aRepaintPixel = LogicToPixel(rRect);    // nur betroffenen Bereich
        }
        return;
    }

    if (bIsInPaint)
        return;

    bIsInPaint = TRUE;

    Rectangle aPixRect = LogicToPixel( rRect );

    USHORT nX1 = pViewData->GetPosX(eHWhich);
    USHORT nY1 = pViewData->GetPosY(eVWhich);

    USHORT nTab = pViewData->GetTabNo();

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    long nScrX = (long) ( pDoc->GetColWidth( nX1, nTab ) * nPPTX );
    while ( nScrX <= aPixRect.Left() && nX1 < MAXCOL )
    {
        ++nX1;
        nScrX += (long) ( pDoc->GetColWidth( nX1, nTab ) * nPPTX );
    }
    USHORT nX2 = nX1;
    while ( nScrX <= aPixRect.Right() && nX2 < MAXCOL )
    {
        ++nX2;
        nScrX += (long) ( pDoc->GetColWidth( nX2, nTab ) * nPPTX );
    }

    long nScrY = (long) ( pDoc->GetRowHeight( nY1, nTab ) * nPPTY );
    while ( nScrY <= aPixRect.Top() && nY1 < MAXROW )
    {
        ++nY1;
        nScrY += (long) ( pDoc->GetRowHeight( nY1, nTab ) * nPPTY );
    }
    USHORT nY2 = nY1;
    while ( nScrY <= aPixRect.Bottom() && nY2 < MAXROW )
    {
        ++nY2;
        nScrY += (long) ( pDoc->GetRowHeight( nY2, nTab ) * nPPTY );
    }

    Draw( nX1,nY1,nX2,nY2, SC_UPDATE_MARKS );           // nicht weiterzeichnen

    OutlinerViewPaint( rRect );

    if (!aInvertRect.IsEmpty())
        Invert( PixelToLogic(aInvertRect) );    // auf das Clipping verlassen...

    bIsInPaint = FALSE;
}

//
//  Draw  ----------------------------------------------------------------
//

void ScGridWindow::Draw( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2, ScUpdateMode eMode )
{
    BOOL bGridFirst = TRUE;     //! entscheiden!!!

    if (pViewData->IsMinimized())
        return;

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );
    USHORT i;

    DBG_ASSERT( nX2<=MAXCOL && nY2<=MAXROW, "GridWin Draw Bereich zu gross" );

    USHORT nPosX = pViewData->GetPosX( eHWhich );
    USHORT nPosY = pViewData->GetPosY( eVWhich );
    if (nX2 < nPosX || nY2 < nPosY)
        return;                                         // unsichtbar
    if (nX1 < nPosX) nX1 = nPosX;
    if (nY1 < nPosY) nY1 = nPosY;

    USHORT nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
    if (nXRight > MAXCOL) nXRight = MAXCOL;
    USHORT nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
    if (nYBottom > MAXROW) nYBottom = MAXROW;

    if (nX1 > nXRight || nY1 > nYBottom)
        return;                                         // unsichtbar
    if (nX2 > nXRight) nX2 = nXRight;
    if (nY2 > nYBottom) nY2 = nYBottom;

    if ( eMode != SC_UPDATE_MARKS )
        if (nX2 < nXRight)
            nX2 = nXRight;                              // zum Weiterzeichnen

        //  ab hier kein return mehr

    ++nPaintCount;                  // merken, dass gemalt wird (wichtig beim Invertieren)

    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = pViewData->GetTabNo();

    pDoc->ExtendHidden( nX1, nY1, nX2, nY2, nTab );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    USHORT nScrX = (USHORT) aScrPos.X();
    USHORT nScrY = (USHORT) aScrPos.Y();

    USHORT nCurX = pViewData->GetCurX();
    USHORT nCurY = pViewData->GetCurY();
    USHORT nCurEndX = nCurX;
    USHORT nCurEndY = nCurY;
    pDoc->ExtendMerge( nCurX, nCurY, nCurEndX, nCurEndY, nTab );
    BOOL bCurVis = nCursorHideCount==0 &&
                    ( nCurEndX+1 >= nX1 && nCurX <= nX2+1 && nCurEndY+1 >= nY1 && nCurY <= nY2+1 );

    //  AutoFill-Anfasser
    if ( !bCurVis && nCursorHideCount==0 && bAutoMarkVisible && aAutoMarkPos.Tab() == nTab &&
            ( aAutoMarkPos.Col() != nCurX || aAutoMarkPos.Row() != nCurY ) )
    {
        USHORT nHdlX = aAutoMarkPos.Col();
        USHORT nHdlY = aAutoMarkPos.Row();
        pDoc->ExtendMerge( nHdlX, nHdlY, nHdlX, nHdlY, nTab );
        bCurVis = ( nHdlX+1 >= nX1 && nHdlX <= nX2 && nHdlY+1 >= nY1 && nHdlY <= nY2 );
        //  links und oben ist nicht betroffen

        //! AutoFill-Anfasser alleine (ohne Cursor) zeichnen ???
    }

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    const ScViewOptions& rOpts = pViewData->GetOptions();
    BOOL bFormulaMode = rOpts.GetOption( VOPT_FORMULAS );
    BOOL bMarkClipped = rOpts.GetOption( VOPT_CLIPMARKS );

        // Datenblock

    RowInfo* pRowInfo = new RowInfo[ROWINFO_MAX];
    USHORT nArrCount = pDoc->FillInfo( pRowInfo, nX1, nY1, nX2, nY2, nTab,
                                        nPPTX, nPPTY, FALSE, bFormulaMode,
                                        &pViewData->GetMarkData() );

    //--------------------------------------------------------------------

    Fraction aZoomX = pViewData->GetZoomX();
    Fraction aZoomY = pViewData->GetZoomY();
    ScOutputData aOutputData( this, OUTTYPE_WINDOW, pRowInfo, nArrCount, pDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nPPTX, nPPTY,
                                &aZoomX, &aZoomY );

    aOutputData.SetSyntaxMode       ( pViewData->IsSyntaxMode() );
    aOutputData.SetGridColor        ( rOpts.GetGridColor() );
    aOutputData.SetShowNullValues   ( rOpts.GetOption( VOPT_NULLVALS ) );
    aOutputData.SetShowFormulas     ( bFormulaMode );
    aOutputData.SetShowSpellErrors  ( !rOpts.IsHideAutoSpell() &&
                                        pDoc->GetDocOptions().IsAutoSpell() );
    aOutputData.SetMarkClipped      ( bMarkClipped );

    aOutputData.SetEditObject( GetEditObject() );
    aOutputData.SetViewShell( pViewData->GetViewShell() );

    BOOL bGrid = rOpts.GetOption( VOPT_GRID );
    BOOL bPage = rOpts.GetOption( VOPT_PAGEBREAKS );

    if ( eMode == SC_UPDATE_CHANGED )
    {
        aOutputData.FindChanged();
        aOutputData.SetSingleGrid(TRUE);
    }

    BOOL bPageMode = pViewData->IsPagebreakMode();
    if (bPageMode)                                      // nach FindChanged
    {
        // SetPagebreakMode initialisiert auch bPrinted Flags
        aOutputData.SetPagebreakMode( pViewData->GetView()->GetPageBreakData() );
    }

    EditView*   pEditView = NULL;
    BOOL        bEditMode = pViewData->HasEditView(eWhich);
    if ( bEditMode && pViewData->GetRefTabNo() == nTab )
    {
        USHORT nEditCol;
        USHORT nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        USHORT nEditEndCol = pViewData->GetEditEndCol();
        USHORT nEditEndRow = pViewData->GetEditEndRow();

        if ( nEditEndCol >= nX1 && nEditCol <= nX2 && nEditEndRow >= nY1 && nEditRow <= nY2 )
            aOutputData.SetEditCell( nEditCol, nEditRow );
        else
            bEditMode = FALSE;

        //  nur Edit-Area zu zeichnen?
        //! dann muss trotzdem noch der Rand / das Gitter gemalt werden!

//      if ( nEditCol <= nX1 && nEditEndCol >= nX2 && nEditRow <= nY1 && nEditEndRow >= nY2 )
//          bOnlyEdit = TRUE;
    }

    MapMode aDrawMode = GetDrawMapMode();
    Rectangle aDrawingRect = PixelToLogic( Rectangle( Point( nScrX, nScrY ),
                                        Size( aOutputData.GetScrW(), aOutputData.GetScrH() ) ),
                                        aDrawMode );

    if (bCurVis)
        HideCursor();

    if ( pDoc->HasBackgroundDraw( nTab, aDrawingRect ) )
    {
        SetMapMode(MAP_PIXEL);
        aOutputData.DrawClear();

            // Drawing Hintergrund

        SetMapMode(aDrawMode);
        DrawRedraw( aOutputData, aDrawingRect, eMode, SC_LAYER_BACK );
    }
    else
        aOutputData.SetSolidBackground(TRUE);

    SetMapMode(MAP_PIXEL);
    aOutputData.DrawBackground();
    if ( bGridFirst && ( bGrid || bPage ) )
        aOutputData.DrawGrid( bGrid, bPage );
    if ( bPageMode )
        DrawPagePreview(nX1,nY1,nX2,nY2);
    aOutputData.DrawShadow();
    aOutputData.DrawFrame();
    aOutputData.DrawStrings();

        // Autofilter- und Pivot-Buttons

    DrawButtons( nX1, nY1, nX2, nY2, pRowInfo, nArrCount );         // Pixel

        // Notiz-Anzeiger

    if ( rOpts.GetOption( VOPT_NOTES ) )
        aOutputData.DrawNoteMarks();

        // Edit-Zellen

    SetMapMode(pViewData->GetLogicMode(eWhich));
    aOutputData.DrawEdit(TRUE);

    SetMapMode(MAP_PIXEL);
    if ( !bGridFirst && ( bGrid || bPage ) )
    {
//      SetMapMode(MAP_PIXEL);
        aOutputData.DrawGrid( bGrid, bPage );
    }
    aOutputData.DrawClipMarks();

    //  Szenario / ChangeTracking muss auf jeden Fall nach DrawGrid sein, auch bei !bGridFirst

    //! Test, ob ChangeTrack-Anzeige aktiv ist
    //! Szenario-Rahmen per View-Optionen abschaltbar?

    USHORT nTabCount = pDoc->GetTableCount();
    ScHighlightRanges* pHigh = pViewData->GetView()->GetHighlightRanges();
    BOOL bHasScenario = ( nTab+1<nTabCount && pDoc->IsScenario(nTab+1) && !pDoc->IsScenario(nTab) );
    BOOL bHasChange = ( pDoc->GetChangeTrack() != NULL );

    if ( bHasChange || bHasScenario || pHigh != NULL )
    {
//      SetMapMode(MAP_PIXEL);

        //! SetChangedClip() mit DrawMarks() zusammenfassen?? (anderer MapMode!)

        BOOL bAny = TRUE;
        if (eMode == SC_UPDATE_CHANGED)
            bAny = aOutputData.SetChangedClip();
        if (bAny)
        {
            if ( bHasChange )
                aOutputData.DrawChangeTrack();

            if ( bHasScenario )
                lcl_DrawScenarioFrames( this, pViewData, eWhich, nX1,nY1,nX2,nY2 );

            if ( pHigh )
                lcl_DrawHighlight( aOutputData, pViewData, *pHigh );

            if (eMode == SC_UPDATE_CHANGED)
                SetClipRegion();
        }
    }

        // Drawing Vordergrund

    SetMapMode(aDrawMode);
    DrawRedraw( aOutputData, aDrawingRect, eMode, SC_LAYER_FRONT );
    DrawRedraw( aOutputData, aDrawingRect, eMode, SC_LAYER_INTERN );
    DrawRedraw( aOutputData, aDrawingRect, eMode, SC_LAYER_CONTROLS );  // als letztes
    DrawSdrGrid( aDrawingRect );

    if (!bIsInScroll)                               // Drawing Markierungen
    {
        BOOL bDraw = TRUE;
        if (eMode == SC_UPDATE_CHANGED)
            bDraw = NeedDrawMarks() && aOutputData.SetChangedClip();
        if (bDraw)
        {
            DrawMarks();
            if (eMode == SC_UPDATE_CHANGED)
                SetClipRegion();
        }
    }

    SetMapMode(MAP_PIXEL);
                            //  InPlace Edit-View
                            //! gleich beim Zeichnen leer lassen !!!
    if ( bEditMode && (pViewData->GetRefTabNo() == pViewData->GetTabNo()) )
    {
        USHORT nCol1 = pViewData->GetEditViewCol();
        USHORT nRow1 = pViewData->GetEditViewRow();
        USHORT nCol2 = pViewData->GetEditEndCol();
        USHORT nRow2 = pViewData->GetEditEndRow();
        SetLineColor();
        SetFillColor( pEditView->GetBackgroundColor() );
        Point aStart = pViewData->GetScrPos( nCol1, nRow1, eWhich );
        Point aEnd = pViewData->GetScrPos( nCol2+1, nRow2+1, eWhich );
        aEnd.X() -= 2;      // Gitter nicht ueberschreiben
        aEnd.Y() -= 2;
        DrawRect( Rectangle( aStart,aEnd ) );

        SetMapMode(pViewData->GetLogicMode());
        pEditView->Paint( PixelToLogic( Rectangle( Point( nScrX, nScrY ),
                            Size( aOutputData.GetScrW(), aOutputData.GetScrH() ) ) ) );
        SetMapMode(MAP_PIXEL);
    }

    if (pViewData->IsActive())
        aOutputData.DrawMark( this );

    if ( pViewData->IsRefMode() && nTab >= pViewData->GetRefStartZ() && nTab <= pViewData->GetRefEndZ() )
    {
        //  Schraffur beim Loeschen per AutoFill
        if ( pViewData->GetRefType() == SC_REFTYPE_FILL )
        {
            ScRange aRange;
            if ( pViewData->GetDelMark( aRange ) )
            {
                if ( aRange.aStart.Col() < nX1 ) aRange.aStart.SetCol(nX1);
                if ( aRange.aEnd.Col() > nX2 )   aRange.aEnd.SetCol(nX2);
                if ( aRange.aStart.Row() < nY1 ) aRange.aStart.SetRow(nY1);
                if ( aRange.aEnd.Row() > nY2 )   aRange.aEnd.SetRow(nY2);
                if ( aRange.aStart.Col() <= aRange.aEnd.Col() &&
                     aRange.aStart.Row() <= aRange.aEnd.Row() )
                {
                    Point aStart = pViewData->GetScrPos( aRange.aStart.Col(),
                                                         aRange.aStart.Row(), eWhich );
                    Point aEnd = pViewData->GetScrPos( aRange.aEnd.Col()+1,
                                                       aRange.aEnd.Row()+1, eWhich );
                    aEnd.X() -= 1;
                    aEnd.Y() -= 1;

                    //  Markierung aufheben - roter Rahmen bleibt stehen
                    Rectangle aRect( aStart,aEnd );
                    Invert( aRect, INVERT_HIGHLIGHT );

                    //! Delete-Bereich extra kennzeichnen?!?!?
                }
            }
        }

        aOutputData.DrawRefMark( pViewData->GetRefStartX(), pViewData->GetRefStartY(),
                                 pViewData->GetRefEndX(), pViewData->GetRefEndY(),
                                 Color(COL_LIGHTRED), FALSE );
    }

        //  Range-Finder

    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pViewData->GetViewShell() );
    if (pHdl)
    {
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && !pRangeFinder->IsHidden() &&
                pRangeFinder->GetDocName() == pDocSh->GetTitle() )
        {
            USHORT nCount = (USHORT)pRangeFinder->Count();
            for (USHORT i=0; i<nCount; i++)
            {
                ScRangeFindData* pData = pRangeFinder->GetObject(i);
                if (pData)
                {
                    ScRange aRef = pData->aRef;
                    aRef.Justify();
                    if ( aRef.aStart.Tab() >= nTab && aRef.aEnd.Tab() <= nTab )
                        aOutputData.DrawRefMark( aRef.aStart.Col(), aRef.aStart.Row(),
                                                aRef.aEnd.Col(), aRef.aEnd.Row(),
                                                Color( ScRangeFindList::GetColorName( i ) ),
                                                TRUE );
                }
            }
        }
    }

        //  Rand (Pixel)

    if ( nX2==MAXCOL || nY2==MAXROW )
    {
        Rectangle aPixRect = Rectangle( Point(), GetOutputSizePixel() );
        SetFillColor( COL_LIGHTGRAY );
        SetLineColor();
        if ( nX2==MAXCOL )
        {
            Rectangle aDrawRect( aPixRect );
            aDrawRect.Left() = nScrX + aOutputData.GetScrW();
            if (aDrawRect.Right() >= aDrawRect.Left())
                DrawRect( aDrawRect );
        }
        if ( nY2==MAXROW )
        {
            Rectangle aDrawRect( aPixRect );
            aDrawRect.Top() = nScrY + aOutputData.GetScrH();
            if ( nX2==MAXCOL )
                aDrawRect.Right() = nScrX + aOutputData.GetScrW() - 1;      // Ecke nicht doppelt
            if (aDrawRect.Bottom() >= aDrawRect.Top())
                DrawRect( aDrawRect );
        }
    }

    if (bCurVis)
        ShowCursor();

    for (i=0; i<nArrCount; i++)
        delete[] pRowInfo[i].pCellInfo;
    delete[] pRowInfo;

    if (pViewData->HasEditView(eWhich))
        SetMapMode(pViewData->GetLogicMode());
    else
        SetMapMode(aDrawMode);

    if ( pNoteMarker )
        pNoteMarker->Draw();        // ueber den Cursor, im Drawing-MapMode

    DrawStartTimer();               // fuer bunte Handles ohne System-Clipping

    //
    //  Wenn waehrend des Paint etwas invertiert wurde (Selektion geaendert aus Basic-Macro),
    //  ist das jetzt durcheinandergekommen und es muss neu gemalt werden
    //

    DBG_ASSERT(nPaintCount, "nPaintCount falsch");
    --nPaintCount;
    if (!nPaintCount)
        CheckNeedsRepaint();
}

void ScGridWindow::CheckNeedsRepaint()
{
    //  called at the end of painting, and from timer after background text width calculation

    if (bNeedsRepaint)
    {
        bNeedsRepaint = FALSE;
        if (aRepaintPixel.IsEmpty())
            Invalidate();
        else
            Invalidate(PixelToLogic(aRepaintPixel));
        aRepaintPixel = Rectangle();
    }
}

void ScGridWindow::DrawPagePreview( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if (pPageData)
    {
        ScDocument* pDoc = pViewData->GetDocument();
        USHORT nTab = pViewData->GetTabNo();
        Size aWinSize = GetOutputSizePixel();
        Color aManual( COL_LIGHTBLUE );
        Color aAutomatic( COL_BLUE );

        Font aFont;
        ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).GetFont(aFont);
        aFont.SetColor( Color( COL_LIGHTGRAY ) );
        //  Groesse wird nach Bedarf eingestellt
        String aPageText = ScGlobal::GetRscString( STR_PAGE );
        aPageText += ' ';

        USHORT nCount = pPageData->GetCount();
        for (USHORT nPos=0; nPos<nCount; nPos++)
        {
            ScPrintRangeData& rData = pPageData->GetData(nPos);
            ScRange aRange = rData.GetPrintRange();
            if ( aRange.aStart.Col() <= nX2+1  && aRange.aEnd.Col()+1 >= nX1 &&
                 aRange.aStart.Row() <= nY2+1 && aRange.aEnd.Row()+1 >= nY1 )
            {
                //  3 Pixel Rahmen um den Druckbereich
                //  (mittlerer Pixel auf den Gitterlinien)

                SetLineColor();
                if (rData.IsAutomatic())
                    SetFillColor( aAutomatic );
                else
                    SetFillColor( aManual );

                Point aStart = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, TRUE );
                Point aEnd = pViewData->GetScrPos(
                                    aRange.aEnd.Col() + 1, aRange.aEnd.Row() + 1, eWhich, TRUE );
                aStart.X() -= 2;
                aStart.Y() -= 2;

                //  Ueberlaeufe verhindern:
                if ( aStart.X() < -10 ) aStart.X() = -10;
                if ( aStart.Y() < -10 ) aStart.Y() = -10;
                if ( aEnd.X() > aWinSize.Width() + 10 )
                    aEnd.X() = aWinSize.Width() + 10;
                if ( aEnd.Y() > aWinSize.Height() + 10 )
                    aEnd.Y() = aWinSize.Height() + 10;

                DrawRect( Rectangle( aStart, Point(aEnd.X(),aStart.Y()+2) ) );
                DrawRect( Rectangle( aStart, Point(aStart.X()+2,aEnd.Y()) ) );
                DrawRect( Rectangle( Point(aStart.X(),aEnd.Y()-2), aEnd ) );
                DrawRect( Rectangle( Point(aEnd.X()-2,aStart.Y()), aEnd ) );

                //  Seitenumbrueche
                //! anders darstellen (gestrichelt ????)

                USHORT nColBreaks = rData.GetPagesX();
                const USHORT* pColEnd = rData.GetPageEndX();
                USHORT nColPos;
                for (nColPos=0; nColPos+1<nColBreaks; nColPos++)
                {
                    USHORT nBreak = pColEnd[nColPos]+1;
                    if ( nBreak >= nX1 && nBreak <= nX2+1 )
                    {
                        //! hidden suchen
                        if ( pDoc->GetColFlags( nBreak, nTab ) & CR_MANUALBREAK )
                            SetFillColor( aManual );
                        else
                            SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        nBreak, aRange.aStart.Row(), eWhich, TRUE );
                        DrawRect( Rectangle( aBreak.X()-1, aStart.Y(), aBreak.X(), aEnd.Y() ) );
                    }
                }

                USHORT nRowBreaks = rData.GetPagesY();
                const USHORT* pRowEnd = rData.GetPageEndY();
                USHORT nRowPos;
                for (nRowPos=0; nRowPos+1<nRowBreaks; nRowPos++)
                {
                    USHORT nBreak = pRowEnd[nRowPos]+1;
                    if ( nBreak >= nY1 && nBreak <= nY2+1 )
                    {
                        //! hidden suchen
                        if ( pDoc->GetRowFlags( nBreak, nTab ) & CR_MANUALBREAK )
                            SetFillColor( aManual );
                        else
                            SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        aRange.aStart.Col(), nBreak, eWhich, TRUE );
                        DrawRect( Rectangle( aStart.X(), aBreak.Y()-1, aEnd.X(), aBreak.Y() ) );
                    }
                }

                //  Seitenzahlen

                USHORT nPrStartY = aRange.aStart.Row();
                for (nRowPos=0; nRowPos<nRowBreaks; nRowPos++)
                {
                    USHORT nPrEndY = pRowEnd[nRowPos];
                    if ( nPrEndY >= nY1 && nPrStartY <= nY2 )
                    {
                        USHORT nPrStartX = aRange.aStart.Col();
                        for (nColPos=0; nColPos<nColBreaks; nColPos++)
                        {
                            USHORT nPrEndX = pColEnd[nColPos];
                            if ( nPrEndX >= nX1 && nPrStartX <= nX2 )
                            {
                                Point aPageStart = pViewData->GetScrPos(
                                                        nPrStartX, nPrStartY, eWhich, TRUE );
                                Point aPageEnd = pViewData->GetScrPos(
                                                        nPrEndX+1,nPrEndY+1, eWhich, TRUE );

                                long nPageNo = rData.GetFirstPage();
                                if ( rData.IsTopDown() )
                                    nPageNo += ((long)nColPos)*nRowBreaks+nRowPos;
                                else
                                    nPageNo += ((long)nRowPos)*nColBreaks+nColPos;
                                String aPageStr = aPageText;
                                aPageStr += String::CreateFromInt32(nPageNo);

                                //  passende Fontgroesse suchen
                                aFont.SetSize( Size( 0,100 ) );
                                SetFont( aFont );
                                Size aSize100( GetTextWidth( aPageStr ), GetTextHeight() );
                                //  40% der Breite oder 60% der Hoehe
                                long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) /
                                                    aSize100.Width();
                                long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) /
                                                    aSize100.Height();
                                aFont.SetSize( Size( 0,Min(nSizeX,nSizeY) ) );
                                SetFont( aFont );

                                //  zentriert ausgeben
                                Size aTextSize( GetTextWidth( aPageStr ), GetTextHeight() );
                                Point aPos( (aPageStart.X()+aPageEnd.X()-aTextSize.Width())/2,
                                            (aPageStart.Y()+aPageEnd.Y()-aTextSize.Height())/2 );
                                DrawText( aPos, aPageStr );
                            }
                            nPrStartX = nPrEndX + 1;
                        }
                    }
                    nPrStartY = nPrEndY + 1;
                }
            }
        }
    }
}

void ScGridWindow::DrawRefMark( USHORT nRefStartX, USHORT nRefStartY,
                                    USHORT nRefEndX, USHORT nRefEndY,
                                    const Color& rColor, BOOL bHandle )
{
    PutInOrder( nRefStartX, nRefEndX );
    PutInOrder( nRefStartY, nRefEndY );

    USHORT nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    if ( nRefStartX == nRefEndX && nRefStartY == nRefEndY )
        pDoc->ExtendMerge( nRefStartX, nRefStartY, nRefEndX, nRefEndY, nTab );

    MapMode aOld = GetMapMode();
    SetMapMode(MAP_PIXEL);

    USHORT nCurX = pViewData->GetCurX();
    USHORT nCurY = pViewData->GetCurY();
    BOOL bHide = ( nCurX+1 >= nRefStartX && nCurX <= nRefEndX+1 &&
                   nCurY+1 >= nRefStartY && nCurY <= nRefEndY+1 );

    Point aStartPos = pViewData->GetScrPos( nRefStartX, nRefStartY, eWhich, TRUE );
    Point aEndPos = pViewData->GetScrPos( nRefEndX+1, nRefEndY+1, eWhich, TRUE );
    aEndPos.X() -= 2;
    aEndPos.Y() -= 2;   // nicht ueber das Gitter

    //  begrenzen um Ueberlaeufe mit Paint-Fehlern zu vermeiden
    long nMinY = -10;
    long nMaxY = GetOutputSizePixel().Height() + 10;    // mit Abstand fuer Handle
    if (aStartPos.Y() < nMinY) aStartPos.Y() = nMinY;
    if (aStartPos.Y() > nMaxY) aStartPos.Y() = nMaxY;
    if (aEndPos.Y() > nMaxY) aEndPos.Y() = nMaxY;
    if (aEndPos.Y() < nMinY) aEndPos.Y() = nMinY;

    if (bHide)
        HideCursor();

    SetLineColor( rColor );
    SetFillColor();
    DrawRect( Rectangle( aStartPos, aEndPos ) );

    if ( bHandle )
    {
        SetLineColor();
        SetFillColor( rColor );
        DrawRect( Rectangle( aEndPos.X()-3, aEndPos.Y()-3,
                                aEndPos.X()+1, aEndPos.Y()+1 ) );
    }

    if (bHide)
        ShowCursor();

    SetMapMode(aOld);
}

void ScGridWindow::DrawButtons( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    RowInfo* pRowInfo, USHORT nArrCount )
{
    USHORT          nCol, nRow, nArrY, nQuery;
    USHORT          nTab = pViewData->GetTabNo();
    ScDocument*     pDoc = pViewData->GetDocument();
    ScDBData*       pDBData = NULL;
    ScQueryParam*   pQueryParam = NULL;

    Point aOldPos  = aComboButton.GetPosPixel();    // Zustand fuer MouseDown/Up
    Size  aOldSize = aComboButton.GetSizePixel();   // merken
    Color aOldCol  = aComboButton.GetColor();

    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        if ( pRowInfo[nArrY].bAutoFilter && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];

            nRow = pThisRowInfo->nRowNo;


            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                if ( pInfo->bAutoFilter && !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    if (!pQueryParam)
                        pQueryParam = new ScQueryParam;

                    BOOL bNewData = TRUE;
                    if (pDBData)
                    {
                        USHORT nStartCol;
                        USHORT nStartRow;
                        USHORT nEndCol;
                        USHORT nEndRow;
                        USHORT nAreaTab;
                        pDBData->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
                        if ( nCol >= nStartCol && nCol <= nEndCol &&
                             nRow >= nStartRow && nRow <= nEndRow )
                            bNewData = FALSE;
                    }
                    if (bNewData)
                    {
                        pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
                        if (pDBData)
                            pDBData->GetQueryParam( *pQueryParam );
                        else
                            DBG_ERROR("Auto-Filter-Button ohne DBData");
                    }

                    //  pQueryParam kann nur MAXQUERY Eintraege enthalten

                    BOOL bSimpleQuery = TRUE;
                    BOOL bColumnFound = FALSE;
                    if (!pQueryParam->bInplace)
                        bSimpleQuery = FALSE;
                    for (nQuery=0; nQuery<MAXQUERY && bSimpleQuery; nQuery++)
                        if (pQueryParam->GetEntry(nQuery).bDoQuery)
                        {
                            //  hier nicht auf EQUAL beschraenken
                            //  (auch bei ">1" soll der Spaltenkopf blau werden)

                            if (pQueryParam->GetEntry(nQuery).nField == nCol)
                                bColumnFound = TRUE;
                            if (nQuery > 0)
                                if (pQueryParam->GetEntry(nQuery).eConnect != SC_AND)
                                    bSimpleQuery = FALSE;
                        }

                    Color   aColor( ( bSimpleQuery && bColumnFound )
                                        ? COL_LIGHTBLUE
                                        : COL_BLACK );
                    long    nSizeX;
                    long    nSizeY;

                    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                    aComboButton.SetOptSizePixel();
                    aComboButton.SetColor( aColor );
                    DrawComboButton( pViewData->GetScrPos( nCol, nRow, eWhich ),
                                     nSizeX, nSizeY );

                    aComboButton.SetPosPixel( aOldPos );    // alten Zustand
                    aComboButton.SetSizePixel( aOldSize );  // fuer MouseUp/Down
                    aComboButton.SetColor( aOldCol );       // wiederherstellen
                }
            }
        }

        if ( pRowInfo[nArrY].bPushButton && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            nRow = pThisRowInfo->nRowNo;
            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                if ( pInfo->bPushButton && !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    Point aScrPos = pViewData->GetScrPos( nCol, nRow, eWhich );
                    long nSizeX;
                    long nSizeY;
                    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                    long nPosX = aScrPos.X();
                    long nPosY = aScrPos.Y();

                    SetLineColor( COL_WHITE );
                    DrawLine( Point(nPosX,nPosY), Point(nPosX,nPosY+nSizeY-1) );
                    DrawLine( Point(nPosX,nPosY), Point(nPosX+nSizeX-1,nPosY) );
//                  SetLineColor( SC_SMALL3DSHADOW );
                    SetLineColor( COL_GRAY );
                    DrawLine( Point(nPosX,nPosY+nSizeY-1), Point(nPosX+nSizeX-1,nPosY+nSizeY-1) );
                    DrawLine( Point(nPosX+nSizeX-1,nPosY), Point(nPosX+nSizeX-1,nPosY+nSizeY-1) );
                    SetLineColor( COL_BLACK );
//                  SetFillColor();
//                  DrawRect( Rectangle( nPosX-1,nPosY-1, nPosX+nSizeX-1,nPosY+nSizeY-1 ) );
                }
            }
        }
    }

    delete pQueryParam;
}

BOOL ScGridWindow::IsAutoFilterActive( USHORT nCol, USHORT nRow, USHORT nTab )
{
    ScDocument*     pDoc    = pViewData->GetDocument();
    ScDBData*       pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
    ScQueryParam    aQueryParam;

    if ( pDBData )
        pDBData->GetQueryParam( aQueryParam );
    else
        DBG_ERROR("Auto-Filter-Button ohne DBData");

    BOOL    bSimpleQuery = TRUE;
    BOOL    bColumnFound = FALSE;
    USHORT  nQuery;

    if ( !aQueryParam.bInplace )
        bSimpleQuery = FALSE;

    //  aQueryParam kann nur MAXQUERY Eintraege enthalten

    for ( nQuery=0; nQuery<MAXQUERY && bSimpleQuery; nQuery++ )
        if ( aQueryParam.GetEntry(nQuery).bDoQuery )
        {
            if (aQueryParam.GetEntry(nQuery).nField == nCol)
                bColumnFound = TRUE;

            if (nQuery > 0)
                if (aQueryParam.GetEntry(nQuery).eConnect != SC_AND)
                    bSimpleQuery = FALSE;
        }

    return ( bSimpleQuery && bColumnFound );
}

void ScGridWindow::DrawComboButton( const Point&    rCellPos,
                                    long            nCellSizeX,
                                    long            nCellSizeY,
                                    BOOL            bBtnIn )
{
    Point   aScrPos  = rCellPos;
    Size    aBtnSize = aComboButton.GetSizePixel();

    if ( nCellSizeX < aBtnSize.Width() || nCellSizeY < aBtnSize.Height() )
    {
        if ( nCellSizeX < aBtnSize.Width() )
            aBtnSize.Width() = nCellSizeX;

        if ( nCellSizeY < aBtnSize.Height() )
            aBtnSize.Height() = nCellSizeY;

        aComboButton.SetSizePixel( aBtnSize );
    }

    aScrPos.X() += nCellSizeX - aBtnSize.Width();
    aScrPos.Y() += nCellSizeY - aBtnSize.Height();

    aComboButton.SetPosPixel( aScrPos );

    HideCursor();
    aComboButton.Draw( bBtnIn );
    ShowCursor();
}

void ScGridWindow::InvertSimple( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL bTestMerge, BOOL bRepeat )
{
    //! if INVERT_HIGHLIGHT swaps foreground and background (like on Mac),
    //! use INVERT_HIGHLIGHT only for cells that have no background color set
    //! (here and in ScOutputData::DrawMark)

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    ScMarkData& rMark = pViewData->GetMarkData();
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();

    USHORT nTestX2 = nX2;
    USHORT nTestY2 = nY2;
    if (bTestMerge)
        pDoc->ExtendMerge( nX1,nY1, nTestX2,nTestY2, nTab );

    USHORT nPosX = pViewData->GetPosX( eHWhich );
    USHORT nPosY = pViewData->GetPosY( eVWhich );
    if (nTestX2 < nPosX || nTestY2 < nPosY)
        return;                                         // unsichtbar
    USHORT nRealX1 = nX1;
    if (nX1 < nPosX)
        nX1 = nPosX;
    if (nY1 < nPosY)
        nY1 = nPosY;

    USHORT nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
    if (nXRight > MAXCOL) nXRight = MAXCOL;
    USHORT nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
    if (nYBottom > MAXROW) nYBottom = MAXROW;

    if (nX1 > nXRight || nY1 > nYBottom)
        return;                                         // unsichtbar
    if (nX2 > nXRight) nX2 = nXRight;
    if (nY2 > nYBottom) nY2 = nYBottom;

    MapMode aOld = GetMapMode(); SetMapMode(MAP_PIXEL);     // erst nach den return's !!!

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    ScInvertMerger aInvert( this );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    USHORT nScrY = (USHORT) aScrPos.Y();
    BOOL bWasHidden = FALSE;
    for (USHORT nY=nY1; nY<=nY2; nY++)
    {
        BOOL bFirstRow = ( nY == nPosY );                       // first visible row?
        BOOL bDoHidden = FALSE;                                 // versteckte nachholen ?
        USHORT nHeightTwips = pDoc->GetRowHeight( nY,nTab );
        BOOL bDoRow = ( nHeightTwips != 0 );
        if (bDoRow)
        {
            if (bTestMerge)
                if (bWasHidden)                 // auf versteckte zusammengefasste testen
                {
//                  --nY;                       // nY geaendert -> vorherige zeichnen
                    bDoHidden = TRUE;
                    bDoRow = TRUE;
                }

            bWasHidden = FALSE;
        }
        else
        {
            bWasHidden = TRUE;
            if (bTestMerge)
                if (nY==nY2)
                    bDoRow = TRUE;              // letzte Zeile aus Block
        }

        if ( bDoRow )
        {
            USHORT nLoopEndX = nX2;
            if (nX2 < nX1)                      // Rest von zusammengefasst
            {
                USHORT nStartX = nX1;
                while ( ((const ScMergeFlagAttr*)pDoc->
                            GetAttr(nStartX,nY,nTab,ATTR_MERGE_FLAG))->IsHorOverlapped() )
                    --nStartX;
                if (nStartX <= nX2)
                    nLoopEndX = nX1;
            }

            USHORT nEndY = nScrY + (USHORT) ( nHeightTwips * nPPTY ) - 1;
            USHORT nScrX = (USHORT) aScrPos.X();
            for (USHORT nX=nX1; nX<=nLoopEndX; nX++)
            {
                USHORT nWidth = (USHORT) ( pDoc->GetColWidth( nX,nTab ) * nPPTX );
                if ( nWidth > 0 )
                {
                    USHORT nEndX = nScrX + nWidth - 1;
                    if (bTestMerge)
                    {
                        USHORT nThisY = nY;
                        const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
                        const ScMergeFlagAttr* pMergeFlag = (const ScMergeFlagAttr*) &pPattern->
                                                                        GetItem(ATTR_MERGE_FLAG);
                        if ( pMergeFlag->IsVerOverlapped() && ( bDoHidden || bFirstRow ) )
                        {
                            while ( pMergeFlag->IsVerOverlapped() && nThisY > 0 &&
                                        ( (pDoc->GetRowFlags( nThisY-1, nTab ) & CR_HIDDEN) || bFirstRow ) )
                            {
                                --nThisY;
                                pPattern = pDoc->GetPattern( nX, nThisY, nTab );
                                pMergeFlag = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                            }
                        }

                        // nur Rest von zusammengefasster zu sehen ?
                        USHORT nThisX = nX;
                        if ( pMergeFlag->IsHorOverlapped() && nX == nPosX && nX > nRealX1 )
                        {
                            while ( pMergeFlag->IsHorOverlapped() )
                            {
                                --nThisX;
                                pPattern = pDoc->GetPattern( nThisX, nThisY, nTab );
                                pMergeFlag = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                            }
                        }

                        if ( rMark.IsCellMarked( nThisX, nThisY, TRUE ) == bRepeat )
                        {
                            if ( !pMergeFlag->IsOverlapped() )
                            {
                                ScMergeAttr* pMerge = (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                if (pMerge->GetColMerge() || pMerge->GetRowMerge())
                                {
                                    Point aEndPos = pViewData->GetScrPos(
                                            nThisX + pMerge->GetColMerge(),
                                            nThisY + pMerge->GetRowMerge(), eWhich );
                                    if ( aEndPos.X() > nScrX && aEndPos.Y() > nScrY )
                                    {
                                        aInvert.AddRect( Rectangle( nScrX,nScrY,aEndPos.X()-1,aEndPos.Y()-1 ) );
                                    }
                                }
                                else if ( nEndX >= nScrX && nEndY >= nScrY )
                                {
                                    aInvert.AddRect( Rectangle( nScrX,nScrY,nEndX,nEndY ) );
                                }
                            }
                        }
                    }
                    else        // !bTestMerge
                    {
                        if ( rMark.IsCellMarked( nX, nY, TRUE ) == bRepeat &&
                                                nEndX >= nScrX && nEndY >= nScrY )
                        {
                            aInvert.AddRect( Rectangle( nScrX,nScrY,nEndX,nEndY ) );
                        }
                    }

                    nScrX = nEndX + 1;
                }
            }
            nScrY = nEndY + 1;
        }
    }

    aInvert.Flush();        // before restoring MapMode

    SetMapMode(aOld);

    CheckInverted();
}

// -------------------------------------------------------------------------

void ScGridWindow::DrawDragRect( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL bMarkDrop )
{
    if ( nX2 < pViewData->GetPosX(eHWhich) || nY2 < pViewData->GetPosY(eVWhich) )
        return;

    //  Nur auf dem Mac wird bei Drag&Drop nur die Einfuegeposition markiert
#ifndef MAC
    bMarkDrop = FALSE;
#endif

    if (bMarkDrop)
        nY2 = nY1;      // nur 1 Zeile

    Update();           // wegen XOR

    MapMode aOld = GetMapMode(); SetMapMode(MAP_PIXEL);

    USHORT nTab = pViewData->GetTabNo();

    USHORT nPosX = pViewData->GetPosX(WhichH(eWhich));
    USHORT nPosY = pViewData->GetPosY(WhichV(eWhich));
    if (nX1 < nPosX) nX1 = nPosX;
    if (nX2 < nPosX) nX2 = nPosX;
    if (nY1 < nPosY) nY1 = nPosY;
    if (nY2 < nPosY) nY2 = nPosY;

    Point aScrPos( pViewData->GetScrPos( nX1, nY1, eWhich ) );

    long nSizeXPix=0;
    long nSizeYPix=0;
    ScDocument* pDoc = pViewData->GetDocument();
    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();
    USHORT i;

    if (bMarkDrop)
    {
        aScrPos.X() -= 1;           // nur die Position markieren
        nSizeXPix   += 2;
    }
    else
    {
        if (nX2<=MAXCOL && nX2>=nX1)
            for (i=nX1; i<=nX2; i++)
                nSizeXPix += (long) ( pDoc->GetColWidth( i, nTab ) * nPPTX );
        else
        {
            aScrPos.X() -= 1;
            nSizeXPix   += 2;
        }
    }

    if (nY2<=MAXROW && nY2>=nY1)
        for (i=nY1; i<=nY2; i++)
            nSizeYPix += (long) ( pDoc->GetRowHeight( i, nTab ) * nPPTY );
    else
    {
        aScrPos.Y() -= 1;
        nSizeYPix   += 2;
    }

    aScrPos.X() -= 2;
    aScrPos.Y() -= 2;
    Rectangle aRect( aScrPos, Size( nSizeXPix + 3, nSizeYPix + 3 ) );

    Invert(Rectangle( aRect.Left(), aRect.Top(), aRect.Left()+2, aRect.Bottom() ));
    Invert(Rectangle( aRect.Right()-2, aRect.Top(), aRect.Right(), aRect.Bottom() ));
    Invert(Rectangle( aRect.Left()+3, aRect.Top(), aRect.Right()-3, aRect.Top()+2 ));
    Invert(Rectangle( aRect.Left()+3, aRect.Bottom()-2, aRect.Right()-3, aRect.Bottom() ));

    SetMapMode(aOld);
}

// -------------------------------------------------------------------------

void ScGridWindow::DrawCursor()
{
    USHORT nTab = pViewData->GetTabNo();
    USHORT nX = pViewData->GetCurX();
    USHORT nY = pViewData->GetCurY();

    //  in verdeckten Zellen nicht zeichnen

    ScDocument* pDoc = pViewData->GetDocument();
    const ScPatternAttr* pPattern = pDoc->GetPattern(nX,nY,nTab);
    const ScMergeFlagAttr& rMerge = (const ScMergeFlagAttr&) pPattern->GetItem(ATTR_MERGE_FLAG);
    if (rMerge.IsOverlapped())
        return;

    //  links/oben ausserhalb des Bildschirms ?

    BOOL bVis = ( nX>=pViewData->GetPosX(eHWhich) && nY>=pViewData->GetPosY(eVWhich) );
    if (!bVis)
    {
        USHORT nEndX = nX;
        USHORT nEndY = nY;
        ScDocument* pDoc = pViewData->GetDocument();
        const ScMergeAttr& rMerge = (const ScMergeAttr&) pPattern->GetItem(ATTR_MERGE);
        if (rMerge.GetColMerge() > 1)
            nEndX += rMerge.GetColMerge()-1;
        if (rMerge.GetRowMerge() > 1)
            nEndX += rMerge.GetRowMerge()-1;
        bVis = ( nEndX>=pViewData->GetPosX(eHWhich) && nEndY>=pViewData->GetPosY(eVWhich) );
    }

    if ( bVis )
    {
        //  hier kein Update, da aus Paint gerufen und laut Zaehler Cursor schon da
        //  wenn Update noetig, dann bei Hide/Showcursor vor dem Hoch-/Runterzaehlen

        MapMode aOld = GetMapMode(); SetMapMode(MAP_PIXEL);

        Point aScrPos = pViewData->GetScrPos( nX, nY, eWhich, TRUE );

        //  rechts/unten ausserhalb des Bildschirms ?

        Size aOutSize = GetOutputSizePixel();
        if ( aScrPos.X() <= aOutSize.Width() + 2 && aScrPos.Y() <= aOutSize.Height() + 2 )
        {
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );

            BOOL bFix = ( pViewData->GetHSplitMode() == SC_SPLIT_FIX ||
                            pViewData->GetVSplitMode() == SC_SPLIT_FIX );
            if ( pViewData->GetActivePart()==eWhich || bFix )
            {
                //  old UNX version with two Invert calls causes flicker.
                //  if optimization is needed, a new flag should be added
                //  to InvertTracking

                aScrPos.X() -= 2;
                aScrPos.Y() -= 2;
                Rectangle aRect( aScrPos, Size( nSizeXPix + 3, nSizeYPix + 3 ) );

                Invert(Rectangle( aRect.Left(), aRect.Top(), aRect.Left()+2, aRect.Bottom() ));
                Invert(Rectangle( aRect.Right()-2, aRect.Top(), aRect.Right(), aRect.Bottom() ));
                Invert(Rectangle( aRect.Left()+3, aRect.Top(), aRect.Right()-3, aRect.Top()+2 ));
                Invert(Rectangle( aRect.Left()+3, aRect.Bottom()-2, aRect.Right()-3, aRect.Bottom() ));
            }
            else
            {
                Rectangle aRect( aScrPos, Size( nSizeXPix - 1, nSizeYPix - 1 ) );
                Invert( aRect );
            }
        }

        SetMapMode(aOld);
    }
}

    //  AutoFill-Anfasser:

void ScGridWindow::DrawAutoFillMark()
{
    if ( bAutoMarkVisible && aAutoMarkPos.Tab() == pViewData->GetTabNo() )
    {
        USHORT nX = aAutoMarkPos.Col();
        USHORT nY = aAutoMarkPos.Row();

        Point aFillPos = pViewData->GetScrPos( nX, nY, eWhich, TRUE );
        long nSizeXPix;
        long nSizeYPix;
        pViewData->GetMergeSizePixel( nX, nY, nSizeXPix, nSizeYPix );
        aFillPos.X() += nSizeXPix;
        aFillPos.Y() += nSizeYPix;

        aFillPos.X() -= 2;
        aFillPos.Y() -= 2;
        Rectangle aFillRect( aFillPos, Size(6,6) );
        //  Anfasser von Zeichenobjekten sind 7*7

        MapMode aOld = GetMapMode(); SetMapMode(MAP_PIXEL);
        Invert( aFillRect );
        SetMapMode(aOld);
    }
}

// -------------------------------------------------------------------------

void ScGridWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( (rDCEvt.GetType() == DATACHANGED_PRINTER) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( rDCEvt.GetType() == DATACHANGED_FONTS && eWhich == pViewData->GetActivePart() )
            pViewData->GetDocShell()->UpdateFontList();

        if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
             (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        {
            //  RepeatResize falls sich die Scrollbar-Groesse geaendert hat
            if ( eWhich == pViewData->GetActivePart() )     // einmal reicht
                pViewData->GetView()->RepeatResize();
        }

        Invalidate();
    }
}




