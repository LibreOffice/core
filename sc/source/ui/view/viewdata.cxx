/*************************************************************************
 *
 *  $RCSfile: viewdata.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-27 10:46:10 $
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
#include <svx/adjitem.hxx>
#include <svx/algitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editview.hxx>
#include <svx/editstat.hxx>
#include <svx/outliner.hxx>
#include <svx/unolingu.hxx>

#include <vcl/svapp.hxx>
#include <vcl/system.hxx>

#include "viewdata.hxx"
#include "docoptio.hxx"
#include "scmod.hxx"
#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "sc.hrc"
#include "patattr.hxx"
#include "editutil.hxx"
#include "scextopt.hxx"

// STATIC DATA -----------------------------------------------------------

#define TAG_TABBARWIDTH "tw:"

static BOOL bMoveArea = FALSE;              //! Member?
USHORT nEditAdjust = SVX_ADJUST_LEFT;       //! Member !!!

//==================================================================

ScViewDataTable::ScViewDataTable() :
                nCurX( 0 ),
                nCurY( 0 ),
                bOldCurValid( FALSE ),
                eHSplitMode( SC_SPLIT_NONE ),
                eVSplitMode( SC_SPLIT_NONE ),
                nHSplitPos( 0 ),
                nVSplitPos( 0 ),
                nFixPosX( 0 ),
                nFixPosY( 0 ),
                eWhichActive( SC_SPLIT_BOTTOMLEFT )
{
    nPosX[0]=nPosX[1]=
    nPosY[0]=nPosY[1]=0;
    nTPosX[0]=nTPosX[1]=
    nTPosY[0]=nTPosY[1]=0;
    nMPosX[0]=nMPosX[1]=
    nMPosY[0]=nMPosY[1]=0;
    nPixPosX[0]=nPixPosX[1]=
    nPixPosY[0]=nPixPosY[1]=0;
}

ScViewDataTable::ScViewDataTable( const ScViewDataTable& rDataTable ) :
                nCurX( rDataTable.nCurX ),
                nCurY( rDataTable.nCurY ),
                bOldCurValid( rDataTable.bOldCurValid ),
                nOldCurX( rDataTable.nOldCurX ),
                nOldCurY( rDataTable.nOldCurY ),
                eHSplitMode( rDataTable.eHSplitMode ),
                eVSplitMode( rDataTable.eVSplitMode ),
                nHSplitPos( rDataTable.nHSplitPos ),
                nVSplitPos( rDataTable.nVSplitPos ),
                nFixPosX( rDataTable.nFixPosX ),
                nFixPosY( rDataTable.nFixPosY ),
                eWhichActive( rDataTable.eWhichActive )
{
    nPosX[0]=rDataTable.nPosX[0];
    nPosX[1]=rDataTable.nPosX[1];
    nPosY[0]=rDataTable.nPosY[0];
    nPosY[1]=rDataTable.nPosY[1];

    nTPosX[0]=rDataTable.nTPosX[0];
    nTPosX[1]=rDataTable.nTPosX[1];
    nTPosY[0]=rDataTable.nTPosY[0];
    nTPosY[1]=rDataTable.nTPosY[1];

    nMPosX[0]=rDataTable.nMPosX[0];
    nMPosX[1]=rDataTable.nMPosX[1];
    nMPosY[0]=rDataTable.nMPosY[0];
    nMPosY[1]=rDataTable.nMPosY[1];

    nPixPosX[0]=rDataTable.nPixPosX[0];
    nPixPosX[1]=rDataTable.nPixPosX[1];
    nPixPosY[0]=rDataTable.nPixPosY[0];
    nPixPosY[1]=rDataTable.nPixPosY[1];
}

ScViewDataTable::~ScViewDataTable()
{
}

//==================================================================

ScViewData::ScViewData( ScDocShell* pDocSh, ScTabViewShell* pViewSh )
    :   pDocShell   ( pDocSh ),
        pViewShell  ( pViewSh ),
        pDoc        ( NULL ),
        pView       ( pViewSh ),
        pOptions    ( new ScViewOptions ),
        nTabNo      ( 0 ),
        nRefTabNo   ( 0 ),
        aZoomX      ( 1,1 ),
        aZoomY      ( 1,1 ),
        aPageZoomX  ( 3,5 ),                    // Page-Default: 60%
        aPageZoomY  ( 3,5 ),
        aLogicMode  ( MAP_100TH_MM ),
        bIsRefMode  ( FALSE ),
        eRefType    ( SC_REFTYPE_NONE ),
        nFillMode   ( SC_FILL_NONE ),
        bDelMarkValid( FALSE ),
        bActive     ( TRUE ),                   //! wie initialisieren?
        bPagebreak  ( FALSE ),
        pSpellingView ( NULL )
{
    SetGridMode     ( TRUE );
    SetSyntaxMode   ( FALSE );
    SetHeaderMode   ( TRUE );
    SetTabMode      ( TRUE );
    SetVScrollMode  ( TRUE );
    SetHScrollMode  ( TRUE );
    SetOutlineMode  ( TRUE );

    aScrSize = Size( (long) ( STD_COL_WIDTH           * PIXEL_PER_TWIPS * OLE_STD_CELLS_X ),
                     (long) ( ScGlobal::nStdRowHeight * PIXEL_PER_TWIPS * OLE_STD_CELLS_Y ) );
    pTabData[0] = new ScViewDataTable;
    for (USHORT i=1; i<=MAXTAB; i++) pTabData[i] = NULL;
    pThisTab = pTabData[nTabNo];
    for (i=0; i<4; i++)
    {
        pEditView[i] = NULL;
        bEditActive[i] = FALSE;
    }

    nEditEndCol = nEditCol = nEditEndRow = nEditRow = 0;
    nTabStartCol = SC_TABSTART_NONE;

    if (pDocShell)
    {
        pDoc = pDocShell->GetDocument();
        *pOptions = pDoc->GetViewOptions();
    }

    //  keine ausgeblendete Tabelle anzeigen:
    if (pDoc && !pDoc->IsVisible(nTabNo))
    {
        while ( !pDoc->IsVisible(nTabNo) && pDoc->HasTable(nTabNo+1) )
            ++nTabNo;

        pTabData[nTabNo] = new ScViewDataTable;
        pThisTab = pTabData[nTabNo];
    }

    CalcPPT();
}

ScViewData::ScViewData( const ScViewData& rViewData )
    :   pDocShell   ( rViewData.pDocShell ),
        pViewShell  ( rViewData.pViewShell ),
        pDoc        ( rViewData.pDoc ),
        pView       ( rViewData.pView ),
        pOptions    ( new ScViewOptions( *(rViewData.pOptions) )  ),
        nTabNo      ( rViewData.nTabNo ),
        nRefTabNo   ( rViewData.nTabNo ),           // kein RefMode
        aZoomX      ( rViewData.aZoomX ),
        aZoomY      ( rViewData.aZoomY ),
        aPageZoomX  ( rViewData.aPageZoomX ),
        aPageZoomY  ( rViewData.aPageZoomY ),
        aLogicMode  ( rViewData.aLogicMode ),
        bIsRefMode  ( FALSE ),
        eRefType    ( SC_REFTYPE_NONE ),
        nFillMode   ( SC_FILL_NONE ),
        bDelMarkValid( FALSE ),
        bActive     ( TRUE ),                               //! wie initialisieren?
        bPagebreak  ( rViewData.bPagebreak ),
        pSpellingView ( rViewData.pSpellingView )
{
    SetGridMode     ( rViewData.IsGridMode() );
    SetSyntaxMode   ( rViewData.IsSyntaxMode() );
    SetHeaderMode   ( rViewData.IsHeaderMode() );
    SetTabMode      ( rViewData.IsTabMode() );
    SetVScrollMode  ( rViewData.IsVScrollMode() );
    SetHScrollMode  ( rViewData.IsHScrollMode() );
    SetOutlineMode  ( rViewData.IsOutlineMode() );

    aScrSize = rViewData.aScrSize;
    for (USHORT i=0; i<=MAXTAB; i++)
        if (rViewData.pTabData[i])
            pTabData[i] = new ScViewDataTable( *rViewData.pTabData[i] );
        else
            pTabData[i] = NULL;
    pThisTab = pTabData[nTabNo];
    for (i=0; i<4; i++)
    {
        pEditView[i] = NULL;
        bEditActive[i] = FALSE;
    }

    nEditEndCol = nEditCol = nEditEndRow = nEditRow = 0;
    nTabStartCol = SC_TABSTART_NONE;
    CalcPPT();
}

void ScViewData::SetDocShell( ScDocShell* pShell )
{
    pDocShell = pShell;
    pDoc = pDocShell->GetDocument();
    *pOptions = pDoc->GetViewOptions();
    CalcPPT();
}

void ScViewData::InitData( ScDocument* pDocument )
{
    pDoc = pDocument;
    *pOptions = pDoc->GetViewOptions();
}

void ScViewData::InitFrom( const ScViewData* pRef )
{
    if (pRef==NULL)
    {
        DBG_ERROR("ScViewData::InitFrom mit NULL");
        return;
    }

    aScrSize    = pRef->aScrSize;
    nTabNo      = pRef->nTabNo;
    aZoomX      = pRef->aZoomX;
    aZoomY      = pRef->aZoomY;
    aPageZoomX  = pRef->aPageZoomX;
    aPageZoomY  = pRef->aPageZoomY;
    bPagebreak  = pRef->bPagebreak;
    aLogicMode  = pRef->aLogicMode;

    SetGridMode     ( pRef->IsGridMode() );
    SetSyntaxMode   ( pRef->IsSyntaxMode() );
    SetHeaderMode   ( pRef->IsHeaderMode() );
    SetTabMode      ( pRef->IsTabMode() );
    SetVScrollMode  ( pRef->IsVScrollMode() );
    SetHScrollMode  ( pRef->IsHScrollMode() );
    SetOutlineMode  ( pRef->IsOutlineMode() );

    for (USHORT i=0; i<=MAXTAB; i++)
    {
        delete pTabData[i];
        if (pRef->pTabData[i])
            pTabData[i] = new ScViewDataTable( *pRef->pTabData[i] );
        else
            pTabData[i] = NULL;
    }
    pThisTab = pTabData[nTabNo];
    CalcPPT();
}

ScDocument* ScViewData::GetDocument() const
{
    if (pDoc)
        return pDoc;
    else if (pDocShell)
        return pDocShell->GetDocument();

    DBG_ERROR("kein Document an ViewData");
    return NULL;
}

ScViewData::~ScViewData()
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTabData[i])
            delete pTabData[i];

    KillEditView();
    delete pOptions;
}

void ScViewData::UpdateThis()
{
    do
    {
        pThisTab = pTabData[nTabNo];
        if (!pThisTab)
        {
            if (nTabNo>0)
                --nTabNo;
            else
                pThisTab = pTabData[0] = new ScViewDataTable;

                // hier keine Assertion, weil sonst Paints kommen, bevor alles initialisiert ist!
        }
    }
    while (!pThisTab);
}

void ScViewData::InsertTab( USHORT nTab )
{
    delete pTabData[MAXTAB];

    for (USHORT i=MAXTAB; i>nTab; i--)
        pTabData[i] = pTabData[i-1];

    pTabData[nTab] = new ScViewDataTable;

    UpdateThis();
    aMarkData.InsertTab( nTab );
}

void ScViewData::DeleteTab( USHORT nTab )
{
    delete pTabData[nTab];

    for (USHORT i=nTab; i<MAXTAB; i++)
        pTabData[i] = pTabData[i+1];

    pTabData[MAXTAB] = NULL;

    UpdateThis();
    aMarkData.DeleteTab( nTab );
}

void ScViewData::CopyTab( USHORT nSrcTab, USHORT nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = pDoc->GetTableCount() - 1;   // am Doc muss vorher kopiert worden sein

    if (nDestTab > MAXTAB)
    {
        DBG_ERROR("Zuviele Tabellen");
        return;
    }

    delete pTabData[MAXTAB];

    for (USHORT i=MAXTAB; i>nDestTab; i--)
        pTabData[i] = pTabData[i-1];

    if ( pTabData[nSrcTab] )
        pTabData[nDestTab] = new ScViewDataTable( *pTabData[nSrcTab] );
    else
        pTabData[nDestTab] = NULL;

    UpdateThis();
    aMarkData.InsertTab( nDestTab );
}

void ScViewData::MoveTab( USHORT nSrcTab, USHORT nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = pDoc->GetTableCount() - 1;

    USHORT i;
    ScViewDataTable* pTab = pTabData[nSrcTab];

    USHORT nInsTab = nDestTab;
    if ( nSrcTab < nDestTab )
    {
        --nInsTab;
        for (i=nSrcTab; i<nDestTab; i++)
            pTabData[i] = pTabData[i+1];
    }
    else
        for (i=nSrcTab; i>nDestTab; i--)
            pTabData[i] = pTabData[i-1];

    pTabData[nDestTab] = pTab;

    UpdateThis();
    aMarkData.DeleteTab( nSrcTab );
    aMarkData.InsertTab( nInsTab );         // ggf. angepasst
}

void ScViewData::UpdateOle( ScSplitPos eWhich )
{
    GetDocShell()->UpdateOle(this);
}

void ScViewData::SetViewShell( ScTabViewShell* pViewSh )
{
    if (pViewSh)
    {
        pViewShell  = pViewSh;
        pView       = pViewSh;
    }
    else
    {
        pViewShell  = NULL;
        pView       = NULL;
    }
}

void ScViewData::SetZoom( const Fraction& rNewX, const Fraction& rNewY )
{
    Fraction aFrac20( 1,5 );
    Fraction aFrac400( 4,1 );

    Fraction aValidX = rNewX;
    if (aValidX<aFrac20) aValidX = aFrac20;
    if (aValidX>aFrac400) aValidX = aFrac400;

    Fraction aValidY = rNewY;
    if (aValidY<aFrac20) aValidY = aFrac20;
    if (aValidY>aFrac400) aValidY = aFrac400;

    if ( bPagebreak )
    {
        aPageZoomX = aValidX;
        aPageZoomY = aValidY;
    }
    else
    {
        aZoomX = aValidX;
        aZoomY = aValidY;
    }

    CalcPPT();
    RecalcPixPos();
    aScenButSize = Size(0,0);
    aLogicMode.SetScaleX( aValidX );
    aLogicMode.SetScaleY( aValidY );
}

void ScViewData::SetPagebreakMode( BOOL bSet )
{
    bPagebreak = bSet;

    CalcPPT();
    RecalcPixPos();
    aScenButSize = Size(0,0);
    aLogicMode.SetScaleX( GetZoomX() );
    aLogicMode.SetScaleY( GetZoomY() );
}

BOOL ScViewData::GetSimpleArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rStartTab,
                                USHORT& rEndCol, USHORT& rEndRow, USHORT& rEndTab, BOOL bMergeMark )
{
    if ( aMarkData.IsMarked() || aMarkData.IsMultiMarked() )
    {
        if ( bMergeMark )
            if ( aMarkData.IsMultiMarked() )
                aMarkData.MarkToSimple();

        if ( aMarkData.IsMarked() && !aMarkData.IsMultiMarked() )
        {
            ScRange aMarkRange;
            aMarkData.GetMarkArea( aMarkRange );
            rStartCol = aMarkRange.aStart.Col();
            rStartRow = aMarkRange.aStart.Row();
            rStartTab = aMarkRange.aStart.Tab();
            rEndCol = aMarkRange.aEnd.Col();
            rEndRow = aMarkRange.aEnd.Row();
            rEndTab = aMarkRange.aEnd.Tab();
        }
        else
        {
            rStartCol = rEndCol = GetCurX();
            rStartRow = rEndRow = GetCurY();
            rStartTab = rEndTab = GetTabNo();
            return FALSE;
        }
    }
    else
    {
        rStartCol = rEndCol = GetCurX();
        rStartRow = rEndRow = GetCurY();
        rStartTab = rEndTab = GetTabNo();
    }
    return TRUE;
}

BOOL ScViewData::GetSimpleArea( ScRange& rRange, BOOL bMergeMark )
{
    if ( aMarkData.IsMarked() || aMarkData.IsMultiMarked() )
    {
        if ( bMergeMark )
            if ( aMarkData.IsMultiMarked() )
                aMarkData.MarkToSimple();

        if ( aMarkData.IsMarked() && !aMarkData.IsMultiMarked() )
            aMarkData.GetMarkArea( rRange );
        else
        {
            rRange = ScRange( GetCurX(), GetCurY(), GetTabNo() );
            return FALSE;
        }
    }
    else
    {
        rRange = ScRange( GetCurX(), GetCurY(), GetTabNo() );
    }
    return TRUE;
}

void ScViewData::GetMultiArea( ScRangeListRef& rRange, BOOL bMergeMark )
{
    BOOL bMulti = aMarkData.IsMultiMarked();
    if (bMulti && bMergeMark)
    {
        aMarkData.MarkToSimple();
        bMulti = aMarkData.IsMultiMarked();
    }
    if (bMulti)
    {
        rRange = new ScRangeList;
        aMarkData.FillRangeListWithMarks( rRange, FALSE );
    }
    else
    {
        ScRange aSimple;
        GetSimpleArea(aSimple,FALSE);
        rRange = new ScRangeList;
        rRange->Append(aSimple);
    }
}

BOOL ScViewData::SimpleColMarked()
{
    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;
    if (GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
        if (nStartRow==0 && nEndRow==MAXROW)
            return TRUE;

    return FALSE;
}

BOOL ScViewData::SimpleRowMarked()
{
    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;
    if (GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
        if (nStartCol==0 && nEndCol==MAXCOL)
            return TRUE;

    return FALSE;
}

void ScViewData::SetFillMode( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    nFillMode   = SC_FILL_FILL;
    nFillStartX = nStartCol;
    nFillStartY = nStartRow;
    nFillEndX   = nEndCol;
    nFillEndY   = nEndRow;
}

void ScViewData::SetDragMode( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                BYTE nMode )
{
    nFillMode   = nMode;
    nFillStartX = nStartCol;
    nFillStartY = nStartRow;
    nFillEndX   = nEndCol;
    nFillEndY   = nEndRow;
}

void ScViewData::ResetFillMode()
{
    nFillMode   = SC_FILL_NONE;
}

void ScViewData::GetFillData( USHORT& rStartCol, USHORT& rStartRow,
                                USHORT& rEndCol, USHORT& rEndRow )
{
    rStartCol = nFillStartX;
    rStartRow = nFillStartY;
    rEndCol   = nFillEndX;
    rEndRow   = nFillEndY;
}

USHORT ScViewData::GetOldCurX() const
{
    if (pThisTab->bOldCurValid)
        return pThisTab->nOldCurX;
    else
        return pThisTab->nCurX;
}

USHORT ScViewData::GetOldCurY() const
{
    if (pThisTab->bOldCurValid)
        return pThisTab->nOldCurY;
    else
        return pThisTab->nCurY;
}

void ScViewData::SetOldCursor( USHORT nNewX, USHORT nNewY )
{
    pThisTab->nOldCurX = nNewX;
    pThisTab->nOldCurY = nNewY;
    pThisTab->bOldCurValid = TRUE;
}

void ScViewData::ResetOldCursor()
{
    pThisTab->bOldCurValid = FALSE;
}

Rectangle ScViewData::GetEditArea( ScSplitPos eWhich, USHORT nPosX, USHORT nPosY,
                                    Window* pWin, const ScPatternAttr* pPattern,
                                    BOOL bForceToTop )
{
    return ScEditUtil( pDoc, nPosX, nPosY, nTabNo, GetScrPos(nPosX,nPosY,eWhich,TRUE),
                        pWin, nPPTX, nPPTY, GetZoomX(), GetZoomY() ).
                            GetEditArea( pPattern, bForceToTop );
}

void ScViewData::SetEditEngine( ScSplitPos eWhich,
                                ScEditEngineDefaulter* pNewEngine,
                                Window* pWin, USHORT nNewX, USHORT nNewY )
{
    ScHSplitPos eHWhich = WhichH(eWhich);

    BOOL bWasThere = FALSE;
    if (pEditView[eWhich])
    {
        //  Wenn die View schon da ist, nichts aufrufen, was die Cursorposition aendert

        if (bEditActive[eWhich])
            bWasThere = TRUE;
        else
            pEditView[eWhich]->SetEditEngine(pNewEngine);

        if (pEditView[eWhich]->GetWindow() != pWin)
        {
            pEditView[eWhich]->SetWindow(pWin);
            DBG_ERROR("EditView Window geaendert");
        }
    }
    else
    {
        pEditView[eWhich] = new EditView( pNewEngine, pWin );
    }

    //  bei IdleFormat wird manchmal ein Cursor gemalt, wenn die View schon weg ist (23576)

    ULONG nEC = pNewEngine->GetControlWord();
    pNewEngine->SetControlWord(nEC & ~EE_CNTRL_DOIDLEFORMAT);

    ULONG nVC = pEditView[eWhich]->GetControlWord();
    pEditView[eWhich]->SetControlWord(nVC & ~EV_CNTRL_AUTOSCROLL);

    bEditActive[eWhich] = TRUE;

    const ScPatternAttr* pPattern = pDoc->GetPattern( nNewX, nNewY, nTabNo );
    SvxCellHorJustify eJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                    pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();

    BOOL bBreak = ( eJust == SVX_HOR_JUSTIFY_BLOCK ) ||
                    ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

    nEditCol = nNewX;
    nEditRow = nNewY;
    const ScMergeAttr* pMergeAttr = (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
    nEditEndCol = nEditCol;
    if (pMergeAttr->GetColMerge() > 1)
        nEditEndCol += pMergeAttr->GetColMerge() - 1;
    nEditEndRow = nEditRow;
    if (pMergeAttr->GetRowMerge() > 1)
        nEditEndRow += pMergeAttr->GetRowMerge() - 1;

    Rectangle aPixRect = ScEditUtil( pDoc, nNewX,nNewY,nTabNo, GetScrPos(nNewX,nNewY,eWhich),
                                        pWin, nPPTX,nPPTY,GetZoomX(),GetZoomY() ).
                                            GetEditArea( pPattern, TRUE );

    //  bei rechtsbuendigem Editieren muss rechts noch Platz fuer den Cursor sein
    if ( nEditAdjust == SVX_ADJUST_RIGHT )
        aPixRect.Right() += 1;

    Rectangle aOutputArea = pWin->PixelToLogic( aPixRect, GetLogicMode() );
    pEditView[eWhich]->SetOutputArea( aOutputArea );

    if ( bActive && eWhich == GetActivePart() )
    {
        long nSizeXPix;
        if (bBreak)
            nSizeXPix = aPixRect.GetWidth();    // Papersize -> kein H-Scrolling
        else
        {
            DBG_ASSERT(pView,"keine View fuer EditView");
            nSizeXPix = pView->GetGridWidth(eHWhich) - aPixRect.Left();
        }
        DBG_ASSERT(pView,"keine View fuer EditView");
        long nSizeYPix = pView->GetGridHeight(WhichV(eWhich)) - aPixRect.Top();

        pNewEngine->SetPaperSize( pView->GetActiveWin()->PixelToLogic( Size( nSizeXPix, nSizeYPix ),
                                        GetLogicMode() ) );

        // sichtbarer Ausschnitt
        Size aPaper = pNewEngine->GetPaperSize();
        Rectangle aVis = pEditView[eWhich]->GetVisArea();
        long nDiff = aVis.Right() - aVis.Left();
        if ( nEditAdjust == SVX_ADJUST_RIGHT )
        {
            aVis.Right() = aPaper.Width() - 1;
            bMoveArea = TRUE;
        }
        else if ( nEditAdjust == SVX_ADJUST_CENTER )
        {
            aVis.Right() = ( aPaper.Width() - 1 + nDiff ) / 2;
            bMoveArea = TRUE;
        }
        else
        {
            aVis.Right() = nDiff;
            bMoveArea = FALSE;
        }
        aVis.Left() = aVis.Right() - nDiff;
        pEditView[eWhich]->SetVisArea(aVis);
        //

        pNewEngine->SetStatusEventHdl( LINK( this, ScViewData, EditEngineHdl ) );

        EditGrowY();        // an alten Text anpassen
        EditGrowX();

        Point aDocPos = pEditView[eWhich]->GetWindowPosTopLeft(0);
        if (aDocPos.Y() < aOutputArea.Top())
            pEditView[eWhich]->Scroll( 0, aOutputArea.Top() - aDocPos.Y() );

        //!     Status (Event) zuruecksetzen
    }

                                                    // hier muss bEditActive schon gesetzt sein
                                                    // (wegen Map-Mode bei Paint)
    if (!bWasThere)
        pNewEngine->InsertView(pEditView[eWhich]);

    //      Hintergrundfarbe der Zelle
    Color aBackCol = ((const SvxBrushItem&)pPattern->GetItem(ATTR_BACKGROUND)).GetColor();
    if ( aBackCol.GetTransparency() > 0 || IsSyntaxMode())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        aBackCol = rStyleSettings.GetWindowColor();
    }
    pEditView[eWhich]->SetBackgroundColor( aBackCol );

    pEditView[eWhich]->Invalidate();            //  noetig ??
    //  noetig, wenn Position geaendert
}

IMPL_LINK_INLINE_START( ScViewData, EmptyEditHdl, EditStatus *, pStatus )
{
    return 0;
}
IMPL_LINK_INLINE_END( ScViewData, EmptyEditHdl, EditStatus *, pStatus )

IMPL_LINK( ScViewData, EditEngineHdl, EditStatus *, pStatus )
{
    ULONG nStatus = pStatus->GetStatusWord();
    if (nStatus & (EE_STAT_HSCROLL | EE_STAT_TEXTHEIGHTCHANGED | EE_STAT_CURSOROUT))
    {
        EditGrowY();
        EditGrowX();

        if (nStatus & EE_STAT_CURSOROUT)
        {
            ScSplitPos eWhich = GetActivePart();
            if (pEditView[eWhich])
                pEditView[eWhich]->ShowCursor(FALSE);
        }
    }
    return 0;
}

void ScViewData::EditGrowX()
{
    ScDocument* pDoc = GetDocument();

    ScSplitPos eWhich = GetActivePart();
    ScHSplitPos eHWhich = WhichH(eWhich);
    EditView* pCurView = pEditView[eWhich];

    if ( !pCurView || !bEditActive[eWhich])
        return;

    ScEditEngineDefaulter* pEngine =
        (ScEditEngineDefaulter*) pCurView->GetEditEngine();
    Window* pWin = pCurView->GetWindow();

    USHORT nRight = GetPosX(eHWhich) + VisibleCellsX(eHWhich);

    Size        aSize = pEngine->GetPaperSize();
    Rectangle   aArea = pCurView->GetOutputArea();
    long        nOldRight = aArea.Right();

    //  Margin ist schon bei der urspruenglichen Breite beruecksichtigt
    long nTextWidth = pEngine->CalcTextWidth();

    BOOL bChanged = FALSE;
    while (aArea.GetWidth() + 0 < nTextWidth && nEditEndCol < nRight)
    {
        ++nEditEndCol;
        long nPix = (long) ( pDoc->GetColWidth( nEditEndCol, nTabNo ) * nPPTX );
        aArea.Right() += pWin->PixelToLogic(Size(nPix,0)).Width();

        if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            aArea.Right() = aArea.Left() + aSize.Width() - 1;

        bChanged = TRUE;
    }

    if (bChanged)
    {
        if ( bMoveArea )
        {
            //  hart auf linksbuendig schalten und VisArea wieder nach links setzen

            pEngine->SetDefaultItem( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );

            Rectangle aVis = pCurView->GetVisArea();
            long nMove = aVis.Left();
            aVis.Left() = 0;
            aVis.Right() -= nMove;
            pCurView->SetVisArea( aVis );
            bMoveArea = FALSE;
        }

        pCurView->SetOutputArea(aArea);

        aArea.Left() = nOldRight;
        pWin->Invalidate(aArea);
    }
}

void ScViewData::EditGrowY()
{
    ScSplitPos eWhich = GetActivePart();
    ScVSplitPos eVWhich = WhichV(eWhich);
    EditView* pCurView = pEditView[eWhich];

    if ( !pCurView || !bEditActive[eWhich])
        return;

    EditEngine* pEngine = pCurView->GetEditEngine();
    Window* pWin = pCurView->GetWindow();

    USHORT nBottom = GetPosY(eVWhich) + VisibleCellsY(eVWhich);

    Size        aSize = pEngine->GetPaperSize();
    Rectangle   aArea = pCurView->GetOutputArea();
    long        nOldBottom = aArea.Bottom();
    long        nTextHeight = pEngine->GetTextHeight();

    BOOL bChanged = FALSE;
    while (aArea.GetHeight() + 100 < nTextHeight && nEditEndRow < nBottom)
    {
        ++nEditEndRow;
        ScDocument* pDoc = GetDocument();
        long nPix = (long) ( pDoc->GetRowHeight( nEditEndRow, nTabNo ) * nPPTY );
        aArea.Bottom() += pWin->PixelToLogic(Size(0,nPix)).Height();

        if ( aArea.Bottom() > aArea.Top() + aSize.Height() - 1 )
            aArea.Bottom() = aArea.Top() + aSize.Height() - 1;

        bChanged = TRUE;
    }

    if (bChanged)
    {
        pCurView->SetOutputArea(aArea);

        if (nEditEndRow >= nBottom)
        {
            ULONG nControl = pEditView[eWhich]->GetControlWord();
            if ((nControl & EV_CNTRL_AUTOSCROLL) == 0)
                pCurView->SetControlWord( nControl | EV_CNTRL_AUTOSCROLL );
        }

        aArea.Top() = nOldBottom;
        pWin->Invalidate(aArea);
    }
}

void ScViewData::ResetEditView()
{
    EditEngine* pEngine = NULL;
    for (USHORT i=0; i<4; i++)
        if (pEditView[i])
        {
            if (bEditActive[i])
            {
                pEngine = pEditView[i]->GetEditEngine();
                pEngine->RemoveView(pEditView[i]);
//              pEditView[i]->SetSelection(0,0,0,0);
            }
            bEditActive[i] = FALSE;
        }

    if (pEngine)
        pEngine->SetStatusEventHdl( LINK( this, ScViewData, EmptyEditHdl ) );
}

void ScViewData::KillEditView()
{
    for (USHORT i=0; i<4; i++)
        if (pEditView[i])
        {
            if (bEditActive[i])
                pEditView[i]->GetEditEngine()->RemoveView(pEditView[i]);
            delete pEditView[i];
            pEditView[i] = NULL;
        }
}

void ScViewData::GetEditView( ScSplitPos eWhich, EditView*& rViewPtr, USHORT& rCol, USHORT& rRow )
{
    rViewPtr = pEditView[eWhich];
    rCol = nEditCol;
    rRow = nEditRow;
}

void ScViewData::SetTabNo( USHORT nNewTab )
{
    if (nNewTab>MAXTAB)
    {
        DBG_ERROR("falsche Tabellennummer");
        return;
    }

    nTabNo = nNewTab;
    if (!pTabData[nTabNo])
        pTabData[nTabNo] = new ScViewDataTable;
    pThisTab = pTabData[nTabNo];

    RecalcPixPos();     //! nicht immer noetig!
}

void ScViewData::SetActivePart( ScSplitPos eNewActive )
{
    pThisTab->eWhichActive = eNewActive;
}

Point ScViewData::GetScrPos( USHORT nWhereX, USHORT nWhereY, ScHSplitPos eWhich ) const
{
    DBG_ASSERT( eWhich==SC_SPLIT_LEFT || eWhich==SC_SPLIT_RIGHT, "Falsche Position" );
    ScSplitPos ePos = ( eWhich == SC_SPLIT_LEFT ) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
    return GetScrPos( nWhereX, nWhereY, ePos );
}

Point ScViewData::GetScrPos( USHORT nWhereX, USHORT nWhereY, ScVSplitPos eWhich ) const
{
    DBG_ASSERT( eWhich==SC_SPLIT_TOP || eWhich==SC_SPLIT_BOTTOM, "Falsche Position" );
    ScSplitPos ePos = ( eWhich == SC_SPLIT_TOP ) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
    return GetScrPos( nWhereX, nWhereY, ePos );
}

Point ScViewData::GetScrPos( USHORT nWhereX, USHORT nWhereY, ScSplitPos eWhich,
                                BOOL bAllowNeg ) const
{
    ScHSplitPos eWhichX;
    ScVSplitPos eWhichY;
    switch( eWhich )
    {
        case SC_SPLIT_TOPLEFT:
            eWhichX = SC_SPLIT_LEFT;
            eWhichY = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_TOPRIGHT:
            eWhichX = SC_SPLIT_RIGHT;
            eWhichY = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_BOTTOMLEFT:
            eWhichX = SC_SPLIT_LEFT;
            eWhichY = SC_SPLIT_BOTTOM;
            break;
        case SC_SPLIT_BOTTOMRIGHT:
            eWhichX = SC_SPLIT_RIGHT;
            eWhichY = SC_SPLIT_BOTTOM;
            break;
    }

    if (pView)
    {
        ((ScViewData*)this)->aScrSize.Width()  = pView->GetGridWidth(eWhichX);
        ((ScViewData*)this)->aScrSize.Height() = pView->GetGridHeight(eWhichY);
    }

    USHORT nTSize;

    USHORT  nPosX = GetPosX(eWhichX);
    USHORT  nX;

    long nScrPosX=0;
    if (nWhereX >= nPosX)
        for (nX=nPosX; nX<nWhereX && (bAllowNeg || nScrPosX<=aScrSize.Width()); nX++)
        {
            if ( nX > MAXCOL )
                nScrPosX = 65535;
            else
            {
                nTSize = pDoc->GetColWidth( nX, nTabNo );
                if (nTSize)
                {
                    long nSizeXPix = (long) ( nTSize * nPPTX );
                    nScrPosX += nSizeXPix;
                }
            }
        }
    else if (bAllowNeg)
        for (nX=nPosX; nX>nWhereX;)
        {
            --nX;
            nTSize = pDoc->GetColWidth( nX, nTabNo );
            if (nTSize)
            {
                long nSizeXPix = (long) ( nTSize * nPPTX );
                nScrPosX -= nSizeXPix;
            }
        }

    USHORT  nPosY = GetPosY(eWhichY);
    USHORT  nY;

    long nScrPosY=0;
    if (nWhereY >= nPosY)
        for (nY=nPosY; nY<nWhereY && (bAllowNeg || nScrPosY<=aScrSize.Height()); nY++)
        {
            if ( nY > MAXROW )
                nScrPosY = 65535;
            else
            {
                nTSize = pDoc->FastGetRowHeight( nY, nTabNo );
                if (nTSize)
                {
                    long nSizeYPix = (long) ( nTSize * nPPTY );
                    nScrPosY += nSizeYPix;
                }
            }
        }
    else if (bAllowNeg)
        for (nY=nPosY; nY>nWhereY;)
        {
            --nY;
            nTSize = pDoc->FastGetRowHeight( nY, nTabNo );
            if (nTSize)
            {
                long nSizeYPix = (long) ( nTSize * nPPTY );
                nScrPosY -= nSizeYPix;
            }
        }

    if (nScrPosX > 32767) nScrPosX=32767;
    if (nScrPosY > 32767) nScrPosY=32767;
    return Point( nScrPosX, nScrPosY );
}

//
//      Anzahl Zellen auf einem Bildschirm
//

USHORT ScViewData::CellsAtX( short nPosX, short nDir, ScHSplitPos eWhichX, USHORT nScrSizeX ) const
{
    DBG_ASSERT( nDir==1 || nDir==-1, "falscher CellsAt Aufruf" );

    if (pView)
        ((ScViewData*)this)->aScrSize.Width()  = pView->GetGridWidth(eWhichX);

    short   nX;
    USHORT  nScrPosX = 0;
    if (nScrSizeX == SC_SIZE_NONE) nScrSizeX = (USHORT) aScrSize.Width();

    if (nDir==1)
        nX = nPosX;             // vorwaerts
    else
        nX = nPosX-1;           // rueckwaerts

    BOOL bOut = FALSE;
    for ( ; nScrPosX<=nScrSizeX && !bOut; nX+=nDir )
    {
        short   nColNo = nX;
        if ( nColNo < 0 || nColNo > MAXCOL )
            bOut = TRUE;
        else
        {
            USHORT nTSize = pDoc->GetColWidth( nColNo, nTabNo );
            if (nTSize)
            {
                long nSizeXPix = (long) ( nTSize * nPPTX );
                nScrPosX += (USHORT) nSizeXPix;
            }
        }
    }

    if (nDir==1)
        nX -= nPosX;
    else
        nX = (nPosX-1)-nX;

    if (nX>0) --nX;
    return nX;
}

USHORT ScViewData::CellsAtY( short nPosY, short nDir, ScVSplitPos eWhichY, USHORT nScrSizeY ) const
{
    DBG_ASSERT( nDir==1 || nDir==-1, "falscher CellsAt Aufruf" );

    if (pView)
        ((ScViewData*)this)->aScrSize.Height() = pView->GetGridHeight(eWhichY);

    short   nY;
    USHORT  nScrPosY = 0;

    if (nScrSizeY == SC_SIZE_NONE) nScrSizeY = (USHORT) aScrSize.Height();

    if (nDir==1)
        nY = nPosY;             // vorwaerts
    else
        nY = nPosY-1;           // rueckwaerts

    BOOL bOut = FALSE;
    for ( ; nScrPosY<=nScrSizeY && !bOut; nY+=nDir )
    {
        short   nRowNo = nY;
        if ( nRowNo < 0 || nRowNo > MAXROW )
            bOut = TRUE;
        else
        {
//          USHORT nTSize = pDoc->GetRowHeight( nRowNo, nTabNo );
            USHORT nTSize = pDoc->FastGetRowHeight( nRowNo, nTabNo );
            if (nTSize)
            {
                long nSizeYPix = (long) ( nTSize * nPPTY );
                nScrPosY += (USHORT) nSizeYPix;
            }
        }
    }

    if (nDir==1)
        nY -= nPosY;
    else
        nY = (nPosY-1)-nY;

    if (nY>0) --nY;
    return nY;
}

USHORT ScViewData::VisibleCellsX( ScHSplitPos eWhichX ) const
{
    return CellsAtX( GetPosX( eWhichX ), 1, eWhichX, SC_SIZE_NONE );
}

USHORT ScViewData::VisibleCellsY( ScVSplitPos eWhichY ) const
{
    return CellsAtY( GetPosY( eWhichY ), 1, eWhichY, SC_SIZE_NONE );
}

USHORT ScViewData::PrevCellsX( ScHSplitPos eWhichX ) const
{
    return CellsAtX( GetPosX( eWhichX ), -1, eWhichX, SC_SIZE_NONE );
}

USHORT ScViewData::PrevCellsY( ScVSplitPos eWhichY ) const
{
    return CellsAtY( GetPosY( eWhichY ), -1, eWhichY, SC_SIZE_NONE );
}

USHORT ScViewData::LastCellsX( ScHSplitPos eWhichX ) const
{
    return CellsAtX( MAXCOL+1, -1, eWhichX, SC_SIZE_NONE );
}

USHORT ScViewData::LastCellsY( ScVSplitPos eWhichY ) const
{
    return CellsAtY( MAXROW+1, -1, eWhichY, SC_SIZE_NONE );
}

BOOL ScViewData::GetMergeSizePixel( USHORT nX, USHORT nY, long& rSizeXPix, long& rSizeYPix )
{
    const ScMergeAttr* pMerge = (const ScMergeAttr*) pDoc->GetAttr( nX,nY,nTabNo, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
    {
        long nOutWidth = 0;
        long nOutHeight = 0;
        USHORT i;
        USHORT nCountX = pMerge->GetColMerge();
        for (i=0; i<nCountX; i++)
            nOutWidth += (long) ( pDoc->GetColWidth(nX+i,nTabNo) * nPPTX );
        USHORT nCountY = pMerge->GetRowMerge();
        for (i=0; i<nCountY; i++)
            nOutHeight += (long) ( pDoc->GetRowHeight(nY+i,nTabNo) * nPPTY );

        rSizeXPix = nOutWidth;
        rSizeYPix = nOutHeight;
        return TRUE;
    }
    else
    {
        rSizeXPix = (long) ( pDoc->GetColWidth( nX, nTabNo ) * nPPTX );
        rSizeYPix = (long) ( pDoc->GetRowHeight( nY, nTabNo ) * nPPTY );
        return FALSE;
    }
}

BOOL ScViewData::GetPosFromPixel( long nClickX, long nClickY, ScSplitPos eWhich,
                                        short& rPosX, short& rPosY,
                                        BOOL bTestMerge, BOOL bRepair, BOOL bNextIfLarge )
{
    //  Hack, damit auch bei "ganzer Bildschirm" ohne Spalten/Zeilenkoepfe
    //  noch nach links/oben gescrollt werden kann:
    if (nClickX == 0)
        nClickX = -1;
    if (nClickY == 0)
        nClickY = -1;

    ScHSplitPos eHWhich = WhichH(eWhich);
    ScVSplitPos eVWhich = WhichV(eWhich);
    short nStartPosX = GetPosX(eHWhich);
    short nStartPosY = GetPosY(eVWhich);
    rPosX = nStartPosX;
    rPosY = nStartPosY;
    long nScrX = 0;
    long nScrY = 0;

    if (nClickX > 0)
    {
        while ( rPosX<=MAXCOL && nClickX >= nScrX )
        {
            nScrX += (long) ( pDoc->GetColWidth( rPosX, nTabNo ) * nPPTX );
            ++rPosX;
        }
        --rPosX;
    }
    else
    {
        while ( rPosX>0 && nClickX < nScrX )
        {
            --rPosX;
            nScrX -= (long) ( pDoc->GetColWidth( rPosX, nTabNo ) * nPPTX );
        }
    }

    if (nClickY > 0)
    {
        while ( rPosY<=MAXROW && nClickY >= nScrY )
        {
            nScrY += (long) ( pDoc->FastGetRowHeight( rPosY, nTabNo ) * nPPTY );
            ++rPosY;
        }
        --rPosY;
    }
    else
    {
        while ( rPosY>0 && nClickY < nScrY )
        {
            --rPosY;
            nScrY -= (long) ( pDoc->FastGetRowHeight( rPosY, nTabNo ) * nPPTY );
        }
    }

    if (bNextIfLarge)       //  zu grosse Zellen ?
    {
        if ( rPosX == nStartPosX && nClickX > 0 )
        {
            if (pView)
                aScrSize.Width() = pView->GetGridWidth(eHWhich);
            if ( nClickX > aScrSize.Width() )
                ++rPosX;
        }
        if ( rPosY == nStartPosY && nClickY > 0 )
        {
            if (pView)
                aScrSize.Height() = pView->GetGridHeight(eVWhich);
            if ( nClickY > aScrSize.Height() )
                ++rPosY;
        }
    }

    if (rPosX<0) rPosX=0;
    if (rPosX>MAXCOL) rPosX=MAXCOL;
    if (rPosY<0) rPosY=0;
    if (rPosY>MAXROW) rPosY=MAXROW;

    if (bTestMerge)
    {
        //! public Methode um Position anzupassen

        BOOL bHOver = FALSE;
        while (pDoc->IsHorOverlapped( rPosX, rPosY, nTabNo ))
            { --rPosX; bHOver=TRUE; }
        BOOL bVOver = FALSE;
        while (pDoc->IsVerOverlapped( rPosX, rPosY, nTabNo ))
            { --rPosY; bVOver=TRUE; }

        if ( bRepair && ( bHOver || bVOver ) )
        {
            const ScMergeAttr* pMerge = (const ScMergeAttr*)
                                pDoc->GetAttr( rPosX, rPosY, nTabNo, ATTR_MERGE );
            if ( ( bHOver && pMerge->GetColMerge() <= 1 ) ||
                 ( bVOver && pMerge->GetRowMerge() <= 1 ) )
            {
                DBG_ERROR("Merge-Fehler gefunden");

                pDoc->RemoveFlagsTab( 0,0, MAXCOL,MAXROW, nTabNo, SC_MF_HOR | SC_MF_VER );
                USHORT nEndCol = MAXCOL;
                USHORT nEndRow = MAXROW;
                pDoc->ExtendMerge( 0,0, nEndCol,nEndRow, nTabNo, TRUE, FALSE );
                if (pDocShell)
                    pDocShell->PostPaint( ScRange(0,0,nTabNo,MAXCOL,MAXROW,nTabNo), PAINT_GRID );
            }
        }
    }

    return FALSE;
}

void ScViewData::GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
                                        short nPosX, short nPosY, BOOL& rLeft, BOOL& rTop )
{
    Point aCellStart = GetScrPos( nPosX, nPosY, eWhich, TRUE );
    long nSizeX;
    long nSizeY;
    GetMergeSizePixel( nPosX, nPosY, nSizeX, nSizeY );
    rLeft = rClickPos.X() - aCellStart.X() <= nSizeX / 2;
    rTop  = rClickPos.Y() - aCellStart.Y() <= nSizeY / 2;
}

void ScViewData::SetPosX( ScHSplitPos eWhich, USHORT nNewPosX )
{
    if (nNewPosX)
    {
        USHORT nOldPosX = pThisTab->nPosX[eWhich];
        long nTPosX = pThisTab->nTPosX[eWhich];
        long nPixPosX = pThisTab->nPixPosX[eWhich];
        USHORT i;
        if ( nNewPosX > nOldPosX )
            for ( i=nOldPosX; i<nNewPosX; i++ )
            {
                long nThis = pDoc->GetColWidth( i,nTabNo );
                nTPosX -= nThis;
                nPixPosX -= (long)(nThis*nPPTX);
            }
        else
            for ( i=nNewPosX; i<nOldPosX; i++ )
            {
                long nThis = pDoc->GetColWidth( i,nTabNo );
                nTPosX += nThis;
                nPixPosX += (long)(nThis*nPPTX);
            }

        pThisTab->nPosX[eWhich] = nNewPosX;
        pThisTab->nTPosX[eWhich] = nTPosX;
        pThisTab->nMPosX[eWhich] = (long) (nTPosX * HMM_PER_TWIPS);
        pThisTab->nPixPosX[eWhich] = nPixPosX;
    }
    else
        pThisTab->nPixPosX[eWhich] =
        pThisTab->nTPosX[eWhich] =
        pThisTab->nMPosX[eWhich] =
        pThisTab->nPosX[eWhich] = 0;
}

void ScViewData::SetPosY( ScVSplitPos eWhich, USHORT nNewPosY )
{
    if (nNewPosY)
    {
        USHORT nOldPosY = pThisTab->nPosY[eWhich];
        long nTPosY = pThisTab->nTPosY[eWhich];
        long nPixPosY = pThisTab->nPixPosY[eWhich];
        USHORT i;
        if ( nNewPosY > nOldPosY )
            for ( i=nOldPosY; i<nNewPosY; i++ )
            {
                long nThis = pDoc->FastGetRowHeight( i,nTabNo );
                nTPosY -= nThis;
                nPixPosY -= (long)(nThis*nPPTY);
            }
        else
            for ( i=nNewPosY; i<nOldPosY; i++ )
            {
                long nThis = pDoc->FastGetRowHeight( i,nTabNo );
                nTPosY += nThis;
                nPixPosY += (long)(nThis*nPPTY);
            }

        pThisTab->nPosY[eWhich] = nNewPosY;
        pThisTab->nTPosY[eWhich] = nTPosY;
        pThisTab->nMPosY[eWhich] = (long) (nTPosY * HMM_PER_TWIPS);
        pThisTab->nPixPosY[eWhich] = nPixPosY;
    }
    else
        pThisTab->nPixPosY[eWhich] =
        pThisTab->nTPosY[eWhich] =
        pThisTab->nMPosY[eWhich] =
        pThisTab->nPosY[eWhich] = 0;
}

void ScViewData::RecalcPixPos()             // nach Zoom-Aenderungen
{
    USHORT i;

    for (USHORT eWhich=0; eWhich<2; eWhich++)
    {
        long nPixPosX = 0;
        USHORT nPosX = pThisTab->nPosX[eWhich];
        for (i=0; i<nPosX; i++)
            nPixPosX -= (long)(pDoc->GetColWidth(i,nTabNo) * nPPTX);
        pThisTab->nPixPosX[eWhich] = nPixPosX;

        long nPixPosY = 0;
        USHORT nPosY = pThisTab->nPosY[eWhich];
        for (i=0; i<nPosY; i++)
            nPixPosY -= (long)(pDoc->FastGetRowHeight(i,nTabNo) * nPPTY);
        pThisTab->nPixPosY[eWhich] = nPixPosY;
    }
}

const MapMode& ScViewData::GetLogicMode( ScSplitPos eWhich )
{
    aLogicMode.SetOrigin( Point( pThisTab->nMPosX[WhichH(eWhich)],
                                    pThisTab->nMPosY[WhichV(eWhich)] ) );
    return aLogicMode;
}

const MapMode& ScViewData::GetLogicMode()
{
    aLogicMode.SetOrigin( Point() );
    return aLogicMode;
}

void ScViewData::SetScreen( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
{
    USHORT nCol;
    USHORT nRow;
    USHORT nTSize;
    long nSizePix;
    long nScrPosX = 0;
    long nScrPosY = 0;

    SetActivePart( SC_SPLIT_BOTTOMLEFT );
    SetPosX( SC_SPLIT_LEFT, nCol1 );
    SetPosY( SC_SPLIT_BOTTOM, nRow1 );

    for (nCol=nCol1; nCol<=nCol2; nCol++)
    {
        nTSize = pDoc->GetColWidth( nCol, nTabNo );
        if (nTSize)
        {
            nSizePix = (long) ( nTSize * nPPTX );
            nScrPosX += (USHORT) nSizePix;
        }
    }

    for (nRow=nRow1; nRow<=nRow2; nRow++)
    {
        nTSize = pDoc->FastGetRowHeight( nRow, nTabNo );
        if (nTSize)
        {
            nSizePix = (long) ( nTSize * nPPTY );
            nScrPosY += (USHORT) nSizePix;
        }
    }

    aScrSize = Size( nScrPosX, nScrPosY );
}

void ScViewData::SetScreenPos( const Point& rVisAreaStart )
{
    long nSize;
    long nTwips;
    long nAdd;
    BOOL bEnd;

    nSize = 0;
    nTwips = (long) (rVisAreaStart.X() / HMM_PER_TWIPS);
    USHORT nX1 = 0;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pDoc->GetColWidth(nX1,nTabNo);
        if (nSize+nAdd <= nTwips+1 && nX1<MAXCOL)
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = TRUE;
    }

    nSize = 0;
    nTwips = (long) (rVisAreaStart.Y() / HMM_PER_TWIPS);
    USHORT nY1 = 0;
    bEnd = FALSE;
    while (!bEnd)
    {
        nAdd = (long) pDoc->FastGetRowHeight(nY1,nTabNo);
        if (nSize+nAdd <= nTwips+1 && nY1<MAXROW)
        {
            nSize += nAdd;
            ++nY1;
        }
        else
            bEnd = TRUE;
    }

    SetActivePart( SC_SPLIT_BOTTOMLEFT );
    SetPosX( SC_SPLIT_LEFT, nX1 );
    SetPosY( SC_SPLIT_BOTTOM, nY1 );

    SetCurX( nX1 );
    SetCurY( nY1 );
}

void ScViewData::SetScreen( const Rectangle& rVisArea )
{
    SetScreenPos( rVisArea.TopLeft() );

    //  hier ohne GetOutputFactor(), weil fuer Ausgabe in Metafile

    aScrSize = rVisArea.GetSize();
    aScrSize.Width() = (long)
        ( aScrSize.Width() * ScGlobal::nScreenPPTX / HMM_PER_TWIPS );
    aScrSize.Height() = (long)
        ( aScrSize.Height() * ScGlobal::nScreenPPTY / HMM_PER_TWIPS );
}

SfxObjectShell* ScViewData::GetSfxDocShell() const
{
    return pDocShell;
}

SfxBindings& ScViewData::GetBindings()
{
    DBG_ASSERT( pViewShell, "GetBindings() without ViewShell" );
    return pViewShell->GetViewFrame()->GetBindings();
}

SfxDispatcher& ScViewData::GetDispatcher()
{
    DBG_ASSERT( pViewShell, "GetDispatcher() without ViewShell" );
    return *pViewShell->GetViewFrame()->GetDispatcher();
}

Window* ScViewData::GetDialogParent()
{
    DBG_ASSERT( pViewShell, "GetDialogParent() ohne ViewShell" );
    return pViewShell->GetDialogParent();
}

Window* ScViewData::GetActiveWin()
{
    DBG_ASSERT( pView, "GetActiveWin() ohne View" );
    return pView->GetActiveWin();
}

ScDrawView* ScViewData::GetScDrawView()
{
    DBG_ASSERT( pView, "GetScDrawView() ohne View" );
    return pView->GetScDrawView();
}

BOOL ScViewData::IsMinimized()
{
    DBG_ASSERT( pView, "IsMinimized() ohne View" );
    return pView->IsMinimized();
}

void ScViewData::UpdateScreenZoom( const Fraction& rNewX, const Fraction& rNewY )
{
    Fraction aOldX = GetZoomX();
    Fraction aOldY = GetZoomY();

    SetZoom(rNewX,rNewY);

    Fraction aWidth = GetZoomX();
    aWidth *= Fraction( aScrSize.Width(),1 );
    aWidth /= aOldX;

    Fraction aHeight = GetZoomY();
    aHeight *= Fraction( aScrSize.Height(),1 );
    aHeight /= aOldY;

    aScrSize.Width()  = (long) aWidth;
    aScrSize.Height() = (long) aHeight;
}

void ScViewData::CalcPPT()
{
    nPPTX = ScGlobal::nScreenPPTX * (double) GetZoomX();
    if (pDocShell)
        nPPTX = nPPTX / pDocShell->GetOutputFactor();   // Faktor ist Drucker zu Bildschirm
    nPPTY = ScGlobal::nScreenPPTY * (double) GetZoomY();
}

//------------------------------------------------------------------

#define SC_OLD_TABSEP   '/'
#define SC_NEW_TABSEP   '+'

void ScViewData::WriteUserData(String& rData)
{
    //  nZoom (bis 364v) oder nZoom/nPageZoom/bPageMode (ab 364w)
    //  nTab
    //  Tab-ControlBreite
    //  pro Tabelle:
    //  CursorX/CursorY/HSplitMode/VSplitMode/HSplitPos/VSplitPos/SplitActive/
    //  PosX[links]/PosX[rechts]/PosY[oben]/PosY[unten]
    //  wenn Zeilen groesser 8192, "+" statt "/"

    USHORT nZoom = (USHORT)((aZoomY.GetNumerator() * 100) / aZoomY.GetDenominator());
    rData = String::CreateFromInt32( nZoom );
    rData += '/';
    nZoom = (USHORT)((aPageZoomY.GetNumerator() * 100) / aPageZoomY.GetDenominator());
    rData += String::CreateFromInt32( nZoom );
    rData += '/';
    if (bPagebreak)
        rData += '1';
    else
        rData += '0';

    rData += ';';
    rData += String::CreateFromInt32( nTabNo );
    rData += ';';
    rData.AppendAscii(RTL_CONSTASCII_STRINGPARAM( TAG_TABBARWIDTH ));
    rData += String::CreateFromInt32( pView->GetTabBarWidth() );

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT i=0; i<nTabCount; i++)
    {
        rData += ';';                   // Numerierung darf auf keinen Fall durcheinanderkommen
        if (pTabData[i])
        {
            sal_Unicode cTabSep = SC_OLD_TABSEP;                // wie 3.1
            if ( pTabData[i]->nCurY > MAXROW_30 ||
                 pTabData[i]->nPosY[0] > MAXROW_30 || pTabData[i]->nPosY[1] > MAXROW_30 ||
                 ( pTabData[i]->eVSplitMode == SC_SPLIT_FIX &&
                    pTabData[i]->nFixPosY > MAXROW_30 ) )
            {
                cTabSep = SC_NEW_TABSEP;        // um eine 3.1-Version nicht umzubringen
            }


            rData += String::CreateFromInt32( pTabData[i]->nCurX );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->nCurY );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->eHSplitMode );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->eVSplitMode );
            rData += cTabSep;
            if ( pTabData[i]->eHSplitMode == SC_SPLIT_FIX )
                rData += String::CreateFromInt32( pTabData[i]->nFixPosX );
            else
                rData += String::CreateFromInt32( pTabData[i]->nHSplitPos );
            rData += cTabSep;
            if ( pTabData[i]->eVSplitMode == SC_SPLIT_FIX )
                rData += String::CreateFromInt32( pTabData[i]->nFixPosY );
            else
                rData += String::CreateFromInt32( pTabData[i]->nVSplitPos );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->eWhichActive );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->nPosX[0] );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->nPosX[1] );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->nPosY[0] );
            rData += cTabSep;
            rData += String::CreateFromInt32( pTabData[i]->nPosY[1] );
        }
    }
}

void ScViewData::ReadUserData(const String& rData)
{
    if (!rData.Len())       // Leerer String kommt bei "neu Laden"
        return;             // dann auch ohne Assertion beenden

    xub_StrLen nCount = rData.GetTokenCount(';');
    if ( nCount <= 2 )
    {
        //  #45208# beim Reload in der Seitenansicht sind evtl. die Preview-UserData
        //  stehengelassen worden. Den Zoom von der Preview will man hier nicht...
        DBG_ERROR("ReadUserData: das sind nicht meine Daten");
        return;
    }

    String aTabOpt;
    xub_StrLen nTagLen = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(TAG_TABBARWIDTH)).Len();

    //-------------------
    // nicht pro Tabelle:
    //-------------------
    USHORT nTabStart = 2;

    String aZoomStr = rData.GetToken(0);                        // Zoom/PageZoom/Modus
    USHORT nNormZoom = aZoomStr.GetToken(0,'/').ToInt32();
    if ( nNormZoom >= MINZOOM && nNormZoom <= MAXZOOM )
        aZoomX = aZoomY = Fraction( nNormZoom, 100 );           //  "normaler" Zoom (immer)
    USHORT nPageZoom = aZoomStr.GetToken(1,'/').ToInt32();
    if ( nPageZoom >= MINZOOM && nPageZoom <= MAXZOOM )
        aPageZoomX = aPageZoomY = Fraction( nPageZoom, 100 );   // Pagebreak-Zoom, wenn gesetzt
    sal_Unicode cMode = aZoomStr.GetToken(2,'/').GetChar(0);    // 0 oder "0"/"1"
    SetPagebreakMode( cMode == '1' );
    // SetPagebreakMode muss immer gerufen werden wegen CalcPPT / RecalcPixPos()

    //
    //  Tabelle kann ungueltig geworden sein (z.B. letzte Version):
    //
    USHORT nNewTab = rData.GetToken(1).ToInt32();
    if (pDoc->HasTable( nNewTab ))
        SetTabNo(nNewTab);

    //
    // wenn vorhanden, TabBar-Breite holen:
    //
    aTabOpt = rData.GetToken(2);

    if ( nTagLen && aTabOpt.Copy(0,nTagLen).EqualsAscii(TAG_TABBARWIDTH) )
    {
        pView->SetTabBarWidth( aTabOpt.Copy(nTagLen).ToInt32() );
        nTabStart = 3;
    }

    //-------------
    // pro Tabelle:
    //-------------
    USHORT nPos = 0;
    while ( nCount > nPos+nTabStart )
    {
        aTabOpt = rData.GetToken(nPos+nTabStart);
        if (!pTabData[nPos])
            pTabData[nPos] = new ScViewDataTable;

        sal_Unicode cTabSep = 0;
        if (aTabOpt.GetTokenCount(SC_OLD_TABSEP) >= 11)
            cTabSep = SC_OLD_TABSEP;
#ifndef SC_LIMIT_ROWS
        else if (aTabOpt.GetTokenCount(SC_NEW_TABSEP) >= 11)
            cTabSep = SC_NEW_TABSEP;
        // '+' ist nur erlaubt, wenn wir mit Zeilen > 8192 umgehen koennen
#endif

        if (cTabSep)
        {
            pTabData[nPos]->nCurX = aTabOpt.GetToken(0,cTabSep).ToInt32();
            pTabData[nPos]->nCurY = aTabOpt.GetToken(1,cTabSep).ToInt32();
            pTabData[nPos]->eHSplitMode = (ScSplitMode) aTabOpt.GetToken(2,cTabSep).ToInt32();
            pTabData[nPos]->eVSplitMode = (ScSplitMode) aTabOpt.GetToken(3,cTabSep).ToInt32();

            if ( pTabData[nPos]->eHSplitMode == SC_SPLIT_FIX )
            {
                pTabData[nPos]->nFixPosX = aTabOpt.GetToken(4,cTabSep).ToInt32();
                UpdateFixX(nPos);
            }
            else
                pTabData[nPos]->nHSplitPos = aTabOpt.GetToken(4,cTabSep).ToInt32();

            if ( pTabData[nPos]->eVSplitMode == SC_SPLIT_FIX )
            {
                pTabData[nPos]->nFixPosY = aTabOpt.GetToken(5,cTabSep).ToInt32();
                UpdateFixY(nPos);
            }
            else
                pTabData[nPos]->nVSplitPos = aTabOpt.GetToken(5,cTabSep).ToInt32();

            pTabData[nPos]->eWhichActive = (ScSplitPos) aTabOpt.GetToken(6,cTabSep).ToInt32();
            pTabData[nPos]->nPosX[0] = aTabOpt.GetToken(7,cTabSep).ToInt32();
            pTabData[nPos]->nPosX[1] = aTabOpt.GetToken(8,cTabSep).ToInt32();
            pTabData[nPos]->nPosY[0] = aTabOpt.GetToken(9,cTabSep).ToInt32();
            pTabData[nPos]->nPosY[1] = aTabOpt.GetToken(10,cTabSep).ToInt32();

            //  Test, ob der aktive Teil laut SplitMode ueberhaupt existiert
            //  (Bug #44516#)
            ScSplitPos eTest = pTabData[nPos]->eWhichActive;
            if ( ( WhichH( eTest ) == SC_SPLIT_RIGHT &&
                    pTabData[nPos]->eHSplitMode == SC_SPLIT_NONE ) ||
                 ( WhichV( eTest ) == SC_SPLIT_TOP &&
                    pTabData[nPos]->eVSplitMode == SC_SPLIT_NONE ) )
            {
                //  dann wieder auf Default (unten links)
                pTabData[nPos]->eWhichActive = SC_SPLIT_BOTTOMLEFT;
                DBG_ERROR("SplitPos musste korrigiert werden");
            }
        }
        ++nPos;
    }

    RecalcPixPos();
}

void ScViewData::ReadExtOptions( const ScExtDocOptions& rOpt )
{
    //  fuer Excel-Import

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        const ScExtTabOptions* pExtTab = rOpt.GetExtTabOptions( nTab );
        if ( pExtTab )
        {
            if (!pTabData[nTab])
                pTabData[nTab] = new ScViewDataTable;
            ScViewDataTable* pViewTab = pTabData[nTab];

#if 0
            //  aDim ist nicht der sichtbare, sondern der belegte Bereich
            //! GT sollte mal die linke obere Ecke des sichtbaren Bereichs herausruecken

            if ( pExtTab->bValidDim )       // Scroll-Position muss vor Fixierung kommen
            {
                ScRange aVisRange = pExtTab->aDim;
                pViewTab->nPosX[SC_SPLIT_LEFT] = aVisRange.aStart.Col();
                ScVSplitPos eDefV = pExtTab->nSplitY ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
                pViewTab->nPosY[eDefV] = aVisRange.aStart.Row();
            }
#endif

            if ( pExtTab->nSplitX || pExtTab->nSplitY )
            {
                if ( pExtTab->bFrozen )
                {
                    if ( pExtTab->nSplitX )
                    {
                        pViewTab->eHSplitMode = SC_SPLIT_FIX;
                        pViewTab->nFixPosX = pExtTab->nSplitX;
                        pViewTab->nHSplitPos = 0;
                        UpdateFixX(nTab);
                        pViewTab->nPosX[SC_SPLIT_RIGHT]  = pExtTab->nLeftSplitCol;
                    }
                    if ( pExtTab->nSplitY )
                    {
                        pViewTab->eVSplitMode = SC_SPLIT_FIX;
                        pViewTab->nFixPosY = pExtTab->nSplitY;
                        pViewTab->nVSplitPos = 0;
                        UpdateFixY(nTab);
                        pViewTab->nPosY[SC_SPLIT_BOTTOM] = pExtTab->nTopSplitRow;
                    }
                }
                else
                {
                    Point aPixel = Application::GetDefaultDevice()->LogicToPixel(
                                    Point( pExtTab->nSplitX, pExtTab->nSplitY ),
                                    MapMode( MAP_TWIP ) );  //! Zoom?
                    if ( pDocShell )
                    {
                        double nFactor = pDocShell->GetOutputFactor();
                        aPixel.X() = (long)( aPixel.X() * nFactor + 0.5 );
                    }
                    if ( pExtTab->nSplitX )
                    {
                        pViewTab->eHSplitMode = SC_SPLIT_NORMAL;
                        pViewTab->nHSplitPos = aPixel.X();
                        pViewTab->nPosX[SC_SPLIT_RIGHT]  = pExtTab->nLeftSplitCol;
                    }
                    if ( pExtTab->nSplitY )
                    {
                        pViewTab->eVSplitMode = SC_SPLIT_NORMAL;
                        pViewTab->nVSplitPos = aPixel.Y();
                        pViewTab->nPosY[SC_SPLIT_BOTTOM] = pExtTab->nTopSplitRow;
                    }
                }

                ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
                switch ( pExtTab->nActPane )
                {
                    case 0: ePos = SC_SPLIT_BOTTOMRIGHT; break;
                    case 1: ePos = SC_SPLIT_TOPRIGHT;    break;
                    case 2: ePos = SC_SPLIT_BOTTOMLEFT;  break;
                    case 3: ePos = SC_SPLIT_TOPLEFT;     break;
                }
                if ( pViewTab->eHSplitMode == SC_SPLIT_NONE )   // hor. nicht geteilt?
                {
                    if (ePos == SC_SPLIT_BOTTOMRIGHT)           // dann immer links
                        ePos = SC_SPLIT_BOTTOMLEFT;
                    else if (ePos == SC_SPLIT_TOPRIGHT)
                        ePos = SC_SPLIT_TOPLEFT;
                }
                if ( pViewTab->eVSplitMode == SC_SPLIT_NONE )   // vert. nicht geteilt?
                {
                    if (ePos == SC_SPLIT_TOPLEFT)               // dann immer unten
                        ePos = SC_SPLIT_BOTTOMLEFT;
                    else if (ePos == SC_SPLIT_TOPRIGHT)
                        ePos = SC_SPLIT_BOTTOMRIGHT;
                }
                pViewTab->eWhichActive = ePos;
            }

            if ( pExtTab->bValidSel )
            {
                ScRange aSelRange = pExtTab->aLastSel;
                pViewTab->nCurX = aSelRange.aStart.Col();
                pViewTab->nCurY = aSelRange.aStart.Row();
            }
        }
    }

    SetTabNo( rOpt.nActTab );

    // RecalcPixPos oder so - auch nMPos - auch bei ReadUserData ??!?!
}

void ScViewData::SetOptions( const ScViewOptions& rOpt )
{
    //  if visibility of horiz. ScrollBar is changed, TabBar may have to be resized...
    BOOL bHScrollChanged = ( rOpt.GetOption(VOPT_HSCROLL) != pOptions->GetOption(VOPT_HSCROLL) );

    //  if graphics are turned on or off, animation has to be started or stopped
    //  graphics are controlled by VOBJ_TYPE_OLE
    BOOL bGraphicsChanged = ( pOptions->GetObjMode(VOBJ_TYPE_OLE) !=
                                   rOpt.GetObjMode(VOBJ_TYPE_OLE) );

    *pOptions = rOpt;
    DBG_ASSERT( pView, "No View" );

    if( pView )
    {
        pView->ViewOptionsHasChanged( bHScrollChanged, bGraphicsChanged );
    }
}

Point ScViewData::GetMousePosPixel()
{
    DBG_ASSERT( pView, "GetMousePosPixel() ohne View" );
    return pView->GetMousePosPixel();
}

void ScViewData::UpdateInputHandler( BOOL bForce )
{
    if (pViewShell)
        pViewShell->UpdateInputHandler( bForce );
}

BOOL ScViewData::IsOle()
{
    return pDocShell && pDocShell->IsOle();
}

BOOL ScViewData::UpdateFixX( USHORT nTab )              // TRUE = Wert geaendert
{
    if (nTab>MAXTAB)        // Default
        nTab=nTabNo;        // akuelle Tabelle

    if (!pView || pTabData[nTab]->eHSplitMode != SC_SPLIT_FIX)
        return FALSE;

    USHORT nFix = pTabData[nTab]->nFixPosX;
    ScDocument* pDoc = GetDocument();
    long nNewPos = 0;
    for (USHORT nX=pTabData[nTab]->nPosX[SC_SPLIT_LEFT]; nX<nFix; nX++)
    {
        USHORT nTSize = pDoc->GetColWidth( nX, nTab );
        if (nTSize)
        {
            long nPix = (long) ( nTSize * nPPTX );
            nNewPos += nPix;
        }
    }
    nNewPos += pView->GetGridOffset().X();
    if (nNewPos != pTabData[nTab]->nHSplitPos)
    {
        pTabData[nTab]->nHSplitPos = nNewPos;
        if (nTab == nTabNo)
            RecalcPixPos();                 //! sollte nicht noetig sein !!!
        return TRUE;
    }

    return FALSE;
}

BOOL ScViewData::UpdateFixY( USHORT nTab )              // TRUE = Wert geaendert
{
    if (nTab>MAXTAB)        // Default
        nTab=nTabNo;        // akuelle Tabelle

    if (!pView || pTabData[nTab]->eVSplitMode != SC_SPLIT_FIX)
        return FALSE;

    USHORT nFix = pTabData[nTab]->nFixPosY;
    ScDocument* pDoc = GetDocument();
    long nNewPos = 0;
    for (USHORT nY=pTabData[nTab]->nPosY[SC_SPLIT_TOP]; nY<nFix; nY++)
    {
        USHORT nTSize = pDoc->FastGetRowHeight( nY, nTab );
        if (nTSize)
        {
            long nPix = (long) ( nTSize * nPPTY );
            nNewPos += nPix;
        }
    }
    nNewPos += pView->GetGridOffset().Y();
    if (nNewPos != pTabData[nTab]->nVSplitPos)
    {
        pTabData[nTab]->nVSplitPos = nNewPos;
        if (nTab == nTabNo)
            RecalcPixPos();                 //! sollte nicht noetig sein !!!
        return TRUE;
    }

    return FALSE;
}

void ScViewData::UpdateOutlinerFlags( Outliner& rOutl ) const
{
    BOOL bOnlineSpell = GetDocument()->GetDocOptions().IsAutoSpell();
    BOOL bHideSpell = GetOptions().IsHideAutoSpell();

    ULONG nCntrl = rOutl.GetControlWord();
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl |= EE_CNTRL_MARKFIELDS;
    nCntrl |= EE_CNTRL_AUTOCORRECT;
    if( bHideSpell )
        nCntrl |= EE_CNTRL_NOREDLINES;
    else
        nCntrl &= ~EE_CNTRL_NOREDLINES;
    if( bOnlineSpell )
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    rOutl.SetControlWord(nCntrl);

    rOutl.SetCalcFieldValueHdl( LINK( SC_MOD(), ScModule, CalcFieldValueHdl ) );

    //  #66115# Der Speller muss auch gesetzt werden, wenn Online-Spelling aus ist,
    //  weil die EditEngine sich vom Speller die Sprache fuer AutoKorrektur holt

    //! no way to set the outliner's spelling language?
    //  (may not bee needed - default language is always used)

    rOutl.SetSpeller( LinguMgr::GetSpellChecker() );
}

ScAddress ScViewData::GetCurPos() const
{
    return ScAddress( GetCurX(), GetCurY(), GetTabNo() );
}




