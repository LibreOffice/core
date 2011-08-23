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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/algitem.hxx>
#include <bf_svx/editobj.hxx>
#include <bf_svx/emphitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>
#include <bf_svx/rotmodit.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <bf_svx/unolingu.hxx>
#include <vcl/outdev.hxx>
#include <math.h>

#include "cell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "rechead.hxx"
#include "editutil.hxx"
#include "subtotal.hxx"
#include "markdata.hxx"
#include "dbcolect.hxx"
#include "bclist.hxx"
namespace binfilter {

// -----------------------------------------------------------------------

// factor from font size to optimal cell height (text width)
#define SC_ROT_BREAK_FACTOR		6

// -----------------------------------------------------------------------

/*N*/ inline BOOL CellVisible( const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	return ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() );
/*N*/ }
/*N*/ 
/*N*/ inline BOOL IsAmbiguousScript( BYTE nScript )
/*N*/ {
/*N*/ 	//!	move to a header file
/*N*/ 	return ( nScript != SCRIPTTYPE_LATIN &&
/*N*/  			 nScript != SCRIPTTYPE_ASIAN &&
/*N*/ 			 nScript != SCRIPTTYPE_COMPLEX );
/*N*/ }

// -----------------------------------------------------------------------------------------

//
//	Datei-Operationen
//

// -----------------------------------------------------------------------------------------

// special handling for non-convertable characters is no longer needed
#if 0

//	read string from a string cell in original CharSet


#endif

// -----------------------------------------------------------------------------------------

/*N*/ void ScColumn::LoadData( SvStream& rStream )
/*N*/ {
/*N*/ 	USHORT 		nNewCount;
/*N*/ 	USHORT 		nNewRow;
/*N*/ 	BYTE		nByte;
/*N*/ 	USHORT		nVer = (USHORT) pDocument->GetSrcVersion();
/*N*/ 
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> nNewCount;
/*N*/ 	if ( nNewCount > MAXROW+1 )						// wuerde das Array zu gross?
/*N*/ 	{
/*?*/ 		pDocument->SetLostData();
/*?*/ 		rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	Resize( nNewCount );							// veraendert nCount nicht
/*N*/ 	for (USHORT i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		rStream >> nNewRow;
/*N*/ 		rStream >> nByte;
/*N*/ 
/*N*/ 		if ( nNewRow > MAXROW )					// Zeilennummer zu gross?
/*N*/ 		{
/*?*/ 			pDocument->SetLostData();
/*?*/ 			rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
/*?*/ 			return;
/*N*/ 		}
/*N*/ 
/*N*/ 		switch ((CellType) nByte)
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE:
/*N*/ 				{
/*N*/ 					ScValueCell* pCell = new ScValueCell( rStream, nVer );
/*N*/ 					Append( nNewRow, pCell );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case CELLTYPE_STRING:
/*N*/ 				{
/*N*/ 					ScStringCell* pCell = new ScStringCell( rStream, nVer );
/*N*/ 					Append( nNewRow, pCell );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case CELLTYPE_SYMBOLS:
/*N*/ 				{
/*N*/                     CharSet eOld = rStream.GetStreamCharSet();
/*N*/                     //  convert into true symbol characters
/*N*/                     rStream.SetStreamCharSet( RTL_TEXTENCODING_SYMBOL );
/*N*/ 					ScStringCell* pCell = new ScStringCell( rStream, nVer );
/*N*/ 					Append( nNewRow, pCell );
/*N*/                     rStream.SetStreamCharSet( eOld );
/*N*/                     ScSymbolStringCellEntry * pEntry = new ScSymbolStringCellEntry;
/*N*/                     pEntry->pCell = pCell;
/*N*/                     pEntry->nRow = nNewRow;
/*N*/                     pDocument->GetLoadedSymbolStringCellsList().Insert(
/*N*/                         pEntry, LIST_APPEND );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case CELLTYPE_EDIT:
/*N*/ 				{
/*N*/ 					ScEditCell* pCell = new ScEditCell( rStream, nVer, pDocument );
/*N*/ 					Append( nNewRow, pCell );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 				{
/*N*/ 					ScFormulaCell* pCell = new ScFormulaCell(
/*N*/ 											pDocument, ScAddress( nCol, nNewRow, nTab ), rStream, aHdr );
/*N*/ 					Append( nNewRow, pCell);
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case CELLTYPE_NOTE:
/*N*/ 				{
/*N*/ 					ScNoteCell *pCell = new ScNoteCell( rStream, nVer );
/*N*/ 					Append( nNewRow, pCell);
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				DBG_ERROR( "Falscher Zellentyp" );
/*N*/ 				rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
/*N*/ 				return;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL lcl_RemoveAny( ScDocument* pDocument, USHORT nCol, USHORT nTab )
/*N*/ {
/*N*/ 	ScDBCollection* pDBColl = pDocument->GetDBCollection();
/*N*/ 	if ( pDBColl )
/*N*/ 	{
/*N*/ 		USHORT nCount = pDBColl->GetCount();
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScDBData* pData = (*pDBColl)[i];
/*N*/ 			if ( pData->IsStripData() &&
/*N*/ 					pData->HasImportParam() && !pData->HasImportSelection() )
/*N*/ 			{
/*?*/ 				ScRange aDBRange;
/*?*/ 				pData->GetArea(aDBRange);
/*?*/ 				if ( nTab == aDBRange.aStart.Tab() &&
/*?*/ 					 nCol >= aDBRange.aStart.Col() && nCol <= aDBRange.aEnd.Col() )
/*?*/ 					return TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

BOOL lcl_RemoveThis( ScDocument* pDocument, USHORT nCol, USHORT nRow, USHORT nTab )
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDBCollection* pDBColl = pDocument->GetDBCollection();

    return FALSE;
}

/*N*/ void ScColumn::SaveData( SvStream& rStream ) const
/*N*/ {
/*N*/ 	CellType eCellType;
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	USHORT i;
/*N*/     ScFontToSubsFontConverter_AutoPtr xFontConverter;
/*N*/     const ULONG nFontConverterFlags = FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
/*N*/ 
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nSaveCount = nCount;
/*N*/ 
/*N*/ 	//	Zeilen hinter MAXROW abziehen
/*N*/ 	USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
/*N*/ 	if ( nSaveMaxRow != MAXROW )
/*N*/ 	{
/*N*/ 		if ( nSaveCount && pItems[nSaveCount-1].nRow > nSaveMaxRow )
/*N*/ 		{
/*?*/ 			pDocument->SetLostData();			// Warnung ausgeben
/*?*/ 			do
/*?*/ 				--nSaveCount;
/*?*/ 			while ( nSaveCount && pItems[nSaveCount-1].nRow > nSaveMaxRow );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Zellen abziehen, die wegen Import nicht gespeichert werden
/*N*/ 	BOOL bRemoveAny = lcl_RemoveAny( pDocument, nCol, nTab );
/*N*/ 	USHORT nEffCount = nSaveCount;
/*N*/ 	if ( bRemoveAny )
/*N*/ 	{
/*?*/ 		for (i=0; i<nSaveCount; i++)
/*?*/ 			if ( lcl_RemoveThis( pDocument, nCol, pItems[i].nRow, nTab ) )
/*?*/ 				--nEffCount;
/*?*/ 
/*?*/ //		String aDbg("Tab ");aDbg+=nTab;aDbg+=" Col ";aDbg+=nCol;
/*?*/ //		aDbg+=" Remove ";aDbg+=nSaveCount-nEffCount; DBG_ERROR(aDbg);
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nEffCount;			// nEffCount: Zellen, die wirklich gespeichert werden
/*N*/ 
/*N*/ 	ScAttrIterator aIter( pAttrArray, 0, MAXROW );
/*N*/ 	USHORT nStt, nEnd;
/*N*/ 	const ScPatternAttr* pAttr;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pAttr = aIter.Next( nStt, nEnd );
/*N*/ 	}
/*N*/     while( pAttr && !(
/*N*/         (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags ))
/*N*/         || pAttr->IsSymbolFont()) );
/*N*/ 
/*N*/ 	for (i=0; i<nSaveCount; i++)		// nSaveCount: Ende auf MAXROW angepasst
/*N*/ 	{
/*N*/ 		USHORT nRow = pItems[i].nRow;
/*N*/ 
/*N*/ 		if ( !bRemoveAny || !lcl_RemoveThis( pDocument, nCol, nRow, nTab ) )
/*N*/ 		{
/*N*/ 			rStream << nRow;
/*N*/ 
/*N*/ 			pCell = pItems[i].pCell;
/*N*/ 			eCellType = pCell->GetCellType();
/*N*/ 
/*N*/ 			switch( eCellType )
/*N*/ 			{
/*N*/ 				case CELLTYPE_VALUE:
/*N*/ 					rStream << (BYTE) eCellType;
/*N*/ 					((ScValueCell*)pCell)->Save( rStream );
/*N*/ 					break;
/*N*/ 				case CELLTYPE_STRING:
/*N*/ 					if( pAttr )
/*N*/ 					{
/*N*/ 						if( nRow > nEnd )
/*N*/                         {
/*N*/                             do
/*N*/                             {
/*N*/                                 do
/*N*/                                 {
/*N*/                                     pAttr = aIter.Next( nStt, nEnd );
/*N*/                                 }
/*N*/                                 while ( pAttr && nRow > nEnd );     // #99139# skip all formats before this cell
/*N*/                             }
/*N*/                             while( pAttr && !(
/*N*/                                 (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags ))
/*N*/                                 || pAttr->IsSymbolFont()) );
/*N*/                         }
/*N*/ 						if( pAttr && nRow >= nStt && nRow <= nEnd )
/*N*/ 							eCellType = CELLTYPE_SYMBOLS;
/*N*/ 					}
/*N*/ 					rStream << (BYTE) eCellType;
/*N*/ 					if ( eCellType == CELLTYPE_SYMBOLS )
/*N*/ 					{
/*N*/ 						//	cell string contains true symbol characters
/*N*/                         CharSet eOld = rStream.GetStreamCharSet();
/*N*/                         rStream.SetStreamCharSet( RTL_TEXTENCODING_SYMBOL );
/*N*/                         ((ScStringCell*)pCell)->Save( rStream, xFontConverter );
/*N*/                         rStream.SetStreamCharSet( eOld );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						((ScStringCell*)pCell)->Save( rStream );
/*N*/ 					break;
/*N*/ 				case CELLTYPE_EDIT:
/*N*/ 					rStream << (BYTE) eCellType;
/*N*/ 					((ScEditCell*)pCell)->Save( rStream );
/*N*/ 					break;
/*N*/ 				case CELLTYPE_FORMULA:
/*N*/ 					rStream << (BYTE) eCellType;
/*N*/ 					((ScFormulaCell*)pCell)->Save( rStream, aHdr );
/*N*/ 					break;
/*N*/ 				case CELLTYPE_NOTE:
/*N*/ 					rStream << (BYTE) eCellType;
/*N*/ 					((ScNoteCell*)pCell)->Save( rStream );
/*N*/ 					break;
/*?*/ 				default:
/*?*/ 					{
/*?*/ 						//	#53846# soll zwar nicht vorkommen, aber falls doch,
/*?*/ 						//	eine leere NoteCell speichern, damit das Dokument
/*?*/ 						//	ueberhaupt wieder geladen werden kann.
/*?*/ 						rStream << (BYTE) CELLTYPE_NOTE;
/*?*/ 						ScNoteCell aDummyCell;
/*?*/ 						aDummyCell.Save( rStream );
/*?*/ 						DBG_ERROR( "Falscher Zellentyp" );
/*?*/ 					}
/*?*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------------------

/*N*/ void ScColumn::LoadNotes( SvStream& rStream )
/*N*/ {
/*N*/ 	ScReadHeader aHdr(rStream);
/*N*/ 
/*N*/ 	USHORT nNoteCount;
/*N*/ 	rStream >> nNoteCount;
/*N*/ 	for (USHORT i=0; i<nNoteCount && rStream.GetError() == SVSTREAM_OK; i++)
/*N*/ 	{
/*N*/ 		USHORT nPos;
/*N*/ 		rStream >> nPos;
/*N*/ 		if (nPos < nCount)
/*N*/ 			pItems[nPos].pCell->LoadNote(rStream);
/*N*/ 		else
/*N*/ 		{
/*N*/ 			DBG_ERROR("falsche Pos in ScColumn::LoadNotes");
/*N*/ 			rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ USHORT ScColumn::NoteCount( USHORT nMaxRow ) const
/*N*/ {
/*N*/ 	USHORT nNoteCount = 0;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if ( pItems[i].pCell->GetNotePtr() && pItems[i].nRow<=nMaxRow )
/*N*/ 			++nNoteCount;
/*N*/ 
/*N*/ 	return nNoteCount;
/*N*/ }

/*N*/ void ScColumn::SaveNotes( SvStream& rStream ) const
/*N*/ {
/*N*/ 	USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	BOOL bRemoveAny = lcl_RemoveAny( pDocument, nCol, nTab );
/*N*/ 	USHORT nNoteCount;
/*N*/ 	if ( bRemoveAny )
/*N*/ 	{
/*?*/ 		//	vorher zaehlen, wieviele Notizen es werden
/*?*/ 
/*?*/ 		nNoteCount = 0;
/*?*/ 		for (i=0; i<nCount; i++)
/*?*/ 			if ( pItems[i].pCell->GetNotePtr() && pItems[i].nRow<=nSaveMaxRow &&
/*?*/ 					!lcl_RemoveThis( pDocument, nCol, pItems[i].nRow, nTab ) )
/*?*/ 				++nNoteCount;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nNoteCount = NoteCount(nSaveMaxRow);
/*N*/ 
/*N*/ 	//	Speichern
/*N*/ 	//	Als Positionen muessen die Indizes gespeichert werden, die beim Laden entstehen,
/*N*/ 	//	also ohne die weggelassenen Zellen mitzuzaehlen.
/*N*/ 
/*N*/ 	ScWriteHeader aHdr(rStream);
/*N*/ 	rStream << nNoteCount;
/*N*/ 
/*N*/ 	USHORT nDestPos = 0;
/*N*/ 	for (i=0; i<nCount && rStream.GetError() == SVSTREAM_OK; i++)
/*N*/ 	{
/*N*/ 		USHORT nRow = pItems[i].nRow;
/*N*/ 		if ( !bRemoveAny || !lcl_RemoveThis( pDocument, nCol, nRow, nTab ) )
/*N*/ 		{
/*N*/ 			const ScPostIt* pNote = pItems[i].pCell->GetNotePtr();
/*N*/ 			if ( pNote && nRow <= nSaveMaxRow )
/*N*/ 			{
/*N*/ 				rStream << nDestPos;
/*N*/ 				rStream << *pNote;
/*N*/ 			}
/*N*/ 			++nDestPos;			// nDestPos zaehlt die in SaveData gespeicherten Zellen
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	SetLostData ist schon in SaveData passiert, wenn noetig
/*N*/ }

// -----------------------------------------------------------------------------------------

/*N*/ void ScColumn::CorrectSymbolCells( CharSet eStreamCharSet )
/*N*/ {
/*N*/     //  #99139# find and correct string cells that are formatted with a symbol font,
/*N*/     //  but are not in the LoadedSymbolStringCellsList
/*N*/     //  (because CELLTYPE_SYMBOLS wasn't written in the file)
/*N*/ 
/*N*/     ScFontToSubsFontConverter_AutoPtr xFontConverter;
/*N*/     const ULONG nFontConverterFlags = FONTTOSUBSFONT_EXPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
/*N*/ 
/*N*/     BOOL bListInitialized = FALSE;
/*N*/     ScSymbolStringCellEntry* pCurrentEntry = NULL;
/*N*/ 
/*N*/     ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
/*N*/     USHORT nStt, nEnd;
/*N*/     const ScPatternAttr* pAttr = aAttrIter.Next( nStt, nEnd );
/*N*/     while ( pAttr )
/*N*/     {
/*N*/         if ( (xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags )) ||
/*N*/                 pAttr->IsSymbolFont() )
/*N*/         {
/*N*/             ScColumnIterator aCellIter( this, nStt, nEnd );
/*N*/             USHORT nRow;
/*N*/             ScBaseCell* pCell;
/*N*/             while ( aCellIter.Next( nRow, pCell ) )
/*N*/             {
/*N*/                 if ( pCell->GetCellType() == CELLTYPE_STRING )
/*N*/                 {
/*N*/                     List& rList = pDocument->GetLoadedSymbolStringCellsList();
/*N*/                     if (!bListInitialized)
/*N*/                     {
/*N*/                         pCurrentEntry = (ScSymbolStringCellEntry*)rList.First();
/*N*/                         bListInitialized = TRUE;
/*N*/                     }
/*N*/ 
/*N*/                     while ( pCurrentEntry && pCurrentEntry->nRow < nRow )
/*N*/                         pCurrentEntry = (ScSymbolStringCellEntry*)rList.Next();
/*N*/ 
/*N*/                     if ( pCurrentEntry && pCurrentEntry->nRow == nRow )
/*N*/                     {
/*N*/                         //  found
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*?*/                         //  not in list -> convert and put into list
/*?*/ 
/*?*/                         ScStringCell* pStrCell = (ScStringCell*)pCell;
/*?*/                         String aOldStr;
/*?*/                         pStrCell->GetString( aOldStr );
/*?*/ 
/*?*/                         //  convert back to stream character set (get original data)
/*?*/                         ByteString aByteStr( aOldStr, eStreamCharSet );
/*?*/ 
/*?*/                         //  convert using symbol encoding, as for CELLTYPE_SYMBOLS cells
/*?*/                         String aNewStr( aByteStr, RTL_TEXTENCODING_SYMBOL );
/*?*/                         pStrCell->SetString( aNewStr );
/*?*/ 
/*?*/                         ScSymbolStringCellEntry * pEntry = new ScSymbolStringCellEntry;
/*?*/                         pEntry->pCell = pStrCell;
/*?*/                         pEntry->nRow = nRow;
/*?*/ 
/*?*/                         if ( pCurrentEntry )
/*?*/                             rList.Insert( pEntry );     // before current entry - pCurrentEntry stays valid
/*?*/                         else
/*?*/                             rList.Insert( pEntry, LIST_APPEND );    // append if already behind last entry
/*N*/                     }
/*N*/                 }
/*N*/             }           
/*N*/         }
/*N*/ 
/*N*/         pAttr = aAttrIter.Next( nStt, nEnd );
/*N*/     }
/*N*/ }

/*N*/ BOOL ScColumn::Load( SvStream& rStream, ScMultipleReadHeader& rHdr )
/*N*/ {
/*N*/ 	rHdr.StartEntry();
/*N*/ 	while (rHdr.BytesLeft() && rStream.GetError() == SVSTREAM_OK)
/*N*/ 	{
/*N*/ 		USHORT nID;
/*N*/ 		rStream >> nID;
/*N*/ 		switch (nID)
/*N*/ 		{
/*N*/ 			case SCID_COLDATA:
/*N*/ 				LoadData( rStream );
/*N*/ 				break;
/*N*/ 			case SCID_COLNOTES:
/*N*/ 				LoadNotes( rStream );
/*N*/ 				break;
/*N*/ 			case SCID_COLATTRIB:
/*N*/ 				pAttrArray->Load( rStream );
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				{
/*N*/ 					DBG_ERROR("unbekannter Sub-Record in ScColumn::Load");
/*N*/ 					ScReadHeader aDummyHeader( rStream );
/*N*/ 				}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/     //  #99139# old versions didn't always write CELLTYPE_SYMBOLS for symbol string cells,
/*N*/     //  so we have to look for remaining string cells in areas that are formatted with
/*N*/     //  symbol font:
/*N*/     CorrectSymbolCells( rStream.GetStreamCharSet() );
/*N*/ 
/*N*/     if ( pDocument->SymbolStringCellsPending() )
/*N*/     {
/*N*/         ScFontToSubsFontConverter_AutoPtr xFontConverter;
/*N*/         const ULONG nFontConverterFlags = FONTTOSUBSFONT_IMPORT | FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS;
/*N*/         ScSymbolStringCellEntry* pE;
/*N*/         USHORT nStt, nEnd;
/*N*/ 
/*N*/         ScAttrIterator aIter( pAttrArray, 0, MAXROW );
/*N*/         const ScPatternAttr* pAttr = aIter.Next( nStt, nEnd );
/*N*/         xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags );
/*N*/ 
/*N*/         List& rList = pDocument->GetLoadedSymbolStringCellsList();
/*N*/         for ( pE = (ScSymbolStringCellEntry*) rList.First(); pE;
/*N*/                 pE = (ScSymbolStringCellEntry*) rList.Next() )
/*N*/         {
/*N*/             const ScPatternAttr* pLastAttr = pAttr;
/*N*/             while ( nEnd < pE->nRow )
/*N*/             {
/*N*/                 pAttr = aIter.Next( nStt, nEnd );
/*N*/             }
/*N*/             if ( pAttr != pLastAttr )
/*N*/                 xFontConverter = pAttr->GetSubsFontConverter( nFontConverterFlags );
/*N*/             pE->pCell->ConvertFont( xFontConverter );
/*N*/             delete pE;
/*N*/         }
/*N*/         rList.Clear();
/*N*/     }
/*N*/     pAttrArray->ConvertFontsAfterLoad();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScColumn::Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	if (!IsEmptyData())				//!	Test, ob alles weggelassen wird?
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_COLDATA;
/*N*/ 		SaveData( rStream );
/*N*/ 	}
/*N*/ 	USHORT nNotes = NoteCount();	//!	Test, ob alles weggelassen wird?
/*N*/ 	if (nNotes)
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_COLNOTES;
/*N*/ 		SaveNotes( rStream );
/*N*/ 	}
/*N*/ 	if (!IsEmptyAttr())
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_COLATTRIB;
/*N*/ 		pAttrArray->Save( rStream );
/*N*/ 	}
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------------------

                                    //	GetNeededSize: optimale Hoehe / Breite in Pixeln

/*N*/ long ScColumn::GetNeededSize( USHORT nRow, OutputDevice* pDev,
/*N*/ 							  double nPPTX, double nPPTY,
/*N*/ 							  const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 							  BOOL bWidth, const ScNeededSizeOptions& rOptions )
/*N*/ {
/*N*/ 	long nValue=0;
/*N*/ 	USHORT nIndex;
/*N*/ 	double nPPT = bWidth ? nPPTX : nPPTY;
/*N*/ 	if (Search(nRow,nIndex))
/*N*/ 	{
/*N*/ 		const ScPatternAttr* pPattern = rOptions.pPattern;
/*N*/ 		if (!pPattern)
/*N*/ 			pPattern = pAttrArray->GetPattern( nRow );
/*N*/ 
/*N*/ 		//		zusammengefasst?
/*N*/ 		//		Merge nicht in bedingter Formatierung
/*N*/ 
/*N*/ 		const ScMergeAttr*		pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
/*N*/ 		const ScMergeFlagAttr*	pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
/*N*/ 
/*N*/ 		if ( bWidth )
/*N*/ 		{
/*N*/ 			if ( pFlag->IsHorOverlapped() )
/*N*/ 				return 0;
/*N*/ 			if ( rOptions.bSkipMerged && pMerge->GetColMerge() > 1 )
/*N*/ 				return 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( pFlag->IsVerOverlapped() )
/*N*/ 				return 0;
/*N*/ 			if ( rOptions.bSkipMerged && pMerge->GetRowMerge() > 1 )
/*N*/ 				return 0;
/*N*/ 		}
/*N*/ 
/*N*/ 		//		bedingte Formatierung
/*N*/ 		const SfxItemSet* pCondSet = NULL;
/*N*/ 		if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
/*N*/ 			pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );
/*N*/ 
/*N*/ 		//	Zeilenumbruch?
/*N*/ 
/*N*/ 		const SfxPoolItem* pCondItem;
/*N*/ 		SvxCellHorJustify eHorJust;
/*N*/ 		if (pCondSet &&
/*N*/ 				pCondSet->GetItemState(ATTR_HOR_JUSTIFY, TRUE, &pCondItem) == SFX_ITEM_SET)
/*N*/ 			eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem*)pCondItem)->GetValue();
/*N*/ 		else
/*N*/ 			eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
/*N*/ 											pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();
/*N*/ 		BOOL bBreak;
/*N*/ 		if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
/*N*/ 			bBreak = TRUE;
/*N*/ 		else if ( pCondSet &&
/*N*/ 					pCondSet->GetItemState(ATTR_LINEBREAK, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 			bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
/*N*/ 		else
/*N*/ 			bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();
/*N*/ 
/*N*/ 		//	get other attributes from pattern and conditional formatting
/*N*/ 
/*N*/ 		SvxCellOrientation eOrient;
/*N*/ 		if (pCondSet &&
/*N*/ 				pCondSet->GetItemState(ATTR_ORIENTATION, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 			eOrient = (SvxCellOrientation)((const SvxOrientationItem*)pCondItem)->GetValue();
/*N*/ 		else
/*N*/ 			eOrient = (SvxCellOrientation)((const SvxOrientationItem&)
/*N*/ 											pPattern->GetItem(ATTR_ORIENTATION)).GetValue();
/*N*/ 		BOOL bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
/*N*/ 				((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
/*N*/ 		if ( bAsianVertical )
/*N*/ 			bBreak = FALSE;
/*N*/ 
/*N*/ 		if ( bWidth && bBreak )		// after determining bAsianVertical (bBreak may be reset)
/*N*/ 			return 0;
/*N*/ 
/*N*/ 		long nRotate = 0;
/*N*/ 		SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
/*N*/ 		if ( eOrient == SVX_ORIENTATION_STANDARD )
/*N*/ 		{
/*N*/ 			if (pCondSet &&
/*N*/ 					pCondSet->GetItemState(ATTR_ROTATE_VALUE, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 				nRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
/*N*/ 			else
/*N*/ 				nRotate = ((const SfxInt32Item&)pPattern->GetItem(ATTR_ROTATE_VALUE)).GetValue();
/*N*/ 			if ( nRotate )
/*N*/ 			{
/*?*/ 				if (pCondSet &&
/*?*/ 						pCondSet->GetItemState(ATTR_ROTATE_MODE, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 					eRotMode = (SvxRotateMode)((const SvxRotateModeItem*)pCondItem)->GetValue();
/*?*/ 				else
/*?*/ 					eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
/*?*/ 												pPattern->GetItem(ATTR_ROTATE_MODE)).GetValue();
/*?*/ 
/*?*/ 				if ( nRotate == 18000 )
/*?*/ 					eRotMode = SVX_ROTATE_MODE_STANDARD;	// keinen Ueberlauf
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		const SvxMarginItem* pMargin;
/*N*/ 		if (pCondSet &&
/*N*/ 				pCondSet->GetItemState(ATTR_MARGIN, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 			pMargin = (const SvxMarginItem*) pCondItem;
/*N*/ 		else
/*N*/ 			pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
/*N*/ 		USHORT nIndent = 0;
/*N*/ 		if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
/*N*/ 		{
/*N*/ 			if (pCondSet &&
/*N*/ 					pCondSet->GetItemState(ATTR_INDENT, TRUE, &pCondItem) == SFX_ITEM_SET)
/*?*/ 				nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
/*N*/ 			else
/*N*/ 				nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
/*N*/ 		}
/*N*/ 
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pCell );
/*N*/ 		if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();
/*N*/ 
/*N*/ 		//	also call SetFont for edit cells, because bGetFont may be set only once
/*N*/ 		//	bGetFont is set also if script type changes
/*N*/ 		if (rOptions.bGetFont)
/*N*/ 		{
/*N*/ 			Fraction aFontZoom = ( eOrient == SVX_ORIENTATION_STANDARD ) ? rZoomX : rZoomY;
/*N*/ 			Font aFont;
/*N*/ 			// font color doesn't matter here
/*N*/ 			pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aFontZoom, pCondSet, nScript );
/*N*/ 			pDev->SetFont(aFont);
/*N*/ 		}
/*N*/ 
/*N*/ 		BOOL bAddMargin = TRUE;
/*N*/ 		BOOL bEditEngine = ( pCell->GetCellType() == CELLTYPE_EDIT ||
/*N*/ 								eOrient == SVX_ORIENTATION_STACKED ||
/*N*/ 								IsAmbiguousScript( nScript ) );
/*N*/ 
/*N*/ 		if (!bEditEngine)									// direkte Ausgabe
/*N*/ 		{
/*N*/ 			String aValStr;
/*N*/ 			Color* pColor;
/*N*/ 			SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 			ULONG nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
/*N*/ 			ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
/*N*/ 										*pFormatter,
/*N*/ 										TRUE, rOptions.bFormula, ftCheck );
/*N*/ 			if (aValStr.Len())
/*N*/ 			{
/*N*/ 				//	SetFont ist nach oben verschoben
/*N*/ 
/*N*/ 				Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
/*N*/ 				if ( eOrient != SVX_ORIENTATION_STANDARD )
/*N*/ 				{
/*?*/ 					long nTemp = aSize.Width();
/*?*/ 					aSize.Width() = aSize.Height();
/*?*/ 					aSize.Height() = nTemp;
/*N*/ 				}
/*N*/ 				else if ( nRotate )
/*N*/ 				{
/*?*/ 					//!	unterschiedliche Skalierung X/Y beruecksichtigen
/*?*/ 
/*?*/ 					double nRealOrient = nRotate * F_PI18000;	// nRotate sind 1/100 Grad
/*?*/ 					double nCosAbs = fabs( cos( nRealOrient ) );
/*?*/ 					double nSinAbs = fabs( sin( nRealOrient ) );
/*?*/ 					long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
/*?*/ 					long nWidth;
/*?*/ 					if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
/*?*/ 						nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
/*?*/ 					else if ( rOptions.bTotalSize )
/*?*/ 					{
/*?*/ 						nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
/*?*/ 						bAddMargin = FALSE;
/*?*/ 						//	nur nach rechts:
/*?*/ 						//!	unterscheiden nach Ausrichtung oben/unten (nur Text/ganze Hoehe)
DBG_BF_ASSERT(0, "STRIP"); 
/*?*/ 					}
/*?*/ 					else
/*?*/ 						nWidth  = (long)( aSize.Height() / nSinAbs );	//! begrenzen?
/*?*/ 
/*?*/ 					if ( bBreak && !rOptions.bTotalSize )
/*?*/ 					{
/*?*/ 						//	#47744# limit size for line break
/*?*/ 						long nCmp = pDev->GetFont().GetSize().Height() * SC_ROT_BREAK_FACTOR;
/*?*/ 						if ( nHeight > nCmp )
/*?*/ 							nHeight = nCmp;
/*?*/ 					}
/*?*/ 
/*?*/ 					aSize = Size( nWidth, nHeight );
/*N*/ 				}
/*N*/ 				nValue = bWidth ? aSize.Width() : aSize.Height();
/*N*/ 
/*N*/ 				if ( bAddMargin )
/*N*/ 				{
/*N*/ 					if (bWidth)
/*N*/ 					{
/*?*/ 						nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
/*?*/ 								  (long) ( pMargin->GetRightMargin() * nPPT );
/*?*/ 						if ( nIndent )
/*?*/ 							nValue += (long) ( nIndent * nPPT );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
/*N*/ 								  (long) ( pMargin->GetBottomMargin() * nPPT );
/*N*/ 				}
/*N*/ 
/*N*/ 												//	Zeilenumbruch ausgefuehrt ?
/*N*/ 
/*N*/ 				if ( bBreak && !bWidth )
/*N*/ 				{
/*N*/ 					//	Test mit EditEngine zur Sicherheit schon bei 90%
/*N*/ 					//	(wegen Rundungsfehlern und weil EditEngine teilweise anders formatiert)
/*N*/ 
/*N*/ 					long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
/*N*/ 										pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
/*N*/ 										nIndent )
/*N*/ 										* nPPT );
/*N*/ 					nDocPixel = (nDocPixel * 9) / 10;			// zur Sicherheit
/*N*/ 					if ( aSize.Width() > nDocPixel )
/*N*/ 						bEditEngine = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bEditEngine)
/*N*/ 		{
/*N*/ 			//	der Font wird bei !bEditEngine nicht jedesmal neu gesetzt
/*N*/ 			Font aOldFont = pDev->GetFont();
/*N*/ 
/*N*/ 			MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );
/*N*/ 
/*N*/ 			// am Dokument speichern ?
/*N*/ 			ScFieldEditEngine* pEngine = pDocument->CreateFieldEditEngine();
/*N*/ 
/*N*/ 			pEngine->SetUpdateMode( FALSE );
/*N*/ 			MapMode aOld = pDev->GetMapMode();
/*N*/ 			pDev->SetMapMode( aHMMMode );
/*N*/ 			pEngine->SetRefDevice( pDev );
/*N*/ 			pEngine->SetForbiddenCharsTable( pDocument->GetForbiddenCharacters() );
/*N*/ 			pEngine->SetAsianCompressionMode( pDocument->GetAsianCompression() );
/*N*/ 			pEngine->SetKernAsianPunctuation( pDocument->GetAsianKerning() );
/*N*/ 			SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
/*N*/ 			pPattern->FillEditItemSet( pSet, pCondSet );
/*N*/ 
/*N*/ //			no longer needed, are setted with the text (is faster)
/*N*/ //			pEngine->SetDefaults( pSet );
/*N*/ 
/*N*/ 
/*N*/ 			Size aPaper = Size( 1000000, 1000000 );
/*N*/ 			if ( eOrient==SVX_ORIENTATION_STACKED && !bAsianVertical )
/*?*/ 				aPaper.Width() = 1;
/*N*/ 			else if (bBreak)
/*N*/ 			{
/*N*/ 				double fWidthFactor = nPPTX;
/*N*/ 				BOOL bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
/*N*/ 				if ( bTextWysiwyg )
/*N*/ 				{
/*N*/ 					//	#95593# if text is formatted for printer, don't use PixelToLogic,
/*N*/ 					//	to ensure the exact same paper width (and same line breaks) as in
/*N*/ 					//	ScEditUtil::GetEditArea, used for output.
/*N*/ 
/*N*/ 					fWidthFactor = HMM_PER_TWIPS;
/*N*/ 				}
/*N*/ 
/*N*/ 				// use original width for hidden columns:
/*N*/ 				long nDocWidth = (long) ( pDocument->GetOriginalWidth(nCol,nTab) * fWidthFactor );
/*N*/ 				USHORT nColMerge = pMerge->GetColMerge();
/*N*/ 				if (nColMerge > 1)
/*N*/ 					for (USHORT nColAdd=1; nColAdd<nColMerge; nColAdd++)
/*N*/ 						nDocWidth += (long) ( pDocument->GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
/*N*/ 				nDocWidth -= (long) ( pMargin->GetLeftMargin() * fWidthFactor )
/*N*/ 						   + (long) ( pMargin->GetRightMargin() * fWidthFactor )
/*N*/ 						   + 1; 	// Ausgabebereich ist Breite-1 Pixel (wegen Gitterlinien)
/*N*/ 				if ( nIndent )
/*N*/ 					nDocWidth -= (long) ( nIndent * fWidthFactor );
/*N*/ 
/*N*/ 				// space for AutoFilter button:  20 * nZoom/100
/*N*/ 				if ( pFlag->HasAutoFilter() && !bTextWysiwyg )
/*?*/ 					nDocWidth -= (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
/*N*/ 
/*N*/ 				aPaper.Width() = nDocWidth;
/*N*/ 
/*N*/ 				if ( !bTextWysiwyg )
/*N*/ 					aPaper = pDev->PixelToLogic( aPaper, aHMMMode );
/*N*/ 			}
/*N*/ 			pEngine->SetPaperSize(aPaper);
/*N*/ 
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_EDIT )
/*N*/ 			{
/*N*/ 				const EditTextObject* pData;
/*N*/ 				((ScEditCell*)pCell)->GetData(pData);
/*N*/ 				pEngine->SetTextNewDefaults(*pData, pSet);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				Color* pColor;
/*N*/ 				SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 				ULONG nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
/*N*/ 				String aString;
/*N*/ 				ScCellFormat::GetString( pCell, nFormat, aString, &pColor,
/*N*/ 											*pFormatter,
/*N*/ 											TRUE, rOptions.bFormula, ftCheck );
/*N*/ 				if (aString.Len())
/*N*/ 					pEngine->SetTextNewDefaults(aString, pSet);
/*N*/ 				else
/*?*/ 					pEngine->SetDefaults(pSet);
/*N*/ 			}
/*N*/ 
/*N*/ 			BOOL bEngineVertical = pEngine->IsVertical();
/*N*/ 			pEngine->SetVertical( bAsianVertical );
/*N*/ 			pEngine->SetUpdateMode( TRUE );
/*N*/ 
/*N*/ 			BOOL bEdWidth = bWidth;
/*N*/ 			if ( eOrient != SVX_ORIENTATION_STANDARD && eOrient != SVX_ORIENTATION_STACKED )
/*N*/ 				bEdWidth = !bEdWidth;
/*N*/ 			if ( nRotate )
/*N*/ 			{
/*?*/ 				//!	unterschiedliche Skalierung X/Y beruecksichtigen
/*?*/ 
/*?*/ 				Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
/*?*/ 				double nRealOrient = nRotate * F_PI18000;	// nRotate sind 1/100 Grad
/*?*/ 				double nCosAbs = fabs( cos( nRealOrient ) );
/*?*/ 				double nSinAbs = fabs( sin( nRealOrient ) );
/*?*/ 				long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
/*?*/ 				long nWidth;
/*?*/ 				if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
/*?*/ 					nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
/*?*/ 				else if ( rOptions.bTotalSize )
/*?*/ 				{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 					nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					nWidth  = (long)( aSize.Height() / nSinAbs );	//! begrenzen?
/*?*/ 				aSize = Size( nWidth, nHeight );
/*?*/ 
/*?*/ 				Size aPixSize = pDev->LogicToPixel( aSize, aHMMMode );
/*?*/ 				if ( bEdWidth )
/*?*/ 					nValue = aPixSize.Width();
/*?*/ 				else
/*?*/ 				{
/*?*/ 					nValue = aPixSize.Height();
/*?*/ 
/*?*/ 					if ( bBreak && !rOptions.bTotalSize )
/*?*/ 					{
/*?*/ 						//	#47744# limit size for line break
/*?*/ 						long nCmp = aOldFont.GetSize().Height() * SC_ROT_BREAK_FACTOR;
/*?*/ 						if ( nValue > nCmp )
/*?*/ 							nValue = nCmp;
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else if ( bEdWidth )
/*N*/ 			{
/*N*/ 				if (bBreak)
/*N*/ 					nValue = 0;
/*N*/ 				else
/*N*/ 					nValue = pDev->LogicToPixel(Size( pEngine->CalcTextWidth(), 0 ),
/*N*/ 										aHMMMode).Width();
/*N*/ 			}
/*N*/ 			else			// Hoehe
/*N*/ 			{
/*N*/ 				nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
/*N*/ 									aHMMMode).Height();
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( nValue && bAddMargin )
/*N*/ 			{
/*N*/ 				if (bWidth)
/*N*/ 				{
/*N*/ 					nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
/*N*/ 							  (long) ( pMargin->GetRightMargin() * nPPT );
/*N*/ 					if (nIndent)
/*?*/ 						nValue += (long) ( nIndent * nPPT );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
/*N*/ 							  (long) ( pMargin->GetBottomMargin() * nPPT );
/*N*/ 
/*N*/ 					if ( bAsianVertical && pDev->GetOutDevType() != OUTDEV_PRINTER )
/*N*/ 					{
/*N*/ 						//	add 1pt extra (default margin value) for line breaks with SetVertical
/*N*/ 						nValue += (long) ( 20 * nPPT );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			//	EditEngine is cached and re-used, so the old vertical flag must be restored
/*N*/ 			pEngine->SetVertical( bEngineVertical );
/*N*/ 
/*N*/ 			pDocument->DisposeFieldEditEngine(pEngine);
/*N*/ 
/*N*/ 			pDev->SetMapMode( aOld );
/*N*/ 			pDev->SetFont( aOldFont );
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bWidth)
/*N*/ 		{
/*N*/ 			//		Platz fuer Autofilter-Button
/*N*/ 			//		20 * nZoom/100
/*N*/ 			//		bedingte Formatierung hier nicht interessant
/*N*/ 
/*N*/ 			INT16 nFlags = ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).GetValue();
/*N*/ 			if (nFlags & SC_MF_AUTO)
/*?*/ 				nValue += (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nValue;
/*N*/ }

/*N*/ long ScColumn::GetSimpleTextNeededSize( USHORT nIndex, OutputDevice* pDev,
/*N*/ 		BOOL bWidth )
/*N*/ {
/*N*/ 	long nValue=0;
/*N*/ 	if ( nIndex < nCount )
/*N*/ 	{
/*N*/ 		USHORT nRow = pItems[nIndex].nRow;
/*N*/ 		const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		String aValStr;
/*N*/ 		Color* pColor;
/*N*/ 		SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 		ULONG nFormat = pPattern->GetNumberFormat( pFormatter );
/*N*/ 		ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
/*N*/ 									*pFormatter, TRUE, FALSE, ftCheck );
/*N*/ 		if ( aValStr.Len() )
/*N*/ 		{
/*N*/ 			if ( bWidth )
/*N*/ 				nValue = pDev->GetTextWidth( aValStr );
/*N*/ 			else
/*?*/ 				nValue = pDev->GetTextHeight();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nValue;
/*N*/ }

/*N*/ USHORT ScColumn::GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
/*N*/ 										const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 										BOOL bFormula, USHORT nOldWidth,
/*N*/ 										const ScMarkData* pMarkData,
/*N*/ 										BOOL bSimpleTextImport )
/*N*/ {
/*N*/ 	if (nCount == 0)
/*N*/ 		return nOldWidth;
/*N*/ 
/*N*/ 	USHORT	nWidth = (USHORT) (nOldWidth * nPPTX);
/*N*/ 	BOOL	bFound = FALSE;
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	ScMarkedDataIter aDataIter(this, pMarkData, TRUE);
/*N*/ 	if ( bSimpleTextImport )
/*N*/ 	{	// alles eins bis auf NumberFormate
/*N*/ 		const ScPatternAttr* pPattern = GetPattern( 0 );
/*N*/ 		Font aFont;
/*N*/ 		// font color doesn't matter here
/*N*/ 		pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &rZoomX, NULL );
/*N*/ 		pDev->SetFont( aFont );
/*N*/ 		const SvxMarginItem* pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
/*N*/ 		long nMargin = (long) ( pMargin->GetLeftMargin() * nPPTX ) +
/*N*/ 						(long) ( pMargin->GetRightMargin() * nPPTX );
/*N*/ 
/*N*/ 		while (aDataIter.Next( nIndex ))
/*N*/ 		{
/*N*/ 			USHORT nThis = (USHORT) (GetSimpleTextNeededSize( nIndex, pDev,
/*N*/ 				TRUE ) + nMargin);
/*N*/ 			if (nThis)
/*N*/ 			{
/*N*/ 				if (nThis>nWidth || !bFound)
/*N*/ 				{
/*N*/ 					nWidth = nThis;
/*N*/ 					bFound = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ScNeededSizeOptions aOptions;
/*?*/ 		aOptions.bFormula = bFormula;
/*?*/ 		const ScPatternAttr* pOldPattern = NULL;
/*?*/ 		BYTE nOldScript = 0;
/*?*/ 
/*?*/ 		while (aDataIter.Next( nIndex ))
/*?*/ 		{
/*?*/ 			USHORT nRow = pItems[nIndex].nRow;
/*?*/ 
/*?*/ 			BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
/*?*/ 			if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();
/*?*/ 
/*?*/ 			const ScPatternAttr* pPattern = GetPattern( nRow );
/*?*/ 			aOptions.pPattern = pPattern;
/*?*/ 			aOptions.bGetFont = (pPattern != pOldPattern || nScript != nOldScript);
/*?*/ 			USHORT nThis = (USHORT) GetNeededSize( nRow, pDev, nPPTX, nPPTY,
/*?*/ 				rZoomX, rZoomY, TRUE, aOptions );
/*?*/ 			pOldPattern = pPattern;
/*?*/ 			if (nThis)
/*?*/ 			{
/*?*/ 				if (nThis>nWidth || !bFound)
/*?*/ 				{
/*?*/ 					nWidth = nThis;
/*?*/ 					bFound = TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bFound)
/*N*/ 	{
/*N*/ 		nWidth += 2;
/*N*/ 		USHORT nTwips = (USHORT) (nWidth / nPPTX);
/*N*/ 		return nTwips;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return nOldWidth;
/*N*/ }

/*N*/ USHORT lcl_GetAttribHeight( const ScPatternAttr& rPattern, USHORT nFontHeightId )
/*N*/ {
/*N*/ 	USHORT nHeight = (USHORT) ((const SvxFontHeightItem&) rPattern.GetItem(nFontHeightId)).GetHeight();
/*N*/ 	const SvxMarginItem* pMargin = (const SvxMarginItem*) &rPattern.GetItem(ATTR_MARGIN);
/*N*/ 	nHeight += nHeight / 5;
/*N*/ 	//	gibt bei 10pt 240
/*N*/ 
/*N*/ 	if ( ((const SvxEmphasisMarkItem&)rPattern.
/*N*/ 			GetItem(ATTR_FONT_EMPHASISMARK)).GetEmphasisMark() != EMPHASISMARK_NONE )
/*N*/ 	{
/*N*/ 		//	add height for emphasis marks
/*N*/ 		//!	font metrics should be used instead
/*N*/ 		nHeight += nHeight / 4;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nHeight + 240 > ScGlobal::nDefFontHeight )
/*N*/ 	{
/*N*/ 		nHeight += ScGlobal::nDefFontHeight;
/*N*/ 		nHeight -= 240;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Standard-Hoehe: TextHeight + Raender - 23
/*N*/ 	//	-> 257 unter Windows
/*N*/ 
/*N*/ 	if (nHeight > STD_ROWHEIGHT_DIFF)
/*N*/ 		nHeight -= STD_ROWHEIGHT_DIFF;
/*N*/ 
/*N*/ 	nHeight += pMargin->GetTopMargin() + pMargin->GetBottomMargin();
/*N*/ 
/*N*/ 	return nHeight;
/*N*/ }

//	pHeight in Twips
//	nMinHeight, nMinStart zur Optimierung: ab nRow >= nMinStart ist mindestens nMinHeight
//	(wird nur bei bStdAllowed ausgewertet)

/*N*/ void ScColumn::GetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT* pHeight,
/*N*/ 								OutputDevice* pDev,
/*N*/ 								double nPPTX, double nPPTY,
/*N*/ 								const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 								BOOL bShrink, USHORT nMinHeight, USHORT nMinStart )
/*N*/ {
/*N*/ 	ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );
/*N*/ 
/*N*/ 	USHORT nStart;
/*N*/ 	USHORT nEnd;
/*N*/ 	USHORT nEditPos = 0;
/*N*/ 	USHORT nNextEnd = 0;
/*N*/ 
/*N*/ 	//	bei bedingter Formatierung werden immer die einzelnen Zellen angesehen
/*N*/ 
/*N*/ 	const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
/*N*/ 	while ( pPattern )
/*N*/ 	{
/*N*/ 		const ScMergeAttr*		pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
/*N*/ 		const ScMergeFlagAttr*	pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
/*N*/ 		if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
/*N*/ 		{
/*N*/ 			//	nix - vertikal bei der zusammengefassten und den ueberdeckten,
/*N*/ 			//		  horizontal nur bei den ueberdeckten (unsichtbaren) -
/*N*/ 			//		  eine nur horizontal zusammengefasste wird aber beruecksichtigt
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			USHORT nRow;
/*N*/ 			BOOL bStdAllowed = ((const SvxOrientationItem&) pPattern->GetItem(ATTR_ORIENTATION)).
/*N*/ 										GetValue() == (USHORT) SVX_ORIENTATION_STANDARD;
/*N*/ 			BOOL bStdOnly = FALSE;
/*N*/ 			if (bStdAllowed)
/*N*/ 			{
/*N*/ 				BOOL bBreak = ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue() ||
/*N*/ 								((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
/*N*/ 									GetItem( ATTR_HOR_JUSTIFY )).GetValue() ==
/*N*/ 									SVX_HOR_JUSTIFY_BLOCK);
/*N*/ 				bStdOnly = !bBreak;
/*N*/ 
/*N*/ 				// bedingte Formatierung: Zellen durchgehen
/*N*/ 				if ( bStdOnly && ((const SfxUInt32Item&)pPattern->
/*N*/ 									GetItem(ATTR_CONDITIONAL)).GetValue() )
/*N*/ 					bStdOnly = FALSE;
/*N*/ 
/*N*/ 				// gedrehter Text: Zellen durchgehen
/*N*/ 				if ( bStdOnly && ((const SfxInt32Item&)pPattern->
/*N*/ 									GetItem(ATTR_ROTATE_VALUE)).GetValue() )
/*N*/ 					bStdOnly = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			if (bStdOnly)
/*N*/ 				if (HasEditCells(nStart,nEnd,nEditPos))		// includes mixed script types
/*N*/ 				{
/*N*/ 					if (nEditPos == nStart)
/*N*/ 					{
/*N*/ 						bStdOnly = FALSE;
/*N*/ 						if (nEnd > nEditPos)
/*N*/ 							nNextEnd = nEnd;
/*N*/ 						nEnd = nEditPos;				// einzeln ausrechnen
/*N*/ 						bStdAllowed = FALSE;			// wird auf jeden Fall per Zelle berechnet
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						nNextEnd = nEnd;
/*N*/ 						nEnd = nEditPos - 1;			// Standard - Teil
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 			if (bStdAllowed)
/*N*/ 			{
/*N*/ 				USHORT nLatHeight = 0;
/*N*/ 				USHORT nCjkHeight = 0;
/*N*/ 				USHORT nCtlHeight = 0;
/*N*/ 				USHORT nDefHeight;
/*N*/ 				BYTE nDefScript = ScGlobal::GetDefaultScriptType();
/*N*/ 				if ( nDefScript == SCRIPTTYPE_ASIAN )
/*N*/ 					nDefHeight = nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
/*N*/ 				else if ( nDefScript == SCRIPTTYPE_COMPLEX )
/*?*/ 					nDefHeight = nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
/*N*/ 				else
/*?*/ 					nDefHeight = nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );
/*N*/ 
/*N*/ 				//	if everything below is already larger, the loop doesn't have to
/*N*/ 				//	be run again
/*N*/ 				USHORT nStdEnd = nEnd;
/*N*/ 				if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
/*N*/ 					nStdEnd = nMinStart ? nMinStart-1 : 0;
/*N*/ 
/*N*/ 				for (nRow=nStart; nRow<=nStdEnd; nRow++)
/*N*/ 					if (nDefHeight > pHeight[nRow-nStartRow])
/*N*/ 						pHeight[nRow-nStartRow] = nDefHeight;
/*N*/ 
/*N*/ 				if ( bStdOnly )
/*N*/ 				{
/*N*/ 					//	if cells are not handled individually below,
/*N*/ 					//	check for cells with different script type
/*N*/ 
/*N*/ 					USHORT nIndex;
/*N*/ 					Search(nStart,nIndex);
/*N*/ 					while ( nIndex < nCount && (nRow=pItems[nIndex].nRow) <= nEnd )
/*N*/ 					{
/*N*/ 						BYTE nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
/*N*/ 						if ( nScript != nDefScript )
/*N*/ 						{
/*N*/ 							if ( nScript == SCRIPTTYPE_ASIAN )
/*N*/ 							{
/*?*/ 								if ( nCjkHeight == 0 )
/*?*/ 									nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
/*?*/ 								if (nCjkHeight > pHeight[nRow-nStartRow])
/*?*/ 									pHeight[nRow-nStartRow] = nCjkHeight;
/*N*/ 							}
/*N*/ 							else if ( nScript == SCRIPTTYPE_COMPLEX )
/*N*/ 							{
/*?*/ 								if ( nCtlHeight == 0 )
/*?*/ 									nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
/*?*/ 								if (nCtlHeight > pHeight[nRow-nStartRow])
/*?*/ 									pHeight[nRow-nStartRow] = nCtlHeight;
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								if ( nLatHeight == 0 )
/*N*/ 									nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );
/*N*/ 								if (nLatHeight > pHeight[nRow-nStartRow])
/*N*/ 									pHeight[nRow-nStartRow] = nLatHeight;
/*N*/ 							}
/*N*/ 						}
/*N*/ 						++nIndex;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if (!bStdOnly)						// belegte Zellen suchen
/*N*/ 			{
/*N*/ 				ScNeededSizeOptions aOptions;
/*N*/ 
/*N*/ 				USHORT nIndex;
/*N*/ 				Search(nStart,nIndex);
/*N*/ 				while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEnd) : FALSE )
/*N*/ 				{
/*N*/ 					//	Zellhoehe nur berechnen, wenn sie spaeter auch gebraucht wird (#37928#)
/*N*/ 
/*N*/ 					if ( bShrink || !(pDocument->GetRowFlags(nRow, nTab) & CR_MANUALSIZE) )
/*N*/ 					{
/*N*/ 						aOptions.pPattern = pPattern;
/*N*/ 						USHORT nHeight = (USHORT)
/*N*/ 								( GetNeededSize( nRow, pDev, nPPTX, nPPTY,
/*N*/ 													rZoomX, rZoomY, FALSE, aOptions ) / nPPTY );
/*N*/ 						if (nHeight > pHeight[nRow-nStartRow])
/*N*/ 							pHeight[nRow-nStartRow] = nHeight;
/*N*/ 					}
/*N*/ 					++nIndex;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (nNextEnd)
/*N*/ 		{
/*N*/ 			nStart = nEnd + 1;
/*N*/ 			nEnd = nNextEnd;
/*N*/ 			nNextEnd = 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pPattern = aIter.Next(nStart,nEnd);
/*N*/ 	}
/*N*/ }


// =========================================================================================



// =========================================================================================

/*N*/ BOOL ScColumn::TestTabRefAbs(USHORT nTable)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 			if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
/*?*/ 				if (((ScFormulaCell*)pItems[i].pCell)->TestTabRefAbs(nTable))
/*?*/ 					bRet = TRUE;
/*N*/ 	return bRet;
/*N*/ }

// =========================================================================================

/*N*/ ScColumnIterator::ScColumnIterator( const ScColumn* pCol, USHORT nStart, USHORT nEnd ) :
/*N*/ 	pColumn( pCol ),
/*N*/ 	nTop( nStart ),
/*N*/ 	nBottom( nEnd )
/*N*/ {
/*N*/ 	pColumn->Search( nTop, nPos );
/*N*/ }

/*N*/ ScColumnIterator::~ScColumnIterator()
/*N*/ {
/*N*/ }

/*N*/ BOOL ScColumnIterator::Next( USHORT& rRow, ScBaseCell*& rpCell )
/*N*/ {
/*N*/ 	if ( nPos < pColumn->nCount )
/*N*/ 	{
/*N*/ 		rRow = pColumn->pItems[nPos].nRow;
/*N*/ 		if ( rRow <= nBottom )
/*N*/ 		{
/*N*/ 			rpCell = pColumn->pItems[nPos].pCell;
/*N*/ 			++nPos;
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rRow = 0;
/*N*/ 	rpCell = NULL;
/*N*/ 	return FALSE;
/*N*/ }

// -----------------------------------------------------------------------------------------

/*N*/ ScMarkedDataIter::ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
/*N*/ 									BOOL bAllIfNone ) :
/*N*/ 	pColumn( pCol ),
/*N*/ 	pMarkIter( NULL ),
/*N*/ 	bNext( TRUE ),
/*N*/ 	bAll( bAllIfNone )
/*N*/ {
/*N*/ 	if (pMarkData && pMarkData->IsMultiMarked())
/*N*/ 		pMarkIter = new ScMarkArrayIter( pMarkData->GetArray() + pCol->GetCol() );
/*N*/ }

/*N*/ ScMarkedDataIter::~ScMarkedDataIter()
/*N*/ {
/*N*/ 	delete pMarkIter;
/*N*/ }

/*N*/ BOOL ScMarkedDataIter::Next( USHORT& rIndex )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if (bNext)
/*N*/ 		{
/*N*/ 			if (!pMarkIter || !pMarkIter->Next( nTop, nBottom ))
/*N*/ 			{
/*N*/ 				if (bAll)					// ganze Spalte
/*N*/ 				{
/*N*/ 					nTop	= 0;
/*N*/ 					nBottom	= MAXROW;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					return FALSE;
/*N*/ 			}
/*N*/ 			pColumn->Search( nTop, nPos );
/*N*/ 			bNext = FALSE;
/*N*/ 			bAll  = FALSE;					// nur beim ersten Versuch
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( nPos >= pColumn->nCount )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		if ( pColumn->pItems[nPos].nRow <= nBottom )
/*N*/ 			bFound = TRUE;
/*N*/ 		else
/*N*/ 			bNext = TRUE;
/*N*/ 	}
/*N*/ 	while (!bFound);
/*N*/ 
/*N*/ 	rIndex = nPos++;
/*N*/ 	return TRUE;
/*N*/ }


//------------

/*N*/ BOOL ScColumn::IsEmptyData() const
/*N*/ {
/*N*/ 	return (nCount == 0);
/*N*/ }

/*N*/ BOOL ScColumn::IsEmptyVisData(BOOL bNotes) const
/*N*/ {
/*N*/ 	if (!pItems || nCount == 0)
/*N*/ 		return TRUE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bVisData = FALSE;
/*N*/ 		USHORT i;
/*N*/ 		for (i=0; i<nCount && !bVisData; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
/*N*/ 				bVisData = TRUE;
/*N*/ 		}
/*N*/ 		return !bVisData;
/*N*/ 	}
/*N*/ }


/*N*/ USHORT ScColumn::GetLastVisDataPos(BOOL bNotes) const
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 		for (i=nCount; i>0 && !bFound; )
/*N*/ 		{
/*N*/ 			--i;
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
/*N*/ 			{
/*N*/ 				bFound = TRUE;
/*N*/ 				nRet = pItems[i].nRow;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ USHORT ScColumn::GetFirstVisDataPos(BOOL bNotes) const
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 		for (i=0; i<nCount && !bFound; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->GetNotePtr()) )
/*N*/ 			{
/*N*/ 				bFound = TRUE;
/*N*/ 				nRet = pItems[i].nRow;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }


/*N*/ BOOL ScColumn::IsEmptyAttr() const
/*N*/ {
/*N*/ 	if (pAttrArray)
/*N*/ 		return pAttrArray->IsEmpty();
/*N*/ 	else
/*N*/ 		return TRUE;
/*N*/ }

/*N*/ BOOL ScColumn::IsEmpty() const
/*N*/ {
/*N*/ 	return (IsEmptyData() && IsEmptyAttr());
/*N*/ }








/*N*/ BOOL ScColumn::HasDataAt(USHORT nRow) const
/*N*/ {
/*	USHORT nIndex;
    return Search( nRow, nIndex );
*/
/*N*/ 		//	immer nur sichtbare interessant ?
/*N*/ 		//!	dann HasVisibleDataAt raus
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		if (CellVisible(pItems[nIndex].pCell))
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ 
/*N*/ }




/*N*/ BOOL ScColumn::IsAllAttrEqual( const ScColumn& rCol, USHORT nStartRow, USHORT nEndRow ) const
/*N*/ {
/*N*/ 	if (pAttrArray && rCol.pAttrArray)
/*N*/ 		return pAttrArray->IsAllEqual( *rCol.pAttrArray, nStartRow, nEndRow );
/*N*/ 	else
/*N*/ 		return !pAttrArray && !rCol.pAttrArray;
/*N*/ }

/*N*/ BOOL ScColumn::IsVisibleAttrEqual( const ScColumn& rCol, USHORT nStartRow, USHORT nEndRow ) const
/*N*/ {
/*N*/ 	if (pAttrArray && rCol.pAttrArray)
/*N*/ 		return pAttrArray->IsVisibleEqual( *rCol.pAttrArray, nStartRow, nEndRow );
/*N*/ 	else
/*N*/ 		return !pAttrArray && !rCol.pAttrArray;
/*N*/ }

/*N*/ BOOL ScColumn::HasVisibleAttr( USHORT& rFirstRow, USHORT& rLastRow, BOOL bSkipFirst ) const
/*N*/ {
/*N*/ 	if (pAttrArray)
/*N*/ 		return pAttrArray->HasVisibleAttr(rFirstRow,rLastRow,bSkipFirst);
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }



/*N*/ void ScColumn::StartListening( SfxListener& rLst, USHORT nRow )
/*N*/ {
/*N*/ 	ScBroadcasterList* pBC = NULL;
/*N*/ 	ScBaseCell* pCell;
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nRow,nIndex))
/*N*/ 	{
/*N*/ 		pCell = pItems[nIndex].pCell;
/*N*/ 		pBC = pCell->GetBroadcaster();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pCell = new ScNoteCell;
/*N*/ 		Insert(nRow, pCell);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!pBC)
/*N*/ 	{
/*N*/ 		pBC = new ScBroadcasterList;
/*N*/ 		pCell->SetBroadcaster(pBC);
/*N*/ 	}
/*N*/ //	rLst.StartListening(*pBC,TRUE);
/*N*/ 	pBC->StartBroadcasting( rLst, TRUE );
/*N*/ }

/*N*/ void ScColumn::MoveListeners( ScBroadcasterList& rSource, USHORT nDestRow )
/*N*/ {
/*N*/ 	ScBroadcasterList* pBC = NULL;
/*N*/ 	ScBaseCell* pCell;
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nDestRow,nIndex))
/*N*/ 	{
/*N*/ 		pCell = pItems[nIndex].pCell;
/*N*/ 		pBC = pCell->GetBroadcaster();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pCell = new ScNoteCell;
/*N*/ 		Insert(nDestRow, pCell);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!pBC)
/*N*/ 	{
/*N*/ 		pBC = new ScBroadcasterList;
/*N*/ 		pCell->SetBroadcaster(pBC);
/*N*/ 	}
/*N*/ 
/*N*/ 	rSource.MoveListenersTo( *pBC );
/*N*/ }

/*N*/ void ScColumn::EndListening( SfxListener& rLst, USHORT nRow )
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nRow,nIndex))
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		ScBroadcasterList* pBC = pCell->GetBroadcaster();
/*N*/ 		if (pBC)
/*N*/ 		{
/*N*/ //			rLst.EndListening(*pBC);
/*N*/ 			pBC->EndBroadcasting(rLst);
/*N*/ 
/*N*/ 			if (!pBC->HasListeners())
/*N*/ 			{
/*N*/ 				if (pCell->GetCellType() == CELLTYPE_NOTE && !pCell->GetNotePtr())
/*N*/ 					DeleteAtIndex(nIndex);
/*N*/ 				else
/*N*/ 					pCell->SetBroadcaster(NULL);
/*N*/ 			}
/*N*/ 		}
/*N*/ //		else
/*N*/ //			DBG_ERROR("ScColumn::EndListening - kein Broadcaster");
/*N*/ 	}
/*N*/ //	else
/*N*/ //		DBG_ERROR("ScColumn::EndListening - keine Zelle");
/*N*/ }


/*N*/ void ScColumn::CompileDBFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 				((ScFormulaCell*) pCell)->CompileDBFormula( bCreateFormulaString );
/*N*/ 		}
/*N*/ }

/*N*/ void ScColumn::CompileNameFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 				((ScFormulaCell*) pCell)->CompileNameFormula( bCreateFormulaString );
/*N*/ 		}
/*N*/ }

/*N*/ void ScColumn::CompileColRowNameFormula()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 				((ScFormulaCell*) pCell)->CompileColRowNameFormula();
/*N*/ 		}
/*N*/ }

/*N*/ void lcl_UpdateSubTotal( ScFunctionData& rData, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	double nValue;
/*N*/ 	BOOL bVal = FALSE;
/*N*/ 	BOOL bCell = TRUE;
/*N*/ 	switch (pCell->GetCellType())
/*N*/ 	{
/*?*/ 		case CELLTYPE_VALUE:
/*?*/ 			nValue = ((ScValueCell*)pCell)->GetValue();
/*?*/ 			bVal = TRUE;
/*?*/ 			break;
/*?*/ 		case CELLTYPE_FORMULA:
/*?*/ 			{
/*?*/ 				if ( rData.eFunc != SUBTOTAL_FUNC_CNT2 )		// da interessiert's nicht
/*?*/ 				{
/*?*/ 					ScFormulaCell* pFC = (ScFormulaCell*)pCell;
/*?*/ 					if ( pFC->GetErrCode() )
/*?*/ 					{
/*?*/ 						if ( rData.eFunc != SUBTOTAL_FUNC_CNT )	// fuer Anzahl einfach weglassen
/*?*/ 							rData.bError = TRUE;
/*?*/ 					}
/*?*/ 					else if (pFC->IsValue())
/*?*/ 					{
/*?*/ 						nValue = pFC->GetValue();
/*?*/ 						bVal = TRUE;
/*?*/ 					}
/*?*/ 					// sonst Text
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		case CELLTYPE_NOTE:
/*?*/ 			bCell = FALSE;
/*?*/ 			break;
/*?*/ 		// bei Strings nichts
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!rData.bError)
/*N*/ 	{
/*N*/ 		switch (rData.eFunc)
/*N*/ 		{
/*?*/ 			case SUBTOTAL_FUNC_SUM:
/*?*/ 			case SUBTOTAL_FUNC_AVE:
/*?*/ 				if (bVal)
/*?*/ 				{
/*?*/ 					++rData.nCount;
/*?*/ 					if (!SubTotal::SafePlus( rData.nVal, nValue ))
/*?*/ 						rData.bError = TRUE;
/*?*/ 				}
/*?*/ 				break;
/*?*/ 			case SUBTOTAL_FUNC_CNT:				// nur Werte
/*?*/ 				if (bVal)
/*?*/ 					++rData.nCount;
/*?*/ 				break;
/*N*/ 			case SUBTOTAL_FUNC_CNT2:			// alle
/*N*/ 				if (bCell)
/*N*/ 					++rData.nCount;
/*N*/ 				break;
/*?*/ 			case SUBTOTAL_FUNC_MAX:
/*?*/ 				if (bVal)
/*?*/ 					if (++rData.nCount == 1 || nValue > rData.nVal )
/*?*/ 						rData.nVal = nValue;
/*?*/ 				break;
/*?*/ 			case SUBTOTAL_FUNC_MIN:
/*?*/ 				if (bVal)
/*?*/ 					if (++rData.nCount == 1 || nValue < rData.nVal )
/*?*/ 						rData.nVal = nValue;
/*?*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//	Mehrfachselektion:
/*N*/ void ScColumn::UpdateSelectionFunction( const ScMarkData& rMark,
/*N*/ 										ScFunctionData& rData, const BYTE* pRowFlags,
/*N*/ 										BOOL bDoExclude, USHORT nExStartRow, USHORT nExEndRow )
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	ScMarkedDataIter aDataIter(this, &rMark, FALSE);
/*N*/ 	while (aDataIter.Next( nIndex ))
/*N*/ 	{
/*N*/ 		USHORT nRow = pItems[nIndex].nRow;
/*N*/ 		if ( !pRowFlags || !( pRowFlags[nRow] & CR_HIDDEN ) )
/*N*/ 			if ( !bDoExclude || nRow < nExStartRow || nRow > nExEndRow )
/*N*/ 				lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
/*N*/ 	}
/*N*/ }

//	bei bNoMarked die Mehrfachselektion weglassen
/*N*/ void ScColumn::UpdateAreaFunction( ScFunctionData& rData, BYTE* pRowFlags,
/*N*/ 									USHORT nStartRow, USHORT nEndRow )
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	Search( nStartRow, nIndex );
/*N*/ 	while ( nIndex<nCount && pItems[nIndex].nRow<=nEndRow )
/*N*/ 	{
/*N*/ 		USHORT nRow = pItems[nIndex].nRow;
/*N*/ 		if ( !pRowFlags || !( pRowFlags[nRow] & CR_HIDDEN ) )
/*N*/ 			lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
/*N*/ 		++nIndex;
/*N*/ 	}
/*N*/ }

/*N*/ long ScColumn::GetWeightedCount() const
/*N*/ {
/*N*/ 	long nTotal = 0;
/*N*/ 
/*N*/ 	//	Notizen werden nicht gezaehlt
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 		switch ( pCell->GetCellType() )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE:
/*N*/ 			case CELLTYPE_STRING:
/*N*/ 				++nTotal;
/*N*/ 				break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 				nTotal += 5 + ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
/*N*/ 				break;
/*N*/ 			case CELLTYPE_EDIT:
/*N*/ 				nTotal += 50;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nTotal;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
