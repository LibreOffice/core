/*************************************************************************
 *
 *  $RCSfile: htmlpars.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-06 12:09:18 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <sot/dtrans.hxx>
#define SC_HTMLPARS_CXX
#include "scitems.hxx"
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#include <svx/algitem.hxx>
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#include <svx/brshitem.hxx>
#include <svx/editeng.hxx>
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#include <svx/impgrf.hxx>
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#include <svx/wghtitem.hxx>
#include <svx/boxitem.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/frmhtml.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/eitem.hxx>
#include <svtools/filter.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>


#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include "htmlpars.hxx"
#include "global.hxx"
#include "flttools.hxx"
#include "document.hxx"
#include "rangelst.hxx"


SV_IMPL_VARARR_SORT( ScHTMLColOffset, ULONG );

//------------------------------------------------------------------------

ScHTMLTableData::ScHTMLTableData(
        const String& rTabName,
        USHORT _nFirstCol, USHORT _nFirstRow,
        USHORT _nColSpan, USHORT _nRowSpan ) :
    aTableName( rTabName ),
    nFirstCol( _nFirstCol ),
    nFirstRow( _nFirstRow ),
    nLastCol( _nFirstCol ),
    nLastRow( _nFirstRow ),
    nColSpan( _nColSpan ),
    nRowSpan( _nRowSpan ),
    nDocCol( _nFirstCol ),
    nDocRow( _nFirstRow ),
    pNestedTables( NULL )
{
}

ScHTMLTableData::~ScHTMLTableData()
{
    if( pNestedTables )
        delete pNestedTables;
}

USHORT ScHTMLTableData::GetCount( const Table& rTab, ULONG nIndex )
{
    USHORT nCount = (USHORT)(ULONG) rTab.Get( nIndex );
    return Max( nCount, (USHORT)1 );        // width/height is at least 1 cell
}

USHORT ScHTMLTableData::GetSize( const Table& rDataTab, ULONG nStart, ULONG nEnd )
{
    USHORT nSize = 0;
    for( ULONG nIndex = nStart; nIndex <= nEnd; nIndex++ )
        nSize += GetCount( rDataTab, nIndex );
    return nSize;
}

USHORT ScHTMLTableData::CalcDocPos( ScHTMLTableDataKey eCRKey, ULONG nIndex ) const
{
    USHORT nFirst = GetFirst( eCRKey );
    return GetDocPos( eCRKey ) + ((nFirst < nIndex) ? GetSize( GetDataTable( eCRKey ), nFirst, nIndex - 1 ) : 0);
}

USHORT ScHTMLTableData::CalcSpan( ScHTMLTableDataKey eCRKey, ULONG nIndex, USHORT nSpan ) const
{
    return GetSize( GetDataTable( eCRKey ), nIndex, nIndex + nSpan - 1 );
}

void ScHTMLTableData::GetRange( ScRange& rRange ) const
{
    USHORT nCol = CalcDocCol( nFirstCol );
    USHORT nRow = CalcDocRow( nFirstRow );
    rRange.aStart.Set( nCol, nRow, 0 );
    rRange.aEnd.Set( nCol + GetSize( tdCol ) - 1, nRow + GetSize( tdRow ) - 1, 0 );
}

ScHTMLTableData* ScHTMLTableData::GetNestedTable( ULONG nTab ) const
{
    return pNestedTables ? pNestedTables->GetTable( nTab ) : NULL;
}

void ScHTMLTableData::SetMaxCount( Table& rDataTab, ULONG nIndex, USHORT nCount )
{
    if( GetCount( rDataTab, nIndex ) < nCount )
    {
        if( rDataTab.IsKeyValid( nIndex ) )
            rDataTab.Replace( nIndex, (void*) nCount );
        else
            rDataTab.Insert( nIndex, (void*) nCount );
    }
}

ScHTMLTableData* ScHTMLTableData::InsertNestedTable(
        ULONG nTab, const String& rTabName,
        USHORT _nFirstCol, USHORT _nFirstRow, USHORT _nColSpan, USHORT _nRowSpan )
{
    if( !pNestedTables )
        pNestedTables = new ScHTMLTableDataTable;
    return pNestedTables->InsertTable( nTab, rTabName, _nFirstCol, _nFirstRow, _nColSpan, _nRowSpan );
}

void ScHTMLTableData::ChangeDocCoord( short nColDiff, short nRowDiff )
{
    nDocCol += nColDiff;
    nDocRow += nRowDiff;
    if( pNestedTables )
        for( ScHTMLTableData* pTable = pNestedTables->GetFirst(); pTable; pTable = pNestedTables->GetNext() )
            pTable->ChangeDocCoord( nColDiff, nRowDiff );
}

void ScHTMLTableData::SetDocCoord( USHORT nCol, USHORT nRow )
{
    short nColDiff = nCol - nFirstCol;
    short nRowDiff = nRow - nFirstRow;
    ChangeDocCoord( nColDiff, nRowDiff );
}

void ScHTMLTableData::RecalcSizeDim( ScHTMLTableDataKey eCRKey )
{
    if( !pNestedTables ) return;
    for( ScHTMLTableData* pTable = pNestedTables->GetFirstInOrder( eCRKey );
            pTable; pTable = pNestedTables->GetNextInOrder() )
    {
        USHORT nIndex = pTable->GetFirst( eCRKey );
        USHORT nSpan = pTable->GetSpan( eCRKey );
        USHORT nSize = pTable->GetSize( eCRKey );
        Table& rDataTab = GetDataTable( eCRKey );
        while( nSpan > 1 )
        {
            nSize -= Min( nSize, GetCount( rDataTab, nIndex ) );
            nIndex++;
            nSpan--;
        }
        SetMaxCount( rDataTab, nIndex, nSize );
    }
}

void ScHTMLTableData::RecalcSize()
{
    if( !pNestedTables ) return;
    pNestedTables->RecalcSizes();
    // all nested tables have the right size now -> calculate it for this table
    RecalcSizeDim( tdCol );
    RecalcSizeDim( tdRow );
    // set correct position of nested tables inside of THIS table
    for( ScHTMLTableData* pTable = pNestedTables->GetFirst(); pTable; pTable = pNestedTables->GetNext() )
        pTable->SetDocCoord( CalcDocCol( pTable->GetFirstCol() ), CalcDocRow( pTable->GetFirstRow() ) );
}

void ScHTMLTableData::SetCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos )
{
    const USHORT nOuterLine = DEF_LINE_WIDTH_2;
    const USHORT nInnerLine = DEF_LINE_WIDTH_0;
    SvxBorderLine aOuterLine, aInnerLine;
    aOuterLine.SetColor( Color( COL_BLACK ) );
    aOuterLine.SetOutWidth( nOuterLine );
    aInnerLine.SetColor( Color( COL_BLACK ) );
    aInnerLine.SetOutWidth( nInnerLine );
    SvxBoxItem aBorderItem;

    for( USHORT nCol = nFirstCol; nCol <= nLastCol; nCol++ )
    {
        SvxBorderLine* pLeftLine = (nCol == nFirstCol) ? &aOuterLine : &aInnerLine;
        SvxBorderLine* pRightLine = (nCol == nLastCol) ? &aOuterLine : &aInnerLine;
        USHORT nCellCol1 = CalcDocCol( nCol ) + rFirstPos.Col();
        USHORT nCellCol2 = nCellCol1 + GetCount( aColCount, nCol ) - 1;
        for( USHORT nRow = nFirstRow; nRow <= nLastRow; nRow++ )
        {
            SvxBorderLine* pTopLine = (nRow == nFirstRow) ? &aOuterLine : &aInnerLine;
            SvxBorderLine* pBottomLine = (nRow == nLastRow) ? &aOuterLine : &aInnerLine;
            USHORT nCellRow1 = CalcDocRow( nRow ) + rFirstPos.Row();
            USHORT nCellRow2 = nCellRow1 + GetCount( aRowCount, nRow ) - 1;
            for( USHORT nCellCol = nCellCol1; nCellCol <= nCellCol2; nCellCol++ )
            {
                aBorderItem.SetLine( (nCellCol == nCellCol1) ? pLeftLine : NULL, BOX_LINE_LEFT );
                aBorderItem.SetLine( (nCellCol == nCellCol2) ? pRightLine : NULL, BOX_LINE_RIGHT );
                for( USHORT nCellRow = nCellRow1; nCellRow <= nCellRow2; nCellRow++ )
                {
                    aBorderItem.SetLine( (nCellRow == nCellRow1) ? pTopLine : NULL, BOX_LINE_TOP );
                    aBorderItem.SetLine( (nCellRow == nCellRow2) ? pBottomLine : NULL, BOX_LINE_BOTTOM );
                    pDoc->ApplyAttr( nCellCol, nCellRow, rFirstPos.Tab(), aBorderItem );
                }
            }
        }
    }

    if( pNestedTables )
        pNestedTables->SetCellBorders( pDoc, rFirstPos );
}



ScHTMLTableDataTable::ScHTMLTableDataTable() :
    pCurrTab( NULL ),
    nCurrTab( 0 )
{
}

ScHTMLTableDataTable::~ScHTMLTableDataTable()
{
    for( ScHTMLTableData* pData = GetFirst(); pData; pData = GetNext() )
        delete pData;
}

ScHTMLTableData* ScHTMLTableDataTable::GetTable( ULONG nTab )
{
    if( nTab == nCurrTab )
        return pCurrTab;
    if( !nTab )
        return NULL;

    ScHTMLTableData* pTable = _Get( nTab );
    if( pTable )
    {
        SetCurrTable( nTab, pTable );
        return pTable;
    }

    ScHTMLTableData* pNested = NULL;
    for( pTable = GetFirst(); pTable; pTable = GetNext() )
    {
        pNested = pTable->GetNestedTable( nTab );
        if( pNested )
        {
            SetCurrTable( nTab, pNested );
            return pNested;
        }
    }
    return NULL;
}

ScHTMLTableData* ScHTMLTableDataTable::GetFirstInOrder( ScHTMLTableDataKey eKey )
{
    nSearchMin1 = nSearchMin2 = 0;
    eSearchKey = eKey;
    return GetNextInOrder();
}

ScHTMLTableData* ScHTMLTableDataTable::GetNextInOrder()
{
    ScHTMLTableDataKey eOppKey = (eSearchKey == tdCol) ? tdRow : tdCol;
    ScHTMLTableData* pTableFound = NULL;
    USHORT nFoundPos1 = (USHORT)~0;
    USHORT nFoundPos2 = (USHORT)~0;

    for( ScHTMLTableData* pTable = GetFirst(); pTable; pTable = GetNext() )
    {
        USHORT nTablePos1 = pTable->GetFirst( eSearchKey ) + pTable->GetSpan( eSearchKey ) - 1;
        USHORT nTablePos2 = pTable->GetFirst( eOppKey );
        if( ((nTablePos1 > nSearchMin1) || ((nTablePos1 == nSearchMin1) && (nTablePos2 > nSearchMin2))) &&
            ((nTablePos1 < nFoundPos1) || ((nTablePos1 == nFoundPos1) && (nTablePos2 < nFoundPos2))) )
        {   // position of this table is between min position and last found position
            pTableFound = pTable;
            nFoundPos1 = nTablePos1;
            nFoundPos2 = nTablePos2;
        }
    }
    nSearchMin1 = nFoundPos1;
    nSearchMin2 = nFoundPos2;
    return pTableFound;
}

USHORT ScHTMLTableDataTable::GetNextFreeRow( ULONG nTab )
{
    ScHTMLTableData* pTable = _Get( nTab );
    if( pTable )
        return pTable->GetFirstRow() + pTable->GetSize( tdRow );

    for( pTable = GetFirst(); pTable; pTable = GetNext() )
        if( pTable->GetNestedTable( nTab ) )
            return pTable->GetFirstRow() + pTable->GetSize( tdRow );

    return 0;
}

ScHTMLTableData* ScHTMLTableDataTable::InsertTable(
        ULONG nTab, const String& rTabName, USHORT nFirstCol, USHORT nFirstRow,
        USHORT nColSpan, USHORT nRowSpan, ULONG nNestedIn )
{
    ScHTMLTableData* pTable = GetTable( nTab );
    // table already exists
    if( pTable )
        return pTable;
    // insert in nested table
    if( nNestedIn )
    {
        pTable = GetTable( nNestedIn );
        if( pTable )
        {
            ScHTMLTableData* pNewTab = pTable->InsertNestedTable( nTab, rTabName, nFirstCol, nFirstRow, nColSpan, nRowSpan );
            SetCurrTable( nTab, pNewTab );
            return pNewTab;
        }
    }
    // insert in this table
    _Insert( nTab, pTable = new ScHTMLTableData( rTabName, nFirstCol, nFirstRow, nColSpan, nRowSpan ) );
    SetCurrTable( nTab, pTable );
    return pTable;
}

void ScHTMLTableDataTable::SetCellCoord( ULONG nTab, USHORT nCol, USHORT nRow )
{
    ScHTMLTableData* pTable = GetTable( nTab );
    if( pTable )
        pTable->SetCellCoord( nCol, nRow );
}

void ScHTMLTableDataTable::RecalcSizes()
{
    for( ScHTMLTableData* pTable = GetFirst(); pTable; pTable = GetNext() )
        pTable->RecalcSize();
}

void ScHTMLTableDataTable::Recalc()
{
    // table sizes
    RecalcSizes();
    // table positions (move tables down)
    USHORT nDocRow;
    USHORT nRowDiff = 0;
    for( ScHTMLTableData* pTable = GetFirst(); pTable; pTable = GetNext() )
    {
        nDocRow = pTable->GetFirstRow() + nRowDiff;
        pTable->SetDocCoord( pTable->GetFirstCol(), nDocRow );
        nRowDiff += (pTable->GetSize( tdRow ) + pTable->GetFirstRow() - pTable->GetLastRow() - 1);
    }
}

void ScHTMLTableDataTable::SetCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos )
{
    DBG_ASSERT( pDoc, "ScHTMLTableDataTable::SetCellBorders - no document" );
    for( ScHTMLTableData* pTable = GetFirst(); pTable; pTable = GetNext() )
        pTable->SetCellBorders( pDoc, rFirstPos );
}

//------------------------------------------------------------------------

ScHTMLParser::ScHTMLParser( EditEngine* pEditP, const Size& aPageSizeP,
            ScDocument* pDocP, BOOL _bCalcWidthHeight ) :
        ScEEParser( pEditP ),
        aPageSize( aPageSizeP ),
        xLockedList( new ScRangeList ),
        pDoc( pDocP ),
        pTables( NULL ),
        pTableData( _bCalcWidthHeight ? NULL : new ScHTMLTableDataTable ),
        pColOffset( _bCalcWidthHeight ? new ScHTMLColOffset : NULL ),
        pLocalColOffset( _bCalcWidthHeight ? new ScHTMLColOffset : NULL ),
        nFirstTableCell(0),
        nTableLevel(0),
        nTable(0),
        nMaxTable(0),
        nColCntStart(0),
        nMaxCol(0),
        nTableWidth(0),
        nColOffset(0),
        nColOffsetStart(0),
        nMetaCnt(0),
        bCalcWidthHeight( _bCalcWidthHeight ),
        bTabInTabCell( FALSE ),
        bFirstRow( TRUE ),
        bInCell( FALSE ),
        bInTitle( FALSE )
{
    // neue Konfiguration setzen
    OfaHtmlOptions* pHtmlOptions = ((OfficeApplication*)SFX_APP())->GetHtmlOptions();
    for ( USHORT j=0; j < SC_HTML_FONTSIZES; j++ )
    {
        // in Twips merken, wie unser SvxFontHeightItem
        aFontHeights[j] = pHtmlOptions->GetFontSize( j ) * 20;
    }

    if( bCalcWidthHeight )
    {
        MakeColNoRef( pLocalColOffset, 0, 0, 0, 0 );
        MakeColNoRef( pColOffset, 0, 0, 0, 0 );
    }
}


ScHTMLParser::~ScHTMLParser()
{
    ScHTMLTableStackEntry* pS;
    while ( pS = aTableStack.Pop() )
    {
        if ( pList->GetPos( pS->pCellEntry ) == LIST_ENTRY_NOTFOUND )
            delete pS->pCellEntry;
        if ( pS->pLocalColOffset != pLocalColOffset )
            delete pS->pLocalColOffset;
        delete pS;
    }
    if ( pLocalColOffset )
        delete pLocalColOffset;
    if ( pColOffset )
        delete pColOffset;
    if ( pTables )
    {
        for ( Table* pT = (Table*) pTables->First(); pT; pT = (Table*) pTables->Next() )
            delete pT;
        delete pTables;
    }
    if ( pTableData )
        delete pTableData;
}


ULONG ScHTMLParser::Read( SvStream& rStream )
{
    Link aOldLink = pEdit->GetImportHdl();
    pEdit->SetImportHdl( LINK( this, ScHTMLParser, HTMLImportHdl ) );
    ULONG nErr = pEdit->Read( rStream, EE_FORMAT_HTML,
        pDoc->GetDocumentShell()->GetHeaderAttributes() );
    pEdit->SetImportHdl( aOldLink );
    // Spaltenbreiten erzeugen
    if( bCalcWidthHeight )
    {
        Adjust();
        OutputDevice* pDefaultDev = Application::GetDefaultDevice();
        USHORT nCount = pColOffset->Count();
        const ULONG* pOff = (const ULONG*) pColOffset->GetData();
        ULONG nOff = *pOff++;
        Size aSize;
        for ( USHORT j = 1; j < nCount; j++, pOff++ )
        {
            aSize.Width() = *pOff - nOff;
            aSize = pDefaultDev->PixelToLogic( aSize, MapMode( MAP_TWIP ) );
            pColWidths->Insert( j-1, (void*)aSize.Width() );
            nOff = *pOff;
        }
    }
    else
        AdjustNoWidthHeight();
    return nErr;
}


void ScHTMLParser::NewActEntry( ScEEParseEntry* pE )
{
    ScEEParser::NewActEntry( pE );
    if ( pE )
    {
        if ( !pE->aSel.HasRange() )
        {   // komplett leer, nachfolgender Text landet im gleichen Absatz!
            pActEntry->aSel.nStartPara = pE->aSel.nEndPara;
            pActEntry->aSel.nStartPos = pE->aSel.nEndPos;
        }
    }
    pActEntry->aSel.nEndPara = pActEntry->aSel.nStartPara;
    pActEntry->aSel.nEndPos = pActEntry->aSel.nStartPos;
}


void ScHTMLParser::EntryEnd( ScEEParseEntry* pE, const ESelection& rSel )
{
    if ( rSel.nEndPara >= pE->aSel.nStartPara )
    {
        pE->aSel.nEndPara = rSel.nEndPara;
        pE->aSel.nEndPos = rSel.nEndPos;
    }
    else if ( rSel.nStartPara == pE->aSel.nStartPara - 1 && !pE->aSel.HasRange() )
    {   // kein Absatz angehaengt aber leer, nichts tun
    }
    else
    {
        DBG_ERRORFILE( "EntryEnd: EditEngine ESelection End < Start" );
    }
}


void ScHTMLParser::NextRow( ImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( nRowMax < ++nRowCnt )
        nRowMax = nRowCnt;
    nColCnt = nColCntStart;
    nColOffset = nColOffsetStart;
    bFirstRow = FALSE;
}


BOOL ScHTMLParser::SeekOffset( ScHTMLColOffset* pOffset, USHORT nOffset,
        USHORT* pCol, USHORT nOffsetTol )
{
    DBG_ASSERT( pOffset, "ScHTMLParser::SeekOffset - illegal call" );
    if ( pOffset->Seek_Entry( nOffset, pCol ) )
        return TRUE;
    USHORT nCount = pOffset->Count();
    if ( !nCount )
        return FALSE;
    USHORT nCol = *pCol;
    // nCol ist Einfuegeposition, da liegt der Naechsthoehere (oder auch nicht)
    if ( nCol < nCount && (((*pOffset)[nCol] - nOffsetTol) <= nOffset) )
        return TRUE;
    // nicht kleiner als alles andere? dann mit Naechstniedrigerem vergleichen
    else if ( nCol && (((*pOffset)[nCol-1] + nOffsetTol) >= nOffset) )
    {
        (*pCol)--;
        return TRUE;
    }
    return FALSE;
}


void ScHTMLParser::MakeCol( ScHTMLColOffset* pOffset, USHORT& nOffset,
        USHORT& nWidth, USHORT nOffsetTol, USHORT nWidthTol )
{
    DBG_ASSERT( pOffset, "ScHTMLParser::MakeCol - illegal call" );
    USHORT nPos;
    if ( SeekOffset( pOffset, nOffset, &nPos, nOffsetTol ) )
        nOffset = (USHORT)(*pOffset)[nPos];
    else
        pOffset->Insert( nOffset );
    if ( nWidth )
    {
        if ( SeekOffset( pOffset, nOffset + nWidth, &nPos, nWidthTol ) )
            nWidth = (USHORT)(*pOffset)[nPos] - nOffset;
        else
            pOffset->Insert( nOffset + nWidth );
    }
}


void ScHTMLParser::MakeColNoRef( ScHTMLColOffset* pOffset, USHORT nOffset,
        USHORT nWidth, USHORT nOffsetTol, USHORT nWidthTol )
{
    DBG_ASSERT( pOffset, "ScHTMLParser::MakeColNoRef - illegal call" );
    USHORT nPos;
    if ( SeekOffset( pOffset, nOffset, &nPos, nOffsetTol ) )
        nOffset = (USHORT)(*pOffset)[nPos];
    else
        pOffset->Insert( nOffset );
    if ( nWidth )
    {
        if ( !SeekOffset( pOffset, nOffset + nWidth, &nPos, nWidthTol ) )
            pOffset->Insert( nOffset + nWidth );
    }
}


void ScHTMLParser::ModifyOffset( ScHTMLColOffset* pOffset, USHORT& nOldOffset,
            USHORT& nNewOffset, USHORT nOffsetTol )
{
    DBG_ASSERT( pOffset, "ScHTMLParser::ModifyOffset - illegal call" );
    USHORT nPos;
    if ( !SeekOffset( pOffset, nOldOffset, &nPos, nOffsetTol ) )
    {
        if ( SeekOffset( pOffset, nNewOffset, &nPos, nOffsetTol ) )
            nNewOffset = (USHORT)(*pOffset)[nPos];
        else
            pOffset->Insert( nNewOffset );
        return ;
    }
    nOldOffset = (USHORT)(*pOffset)[nPos];
    USHORT nPos2;
    if ( SeekOffset( pOffset, nNewOffset, &nPos2, nOffsetTol ) )
    {
        nNewOffset = (USHORT)(*pOffset)[nPos2];
        return ;
    }
    ULONG* pData = ((ULONG*) pOffset->GetData()) + nPos;        //! QAD
    long nDiff = nNewOffset - nOldOffset;
    if ( nDiff < 0 )
    {
        const ULONG* pStop = pOffset->GetData();
        do
        {
            *pData += nDiff;
        } while ( pStop < pData-- );
    }
    else
    {
        const ULONG* pStop = pOffset->GetData() + pOffset->Count();
        do
        {
            *pData += nDiff;
        } while ( ++pData < pStop );
    }
}


void ScHTMLParser::SkipLocked( ScEEParseEntry* pE, BOOL bJoin )
{
    if ( pE->nCol <= MAXCOL )
    {   // wuerde sonst bei ScAddress falschen Wert erzeugen, evtl. Endlosschleife!
        BOOL bBadCol = FALSE;
        BOOL bAgain;
        ScRange aRange( pE->nCol, pE->nRow, 0,
            pE->nCol + pE->nColOverlap - 1, pE->nRow + pE->nRowOverlap - 1, 0 );
        do
        {
            bAgain = FALSE;
            for ( ScRange* pR = xLockedList->First(); pR; pR = xLockedList->Next() )
            {
                if ( pR->Intersects( aRange ) )
                {
                    pE->nCol = pR->aEnd.Col() + 1;
                    USHORT nTmp = pE->nCol + pE->nColOverlap - 1;
                    if ( pE->nCol > MAXCOL || nTmp > MAXCOL )
                        bBadCol = TRUE;
                    else
                    {
                        bAgain = TRUE;
                        aRange.aStart.SetCol( pE->nCol );
                        aRange.aEnd.SetCol( nTmp );
                    }
                    break;
                }
            }
        } while ( bAgain );
        if ( bJoin && !bBadCol )
            xLockedList->Join( aRange );
    }
}


void ScHTMLParser::Adjust()
{
    for ( ScRange* pR = xLockedList->First(); pR; pR = xLockedList->Next() )
        delete pR;
    xLockedList->Clear();
    ScHTMLAdjustStack aStack;
    ScHTMLAdjustStackEntry* pS;
    USHORT nTab = 0;
    USHORT nLastCol = (USHORT)~0;
    USHORT nNextRow = 0;
    USHORT nCurRow = 0;
    USHORT nPageWidth = (USHORT) aPageSize.Width();
    Table* pTab = NULL;
    for ( ScEEParseEntry* pE = pList->First(); pE; pE = pList->Next() )
    {
        if ( pE->nTab < nTab )
        {   // Table beendet
            if ( pS = aStack.Pop() )
            {
                nLastCol = pS->nLastCol;
                nNextRow = pS->nNextRow;
                nCurRow = pS->nCurRow;
            }
            delete pS;
            nTab = pE->nTab;
            pTab = (pTables ? (Table*) pTables->Get( nTab ) : NULL);

        }
        USHORT nRow = pE->nRow;
        if ( pE->nCol <= nLastCol )
        {   // naechste Zeile
            if ( pE->nRow < nNextRow )
                pE->nRow = nCurRow = nNextRow;
            else
                nCurRow = nNextRow = pE->nRow;
            USHORT nR;
            if ( pTab && (nR = (USHORT)(ULONG) pTab->Get( nCurRow )) )
                nNextRow += nR;
            else
                nNextRow++;
        }
        else
            pE->nRow = nCurRow;
        nLastCol = pE->nCol;    // eingelesene Col
        if ( pE->nTab > nTab )
        {   // neue Table
            aStack.Push( new ScHTMLAdjustStackEntry(
                nLastCol, nNextRow, nCurRow ) );
            nTab = pE->nTab;
            pTab = (pTables ? (Table*) pTables->Get( nTab ) : NULL);
            // neuer Zeilenabstand
            USHORT nR;
            if ( pTab && (nR = (USHORT)(ULONG) pTab->Get( nCurRow )) )
                nNextRow = nCurRow + nR;
            else
                nNextRow = nCurRow + 1;
        }
        if ( nTab == 0 )
            pE->nWidth = nPageWidth;
        else
        {   // echte Table, keine Absaetze auf der Wiese
            if ( pTab )
            {
                USHORT nRowSpan = pE->nRowOverlap;
                for ( USHORT j=0; j < nRowSpan; j++ )
                {   // aus merged Zeilen resultierendes RowSpan
                    USHORT nRows = (USHORT)(ULONG) pTab->Get( nRow+j );
                    if ( nRows > 1 )
                    {
                        pE->nRowOverlap += nRows - 1;
                        if ( j == 0 )
                        {   // merged Zeilen verschieben die naechste Zeile
                            USHORT nTmp = nCurRow + nRows;
                            if ( nNextRow < nTmp )
                                nNextRow = nTmp;
                        }
                    }
                }
            }
        }
        // echte Col
        SeekOffset( pColOffset, pE->nOffset, &pE->nCol );
        USHORT nColBeforeSkip = pE->nCol;
        SkipLocked( pE, FALSE );
        if ( pE->nCol != nColBeforeSkip )
        {
            USHORT nCount = pColOffset->Count();
            if ( nCount <= pE->nCol )
            {
                pE->nOffset = (USHORT) (*pColOffset)[nCount-1];
                MakeCol( pColOffset, pE->nOffset, pE->nWidth );
            }
            else
            {
                pE->nOffset = (USHORT) (*pColOffset)[pE->nCol];
            }
        }
        USHORT nPos;
        if ( pE->nWidth && SeekOffset( pColOffset, pE->nOffset + pE->nWidth, &nPos ) )
            pE->nColOverlap = nPos - pE->nCol;
        else
        {
//2do: das muss nicht korrekt sein, ist aber..
            pE->nColOverlap = 1;
        }
        xLockedList->Join( ScRange( pE->nCol, pE->nRow, 0,
            pE->nCol + pE->nColOverlap - 1, pE->nRow + pE->nRowOverlap - 1, 0 ) );
        // MaxDimensions mitfuehren
        USHORT nTmp = pE->nCol + pE->nColOverlap;
        if ( nColMax < nTmp )
            nColMax = nTmp;
        nTmp = pE->nRow + pE->nRowOverlap;
        if ( nRowMax < nTmp )
            nRowMax = nTmp;
    }
    while ( pS = aStack.Pop() )
        delete pS;
}


void ScHTMLParser::AdjustNoWidthHeight()
{
    DBG_ASSERT( pTableData, "ScHTMLParser::AdjustNoWidthHeight - no table data" );
    pTableData->Recalc();

    USHORT nLastCol = 0;
    USHORT nLastRow = 0;
    USHORT nLastTab = 0;
    USHORT nDocCol = 0;
    USHORT nDocRow = 0;

    for ( ScEEParseEntry* pEntry = pList->First(); pEntry; pEntry = pList->Next() )
    {
        ScHTMLTableData* pTable = pTableData->GetTable( pEntry->nTab );

        if( pTable )
        {
            nDocCol = pTable->CalcDocCol( pEntry->nCol );
            nDocRow = pTable->CalcDocRow( pEntry->nRow );
            pEntry->nColOverlap = pTable->CalcColSpan( pEntry->nCol, pEntry->nColOverlap );
            pEntry->nRowOverlap = pTable->CalcRowSpan( pEntry->nRow, pEntry->nRowOverlap );
        }
        else if( nLastTab )
        {
            nDocCol = pEntry->nCol;
            nDocRow = pTableData->GetNextFreeRow( nLastTab );
        }
        else
        {
            nDocCol += pEntry->nCol - nLastCol;
            nDocRow += pEntry->nRow - nLastRow;
        }

        nLastCol = pEntry->nCol;
        nLastRow = pEntry->nRow;
        nLastTab = pEntry->nTab;

        pEntry->nCol = nDocCol;
        pEntry->nRow = nDocRow;

        nColMax = Max( nColMax, (USHORT)(pEntry->nCol + pEntry->nColOverlap) );
        nRowMax = Max( nRowMax, (USHORT)(pEntry->nRow + pEntry->nRowOverlap) );
    }
}


USHORT ScHTMLParser::GetWidth( ScEEParseEntry* pE )
{
    if( !bCalcWidthHeight )
        return 0;

    if ( pE->nWidth )
        return pE->nWidth;
    USHORT nPos = Min( pE->nCol - nColCntStart + pE->nColOverlap,
        pLocalColOffset->Count() - 1 );
    USHORT nOff2 = (USHORT) (*pLocalColOffset)[nPos];
    if ( pE->nOffset < nOff2 )
        return nOff2 - pE->nOffset;
    return 0;
}


void ScHTMLParser::SetWidths()
{
    if( !bCalcWidthHeight ) return;

    ScEEParseEntry* pE;
    USHORT nCol;
    if ( !nTableWidth )
        nTableWidth = (USHORT) aPageSize.Width();
    USHORT nColsPerRow = nMaxCol - nColCntStart;
    if ( !nColsPerRow )
        nColsPerRow = 1;
    if ( pLocalColOffset->Count() <= 2 )
    {   // nur PageSize, es gab keine Width-Angabe
        USHORT nWidth = nTableWidth / nColsPerRow;
        USHORT nOff = nColOffsetStart;
        pLocalColOffset->Remove( (USHORT)0, pLocalColOffset->Count() );
        for ( nCol = 0; nCol <= nColsPerRow; nCol++, nOff += nWidth )
        {
            MakeColNoRef( pLocalColOffset, nOff, 0, 0, 0 );
        }
        nTableWidth = (USHORT)((*pLocalColOffset)[pLocalColOffset->Count() -1 ] - (*pLocalColOffset)[0]);
        pE = pList->Seek( nFirstTableCell );
        while ( pE )
        {
            if ( pE->nTab == nTable )
            {
                pE->nOffset = (USHORT) (*pLocalColOffset)[pE->nCol - nColCntStart];
                pE->nWidth = 0;     // to be recalculated later
            }
            pE = pList->Next();
        }
    }
    else
    {   // einige mit einige ohne Width
        pE = pList->Seek( nFirstTableCell );
        // #36350# wieso eigentlich kein pE ?!?
        if ( pE )
        {
            USHORT* pOffsets = new USHORT[ nColsPerRow+1 ];
            memset( pOffsets, 0, (nColsPerRow+1) * sizeof(USHORT) );
            USHORT* pWidths = new USHORT[ nColsPerRow ];
            memset( pWidths, 0, nColsPerRow * sizeof(USHORT) );
            pOffsets[0] = nColOffsetStart;
            while ( pE )
            {
                if ( pE->nTab == nTable && pE->nWidth )
                {
                    nCol = pE->nCol - nColCntStart;
                    if ( nCol < nColsPerRow )
                    {
                        if ( pE->nColOverlap == 1 )
                        {
                            if ( pWidths[nCol] < pE->nWidth )
                                pWidths[nCol] = pE->nWidth;
                        }
                        else
                        {   // try to find a single undefined width
                            USHORT nTotal = 0;
                            BOOL bFound = FALSE;
                            USHORT nHere;
                            USHORT nStop = Min( USHORT(nCol + pE->nColOverlap), nColsPerRow );
                            for ( ; nCol < nStop; nCol++ )
                            {
                                if ( pWidths[nCol] )
                                    nTotal += pWidths[nCol];
                                else
                                {
                                    if ( bFound )
                                    {
                                        bFound = FALSE;
                                        break;  // for
                                    }
                                    bFound = TRUE;
                                    nHere = nCol;
                                }
                            }
                            if ( bFound && pE->nWidth > nTotal )
                                pWidths[nHere] = pE->nWidth - nTotal;
                        }
                    }
                }
                pE = pList->Next();
            }
            USHORT nWidths = 0;
            USHORT nUnknown = 0;
            for ( nCol = 0; nCol < nColsPerRow; nCol++ )
            {
                if ( pWidths[nCol] )
                    nWidths += pWidths[nCol];
                else
                    nUnknown++;
            }
            if ( nUnknown )
            {
                USHORT nW = ((nWidths < nTableWidth) ?
                    ((nTableWidth - nWidths) / nUnknown) :
                    (nTableWidth / nUnknown));
                for ( nCol = 0; nCol < nColsPerRow; nCol++ )
                {
                    if ( !pWidths[nCol] )
                        pWidths[nCol] = nW;
                }
            }
            for ( nCol = 1; nCol <= nColsPerRow; nCol++ )
            {
                pOffsets[nCol] = pOffsets[nCol-1] + pWidths[nCol-1];
            }
            pLocalColOffset->Remove( (USHORT)0, pLocalColOffset->Count() );
            for ( nCol = 0; nCol <= nColsPerRow; nCol++ )
            {
                MakeColNoRef( pLocalColOffset, pOffsets[nCol], 0, 0, 0 );
            }
            nTableWidth = pOffsets[nColsPerRow] - pOffsets[0];

            pE = pList->Seek( nFirstTableCell );
            while ( pE )
            {
                if ( pE->nTab == nTable )
                {
                    nCol = pE->nCol - nColCntStart;
                    DBG_ASSERT( nCol < nColsPerRow, "ScHTMLParser::SetWidths: column overflow" );
                    if ( nCol < nColsPerRow )
                    {
                        pE->nOffset = pOffsets[nCol];
                        nCol += pE->nColOverlap;
                        if ( nCol > nColsPerRow )
                            nCol = nColsPerRow;
                        pE->nWidth = pOffsets[nCol] - pE->nOffset;
                    }
                }
                pE = pList->Next();
            }

            delete [] pWidths;
            delete [] pOffsets;
        }
    }
    if ( pLocalColOffset->Count() )
    {
        USHORT nMax = (USHORT) (*pLocalColOffset)[pLocalColOffset->Count() - 1];
        if ( aPageSize.Width() < nMax )
            aPageSize.Width() = nMax;
    }
    pE = pList->Seek( nFirstTableCell );
    while ( pE )
    {
        if ( pE->nTab == nTable )
        {
            if ( !pE->nWidth )
            {
                pE->nWidth = GetWidth( pE );
                DBG_ASSERT( pE->nWidth, "SetWidths: pE->nWidth == 0" );
            }
            MakeCol( pColOffset, pE->nOffset, pE->nWidth );
        }
        pE = pList->Next();
    }
}


void ScHTMLParser::Colonize( ScEEParseEntry* pE )
{
    USHORT nCol = pE->nCol;
    SkipLocked( pE );       // Spaltenverdraengung nach rechts

    if( bCalcWidthHeight )
    {
        if ( nCol < pE->nCol )
        {   // verdraengt
            nCol = pE->nCol - nColCntStart;
            USHORT nCount = pLocalColOffset->Count();
            if ( nCol < nCount )
                nColOffset = (USHORT) (*pLocalColOffset)[nCol];
            else
                nColOffset = (USHORT) (*pLocalColOffset)[nCount - 1];
        }
        pE->nOffset = nColOffset;
        USHORT nWidth = GetWidth( pE );
        MakeCol( pLocalColOffset, pE->nOffset, nWidth );
        if ( pE->nWidth )
            pE->nWidth = nWidth;
        nColOffset = pE->nOffset + nWidth;
        if ( nTableWidth < nColOffset - nColOffsetStart )
            nTableWidth = nColOffset - nColOffsetStart;
    }
}


void ScHTMLParser::CloseEntry( ImportInfo* pInfo )
{
    bInCell = FALSE;
    if ( bTabInTabCell )
    {   // in TableOff vom Stack geholt
        bTabInTabCell = FALSE;
        if ( pList->GetPos( pActEntry ) == LIST_ENTRY_NOTFOUND )
            delete pActEntry;
        NewActEntry( pList->Last() );   // neuer freifliegender pActEntry
        return ;
    }
    if ( pActEntry->nCol == (USHORT)~0 )
        pActEntry->nCol = nColCnt;
    if ( pActEntry->nRow == (USHORT)~0 )
        pActEntry->nRow = nRowCnt;
    if ( pActEntry->nTab == 0 )
        pActEntry->nWidth = (USHORT) aPageSize.Width();
    Colonize( pActEntry );
    nColCnt = pActEntry->nCol + pActEntry->nColOverlap;
    if ( nMaxCol < nColCnt )
        nMaxCol = nColCnt;          // TableStack MaxCol
    if ( nColMax < nColCnt )
        nColMax = nColCnt;      // globales MaxCol fuer ScEEParser GetDimensions!
    EntryEnd( pActEntry, pInfo->aSelection );
    ESelection& rSel = pActEntry->aSel;
    while ( rSel.nStartPara < rSel.nEndPara
            && pEdit->GetTextLen( rSel.nStartPara ) == 0 )
    {   // vorgehaengte Leerabsaetze strippen
        rSel.nStartPara++;
    }
    while ( rSel.nEndPos == 0 && rSel.nEndPara > rSel.nStartPara )
    {   // angehaengte Leerabsaetze strippen
        rSel.nEndPara--;
        rSel.nEndPos = pEdit->GetTextLen( rSel.nEndPara );
    }
    if ( rSel.nStartPara > rSel.nEndPara )
    {   // gibt GPF in CreateTextObject
        DBG_ERRORFILE( "CloseEntry: EditEngine ESelection Start > End" );
        rSel.nEndPara = rSel.nStartPara;
    }
    if ( rSel.HasRange() && bCalcWidthHeight )
        pActEntry->aItemSet.Put( SfxBoolItem( ATTR_LINEBREAK, TRUE ) );
    pList->Insert( pActEntry, LIST_APPEND );
    NewActEntry( pActEntry );   // neuer freifliegender pActEntry
}


IMPL_LINK( ScHTMLParser, HTMLImportHdl, ImportInfo*, pInfo )
{
#if defined(erDEBUG) //|| 1
    static ESelection aDebugSel;
    static String aDebugStr;
    static SvFileStream* pDebugStrm = NULL;
    static ULONG nDebugStrmPos = 0;
    static ULONG nDebugCount = 0;
    static ULONG nDebugCountAll = 0;
    static const sal_Char* sDebugState[15] = {
                    "RTFIMP_START", "RTFIMP_END",
                    "RTFIMP_NEXTTOKEN", "RTFIMP_UNKNOWNATTR",
                    "RTFIMP_SETATTR",
                    "RTFIMP_INSERTTEXT",
                    "RTFIMP_INSERTPARA",
                    "HTMLIMP_START", "HTMLIMP_END",
                    "HTMLIMP_NEXTTOKEN", "HTMLIMP_UNKNOWNATTR",
                    "HTMLIMP_SETATTR",
                    "HTMLIMP_INSERTTEXT",
                    "HTMLIMP_INSERTPARA", "HTMLIMP_INSERTFIELD"
    };

    nDebugCountAll++;
    if ( pInfo->eState != HTMLIMP_NEXTTOKEN     // not too much
      || pInfo->nToken == HTML_TABLE_ON
      || pInfo->nToken == HTML_TABLE_OFF
      || pInfo->nToken == HTML_TABLEROW_ON
      || pInfo->nToken == HTML_TABLEROW_OFF
      || pInfo->nToken == HTML_TABLEHEADER_ON
      || pInfo->nToken == HTML_TABLEHEADER_OFF
      || pInfo->nToken == HTML_TABLEDATA_ON
      || pInfo->nToken == HTML_TABLEDATA_OFF
      || !aDebugSel.IsEqual( pInfo->aSelection )
      || pInfo->aText.Len() || aDebugStr != pInfo->aText
        )
    {
        aDebugSel = pInfo->aSelection;
        aDebugStr = pInfo->aText;
        nDebugCount++;
        if ( !pDebugStrm )
        {
            pDebugStrm = new SvFileStream( "d:\\erdbghtm.log",
                STREAM_WRITE | STREAM_TRUNC );
        }
        else
        {
            pDebugStrm->ReOpen();
            pDebugStrm->Seek( nDebugStrmPos );
        }
        SvFileStream& rS = *pDebugStrm;
        rS.WriteNumber( nDebugCountAll ); rS << ".: ";
        rS.WriteNumber( nDebugCount ); rS << ". State: ";
        rS.WriteNumber( (USHORT) pInfo->eState );
        rS << ' ' << sDebugState[pInfo->eState] << endl;
        rS << "SPar,SPos EPar,EPos: ";
        rS.WriteNumber( aDebugSel.nStartPara ); rS << ',';
        rS.WriteNumber( aDebugSel.nStartPos ); rS << ' ';
        rS.WriteNumber( aDebugSel.nEndPara ); rS << ',';
        rS.WriteNumber( aDebugSel.nEndPos ); rS << endl;
        if ( aDebugStr.Len() )
        {
            rS << "Text: \"" << aDebugStr << '\"' << endl;
        }
        else
        {
            rS << "Text:" << endl;
        }
        rS << "Token: "; rS.WriteNumber( pInfo->nToken );
        switch ( pInfo->nToken )
        {
            case HTML_TABLE_ON:
                rS << " HTML_TABLE_ON";
                break;
            case HTML_TABLE_OFF:
                rS << " HTML_TABLE_OFF";
                break;
            case HTML_TABLEROW_ON:
                rS << " HTML_TABLEROW_ON";
                break;
            case HTML_TABLEROW_OFF:
                rS << " HTML_TABLEROW_OFF";
                break;
            case HTML_TABLEHEADER_ON:
                rS << " HTML_TABLEHEADER_ON";
                break;
            case HTML_TABLEHEADER_OFF:
                rS << " HTML_TABLEHEADER_OFF";
                break;
            case HTML_TABLEDATA_ON:
                rS << " HTML_TABLEDATA_ON";
                break;
            case HTML_TABLEDATA_OFF:
                rS << " HTML_TABLEDATA_OFF";
                break;
        }
        rS << " Value: "; rS.WriteNumber( pInfo->nTokenValue );
        rS << endl << endl;
        nDebugStrmPos = pDebugStrm->Tell();
        pDebugStrm->Close();
    }
#endif
    switch ( pInfo->eState )
    {
        case HTMLIMP_NEXTTOKEN:
            ProcToken( pInfo );
            break;
        case HTMLIMP_UNKNOWNATTR:
            ProcToken( pInfo );
            break;
        case HTMLIMP_START:
            break;
        case HTMLIMP_END:
            if ( pInfo->aSelection.nEndPos )
            {   // falls noch Text: letzten Absatz erzeugen
                // Zeile weiterschalten ohne CloseEntry aufzurufen
                bInCell = FALSE;
                NextRow( pInfo );
                bInCell = TRUE;
                CloseEntry( pInfo );
            }
            while ( nTableLevel > 0 )
                TableOff( pInfo );      // vergessene </TABLE> ausbuegeln
            break;
        case HTMLIMP_SETATTR:
            break;
        case HTMLIMP_INSERTTEXT:
            break;
        case HTMLIMP_INSERTPARA:
            if ( nTableLevel < 1 )
            {
                CloseEntry( pInfo );
                NextRow( pInfo );
            }
            break;
        case HTMLIMP_INSERTFIELD:
            break;
        default:
            DBG_ERRORFILE("HTMLImportHdl: unknown ImportInfo.eState");
    }
    return 0;
}


// Groesster Gemeinsamer Teiler nach Euklid (Kettendivision)
// Sonderfall: 0 und irgendwas geben 1
USHORT lcl_GGT( USHORT a, USHORT b )
{
    if ( !a || !b )
        return 1;
    do
    {
        if ( a > b )
            a -= USHORT(a / b) * b;
        else
            b -= USHORT(b / a) * a;
    } while ( a && b );
    return (a ? a : b);
}


// Kleinstes Gemeinsames Vielfaches: a * b / GGT(a,b)
USHORT lcl_KGV( USHORT a, USHORT b )
{
    if ( a > b )    // Ueberlauf unwahrscheinlicher machen
        return (a / lcl_GGT(a,b)) * b;
    else
        return (b / lcl_GGT(a,b)) * a;
}


void ScHTMLParser::TableDataOn( ImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( !nTableLevel )
    {
        DBG_ERROR( "Dummbatz-Dok! <TH> oder <TD> ohne vorheriges <TABLE>" );
        TableOn( pInfo );
    }
    bInCell = TRUE;
    BOOL bHorJustifyCenterTH = (pInfo->nToken == HTML_TABLEHEADER_ON);
    const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_COLSPAN:
            {
                pActEntry->nColOverlap = ( USHORT ) pOption->GetString().ToInt32();
            }
            break;
            case HTML_O_ROWSPAN:
            {
                pActEntry->nRowOverlap = ( USHORT ) pOption->GetString().ToInt32();
            }
            break;
            case HTML_O_ALIGN:
            {
                bHorJustifyCenterTH = FALSE;
                SvxCellHorJustify eVal;
                const String& rOptVal = pOption->GetString();
                if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_AL_right ) == COMPARE_EQUAL )
                    eVal = SVX_HOR_JUSTIFY_RIGHT;
                else if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_AL_center ) == COMPARE_EQUAL )
                    eVal = SVX_HOR_JUSTIFY_CENTER;
                else if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_AL_left ) == COMPARE_EQUAL )
                    eVal = SVX_HOR_JUSTIFY_LEFT;
                else
                    eVal = SVX_HOR_JUSTIFY_STANDARD;
                if ( eVal != SVX_HOR_JUSTIFY_STANDARD )
                    pActEntry->aItemSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY) );
            }
            break;
            case HTML_O_VALIGN:
            {
                SvxCellVerJustify eVal;
                const String& rOptVal = pOption->GetString();
                if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_VA_top ) == COMPARE_EQUAL )
                    eVal = SVX_VER_JUSTIFY_TOP;
                else if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_VA_middle ) == COMPARE_EQUAL )
                    eVal = SVX_VER_JUSTIFY_CENTER;
                else if ( rOptVal.CompareIgnoreCaseToAscii( sHTML_VA_bottom ) == COMPARE_EQUAL )
                    eVal = SVX_VER_JUSTIFY_BOTTOM;
                else
                    eVal = SVX_VER_JUSTIFY_STANDARD;
                pActEntry->aItemSet.Put( SvxVerJustifyItem( eVal, ATTR_VER_JUSTIFY) );
            }
            break;
            case HTML_O_WIDTH:
            {
                pActEntry->nWidth = GetWidthPixel( pOption );
            }
            break;
            case HTML_O_BGCOLOR:
            {
                Color aColor;
                pOption->GetColor( aColor );
                pActEntry->aItemSet.Put(
                    SvxBrushItem( aColor, ATTR_BACKGROUND ) );
            }
            break;
            case HTML_O_SDVAL:
            {
                pActEntry->pValStr = new String( pOption->GetString() );
            }
            break;
            case HTML_O_SDNUM:
            {
                pActEntry->pNumStr = new String( pOption->GetString() );
            }
            break;
        }
    }
    pActEntry->nCol = nColCnt;
    pActEntry->nRow = nRowCnt;
    pActEntry->nTab = nTable;

    if( pTableData )
    {
        SkipLocked( pActEntry, FALSE );
        nColCnt = pActEntry->nCol;
        nRowCnt = pActEntry->nRow;
        pTableData->SetCellCoord( nTable, pActEntry->nCol, pActEntry->nRow );
    }

    if ( bHorJustifyCenterTH )
        pActEntry->aItemSet.Put(
            SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY) );
}


void ScHTMLParser::TableOn( ImportInfo* pInfo )
{
    String aTabName;
    if ( ++nTableLevel > 1 )
    {   // Table in Table
        USHORT nTmpColOffset = nColOffset;  // wird in Colonize noch angepasst
        Colonize( pActEntry );
        aTableStack.Push( new ScHTMLTableStackEntry(
            pActEntry, xLockedList, pLocalColOffset, nFirstTableCell,
            nColCnt, nRowCnt, nColCntStart, nMaxCol, nTable,
            nTableWidth, nColOffset, nColOffsetStart,
            bFirstRow ) );
        USHORT nLastWidth = nTableWidth;
        nTableWidth = GetWidth( pActEntry );
        if ( nTableWidth == nLastWidth && nMaxCol - nColCntStart > 1 )
        {   // es muss mehr als einen geben, also kann dieser nicht alles sein
            nTableWidth = nLastWidth / (nMaxCol - nColCntStart);
        }
        nLastWidth = nTableWidth;
        if ( pInfo->nToken == HTML_TABLE_ON )
        {   // es kann auch TD oder TH sein, wenn es vorher kein TABLE gab
            const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
            USHORT nArrLen = pOptions->Count();
            for ( USHORT i = 0; i < nArrLen; i++ )
            {
                const HTMLOption* pOption = (*pOptions)[i];
                switch( pOption->GetToken() )
                {
                    case HTML_O_WIDTH:
                    {   // Prozent: von Dokumentbreite bzw. aeusserer Zelle
                        nTableWidth = GetWidthPixel( pOption );
                    }
                    break;
                    case HTML_O_ID:
                    {
                        aTabName.Assign( pOption->GetString() );
                    }
                    break;
                }
            }
        }
        bInCell = FALSE;
        if ( bTabInTabCell && !(nTableWidth < nLastWidth) )
        {   // mehrere Tabellen in einer Zelle, untereinander
            bTabInTabCell = FALSE;
            NextRow( pInfo );
        }
        else
        {   // in dieser Zelle geht's los, oder nebeneinander
            bTabInTabCell = FALSE;
            nColCntStart = nColCnt;
            nColOffset = nTmpColOffset;
            nColOffsetStart = nColOffset;
        }

        if( pTableData )
            pTableData->InsertTable( nMaxTable + 1, aTabName, pActEntry->nCol, pActEntry->nRow,
                pActEntry->nColOverlap, pActEntry->nRowOverlap, nTable );

        ScEEParseEntry* pE = pList->Last();
        NewActEntry( pE );      // neuer freifliegender pActEntry
        xLockedList = new ScRangeList;
    }
    else
    {   // einfache Table auf Dokumentebene
        EntryEnd( pActEntry, pInfo->aSelection );
        if ( pActEntry->aSel.HasRange() )
        {   // noch fliegender Text
            CloseEntry( pInfo );
            NextRow( pInfo );
        }
        aTableStack.Push( new ScHTMLTableStackEntry(
            pActEntry, xLockedList, pLocalColOffset, nFirstTableCell,
            nColCnt, nRowCnt, nColCntStart, nMaxCol, nTable,
            nTableWidth, nColOffset, nColOffsetStart,
            bFirstRow ) );
        nTableWidth = 0;
        if ( pInfo->nToken == HTML_TABLE_ON )
        {   // es kann auch TD oder TH sein, wenn es vorher kein TABLE gab
            const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
            USHORT nArrLen = pOptions->Count();
            for ( USHORT i = 0; i < nArrLen; i++ )
            {
                const HTMLOption* pOption = (*pOptions)[i];
                switch( pOption->GetToken() )
                {
                    case HTML_O_WIDTH:
                    {   // Prozent: von Dokumentbreite bzw. aeusserer Zelle
                        nTableWidth = GetWidthPixel( pOption );
                    }
                    break;
                    case HTML_O_ID:
                    {
                        aTabName.Assign( pOption->GetString() );
                    }
                    break;
                }
            }
        }

        if( pTableData )
            pTableData->InsertTable( nMaxTable + 1, aTabName, nColCnt, nRowCnt, 1, 1 );
    }
    nTable = ++nMaxTable;
    bFirstRow = TRUE;
    nFirstTableCell = pList->Count();

    if( bCalcWidthHeight )
    {
        pLocalColOffset = new ScHTMLColOffset;
        MakeColNoRef( pLocalColOffset, nColOffsetStart, 0, 0, 0 );
    }
}


void ScHTMLParser::TableOff( ImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( nColCnt > nColCntStart )
        NextRow( pInfo );       // das optionale TableRowOff war nicht
    if ( !nTableLevel )
    {
        DBG_ERROR( "Dummbatz-Dok! </TABLE> ohne oeffnendes <TABLE>" );
        return ;
    }
    if ( --nTableLevel > 0 )
    {   // Table in Table beendet
        ScHTMLTableStackEntry* pS = aTableStack.Pop();
        if ( pS )
        {
            ScEEParseEntry* pE = pS->pCellEntry;
            USHORT nRows = nRowCnt - pS->nRowCnt;
            if ( nRows > 1 && bCalcWidthHeight )
            {   // Groesse der Tabelle an dieser Position eintragen
                USHORT j;
                USHORT nCol = nColCntStart;
                USHORT nRow = pS->nRowCnt;
                USHORT nTab = pS->nTable;
                if ( !pTables )
                    pTables = new Table;
                // Hoehen der aeusseren Table
                Table* pTab1 = (Table*) pTables->Get( nTab );
                if ( !pTab1 )
                {
                    pTab1 = new Table;
                    pTables->Insert( nTab, pTab1 );
                }
                USHORT nRowSpan = pE->nRowOverlap;
                USHORT nRowKGV;
                USHORT nRowsPerRow1;    // aeussere Table
                USHORT nRowsPerRow2;    // innere Table
                if ( nRowSpan > 1 )
                {   // KGV auf das sich aussere und innere Zeilen
                    // abbilden lassen
                    nRowKGV = lcl_KGV( nRowSpan, nRows );
                    nRowsPerRow1 = nRowKGV / nRowSpan;
                    nRowsPerRow2 = nRowKGV / nRows;
                }
                else
                {
                    nRowKGV = nRowsPerRow1 = nRows;
                    nRowsPerRow2 = 1;
                }
                Table* pTab2 = NULL;
                if ( nRowsPerRow2 > 1 )
                {   // Hoehen der inneren Table
                    pTab2 = new Table;
                    pTables->Insert( nTable, pTab2 );
                }
                // void* Data-Entry der Table-Class fuer das
                // Hoehen-Mapping missbrauchen
                if ( nRowKGV > 1 )
                {
                    if ( nRowsPerRow1 > 1 )
                    {   // aussen
                        for ( j=0; j < nRowSpan; j++ )
                        {
                            ULONG nRowKey = nRow + j;
                            USHORT nR = (USHORT)(ULONG) pTab1->Get( nRowKey );
                            if ( !nR )
                                pTab1->Insert( nRowKey, (void*) nRowsPerRow1 );
                            else if ( nRowsPerRow1 > nR )
                                pTab1->Replace( nRowKey, (void*) nRowsPerRow1 );
                                //2do: wie geht das noch besser?
                            else if ( nRowsPerRow1 < nR && nRowSpan == 1
                              && nTable == nMaxTable )
                            {   // Platz uebrig, evtl. besser mergen
                                USHORT nAdd = nRowsPerRow1 - (nR % nRowsPerRow1);
                                nR += nAdd;
                                if ( (nR % nRows) == 0 )
                                {   // nur wenn abbildbar
                                    USHORT nR2 = (USHORT)(ULONG) pTab1->Get( nRowKey+1 );
                                    if ( nR2 > nAdd )
                                    {   // nur wenn wirklich Platz
                                        pTab1->Replace( nRowKey, (void*) nR );
                                        pTab1->Replace( nRowKey+1, (void*) (nR2 - nAdd) );
                                        nRowsPerRow2 = nR / nRows;
                                    }
                                }
                            }
                        }
                    }
                    if ( nRowsPerRow2 > 1 )
                    {   // innen
                        if ( !pTab2 )
                        {   // nRowsPerRow2 kann erhoeht worden sein
                            pTab2 = new Table;
                            pTables->Insert( nTable, pTab2 );
                        }
                        for ( j=0; j < nRows; j++ )
                        {
                            ULONG nRowKey = nRow + j;
                            USHORT nR = (USHORT)(ULONG) pTab2->Get( nRowKey );
                            if ( !nR )
                                pTab2->Insert( nRowKey, (void*) nRowsPerRow2 );
                            else if ( nRowsPerRow2 > nR )
                                pTab2->Replace( nRowKey, (void*) nRowsPerRow2 );
                        }
                    }
                }
            }

            if( bCalcWidthHeight )
            {
                SetWidths();

                if ( !pE->nWidth )
                    pE->nWidth = nTableWidth;
                else if ( pE->nWidth < nTableWidth )
                {
                    USHORT nOldOffset = pE->nOffset + pE->nWidth;
                    USHORT nNewOffset = pE->nOffset + nTableWidth;
                    ModifyOffset( pS->pLocalColOffset, nOldOffset, nNewOffset );
                    USHORT nTmp = nNewOffset - pE->nOffset - pE->nWidth;
                    pE->nWidth = nNewOffset - pE->nOffset;
                    pS->nTableWidth += nTmp;
                    if ( pS->nColOffset >= nOldOffset )
                        pS->nColOffset += nTmp;
                }
            }

            nColCnt = pE->nCol + pE->nColOverlap;
            nRowCnt = pS->nRowCnt;
            nColCntStart = pS->nColCntStart;
            nMaxCol = pS->nMaxCol;
            nTable = pS->nTable;
            nTableWidth = pS->nTableWidth;
            nFirstTableCell = pS->nFirstTableCell;
            nColOffset = pS->nColOffset;
            nColOffsetStart = pS->nColOffsetStart;
            bFirstRow = pS->bFirstRow;
            xLockedList = pS->xLockedList;
            if ( pLocalColOffset )
                delete pLocalColOffset;
            pLocalColOffset = pS->pLocalColOffset;
            delete pActEntry;
            // pActEntry bleibt erstmal erhalten falls da noch 'ne Table in
            // der gleichen Zelle aufgemacht werden sollte (in HTML ist ja
            // alles moeglich..) und wird in CloseEntry deleted
            pActEntry = pE;
            delete pS;
        }
        bTabInTabCell = TRUE;
        bInCell = TRUE;
    }
    else
    {   // einfache Table beendet
        SetWidths();
        ScHTMLTableStackEntry* pS = aTableStack.Pop();
        nMaxCol = 0;
        nTable = 0;
        if ( pS )
        {
            if ( pLocalColOffset )
                delete pLocalColOffset;
            pLocalColOffset = pS->pLocalColOffset;
            delete pS;
        }
    }
}


void ScHTMLParser::Image( ImportInfo* pInfo )
{
    if( !bCalcWidthHeight ) return;

    if ( !pActEntry->pImageList )
        pActEntry->pImageList = new ScHTMLImageList;
    ScHTMLImageList* pIL = pActEntry->pImageList;
    ScHTMLImage* pImage = new ScHTMLImage;
    pIL->Insert( pImage, LIST_APPEND );
    const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_SRC:
            {
                pImage->aURL = INetURLObject::RelToAbs( pOption->GetString() );
            }
            break;
            case HTML_O_ALT:
            {
                if ( !pActEntry->bHasGraphic )
                {   // ALT text only if not any image loaded
                    if ( pActEntry->aAltText.Len() )
                        pActEntry->aAltText.AppendAscii( "; " );
                    pActEntry->aAltText += pOption->GetString();
                }
            }
            break;
            case HTML_O_WIDTH:
            {
                pImage->aSize.Width() = (long)pOption->GetNumber();
            }
            break;
            case HTML_O_HEIGHT:
            {
                pImage->aSize.Height() = (long)pOption->GetNumber();
            }
            break;
            case HTML_O_HSPACE:
            {
                pImage->aSpace.X() = (long)pOption->GetNumber();
            }
            break;
            case HTML_O_VSPACE:
            {
                pImage->aSpace.Y() = (long)pOption->GetNumber();
            }
            break;
        }
    }
    if ( !pImage->aURL.Len() )
    {
        DBG_ERRORFILE( "Image: Grafik ohne URL ?!?" );
        return ;
    }

    USHORT nFormat;
    Graphic* pGraphic = new Graphic;
    GraphicFilter* pFilter = ::GetGrfFilter();
    if ( GRFILTER_OK != ::LoadGraphic( pImage->aURL, pImage->aFilterName,
            *pGraphic, pFilter, &nFormat ) )
    {
        delete pGraphic;
        return ;        // dumm gelaufen
    }
    if ( !pActEntry->bHasGraphic )
    {   // discard any ALT text in this cell if we have any image
        pActEntry->bHasGraphic = TRUE;
        pActEntry->aAltText.Erase();
    }
    pImage->aFilterName = pFilter->GetImportFormatName( nFormat );
    pImage->pGraphic = pGraphic;
    if ( !(pImage->aSize.Width() && pImage->aSize.Height()) )
    {
        OutputDevice* pDefaultDev = Application::GetDefaultDevice();
        pImage->aSize = pDefaultDev->LogicToPixel( pGraphic->GetPrefSize(),
            pGraphic->GetPrefMapMode() );
    }
    if ( pIL->Count() > 0 )
    {
        long nWidth = 0;
        for ( ScHTMLImage* pI = pIL->First(); pI; pI = pIL->Next() )
        {
            if ( pI->nDir & nHorizontal )
                nWidth += pI->aSize.Width() + 2 * pI->aSpace.X();
            else
                nWidth = 0;
        }
        if ( pActEntry->nWidth
          && (nWidth + pImage->aSize.Width() + 2 * pImage->aSpace.X()
                >= pActEntry->nWidth) )
            pIL->Last()->nDir = nVertical;
    }
}


void ScHTMLParser::ColOn( ImportInfo* pInfo )
{
    if( !bCalcWidthHeight ) return;

    const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_WIDTH:
            {
                USHORT nVal = GetWidthPixel( pOption );
                MakeCol( pLocalColOffset, nColOffset, nVal, 0, 0 );
                nColOffset += nVal;
            }
            break;
        }
    }
}


USHORT ScHTMLParser::GetWidthPixel( const HTMLOption* pOption )
{
    if( !bCalcWidthHeight )
        return 0;

    const String& rOptVal = pOption->GetString();
    if ( rOptVal.Search('%') != STRING_NOTFOUND )
    {   // Prozent
        USHORT nW = (nTableWidth ? nTableWidth : (USHORT) aPageSize.Width());
        return (USHORT)((pOption->GetNumber() * nW) / 100);
    }
    else
    {
        if ( rOptVal.Search('*') != STRING_NOTFOUND )
        {   // relativ zu was?!?
//2do: ColArray aller relativen Werte sammeln und dann MakeCol
            return 0;
        }
        else
            return (USHORT)pOption->GetNumber();    // Pixel
    }
}


void ScHTMLParser::AnchorOn( ImportInfo* pInfo )
{
    const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
    USHORT nArrLen = pOptions->Count();
    for ( USHORT i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_NAME:
            {
                pActEntry->pName = new String( pOption->GetString() );
            }
            break;
        }
    }
}


BOOL ScHTMLParser::IsAtBeginningOfText( ImportInfo* pInfo )
{
    ESelection& rSel = pActEntry->aSel;
    return rSel.nStartPara == rSel.nEndPara &&
        rSel.nStartPara <= pInfo->aSelection.nEndPara &&
        pEdit->GetTextLen( rSel.nStartPara ) == 0;
}


void ScHTMLParser::FontOn( ImportInfo* pInfo )
{
    if ( IsAtBeginningOfText( pInfo ) )
    {   // nur am Anfang des Textes, gilt dann fuer gesamte Zelle
        const HTMLOptions* pOptions = ((HTMLParser*)pInfo->pParser)->GetOptions();
        USHORT nArrLen = pOptions->Count();
        for ( USHORT i = 0; i < nArrLen; i++ )
        {
            const HTMLOption* pOption = (*pOptions)[i];
            switch( pOption->GetToken() )
            {
                case HTML_O_FACE :
                {
                    const String& rFace = pOption->GetString();
                    String aFontName;
                    xub_StrLen nPos = 0;
                    while( nPos != STRING_NOTFOUND )
                    {   // Fontliste, VCL: Semikolon als Separator, HTML: Komma
                        String aFName = rFace.GetToken( 0, ',', nPos );
                        aFName.EraseTrailingChars().EraseLeadingChars();
                        if( aFontName.Len() )
                            aFontName += ';';
                        aFontName += aFName;
                    }
                    if ( aFontName.Len() )
                        pActEntry->aItemSet.Put( SvxFontItem( FAMILY_DONTKNOW,
                            aFontName, EMPTY_STRING, PITCH_DONTKNOW,
                            RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );
                }
                break;
                case HTML_O_SIZE :
                {
                    USHORT nSize = (USHORT) pOption->GetNumber();
                    if ( nSize == 0 )
                        nSize = 1;
                    else if ( nSize > SC_HTML_FONTSIZES )
                        nSize = SC_HTML_FONTSIZES;
                    pActEntry->aItemSet.Put( SvxFontHeightItem(
                        aFontHeights[nSize-1], 100, ATTR_FONT_HEIGHT ) );
                }
                break;
                case HTML_O_COLOR :
                {
                    Color aColor;
                    pOption->GetColor( aColor );
                    pActEntry->aItemSet.Put( SvxColorItem( aColor, ATTR_FONT_COLOR ) );
                }
                break;
            }
        }
    }
}


void ScHTMLParser::ProcToken( ImportInfo* pInfo )
{
#ifdef DEBUG
    HTML_TOKEN_IDS eTokenId = (HTML_TOKEN_IDS)pInfo->nToken;
#endif
    BOOL bSetLastToken = TRUE;
    switch ( pInfo->nToken )
    {
        case HTML_META:
        {
            USHORT nContentOpt = HTML_O_CONTENT;
            rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;
            HTMLParser* pParser = (HTMLParser*) pInfo->pParser;
            const HTMLOptions* pOptions = pParser->GetOptions( &nContentOpt );
            SfxFrameHTMLParser::ParseMetaOptions(
                &pDoc->GetDocumentShell()->GetDocInfo(),
                pDoc->GetDocumentShell()->GetHeaderAttributes(),
                pOptions, nMetaCnt, eEnc );
            // If the encoding is set by a META tag, it may only overwrite the
            // current encoding if both, the current and the new encoding, are 1-BYTE
            // encodings. Everything else cannot lead to reasonable results.
            if( RTL_TEXTENCODING_DONTKNOW != eEnc &&
                    eEnc < RTL_TEXTENCODING_STD_COUNT &&
                    pParser->GetSrcEncoding() < RTL_TEXTENCODING_STD_COUNT )
                pParser->SetSrcEncoding( eEnc );
        }
        break;
        case HTML_TITLE_ON:
        {
            bInTitle = TRUE;
            aString.Erase();
        }
        break;
        case HTML_TITLE_OFF:
        {
            if ( bInTitle && aString.Len() )
            {
                // Leerzeichen von Zeilenumbruechen raus
                aString.EraseLeadingChars();
                aString.EraseTrailingChars();
                pDoc->GetDocumentShell()->GetDocInfo().SetTitle( aString );
            }
            bInTitle = FALSE;
        }
        break;
        case HTML_TABLE_ON:
        {
            TableOn( pInfo );
        }
        break;
        case HTML_COL_ON:
        {
            ColOn( pInfo );
        }
        break;
        case HTML_TABLEHEADER_ON:       // oeffnet Zelle
        {
            if ( bInCell )
                CloseEntry( pInfo );
            // bInCell nicht TRUE setzen, das macht TableDataOn
            pActEntry->aItemSet.Put(
                SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT) );
        }   // fall thru
        case HTML_TABLEDATA_ON:         // oeffnet Zelle
        {
            TableDataOn( pInfo );
        }
        break;
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:        // schliesst Zelle
        {
            if ( bInCell )
                CloseEntry( pInfo );    // aber nur wenn's auch eine war
        }
        break;
        case HTML_TABLEROW_ON:          // vor erster Zelle in Row
        {
            if ( nColCnt > nColCntStart )
                NextRow( pInfo );       // das optionale TableRowOff war nicht
            nColOffset = nColOffsetStart;
        }
        break;
        case HTML_TABLEROW_OFF:         // nach letzter Zelle in Row
        {
            NextRow( pInfo );
        }
        break;
        case HTML_TABLE_OFF:
        {
            TableOff( pInfo );
        }
        break;
        case HTML_IMAGE:
        {
            Image( pInfo );
        }
        break;
        case HTML_PARABREAK_OFF:
        {   // nach einem Image geht es vertikal weiter
            if ( pActEntry->pImageList && pActEntry->pImageList->Count() > 0 )
                pActEntry->pImageList->Last()->nDir = nVertical;
        }
        break;
        case HTML_ANCHOR_ON:
        {
            AnchorOn( pInfo );
        }
        break;
        case HTML_FONT_ON :
        {
            FontOn( pInfo );
        }
        break;
        case HTML_BIGPRINT_ON :
        {
//2do: aktuelle Fontgroesse merken und einen groesser
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxFontHeightItem(
                    aFontHeights[3], 100, ATTR_FONT_HEIGHT ) );
        }
        break;
        case HTML_SMALLPRINT_ON :
        {
//2do: aktuelle Fontgroesse merken und einen kleiner
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxFontHeightItem(
                    aFontHeights[0], 100, ATTR_FONT_HEIGHT ) );
        }
        break;
        case HTML_BOLD_ON :
        case HTML_STRONG_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxWeightItem( WEIGHT_BOLD,
                    ATTR_FONT_WEIGHT ) );
        }
        break;
        case HTML_ITALIC_ON :
        case HTML_EMPHASIS_ON :
        case HTML_ADDRESS_ON :
        case HTML_BLOCKQUOTE_ON :
        case HTML_BLOCKQUOTE30_ON :
        case HTML_CITIATION_ON :
        case HTML_VARIABLE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxPostureItem( ITALIC_NORMAL,
                    ATTR_FONT_POSTURE ) );
        }
        break;
        case HTML_DEFINSTANCE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
            {
                pActEntry->aItemSet.Put( SvxWeightItem( WEIGHT_BOLD,
                    ATTR_FONT_WEIGHT ) );
                pActEntry->aItemSet.Put( SvxPostureItem( ITALIC_NORMAL,
                    ATTR_FONT_POSTURE ) );
            }
        }
        break;
        case HTML_UNDERLINE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE,
                    ATTR_FONT_UNDERLINE ) );
        }
        break;
        case HTML_TEXTTOKEN:
        {
            if ( bInTitle )
                aString += pInfo->aText;
        }
        break;
        default:
        {   // nLastToken nicht setzen!
            bSetLastToken = FALSE;
        }
    }
    if ( bSetLastToken )
        nLastToken = pInfo->nToken;
}



