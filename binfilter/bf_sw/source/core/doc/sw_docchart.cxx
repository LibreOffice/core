/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <float.h>


#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <vcl/window.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>

#include <errhdl.hxx>

#include <ndindex.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <viewsh.hxx>
#include <ndole.hxx>
#include <cntfrm.hxx>
#include <swtblfmt.hxx>
#include <tblsel.hxx>
#include <cellatr.hxx>
namespace binfilter {


/*N*/ SchMemChart *SwTable::UpdateData( SchMemChart* pData,
/*N*/ 								const String* pSelection ) const
/*N*/ {
/*N*/ 	SwCalc aCalc( *GetFrmFmt()->GetDoc() );
/*N*/ 	SwTblCalcPara aCalcPara( aCalc, *this );
/*N*/ 	String sSelection, sRowColInfo;
/*N*/ 	BOOL bSetChartRange = TRUE;
/*N*/ 
/*N*/ 	// worauf bezieht sich das Chart?
/*N*/ 	if( pData && pData->SomeData1().Len() )
/*N*/ 	{
/*?*/ 		sSelection = pData->SomeData1();
/*?*/ 		sRowColInfo = pData->SomeData2();
/*N*/ 	}
/*N*/ 	else if( pData && pData->GetChartRange().maRanges.size() )
/*N*/ 	{
/*?*/ 		SchDLL::ConvertChartRangeForWriter( *pData, FALSE );
/*?*/ 		sSelection = pData->SomeData1();
/*?*/ 		sRowColInfo = pData->SomeData2();
/*?*/ 		bSetChartRange = FALSE;
/*N*/ 	}
/*N*/ 	else if( pSelection )
/*N*/ 	{
/*?*/ 		sSelection = *pSelection;
/*?*/ 		sRowColInfo.AssignAscii( RTL_CONSTASCII_STRINGPARAM("11") );
/*N*/ 	}
/*N*/ 
/*N*/ 	SwChartLines aLines;
/*N*/ 	if( !IsTblComplexForChart( sSelection, &aLines ))
/*N*/ 	{
/*N*/ 		USHORT nLines = aLines.Count(), nBoxes = aLines[0]->Count();
/*N*/ 
/*N*/ 		if( !pData )
/*N*/ 		{
/*N*/ 			//JP 08.02.99: als default wird mit Spalten/Zeilenueberschrift
/*N*/ 			//				eingefuegt, deshalb das -1
/*?*/ 			pData = SchDLL::NewMemChart( nBoxes-1, nLines-1 );
/*?*/ 			pData->SetSubTitle( aEmptyStr );
/*?*/ 			pData->SetXAxisTitle( aEmptyStr );
/*?*/ 			pData->SetYAxisTitle( aEmptyStr );
/*?*/ 			pData->SetZAxisTitle( aEmptyStr );
/*N*/ 		}
/*N*/ 
/*N*/ 		USHORT nRowStt = 0, nColStt = 0;
/*N*/ 		if( sRowColInfo.Len() )
/*N*/ 		{
/*?*/ 			if( '1' == sRowColInfo.GetChar( 0 ))
/*?*/ 				++nRowStt;
/*?*/ 			if( '1' == sRowColInfo.GetChar( 1 ))
/*?*/ 				++nColStt;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( (nBoxes - nColStt) > pData->GetColCount() )
/*N*/ 			SchDLL::MemChartInsertCols( *pData, 0, (nBoxes - nColStt) - pData->GetColCount() );
/*N*/ 		else if( (nBoxes - nColStt) < pData->GetColCount() )
/*?*/ 			SchDLL::MemChartRemoveCols( *pData, 0, pData->GetColCount() - (nBoxes - nColStt) );
/*N*/ 
/*N*/ 		if( (nLines - nRowStt) > pData->GetRowCount() )
/*N*/ 			SchDLL::MemChartInsertRows( *pData, 0, (nLines - nRowStt) - pData->GetRowCount() );
/*N*/ 		else if( (nLines - nRowStt) < pData->GetRowCount() )
/*?*/ 			SchDLL::MemChartRemoveRows( *pData, 0, pData->GetRowCount() - (nLines - nRowStt) );
/*N*/ 
/*N*/ 
/*N*/ 		ASSERT( pData->GetRowCount() >= (nLines - nRowStt ) &&
/*N*/ 				pData->GetColCount() >= (nBoxes - nColStt ),
/*N*/ 					"Die Struktur fuers Chart ist zu klein,\n"
/*N*/ 					"es wird irgendwo in den Speicher geschrieben!" );
/*N*/ 
/*N*/ 		// Row-Texte setzen
/*N*/ 		USHORT n;
/*N*/ 		if( nRowStt )
/*?*/ 			for( n = nColStt; n < nBoxes; ++n )
/*?*/ 			{
/*?*/ 				const SwTableBox *pBox = (*aLines[ 0 ])[ n ];
/*?*/ 				ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
/*?*/ 				SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
/*?*/ 				const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
/*?*/ 				if( !pTNd )
/*?*/ 					pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
/*?*/ 								->GetTxtNode();
/*?*/ 
/*?*/ 				pData->SetColText( n - nColStt, pTNd->GetExpandTxt() );
/*?*/ 			}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aText;
/*N*/ 			for( n = nColStt; n < nBoxes; ++n )
/*N*/ 			{
/*N*/ 				SchDLL::GetDefaultForColumnText( *pData, n - nColStt, aText );
/*N*/ 				pData->SetColText( n - nColStt, aText );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Col-Texte setzen
/*N*/ 		if( nColStt )
/*N*/ 			for( n = nRowStt; n < nLines; ++n )
/*N*/ 			{
/*N*/ 				const SwTableBox *pBox = (*aLines[ n ])[ 0 ];
/*N*/ 				ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
/*N*/ 				SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
/*N*/ 				const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
/*N*/ 				if( !pTNd )
/*N*/ 					pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
/*N*/ 								->GetTxtNode();
/*N*/ 
/*N*/ 				pData->SetRowText( n - nRowStt, pTNd->GetExpandTxt() );
/*N*/ 			}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aText;
/*N*/ 			for( n = nRowStt; n < nLines; ++n )
/*N*/ 			{
/*N*/ 				SchDLL::GetDefaultForRowText( *pData, n - nRowStt, aText );
/*N*/ 				pData->SetRowText( n - nRowStt, aText );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// und dann fehlen nur noch die Daten
/*N*/ 		const SwTblBoxNumFormat& rDfltNumFmt = *(SwTblBoxNumFormat*)
/*N*/ 										GetDfltAttr( RES_BOXATR_FORMAT );
/*N*/ 		pData->SetNumberFormatter( GetFrmFmt()->GetDoc()->GetNumberFormatter());
/*N*/ 
/*N*/ 		int bFirstRow = TRUE;
/*N*/ 		for( n = nRowStt; n < nLines; ++n )
/*N*/ 		{
/*N*/ 			for( USHORT i = nColStt; i < nBoxes; ++i )
/*N*/ 			{
/*N*/ 				const SwTableBox* pBox = (*aLines[ n ])[ i ];
/*N*/ 				ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
/*N*/ 				SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
/*N*/ 				const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
/*N*/ 				if( !pTNd )
/*?*/ 					pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
/*?*/ 								->GetTxtNode();
/*N*/ 
/*N*/ 				pData->SetData( short( i - nColStt ),
/*N*/ 								short( n - nRowStt ),
/*N*/ 								pTNd->GetTxt().Len()
/*N*/ 										? pBox->GetValue( aCalcPara )
/*N*/ 										: DBL_MIN );
/*N*/ 
/*N*/ 				if( i == nColStt || bFirstRow )
/*N*/ 				{
/*N*/ 					// first box of row set the numberformat
/*N*/ 					const SwTblBoxNumFormat& rNumFmt = pBox->GetFrmFmt()->
/*N*/ 														GetTblBoxNumFmt();
/*N*/ 					if( rNumFmt != rDfltNumFmt )
/*N*/ 					{
/*?*/ 						pData->SetNumFormatIdCol( i, rNumFmt.GetValue() );
/*?*/ 						if( bFirstRow )
/*?*/ 							pData->SetNumFormatIdRow( n, rNumFmt.GetValue() );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			bFirstRow = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( pData )
/*N*/ 	{
/*?*/ 		if( pData->GetColCount() )
/*?*/ 			SchDLL::MemChartRemoveCols( *pData, 0, pData->GetColCount() );
/*?*/ 		if( pData->GetRowCount() )
/*?*/ 			SchDLL::MemChartRemoveRows( *pData, 0, pData->GetRowCount() );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		bSetChartRange = FALSE;
/*N*/ 
/*N*/ 	if( bSetChartRange )
/*N*/ 	{
/*N*/ 		// convert the selection string to the SchartRanges
/*N*/ 		pData->SomeData1() = sSelection;
/*N*/ 		pData->SomeData2() = sRowColInfo;
/*N*/ 		SchDLL::ConvertChartRangeForWriter( *pData, TRUE );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pData;
/*N*/ }

/*N*/ BOOL SwTable::IsTblComplexForChart( const String& rSelection,
/*N*/ 									SwChartLines* pGetCLines ) const
/*N*/ {
/*N*/ 	const SwTableBox* pSttBox, *pEndBox;
/*N*/ 	if( 2 < rSelection.Len() )
/*N*/ 	{
/*N*/ 		// spitze Klammern am Anfang & Ende enfernen
/*?*/ 		String sBox( rSelection );
/*?*/ 		if( '<' == sBox.GetChar( 0  ) ) sBox.Erase( 0, 1 );
/*?*/ 		if( '>' == sBox.GetChar( sBox.Len()-1  ) ) sBox.Erase( sBox.Len()-1 );
/*?*/ 
/*?*/ 		xub_StrLen nTrenner = sBox.Search( ':' );
/*?*/ 		ASSERT( STRING_NOTFOUND != nTrenner, "keine gueltige Selektion" );
/*?*/ 
/*?*/ 		pSttBox = GetTblBox( sBox.Copy( 0, nTrenner ));
/*?*/ 		pEndBox = GetTblBox( sBox.Copy( nTrenner+1 ));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwTableLines* pLns = &GetTabLines();
/*N*/ 		pSttBox = (*pLns)[ 0 ]->GetTabBoxes()[ 0 ];
/*N*/ 		while( !pSttBox->GetSttNd() )
/*N*/ 			// bis zur Content Box!
/*?*/ 			pSttBox = pSttBox->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];
/*N*/ 
/*N*/ 		const SwTableBoxes* pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
/*N*/ 		pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
/*N*/ 		while( !pEndBox->GetSttNd() )
/*N*/ 		{
/*N*/ 			// bis zur Content Box!
/*?*/ 			pLns = &pEndBox->GetTabLines();
/*?*/ 			pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
/*?*/ 			pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return !pSttBox || !pEndBox || !::binfilter::ChkChartSel( *pSttBox->GetSttNd(),
/*N*/ 										*pEndBox->GetSttNd(), pGetCLines );
/*N*/ }



/*N*/ IMPL_LINK( SwDoc, DoUpdateAllCharts, Timer *, pTimer )
/*N*/ {
/*N*/ 	ViewShell* pVSh;
/*N*/ 	GetEditShell( &pVSh );
/*N*/ 	if( pVSh )
/*N*/ 	{
/*N*/ 		const SwFrmFmts& rTblFmts = *GetTblFrmFmts();
/*N*/ 		for( USHORT n = 0; n < rTblFmts.Count(); ++n )
/*N*/ 		{
/*N*/ 			SwTable* pTmpTbl;
/*N*/ 			const SwTableNode* pTblNd;
/*N*/ 			SwFrmFmt* pFmt = rTblFmts[ n ];
/*N*/ 
/*N*/ 			if( 0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
/*N*/ 				0 != ( pTblNd = pTmpTbl->GetTableNode() ) &&
/*N*/ 				pTblNd->GetNodes().IsDocNodes() )
/*N*/ 			{
/*N*/ 				_UpdateCharts( *pTmpTbl, *pVSh );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ void SwDoc::_UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const
/*N*/ {
/*N*/ 	String aName( rTbl.GetFrmFmt()->GetName() );
/*N*/ 	SwOLENode *pONd;
/*N*/ 	SwStartNode *pStNd;
/*N*/ 	SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
/*N*/ 	while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
/*N*/ 	{
/*N*/ 		aIdx++;
/*N*/ 		SwFrm* pFrm;
/*N*/ 		if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
/*N*/ 			aName.Equals( pONd->GetChartTblName() ) &&
/*N*/ 			0 != ( pFrm = pONd->GetFrm() ) )
/*N*/ 		{
/*N*/ 			SwOLEObj& rOObj = pONd->GetOLEObj();
/*N*/ 
/*N*/ 			SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
/*N*/ 			FASTBOOL bDelData = 0 == pData;
/*N*/ 
/*N*/ 			ASSERT( pData, "UpdateChart ohne irgendwelche Daten?" );
/*N*/ 			pData = rTbl.UpdateData( pData );
/*N*/ 
/*N*/ 			if( pData->GetColCount() && pData->GetRowCount() )
/*N*/ 			{
/*N*/ 				SchDLL::Update( rOObj.GetOleRef(), pData, rVSh.GetWin() );
/*N*/ 
/*N*/ 				SwClientIter aIter( *pONd );
/*N*/ 				for( pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) ); pFrm;
/*N*/ 						pFrm = (SwFrm*)aIter.Next() )
/*N*/ 				{
/*N*/ 					if( pFrm->Frm().HasArea() )
/*N*/ 						rVSh.InvalidateWindows( pFrm->Frm() );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( bDelData )
/*?*/ 				delete pData;
/*N*/ 		}
/*N*/ 		aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
/*N*/ 	}
/*N*/ }


/*N*/ void SwDoc::SetTableName( SwFrmFmt& rTblFmt, const String &rNewName )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	const String aOldName( rTblFmt.GetName() );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
