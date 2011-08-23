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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifdef MAC
    // StackSpace
#include <mac_start.h>
#include <Memory.h>
#include <fp.h>
#include <mac_end.h>
#endif

#if defined (SOLARIS) || defined (FREEBSD)
#include <ieeefp.h>
#elif ( defined ( LINUX ) && ( GLIBC < 2 ) )
#include <i386/ieeefp.h>
#endif

#include <bf_svtools/zforlist.hxx>
#include <float.h>			// _finite

#include "interpre.hxx"
#include "scmatrix.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "rangenam.hxx"
#include "progress.hxx"
#include "bclist.hxx"
namespace binfilter {

// jetzt fuer alle Systeme niedriger Wert, Rest wird per FormulaTree ausgebuegelt,
// falls wirklich eine tiefe Rekursion ist, geht das schneller, als zu versuchen
// und dann doch nicht zu schaffen..
#define SIMPLEMAXRECURSION
#ifdef SIMPLEMAXRECURSION
 #define MAXRECURSION 50
#else
#if defined( WIN ) || defined( OS2 )
 #define MAXRECURSION 50
#elif defined( MAC )
 // wird per StackSpace() ermittelt
#else
 #define MAXRECURSION 1000
#endif
#endif

// STATIC DATA -----------------------------------------------------------

#ifdef USE_MEMPOOL
// MemPools auf 4k Boundaries  - 64 Bytes ausrichten
/*N*/ const USHORT nMemPoolValueCell = (0x8000 - 64) / sizeof(ScValueCell);
/*N*/ const USHORT nMemPoolFormulaCell = (0x8000 - 64) / sizeof(ScFormulaCell);
/*N*/ const USHORT nMemPoolStringCell = (0x4000 - 64) / sizeof(ScStringCell);
/*N*/ const USHORT nMemPoolNoteCell = (0x1000 - 64) / sizeof(ScNoteCell);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScValueCell,   nMemPoolValueCell, nMemPoolValueCell )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell, nMemPoolFormulaCell, nMemPoolFormulaCell )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScStringCell,  nMemPoolStringCell, nMemPoolStringCell )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScNoteCell,	 nMemPoolNoteCell, nMemPoolNoteCell )
#endif

#ifdef _MSC_VER
#pragma code_seg()
#endif

INT8 ScFormulaCell::nIterMode = 0;

#ifdef DBG_UTIL
static const sal_Char __FAR_DATA msgDbgInfinity[] =
    "Formelzelle INFINITY ohne Err503 !!! (os/2?)\n"
    "NICHTS anruehren und ER bescheid sagen!";
#endif

// -----------------------------------------------------------------------

DECLARE_LIST (ScFormulaCellList, ScFormulaCell*)//STRIP008 ;


/*N*/ ScBaseCell* ScBaseCell::Clone(ScDocument* pDoc) const
/*N*/ {
/*N*/ 	switch (eCellType)
/*N*/ 	{
/*N*/ 		case CELLTYPE_VALUE:
/*N*/ 			return new ScValueCell(*(const ScValueCell*)this);
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			return new ScStringCell(*(const ScStringCell*)this);
/*N*/ 		case CELLTYPE_EDIT:
/*N*/ 			return new ScEditCell(*(const ScEditCell*)this, pDoc);
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			return new ScFormulaCell(pDoc, ((ScFormulaCell*)this)->aPos,
/*N*/ 				*(const ScFormulaCell*)this);
/*N*/ 		case CELLTYPE_NOTE:
/*N*/ 			return new ScNoteCell(*(const ScNoteCell*)this);
/*N*/ 		default:
/*N*/ 			DBG_ERROR("Unbekannter Zellentyp");
/*N*/ 			return NULL;
/*N*/ 	}
/*N*/ }

/*N*/ ScBaseCell::~ScBaseCell()
/*N*/ {
/*N*/ 	delete pNote;
/*N*/ 	delete pBroadcaster;
/*N*/ 	DBG_ASSERT( eCellType == CELLTYPE_DESTROYED, "BaseCell Destructor" );
/*N*/ }

/*N*/ void ScBaseCell::Delete()
/*N*/ {
/*N*/ 	DELETEZ(pNote);
/*N*/ 	switch (eCellType)
/*N*/ 	{
/*N*/ 		case CELLTYPE_VALUE:
/*N*/ 			delete (ScValueCell*) this;
/*N*/ 			break;
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			delete (ScStringCell*) this;
/*N*/ 			break;
/*N*/ 		case CELLTYPE_EDIT:
/*N*/ 			delete (ScEditCell*) this;
/*N*/ 			break;
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			delete (ScFormulaCell*) this;
/*N*/ 			break;
/*N*/ 		case CELLTYPE_NOTE:
/*N*/ 			delete (ScNoteCell*) this;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("Unbekannter Zellentyp");
/*N*/ 			break;
/*N*/ 	}
/*N*/ }

/*N*/ void ScBaseCell::SetNote( const ScPostIt& rNote )
/*N*/ {
/*N*/ 	if (rNote.GetText().Len() > 0)
/*N*/ 	{
/*N*/ 		if (!pNote)
/*N*/ 			pNote = new ScPostIt(rNote);
/*N*/ 		else
/*N*/ 			*pNote = rNote;
/*N*/ 	}
/*N*/ 	else
/*?*/ 		DELETEZ(pNote);
/*N*/ }

/*N*/ BOOL ScBaseCell::GetNote( ScPostIt& rNote ) const
/*N*/ {
/*N*/ 	if ( pNote )
/*N*/ 		rNote = *pNote;
/*N*/ 	else
/*?*/ 		rNote.Clear();
/*N*/ 
/*N*/ 	return ( pNote != NULL );
/*N*/ }

/*N*/ ScBaseCell* ScBaseCell::CreateTextCell( const String& rString, ScDocument* pDoc )
/*N*/ {
/*N*/ 	if ( rString.Search('\n') != STRING_NOTFOUND || rString.Search(CHAR_CR) != STRING_NOTFOUND )
/*?*/ 		return new ScEditCell( rString, pDoc );
/*N*/ 	else
/*N*/ 		return new ScStringCell( rString );
/*N*/ }

/*N*/ void ScBaseCell::LoadNote( SvStream& rStream )
/*N*/ {
/*N*/ 	pNote = new ScPostIt;
/*N*/ 	rStream >> *pNote;
/*N*/ }

/*N*/ void ScBaseCell::SetBroadcaster(ScBroadcasterList* pNew)
/*N*/ {
/*N*/ 	delete pBroadcaster;
/*N*/ 	pBroadcaster = pNew;
/*N*/ }

/*M*/ void ScBaseCell::StartListeningTo( ScDocument* pDoc, USHORT nOnlyNames )
/*M*/ {
/*M*/ 	if ( eCellType == CELLTYPE_FORMULA && !pDoc->IsClipOrUndo()
/*M*/ 			&& !pDoc->GetNoListening()
/*M*/ 			&& !((ScFormulaCell*)this)->IsInChangeTrack()
/*M*/ 		)
/*M*/ 	{
/*M*/ 		pDoc->SetDetectiveDirty(TRUE);	// es hat sich was geaendert...
/*M*/ 
/*M*/ 		ScFormulaCell* pFormCell = (ScFormulaCell*)this;
/*M*/         ScTokenArray* pArr = pFormCell->GetCode();
/*M*/ 		if( pArr->IsRecalcModeAlways() )
/*M*/ 			pDoc->StartListeningArea( BCA_LISTEN_ALWAYS, pFormCell );
/*M*/ 		else
/*M*/ 		{
/*M*/             if ( nOnlyNames && ((nOnlyNames & SC_LISTENING_EXCEPT) == 0) &&
/*M*/                     pArr->IsReplacedSharedFormula() )
/*M*/                 nOnlyNames = 0;
/*M*/ 			pArr->Reset();
/*M*/ 			for( ScToken* t = pArr->GetNextReferenceRPN(); t;
/*M*/ 						  t = pArr->GetNextReferenceRPN() )
/*M*/ 			{
/*M*/ 				StackVar eType = t->GetType();
/*M*/ 				SingleRefData& rRef1 = t->GetSingleRef();
/*M*/ 				SingleRefData& rRef2 = (eType == svDoubleRef ?
/*M*/ 					t->GetDoubleRef().Ref2 : rRef1);
/*M*/                 BOOL bDo = FALSE;
/*M*/                 if ( !nOnlyNames )
/*M*/                     bDo = TRUE;
/*M*/                 else
/*M*/                 {
/*M*/                     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bDo = pArr->IsReplacedSharedFormula();
/*M*/                 }
/*M*/                 if ( bDo )
/*M*/ 				{
/*M*/ 					switch( eType )
/*M*/ 					{
/*M*/ 						case svSingleRef:
/*M*/ 							rRef1.CalcAbsIfRel( pFormCell->aPos );
/*M*/ 							if ( rRef1.Valid() )
/*M*/ 							{
/*M*/ 								pDoc->StartListeningCell(
/*M*/ 									ScAddress( rRef1.nCol,
/*M*/ 											   rRef1.nRow,
/*M*/ 											   rRef1.nTab ), pFormCell );
/*M*/ 							}
/*M*/ 						break;
/*M*/ 						case svDoubleRef:
/*M*/ 							t->CalcAbsIfRel( pFormCell->aPos );
/*M*/ 							if ( rRef1.Valid() && rRef2.Valid() )
/*M*/ 							{
/*M*/ 								if ( t->GetOpCode() == ocColRowNameAuto )
/*M*/ 								{	// automagically
/*?*/ 									if ( rRef1.IsColRel() )
/*?*/ 									{	// ColName
/*?*/ 										pDoc->StartListeningArea( ScRange (
/*?*/ 											0,
/*?*/ 											rRef1.nRow,
/*?*/ 											rRef1.nTab,
/*?*/ 											MAXCOL,
/*?*/ 											rRef2.nRow,
/*?*/ 											rRef2.nTab ), pFormCell );
/*?*/ 									}
/*?*/ 									else
/*?*/ 									{	// RowName
/*?*/ 										pDoc->StartListeningArea( ScRange (
/*?*/ 											rRef1.nCol,
/*?*/ 											0,
/*?*/ 											rRef1.nTab,
/*?*/ 											rRef2.nCol,
/*?*/ 											MAXROW,
/*?*/ 											rRef2.nTab ), pFormCell );
/*?*/ 									}
/*M*/ 								}
/*M*/ 								else
/*M*/ 								{
/*M*/ 									pDoc->StartListeningArea( ScRange (
/*M*/ 										rRef1.nCol,
/*M*/ 										rRef1.nRow,
/*M*/ 										rRef1.nTab,
/*M*/ 										rRef2.nCol,
/*M*/ 										rRef2.nRow,
/*M*/ 										rRef2.nTab ), pFormCell );
/*M*/ 								}
/*M*/ 							}
/*M*/ 						break;
/*M*/ 					}
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/         pArr->SetReplacedSharedFormula( FALSE );
/*M*/ 	}
/*M*/ }

//	pArr gesetzt -> Referenzen von anderer Zelle nehmen
// dann muss auch aPos uebergeben werden!

/*N*/ void ScBaseCell::EndListeningTo( ScDocument* pDoc, USHORT nOnlyNames,
/*N*/ 		ScTokenArray* pArr, ScAddress aPos )
/*N*/ {
/*N*/ 	if ( eCellType == CELLTYPE_FORMULA && !pDoc->IsClipOrUndo()
/*N*/ 			&& !((ScFormulaCell*)this)->IsInChangeTrack()
/*N*/ 		)
/*N*/ 	{
/*N*/ 		pDoc->SetDetectiveDirty(TRUE);	// es hat sich was geaendert...
/*N*/ 
/*N*/ 		ScFormulaCell* pFormCell = (ScFormulaCell*)this;
/*N*/ 		if( pFormCell->GetCode()->IsRecalcModeAlways() )
/*?*/ 			pDoc->EndListeningArea( BCA_LISTEN_ALWAYS, pFormCell );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (!pArr)
/*N*/ 			{
/*N*/ 				pArr = pFormCell->GetCode();
/*N*/ 				aPos = pFormCell->aPos;
/*N*/ 			}
/*N*/ 			pArr->Reset();
/*N*/ 			for( ScToken* t = pArr->GetNextReferenceRPN(); t;
/*N*/ 						  t = pArr->GetNextReferenceRPN() )
/*N*/ 			{
/*N*/ 				StackVar eType = t->GetType();
/*N*/ 				SingleRefData& rRef1 = t->GetSingleRef();
/*N*/ 				SingleRefData& rRef2 = (eType == svDoubleRef ?
/*N*/ 					t->GetDoubleRef().Ref2 : rRef1);
/*N*/                 BOOL bDo;
/*N*/                 if ( !nOnlyNames )
/*N*/                     bDo = TRUE;
/*N*/                 else
/*N*/                 {
/*?*/                     bDo = FALSE;
/*?*/                     if ( nOnlyNames & SC_LISTENING_NAMES_REL )
/*?*/                         bDo |= (rRef1.IsRelName() || rRef2.IsRelName());
/*?*/                     if ( nOnlyNames & SC_LISTENING_NAMES_ABS )
/*?*/                    {  DBG_BF_ASSERT(0, "STRIP");} //STRIP001    bDo |= t->IsRPNReferenceAbsName();
/*?*/                     if ( nOnlyNames & SC_LISTENING_EXCEPT )
/*?*/                         bDo = !bDo;
/*N*/                 }
/*N*/                 if ( bDo )
/*N*/ 				{
/*N*/ 					switch( t->GetType() )
/*N*/ 					{
/*N*/ 						case svSingleRef:
/*N*/ 							rRef1.CalcAbsIfRel( aPos );
/*N*/ 							if ( rRef1.Valid() )
/*N*/ 							{
/*N*/ 								pDoc->EndListeningCell(
/*N*/ 									ScAddress( rRef1.nCol,
/*N*/ 											   rRef1.nRow,
/*N*/ 											   rRef1.nTab ), pFormCell );
/*N*/ 							}
/*N*/ 						break;
/*N*/ 						case svDoubleRef:
/*N*/ 							t->CalcAbsIfRel( aPos );
/*N*/ 							if ( rRef1.Valid() && rRef2.Valid() )
/*N*/ 							{
/*N*/ 								if ( t->GetOpCode() == ocColRowNameAuto )
/*N*/ 								{	// automagically
/*?*/ 									if ( rRef1.IsColRel() )
/*?*/ 									{	// ColName
/*?*/ 										pDoc->EndListeningArea( ScRange (
/*?*/ 											0,
/*?*/ 											rRef1.nRow,
/*?*/ 											rRef1.nTab,
/*?*/ 											MAXCOL,
/*?*/ 											rRef2.nRow,
/*?*/ 											rRef2.nTab ), pFormCell );
/*?*/ 									}
/*?*/ 									else
/*?*/ 									{	// RowName
/*?*/ 										pDoc->EndListeningArea( ScRange (
/*?*/ 											rRef1.nCol,
/*?*/ 											0,
/*?*/ 											rRef1.nTab,
/*?*/ 											rRef2.nCol,
/*?*/ 											MAXROW,
/*?*/ 											rRef2.nTab ), pFormCell );
/*?*/ 									}
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									pDoc->EndListeningArea( ScRange (
/*N*/ 										rRef1.nCol,
/*N*/ 										rRef1.nRow,
/*N*/ 										rRef1.nTab,
/*N*/ 										rRef2.nCol,
/*N*/ 										rRef2.nRow,
/*N*/ 										rRef2.nTab ), pFormCell );
/*N*/ 								}
/*N*/ 							}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScBaseCell::HasValueData() const
/*N*/ {
/*N*/ 	switch ( eCellType )
/*N*/ 	{
/*N*/ 		case CELLTYPE_VALUE :
/*N*/ 			return TRUE;
/*N*/ 		case CELLTYPE_FORMULA :
/*N*/ 			return ((ScFormulaCell*)this)->IsValue();
/*N*/ 		default:
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScBaseCell::HasStringData() const
/*N*/ {
/*N*/ 	switch ( eCellType )
/*N*/ 	{
/*N*/ 		case CELLTYPE_STRING :
/*N*/ 		case CELLTYPE_EDIT :
/*N*/ 			return TRUE;
/*N*/ 		case CELLTYPE_FORMULA :
/*N*/ 			return !((ScFormulaCell*)this)->IsValue();
/*N*/ 		default:
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ String ScBaseCell::GetStringData() const
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 	switch ( eCellType )
/*N*/ 	{
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 			((const ScStringCell*)this)->GetString( aStr );
/*N*/ 			break;
/*N*/ 		case CELLTYPE_EDIT:
/*N*/ 			((const ScEditCell*)this)->GetString( aStr );
/*N*/ 			break;
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			((ScFormulaCell*)this)->GetString( aStr );		// an der Formelzelle nicht-const
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return aStr;
/*N*/ }

//	static
/*N*/ BOOL ScBaseCell::CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 )
/*N*/ {
/*N*/ 	CellType eType1 = CELLTYPE_NONE;
/*N*/ 	CellType eType2 = CELLTYPE_NONE;
/*N*/ 	if ( pCell1 )
/*N*/ 	{
/*N*/ 		eType1 = pCell1->GetCellType();
/*N*/ 		if (eType1 == CELLTYPE_EDIT)
/*N*/ 			eType1 = CELLTYPE_STRING;
/*N*/ 		else if (eType1 == CELLTYPE_NOTE)
/*N*/ 			eType1 = CELLTYPE_NONE;
/*N*/ 	}
/*N*/ 	if ( pCell2 )
/*N*/ 	{
/*N*/ 		eType2 = pCell2->GetCellType();
/*N*/ 		if (eType2 == CELLTYPE_EDIT)
/*N*/ 			eType2 = CELLTYPE_STRING;
/*N*/ 		else if (eType2 == CELLTYPE_NOTE)
/*N*/ 			eType2 = CELLTYPE_NONE;
/*N*/ 	}
/*N*/ 	if ( eType1 != eType2 )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	switch ( eType1 )				// beide Typen gleich
/*N*/ 	{
/*N*/ 		case CELLTYPE_NONE:			// beide leer
/*N*/ 			return TRUE;
/*N*/ 		case CELLTYPE_VALUE:		// wirklich Value-Zellen
/*N*/ 			return ( ((const ScValueCell*)pCell1)->GetValue() ==
/*N*/ 					 ((const ScValueCell*)pCell2)->GetValue() );
/*N*/ 		case CELLTYPE_STRING:		// String oder Edit
/*N*/ 			{
/*N*/ 				String aText1;
/*N*/ 				if ( pCell1->GetCellType() == CELLTYPE_STRING )
/*N*/ 					((const ScStringCell*)pCell1)->GetString(aText1);
/*N*/ 				else
/*N*/ 					((const ScEditCell*)pCell1)->GetString(aText1);
/*N*/ 				String aText2;
/*N*/ 				if ( pCell2->GetCellType() == CELLTYPE_STRING )
/*N*/ 					((const ScStringCell*)pCell2)->GetString(aText2);
/*N*/ 				else
/*N*/ 					((const ScEditCell*)pCell2)->GetString(aText2);
/*N*/ 				return ( aText1 == aText2 );
/*N*/ 			}
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 			{
/*N*/ 				//!	eingefuegte Zeilen / Spalten beruecksichtigen !!!!!
/*N*/ 				//!	Vergleichsfunktion an der Formelzelle ???
/*N*/ 				//!	Abfrage mit ScColumn::SwapRow zusammenfassen!
/*N*/ 
/*N*/ 				ScTokenArray* pCode1 = ((ScFormulaCell*)pCell1)->GetCode();
/*N*/ 				ScTokenArray* pCode2 = ((ScFormulaCell*)pCell2)->GetCode();
/*N*/ 
/*N*/ 				if (pCode1->GetLen() == pCode2->GetLen())		// nicht-UPN
/*N*/ 				{
/*N*/ 					BOOL bEqual = TRUE;
/*N*/ 					USHORT nLen = pCode1->GetLen();
/*N*/ 					ScToken** ppToken1 = pCode1->GetArray();
/*N*/ 					ScToken** ppToken2 = pCode2->GetArray();
/*N*/ 					for (USHORT i=0; i<nLen; i++)
/*N*/ 						if ( !ppToken1[i]->TextEqual(*(ppToken2[i])) )
/*N*/ 						{
/*N*/ 							bEqual = FALSE;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 					if (bEqual)
/*N*/ 						return TRUE;
/*N*/ 				}
/*N*/ 
/*N*/ 				return FALSE;		// unterschiedlich lang oder unterschiedliche Tokens
/*N*/ 			}
/*N*/ 		default:
/*N*/ 			DBG_ERROR("huch, was fuer Zellen???");
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

//-----------------------------------------------------------------------------------

/*N*/ ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
/*N*/ 							  const String& rFormula, BYTE cMatInd ) :
/*N*/ 	ScBaseCell( CELLTYPE_FORMULA ),
/*N*/ 	aPos( rPos ),
/*N*/ 	pCode( NULL ),
/*N*/ 	nErgValue( 0.0 ),
/*N*/ 	bIsValue( TRUE ),
/*N*/ 	bDirty( TRUE ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cMatInd != 0
/*N*/ 	bChanged( FALSE ),
/*N*/ 	bRunning( FALSE ),
/*N*/ 	bCompile( FALSE ),
/*N*/ 	bSubTotal( FALSE ),
/*N*/ 	pDocument( pDoc ),
/*N*/ 	nFormatType( NUMBERFORMAT_NUMBER ),
/*N*/ 	nFormatIndex(0),
/*N*/ 	cMatrixFlag ( cMatInd ),
/*N*/ 	pMatrix( NULL ),
/*N*/ 	bIsIterCell (FALSE),
/*N*/ 	bInChangeTrack( FALSE ),
/*N*/ 	bTableOpDirty( FALSE ),
/*N*/ 	pPrevious(0),
/*N*/ 	pNext(0),
/*N*/ 	pPreviousTrack(0),
/*N*/ 	pNextTrack(0),
/*N*/ 	nMatCols(0),
/*N*/ 	nMatRows(0)
/*N*/ {
/*N*/ 	Compile( rFormula, TRUE );	// bNoListening, erledigt Insert
/*N*/ }

// Wird von den Importfiltern verwendet

/*N*/ ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
/*N*/ 							  const ScTokenArray* pArr, BYTE cInd ) :
/*N*/ 	ScBaseCell( CELLTYPE_FORMULA ),
/*N*/ 	aPos( rPos ),
/*N*/ 	pCode( pArr ? new ScTokenArray( *pArr ) : new ScTokenArray ),
/*N*/ 	nErgValue( 0.0 ),
/*N*/ 	bIsValue( TRUE ),
/*N*/ 	bDirty( NULL != pArr ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cInd != 0
/*N*/ 	bChanged( FALSE ),
/*N*/ 	bRunning( FALSE ),
/*N*/ 	bCompile( FALSE ),
/*N*/ 	bSubTotal( FALSE ),
/*N*/ 	pDocument( pDoc ),
/*N*/ 	nFormatType( NUMBERFORMAT_NUMBER ),
/*N*/ 	nFormatIndex(0),
/*N*/ 	cMatrixFlag ( cInd ),
/*N*/ 	pMatrix ( NULL ),
/*N*/ 	bIsIterCell (FALSE),
/*N*/ 	bInChangeTrack( FALSE ),
/*N*/ 	bTableOpDirty( FALSE ),
/*N*/ 	pPrevious(0),
/*N*/ 	pNext(0),
/*N*/ 	pPreviousTrack(0),
/*N*/ 	pNextTrack(0),
/*N*/ 	nMatCols(0),
/*N*/ 	nMatRows(0)
/*N*/ {
/*N*/ 	// UPN-Array erzeugen
/*N*/ 	if( pCode->GetLen() && !pCode->GetError() && !pCode->GetCodeLen() )
/*N*/ 	{
/*N*/ 		ScCompiler aComp(pDocument, aPos, *pCode);
/*N*/ 		bSubTotal = aComp.CompileTokenArray();
/*N*/ 		nFormatType = aComp.GetNumFormatType();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pCode->Reset();
/*N*/ 		if ( pCode->GetNextOpCodeRPN( ocSubTotal ) )
/*N*/ 			bSubTotal = TRUE;
/*N*/ 	}
/*N*/ }

/*N*/ ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rNewPos,
/*N*/                               const ScFormulaCell& rScFormulaCell, USHORT nCopyFlags ) :
/*N*/ 	ScBaseCell( rScFormulaCell ),
/*N*/ 	SfxListener(),
/*N*/ 	aErgString( rScFormulaCell.aErgString ),
/*N*/ 	nErgValue( rScFormulaCell.nErgValue ),
/*N*/ 	bIsValue( rScFormulaCell.bIsValue ),
/*N*/ 	bDirty( rScFormulaCell.bDirty ),
/*N*/ 	bChanged( rScFormulaCell.bChanged ),
/*N*/ 	bRunning( rScFormulaCell.bRunning ),
/*N*/ 	bCompile( rScFormulaCell.bCompile ),
/*N*/ 	bSubTotal( rScFormulaCell.bSubTotal ),
/*N*/ 	pDocument( pDoc ),
/*N*/ 	nFormatType( rScFormulaCell.nFormatType ),
/*N*/ 	nFormatIndex( pDoc == rScFormulaCell.pDocument ? rScFormulaCell.nFormatIndex : 0 ),
/*N*/ 	cMatrixFlag ( rScFormulaCell.cMatrixFlag ),
/*N*/ 	bIsIterCell (FALSE),
/*N*/ 	bInChangeTrack( FALSE ),
/*N*/ 	bTableOpDirty( FALSE ),
/*N*/ 	pPrevious(0),
/*N*/ 	pNext(0),
/*N*/ 	pPreviousTrack(0),
/*N*/ 	pNextTrack(0),
/*N*/ 	aPos( rNewPos ),
/*N*/ 	nMatCols( rScFormulaCell.nMatCols ),
/*N*/ 	nMatRows( rScFormulaCell.nMatRows )
/*N*/ {
/*N*/ 	if (rScFormulaCell.pMatrix)
/*?*/ 		pMatrix = rScFormulaCell.pMatrix->Clone();
/*N*/ 	else
/*N*/ 		pMatrix = NULL;
/*N*/ 	pCode = rScFormulaCell.pCode->Clone();
/*N*/ 
/*N*/     if ( nCopyFlags & 0x0001 )
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pCode->ReadjustRelative3DReferences( rScFormulaCell.aPos, aPos );
/*N*/ 
/*N*/ 	// evtl. Fehler zuruecksetzen und neu kompilieren
/*N*/ 	//	nicht im Clipboard - da muss das Fehlerflag erhalten bleiben
/*N*/ 	//	Spezialfall Laenge=0: als Fehlerzelle erzeugt, dann auch Fehler behalten
/*N*/ 	if ( pCode->GetError() && !pDocument->IsClipboard() && pCode->GetLen() )
/*N*/ 	{
/*N*/ 		pCode->SetError( 0 );
/*N*/ 		bCompile = TRUE;
/*N*/ 	}
/*N*/     //! Compile ColRowNames on URM_MOVE/URM_COPY _after_ UpdateReference
/*N*/     BOOL bCompileLater = FALSE;
/*N*/     BOOL bClipMode = rScFormulaCell.pDocument->IsClipboard();
/*N*/ 	if( !bCompile )
/*N*/     {   // Name references with references and ColRowNames
/*N*/         pCode->Reset();
/*N*/ 		for( ScToken* t = pCode->GetNextReferenceOrName(); t && !bCompile;
/*N*/ 					  t = pCode->GetNextReferenceOrName() )
/*N*/ 		{
/*N*/ 			if ( t->GetType() == svIndex )
/*N*/ 			{
/*?*/ 				ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex( t->GetIndex() );
/*?*/ 				if( pRangeData )
/*?*/ 				{
/*?*/ 					if( pRangeData->HasReferences() )
/*?*/ 						bCompile = TRUE;
/*?*/ 				}
/*?*/ 				else
/*?*/                     bCompile = TRUE;    // invalid reference!
/*N*/ 			}
/*N*/ 			else if ( t->GetOpCode() == ocColRowName )
/*N*/ 			{
/*N*/                 bCompile = TRUE;        // new lookup needed
/*N*/                 bCompileLater = bClipMode;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( bCompile )
/*N*/ 	{
/*N*/         if ( !bCompileLater && bClipMode )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pCode->Reset();
/*N*/ 		}
/*N*/         if ( !bCompileLater )
/*N*/         {
/*N*/             // bNoListening, bei in Clip/Undo sowieso nicht,
/*N*/             // bei aus Clip auch nicht, sondern nach Insert(Clone) und UpdateReference
/*N*/             CompileTokenArray( TRUE );
/*N*/         }
/*N*/ 	}
/*N*/ }

// +---+---+---+---+---+---+---+---+
// |           |Str|Num|Dir|cMatrix|
// +---+---+---+---+---+---+---+---+

/*N*/ ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
/*N*/ 							  SvStream& rStream, ScMultipleReadHeader& rHdr ) :
/*N*/ 	ScBaseCell( CELLTYPE_FORMULA ),
/*N*/ 	aPos( rPos ),
/*N*/ 	pCode( new ScTokenArray ),
/*N*/ 	nErgValue( 0.0 ),
/*N*/ 	bIsValue( TRUE ),
/*N*/ 	bDirty( FALSE ),
/*N*/ 	bChanged( FALSE ),
/*N*/ 	bRunning( FALSE ),
/*N*/ 	bCompile( FALSE ),
/*N*/ 	bSubTotal( FALSE ),
/*N*/ 	pDocument( pDoc ),
/*N*/ 	nFormatType( 0 ),
/*N*/ 	nFormatIndex(0),
/*N*/ 	pMatrix ( NULL ),
/*N*/ 	bIsIterCell (FALSE),
/*N*/ 	bInChangeTrack( FALSE ),
/*N*/ 	bTableOpDirty( FALSE ),
/*N*/ 	pPrevious(0),
/*N*/ 	pNext(0),
/*N*/ 	pPreviousTrack(0),
/*N*/ 	pNextTrack(0),
/*N*/ 	nMatCols(0),
/*N*/ 	nMatRows(0)
/*N*/ {
/*N*/ //	ScReadHeader aHdr( rStream );
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	USHORT nVer = (USHORT) pDoc->GetSrcVersion();
/*N*/ 
/*N*/ 	if( nVer >= SC_NUMFMT )
/*N*/ 	{
/*N*/ 		BYTE cData;
/*N*/ 		rStream >> cData;
/*N*/ #ifdef DBG_UTIL
/*N*/ //		static BOOL bShown = 0;
/*N*/ //		if ( !bShown && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
/*N*/ //		{
/*N*/ //			bShown = 1;
/*N*/ //			DBG_ERRORFILE( "bei inkompatiblem FileFormat den FormatIndex umheben!" );
/*N*/ //		}
/*N*/ #endif
/*N*/ 		if( cData & 0x0F )
/*N*/ 		{
/*N*/ 			BYTE nSkip = cData & 0x0F;
/*N*/ 			if ( (cData & 0x10) && nSkip >= sizeof(UINT32) )
/*N*/ 			{
/*N*/ 				UINT32 n;
/*N*/ 				rStream >> n;
/*N*/ 				nFormatIndex = n;
/*N*/ 				nSkip -= sizeof(UINT32);
/*N*/ 			}
/*N*/ 			if ( nSkip )
/*?*/ 				rStream.SeekRel( nSkip );
/*N*/ 		}
/*N*/ 		BYTE cFlags;
/*N*/ 		rStream >> cFlags >> nFormatType;
/*N*/ 		cMatrixFlag = (BYTE) ( cFlags & 0x03 );
/*N*/ 		bDirty = BOOL( ( cFlags & 0x04 ) != 0 );
/*N*/ 		if( cFlags & 0x08 )
/*N*/ 			rStream >> nErgValue;
/*N*/ 		if( cFlags & 0x10 )
/*N*/ 		{
/*N*/ 			rStream.ReadByteString( aErgString, rStream.GetStreamCharSet() );
/*N*/ 			bIsValue = FALSE;
/*N*/ 		}
/*N*/ 		pCode->Load( rStream, nVer, aPos );
/*N*/ 		if ( (cFlags & 0x18) == 0 )
/*N*/ 			bDirty = TRUE;		// #67161# no result stored => recalc
/*N*/ 		if( cFlags & 0x20 )
/*N*/ 			bSubTotal = TRUE;
/*N*/ 		else if ( nVer < SC_SUBTOTAL_BUGFIX )
/*N*/ 		{	// #65285# in alten Dokumenten war Flag nicht gesetzt, wenn Formel
/*?*/ 			// manuell eingegeben wurde (nicht via Daten->Teilergebnisse)
/*N*/ 			if ( pCode->HasOpCodeRPN( ocSubTotal ) )
/*N*/ 			{
/*?*/ 				bDirty = TRUE;		// neu berechnen
/*?*/ 				bSubTotal = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( cMatrixFlag == MM_FORMULA && rHdr.BytesLeft() )
/*N*/ 			rStream >> nMatCols >> nMatRows;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		UINT16 nCodeLen;
/*N*/ 		if( pDoc->GetSrcVersion() >= SC_FORMULA_LCLVER )
/*N*/ 			rStream.SeekRel( 2 );
/*N*/ 		rStream >> cMatrixFlag >> nCodeLen;
/*N*/ 		if( cMatrixFlag == 5 )
/*N*/ 			cMatrixFlag = 0;
/*N*/ 		cMatrixFlag &= 3;
/*N*/ 		if( nCodeLen )
/*N*/ 			pCode->Load30( rStream, aPos );
/*N*/ 		// Wir koennen hier bei Calc 3.0-Docs noch kein UPN-Array
/*N*/ 		// erzeugen, da die Named Ranges noch nicht eingelesen sind
/*N*/ 	}
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 	//	after loading, it must be known if ocMacro is in any formula
/*N*/ 	//	(for macro warning, and to insert the hidden view)
/*N*/ 	if ( !pDoc->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
/*N*/ 		pDoc->SetHasMacroFunc( TRUE );
/*N*/ }

/*N*/ BOOL lcl_IsBeyond( ScTokenArray* pCode, USHORT nMaxRow )
/*N*/ {
/*N*/ 	ScToken* t;
/*N*/ 	pCode->Reset();
/*N*/ 	while ( t = pCode->GetNextReferenceRPN() )	// RPN -> auch in Namen
/*N*/ 		if ( t->GetSingleRef().nRow > nMaxRow ||
/*N*/ 				(t->GetType() == svDoubleRef &&
/*N*/ 				t->GetDoubleRef().Ref2.nRow > nMaxRow) )
/*N*/ 			return TRUE;
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScFormulaCell::Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
/*N*/ 	if ( nSaveMaxRow < MAXROW && lcl_IsBeyond( pCode, nSaveMaxRow ) )
/*N*/ 	{
/*?*/ 		//	Zelle mit Ref-Error erzeugen und speichern
/*?*/ 		//	StartEntry/EndEntry passiert beim Speichern der neuen Zelle
/*?*/ 
/*?*/ 		SingleRefData aRef;
/*?*/ 		aRef.InitAddress(ScAddress());
/*?*/ 		aRef.SetColRel(TRUE);
/*?*/ 		aRef.SetColDeleted(TRUE);
/*?*/ 		aRef.SetRowRel(TRUE);
/*?*/ 		aRef.SetRowDeleted(TRUE);
/*?*/ 		aRef.CalcRelFromAbs(aPos);
/*?*/ 		ScTokenArray aArr;
/*?*/ 		aArr.AddSingleReference(aRef);
/*?*/ 		aArr.AddOpCode(ocStop);
/*?*/ 		ScFormulaCell* pErrCell = new ScFormulaCell( pDocument, aPos, &aArr );
/*?*/ 		pErrCell->Save( rStream, rHdr );
/*?*/ 		delete pErrCell;
/*?*/ 
/*?*/ 		pDocument->SetLostData();			// Warnung ausgeben
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	if ( bIsValue && !pCode->GetError() && !::rtl::math::isFinite( nErgValue ) )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( msgDbgInfinity );
/*N*/ 		pCode->SetError( errIllegalFPOperation );
/*N*/ 	}
/*N*/ 	BYTE cFlags = cMatrixFlag & 0x03;
/*N*/ 	if( bDirty )
/*N*/ 		cFlags |= 0x04;
/*N*/ 	// Daten speichern?
/*N*/ 	if( pCode->IsRecalcModeNormal() && !pCode->GetError() )
/*N*/ 		cFlags |= bIsValue ? 0x08 : 0x10;
/*N*/ 	if ( bSubTotal )
/*N*/ 		cFlags |= 0x20;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	static BOOL bShown = 0;
/*N*/ 	if ( !bShown && rStream.GetVersion() > SOFFICE_FILEFORMAT_50 )
/*N*/ 	{
/*N*/ 		bShown = 1;
/*N*/ 		DBG_ERRORFILE( "bei inkompatiblem FileFormat den FormatIndex umheben!" );
/*N*/ 	}
/*N*/ //	rStream << (BYTE) 0x00;
/*N*/ #endif
/*N*/ 	if ( nFormatIndex )
/*N*/ 		rStream << (BYTE) (0x10 | sizeof(UINT32)) << static_cast<sal_uInt32>(nFormatIndex);
/*N*/ 	else
/*N*/ 		rStream << (BYTE) 0x00;
/*N*/ 	rStream << cFlags << (UINT16) nFormatType;
/*N*/ 	if( cFlags & 0x08 )
/*N*/ 		rStream << nErgValue;
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		rStream.WriteByteString( aErgString, rStream.GetStreamCharSet() );
/*N*/ 	pCode->Store( rStream, aPos );
/*N*/ 	if ( cMatrixFlag == MM_FORMULA )
/*N*/ 		rStream << nMatCols << nMatRows;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ }

/*N*/  ScBaseCell* ScFormulaCell::Clone( ScDocument* pDoc, const ScAddress& rPos,
/*N*/  		BOOL bNoListening ) const
/*N*/  {
/*N*/  	ScFormulaCell* pCell = new ScFormulaCell( pDoc, rPos, *this );
/*N*/  	if ( !bNoListening )
/*N*/  		pCell->StartListeningTo( pDoc );
/*N*/  	return pCell;
/*N*/  }

/*N*/ void ScFormulaCell::GetFormula( String& rFormula ) const
/*N*/ {
/*N*/ 	if( pCode->GetError() && !pCode->GetLen() )
/*N*/ 	{
/*?*/ 		rFormula = ScGlobal::GetErrorString( pCode->GetError() ); return;
/*N*/ 	}
/*N*/ 	else if( cMatrixFlag == MM_REFERENCE )
/*N*/ 	{
/*N*/ 		// Referenz auf eine andere Zelle, die eine Matrixformel enthaelt
/*N*/ 		pCode->Reset();
/*N*/ 		ScToken* p = pCode->GetNextReferenceRPN();
/*N*/ 		if( p )
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = NULL;
/*N*/ 			if ( !IsInChangeTrack() )
/*N*/ 			{
/*N*/ 				SingleRefData& rRef = p->GetSingleRef();
/*N*/ 				rRef.CalcAbsIfRel( aPos );
/*N*/ 				if ( rRef.Valid() )
/*N*/ 					pCell = pDocument->GetCell( ScAddress( rRef.nCol,
/*N*/ 						rRef.nRow, rRef.nTab ) );
/*N*/ 			}
/*N*/ 			if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			{
/*N*/ 				((ScFormulaCell*)pCell)->GetFormula(rFormula);
/*N*/ 				return;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				ScCompiler aComp( pDocument, aPos, *pCode );
/*?*/ 				aComp.CreateStringFromTokenArray( rFormula );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			DBG_ERROR("ScFormulaCell::GetFormula: Keine Matrix");
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScCompiler aComp( pDocument, aPos, *pCode );
/*N*/ 		aComp.CreateStringFromTokenArray( rFormula );
/*N*/ 	}
/*N*/ 
/*N*/ 	rFormula.Insert( '=',0 );
/*N*/ 	if( cMatrixFlag )
/*N*/ 	{
/*N*/ 		rFormula.Insert('{', 0);
/*N*/ 		rFormula += '}';
/*N*/ 	}
/*N*/ }

/*N*/ void ScFormulaCell::Compile( const String& rFormula, BOOL bNoListening )
/*N*/ {
/*N*/ 	if ( pDocument->IsClipOrUndo() ) return;
/*N*/ 	BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
/*N*/ 	if ( bWasInFormulaTree )
/*?*/ 		pDocument->RemoveFromFormulaTree( this );
/*N*/ 	// pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
/*N*/ 	if ( pCode )
/*N*/ 		pCode->Clear();
/*N*/ 	ScTokenArray* pCodeOld = pCode;
/*N*/ 	ScCompiler aComp(pDocument, aPos);
/*N*/ 	if ( pDocument->IsImportingXML() )
/*?*/ 		aComp.SetCompileEnglish( TRUE );
/*N*/ 	pCode = aComp.CompileString( rFormula );
/*N*/ 	if ( pCodeOld )
/*N*/ 		delete pCodeOld;
/*N*/ 	if( !pCode->GetError() )
/*N*/ 	{
/*N*/ 		if ( !pCode->GetLen() && aErgString.Len() && rFormula == aErgString )
/*N*/ 		{	// #65994# nicht rekursiv CompileTokenArray/Compile/CompileTokenArray
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if ( rFormula.GetChar(0) == '=' )
/*N*/ 		}
/*N*/ 		bCompile = TRUE;
/*N*/ 		CompileTokenArray( bNoListening );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bChanged = TRUE;
/*N*/ 		SetTextWidth( TEXTWIDTH_DIRTY );
/*N*/ 		SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*N*/ 	}
/*N*/ 	if ( bWasInFormulaTree )
/*?*/ 		pDocument->PutInFormulaTree( this );
/*N*/ }


/*N*/ void ScFormulaCell::CompileTokenArray( BOOL bNoListening )
/*N*/ {
/*N*/ 	// Noch nicht compiliert?
/*N*/ 	if( !pCode->GetLen() && aErgString.Len() )
/*N*/ 		Compile( aErgString );
/*N*/ 	else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetError() )
/*N*/ 	{
/*N*/ 		// RPN-Laenge kann sich aendern
/*N*/ 		BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
/*N*/ 		if ( bWasInFormulaTree )
/*?*/ 			pDocument->RemoveFromFormulaTree( this );
/*N*/ 
/*N*/ 		// Laden aus Filter? Dann noch nix machen!
/*N*/ 		if( pDocument->IsInsertingFromOtherDoc() )
/*N*/ 			bNoListening = TRUE;
/*N*/ 
/*N*/ 		if( !bNoListening && pCode->GetCodeLen() )
/*?*/ 			EndListeningTo( pDocument );
/*N*/ 		ScCompiler aComp(pDocument, aPos, *pCode );
/*N*/ 		bSubTotal = aComp.CompileTokenArray();
/*N*/ 		if( !pCode->GetError() )
/*N*/ 		{
/*N*/ 			nFormatType = aComp.GetNumFormatType();
/*N*/ 			nFormatIndex = 0;
/*N*/ 			bChanged = TRUE;
/*N*/ 			nErgValue = 0.0;
/*N*/ 			aErgString.Erase();
/*N*/ 			bCompile = FALSE;
/*N*/ 			if ( !bNoListening )
/*N*/ 				StartListeningTo( pDocument );
/*N*/ 		}
/*N*/ 		if ( bWasInFormulaTree )
/*?*/ 			pDocument->PutInFormulaTree( this );
/*N*/ 	}
/*N*/ }


/*N*/ void ScFormulaCell::CompileXML( ScProgress& rProgress )
/*N*/ {
/*N*/ 	if ( cMatrixFlag == MM_REFERENCE )
/*N*/ 	{	// is already token code via ScDocFunc::EnterMatrix, ScDocument::InsertMatrixFormula
/*N*/ 		// just establish listeners
/*N*/ 		StartListeningTo( pDocument );
/*N*/ 		return ;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScCompiler aComp( pDocument, aPos, *pCode );
/*N*/ 	aComp.SetCompileEnglish( TRUE );
/*N*/ 	aComp.SetImportXML( TRUE );
/*N*/ 	String aFormula;
/*N*/ 	aComp.CreateStringFromTokenArray( aFormula );
/*N*/     pDocument->DecXMLImportedFormulaCount( aFormula.Len() );
/*N*/     rProgress.SetStateCountDownOnPercent( pDocument->GetXMLImportedFormulaCount() );
/*N*/ 	// pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
/*N*/ 	if ( pCode )
/*N*/ 		pCode->Clear();
/*N*/ 	ScTokenArray* pCodeOld = pCode;
/*N*/ 	pCode = aComp.CompileString( aFormula );
/*N*/ 	delete pCodeOld;
/*N*/ 	if( !pCode->GetError() )
/*N*/ 	{
/*N*/ 		if ( !pCode->GetLen() )
/*N*/ 		{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if ( aFormula.GetChar(0) == '=' )
/*N*/ 		}
/*N*/ 		bSubTotal = aComp.CompileTokenArray();
/*N*/ 		if( !pCode->GetError() )
/*N*/ 		{
/*N*/ 			nFormatType = aComp.GetNumFormatType();
/*N*/ 			nFormatIndex = 0;
/*N*/ 			bChanged = TRUE;
/*N*/ 			bCompile = FALSE;
/*N*/ 			StartListeningTo( pDocument );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bChanged = TRUE;
/*N*/ 		SetTextWidth( TEXTWIDTH_DIRTY );
/*N*/ 		SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Same as in Load: after loading, it must be known if ocMacro is in any formula
/*N*/ 	//	(for macro warning, CompileXML is called at the end of loading XML file)
/*N*/ 	if ( !pDocument->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
/*N*/ 		pDocument->SetHasMacroFunc( TRUE );
/*N*/ }


/*N*/ void ScFormulaCell::CalcAfterLoad()
/*N*/ {
/*N*/ 	BOOL bNewCompiled = FALSE;
/*N*/ 	// Falls ein Calc 1.0-Doc eingelesen wird, haben wir ein Ergebnis,
/*N*/ 	// aber kein TokenArray
/*N*/ 	if( !pCode->GetLen() && aErgString.Len() )
/*N*/ 	{
/*?*/ 		Compile( aErgString, TRUE );
/*?*/ 		aErgString.Erase();
/*?*/ 		bDirty = TRUE;
/*?*/ 		bNewCompiled = TRUE;
/*N*/ 	}
/*N*/ 	// Das UPN-Array wird nicht erzeugt, wenn ein Calc 3.0-Doc eingelesen
/*N*/ 	// wurde, da die RangeNames erst jetzt existieren.
/*N*/ 	if( pCode->GetLen() && !pCode->GetCodeLen() && !pCode->GetError() )
/*N*/ 	{
/*N*/ 		ScCompiler aComp(pDocument, aPos, *pCode);
/*N*/ 		bSubTotal = aComp.CompileTokenArray();
/*N*/ 		nFormatType = aComp.GetNumFormatType();
/*N*/ 		nFormatIndex = 0;
/*N*/ 		bDirty = TRUE;
/*N*/ 		bCompile = FALSE;
/*N*/ 		bNewCompiled = TRUE;
/*N*/ 	}
/*N*/ 	// irgendwie koennen unter os/2 mit rotter FPU-Exception /0 ohne Err503
/*N*/ 	// gespeichert werden, woraufhin spaeter im NumberFormatter die BLC Lib
/*N*/ 	// bei einem fabs(-NAN) abstuerzt (#32739#)
/*N*/ 	// hier fuer alle Systeme ausbuegeln, damit da auch Err503 steht
/*N*/ 	if ( bIsValue && !::rtl::math::isFinite( nErgValue ) )
/*N*/ 	{
/*?*/ 		DBG_ERRORFILE("Formelzelle INFINITY !!! Woher kommt das Dokument?");
/*?*/ 		nErgValue = 0.0;
/*?*/ 		pCode->SetError( errIllegalFPOperation );
/*?*/ 		bDirty = TRUE;
/*N*/ 	}
/*N*/ 	// DoubleRefs bei binaeren Operatoren waren vor v5.0 immer Matrix,
/*N*/ 	// jetzt nur noch wenn in Matrixformel, sonst implizite Schnittmenge
/*N*/ 	if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
/*N*/ 			GetMatrixFlag() == MM_NONE && pCode->HasMatrixDoubleRefOps() )
/*N*/ 	{
/*N*/ 		cMatrixFlag = MM_FORMULA;
/*N*/ 		nMatCols = 1;
/*N*/ 		nMatRows = 1;
/*N*/ 	}
/*N*/ 	// Muss die Zelle berechnet werden?
/*N*/ 	// Nach Load koennen Zellen einen Fehlercode enthalten, auch dann
/*N*/ 	// Listener starten und ggbf. neu berechnen wenn nicht RECALCMODE_NORMAL
/*N*/ 	if( !bNewCompiled || !pCode->GetError() )
/*N*/ 	{
/*N*/ 		StartListeningTo( pDocument );
/*N*/ 		if( !pCode->IsRecalcModeNormal() )
/*N*/ 			bDirty = TRUE;
/*N*/ 	}
/*N*/ 	if ( pCode->GetError() == errInterpOverflow )
/*N*/ 	{	// versuchen Err527 wegzubuegeln
/*N*/ 		bDirty = TRUE;
/*N*/ 	}
/*N*/ 	else if ( pCode->IsRecalcModeAlways() )
/*N*/ 	{	// zufall(), heute(), jetzt() bleiben immer im FormulaTree, damit sie
/*N*/ 		// auch bei jedem F9 berechnet werden.
/*N*/ 		bDirty = TRUE;
/*N*/ 	}
/*N*/ 	// Noch kein SetDirty weil noch nicht alle Listener bekannt, erst in
/*N*/ 	// SetDirtyAfterLoad.
/*N*/ }

/*N*/ void ScFormulaCell::Interpret()
/*N*/ {
/*N*/ 	static USHORT nRecCount = 0;
/*N*/ 	static ScFormulaCell* pLastIterInterpreted = NULL;
/*N*/ 	if ( !IsDirtyOrInTableOpDirty() )
/*N*/ 		return;			// fuer IterCircRef, nix doppelt
/*N*/ 
/*N*/ 	//!	HACK:
/*N*/ 	//	Wenn der Aufruf aus einem Reschedule im DdeLink-Update kommt, dirty stehenlassen
/*N*/ 	//	Besser: Dde-Link Update ohne Reschedule oder ganz asynchron !!!
/*N*/ 
/*N*/     if ( pDocument->IsInDdeLinkUpdate() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (bRunning)
/*N*/ 	{
/*N*/ 		// Keine Iterierung?
/*N*/ 		if (!pDocument->GetDocOptions().IsIter())
/*N*/ 			pCode->SetError( errCircularReference );
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if (pCode->GetError() == errCircularReference)
/*?*/ 				pCode->SetError( 0 );
/*?*/ 			nIterMode = 1;
/*?*/ 			bIsIterCell = TRUE;
/*?*/ 			pLastIterInterpreted = NULL;
/*N*/ 		}
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	// #63038# fuer GetErrCode, IsValue, GetValue nicht mehrfach interpretieren
/*N*/ 	if ( nIterMode && pLastIterInterpreted == this )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	if( !pCode->GetCodeLen() && !pCode->GetError() )
/*N*/     {
/*N*/         // #i11719# no UPN and no error and no token code but result string present
/*N*/         // => interpretation of this cell during name-compilation and unknown names
/*N*/         // => can't exchange underlying code array in CompileTokenArray() /
/*N*/         // Compile() because interpreter's token iterator would crash.
/*N*/         // This should only be a temporary condition and, since we set an
/*N*/         // error, if ran into it again we'd bump into the dirty-clearing
/*N*/         // condition further down.
/*N*/         if ( !pCode->GetLen() && aErgString.Len() )
/*N*/         {
/*N*/             pCode->SetError( errNoCode );
/*N*/             // This is worth an assertion; if encountered in daily work
/*N*/             // documents we might need another solution. Or just confirm correctness.
/*N*/             DBG_ERRORFILE( "ScFormulaCell::Interpret: no UPN, no error, no token, but string" );
/*N*/             return;
/*N*/         }
/*?*/ 		CompileTokenArray();
/*N*/     }
/*N*/ 
/*N*/ 	if( pCode->GetCodeLen() && pDocument )
/*N*/ 	{
/*N*/ #if defined(MAC) && !defined(SIMPLEMAXRECURSION)
/*N*/ 		if( StackSpace() < 2048 )	// 2K Stack noch uebriglassen
/*N*/ #else
/*N*/ 		if( nRecCount > MAXRECURSION )
/*N*/ #endif
/*N*/ 		{
/*?*/ 			pCode->SetError( errInterpOverflow );
/*?*/ 			// Zelle bleibt in FormulaTree, naechstes Mal sind evtl.
/*?*/ 			// Vorgaenger bereits berechnet worden bzw. von der View wird
/*?*/ 			// via ScCellFormat::GetString CalcFormulaTree angeworfen
/*?*/ 			bDirty = FALSE;
/*?*/ 			bTableOpDirty = FALSE;
/*?*/ 			nErgValue = 0.0;
/*?*/ 			bIsValue = TRUE;
/*?*/ 			nIterMode = 0;
/*?*/ 			bIsIterCell = FALSE;
/*?*/ 			pLastIterInterpreted = NULL;
/*?*/ 			bChanged = TRUE;
/*?*/ 			SetTextWidth( TEXTWIDTH_DIRTY );
/*?*/ 			SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*?*/ 			return;
/*N*/ 		}
/*N*/ 		nRecCount++;
/*N*/ 		pDocument->IncInterpretLevel();
/*N*/ 		ScInterpreter* p = new ScInterpreter( this, pDocument, aPos, *pCode );
/*N*/ 		USHORT nOldErrCode = pCode->GetError();
/*N*/ 		USHORT nIterCount = 0;
/*N*/ 		if ( nIterMode == 0 )
/*N*/ 		{	// nur beim ersten Mal
/*N*/ // wenn neu kompilierte Zelle 0.0 ergibt wird kein Changed erkannt
/*N*/ // und die Zelle wird nicht sofort repainted!
/*N*/ //			bChanged = FALSE;
/*N*/ 			if ( nOldErrCode == errNoConvergence
/*N*/ 			  && pDocument->GetDocOptions().IsIter() )
/*?*/ 				pCode->SetError( 0 );
/*N*/ 		}
/*N*/ 		BOOL bRepeat = TRUE;
/*N*/ 		while( bRepeat )
/*N*/ 		{
/*N*/ 			if ( pMatrix )
/*N*/ 			{
/*?*/ 				delete pMatrix;
/*?*/ 				pMatrix = NULL;
/*N*/ 			}
/*N*/ 
/*N*/ 			switch ( pCode->GetError() )
/*N*/ 			{
/*?*/ 				case errCircularReference :		// wird neu festgestellt
/*?*/ 				case errInterpOverflow :		// Err527 eine Chance geben
/*?*/ 					pCode->SetError( 0 );
/*?*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 			bRunning = TRUE;
/*N*/ 			p->Interpret();
/*N*/ 			bRunning = FALSE;
/*N*/ 			if( pCode->GetError()
/*N*/ 			 && pCode->GetError() != errCircularReference )
/*N*/ 			{
/*N*/ 				bDirty = FALSE;
/*N*/ 				bTableOpDirty = FALSE;
/*N*/ 				nIterMode = 0;
/*N*/ 				bIsIterCell = FALSE;
/*N*/ 				pLastIterInterpreted = NULL;
/*N*/ 				bChanged = TRUE;
/*N*/ 				bIsValue = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			if( nIterMode == 1 && bIsIterCell )
/*N*/ 			{
/*?*/ 				pLastIterInterpreted = NULL;
/*?*/ 				++nIterCount;
/*?*/ 				// schoen konvergiert?
/*?*/ 				if( (p->GetResultType() == svDouble
/*?*/ 				  && fabs( p->GetNumResult() - nErgValue ) <=
/*?*/ 					 pDocument->GetDocOptions().GetIterEps())
/*?*/ 					)
/*?*/ 				{
/*?*/ 					nIterMode = 0;
/*?*/ 					bIsIterCell = FALSE;
/*?*/ 					bDirty = FALSE;
/*?*/ 					bTableOpDirty = FALSE;
/*?*/ 					bRepeat = FALSE;
/*?*/ 				}
/*?*/ 				// Zu oft rumgelaufen?
/*?*/ 				else if( nIterCount >= pDocument->GetDocOptions().GetIterCount() )
/*?*/ 				{
/*?*/ 					nIterMode = 0;
/*?*/ 					bIsIterCell = FALSE;
/*?*/ 					bDirty = FALSE;
/*?*/ 					bTableOpDirty = FALSE;
/*?*/ 					bRepeat = FALSE;
/*?*/ 					pCode->SetError( errNoConvergence );
/*?*/ 				}
/*?*/ 				if ( p->GetResultType() == svDouble )
/*?*/ 				{
/*?*/ 					if( !bIsValue || nErgValue != p->GetNumResult() )
/*?*/ 						bChanged = TRUE;
/*?*/ 					bIsValue = TRUE;
/*?*/ 					nErgValue = p->GetNumResult();
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bRepeat = FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 		switch( p->GetResultType() )
/*N*/ 		{
/*N*/ 			case svDouble:
/*N*/ 				if( nErgValue != p->GetNumResult() || !bIsValue )
/*N*/ 				{
/*N*/ 					bChanged = TRUE;
/*N*/ 					bIsValue = TRUE;
/*N*/ 					nErgValue = p->GetNumResult();
/*N*/ 				}
/*N*/ 			break;
/*N*/ 			case svString:
/*N*/ 				if( aErgString != p->GetStringResult() || bIsValue )
/*N*/ 				{
/*N*/ 					bChanged = TRUE;
/*N*/ 					bIsValue = FALSE;
/*N*/ 					aErgString = p->GetStringResult();
/*N*/ 				}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Neuer Fehlercode?
/*N*/ 		if( !bChanged && pCode->GetError() != nOldErrCode )
/*N*/ 			bChanged = TRUE;
/*N*/ 		// Anderes Zahlenformat?
/*N*/ 		if( nFormatType != p->GetRetFormatType() )
/*N*/ 		{
/*N*/ 			nFormatType = p->GetRetFormatType();
/*N*/ 			bChanged = TRUE;
/*N*/ 		}
/*N*/ 		if( nFormatIndex != p->GetRetFormatIndex() )
/*N*/ 		{
/*N*/ 			nFormatIndex = p->GetRetFormatIndex();
/*N*/ 			bChanged = TRUE;
/*N*/ 		}
/*N*/ 		// Genauigkeit wie angezeigt?
/*N*/ 		if ( bIsValue && !pCode->GetError()
/*N*/ 		  && pDocument->GetDocOptions().IsCalcAsShown()
/*N*/ 		  && nFormatType != NUMBERFORMAT_DATE
/*N*/ 		  && nFormatType != NUMBERFORMAT_TIME
/*N*/ 		  && nFormatType != NUMBERFORMAT_DATETIME )
/*N*/ 		{
/*?*/ 			ULONG nFormat = pDocument->GetNumberFormat( aPos );
/*?*/ 			if ( nFormatIndex && (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*?*/ 				nFormat = nFormatIndex;
/*?*/ 			if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*?*/ 				nFormat = ScGlobal::GetStandardFormat(
/*?*/ 					*pDocument->GetFormatTable(), nFormat, nFormatType );
/*?*/ 			nErgValue = pDocument->RoundValueAsShown( nErgValue, nFormat );
/*N*/ 		}
/*N*/ 		if ( nIterMode == 0 )
/*N*/ 		{
/*N*/ 			bDirty = FALSE;
/*N*/ 			bTableOpDirty = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pLastIterInterpreted = this;
/*N*/ 		pMatrix = p->GetMatrixResult();
/*N*/ 		if( pMatrix )
/*N*/ 		{
/*N*/ #if 0
/*?*/ //! MatrixFormel immer changed?!?
/*?*/ // ist bei MD's Rundumschlag von r1.167 --> r1.168 reingekommen
/*?*/ // => ewiges Repaint von MatrixFormel, besonders bei DDE laestig
/*?*/ // ab r1.260 (sv369b) probieren wir's mal ohne..
/*?*/ 			if( cMatrixFlag == MM_FORMULA )
/*?*/ 				bChanged = TRUE;
/*?*/ 			else
/*N*/ #else
/*N*/ 			if( cMatrixFlag != MM_FORMULA )
/*N*/ #endif
/*N*/ 			{	// mit linker oberer Ecke weiterleben
/*?*/ 				delete pMatrix;
/*?*/ 				pMatrix = NULL;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( bChanged )
/*N*/ 		{
/*N*/ 			SetTextWidth( TEXTWIDTH_DIRTY );
/*N*/ 			SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*N*/ 		}
/*N*/ 		delete p;
/*N*/ 		nRecCount--;
/*N*/ 		pDocument->DecInterpretLevel();
/*N*/ 		if ( pCode->GetError() != errInterpOverflow
/*N*/ 		  && !pCode->IsRecalcModeAlways() )
/*N*/ 			pDocument->RemoveFromFormulaTree( this );
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if ( bIsValue && !pCode->GetError() && !::rtl::math::isFinite( nErgValue ) )
/*N*/ 		{
/*N*/ 			DBG_ERRORFILE( msgDbgInfinity );
/*N*/ 			nErgValue = 0.0;
/*N*/ 			pCode->SetError( errIllegalFPOperation );
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/ 		//	FORCED Zellen auch sofort auf Gueltigkeit testen (evtl. Makro starten)
/*N*/ 
/*N*/ 		if ( pCode->IsRecalcModeForced() )
/*N*/ 		{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			ULONG nValidation = ((const SfxUInt32Item*) pDocument->GetAttr(
/*N*/ 		}
/*N*/ 
/*N*/ 		// Reschedule verlangsamt das ganze erheblich, nur bei Prozentaenderung ausfuehren
/*N*/ 		ScProgress::GetInterpretProgress()->SetStateCountDownOnPercent(
/*N*/ 			pDocument->GetFormulaCodeInTree() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	Zelle bei Compiler-Fehlern nicht ewig auf dirty stehenlassen
/*N*/ 		DBG_ASSERT( pCode->GetError(), "kein UPN-Code und kein Fehler ?!?!" );
/*N*/ 		bDirty = FALSE;
/*N*/ 		bTableOpDirty = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ ULONG ScFormulaCell::GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const
/*N*/ {
/*N*/ 	if ( nFormatIndex && (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*N*/ 		return nFormatIndex;
/*N*/ 	if ( bIsValue )		//! nicht IsValue()
/*N*/ 		return ScGlobal::GetStandardFormat(	nErgValue, rFormatter, nFormat, nFormatType );
/*N*/ 	else
/*N*/ 		return ScGlobal::GetStandardFormat(	rFormatter, nFormat, nFormatType );
/*N*/ }


/*N*/ void __EXPORT ScFormulaCell::SFX_NOTIFY( SfxBroadcaster& rBC,
/*N*/ 		const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType )
/*N*/ {
/*N*/ 	if ( !pDocument->IsInDtorClear() && !pDocument->GetHardRecalcState() )
/*N*/ 	{
/*N*/ 		const ScHint* p = PTR_CAST( ScHint, &rHint );
/*N*/ 		if( p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING |
/*N*/ 				SC_HINT_TABLEOPDIRTY)) )
/*N*/ 		{
/*N*/             BOOL bForceTrack = FALSE;
/*N*/ 			if ( p->GetId() & SC_HINT_TABLEOPDIRTY )
/*N*/             {
/*?*/                 bForceTrack = !bTableOpDirty;
/*?*/                 if ( !bTableOpDirty )
/*?*/                 {
/*?*/                     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDocument->AddTableOpFormulaCell( this );
/*?*/                 }
/*N*/             }
/*N*/ 			else
/*N*/             {
/*N*/                 bForceTrack = !bDirty;
/*N*/ 				bDirty = TRUE;
/*N*/             }
/*N*/             // #35962# Don't remove from FormulaTree to put in FormulaTrack to
/*N*/             // put in FormulaTree again and again, only if necessary.
/*N*/             // Any other means except RECALCMODE_ALWAYS by which a cell could
/*N*/             // be in FormulaTree if it would notify other cells through
/*N*/             // FormulaTrack which weren't in FormulaTrack/FormulaTree before?!?
/*N*/             // #87866# Yes. The new TableOpDirty made it necessary to have a
/*N*/             // forced mode where formulas may still be in FormulaTree from
/*N*/             // TableOpDirty but have to notify dependents for normal dirty.
/*N*/             if ( (bForceTrack || !pDocument->IsInFormulaTree( this )
/*N*/ 					|| pCode->IsRecalcModeAlways())
/*N*/ 					&& !pDocument->IsInFormulaTrack( this ) )
/*N*/ 				pDocument->AppendToFormulaTrack( this );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScFormulaCell::SetDirty()
/*N*/ {
/*N*/ 	if ( !IsInChangeTrack() )
/*N*/ 	{
/*N*/ 		if ( pDocument->GetHardRecalcState() )
/*N*/ 			bDirty = TRUE;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Mehrfach-FormulaTracking in Load und in CompileAll
/*N*/ 			// nach CopyScenario und CopyBlockFromClip vermeiden.
/*N*/ 			// Wenn unbedingtes FormulaTracking noetig, vor SetDirty bDirty=FALSE
/*N*/ 			// setzen, z.B. in CompileTokenArray
/*N*/ 			if ( !bDirty || !pDocument->IsInFormulaTree( this ) )
/*N*/ 			{
/*N*/ 				bDirty = TRUE;
/*N*/ 				pDocument->AppendToFormulaTrack( this );
/*N*/ 				pDocument->TrackFormulas();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/  void ScFormulaCell::SetTableOpDirty()
/*N*/  {
/*N*/  	if ( !IsInChangeTrack() )
/*N*/  	{
/*N*/  		if ( pDocument->GetHardRecalcState() )
/*N*/  			bTableOpDirty = TRUE;
/*N*/  		else
/*N*/  		{
/*N*/  			if ( !bTableOpDirty || !pDocument->IsInFormulaTree( this ) )
/*N*/  			{
/*N*/                  if ( !bTableOpDirty )
/*N*/                  {
/*N*/                      pDocument->AddTableOpFormulaCell( this );
/*N*/                      bTableOpDirty = TRUE;
/*N*/                  }
/*N*/  				pDocument->AppendToFormulaTrack( this );
/*N*/  				pDocument->TrackFormulas( SC_HINT_TABLEOPDIRTY );
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  }


/*N*/ BOOL ScFormulaCell::IsDirtyOrInTableOpDirty()
/*N*/ {
/*N*/ 	return bDirty || (bTableOpDirty && pDocument->IsInInterpreterTableOp());
/*N*/ }


/*N*/ void ScFormulaCell::SetErrCode( USHORT n )
/*N*/ {
/*N*/ 	pCode->SetError( n );
/*N*/ 	bIsValue = FALSE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScDetectiveRefIter::ScDetectiveRefIter( ScFormulaCell* pCell )
/*N*/ {
/*N*/ 	pCode = pCell->GetCode();
/*N*/ 	pCode->Reset();
/*N*/ 	aPos = pCell->aPos;
/*N*/ }

/*N*/ BOOL lcl_ScDetectiveRefIter_SkipRef( ScToken* p )
/*N*/ {
/*N*/ 	SingleRefData& rRef1 = p->GetSingleRef();
/*N*/ 	if ( rRef1.IsColDeleted() || rRef1.IsRowDeleted() || rRef1.IsTabDeleted()
/*N*/ 			|| !rRef1.Valid() )
/*N*/ 		return TRUE;
/*N*/ 	if ( p->GetType() == svDoubleRef )
/*N*/ 	{
/*N*/ 		SingleRefData& rRef2 = p->GetDoubleRef().Ref2;
/*N*/ 		if ( rRef2.IsColDeleted() || rRef2.IsRowDeleted() || rRef2.IsTabDeleted()
/*N*/ 				|| !rRef2.Valid() )
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDetectiveRefIter::GetNextRef( ScTripel& rStart, ScTripel& rEnd )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 
/*N*/ 	ScToken* p = pCode->GetNextReferenceRPN();
/*N*/ 	if (p)
/*N*/ 		p->CalcAbsIfRel( aPos );
/*N*/ 
/*N*/ 	while ( p && lcl_ScDetectiveRefIter_SkipRef( p ) )
/*N*/ 	{
/*N*/ 		p = pCode->GetNextReferenceRPN();
/*N*/ 		if (p)
/*N*/ 			p->CalcAbsIfRel( aPos );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( p )
/*N*/ 	{
/*N*/ 		SingleDoubleRefProvider aProv( *p );
/*N*/ 		rStart.Put( aProv.Ref1.nCol,
/*N*/ 					aProv.Ref1.nRow,
/*N*/ 					aProv.Ref1.nTab );
/*N*/ 		rEnd.Put( aProv.Ref2.nCol,
/*N*/ 				  aProv.Ref2.nRow,
/*N*/ 				  aProv.Ref2.nTab );
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

//-----------------------------------------------------------------------------------

/*N*/ ScFormulaCell::~ScFormulaCell()
/*N*/ {
/*N*/ 	pDocument->RemoveFromFormulaTree( this );
/*N*/ 	delete pCode;
/*N*/ 	delete pMatrix;
/*N*/ 	pMatrix = NULL;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	eCellType = CELLTYPE_DESTROYED;
/*N*/ #endif
/*N*/ }


/*N*/ #ifdef DBG_UTIL
/*N*/ 
/*N*/ ScStringCell::~ScStringCell()
/*N*/ {
/*N*/ 	eCellType = CELLTYPE_DESTROYED;
/*N*/ }
/*N*/ #endif
                                    //! ValueCell auch nur bei DBG_UTIL,
                                    //! auch in cell.hxx aendern !!!!!!!!!!!!!!!!!!!!

/*N*/ ScValueCell::~ScValueCell()
/*N*/ {
/*N*/ 	eCellType = CELLTYPE_DESTROYED;
/*N*/ }

/*N*/ #ifdef DBG_UTIL
/*N*/ 
/*N*/ ScNoteCell::~ScNoteCell()
/*N*/ {
/*N*/ 	eCellType = CELLTYPE_DESTROYED;
/*N*/ }
/*N*/ #endif




}
