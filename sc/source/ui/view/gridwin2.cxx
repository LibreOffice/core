/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gridwin2.cxx,v $
 * $Revision: 1.16.32.1 $
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
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>

#include "gridwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "pivot.hxx"
//CHINA001 #include "pfiltdlg.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "pagedata.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpoutput.hxx"     // ScDPPositionData
#include "dpshttab.hxx"
#include "dbdocfun.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include "scabstdlg.hxx" //CHINA001
using namespace com::sun::star;


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

BOOL ScGridWindow::HasPageFieldData( SCCOL nCol, SCROW nRow ) const
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(nCol, nRow, nTab);
    if ( pDPObj && nCol > 0 )
    {
        // look for the dimension header left of the drop-down arrow
        USHORT nOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        long nField = pDPObj->GetHeaderDim( ScAddress( nCol-1, nRow, nTab ), nOrient );
        if ( nField >= 0 && nOrient == sheet::DataPilotFieldOrientation_PAGE )
        {
            BOOL bIsDataLayout = FALSE;
            String aFieldName = pDPObj->GetDimName( nField, bIsDataLayout );
            if ( aFieldName.Len() && !bIsDataLayout )
                return TRUE;
        }
    }
    return FALSE;
}

// private method for mouse button handling
BOOL ScGridWindow::DoPageFieldSelection( SCCOL nCol, SCROW nRow )
{
    if ( HasPageFieldData( nCol, nRow ) )
    {
        DoPageFieldMenue( nCol, nRow );
        return TRUE;
    }
    return FALSE;
}

void ScGridWindow::DoPushButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor(nCol, nRow, nTab);

    if (pDPObj)
    {
        USHORT nOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        ScAddress aPos( nCol, nRow, nTab );
        long nField = pDPObj->GetHeaderDim( aPos, nOrient );
        if ( nField >= 0 )
        {
            bDPMouse   = TRUE;
            nDPField   = nField;
            pDragDPObj = pDPObj;
            DPTestMouse( rMEvt, TRUE );
            StartTracking();
        }
        else if ( pDPObj->IsFilterButton(aPos) )
        {
            ReleaseMouse();         // may have been captured in ButtonDown

            ScQueryParam aQueryParam;
            SCTAB nSrcTab = 0;
            const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
            DBG_ASSERT(pDesc, "no sheet source for filter button");
            if (pDesc)
            {
                aQueryParam = pDesc->aQueryParam;
                nSrcTab = pDesc->aSourceRange.aStart.Tab();
            }

            SfxItemSet aArgSet( pViewData->GetViewShell()->GetPool(),
                                        SCITEM_QUERYDATA, SCITEM_QUERYDATA );
            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

//CHINA001          ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
//CHINA001          pViewData->GetViewShell()->GetDialogParent(),
//CHINA001          aArgSet, nSrcTab );
            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

            AbstractScPivotFilterDlg* pDlg = pFact->CreateScPivotFilterDlg( pViewData->GetViewShell()->GetDialogParent(),
                                                                            aArgSet, nSrcTab,
                                                                            RID_SCDLG_PIVOTFILTER);
            DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
            if ( pDlg->Execute() == RET_OK )
            {
                ScSheetSourceDesc aNewDesc;
                if (pDesc)
                    aNewDesc = *pDesc;

                const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
                aNewDesc.aQueryParam = rQueryItem.GetQueryData();

                ScDPObject aNewObj( *pDPObj );
                aNewObj.SetSheetDesc( aNewDesc );
                ScDBDocFunc aFunc( *pViewData->GetDocShell() );
                aFunc.DataPilotUpdate( pDPObj, &aNewObj, TRUE, FALSE );
                pViewData->GetView()->CursorPosChanged();       // shells may be switched
            }
            delete pDlg;
        }
        else
            Sound::Beep();
    }
    else
    {
        DBG_ERROR("Da is ja garnix");
    }
}

// -----------------------------------------------------------------------
//
//  Data Pilot interaction
//

void ScGridWindow::DPTestMouse( const MouseEvent& rMEvt, BOOL bMove )
{
    DBG_ASSERT(pDragDPObj, "pDragDPObj missing");

    //  scroll window if at edges
    //! move this to separate method

    BOOL bTimer = FALSE;
    Point aPixel = rMEvt.GetPosPixel();

    SCsCOL nDx = 0;
    SCsROW nDy = 0;
    if ( aPixel.X() < 0 )
        nDx = -1;
    if ( aPixel.Y() < 0 )
        nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPixel.X() >= aSize.Width() )
        nDx = 1;
    if ( aPixel.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        UpdateDragRect( FALSE, Rectangle() );

        if ( nDx  != 0)
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = TRUE;
    }

    //  ---

    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), eWhich, nPosX, nPosY );
    BOOL    bMouseLeft;
    BOOL    bMouseTop;
    pViewData->GetMouseQuadrant( aPixel, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

    ScAddress aPos( nPosX, nPosY, pViewData->GetTabNo() );

    Rectangle aPosRect;
    USHORT nOrient;
    long nDimPos;
    BOOL bHasRange = pDragDPObj->GetHeaderDrag( aPos, bMouseLeft, bMouseTop, nDPField,
                                                aPosRect, nOrient, nDimPos );
    UpdateDragRect( bHasRange && bMove, aPosRect );

    if (bMove)          // set mouse pointer
    {
        PointerStyle ePointer = POINTER_PIVOT_DELETE;
        if ( bHasRange )
            switch (nOrient)
            {
                case sheet::DataPilotFieldOrientation_COLUMN: ePointer = POINTER_PIVOT_COL; break;
                case sheet::DataPilotFieldOrientation_ROW:    ePointer = POINTER_PIVOT_ROW; break;
                case sheet::DataPilotFieldOrientation_PAGE:
                case sheet::DataPilotFieldOrientation_DATA:   ePointer = POINTER_PIVOT_FIELD;   break;
            }
        SetPointer( ePointer );
    }
    else                // execute change
    {
        if (!bHasRange)
            nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

        BOOL bIsDataLayout;
        String aDimName = pDragDPObj->GetDimName( nDPField, bIsDataLayout );
        if ( bIsDataLayout && ( nOrient != sheet::DataPilotFieldOrientation_COLUMN &&
                                nOrient != sheet::DataPilotFieldOrientation_ROW ) )
        {
            //  removing data layout is not allowed
            pViewData->GetView()->ErrorMessage(STR_PIVOT_MOVENOTALLOWED);
        }
        else
        {
            ScDPSaveData aSaveData( *pDragDPObj->GetSaveData() );

            ScDPSaveDimension* pDim;
            if ( bIsDataLayout )
                pDim = aSaveData.GetDataLayoutDimension();
            else
                pDim = aSaveData.GetDimensionByName(aDimName);
            pDim->SetOrientation( nOrient );
            aSaveData.SetPosition( pDim, nDimPos );

            //! docfunc method with ScDPSaveData as argument?

            ScDPObject aNewObj( *pDragDPObj );
            aNewObj.SetSaveData( aSaveData );
            ScDBDocFunc aFunc( *pViewData->GetDocShell() );
            // when dragging fields, allow re-positioning (bAllowMove)
            aFunc.DataPilotUpdate( pDragDPObj, &aNewObj, TRUE, FALSE, TRUE );
            pViewData->GetView()->CursorPosChanged();       // shells may be switched
        }
    }

    if (bTimer && bMove)
        pViewData->GetView()->SetTimer( this, rMEvt );          // repeat event
    else
        pViewData->GetView()->ResetTimer();
}

void ScGridWindow::DPMouseMove( const MouseEvent& rMEvt )
{
    DPTestMouse( rMEvt, TRUE );
}

void ScGridWindow::DPMouseButtonUp( const MouseEvent& rMEvt )
{
    bDPMouse = FALSE;
    ReleaseMouse();

    DPTestMouse( rMEvt, FALSE );
    SetPointer( Pointer( POINTER_ARROW ) );
}

// -----------------------------------------------------------------------

void ScGridWindow::UpdateDragRect( BOOL bShowRange, const Rectangle& rPosRect )
{
    SCCOL nStartX = ( rPosRect.Left()   >= 0 ) ? static_cast<SCCOL>(rPosRect.Left())   : SCCOL_MAX;
    SCROW nStartY = ( rPosRect.Top()    >= 0 ) ? static_cast<SCROW>(rPosRect.Top())    : SCROW_MAX;
    SCCOL nEndX   = ( rPosRect.Right()  >= 0 ) ? static_cast<SCCOL>(rPosRect.Right())  : SCCOL_MAX;
    SCROW nEndY   = ( rPosRect.Bottom() >= 0 ) ? static_cast<SCROW>(rPosRect.Bottom()) : SCROW_MAX;

    if ( bShowRange == bDragRect && nDragStartX == nStartX && nDragEndX == nEndX &&
                                    nDragStartY == nStartY && nDragEndY == nEndY )
    {
        return;         // everything unchanged
    }

    // if ( bDragRect )
    //  DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
    if ( bShowRange )
    {
        nDragStartX = nStartX;
        nDragStartY = nStartY;
        nDragEndX = nEndX;
        nDragEndY = nEndY;
        bDragRect = TRUE;
        // DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
    }
    else
        bDragRect = FALSE;

    UpdateDragRectOverlay();
}

// -----------------------------------------------------------------------

//  Page-Break-Modus

USHORT ScGridWindow::HitPageBreak( const Point& rMouse, ScRange* pSource,
                                    SCCOLROW* pBreak, SCCOLROW* pPrev )
{
    USHORT nFound = SC_PD_NONE;     // 0
    ScRange aSource;
    SCCOLROW nBreak = 0;
    SCCOLROW nPrev = 0;

    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if ( pPageData )
    {
        BOOL bHori = FALSE;
        BOOL bVert = FALSE;
        SCCOL nHitX = 0;
        SCROW nHitY = 0;

        long nMouseX = rMouse.X();
        long nMouseY = rMouse.Y();
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( nMouseX, nMouseY, eWhich, nPosX, nPosY );
        Point aTL = pViewData->GetScrPos( nPosX, nPosY, eWhich );
        Point aBR = pViewData->GetScrPos( nPosX+1, nPosY+1, eWhich );

        //  Horizontal mehr Toleranz als vertikal, weil mehr Platz ist
        if ( nMouseX <= aTL.X() + 4 )
        {
            bHori = TRUE;
            nHitX = nPosX;
        }
        else if ( nMouseX >= aBR.X() - 6 )
        {
            bHori = TRUE;
            nHitX = nPosX+1;                    // linker Rand der naechsten Zelle
        }
        if ( nMouseY <= aTL.Y() + 2 )
        {
            bVert = TRUE;
            nHitY = nPosY;
        }
        else if ( nMouseY >= aBR.Y() - 4 )
        {
            bVert = TRUE;
            nHitY = nPosY+1;                    // oberer Rand der naechsten Zelle
        }

        if ( bHori || bVert )
        {
            USHORT nCount = sal::static_int_cast<USHORT>( pPageData->GetCount() );
            for (USHORT nPos=0; nPos<nCount && !nFound; nPos++)
            {
                ScPrintRangeData& rData = pPageData->GetData(nPos);
                ScRange aRange = rData.GetPrintRange();
                BOOL bLHit = ( bHori && nHitX == aRange.aStart.Col() );
                BOOL bRHit = ( bHori && nHitX == aRange.aEnd.Col() + 1 );
                BOOL bTHit = ( bVert && nHitY == aRange.aStart.Row() );
                BOOL bBHit = ( bVert && nHitY == aRange.aEnd.Row() + 1 );
                BOOL bInsideH = ( nPosX >= aRange.aStart.Col() && nPosX <= aRange.aEnd.Col() );
                BOOL bInsideV = ( nPosY >= aRange.aStart.Row() && nPosY <= aRange.aEnd.Row() );

                if ( bLHit )
                {
                    if ( bTHit )
                        nFound = SC_PD_RANGE_TL;
                    else if ( bBHit )
                        nFound = SC_PD_RANGE_BL;
                    else if ( bInsideV )
                        nFound = SC_PD_RANGE_L;
                }
                else if ( bRHit )
                {
                    if ( bTHit )
                        nFound = SC_PD_RANGE_TR;
                    else if ( bBHit )
                        nFound = SC_PD_RANGE_BR;
                    else if ( bInsideV )
                        nFound = SC_PD_RANGE_R;
                }
                else if ( bTHit && bInsideH )
                    nFound = SC_PD_RANGE_T;
                else if ( bBHit && bInsideH )
                    nFound = SC_PD_RANGE_B;
                if (nFound)
                    aSource = aRange;

                //  Umbrueche

                if ( bVert && bInsideH && !nFound )
                {
                    size_t nRowCount = rData.GetPagesY();
                    const SCROW* pRowEnd = rData.GetPageEndY();
                    for (size_t nRowPos=0; nRowPos+1<nRowCount; nRowPos++)
                        if ( pRowEnd[nRowPos]+1 == nHitY )
                        {
                            nFound = SC_PD_BREAK_V;
                            aSource = aRange;
                            nBreak = nHitY;
                            if ( nRowPos )
                                nPrev = pRowEnd[nRowPos-1]+1;
                            else
                                nPrev = aRange.aStart.Row();
                        }
                }
                if ( bHori && bInsideV && !nFound )
                {
                    size_t nColCount = rData.GetPagesX();
                    const SCCOL* pColEnd = rData.GetPageEndX();
                    for (size_t nColPos=0; nColPos+1<nColCount; nColPos++)
                        if ( pColEnd[nColPos]+1 == nHitX )
                        {
                            nFound = SC_PD_BREAK_H;
                            aSource = aRange;
                            nBreak = nHitX;
                            if ( nColPos )
                                nPrev = pColEnd[nColPos-1]+1;
                            else
                                nPrev = aRange.aStart.Col();
                        }
                }
            }
        }
    }

    if (pSource)
        *pSource = aSource;     // Druckbereich
    if (pBreak)
        *pBreak = nBreak;       // X/Y Position des verchobenen Seitenumbruchs
    if (pPrev)
        *pPrev = nPrev;         // X/Y Anfang der Seite, die am Umbruch zuende ist
    return nFound;
}

void ScGridWindow::PagebreakMove( const MouseEvent& rMEvt, BOOL bUp )
{
    //! Scrolling und Umschalten mit RFMouseMove zusammenfassen !
    //! (Weginvertieren vor dem Scrolling ist anders)

    //  Scrolling

    BOOL bTimer = FALSE;
    Point aPos = rMEvt.GetPosPixel();
    SCsCOL nDx = 0;
    SCsROW nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        if ( bPagebreakDrawn )          // weginvertieren
        {
            // DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
            //              aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = FALSE;
            UpdateDragRectOverlay();
        }

        if ( nDx != 0 ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
        bTimer = TRUE;
    }

    //  Umschalten bei Fixierung (damit Scrolling funktioniert)

    if ( eWhich == pViewData->GetActivePart() )     //??
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
            if ( nDx > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }

        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
            if ( nDy > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }
    }

    //  ab hier neu

    //  gesucht wird eine Position zwischen den Zellen (vor nPosX / nPosY)
    SCsCOL nPosX;
    SCsROW nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    BOOL bLeft, bTop;
    pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bLeft, bTop );
    if ( !bLeft ) ++nPosX;
    if ( !bTop )  ++nPosY;

    BOOL bBreak = ( nPagebreakMouse == SC_PD_BREAK_H || nPagebreakMouse == SC_PD_BREAK_V );
    BOOL bHide = FALSE;
    BOOL bToEnd = FALSE;
    ScRange aDrawRange = aPagebreakSource;
    if ( bBreak )
    {
        if ( nPagebreakMouse == SC_PD_BREAK_H )
        {
            if ( nPosX > aPagebreakSource.aStart.Col() &&
                 nPosX <= aPagebreakSource.aEnd.Col() + 1 )     // ans Ende ist auch erlaubt
            {
                bToEnd = ( nPosX == aPagebreakSource.aEnd.Col() + 1 );
                aDrawRange.aStart.SetCol( nPosX );
                aDrawRange.aEnd.SetCol( nPosX - 1 );
            }
            else
                bHide = TRUE;
        }
        else
        {
            if ( nPosY > aPagebreakSource.aStart.Row() &&
                 nPosY <= aPagebreakSource.aEnd.Row() + 1 )     // ans Ende ist auch erlaubt
            {
                bToEnd = ( nPosY == aPagebreakSource.aEnd.Row() + 1 );
                aDrawRange.aStart.SetRow( nPosY );
                aDrawRange.aEnd.SetRow( nPosY - 1 );
            }
            else
                bHide = TRUE;
        }
    }
    else
    {
        if ( nPagebreakMouse & SC_PD_RANGE_L )
            aDrawRange.aStart.SetCol( nPosX );
        if ( nPagebreakMouse & SC_PD_RANGE_T )
            aDrawRange.aStart.SetRow( nPosY );
        if ( nPagebreakMouse & SC_PD_RANGE_R )
        {
            if ( nPosX > 0 )
                aDrawRange.aEnd.SetCol( nPosX-1 );
            else
                bHide = TRUE;
        }
        if ( nPagebreakMouse & SC_PD_RANGE_B )
        {
            if ( nPosY > 0 )
                aDrawRange.aEnd.SetRow( nPosY-1 );
            else
                bHide = TRUE;
        }
        if ( aDrawRange.aStart.Col() > aDrawRange.aEnd.Col() ||
             aDrawRange.aStart.Row() > aDrawRange.aEnd.Row() )
            bHide = TRUE;
    }

    if ( !bPagebreakDrawn || bUp || aDrawRange != aPagebreakDrag )
    {
        //  zeichnen...

        if ( bPagebreakDrawn )
        {
            // weginvertieren
            // DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
            //              aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = FALSE;
        }
        aPagebreakDrag = aDrawRange;
        if ( !bUp && !bHide )
        {
            // hininvertieren
            // DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
            //              aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = TRUE;
        }
        UpdateDragRectOverlay();
    }

    //  bei ButtonUp die Aenderung ausfuehren

    if ( bUp )
    {
        ScViewFunc* pViewFunc = pViewData->GetView();
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        BOOL bUndo (pDoc->IsUndoEnabled());

        if ( bBreak )
        {
            BOOL bColumn = ( nPagebreakMouse == SC_PD_BREAK_H );
            SCCOLROW nNew = bColumn ? static_cast<SCCOLROW>(nPosX) : static_cast<SCCOLROW>(nPosY);
            if ( nNew != nPagebreakBreak )
            {
                if (bUndo)
                {
                    String aUndo = ScGlobal::GetRscString( STR_UNDO_DRAG_BREAK );
                    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                }

                BOOL bGrow = !bHide && nNew > nPagebreakBreak;
                if ( bColumn )
                {
                    if ( pDoc->GetColFlags( static_cast<SCCOL>(nPagebreakBreak), nTab ) & CR_MANUALBREAK )
                    {
                        ScAddress aOldAddr( static_cast<SCCOL>(nPagebreakBreak), nPosY, nTab );
                        pViewFunc->DeletePageBreak( TRUE, TRUE, &aOldAddr, FALSE );
                    }
                    if ( !bHide && !bToEnd )    // am Ende nicht
                    {
                        ScAddress aNewAddr( static_cast<SCCOL>(nNew), nPosY, nTab );
                        pViewFunc->InsertPageBreak( TRUE, TRUE, &aNewAddr, FALSE );
                    }
                    if ( bGrow )
                    {
                        //  vorigen Break auf hart, und Skalierung aendern
                        if ( static_cast<SCCOL>(nPagebreakPrev) > aPagebreakSource.aStart.Col() &&
                                !(pDoc->GetColFlags( static_cast<SCCOL>(nPagebreakPrev), nTab ) & CR_MANUALBREAK) )
                        {
                            ScAddress aPrev( static_cast<SCCOL>(nPagebreakPrev), nPosY, nTab );
                            pViewFunc->InsertPageBreak( TRUE, TRUE, &aPrev, FALSE );
                        }

                        if (!pDocSh->AdjustPrintZoom( ScRange(
                                      static_cast<SCCOL>(nPagebreakPrev),0,nTab, static_cast<SCCOL>(nNew-1),0,nTab ) ))
                            bGrow = FALSE;
                    }
                }
                else
                {
                    if ( pDoc->GetRowFlags( nPagebreakBreak, nTab ) & CR_MANUALBREAK )
                    {
                        ScAddress aOldAddr( nPosX, nPagebreakBreak, nTab );
                        pViewFunc->DeletePageBreak( FALSE, TRUE, &aOldAddr, FALSE );
                    }
                    if ( !bHide && !bToEnd )    // am Ende nicht
                    {
                        ScAddress aNewAddr( nPosX, nNew, nTab );
                        pViewFunc->InsertPageBreak( FALSE, TRUE, &aNewAddr, FALSE );
                    }
                    if ( bGrow )
                    {
                        //  vorigen Break auf hart, und Skalierung aendern
                        if ( nPagebreakPrev > aPagebreakSource.aStart.Row() &&
                                !(pDoc->GetRowFlags( nPagebreakPrev, nTab ) & CR_MANUALBREAK) )
                        {
                            ScAddress aPrev( nPosX, nPagebreakPrev, nTab );
                            pViewFunc->InsertPageBreak( FALSE, TRUE, &aPrev, FALSE );
                        }

                        if (!pDocSh->AdjustPrintZoom( ScRange(
                                      0,nPagebreakPrev,nTab, 0,nNew-1,nTab ) ))
                            bGrow = FALSE;
                    }
                }

                if (bUndo)
                {
                    pDocSh->GetUndoManager()->LeaveListAction();
                }

                if (!bGrow)     // sonst in AdjustPrintZoom schon passiert
                {
                    pViewFunc->UpdatePageBreakData( TRUE );
                    pDocSh->SetDocumentModified();
                }
            }
        }
        else if ( bHide || aPagebreakDrag != aPagebreakSource )
        {
            //  Druckbereich setzen

            String aNewRanges;
            USHORT nOldCount = pDoc->GetPrintRangeCount( nTab );
            if ( nOldCount )
            {
                for (USHORT nPos=0; nPos<nOldCount; nPos++)
                {
                    const ScRange* pOld = pDoc->GetPrintRange( nTab, nPos );
                    if ( pOld )
                    {
                        String aTemp;
                        if ( *pOld != aPagebreakSource )
                            pOld->Format( aTemp, SCA_VALID );
                        else if ( !bHide )
                            aPagebreakDrag.Format( aTemp, SCA_VALID );
                        if (aTemp.Len())
                        {
                            if ( aNewRanges.Len() )
                                aNewRanges += ';';
                            aNewRanges += aTemp;
                        }
                    }
                }
            }
            else if (!bHide)
                aPagebreakDrag.Format( aNewRanges, SCA_VALID );

            pViewFunc->SetPrintRanges( pDoc->IsPrintEntireSheet( nTab ), &aNewRanges, NULL, NULL, FALSE );
        }
    }

    //  Timer fuer Scrolling

    if (bTimer && !bUp)
        pViewData->GetView()->SetTimer( this, rMEvt );          // Event wiederholen
    else
        pViewData->GetView()->ResetTimer();
}




