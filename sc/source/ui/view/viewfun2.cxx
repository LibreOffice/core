/*************************************************************************
 *
 *  $RCSfile: viewfun2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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
#define _SVSTDARR_STRINGS
#include <svx/boxitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/srchitem.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/stritem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/svstdarr.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>

#include "viewfunc.hxx"

#include "sc.hrc"
#include "globstr.hrc"

#include "arealink.hxx"
#include "attrib.hxx"
#include "autoform.hxx"
#include "cell.hxx"                 // EnterAutoSum
#include "compiler.hxx"
#include "docfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "global.hxx"
#include "patattr.hxx"
#include "printfun.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "refundo.hxx"
#include "tablink.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "undotab.hxx"


// STATIC DATA ---------------------------------------------------------------


//----------------------------------------------------------------------------

BOOL ScViewFunc::AdjustBlockHeight( BOOL bPaint, ScMarkData* pMarkData )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if (!pMarkData)
        pMarkData = &GetViewData()->GetMarkData();

    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT* pRanges = new USHORT[MAXROW+1];
    USHORT nRangeCnt = pMarkData->GetMarkRowRanges( pRanges );
    if (!nRangeCnt)
    {
        pRanges[0] = pRanges[1] = GetViewData()->GetCurY();
        nRangeCnt = 1;
    }

    VirtualDevice aVirtDev;
    BOOL bAnyChanged = FALSE;
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        if (pMarkData->GetTableSelect(nTab))
        {
            USHORT* pOneRange = pRanges;
            BOOL bChanged = FALSE;
            USHORT nPaintY;
            for (USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
            {
                USHORT nStartNo = *(pOneRange++);
                USHORT nEndNo = *(pOneRange++);
                if (pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, 0, &aVirtDev,
                                            GetViewData()->GetPPTX(),
                                            GetViewData()->GetPPTY(),
                                            GetViewData()->GetZoomX(),
                                            GetViewData()->GetZoomY(),
                                            FALSE ))
                {
                    if (!bChanged)
                        nPaintY = nStartNo;
                    bAnyChanged = bChanged = TRUE;
                }
            }
            if ( bPaint && bChanged )
                pDocSh->PostPaint( 0, nPaintY, nTab, MAXCOL, MAXROW, nTab,
                                                    PAINT_GRID | PAINT_LEFT );
        }
    }
    delete[] pRanges;

    if ( bPaint && bAnyChanged )
        pDocSh->UpdateOle(GetViewData());

    return bAnyChanged;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::AdjustRowHeight( USHORT nStartRow, USHORT nEndRow, BOOL bPaint )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();
    double nPPTX = GetViewData()->GetPPTX();
    double nPPTY = GetViewData()->GetPPTY();
    USHORT nOldPixel;
    if (nStartRow == nEndRow)
        nOldPixel = (USHORT) (pDoc->GetRowHeight(nStartRow,nTab) * nPPTY);

    VirtualDevice aVirtDev;
    BOOL bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, &aVirtDev,
                                            nPPTX, nPPTY,
                                            GetViewData()->GetZoomX(),
                                            GetViewData()->GetZoomY(),
                                            FALSE );

    if (bChanged && ( nStartRow == nEndRow ))
    {
        USHORT nNewPixel = (USHORT) (pDoc->GetRowHeight(nStartRow,nTab) * nPPTY);
        if ( nNewPixel == nOldPixel )
            bChanged = FALSE;
    }

    if ( bPaint && bChanged )
        GetViewData()->GetDocShell()->PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
                                            PAINT_GRID | PAINT_LEFT );

    return bChanged;
}


//----------------------------------------------------------------------------

enum ScAutoSum
{
    ScAutoSumNone = 0,
    ScAutoSumData,
    ScAutoSumSum
};


ScAutoSum lcl_IsAutoSumData( ScDocument* pDoc, USHORT nCol, USHORT nRow,
        USHORT nTab, ScDirection eDir, USHORT& nExtend )
{
    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if ( pCell && pCell->HasValueData() )
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            ScTokenArray* pCode = ((ScFormulaCell*)pCell)->GetCode();
            if ( pCode && pCode->GetOuterFuncOpCode() == ocSum )
            {
                if ( pCode->GetAdjacentExtendOfOuterFuncRefs( nExtend,
                        ScAddress( nCol, nRow, nTab ), eDir ) )
                    return ScAutoSumSum;
            }
        }
        return ScAutoSumData;
    }
    return ScAutoSumNone;
}


//----------------------------------------------------------------------------

#define SC_AUTOSUM_MAXCOUNT     20

ScAutoSum lcl_SeekAutoSumData( ScDocument* pDoc, USHORT& nCol, USHORT& nRow,
        USHORT nTab, ScDirection eDir, USHORT& nExtend )
{
    USHORT nCount = 0;
    while (nCount < SC_AUTOSUM_MAXCOUNT)
    {
        if ( eDir == DIR_TOP )
        {
            if (nRow)
                --nRow;
            else
                return ScAutoSumNone;
        }
        else
        {
            if (nCol)
                --nCol;
            else
                return ScAutoSumNone;
        }
        ScAutoSum eSum;
        if ( (eSum = lcl_IsAutoSumData(
                pDoc, nCol, nRow, nTab, eDir, nExtend )) != ScAutoSumNone )
            return eSum;
        ++nCount;
    }
    return ScAutoSumNone;
}

#undef SC_AUTOSUM_MAXCOUNT


//----------------------------------------------------------------------------

BOOL ScViewFunc::GetAutoSumArea( ScRangeList& rRangeList )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();

    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();

    USHORT nStartCol = nCol;
    USHORT nStartRow = nRow;
    USHORT nEndCol   = nCol;
    USHORT nEndRow   = nRow;
    USHORT nSeekCol  = nCol;
    USHORT nSeekRow  = nRow;
    USHORT nExtend;     // wird per Reference gueltig bei ScAutoSumSum

    BOOL bCol = FALSE;
    BOOL bRow = FALSE;

    ScAutoSum eSum;
    if ( nRow
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_TOP, nExtend )) == ScAutoSumData )
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_LEFT, nExtend )) == ScAutoSumData )
        )
    {
        bRow = TRUE;
        nSeekRow = nRow - 1;
    }
    else if ( nCol && (eSum = lcl_IsAutoSumData( pDoc, nCol-1, nRow, nTab,
            DIR_LEFT, nExtend )) == ScAutoSumData )
    {
        bCol = TRUE;
        nSeekCol = nCol - 1;
    }
    else if (eSum = lcl_SeekAutoSumData( pDoc, nCol, nSeekRow, nTab, DIR_TOP, nExtend ))
        bRow = TRUE;
    else if (eSum = lcl_SeekAutoSumData( pDoc, nSeekCol, nRow, nTab, DIR_LEFT, nExtend ))
        bCol = TRUE;

    if ( bCol || bRow )
    {
        if ( bRow )
        {
            nStartRow = nSeekRow;       // nSeekRow evtl. per Reference angepasst
            if ( eSum == ScAutoSumSum )
                nEndRow = nStartRow;        // nur Summen summieren
            else
                nEndRow = nRow - 1;     // Datenbereich evtl. nach unten erweitern
        }
        else
        {
            nStartCol = nSeekCol;       // nSeekCol evtl. per Reference angepasst
            if ( eSum == ScAutoSumSum )
                nEndCol = nStartCol;        // nur Summen summieren
            else
                nEndCol = nCol - 1;     // Datenbereich evtl. nach rechts erweitern
        }
        BOOL bContinue = FALSE;
        do
        {
            if ( eSum == ScAutoSumData )
            {
                if ( bRow )
                {
                    while ( nStartRow && lcl_IsAutoSumData( pDoc, nCol,
                            nStartRow-1, nTab, DIR_TOP, nExtend ) == eSum )
                        --nStartRow;
                }
                else
                {
                    while ( nStartCol && lcl_IsAutoSumData( pDoc, nStartCol-1,
                            nRow, nTab, DIR_LEFT, nExtend ) == eSum )
                        --nStartCol;
                }
            }
            rRangeList.Append(
                ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab ) );
            if ( eSum == ScAutoSumSum )
            {
                if ( bRow )
                {
                    USHORT nTmp = nEndRow = nExtend;
                    ScAutoSum eSkip;
                    while ( (eSkip = lcl_IsAutoSumData( pDoc, nCol,
                            nEndRow, nTab, DIR_TOP, nExtend )) == ScAutoSumData
                            && nEndRow )
                        --nEndRow;
                    if ( eSkip == ScAutoSumSum && nEndRow < nTmp )
                    {
                        bContinue = TRUE;
                        nStartRow = nEndRow;
                    }
                    else
                        bContinue = FALSE;
                }
                else
                {
                    USHORT nTmp = nEndCol = nExtend;
                    ScAutoSum eSkip;
                    while ( (eSkip = lcl_IsAutoSumData( pDoc, nEndCol,
                            nRow, nTab, DIR_LEFT, nExtend )) == ScAutoSumData
                            && nEndCol )
                        --nEndCol;
                    if ( eSkip == ScAutoSumSum && nEndCol < nTmp )
                    {
                        bContinue = TRUE;
                        nStartCol = nEndCol;
                    }
                    else
                        bContinue = FALSE;
                }
            }
        } while ( bContinue );
        return TRUE;
    }
    return FALSE;
}


//----------------------------------------------------------------------------

void ScViewFunc::EnterAutoSum(const ScRangeList& rRangeList)        // Block mit Summen fuellen
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    String aRef;
    rRangeList.Format( aRef, SCA_VALID, pDoc );

    String aFormula = '=';
    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    ScFuncDesc* pDesc = pFuncMgr->Get( SC_OPCODE_SUM );
    if ( pDesc && pDesc->pFuncName )
    {
        aFormula += *pDesc->pFuncName;
        aFormula += '(';
        aFormula += aRef;
        aFormula += ')';
    }

    EnterBlock( aFormula, NULL );
}


//----------------------------------------------------------------------------

void ScViewFunc::EnterBlock( const String& rString, const EditTextObject* pData )
{
    //  Mehrfachselektion vorher abfragen...

    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rMark.IsMultiMarked() )
    {
        rMark.MarkToSimple();
        if ( rMark.IsMultiMarked() )
        {       // "Einfuegen auf Mehrfachselektion nicht moeglich"
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);

            //  insert into single cell
            if ( pData )
                EnterData( nCol, nRow, nTab, pData );
            else
                EnterData( nCol, nRow, nTab, rString );
            return;
        }
    }

    ScDocument* pDoc = GetViewData()->GetDocument();
    String aNewStr = rString;
    if ( pData )
    {
        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool() );
        aEngine.SetText(*pData);

        ScEditAttrTester aTester( &aEngine );
        if (!aTester.NeedsObject())
        {
            aNewStr = aEngine.GetText();
            pData = NULL;
        }
    }

    //  Einfuegen per PasteFromClip

    WaitObject aWait( GetFrameWin() );

    ScAddress aPos( nCol, nRow, nTab );

    ScDocument* pInsDoc = new ScDocument( SCDOCMODE_CLIP );
    pInsDoc->ResetClip( pDoc, nTab );

    if (aNewStr.GetChar(0) == '=')                      // Formel ?
    {
        //  SetString geht nicht, weil in Clipboard-Dokumenten nicht kompiliert wird!
        ScFormulaCell* pFCell = new ScFormulaCell( pDoc, aPos, aNewStr );
        pInsDoc->PutCell( nCol, nRow, nTab, pFCell );
    }
    else if ( pData )
        pInsDoc->PutCell( nCol, nRow, nTab, new ScEditCell( pData, pDoc, NULL ) );
    else
        pInsDoc->SetString( nCol, nRow, nTab, aNewStr );

    pInsDoc->SetClipArea( ScRange(aPos) );
    // auf Block einfuegen, mit Undo etc.
    if ( PasteFromClip( IDF_CONTENTS, pInsDoc, PASTE_NOFUNC, FALSE, FALSE,
            FALSE, INS_NONE, IDF_ATTRIB ) )
    {
        const SfxUInt32Item* pItem = (SfxUInt32Item*) pInsDoc->GetAttr(
            nCol, nRow, nTab, ATTR_VALUE_FORMAT );
        if ( pItem )
        {   // Numberformat setzen wenn inkompatibel
            // MarkData wurde bereits in PasteFromClip MarkToSimple'ed
            ScMarkData& rMark = GetViewData()->GetMarkData();
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ScPatternAttr* pPattern = new ScPatternAttr( pDoc->GetPool() );
            pPattern->GetItemSet().Put( *pItem );
            short nNewType = pDoc->GetFormatTable()->GetType( pItem->GetValue() );
            pDoc->ApplyPatternIfNumberformatIncompatible( aRange, rMark,
                *pPattern, nNewType );
            delete pPattern;
        }
    }

    delete pInsDoc;
}


//----------------------------------------------------------------------------

void ScViewFunc::PaintWidthHeight( BOOL bColumns, USHORT nStart, USHORT nEnd )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocument* pDoc = GetViewData()->GetDocument();

    USHORT nParts = PAINT_GRID;
    USHORT nStartCol = 0;
    USHORT nStartRow = 0;
    USHORT nEndCol = MAXCOL;            // fuer Test auf Merge
    USHORT nEndRow = MAXROW;
    if ( bColumns )
    {
        nParts |= PAINT_TOP;
        nStartCol = nStart;
        nEndCol = nEnd;
    }
    else
    {
        nParts |= PAINT_LEFT;
        nStartRow = nStart;
        nEndRow = nEnd;
    }
    if (pDoc->HasAttrib( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        nStartCol = 0;
        nStartRow = 0;
    }
    GetViewData()->GetDocShell()->PostPaint( nStartCol,nStartRow,nTab, MAXCOL,MAXROW,nTab, nParts );
}


//----------------------------------------------------------------------------
//  manueller Seitenumbruch

void ScViewFunc::InsertPageBreak( BOOL bColumn, BOOL bRecord, const ScAddress* pPos,
                                    BOOL bSetModified )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        InsertPageBreak( bColumn, aCursor, bRecord, bSetModified, FALSE );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( TRUE );    // fuer PageBreak-Modus
}


//----------------------------------------------------------------------------

void ScViewFunc::DeletePageBreak( BOOL bColumn, BOOL bRecord, const ScAddress* pPos,
                                    BOOL bSetModified )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        RemovePageBreak( bColumn, aCursor, bRecord, bSetModified, FALSE );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( TRUE );    // fuer PageBreak-Modus
}

//----------------------------------------------------------------------------

void ScViewFunc::RemoveManualBreaks()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();

    ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
    pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
    pDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, FALSE, pUndoDoc );
    pDocSh->GetUndoManager()->AddUndoAction(
                                new ScUndoRemoveBreaks( pDocSh, nTab, pUndoDoc ) );

    pDoc->RemoveManualBreaks(nTab);
    pDoc->UpdatePageBreaks(nTab);

    UpdatePageBreakData( TRUE );
    pDocSh->SetDocumentModified();
    pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
}

//----------------------------------------------------------------------------

void ScViewFunc::SetPrintZoom(USHORT nScale, USHORT nPages)
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    USHORT nTab = GetViewData()->GetTabNo();
    pDocSh->SetPrintZoom( nTab, nScale, nPages );
}

void ScViewFunc::AdjustPrintZoom()
{
    ScRange aRange;
    if ( !GetViewData()->GetSimpleArea( aRange, TRUE ) )
        GetViewData()->GetMarkData().GetMultiMarkArea( aRange );
    GetViewData()->GetDocShell()->AdjustPrintZoom( aRange );
}

//----------------------------------------------------------------------------

void ScViewFunc::SetPrintRanges( const String* pPrint,
                                const String* pRepCol, const String* pRepRow,
                                BOOL bAddPrint )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();
    USHORT      nCurTab = GetViewData()->GetTabNo();
    ScRange     aRange( 0,0,nCurTab );  // falls Tab im String nicht angegeben

    ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();
    USHORT nAdd = 0;
    if ( bAddPrint )
        nAdd = pDoc->GetPrintRangeCount( nCurTab );

    //  Druckbereiche

    if ( pPrint )
    {
        if ( !pPrint->Len() )
            pDoc->SetPrintRangeCount( nCurTab, nAdd );      // loeschen bzw. unveraendert
        else
        {
            USHORT nTCount = pPrint->GetTokenCount();
            pDoc->SetPrintRangeCount( nCurTab, nTCount + nAdd );
            for (USHORT i=0; i<nTCount; i++)
            {
                String aToken = pPrint->GetToken(i);
                if ( aRange.ParseAny( aToken, pDoc ) & SCA_VALID )
                    pDoc->SetPrintRange( nCurTab, i + nAdd, aRange );
            }
        }
    }
    else        //  sonst Selektion (Druckbereich wird immer gesetzt)
    {
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if ( GetViewData()->GetSimpleArea( aRange, TRUE ) )
        {
            pDoc->SetPrintRangeCount( nCurTab, nAdd + 1 );
            pDoc->SetPrintRange( nCurTab, nAdd, aRange );
        }
        else if ( rMark.IsMultiMarked() )
        {
            ScRangeListRef aList( new ScRangeList );
            rMark.FillRangeListWithMarks( aList, FALSE );
            USHORT nCnt = (USHORT) aList->Count();
            if ( nCnt )
            {
                pDoc->SetPrintRangeCount( nCurTab, nCnt + nAdd );
                ScRangePtr pR;
                USHORT i;
                for ( pR = aList->First(), i=0; i < nCnt;
                      pR = aList->Next(), i++ )
                {
                    pDoc->SetPrintRange( nCurTab, i + nAdd, *pR );
                }
            }
        }
    }

    //  Wiederholungsspalten

    if ( pRepCol )
    {
        if ( !pRepCol->Len() )
            pDoc->SetRepeatColRange( nCurTab, NULL );
        else
            if ( aRange.ParseAny( *pRepCol, pDoc ) & SCA_VALID )
                pDoc->SetRepeatColRange( nCurTab, &aRange );
    }

    //  Wiederholungszeilen

    if ( pRepRow )
    {
        if ( !pRepRow->Len() )
            pDoc->SetRepeatRowRange( nCurTab, NULL );
        else
            if ( aRange.ParseAny( *pRepRow, pDoc ) & SCA_VALID )
                pDoc->SetRepeatRowRange( nCurTab, &aRange );
    }

    //  Umbrueche und Undo

    ScPrintRangeSaver* pNewRanges = pDoc->CreatePrintRangeSaver();
    pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoPrintRange( pDocSh, nCurTab, pOldRanges, pNewRanges ) );

    ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nCurTab ).UpdatePages();

    SfxBindings& rBindings = SFX_BINDINGS();
    rBindings.Invalidate( SID_DELETE_PRINTAREA );

    pDocSh->SetDocumentModified();
}

//----------------------------------------------------------------------------
//  Zellen zusammenfassen

BOOL ScViewFunc::TestMergeCells()           // Vorab-Test (fuer Menue)
{
    ScMarkData& rMark = GetViewData()->GetMarkData();
    rMark.MarkToSimple();
    return rMark.IsMarked() && !rMark.IsMultiMarked();
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::MergeCells( BOOL bApi, BOOL& rDoContents, BOOL bRecord )
{
    //  Editable- und Verschachtelungs-Abfrage muss vorneweg sein (auch in DocFunc),
    //  damit dann nicht die Inhalte-QueryBox kommt
    if (!SelectionEditable())
    {
        ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    ScMarkData& rMark = GetViewData()->GetMarkData();
    rMark.MarkToSimple();
    if (!rMark.IsMarked())
    {
        ErrorMessage(STR_NOMULTISELECT);
        return FALSE;
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

    ScRange aMarkRange;
    rMark.GetMarkArea( aMarkRange );
    USHORT nStartCol = aMarkRange.aStart.Col();
    USHORT nStartRow = aMarkRange.aStart.Row();
    USHORT nStartTab = aMarkRange.aStart.Tab();
    USHORT nEndCol = aMarkRange.aEnd.Col();
    USHORT nEndRow = aMarkRange.aEnd.Row();
    USHORT nEndTab = aMarkRange.aEnd.Tab();
    if ( nStartCol == nEndCol && nStartRow == nEndRow )
    {
        // nichts zu tun
        return TRUE;
    }

    if ( pDoc->HasAttrib( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {       // "Zusammenfassen nicht verschachteln !"
        ErrorMessage(STR_MSSG_MERGECELLS_0);
        return FALSE;
    }

    BOOL bOk = TRUE;
    BOOL bNeedContents = FALSE;

    if ( !pDoc->IsBlockEmpty( nStartTab, nStartCol,nStartRow+1, nStartCol,nEndRow ) ||
         !pDoc->IsBlockEmpty( nStartTab, nStartCol+1,nStartRow, nEndCol,nEndRow ) )
    {
        bNeedContents = TRUE;
        if (!bApi)
        {
            MessBox aBox( GetViewData()->GetDialogParent(),
                            WinBits(WB_YES_NO_CANCEL | WB_DEF_NO),
                            ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                            ScGlobal::GetRscString( STR_MERGE_NOTEMPTY ) );
            USHORT nRetVal = aBox.Execute();

            if ( nRetVal == RET_YES )
                rDoContents = TRUE;
            else if ( nRetVal == RET_CANCEL )
                bOk = FALSE;
        }
    }

    if (bOk)
    {
        HideCursor();
        bOk = pDocSh->GetDocFunc().MergeCells( aMarkRange, rDoContents, bRecord, bApi );
        ShowCursor();

        if (bOk)
        {
            SetCursor( nStartCol, nStartRow );
            //DoneBlockMode( FALSE);
            Unmark();

            pDocSh->UpdateOle(GetViewData());
            UpdateInputLine();
        }
    }

    return bOk;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::TestRemoveMerge()
{
    BOOL bMerged = FALSE;
    ScRange aRange;
    if (GetViewData()->GetSimpleArea( aRange ))
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        if ( pDoc->HasAttrib( aRange, HASATTR_MERGED ) )
            bMerged = TRUE;
    }
    return bMerged;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::RemoveMerge( BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea( aRange ))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        HideCursor();
        BOOL bOk = pDocSh->GetDocFunc().UnmergeCells( aRange, bRecord, FALSE );
        ShowCursor();
        if (bOk)
            pDocSh->UpdateOle(GetViewData());
    }
    return TRUE;        //! bOk ??
}

//----------------------------------------------------------------------------

void ScViewFunc::FillSimple( FillDir eDir, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSuccess = pDocSh->GetDocFunc().FillSimple( aRange, &rMark, eDir, bRecord, FALSE );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            UpdateScrollBars();
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//----------------------------------------------------------------------------

void ScViewFunc::FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                             double fStart, double fStep, double fMax, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSuccess = pDocSh->GetDocFunc().
                        FillSeries( aRange, &rMark, eDir, eCmd, eDateCmd,
                                    fStart, fStep, fMax, bRecord, FALSE );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            UpdateScrollBars();
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//----------------------------------------------------------------------------

void ScViewFunc::FillAuto( FillDir eDir, USHORT nStartCol, USHORT nStartRow,
                            USHORT nEndCol, USHORT nEndRow, USHORT nCount, BOOL bRecord )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScRange aRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab );
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    const ScMarkData& rMark = GetViewData()->GetMarkData();
    BOOL bSuccess = pDocSh->GetDocFunc().
                    FillAuto( aRange, &rMark, eDir, nCount, bRecord, FALSE );
    if (bSuccess)
    {
        MarkRange( aRange, FALSE );         // aRange ist in FillAuto veraendert worden
        pDocSh->UpdateOle(GetViewData());
        UpdateScrollBars();
    }
}

//----------------------------------------------------------------------------

void ScViewFunc::FillTab( USHORT nFlags, USHORT nFunction, BOOL bSkipEmpty, BOOL bAsLink )
{
    if (!SelectionEditable())           //! vordere Tabelle darf geschuetzt sein !!!
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    USHORT nTab = GetViewData()->GetTabNo();

    ScRange aMarkRange;
    rMark.MarkToSimple();
    BOOL bMulti = rMark.IsMultiMarked();
    if (bMulti)
        rMark.GetMultiMarkArea( aMarkRange );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkRange );
    else
        aMarkRange = ScRange( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    ScDocument* pUndoDoc = NULL;
//  if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab );
//      pUndoDoc->SelectTable( nTab, TRUE );        // nur fuer Markierung

        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT i=0; i<nTabCount; i++)
            if (i != nTab && rMark.GetTableSelect(i))
            {
                pUndoDoc->AddUndoTab( i, i );
                aMarkRange.aStart.SetTab( i );
                aMarkRange.aEnd.SetTab( i );
                pDoc->CopyToDocument( aMarkRange, IDF_ALL, bMulti, pUndoDoc );
//              pUndoDoc->SelectTable( i, TRUE );
            }
    }

    if (bMulti)
        pDoc->FillTabMarked( nTab, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    else
    {
        aMarkRange.aStart.SetTab( nTab );
        aMarkRange.aEnd.SetTab( nTab );
        pDoc->FillTab( aMarkRange, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    }

//  if ( bRecord )
    {   //! fuer ChangeTrack erst zum Schluss
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoFillTable( pDocSh, rMark,
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nTab,
                                aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab,
                                pUndoDoc, bMulti, nTab, nFlags, nFunction, bSkipEmpty, bAsLink ) );
    }

    pDocSh->PostPaintGridAll();
    pDocSh->PostDataChanged();
}


//----------------------------------------------------------------------------
//  AutoFormat

ScAutoFormatData* ScViewFunc::CreateAutoFormatData()
{
    ScAutoFormatData* pData = NULL;
    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;
    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
    {
        if ( nEndCol-nStartCol >= 3 && nEndRow-nStartRow >= 3 )
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            pData = new ScAutoFormatData;
            pDoc->GetAutoFormatData( nStartTab, nStartCol,nStartRow,nEndCol,nEndRow, *pData );
        }
    }
    return pData;
}


//----------------------------------------------------------------------------

void ScViewFunc::AutoFormat( USHORT nFormatNo, BOOL bRecord )
{
#if 1

    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();

        BOOL bSuccess = pDocSh->GetDocFunc().AutoFormat( aRange, &rMark, nFormatNo, bRecord, FALSE );
        if (bSuccess)
            pDocSh->UpdateOle(GetViewData());
    }
    else
        ErrorMessage(STR_NOMULTISELECT);

#else

    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;

    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSize = (*ScGlobal::GetAutoFormat())[nFormatNo]->GetIncludeWidthHeight();

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab, bSize, bSize );
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    IDF_ATTRIB, FALSE, pUndoDoc );
            if (bSize)
            {
                pDoc->CopyToDocument( nStartCol,0,nStartTab, nEndCol,MAXROW,nEndTab,
                                                            IDF_NONE, FALSE, pUndoDoc );
                pDoc->CopyToDocument( 0,nStartRow,nStartTab, MAXCOL,nEndRow,nEndTab,
                                                            IDF_NONE, FALSE, pUndoDoc );
            }
            pDoc->BeginDrawUndo();
        }

        GetFrameWin()->EnterWait();
        pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, rMark );
        GetFrameWin()->LeaveWait();

        if (bSize)
        {
            SetMarkedWidthOrHeight( TRUE, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, FALSE, FALSE );
            SetMarkedWidthOrHeight( FALSE, SC_SIZE_VISOPT, 0, FALSE, FALSE );
            pDocSh->PostPaint( 0,0,nStartTab, MAXCOL,MAXROW,nStartTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP );
        }
        else
        {
            BOOL bAdj = AdjustBlockHeight( FALSE );
            if (bAdj)
                pDocSh->PostPaint( 0,nStartRow,nStartTab, MAXCOL,MAXROW,nStartTab,
                                    PAINT_GRID | PAINT_LEFT );
            else
                pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                                    nEndCol, nEndRow, nEndTab, PAINT_GRID );
        }

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoAutoFormat( pDocSh,
                        ScRange(nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab),
                        pUndoDoc, rMark, bSize, nFormatNo ) );
        }

        pDocSh->UpdateOle(GetViewData());
        pDocSh->SetDocumentModified();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);

#endif
}


//----------------------------------------------------------------------------
//  Suchen & Ersetzen

void ScViewFunc::SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        BOOL bAddUndo, BOOL bIsApi )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    USHORT nCol = GetViewData()->GetCurX();
    USHORT nRow = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();
    BOOL bAttrib = pSearchItem->GetPattern();
    USHORT nCommand = pSearchItem->GetCommand();
    BOOL bAllTables = pSearchItem->IsAllTables();
    BOOL* pOldSelectedTables = NULL;
    USHORT nOldSelectedCount = 0;
    USHORT nOldTab = nTab;
    USHORT nLastTab = pDoc->GetTableCount() - 1;
    USHORT nStartTab, nEndTab;
    if ( bAllTables )
    {
        nStartTab = 0;
        nEndTab = nLastTab;
        pOldSelectedTables = new BOOL [ nEndTab + 1 ];
        for ( USHORT j = 0; j <= nEndTab; j++ )
        {
            pOldSelectedTables[j] = rMark.GetTableSelect( j );
            if ( pOldSelectedTables[j] )
                ++nOldSelectedCount;
        }
    }
    else
    {   //! mindestens eine ist immer selektiert
        nStartTab = nEndTab = rMark.GetFirstSelected();
        for ( USHORT j = nStartTab + 1; j <= nLastTab; j++ )
        {
            if ( rMark.GetTableSelect( j ) )
                nEndTab = j;
        }
    }

    if (   nCommand == SVX_SEARCHCMD_REPLACE
        || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
    {
        for ( USHORT j = nStartTab; j <= nEndTab; j++ )
        {
            if ( (bAllTables || rMark.GetTableSelect( j )) &&
                    pDoc->IsTabProtected( j ) )
            {
                if ( pOldSelectedTables )
                    delete [] pOldSelectedTables;
                ErrorMessage(STR_PROTECTIONERR);
                return;
            }
        }
    }

    if (   nCommand == SVX_SEARCHCMD_FIND
        || nCommand == SVX_SEARCHCMD_FIND_ALL)
        bAddUndo = FALSE;

    //!     bAttrib bei Undo beruecksichtigen !!!

    ScDocument* pUndoDoc = NULL;
    ScMarkData* pUndoMark = NULL;
    String aUndoStr;
    if (bAddUndo)
    {
        pUndoMark = new ScMarkData( rMark );                // Markierung wird veraendert
        if ( nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
        }
    }

    if ( bAllTables )
    {   //! alles selektieren, erst nachdem pUndoMark erzeugt wurde
        for ( USHORT j = nStartTab; j <= nEndTab; j++ )
        {
            rMark.SelectTable( j, TRUE );
        }
    }

    DoneBlockMode(TRUE);                // Markierung nicht loeschen!
    InitOwnBlockMode();

    //  wenn vom Anfang an gesucht wird, nicht nochmal fragen ob vom Anfang gesucht werden soll
    BOOL bFirst = TRUE;
    if ( nCol == 0 && nRow == 0 && nTab == nStartTab && !pSearchItem->GetBackward()  )
        bFirst = FALSE;

    BOOL bFound = FALSE;
    while (TRUE)
    {
        GetFrameWin()->EnterWait();
        if (pDoc->SearchAndReplace( *pSearchItem, nCol, nRow, nTab, rMark, aUndoStr, pUndoDoc ) )
        {
            bFound = TRUE;
            bFirst = TRUE;
            if (bAddUndo)
            {
                GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                    new ScUndoReplace( GetViewData()->GetDocShell(), *pUndoMark,
                                        nCol, nRow, nTab,
                                        aUndoStr, pUndoDoc, pSearchItem ) );
                pUndoDoc = NULL;
            }

            break;                  // Abbruch while True
        }
        else if ( bFirst && (nCommand == SVX_SEARCHCMD_FIND ||
                nCommand == SVX_SEARCHCMD_REPLACE) )
        {
            bFirst = FALSE;
            USHORT nRetVal;
            GetFrameWin()->LeaveWait();
            if ( bIsApi )
                nRetVal = RET_NO;
            else
            {
                //  Suchen-Dialog als Parent, wenn vorhanden
                Window* pParent = GetParentOrChild(SID_SEARCH_DLG);
                USHORT nStrId;
                if ( pSearchItem->GetBackward() )
                {
                    if ( nStartTab == nEndTab )
                        nStrId = STR_MSSG_SEARCHANDREPLACE_1;
                    else
                        nStrId = STR_MSSG_SEARCHANDREPLACE_4;
                }
                else
                {
                    if ( nStartTab == nEndTab )
                        nStrId = STR_MSSG_SEARCHANDREPLACE_2;
                    else
                        nStrId = STR_MSSG_SEARCHANDREPLACE_5;
                }
                MessBox aBox( pParent, WinBits(WB_YES_NO | WB_DEF_YES),
                                ScGlobal::GetRscString( STR_MSSG_SEARCHANDREPLACE_3 ),
                                ScGlobal::GetRscString( nStrId ) );
                nRetVal = aBox.Execute();
            }

            if ( nRetVal == RET_YES )
            {
                ScDocument::GetSearchAndReplaceStart( *pSearchItem, nCol, nRow );
                if (pSearchItem->GetBackward())
                    nTab = nEndTab;
                else
                    nTab = nStartTab;
            }
            else
            {
                break;                  // Abbruch while True
            }
        }
        else                            // nichts gefunden
        {
            if ( nCommand == SVX_SEARCHCMD_FIND_ALL || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
            {
                pDocSh->PostPaintGridAll();                             // Markierung
            }

            GetFrameWin()->LeaveWait();
            if (!bIsApi)
            {
                //  Suchen-Dialog als Parent, wenn vorhanden
                Window* pParent = GetParentOrChild(SID_SEARCH_DLG);
                // "nichts gefunden"
                InfoBox aBox( pParent, ScGlobal::GetRscString( STR_MSSG_SEARCHANDREPLACE_0 ) );
                aBox.Execute();
            }

            break;                      // Abbruch while True
        }
    }                               // of while TRUE

    if ( pOldSelectedTables )
    {   // urspruenglich selektierte Tabellen wiederherstellen
        for ( USHORT j = nStartTab; j <= nEndTab; j++ )
        {
            rMark.SelectTable( j, pOldSelectedTables[j] );
        }
        if ( bFound )
        {   // durch Fundstelle neu selektierte Tabelle bleibt
            rMark.SelectTable( nTab, TRUE );
            // wenn vorher nur eine selektiert war, ist es ein Tausch
            //! wenn nicht, ist jetzt evtl. eine mehr selektiert
            if ( nOldSelectedCount == 1 && nTab != nOldTab )
                rMark.SelectTable( nOldTab, FALSE );
        }
        delete [] pOldSelectedTables;
    }
    if ( bFound )
    {
        if ( nTab != GetViewData()->GetTabNo() )
            SetTabNo( nTab );

        //  wenn nichts markiert ist, DoneBlockMode, damit von hier aus
        //  direkt per Shift-Cursor markiert werden kann:
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            DoneBlockMode(TRUE);

        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
        SetCursor( nCol, nRow, TRUE );

        if (   nCommand == SVX_SEARCHCMD_REPLACE
            || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            if ( nCommand == SVX_SEARCHCMD_REPLACE )
                pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID );
            else
                pDocSh->PostPaintGridAll();
            pDocSh->SetDocumentModified();
        }
        else if ( nCommand == SVX_SEARCHCMD_FIND_ALL )
            pDocSh->PostPaintGridAll();                             // Markierung
        GetFrameWin()->LeaveWait();
    }

    delete pUndoDoc;            // loeschen wenn nicht benutzt
    delete pUndoMark;           // kann immer geloescht werden
}


//----------------------------------------------------------------------------
//  Zielwertsuche

void ScViewFunc::Solve( const ScSolveParam& rParam )
{
    ScDocument* pDoc = GetViewData()->GetDocument();

    USHORT nDestCol = rParam.aRefVariableCell.Col();
    USHORT nDestRow = rParam.aRefVariableCell.Row();
    USHORT nDestTab = rParam.aRefVariableCell.Tab();

    if (!pDoc->IsBlockEditable(nDestTab, nDestCol,nDestRow, nDestCol,nDestRow))
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    if ( pDoc )
    {
        String  aTargetValStr;
        if ( rParam.pStrTargetVal != NULL )
            aTargetValStr = *(rParam.pStrTargetVal);

        String  aMsgStr;
        String  aResStr;
        double  nSolveResult;

        GetFrameWin()->EnterWait();

        BOOL    bExact =
                    pDoc->Solver(
                        rParam.aRefFormulaCell.Col(),
                        rParam.aRefFormulaCell.Row(),
                        rParam.aRefFormulaCell.Tab(),
                        nDestCol, nDestRow, nDestTab,
                        aTargetValStr,
                        nSolveResult );

        GetFrameWin()->LeaveWait();

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        ULONG nFormat = 0;
        const ScPatternAttr* pPattern = pDoc->GetPattern( nDestCol, nDestRow, nDestTab );
        if ( pPattern )
            nFormat = pPattern->GetNumberFormat( pFormatter );
        Color* p;
        pFormatter->GetOutputString( nSolveResult, nFormat, aResStr, &p );

        if ( bExact )
        {
            aMsgStr  = ScGlobal::GetRscString( STR_MSSG_SOLVE_0 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_1 );
            aMsgStr += String( aResStr );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_2 );
        }
        else
        {
            aMsgStr  = ScGlobal::GetRscString( STR_MSSG_SOLVE_3 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_4 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_5 );
            aMsgStr += String( aResStr );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_6 );
        }

        MessBox aBox( GetViewData()->GetDialogParent(),
                        WinBits(WB_YES_NO | WB_DEF_NO),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ), aMsgStr );
        USHORT nRetVal = aBox.Execute();

        if ( RET_YES == nRetVal )
            EnterData( nDestCol, nDestRow, nDestTab, nSolveResult );

        GetViewData()->GetViewShell()->UpdateInputHandler( TRUE );
    }
}


//----------------------------------------------------------------------------
//  Mehrfachoperation

void ScViewFunc::TabOp( const ScTabOpParam& rParam, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        pDocSh->GetDocFunc().TabOp( aRange, &rMark, rParam, bRecord, FALSE );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}


//----------------------------------------------------------------------------

void ScViewFunc::MakeScenario( const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScMarkData& rMark   = GetViewData()->GetMarkData();
    USHORT      nTab    = GetViewData()->GetTabNo();

    USHORT nNewTab = pDocSh->MakeScenario( nTab, rName, rComment, rColor, nFlags, rMark );
    if (nFlags & SC_SCENARIO_COPYALL)
        SetTabNo( nNewTab, TRUE );          // SC_SCENARIO_COPYALL -> sichtbar
    else
    {
        SfxBindings& rBindings = SFX_BINDINGS();
        rBindings.Invalidate( SID_STATUS_DOCPOS );      // Statusbar
        rBindings.Invalidate( SID_TABLES_COUNT );
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Invalidate( FID_TABLE_SHOW );
    }
}


//----------------------------------------------------------------------------

void ScViewFunc::ExtendScenario()
{
    if (!SelectionEditable())
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

        //  Undo: Attribute anwenden

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScPatternAttr aPattern( pDoc->GetPool() );
    aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
    aPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
    ApplySelectionPattern(aPattern);
}


//----------------------------------------------------------------------------

void ScViewFunc::UseScenario( const String& rName )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    USHORT      nTab    = GetViewData()->GetTabNo();

    DoneBlockMode();
    InitOwnBlockMode();
    pDocSh->UseScenario( nTab, rName );
}


//----------------------------------------------------------------------------
//  Tabelle einfuegen

BOOL ScViewFunc::InsertTable( const String& rName, USHORT nTab, BOOL bRecord )
{
    //  Reihenfolge Tabelle/Name ist bei DocFunc umgekehrt
    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        InsertTable( nTab, rName, bRecord, FALSE );
    if (bSuccess)
        SetTabNo( nTab, TRUE );

    return bSuccess;
}

//----------------------------------------------------------------------------
//  Tabellen einfuegen

BOOL ScViewFunc::InsertTables(SvStrings *pNames, USHORT nTab,
                                            USHORT nCount, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();

    SvStrings *pNameList= NULL;

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
    {
        pNameList= new SvStrings;
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage
    }

    BOOL bFlag=FALSE;

    String aValTabName;
    String *pStr;

    for(int i=0;i<nCount;i++)
    {
        if(pNames!=NULL)
        {
            pStr=pNames->GetObject(i);
        }
        else
        {
            aValTabName.Erase();
            pDoc->CreateValidTabName( aValTabName);
            pStr=&aValTabName;
        }

        if(pDoc->InsertTab( nTab+i,*pStr))
        {
            bFlag=TRUE;
            pDocSh->Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab+i ) );
        }
        else
        {
            break;
        }

        if(pNameList!=NULL)
            pNameList->Insert(new String(*pStr),pNameList->Count());

    }

    if (bFlag)
    {
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTables( pDocSh, nTab, FALSE, pNameList));

        //  Views updaten:

        SetTabNo( nTab, TRUE );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::AppendTable( const String& rName, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc   = pDocSh->GetDocument();

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage

    if (pDoc->InsertTab( SC_TAB_APPEND, rName ))
    {
        USHORT nTab = pDoc->GetTableCount()-1;
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( pDocSh, nTab, TRUE, rName));
        GetViewData()->InsertTab( nTab );
        SetTabNo( nTab, TRUE );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::DeleteTable( USHORT nTab, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();

    BOOL bSuccess = pDocSh->GetDocFunc().DeleteTable( nTab, bRecord, FALSE );
    if (bSuccess)
    {
        USHORT nNewTab = nTab;
        if ( nNewTab >= pDoc->GetTableCount() )
            --nNewTab;
        SetTabNo( nNewTab, TRUE );
    }
    return bSuccess;
}

BOOL ScViewFunc::DeleteTables(const SvUShorts &TheTabs, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();
    USHORT      nNewTab = TheTabs[0];
    int         i;
    WaitObject aWait( GetFrameWin() );

    BOOL bWasLinked = FALSE;
    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
//      pUndoDoc->InitDrawLayer( pDocSh );
        USHORT nCount = pDoc->GetTableCount();

//      pUndoDoc->InitUndo( pDoc, 0, nCount-1 );        // incl. Ref.

        String aOldName;
        for(i=0;i<TheTabs.Count();i++)
        {
            USHORT nTab = TheTabs[i];
            if (i==0)
                pUndoDoc->InitUndo( pDoc, nTab,nTab, TRUE,TRUE );   // incl. Spalten/Zeilenflags
            else
                pUndoDoc->AddUndoTab( nTab,nTab, TRUE,TRUE );       // incl. Spalten/Zeilenflags

            pDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pUndoDoc );
            pDoc->GetName( nTab, aOldName );
            pUndoDoc->RenameTab( nTab, aOldName, FALSE );
            if (pDoc->IsLinked(nTab))
            {
                bWasLinked = TRUE;
                pUndoDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), pDoc->GetLinkDoc(nTab),
                                    pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                    pDoc->GetLinkTab(nTab) );
            }
            if ( pDoc->IsScenario(nTab) )
            {
                pUndoDoc->SetScenario( nTab, TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
                pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
                BOOL bActive = pDoc->IsActiveScenario( nTab );
                pUndoDoc->SetActiveScenario( nTab, bActive );
            }

            //  Drawing-Layer muss sein Undo selbst in der Hand behalten !!!
            //      pUndoDoc->TransferDrawPage(pDoc, nTab,nTab);
        }

        pUndoDoc->AddUndoTab( 0, nCount-1 );            //  alle Tabs fuer Referenzen

        pDoc->BeginDrawUndo();                          //  DeleteTab erzeugt ein SdrUndoDelPage

        pUndoData = new ScRefUndoData( pDoc );
    }

    BOOL bDelError=FALSE;

    for(i=TheTabs.Count()-1;i>=0;i--)
    {
        if (pDoc->DeleteTab( TheTabs[i], pUndoDoc ))
        {
            bDelError=TRUE;
            pDocSh->Broadcast( ScTablesHint( SC_TAB_DELETED, TheTabs[i] ) );
        }
    }
    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoDeleteTab( GetViewData()->GetDocShell(), TheTabs,
                                            pUndoDoc, pUndoData ));
    }


    if(bDelError)
    {
        if ( nNewTab >= pDoc->GetTableCount() )
                --nNewTab;

            SetTabNo( nNewTab, TRUE );
        if (bWasLinked)
        {
            pDocSh->UpdateLinks();              // Link-Manager updaten
            SFX_BINDINGS().Invalidate(SID_LINKS);
        }

        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    }
    else
    {
        delete pUndoDoc;
        delete pUndoData;
    }
    return bDelError;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::RenameTable( const String& rName, USHORT nTab )
{
    //  Reihenfolge Tabelle/Name ist bei DocFunc umgekehrt
    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        RenameTable( nTab, rName, TRUE, FALSE );
    if (bSuccess)
    {
        //  Der Tabellenname koennte in einer Formel vorkommen...
        GetViewData()->GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}


//----------------------------------------------------------------------------

void ScViewFunc::InsertAreaLink( const String& rFile,
                                    const String& rFilter, const String& rOptions,
                                    const String& rSource )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nPosX = GetViewData()->GetCurX();
    USHORT nPosY = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();
    ScAddress aPos( nPosX, nPosY, nTab );

    String aFilterName = rFilter;
    String aNewOptions = rOptions;
    if (!aFilterName.Len())
        ScDocumentLoader::GetFilterName( rFile, aFilterName, aNewOptions );

    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocSh, rFile, aFilterName, aNewOptions, rSource, aPos );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rFile, &aFilterName, &rSource );

    //  Undo fuer den leeren Link

    pDocSh->GetUndoManager()->AddUndoAction( new ScUndoInsertAreaLink( pDocSh,
                                                rFile, aFilterName, aNewOptions,
                                                rSource, ScRange(aPos) ) );

    //  Update hat sein eigenes Undo

    pLink->SetDoInsert(FALSE);  // beim ersten Update nichts einfuegen
    pLink->Update();            // kein SetInCreate -> Update ausfuehren
    pLink->SetDoInsert(TRUE);   // Default = TRUE

    SfxBindings& rBindings = SFX_BINDINGS();
    rBindings.Invalidate( SID_LINKS );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}


//----------------------------------------------------------------------------

void ScViewFunc::InsertTableLink( const String& rFile,
                                    const String& rFilter, const String& rOptions,
                                    const String& rTabName )
{
    String aFilterName = rFilter;
    String aOpt = rOptions;
    ScDocumentLoader aLoader( rFile, aFilterName, aOpt );
    if (!aLoader.IsError())
    {
        ScDocShell* pSrcSh = aLoader.GetDocShell();
        ScDocument* pSrcDoc = pSrcSh->GetDocument();
        USHORT nTab = MAXTAB+1;
        if (!rTabName.Len())                // kein Name angegeben -> erste Tabelle
            nTab = 0;
        else
        {
            String aTemp;
            USHORT nCount = pSrcDoc->GetTableCount();
            for (USHORT i=0; i<nCount; i++)
            {
                pSrcDoc->GetName( i, aTemp );
                if ( aTemp == rTabName )
                    nTab = i;
            }
        }

        if ( nTab <= MAXTAB )
            ImportTables( pSrcSh, 1, &nTab, TRUE,
                        GetViewData()->GetTabNo() );
    }
}


//----------------------------------------------------------------------------
//  Tabellen aus anderem Dokument kopieren / linken

void ScViewFunc::ImportTables( ScDocShell* pSrcShell,
                                USHORT nCount, const USHORT* pSrcTabs, BOOL bLink,USHORT nTab )
{
    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    //USHORT nTab = GetViewData()->GetTabNo();

    BOOL bError = FALSE;
    BOOL bRefs = FALSE;
    BOOL bName = FALSE;

    if (pSrcDoc->GetDrawLayer())
        pDocSh->MakeDrawLayer();

    pDoc->BeginDrawUndo();          // drawing layer must do its own undo actions

    USHORT nInsCount = 0;
    USHORT i;
    for( i=0; i<nCount; i++ )
    {   // #63304# insert sheets first and update all references
        String aName;
        pSrcDoc->GetName( pSrcTabs[i], aName );
        pDoc->CreateValidTabName( aName );
        if ( !pDoc->InsertTab( nTab+i, aName ) )
        {
            bError = TRUE;      // total error
            break;  // for
        }
        ++nInsCount;
    }
    for (i=0; i<nCount && !bError; i++)
    {
        USHORT nSrcTab = pSrcTabs[i];
        USHORT nDestTab1=nTab+i;
        ULONG nErrVal = pDoc->TransferTab( pSrcDoc, nSrcTab, nDestTab1,
            FALSE );        // no insert

        switch (nErrVal)
        {
            case 0:                     // interner Fehler oder voll Fehler
                bError = TRUE;
                break;
            case 2:
                bRefs = TRUE;
                break;
            case 3:
                bName = TRUE;
                break;
            case 4:
                bRefs = bName = TRUE;
                break;
        }

        // TransferTab doesn't copy drawing objects with bInsertNew=FALSE
        if ( !bError )
            pDoc->TransferDrawPage( pSrcDoc, nSrcTab, nDestTab1 );

        if(!bError &&pSrcDoc->IsScenario(nSrcTab))
        {
            String aComment;
            Color  aColor;
            USHORT nFlags;

            pSrcDoc->GetScenarioData(nSrcTab, aComment,aColor, nFlags);
            pDoc->SetScenario( nDestTab1,TRUE);
            pDoc->SetScenarioData( nTab+i,aComment,aColor,nFlags);
            BOOL bActive = pSrcDoc->IsActiveScenario(nSrcTab );
            pDoc->SetActiveScenario( nDestTab1, bActive );
            BOOL bVisible=pSrcDoc->IsVisible(nSrcTab);
            pDoc->SetVisible(nDestTab1,bVisible );

        }
    }

    if (bLink)
    {
        SvxLinkManager* pLinkManager = pDoc->GetLinkManager();

        SfxMedium* pMed = pSrcShell->GetMedium();
        String aFileName = pMed->GetName();
        String aFilterName;
        if (pMed->GetFilter())
            aFilterName = pMed->GetFilter()->GetFilterName();
        String aOptions = ScDocumentLoader::GetOptions(*pMed);

        BOOL bWasThere = pDoc->HasLink( aFileName, aFilterName, aOptions );

        String aTabStr;
        for (i=0; i<nInsCount; i++)
        {
            pSrcDoc->GetName( pSrcTabs[i], aTabStr );
            pDoc->SetLink( nTab+i, SC_LINK_NORMAL,
                                aFileName, aFilterName, aOptions, aTabStr );
        }

        if (!bWasThere)         // Link pro Quelldokument nur einmal eintragen
        {
            ScTableLink* pLink = new ScTableLink( pDocSh, aFileName, aFilterName, aOptions );
            pLink->SetInCreate( TRUE );
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aFilterName );
            pLink->Update();
            pLink->SetInCreate( FALSE );

            SfxBindings& rBindings = SFX_BINDINGS();
            rBindings.Invalidate( SID_LINKS );
        }
    }


    pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoImportTab( pDocSh, nTab, nCount, bLink ) );

    for (i=0; i<nInsCount; i++)
        GetViewData()->InsertTab(nTab);
    SetTabNo(nTab,TRUE);
    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS );

    SfxApplication* pSfxApp = SFX_APP();
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );

    pDocSh->PostPaintExtras();
    pDocSh->PostPaintGridAll();
    pDocSh->SetDocumentModified();

    if (bRefs)
        ErrorMessage(STR_ABSREFLOST);
    if (bName)
        ErrorMessage(STR_NAMECONFLICT);
}


//----------------------------------------------------------------------------
//  Tabelle in anderes Dokument verschieben / kopieren

void ScViewFunc::MoveTable( USHORT nDestDocNo, USHORT nDestTab, BOOL bCopy )
{
    ScDocument* pDoc       = GetViewData()->GetDocument();
    ScDocShell* pDocShell  = GetViewData()->GetDocShell();
    ScDocument* pDestDoc   = NULL;
    ScDocShell* pDestShell = NULL;
    ScTabViewShell* pDestViewSh = NULL;

    BOOL bNewDoc = ( nDestDocNo == SC_DOC_NEW );
    if ( bNewDoc )
    {
        nDestTab = 0;           // als erstes einfügen

        //  ohne SFX_CALLMODE_RECORD ausfuehren, weil schon im Move-Befehl enthalten:

        String aUrl = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("private:factory/"));
        aUrl.AppendAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));               // "scalc4"
        SfxStringItem aItem( SID_FILE_NAME, aUrl );
        const SfxObjectItem* pObjItem = (const SfxObjectItem*) SFX_DISPATCHER().Execute(
                    SID_OPENDOC, SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON, &aItem, 0L );
        if (pObjItem)
        {
            pDestShell = PTR_CAST( ScDocShell, pObjItem->GetShell() );
            if (pDestShell)
                pDestViewSh = pDestShell->GetBestViewShell();
        }

        //  Mit SID_NEWDOCDIRECT geht's diese Woche gerade mal gar nicht,
        //  weil es ein SfxFrameItem zurueckgibt
#if 0
        //! SID_NEWDOCDIRECT will immer aufzeichnen !!!
        //! Mit CALLMODE_API wuerde der Returnwert nicht stimmen (SbxObject statt Frame)

        String aFactory = STRING_SCAPP;     // "scalc4"
        SfxStringItem aItem( SID_NEWDOCDIRECT, aFactory );
        const SfxViewFrameItem* pViewFrameItem = (const SfxViewFrameItem*)
            SFX_DISPATCHER().Execute( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, &aItem, 0L );

        if ( pViewFrameItem )
        {
            SfxViewFrame* pFrame = pViewFrameItem->GetFrame();
            if (pFrame)
            {
                pDestViewSh = PTR_CAST( ScTabViewShell, pFrame->GetViewShell() );
                pDestShell  = PTR_CAST( ScDocShell, pFrame->GetObjectShell() );
            }
        }
#endif
    }
    else
        pDestShell = ScDocShell::GetShellByNum( nDestDocNo );

    if (!pDestShell)
    {
        DBG_ERROR("Dest-Doc nicht gefunden !!!");
        return;
    }

    pDestDoc = pDestShell->GetDocument();

    USHORT nTab = GetViewData()->GetTabNo();

    if (pDestDoc != pDoc)
    {
        if (bNewDoc)
        {
            while (pDestDoc->GetTableCount() > 1)
                pDestDoc->DeleteTab(0);
            pDestDoc->RenameTab( 0,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("______42_____")),
                        FALSE );
        }

        ScMarkData& rMark       = GetViewData()->GetMarkData();
        USHORT      nTabCount   = pDoc->GetTableCount();
        USHORT      nTabSelCount = rMark.GetSelectCount();

        SvUShorts   TheTabs;

        int i;
        for(i=0;i<nTabCount;i++)
        {
            if(rMark.GetTableSelect(i))
            {
                String aTabName;
                pDoc->GetName( i, aTabName);
                TheTabs.Insert(i,TheTabs.Count());
                for(USHORT j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j))&&(pDoc->IsScenario(j)))
                    {
                        pDoc->GetName( j, aTabName);
                        TheTabs.Insert(j,TheTabs.Count());
                        i=j;
                    }
                    else break;
                }
            }
        }

        GetFrameWin()->EnterWait();

        if (pDoc->GetDrawLayer())
            pDestShell->MakeDrawLayer();

        if (!bNewDoc)
            pDestDoc->BeginDrawUndo();      // drawing layer must do its own undo actions

        ULONG nErrVal =1;
        if(nDestTab==0xFFFF) nDestTab=pDestDoc->GetTableCount();
        USHORT nDestTab1=nDestTab;
        for( i=0; i<TheTabs.Count(); i++, nDestTab1++ )
        {   // #63304# insert sheets first and update all references
            String aName;
            pDoc->GetName( TheTabs[i], aName );
            pDestDoc->CreateValidTabName( aName );
            if ( !pDestDoc->InsertTab( nDestTab1, aName ) )
            {
                nErrVal = 0;        // total error
                break;  // for
            }
        }
        if ( nErrVal > 0 )
        {
            nDestTab1 = nDestTab;
            for(i=0;i<TheTabs.Count();i++)
            {
                nErrVal = pDestDoc->TransferTab( pDoc, TheTabs[i], nDestTab1,
                    FALSE );        // no insert

                // TransferTab doesn't copy drawing objects with bInsertNew=FALSE
                if ( nErrVal > 0 )
                    pDestDoc->TransferDrawPage( pDoc, TheTabs[i], nDestTab1 );

                if(nErrVal>0 && pDoc->IsScenario(TheTabs[i]))
                {
                    String aComment;
                    Color  aColor;
                    USHORT nFlags;

                    pDoc->GetScenarioData(TheTabs[i], aComment,aColor, nFlags);
                    pDestDoc->SetScenario(nDestTab1,TRUE);
                    pDestDoc->SetScenarioData(nDestTab1,aComment,aColor,nFlags);
                    BOOL bActive = pDoc->IsActiveScenario(TheTabs[i]);
                    pDestDoc->SetActiveScenario(nDestTab1, bActive );

                    BOOL bVisible=pDoc->IsVisible(TheTabs[i]);
                    pDestDoc->SetVisible(nDestTab1,bVisible );

                }
                nDestTab1++;
            }
        }
        String sName;
        if (!bNewDoc)
        {
            pDestDoc->GetName(nDestTab, sName);
            pDestShell->GetUndoManager()->AddUndoAction(
                            new ScUndoImportTab( pDestShell, nDestTab, TheTabs.Count(), FALSE));

        }
        else
        {
            pDestShell->GetUndoManager()->Clear();
        }

        GetFrameWin()->LeaveWait();
        switch (nErrVal)
        {
            case 0:                     // interner Fehler oder voll Fehler
            {
                ErrorMessage(STR_TABINSERT_ERROR);
                return;
            }
            break;
            case 2:
                ErrorMessage(STR_ABSREFLOST);
            break;
            case 3:
                ErrorMessage(STR_NAMECONFLICT);
            break;
            case 4:
            {
                ErrorMessage(STR_ABSREFLOST);
                ErrorMessage(STR_NAMECONFLICT);
            }
            break;
            default:
            break;
        }
        //pDestShell->GetUndoManager()->Clear();        //! Undo implementieren !!!
/*
        String sName;
        pDestDoc->GetName(nDestTab, sName);
        pDestShell->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( pDestShell, nDestTab, TRUE, sName ) );
*/
        if (!bCopy)
        {
            if(nTabCount!=nTabSelCount)
                DeleteTables(TheTabs);// incl. Paint & Undo
            else
                ErrorMessage(STR_TABREMOVE_ERROR);
        }

        if (bNewDoc)
        {
            //  ChartListenerCollection must be updated before DeleteTab
            if ( pDestDoc->IsChartListenerCollectionNeedsUpdate() )
                pDestDoc->UpdateChartListenerCollection();

            pDestDoc->DeleteTab(TheTabs.Count());               // alte erste Tabelle
//?         pDestDoc->SelectTable(0, TRUE);     // neue erste Tabelle selektieren
            if (pDestViewSh)
                pDestViewSh->TabChanged();      // Pages auf dem Drawing-Layer
            pDestShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                    PAINT_GRID | PAINT_TOP | PAINT_LEFT |
                                    PAINT_EXTRAS | PAINT_SIZE );
            //  PAINT_SIZE fuer Gliederung
        }
        else
        {
            pDestShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, nDestTab ) );
            pDestShell->PostPaintExtras();
            pDestShell->PostPaintGridAll();
        }

        TheTabs.Remove(0,TheTabs.Count());

        pDestShell->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    }
    else                    // innerhalb des Dokuments
    {

        ScMarkData& rMark       = GetViewData()->GetMarkData();
        USHORT      nTabCount   = pDoc->GetTableCount();
        USHORT      nTabSelCount = rMark.GetSelectCount();

        SvUShorts   TheTabs;
        SvUShorts   TheDestTabs;
        SvStrings   TheTabNames;
        String      aDestName;
        String      *pString;

        int i;
        for(i=0;i<nTabCount;i++)
        {
            if(rMark.GetTableSelect(i))
            {
                String aTabName;
                pDoc->GetName( i, aTabName);
                TheTabNames.Insert(new String(aTabName),TheTabNames.Count());

                for(USHORT j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j))&&(pDoc->IsScenario(j)))
                    {
                        pDoc->GetName( j, aTabName);
                        TheTabNames.Insert(new String(aTabName),TheTabNames.Count());
                        i=j;
                    }
                    else break;
                }

            }
        }

        if (bCopy)
            pDoc->BeginDrawUndo();          // drawing layer must do its own undo actions

        pDoc->GetName( nDestTab, aDestName);
        USHORT nDestTab1=nDestTab;
        USHORT nMovTab=0;
        for(i=0;i<TheTabNames.Count();i++)
        {
            nTabCount   = pDoc->GetTableCount();
            pString=TheTabNames[i];
            if(!pDoc->GetTable(*pString,nMovTab))
            {
                nMovTab=nTabCount;
            }
            if(!pDoc->GetTable(aDestName,nDestTab1))
            {
                nDestTab1=nTabCount;
            }
            pDocShell->MoveTable( nMovTab, nDestTab1, bCopy, FALSE );   // Undo ist hier

            if(bCopy && pDoc->IsScenario(nMovTab))
            {
                String aComment;
                Color  aColor;
                USHORT nFlags;

                pDoc->GetScenarioData(nMovTab, aComment,aColor, nFlags);
                pDoc->SetScenario(nDestTab1,TRUE);
                pDoc->SetScenarioData(nDestTab1,aComment,aColor,nFlags);
                BOOL bActive = pDoc->IsActiveScenario(nMovTab );
                pDoc->SetActiveScenario( nDestTab1, bActive );
                BOOL bVisible=pDoc->IsVisible(nMovTab);
                pDoc->SetVisible(nDestTab1,bVisible );
            }

            TheTabs.Insert(nMovTab,TheTabs.Count());

            if(!bCopy)
            {
                if(!pDoc->GetTable(*pString,nDestTab1))
                {
                    nDestTab1=nTabCount;
                }
            }

            TheDestTabs.Insert(nDestTab1,TheDestTabs.Count());
            delete pString;
        }

        nTab = GetViewData()->GetTabNo();

        if (bCopy)
        {
            pDocShell->GetUndoManager()->AddUndoAction(
                    new ScUndoCopyTab( pDocShell, TheTabs, TheDestTabs));
        }
        else
        {
            pDocShell->GetUndoManager()->AddUndoAction(
                    new ScUndoMoveTab( pDocShell, TheTabs, TheDestTabs));
        }

        USHORT nNewTab = nDestTab;
        if (nNewTab == SC_TAB_APPEND)
            nNewTab = pDoc->GetTableCount()-1;
        else if (!bCopy && nTab<nDestTab)
            nNewTab--;

        SetTabNo( nNewTab, TRUE );
    }
}


//----------------------------------------------------------------------------

void ScViewFunc::ShowTable( const String& rName )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    BOOL bFound = FALSE;
    USHORT nPos = 0;
    String aTabName;
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT i=0; i<nCount; i++)
    {
        pDoc->GetName( i, aTabName );
        if ( aTabName == rName )
        {
            nPos = i;
            bFound = TRUE;
        }
    }

    if (bFound)
    {
        pDoc->SetVisible( nPos, TRUE );
        pDocSh->GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( pDocSh, nPos, TRUE ) );
        SetTabNo( nPos, TRUE );
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
        pDocSh->SetDocumentModified();
    }
    else
        Sound::Beep();
}


//----------------------------------------------------------------------------

void ScViewFunc::HideTable( USHORT nTab )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    USHORT nVisible = 0;
    USHORT nCount = pDoc->GetTableCount();
    for (USHORT i=0; i<nCount; i++)
    {
        if (pDoc->IsVisible(i))
            ++nVisible;
    }

    if (nVisible > 1)
    {
        pDoc->SetVisible( nTab, FALSE );
        pDocSh->GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( pDocSh, nTab, FALSE ) );

        //  Views updaten:
        pDocSh->Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );

        SetTabNo( nTab, TRUE );
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
        pDocSh->SetDocumentModified();
    }
    else
        Sound::Beep();
}


//----------------------------------------------------------------------------

void ScViewFunc::InsertSpecialChar( const String& rStr, const Font& rFont )
{
    if (!SelectionEditable())
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    const sal_Unicode* pChar    = rStr.GetBuffer();
    ScTabViewShell* pViewShell  = GetViewData()->GetViewShell();
    SvxFontItem     aFontItem( rFont.GetFamily(),
                               rFont.GetName(),
                               rFont.GetStyleName(),
                               rFont.GetPitch(),
                               rFont.GetCharSet(),
                               ATTR_FONT );
    ApplyAttr( aFontItem );

    while ( *pChar )
        pViewShell->TabKeyInput( KeyEvent( *(pChar++), KeyCode() ) );
}


//----------------------------------------------------------------------------

void ScViewFunc::UpdateLineAttrs( SvxBorderLine&       rLine,
                                  const SvxBorderLine* pDestLine,
                                  const SvxBorderLine* pSrcLine,
                                  BOOL                 bColor )
{
    if ( pSrcLine && pDestLine )
    {
        if ( bColor )
        {
            rLine.SetColor      ( pSrcLine->GetColor() );
            rLine.SetOutWidth   ( pDestLine->GetOutWidth() );
            rLine.SetInWidth    ( pDestLine->GetInWidth() );
            rLine.SetDistance   ( pDestLine->GetDistance() );
        }
        else
        {
            rLine.SetColor      ( pDestLine->GetColor() );
            rLine.SetOutWidth   ( pSrcLine->GetOutWidth() );
            rLine.SetInWidth    ( pSrcLine->GetInWidth() );
            rLine.SetDistance   ( pSrcLine->GetDistance() );
        }
    }
}


#define SET_LINE_ATTRIBUTES(LINE,BOXLINE) \
    pBoxLine = aBoxItem.Get##LINE();                                \
    if ( pBoxLine )                                                 \
    {                                                               \
        if ( pLine )                                                \
        {                                                           \
            UpdateLineAttrs( aLine, pBoxLine, pLine, bColorOnly );  \
            aBoxItem.SetLine( &aLine, BOXLINE );                    \
        }                                                           \
        else                                                        \
            aBoxItem.SetLine( NULL, BOXLINE );                      \
    }


//----------------------------------------------------------------------------

void ScViewFunc::SetSelectionFrameLines( const SvxBorderLine* pLine,
                                         BOOL bColorOnly )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocument*             pDoc = GetViewData()->GetDocument();
    ScMarkData&             rMark = GetViewData()->GetMarkData();
    ScDocShell*             pDocSh = GetViewData()->GetDocShell();
    const ScPatternAttr*    pSelAttrs = GetSelectionPattern();
    const SfxPoolItem*      pBorderAttr = NULL;
    SfxItemState            eItemState =
                                pSelAttrs->GetItemSet().
                                    GetItemState( ATTR_BORDER,
                                                  TRUE,
                                                  &pBorderAttr );

    if ( eItemState != SFX_ITEM_DEFAULT )
    {
        if ( eItemState == SFX_ITEM_SET )
        {
            DBG_ASSERT( pBorderAttr, "Border-Attr not set!" );
            SvxBoxItem      aBoxItem( *(const SvxBoxItem*)pBorderAttr );
            SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );
            ScDocument*     pDoc = GetViewData()->GetDocument();
            SfxItemSet*     pOldSet = new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            ATTR_PATTERN_START,
                                            ATTR_PATTERN_END );
            SfxItemSet*     pNewSet = new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            ATTR_PATTERN_START,
                                            ATTR_PATTERN_END );

            //------------------------------------------------------------
            const SvxBorderLine*    pBoxLine = NULL;
            SvxBorderLine           aLine;

            // hier wird die pBoxLine benutzt:

            SET_LINE_ATTRIBUTES(Top,BOX_LINE_TOP)
            SET_LINE_ATTRIBUTES(Bottom,BOX_LINE_BOTTOM)
            SET_LINE_ATTRIBUTES(Left,BOX_LINE_LEFT)
            SET_LINE_ATTRIBUTES(Right,BOX_LINE_RIGHT)

            aBoxInfoItem.SetLine( aBoxItem.GetTop(), BOXINFO_LINE_HORI );
            aBoxInfoItem.SetLine( aBoxItem.GetLeft(), BOXINFO_LINE_VERT );
            aBoxInfoItem.ResetFlags(); // Lines auf Valid setzen

            pOldSet->Put( *pBorderAttr );
            pNewSet->Put( aBoxItem );
            pNewSet->Put( aBoxInfoItem );

            ApplyAttributes( pNewSet, pOldSet );

            delete pOldSet;
            delete pNewSet;
        }
        else // if ( eItemState == SFX_ITEM_DONTCARE )
        {
            rMark.MarkToMulti();
            pDoc->ApplySelectionLineStyle( rMark, pLine, bColorOnly );
        }

        ScRange aMarkRange;
        rMark.GetMultiMarkArea( aMarkRange );
        USHORT nStartCol = aMarkRange.aStart.Col();
        USHORT nStartRow = aMarkRange.aStart.Row();
        USHORT nStartTab = aMarkRange.aStart.Tab();
        USHORT nEndCol = aMarkRange.aEnd.Col();
        USHORT nEndRow = aMarkRange.aEnd.Row();
        USHORT nEndTab = aMarkRange.aEnd.Tab();
        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol, nEndRow, nEndTab,
                           PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

        pDocSh->UpdateOle( GetViewData() );
        pDocSh->SetDocumentModified();
    }
}

#undef SET_LINE_ATTRIBUTES


//----------------------------------------------------------------------------

void ScViewFunc::SetConditionalFormat( const ScConditionalFormat& rNew )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ULONG nIndex = pDoc->AddCondFormat(rNew);           // dafuer gibt's kein Undo
    SfxUInt32Item aItem( ATTR_CONDITIONAL, nIndex );

    ApplyAttr( aItem );         // mit Paint und Undo...
}


//----------------------------------------------------------------------------

void ScViewFunc::SetValidation( const ScValidationData& rNew )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ULONG nIndex = pDoc->AddValidationEntry(rNew);      // dafuer gibt's kein Undo
    SfxUInt32Item aItem( ATTR_VALIDDATA, nIndex );

    ApplyAttr( aItem );         // mit Paint und Undo...
}


