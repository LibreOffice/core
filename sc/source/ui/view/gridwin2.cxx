/*************************************************************************
 *
 *  $RCSfile: gridwin2.cxx,v $
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
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>

#include "gridwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "pivot.hxx"
#include "pfiltdlg.hxx"
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

using namespace com::sun::star;


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

void ScGridWindow::DoPushButton( USHORT nCol, USHORT nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    USHORT nTab = pViewData->GetTabNo();

    ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
    ScPivot* pPivot = pPivotCollection->GetPivotAtCursor(nCol, nRow, nTab);

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor(nCol, nRow, nTab);

    if (pPivot)             // alte Pivottabellen
    {
        if (pPivot->IsFilterAtCursor(nCol, nRow, nTab))
        {
            ReleaseMouse();     // falls schon beim ButtonDown gecaptured, #44018#

            ScQueryParam aQueryParam;
            pPivot->GetQuery(aQueryParam);
            USHORT nSrcTab = pPivot->GetSrcArea().aStart.Tab();

            SfxItemSet aArgSet( pViewData->GetViewShell()->GetPool(),
                                        SCITEM_QUERYDATA, SCITEM_QUERYDATA );
            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

            ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
                                                pViewData->GetViewShell()->GetDialogParent(),
                                                aArgSet, nSrcTab );
            if ( pDlg->Execute() == RET_OK )
            {
                ScPivot* pNewPivot = pPivot->CreateNew();

                const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
                pNewPivot->SetQuery(rQueryItem.GetQueryData());

                PivotField* pColArr = new PivotField[PIVOT_MAXFIELD];
                short nColCount;
                pPivot->GetColFields( pColArr, nColCount );
                PivotField* pRowArr = new PivotField[PIVOT_MAXFIELD];
                short nRowCount;
                pPivot->GetRowFields( pRowArr, nRowCount );
                PivotField* pDataArr = new PivotField[PIVOT_MAXFIELD];
                short nDataCount;
                pPivot->GetDataFields( pDataArr, nDataCount );

                pNewPivot->SetColFields( pColArr, nColCount );
                pNewPivot->SetRowFields( pRowArr, nRowCount );
                pNewPivot->SetDataFields( pDataArr, nDataCount );

                pNewPivot->SetName( pPivot->GetName() );
                pNewPivot->SetTag( pPivot->GetTag() );

                pViewData->GetDocShell()->PivotUpdate( pPivot, pNewPivot );
            }
            delete pDlg;
        }
        else
        {
            USHORT nField;
            if (pPivot->GetColFieldAtCursor(nCol, nRow, nTab, nField))
            {
                bPivotMouse     = TRUE;
                nPivotField     = nField;
                bPivotColField  = TRUE;
                nPivotCol       = nCol;
                pDragPivot      = pPivot;
                PivotTestMouse( rMEvt, TRUE );
                // CaptureMouse();
                StartTracking();
            }
            else if (pPivot->GetRowFieldAtCursor(nCol, nRow, nTab, nField))
            {
                bPivotMouse     = TRUE;
                nPivotField     = nField;
                bPivotColField  = FALSE;
                nPivotCol       = nCol;
                pDragPivot      = pPivot;
                PivotTestMouse( rMEvt, TRUE );
                // CaptureMouse();
                StartTracking();
            }
        }
    }
    else if (pDPObj)
    {
        ScAddress aPos( nCol, nRow, nTab );
        long nField = pDPObj->GetHeaderDim(aPos);
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
            USHORT nSrcTab = 0;
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

            ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
                                                pViewData->GetViewShell()->GetDialogParent(),
                                                aArgSet, nSrcTab );
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
        DBG_ERROR("Da is ja garnix");
}

void ScGridWindow::DoPivotDrop( BOOL bDelete, BOOL bToCols, short nDestPos )
{
    if ( nPivotField == PIVOT_DATA_FIELD && bDelete )
    {
        pViewData->GetView()->ErrorMessage(STR_PIVOT_MOVENOTALLOWED);
        return;
    }

    if ( bPivotColField != bToCols && !bDelete )
    {
        short nDestCount = bToCols ? pDragPivot->GetColFieldCount()
                                   : pDragPivot->GetRowFieldCount();
        if ( nDestCount >= PIVOT_MAXFIELD )     // schon voll?
        {
            //  Versuch, mehr als PIVOT_MAXFIELD Eintraege zu erzeugen
            pViewData->GetView()->ErrorMessage(STR_PIVOT_ERROR);
            return;
        }
    }

    PivotField* pColArr = new PivotField[PIVOT_MAXFIELD];
    short nColCount;
    pDragPivot->GetColFields( pColArr, nColCount );

    PivotField* pRowArr = new PivotField[PIVOT_MAXFIELD];
    short nRowCount;
    pDragPivot->GetRowFields( pRowArr, nRowCount );

    PivotField* pDataArr = new PivotField[PIVOT_MAXFIELD];
    short nDataCount;
    pDragPivot->GetDataFields( pDataArr, nDataCount );

    short nOldPos = 0;
    short i;
    PivotField aMoveField;

    PivotField* pSource = bPivotColField ? pColArr : pRowArr;
    short& rCount = bPivotColField ? nColCount : nRowCount;

    BOOL bFound = FALSE;
    for (i=0; i<rCount && !bFound; i++)
        if (pSource[i].nCol == (short) nPivotField)
        {
            nOldPos = i;
            aMoveField = pSource[i];
            --rCount;
            if (i<rCount)
                memmove( &pSource[i], &pSource[i+1], (rCount-i)*sizeof(PivotField) );
            if ( bPivotColField == bToCols )
                if (nDestPos > i)
                    --nDestPos;
            bFound = TRUE;
        }

    if (bFound)
    {
        if (!bDelete)
        {
            PivotField* pDest = bToCols ? pColArr : pRowArr;
            short& rDestCount = bToCols ? nColCount : nRowCount;
            if (nDestPos < rDestCount)
                memmove( &pDest[nDestPos+1], &pDest[nDestPos],
                            (rDestCount-nDestPos)*sizeof(PivotField) );
            pDest[nDestPos] = aMoveField;
            ++rDestCount;
        }

        ScPivotCollection* pPivotCollection = pViewData->GetDocument()->GetPivotCollection();

        BOOL bEmpty = ( nColCount + nRowCount == 0 ||
                        ( nColCount + nRowCount == 1 && nDataCount <= 1 ) );

        if ( bEmpty )               // Pivottabelle loeschen
        {
            pViewData->GetDocShell()->PivotUpdate( pDragPivot, NULL );
        }
        else
        {
            ScPivot* pNewPivot = pDragPivot->CreateNew();
            pNewPivot->SetColFields( pColArr, nColCount );
            pNewPivot->SetRowFields( pRowArr, nRowCount );
            pNewPivot->SetDataFields( pDataArr, nDataCount );

            pNewPivot->SetName( pDragPivot->GetName() );
            pNewPivot->SetTag( pDragPivot->GetTag() );

            pViewData->GetDocShell()->PivotUpdate( pDragPivot, pNewPivot );
        }
        pDragPivot = NULL;
    }
    else
        DBG_ASSERT(0,"Pivot-Eintrag nicht gefunden");

    delete[] pColArr;
    delete[] pRowArr;
    delete[] pDataArr;
}

BOOL ScGridWindow::PivotTestMouse( const MouseEvent& rMEvt, BOOL bMove )
{
    BOOL bRet = FALSE;
    BOOL bTimer = FALSE;
    Point aPos = rMEvt.GetPosPixel();

    short nDx = 0;
    short nDy = 0;
    if ( aPos.X() < 0 )
        nDx = -1;
    if ( aPos.Y() < 0 )
        nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx || nDy )
    {
        if (bDragRect)
        {
            DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
            bDragRect = FALSE;
        }

        if ( nDx )
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = TRUE;
    }

    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    BOOL    bMouseLeft;
    BOOL    bMouseTop;
    pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

    USHORT nPiCol1;
    USHORT nPiRow1;
    USHORT nPiCol2;
    USHORT nPiRow2;
    USHORT nTab;
    pDragPivot->GetDestArea( nPiCol1, nPiRow1, nPiCol2, nPiRow2, nTab );

    if ( nPosX >= (short) nPiCol1 && nPosX <= (short) nPiCol2 &&
         nPosY >= (short) nPiRow1 && nPosY <= (short) nPiRow2 )
    {
        short nFilterAdd = 2;       // Platz fuer Filter-Button
        short nColRows   = 1;       //! Ueberschrift: 0, wenn keine Zeilen, aber mehrere Datenfelder
        USHORT nNewStartX;
        USHORT nNewStartY;
        USHORT nNewEndX;
        USHORT nNewEndY;

        short nRelX = nPosX - (short) nPiCol1;
        short nRelY = nPosY - (short) nPiRow1 - nFilterAdd;

        PivotField* pFieldArr = new PivotField[PIVOT_MAXFIELD];
        short nColCount;
        pDragPivot->GetColFields( pFieldArr, nColCount );
        short nRowCount;
        pDragPivot->GetRowFields( pFieldArr, nRowCount );
        delete[] pFieldArr;

        BOOL bBefore;
        short nColSize = Max( nColCount, (short) 1 );
        short nRowSize = Max( nRowCount, (short) 1 );

        BOOL bToCols;
        if (nRelX < nColSize && nRelY >= nRowSize)
            bToCols = TRUE;                                     // links
        else if (nRelY < nRowSize && nRelX >= nColSize)
            bToCols = FALSE;                                    // oben
        else
            bToCols = ( nRelY-nRowSize > nRelX-nColSize );

        short nDestCol = 0;
        short nDestRow = 0;
        BOOL bNothing = FALSE;

        if ( bToCols )
        {
            bBefore = bMouseLeft;
            nDestCol = nRelX;
            if (nDestCol < 0)
            {
                nDestCol = 0;
                bBefore = TRUE;
            }
            if (nDestCol >= nColCount)
            {
                nDestCol = nColCount-1;
                bBefore = FALSE;
            }

            nNewStartY = nPiRow1 + (USHORT) nFilterAdd + nRowCount + nColRows;
            nNewEndY   = nPiRow2 - 1;
            nNewStartX = nPiCol1 + (USHORT) nDestCol;
            nNewEndX   = nNewStartX;

            if ( !bPivotColField )                  // von der anderen Seite
            {
                if (bBefore)
                    nNewEndX = nNewStartX - 1;                      // vor dem Feld
                else
                    nNewStartX = nNewEndX + 1;                      // hinter dem Feld
            }
            else
            {
                USHORT nThisCol = (USHORT) nPosX;           // absolute Spalte ( == Maus )
                if ( nThisCol < nPivotCol )
                {
                    nNewEndX = nNewStartX - 1;                      // vor dem Feld
                    bBefore = TRUE;
                }
                else if ( nThisCol > nPivotCol )
                {
                    nNewStartX = nNewEndX + 1;                      // hinter dem Feld
                    bBefore = FALSE;
                }
                else
                    bNothing = TRUE;
            }
            SetPointer( Pointer( POINTER_PIVOT_ROW ) );
        }
        else
        {
            nDestRow;
            if (nRelY <= 0 && nRelX < nColCount+nRowCount)
            {
                nDestRow = nRelX-nColCount;
                bBefore = bMouseLeft;
            }
            else
            {
                nDestRow = nRelY-1;
                bBefore = bMouseTop;
            }
            if (nDestRow < 0)
            {
                nDestRow = 0;
                bBefore = TRUE;
            }
            if (nDestRow >= nRowCount)
            {
                nDestRow = nRowCount-1;
                bBefore = FALSE;
            }

            nNewStartX = nPiCol1 + (USHORT) nColCount;
            nNewEndX   = nPiCol2 - 1;
            nNewStartY = nPiRow1 + (USHORT) nFilterAdd + nDestRow + nColRows;
            nNewEndY   = nNewStartY;
            if ( bPivotColField )                   // von der anderen Seite
            {
                if (bBefore)
                    nNewEndY = nNewStartY - 1;                      // vor dem Feld
                else
                    nNewStartY = nNewEndY + 1;                      // hinter dem Feld
            }
            else
            {
                USHORT nThisCol = nDestRow+nColCount+nPiCol1;       // absolute Spalte
                if ( nThisCol < nPivotCol )
                {
                    bBefore = TRUE;
                    nNewEndY = nNewStartY - 1;                      // vor dem Feld
                }
                else if ( nThisCol > nPivotCol )
                {
                    bBefore = FALSE;
                    nNewStartY = nNewEndY + 1;                      // hinter dem Feld
                }
                else
                    bNothing = TRUE;
            }
            SetPointer( Pointer( POINTER_PIVOT_COL ) );
        }

        if (bMove)
        {
            if ( nNewStartX != nDragStartX || nNewEndX != nDragEndX ||
                 nNewStartY != nDragStartY || nNewEndY != nDragEndY || !bDragRect )
            {
                if (bDragRect)
                    DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );

                nDragStartX = nNewStartX;
                nDragStartY = nNewStartY;
                nDragEndX = nNewEndX;
                nDragEndY = nNewEndY;
                bDragRect = TRUE;

                DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
            }
        }
        else
        {
            if (bDragRect)
            {
                DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
                bDragRect = FALSE;
            }

            if (!bNothing)
            {
                short nDestPos = bToCols ? nDestCol : nDestRow;
                if (!bBefore)
                    ++nDestPos;
                DoPivotDrop( FALSE, bToCols, nDestPos );
            }
        }

        bRet = TRUE;
    }
    else
    {
        if (bMove)
            SetPointer( Pointer( POINTER_NOTALLOWED ) );
        if (bDragRect)
            DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
        bDragRect = FALSE;

        if (!bMove)
            DoPivotDrop( TRUE, FALSE,0 );
    }

    if (bTimer && bMove)
        pViewData->GetView()->SetTimer( this, rMEvt );          // Event wiederholen
    else
        pViewData->GetView()->ResetTimer();

    return bRet;
}

void ScGridWindow::PivotMouseMove( const MouseEvent& rMEvt )
{
    PivotTestMouse( rMEvt, TRUE );
}

void ScGridWindow::PivotMouseButtonUp( const MouseEvent& rMEvt )
{
    bPivotMouse = FALSE;        // als erstes, falls PivotTestMouse eine Fehlermeldung bringt
    ReleaseMouse();

    PivotTestMouse( rMEvt, FALSE );
    SetPointer( Pointer( POINTER_ARROW ) );
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

    short nDx = 0;
    short nDy = 0;
    if ( aPixel.X() < 0 )
        nDx = -1;
    if ( aPixel.Y() < 0 )
        nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPixel.X() >= aSize.Width() )
        nDx = 1;
    if ( aPixel.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx || nDy )
    {
        UpdateDragRect( FALSE, Rectangle() );

        if ( nDx )
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = TRUE;
    }

    //  ---

    short   nPosX;
    short   nPosY;
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
        PointerStyle ePointer = POINTER_NOTALLOWED;
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
        if ( bIsDataLayout && nOrient == sheet::DataPilotFieldOrientation_HIDDEN )
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
            aFunc.DataPilotUpdate( pDragDPObj, &aNewObj, TRUE, FALSE );
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
    USHORT nStartX = ( rPosRect.Left()   >= 0 ) ? rPosRect.Left()   : USHRT_MAX;
    USHORT nStartY = ( rPosRect.Top()    >= 0 ) ? rPosRect.Top()    : USHRT_MAX;
    USHORT nEndX   = ( rPosRect.Right()  >= 0 ) ? rPosRect.Right()  : USHRT_MAX;
    USHORT nEndY   = ( rPosRect.Bottom() >= 0 ) ? rPosRect.Bottom() : USHRT_MAX;

    if ( bShowRange == bDragRect && nDragStartX == nStartX && nDragEndX == nEndX &&
                                    nDragStartY == nStartY && nDragEndY == nEndY )
    {
        return;         // everything unchanged
    }

    if ( bDragRect )
        DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
    if ( bShowRange )
    {
        nDragStartX = nStartX;
        nDragStartY = nStartY;
        nDragEndX = nEndX;
        nDragEndY = nEndY;
        bDragRect = TRUE;
        DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
    }
    else
        bDragRect = FALSE;
}

// -----------------------------------------------------------------------

//  Page-Break-Modus

USHORT ScGridWindow::HitPageBreak( const Point& rMouse, ScRange* pSource,
                                    USHORT* pBreak, USHORT* pPrev )
{
    USHORT nFound = SC_PD_NONE;     // 0
    ScRange aSource;
    USHORT nBreak = 0;
    USHORT nPrev = 0;

    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if ( pPageData )
    {
        BOOL bHori = FALSE;
        BOOL bVert = FALSE;
        USHORT nHitX, nHitY;

        long nMouseX = rMouse.X();
        long nMouseY = rMouse.Y();
        short nPosX, nPosY;
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
            USHORT nCount = pPageData->GetCount();
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
                    USHORT nRowCount = rData.GetPagesY();
                    const USHORT* pRowEnd = rData.GetPageEndY();
                    for (USHORT nRowPos=0; nRowPos+1<nRowCount; nRowPos++)
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
                    USHORT nColCount = rData.GetPagesX();
                    const USHORT* pColEnd = rData.GetPageEndX();
                    for (USHORT nColPos=0; nColPos+1<nColCount; nColPos++)
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
    short nDx = 0;
    short nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx || nDy )
    {
        if ( bPagebreakDrawn )          // weginvertieren
        {
            DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
                            aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = FALSE;
        }

        if ( nDx ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
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
    short nPosX, nPosY;
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
            DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
                            aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = FALSE;
        }
        aPagebreakDrag = aDrawRange;
        if ( !bUp && !bHide )
        {
            // hininvertieren
            DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
                            aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), FALSE );
            bPagebreakDrawn = TRUE;
        }
    }

    //  bei ButtonUp die Aenderung ausfuehren

    if ( bUp )
    {
        ScViewFunc* pViewFunc = pViewData->GetView();
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = pViewData->GetTabNo();

        if ( bBreak )
        {
            BOOL bColumn = ( nPagebreakMouse == SC_PD_BREAK_H );
            USHORT nNew = bColumn ? nPosX : nPosY;
            if ( nNew != nPagebreakBreak )
            {
                String aUndo = ScGlobal::GetRscString( STR_UNDO_DRAG_BREAK );
                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

                BOOL bGrow = !bHide && nNew > nPagebreakBreak;
                if ( bColumn )
                {
                    if ( pDoc->GetColFlags( nPagebreakBreak, nTab ) & CR_MANUALBREAK )
                    {
                        ScAddress aOldAddr( nPagebreakBreak, nPosY, nTab );
                        pViewFunc->DeletePageBreak( TRUE, TRUE, &aOldAddr, FALSE );
                    }
                    if ( !bHide && !bToEnd )    // am Ende nicht
                    {
                        ScAddress aNewAddr( nNew, nPosY, nTab );
                        pViewFunc->InsertPageBreak( TRUE, TRUE, &aNewAddr, FALSE );
                    }
                    if ( bGrow )
                    {
                        //  vorigen Break auf hart, und Skalierung aendern
                        if ( nPagebreakPrev > aPagebreakSource.aStart.Col() &&
                                !(pDoc->GetColFlags( nPagebreakPrev, nTab ) & CR_MANUALBREAK) )
                        {
                            ScAddress aPrev( nPagebreakPrev, nPosY, nTab );
                            pViewFunc->InsertPageBreak( TRUE, TRUE, &aPrev, FALSE );
                        }

                        if (!pDocSh->AdjustPrintZoom( ScRange(
                                      nPagebreakPrev,0,nTab, nNew-1,0,nTab ) ))
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

                pDocSh->GetUndoManager()->LeaveListAction();

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

            pViewFunc->SetPrintRanges( &aNewRanges, NULL, NULL, FALSE );
        }
    }

    //  Timer fuer Scrolling

    if (bTimer && !bUp)
        pViewData->GetView()->SetTimer( this, rMEvt );          // Event wiederholen
    else
        pViewData->GetView()->ResetTimer();
}




