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
#include "dpcontrol.hxx"
#include "dpcontrol.hrc"
#include "strload.hxx"
#include "userlist.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include "scabstdlg.hxx" //CHINA001

#include <vector>
#include <hash_map>

using namespace com::sun::star;
using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::std::vector;
using ::std::auto_ptr;
using ::std::hash_map;
using ::rtl::OUString;
using ::rtl::OUStringHash;

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

DataPilotFieldOrientation ScGridWindow::GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const
{
    using namespace ::com::sun::star::sheet;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return DataPilotFieldOrientation_HIDDEN;

    USHORT nOrient = DataPilotFieldOrientation_HIDDEN;

    // Check for page field first.
    if (nCol > 0)
    {
        // look for the dimension header left of the drop-down arrow
        long nField = pDPObj->GetHeaderDim( ScAddress( nCol-1, nRow, nTab ), nOrient );
        if ( nField >= 0 && nOrient == DataPilotFieldOrientation_PAGE )
        {
            BOOL bIsDataLayout = FALSE;
            String aFieldName = pDPObj->GetDimName( nField, bIsDataLayout );
            if ( aFieldName.Len() && !bIsDataLayout )
                return DataPilotFieldOrientation_PAGE;
        }
    }

    nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    // Now, check for row/column field.
    long nField = pDPObj->GetHeaderDim(ScAddress(nCol, nRow, nTab), nOrient);
    if (nField >= 0 && (nOrient == DataPilotFieldOrientation_COLUMN || nOrient == DataPilotFieldOrientation_ROW) )
    {
        BOOL bIsDataLayout = FALSE;
        String aFieldName = pDPObj->GetDimName(nField, bIsDataLayout);
        if (aFieldName.Len() && !bIsDataLayout)
            return static_cast<DataPilotFieldOrientation>(nOrient);
    }

    return DataPilotFieldOrientation_HIDDEN;
}

// private method for mouse button handling
BOOL ScGridWindow::DoPageFieldSelection( SCCOL nCol, SCROW nRow )
{
    if (GetDPFieldOrientation( nCol, nRow ) == sheet::DataPilotFieldOrientation_PAGE)
    {
        LaunchPageFieldMenu( nCol, nRow );
        return TRUE;
    }
    return FALSE;
}

bool ScGridWindow::DoAutoFilterButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    Point aScrPos  = pViewData->GetScrPos(nCol, nRow, eWhich);
    Point aDiffPix = rMEvt.GetPosPixel();

    aDiffPix -= aScrPos;
    BOOL bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    if ( bLayoutRTL )
        aDiffPix.X() = -aDiffPix.X();

    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    mpFilterButton.reset(new ScDPFieldButton(this, &GetSettings().GetStyleSettings(), &pViewData->GetZoomX(), &pViewData->GetZoomY()));
    mpFilterButton->setBoundingBox(aScrPos, aScrSize);
    Point aPopupPos;
    Size aPopupSize;
    mpFilterButton->getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRec(aPopupPos, aPopupSize);
    if (aRec.IsInside(rMEvt.GetPosPixel()))
    {
        if ( DoPageFieldSelection( nCol, nRow ) )
            return true;

        bool bFilterActive = IsAutoFilterActive(nCol, nRow, nTab);
        mpFilterButton->setHasHiddenMember(bFilterActive);
        mpFilterButton->setDrawBaseButton(false);
        mpFilterButton->setDrawPopupButton(true);
        mpFilterButton->setPopupPressed(true);
        HideCursor();
        mpFilterButton->draw();
        ShowCursor();
        DoAutoFilterMenue(nCol, nRow, false);
        return true;
    }

    return false;
}

void ScGridWindow::DoPushButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor(nCol, nRow, nTab);

#if OLD_PIVOT_IMPLEMENTATION
    ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
    ScPivot* pPivot = pPivotCollection->GetPivotAtCursor(nCol, nRow, nTab);

    if (pPivot)             // alte Pivottabellen
    {
        if (pPivot->IsFilterAtCursor(nCol, nRow, nTab))
        {
            ReleaseMouse();     // falls schon beim ButtonDown gecaptured, #44018#

            ScQueryParam aQueryParam;
            pPivot->GetQuery(aQueryParam);
            SCTAB nSrcTab = pPivot->GetSrcArea().aStart.Tab();

            SfxItemSet aArgSet( pViewData->GetViewShell()->GetPool(),
                                        SCITEM_QUERYDATA, SCITEM_QUERYDATA );
            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

            //CHINA001 ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
            //CHINA001                                  pViewData->GetViewShell()->GetDialogParent(),
            //CHINA001                                  aArgSet, nSrcTab );
            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

            AbstractScPivotFilterDlg* pDlg = pFact->CreateScPivotFilterDlg( pViewData->GetViewShell()->GetDialogParent(),
                                                                            aArgSet, nSrcTab,
                                                                            RID_SCDLG_PIVOTFILTER);
            DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
            if ( pDlg->Execute() == RET_OK )
            {
                ScPivot* pNewPivot = pPivot->CreateNew();

                const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
                pNewPivot->SetQuery(rQueryItem.GetQueryData());

                PivotField* pColArr = new PivotField[PIVOT_MAXFIELD];
                SCSIZE nColCount;
                pPivot->GetColFields( pColArr, nColCount );
                PivotField* pRowArr = new PivotField[PIVOT_MAXFIELD];
                SCSIZE nRowCount;
                pPivot->GetRowFields( pRowArr, nRowCount );
                PivotField* pDataArr = new PivotField[PIVOT_MAXFIELD];
                SCSIZE nDataCount;
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
            SCCOL nField;
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
#endif

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

            if (DPTestFieldPopupArrow(rMEvt, aPos, pDPObj))
            {
                // field name pop up menu has been launched.  Don't activate
                // field move.
                bDPMouse = false;
                return;
            }

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

#if OLD_PIVOT_IMPLEMENTATION
void ScGridWindow::DoPivotDrop( BOOL bDelete, BOOL bToCols, SCSIZE nDestPos )
{
    if ( nPivotField == PIVOT_DATA_FIELD && bDelete )
    {
        pViewData->GetView()->ErrorMessage(STR_PIVOT_MOVENOTALLOWED);
        return;
    }

    if ( bPivotColField != bToCols && !bDelete )
    {
        SCSIZE nDestCount = bToCols ? pDragPivot->GetColFieldCount()
                                   : pDragPivot->GetRowFieldCount();
        if ( nDestCount >= PIVOT_MAXFIELD )     // schon voll?
        {
            //  Versuch, mehr als PIVOT_MAXFIELD Eintraege zu erzeugen
            pViewData->GetView()->ErrorMessage(STR_PIVOT_ERROR);
            return;
        }
    }

    PivotField* pColArr = new PivotField[PIVOT_MAXFIELD];
    SCSIZE nColCount;
    pDragPivot->GetColFields( pColArr, nColCount );

    PivotField* pRowArr = new PivotField[PIVOT_MAXFIELD];
    SCSIZE nRowCount;
    pDragPivot->GetRowFields( pRowArr, nRowCount );

    PivotField* pDataArr = new PivotField[PIVOT_MAXFIELD];
    SCSIZE nDataCount;
    pDragPivot->GetDataFields( pDataArr, nDataCount );

    SCSIZE nOldPos = 0;
    PivotField aMoveField;

    PivotField* pSource = bPivotColField ? pColArr : pRowArr;
    SCSIZE& rCount = bPivotColField ? nColCount : nRowCount;

    BOOL bFound = FALSE;
    for (SCSIZE i=0; i<rCount && !bFound; i++)
        if (pSource[i].nCol == nPivotField)
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
            SCSIZE& rDestCount = bToCols ? nColCount : nRowCount;

            if (nDestPos < rDestCount)
                memmove( &pDest[nDestPos+1], &pDest[nDestPos],
                            (rDestCount-nDestPos)*sizeof(PivotField) );
            pDest[nDestPos] = aMoveField;
            ++rDestCount;
        }

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
    {
        DBG_ASSERT(0,"Pivot-Eintrag nicht gefunden");
    }

    delete[] pColArr;
    delete[] pRowArr;
    delete[] pDataArr;
}

BOOL ScGridWindow::PivotTestMouse( const MouseEvent& rMEvt, BOOL bMove )
{
    BOOL bRet = FALSE;
    BOOL bTimer = FALSE;
    Point aPos = rMEvt.GetPosPixel();

    SCsCOL nDx = 0;
    SCsROW nDy = 0;
    if ( aPos.X() < 0 )
        nDx = -1;
    if ( aPos.Y() < 0 )
        nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        if (bDragRect)
        {
            // DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
            bDragRect = FALSE;
            UpdateDragRectOverlay();
        }

        if ( nDx != 0 )
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = TRUE;
    }

    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    BOOL    bMouseLeft;
    BOOL    bMouseTop;
    pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

    SCCOL nPiCol1;
    SCROW nPiRow1;
    SCCOL nPiCol2;
    SCROW nPiRow2;
    SCTAB nTab;
    pDragPivot->GetDestArea( nPiCol1, nPiRow1, nPiCol2, nPiRow2, nTab );

    if ( nPosX >= (SCsCOL) nPiCol1 && nPosX <= (SCsCOL) nPiCol2 &&
         nPosY >= (SCsROW) nPiRow1 && nPosY <= (SCsROW) nPiRow2 )
    {
        SCsROW nFilterAdd = 2;      // Platz fuer Filter-Button
        SCsROW nColRows   = 1;      //! Ueberschrift: 0, wenn keine Zeilen, aber mehrere Datenfelder
        SCCOL nNewStartX;
        SCROW nNewStartY;
        SCCOL nNewEndX;
        SCROW nNewEndY;

        SCsCOL nRelX = nPosX - (SCsCOL) nPiCol1;
        SCsROW nRelY = nPosY - (SCsROW) nPiRow1 - nFilterAdd;

        PivotField* pFieldArr = new PivotField[PIVOT_MAXFIELD];
        SCSIZE nColCount;
        pDragPivot->GetColFields( pFieldArr, nColCount );
        SCSIZE nRowCount;
        pDragPivot->GetRowFields( pFieldArr, nRowCount );
        delete[] pFieldArr;

        BOOL bBefore;
        SCsCOL nColSize = static_cast<SCsCOL>(Max( nColCount, (SCSIZE) 1 ));
        SCsROW nRowSize = static_cast<SCsROW>(Max( nRowCount, (SCSIZE) 1 ));

        BOOL bToCols;
        if (nRelX < nColSize && nRelY >= nRowSize)
            bToCols = TRUE;                                     // links
        else if (nRelY < nRowSize && nRelX >= nColSize)
            bToCols = FALSE;                                    // oben
        else
            bToCols = ( nRelY-nRowSize > static_cast<SCsCOLROW>(nRelX-nColSize) );

        SCsCOL nDestCol = 0;
        SCsROW nDestRow = 0;
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
            if (nDestCol >= static_cast<SCsCOL>(nColCount))
            {
                nDestCol = static_cast<SCsCOL>(nColCount)-1;
                bBefore = FALSE;
            }

            nNewStartY = nPiRow1 + nFilterAdd + static_cast<SCROW>(nRowCount) + nColRows;
            nNewEndY   = nPiRow2 - 1;
            nNewStartX = nPiCol1 + (SCCOL) nDestCol;
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
                SCCOL nThisCol = (SCCOL) nPosX;         // absolute Spalte ( == Maus )
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
            if (nRelY <= 0 && static_cast<SCsCOLROW>(nRelX) < static_cast<SCsCOLROW>(nColCount)+static_cast<SCsCOLROW>(nRowCount))
            {
                nDestRow = static_cast<SCsCOLROW>(nRelX) - static_cast<SCsCOLROW>(nColCount);
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
            if (nDestRow >= static_cast<SCsROW>(nRowCount))
            {
                nDestRow = static_cast<SCsROW>(nRowCount)-1;
                bBefore = FALSE;
            }

            nNewStartX = nPiCol1 + (SCCOL) nColCount;
            nNewEndX   = nPiCol2 - 1;
            nNewStartY = nPiRow1 + nFilterAdd + nDestRow + nColRows;
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
                SCCOL nThisCol =
                    static_cast<SCCOL>(static_cast<SCCOLROW>(nDestRow) +
                            static_cast<SCCOLROW>(nColCount) + nPiCol1);
                // absolute Spalte
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
                //if (bDragRect)
                //  DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );

                nDragStartX = nNewStartX;
                nDragStartY = nNewStartY;
                nDragEndX = nNewEndX;
                nDragEndY = nNewEndY;
                bDragRect = TRUE;

                // DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );

                UpdateDragRectOverlay();
            }
        }
        else
        {
            if (bDragRect)
            {
                // DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
                bDragRect = FALSE;
                UpdateDragRectOverlay();
            }

            if (!bNothing)
            {
                SCSIZE nDestPos = bToCols ? static_cast<SCSIZE>(nDestCol) : static_cast<SCSIZE>(nDestRow);
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
            SetPointer( Pointer( POINTER_PIVOT_DELETE ) );
        // if (bDragRect)
        //  DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, FALSE );
        bDragRect = FALSE;
        UpdateDragRectOverlay();

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
#endif

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

bool ScGridWindow::DPTestFieldPopupArrow(const MouseEvent& rMEvt, const ScAddress& rPos, ScDPObject* pDPObj)
{
    // Get the geometry of the cell.
    Point aScrPos = pViewData->GetScrPos(rPos.Col(), rPos.Row(), eWhich);
    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel(rPos.Col(), rPos.Row(), nSizeX, nSizeY);
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    ScDPFieldButton aBtn(this, &GetSettings().GetStyleSettings());
    aBtn.setBoundingBox(aScrPos, aScrSize);
    Point aPopupPos;
    Size aPopupSize;
    aBtn.getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRec(aPopupPos, aPopupSize);
    if (aRec.IsInside(rMEvt.GetPosPixel()))
    {
        // Mouse cursor inside the popup arrow box.  Launch the field menu.
        DPLaunchFieldPopupMenu(OutputToScreenPixel(aScrPos), aScrSize, rPos, pDPObj);
        return true;
    }

    return false;
}

namespace {

struct DPFieldPopupData : public ScDPFieldPopupWindow::ExtendedData
{
    ScPivotParam    maDPParam;
    ScDPObject*     mpDPObj;
    long            mnDim;
};

class DPFieldPopupOKAction : public ScMenuFloatingWindow::Action
{
public:
    explicit DPFieldPopupOKAction(ScGridWindow* p) :
        mpGridWindow(p) {}

    virtual void execute()
    {
        mpGridWindow->UpdateDPFromFieldPopupMenu();
    }
private:
    ScGridWindow* mpGridWindow;
};

class PopupSortAction : public ScMenuFloatingWindow::Action
{
public:
    enum SortType { ASCENDING, DESCENDING, CUSTOM };

    explicit PopupSortAction(const ScAddress& rPos, SortType eType, sal_uInt16 nUserListIndex, ScTabViewShell* pViewShell) :
        maPos(rPos), meType(eType), mnUserListIndex(nUserListIndex), mpViewShell(pViewShell) {}

    virtual void execute()
    {
        switch (meType)
        {
            case ASCENDING:
                mpViewShell->DataPilotSort(maPos, true);
            break;
            case DESCENDING:
                mpViewShell->DataPilotSort(maPos, false);
            break;
            case CUSTOM:
                mpViewShell->DataPilotSort(maPos, true, &mnUserListIndex);
            break;
            default:
                ;
        }
    }

private:
    ScAddress       maPos;
    SortType        meType;
    sal_uInt16      mnUserListIndex;
    ScTabViewShell* mpViewShell;
};

}

void ScGridWindow::DPLaunchFieldPopupMenu(
    const Point& rSrcPos, const Size& rSrcSize, const ScAddress& rPos, ScDPObject* pDPObj)
{
    // We need to get the list of field members.
    auto_ptr<DPFieldPopupData> pDPData(new DPFieldPopupData);
    pDPObj->FillLabelData(pDPData->maDPParam);
    pDPData->mpDPObj = pDPObj;

    USHORT nOrient;
    pDPData->mnDim = pDPObj->GetHeaderDim(rPos, nOrient);

    if (pDPData->maDPParam.maLabelArray.size() <= static_cast<size_t>(pDPData->mnDim))
        // out-of-bound dimension ID.  This should never happen!
        return;

    const ScDPLabelData& rLabelData = *pDPData->maDPParam.maLabelArray[pDPData->mnDim];

    mpDPFieldPopup.reset(new ScDPFieldPopupWindow(this));
    mpDPFieldPopup->setExtendedData(pDPData.release());
    mpDPFieldPopup->setOKAction(new DPFieldPopupOKAction(this));
    {
        sal_Int32 n = rLabelData.maMembers.getLength();
        mpDPFieldPopup->setMemberSize(n);
        for (sal_Int32 i = 0; i < n; ++i)
            mpDPFieldPopup->addMember(rLabelData.maMembers[i], rLabelData.maVisible[i]);
        mpDPFieldPopup->initMembers();
    }

    vector<OUString> aUserSortNames;
    ScUserList* pUserList = ScGlobal::GetUserList();
    if (pUserList)
    {
        sal_uInt16 n = pUserList->GetCount();
        aUserSortNames.reserve(n);
        for (sal_uInt16 i = 0; i < n; ++i)
        {
            ScUserListData* pData = static_cast<ScUserListData*>((*pUserList)[i]);
            aUserSortNames.push_back(pData->GetString());
        }
    }

    // Populate the menus.
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    mpDPFieldPopup->addMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_ASC).GetString(), true,
        new PopupSortAction(rPos, PopupSortAction::ASCENDING, 0, pViewShell));
    mpDPFieldPopup->addMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_DESC).GetString(), true,
        new PopupSortAction(rPos, PopupSortAction::DESCENDING, 0, pViewShell));
    ScMenuFloatingWindow* pSubMenu = mpDPFieldPopup->addSubMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_CUSTOM).GetString(), !aUserSortNames.empty());

    if (pSubMenu && !aUserSortNames.empty())
    {
        size_t n = aUserSortNames.size();
        for (size_t i = 0; i < n; ++i)
        {
            pSubMenu->addMenuItem(
                aUserSortNames[i], true,
                new PopupSortAction(rPos, PopupSortAction::CUSTOM, i, pViewShell));
        }
    }

    mpDPFieldPopup->SetPopupModeEndHdl( LINK(this, ScGridWindow, PopupModeEndHdl) );
    Rectangle aCellRect(rSrcPos, rSrcSize);
    mpDPFieldPopup->StartPopupMode(aCellRect, (FLOATWIN_POPUPMODE_DOWN | FLOATWIN_POPUPMODE_GRABFOCUS));
}

void ScGridWindow::UpdateDPFromFieldPopupMenu()
{
    if (!mpDPFieldPopup.get())
        return;

    DPFieldPopupData* pDPData = static_cast<DPFieldPopupData*>(mpDPFieldPopup->getExtendedData());
    if (!pDPData)
        return;

    ScDPObject* pDPObj = pDPData->mpDPObj;
    ScDPObject aNewDPObj(*pDPObj);
    aNewDPObj.BuildAllDimensionMembers();
    ScDPSaveData* pSaveData = aNewDPObj.GetSaveData();

    BOOL bIsDataLayout;
    String aDimName = pDPObj->GetDimName(pDPData->mnDim, bIsDataLayout);
    ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aDimName);
    if (!pDim)
        return;

    hash_map<OUString, bool, OUStringHash> aResult;
    mpDPFieldPopup->getResult(aResult);
    pDim->UpdateMemberVisibility(aResult);

    ScDBDocFunc aFunc(*pViewData->GetDocShell());
    aFunc.DataPilotUpdate(pDPObj, &aNewDPObj, true, false);
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




