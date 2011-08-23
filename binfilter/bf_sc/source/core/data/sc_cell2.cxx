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

#ifndef _SV_MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#ifndef _EDITOBJ_HXX //autogen
#include <bf_svx/editobj.hxx>
#endif

#include "cell.hxx"
#include "rangenam.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "editutil.hxx"
#include "chgtrack.hxx"
#include "indexmap.hxx"
namespace binfilter {


// STATIC DATA -----------------------------------------------------------

#ifdef _MSC_VER
#pragma code_seg("SCSTATICS")
#endif

#ifdef USE_MEMPOOL
const USHORT nMemPoolEditCell = (0x1000 - 64) / sizeof(ScNoteCell);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScEditCell, nMemPoolEditCell, nMemPoolEditCell )
#endif

#ifdef _MSC_VER
#pragma code_seg()
#endif

// -----------------------------------------------------------------------

/*N*/ ScEditCell::ScEditCell( const EditTextObject* pObject, ScDocument* pDocP,
/*N*/ 			const SfxItemPool* pFromPool )  :
/*N*/ 		ScBaseCell( CELLTYPE_EDIT ),
/*N*/ 		pString( NULL ),
/*N*/ 		pDoc( pDocP )
/*N*/ {
/*N*/ 	SetTextObject( pObject, pFromPool );
/*N*/ }

/*N*/ ScEditCell::ScEditCell( const ScEditCell& rEditCell, ScDocument* pDocP )  :
/*N*/ 		ScBaseCell( CELLTYPE_EDIT ),
/*N*/ 		pString( NULL ),
/*N*/ 		pDoc( pDocP )
/*N*/ {
/*N*/ 	SetTextObject( rEditCell.pData, rEditCell.pDoc->GetEditPool() );
/*N*/ }

/*N*/ ScEditCell::ScEditCell( SvStream& rStream, USHORT nVer, ScDocument* pDocP ) :
/*N*/ 		ScBaseCell( CELLTYPE_EDIT ),
/*N*/ 		pString( NULL ),
/*N*/ 		pDoc( pDocP )
/*N*/ {
/*N*/ 	if( nVer >= SC_DATABYTES2 )
/*N*/ 	{
/*N*/ 		BYTE cData;
/*N*/ 		rStream >> cData;
/*N*/ 		if( cData & 0x0F )
/*?*/ 			rStream.SeekRel( cData & 0x0F );
/*N*/ 	}
/*N*/ 	if ( nVer < SC_VERSION_EDITPOOL )
/*N*/ 	{	// jedes hat seinen eigenen Pool, dem neuen zuordnen
/*?*/ 		EditTextObject* pTmp = EditTextObject::Create( rStream );
/*?*/ 		SetTextObject( pTmp, NULL );
/*?*/ 		delete pTmp;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pData = EditTextObject::Create( rStream, pDoc->GetEditPool() );
/*N*/ }

/*N*/ ScEditCell::ScEditCell( const String& rString, ScDocument* pDocP )  :
/*N*/ 		ScBaseCell( CELLTYPE_EDIT ),
/*N*/ 		pString( NULL ),
/*N*/ 		pDoc( pDocP )
/*N*/ {
/*N*/ 	DBG_ASSERT( rString.Search('\n') != STRING_NOTFOUND ||
/*N*/ 				rString.Search(CHAR_CR) != STRING_NOTFOUND,
/*N*/ 				"EditCell mit einfachem Text !?!?" );
/*N*/ 
/*N*/ 	EditEngine& rEngine = pDoc->GetEditEngine();
/*N*/ 	rEngine.SetText( rString );
/*N*/ 	pData = rEngine.CreateTextObject();
/*N*/ }

/*N*/ ScEditCell::~ScEditCell()
/*N*/ {
/*N*/ 	delete pData;
/*N*/ 	delete pString;
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	eCellType = CELLTYPE_DESTROYED;
/*N*/ #endif
/*N*/ }



/*N*/ void ScEditCell::GetData( const EditTextObject*& rpObject ) const
/*N*/ {
/*N*/ 	rpObject = pData;
/*N*/ }

/*N*/ void ScEditCell::GetString( String& rString ) const
/*N*/ {
/*N*/ 	if ( pString )
/*N*/ 		rString = *pString;
/*N*/ 	else if ( pData )
/*N*/ 	{
/*N*/ 		// auch Text von URL-Feldern, Doc-Engine ist eine ScFieldEditEngine
/*N*/ 		EditEngine& rEngine = pDoc->GetEditEngine();
/*N*/ 		rEngine.SetText( *pData );
/*N*/ 		rString = ScEditUtil::GetSpaceDelimitedString(rEngine);		// space between paragraphs
/*N*/ 		// kurze Strings fuer Formeln merken
/*N*/ 		if ( rString.Len() < MAXSTRLEN )
/*N*/ 			((ScEditCell*)this)->pString = new String( rString );	//! non-const
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }

/*N*/ void ScEditCell::Save( SvStream& rStream ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(pData,"StoreTextObject(NULL)");
/*N*/ 	rStream << (BYTE) 0x00;
/*N*/ 	if ( rStream.GetVersion() < SOFFICE_FILEFORMAT_50 )
/*N*/ 	{	// jedem seinen eigenen Pool
/*?*/ 		ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), TRUE );
/*?*/ 		// #52396# richtige Metric schreiben
/*?*/ 		aEngine.SetRefMapMode( MAP_100TH_MM );
/*?*/ 		aEngine.SetText( *pData );
/*?*/ 		EditTextObject* pTmp = aEngine.CreateTextObject();
/*?*/ 		pTmp->Store( rStream );
/*?*/ 		delete pTmp;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pData->Store( rStream );
/*N*/ }

/*N*/ void ScEditCell::SetTextObject( const EditTextObject* pObject,
/*N*/ 			const SfxItemPool* pFromPool )
/*N*/ {
/*N*/ 	if ( pObject )
/*N*/ 	{
/*N*/ 		if ( pFromPool && pDoc->GetEditPool() == pFromPool )
/*N*/ 			pData = pObject->Clone();
/*N*/ 		else
/*N*/ 		{	//! anderer Pool
/*?*/ 			// Leider gibt es keinen anderen Weg, um den Pool umzuhaengen,
/*N*/ 	// als das Object durch eine entsprechende Engine zu schleusen.. // cellformats.sdc
/*N*/ /*?*/ 			EditEngine& rEngine = pDoc->GetEditEngine();
/*N*/ /*?*/ 				rEngine.SetText( *pObject );
/*N*/ /*?*/ 				pData = rEngine.CreateTextObject();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pData = NULL;
/*N*/ }

//---------------------------------------------------------------------

/*N*/ void ScFormulaCell::GetEnglishFormula( String& rFormula, BOOL bCompileXML ) const
/*N*/ {
/*N*/ 	//!	mit GetFormula zusammenfassen !!!
/*N*/ 
/*N*/ 	if( pCode->GetError() && !pCode->GetLen() )
/*N*/ 	{
/*N*/ 		rFormula = ScGlobal::GetErrorString( pCode->GetError() ); return;
/*N*/ 	}
/*N*/ 	else if( cMatrixFlag == MM_REFERENCE )
/*N*/ 	{
/*N*/ 		// Referenz auf eine andere Zelle, die eine Matrixformel enthaelt
/*N*/ 		pCode->Reset();
/*N*/ 		ScToken* p = pCode->GetNextReferenceRPN();
/*N*/ 		if( p )
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell;
/*N*/ 			SingleRefData& rRef = p->GetSingleRef();
/*N*/ 			rRef.CalcAbsIfRel( aPos );
/*N*/ 			if ( rRef.Valid() )
/*N*/ 				pCell = pDocument->GetCell( ScAddress( rRef.nCol,
/*N*/ 					rRef.nRow, rRef.nTab ) );
/*N*/ 			else
/*N*/ 				pCell = NULL;
/*N*/ 			if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			{
/*N*/ 				((ScFormulaCell*)pCell)->GetEnglishFormula(rFormula, bCompileXML);
/*N*/ 				return;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				ScCompiler aComp( pDocument, aPos, *pCode );
/*N*/ 				aComp.SetCompileEnglish( TRUE );
/*N*/ 				aComp.SetCompileXML( bCompileXML );
/*N*/ 				aComp.CreateStringFromTokenArray( rFormula );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			DBG_ERROR("ScFormulaCell::GetEnglishFormula: Keine Matrix");
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScCompiler aComp( pDocument, aPos, *pCode );
/*N*/ 		aComp.SetCompileEnglish( TRUE );
/*N*/ 		aComp.SetCompileXML( bCompileXML );
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

/*N*/ void ScFormulaCell::GetEnglishFormula( ::rtl::OUStringBuffer& rBuffer, BOOL bCompileXML ) const
/*N*/ {
/*N*/ 	//!	mit GetFormula zusammenfassen !!!
/*N*/ 
/*N*/ 	if( pCode->GetError() && !pCode->GetLen() )
/*N*/ 	{
/*?*/ 		rBuffer = ::rtl::OUStringBuffer(ScGlobal::GetErrorString( pCode->GetError() )); return;
/*N*/ 	}
/*N*/ 	else if( cMatrixFlag == MM_REFERENCE )
/*N*/ 	{
/*?*/ 		// Referenz auf eine andere Zelle, die eine Matrixformel enthaelt
/*?*/ 		pCode->Reset();
/*?*/ 		ScToken* p = pCode->GetNextReferenceRPN();
/*?*/ 		if( p )
/*?*/ 		{
/*?*/ 			ScBaseCell* pCell;
/*?*/ 			SingleRefData& rRef = p->GetSingleRef();
/*?*/ 			rRef.CalcAbsIfRel( aPos );
/*?*/ 			if ( rRef.Valid() )
/*?*/ 				pCell = pDocument->GetCell( ScAddress( rRef.nCol,
/*?*/ 					rRef.nRow, rRef.nTab ) );
/*?*/ 			else
/*?*/ 				pCell = NULL;
/*?*/ 			if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
/*?*/ 			{
/*?*/ 				((ScFormulaCell*)pCell)->GetEnglishFormula(rBuffer, bCompileXML);
/*?*/ 				return;
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				ScCompiler aComp( pDocument, aPos, *pCode );
/*?*/ 				aComp.SetCompileEnglish( TRUE );
/*?*/ 				aComp.SetCompileXML( bCompileXML );
/*?*/ 				aComp.CreateStringFromTokenArray( rBuffer );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			DBG_ERROR("ScFormulaCell::GetEnglishFormula: Keine Matrix");
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScCompiler aComp( pDocument, aPos, *pCode );
/*N*/ 		aComp.SetCompileEnglish( TRUE );
/*N*/ 		aComp.SetCompileXML( bCompileXML );
/*N*/ 		aComp.CreateStringFromTokenArray( rBuffer );
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_Unicode ch('=');
/*N*/ 	rBuffer.insert( 0, &ch, 1 );
/*N*/ 	if( cMatrixFlag )
/*N*/ 	{
/*N*/ 		sal_Unicode ch2('{');
/*N*/ 		rBuffer.insert(0, &ch2, 1);
/*N*/ 		rBuffer.append(sal_Unicode('}'));
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScFormulaCell::IsValue()
/*N*/ {
/*N*/ 	if (IsDirtyOrInTableOpDirty() && pDocument->GetAutoCalc())
/*N*/ 		Interpret();
/*N*/ 	return bIsValue;
/*N*/ }

/*N*/ double ScFormulaCell::GetValue()
/*N*/ {
/*N*/ 	if (IsDirtyOrInTableOpDirty() && pDocument->GetAutoCalc())
/*?*/ 		Interpret();
/*N*/ 	if ( !pCode->GetError() || pCode->GetError() == errDoubleRef)
/*N*/ 		return nErgValue;
/*N*/ 	return 0.0;
/*N*/ }

/*N*/ double ScFormulaCell::GetValueAlways()
/*N*/ {
/*N*/ 	// for goal seek: return result value even if error code is set
/*N*/ 
/*N*/ 	if (IsDirtyOrInTableOpDirty() && pDocument->GetAutoCalc())
/*N*/ 		Interpret();
/*N*/ 	return nErgValue;
/*N*/ }

/*N*/ void ScFormulaCell::GetString( String& rString )
/*N*/ {
/*N*/ 	if (IsDirtyOrInTableOpDirty() && pDocument->GetAutoCalc())
/*?*/ 		Interpret();
/*N*/ 	if ( !pCode->GetError() || pCode->GetError() == errDoubleRef)
/*N*/ 		rString = aErgString;
/*N*/ 	else
/*?*/ 		rString.Erase();
/*N*/ }

/*N*/ void ScFormulaCell::GetMatrix(ScMatrix** ppMat)
/*N*/ {
/*N*/ 	if ( pDocument->GetAutoCalc() )
/*N*/ 	{
/*N*/ 		// war !bDirty gespeichert aber zugehoerige Matrixzelle bDirty?
/*N*/ 		// => wir brauchen pMatrix
/*N*/ 		if ( !pMatrix && cMatrixFlag == MM_FORMULA )
/*N*/ 			bDirty = TRUE;
/*N*/ 		if ( IsDirtyOrInTableOpDirty() )
/*N*/ 			Interpret();
/*N*/ 	}
/*N*/ 	if (!pCode->GetError())
/*N*/ 		*ppMat = pMatrix;
/*N*/ 	else
/*N*/ 		*ppMat = NULL;
/*N*/ }

/*N*/ BOOL ScFormulaCell::GetMatrixOrigin( ScAddress& rPos ) const
/*N*/ {
/*N*/ 	switch ( cMatrixFlag )
/*N*/ 	{
/*N*/ 		case MM_FORMULA :
/*N*/ 			rPos = aPos;
/*N*/ 			return TRUE;
/*N*/ 		break;
/*N*/ 		case MM_REFERENCE :
/*N*/ 		{
/*N*/ 			pCode->Reset();
/*N*/ 			ScToken* t = pCode->GetNextReferenceRPN();
/*N*/ 			if( t )
/*N*/ 			{
/*N*/ 				SingleRefData& rRef = t->GetSingleRef();
/*N*/ 				rRef.CalcAbsIfRel( aPos );
/*N*/ 				if ( rRef.Valid() )
/*N*/ 				{
/*N*/ 					rPos.Set( rRef.nCol, rRef.nRow, rRef.nTab );
/*N*/ 					return TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*
 Edge-Values:

   8
 4   16
   2

 innerhalb: 1
 ausserhalb: 0
 (reserviert: offen: 32)
 */

/*N*/ USHORT ScFormulaCell::GetMatrixEdge( ScAddress& rOrgPos )
/*N*/ {
/*N*/ 	switch ( cMatrixFlag )
/*N*/ 	{
/*N*/ 		case MM_FORMULA :
/*N*/ 		case MM_REFERENCE :
/*N*/ 		{
/*N*/ 			static USHORT nC, nR;
/*N*/ 			ScAddress aOrg;
/*N*/ 			if ( !GetMatrixOrigin( aOrg ) )
/*N*/ 				return 0;				// dumm gelaufen..
/*N*/ 			if ( aOrg != rOrgPos )
/*N*/ 			{	// erstes Mal oder andere Matrix als letztes Mal
/*N*/ 				rOrgPos = aOrg;
/*N*/ 				ScFormulaCell* pFCell;
/*N*/ 				if ( cMatrixFlag == MM_REFERENCE )
/*N*/ 					pFCell = (ScFormulaCell*) pDocument->GetCell( aOrg );
/*N*/ 				else
/*N*/ 					pFCell = this;		// this MM_FORMULA
/*N*/ 				// this gibt's nur einmal, kein Vergleich auf pFCell==this
/*N*/ 				if ( pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA
/*N*/ 				  && pFCell->cMatrixFlag == MM_FORMULA )
/*N*/ 				{
/*N*/ 					pFCell->GetMatColsRows( nC, nR );
/*N*/ 					if ( nC == 0 || nR == 0 )
/*N*/ 					{	// aus altem Dokument geladen, neu erzeugen
/*?*/ 						nC = nR = 1;
/*?*/ 						ScAddress aTmpOrg;
/*?*/ 						ScBaseCell* pCell;
/*?*/ 						ScAddress aAdr( aOrg );
/*?*/ 						aAdr.IncCol();
/*?*/ 						BOOL bCont = TRUE;
/*?*/ 						do
/*?*/ 						{
/*?*/ 							pCell = pDocument->GetCell( aAdr );
/*?*/ 							if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA
/*?*/ 							  && ((ScFormulaCell*)pCell)->cMatrixFlag == MM_REFERENCE
/*?*/ 							  && GetMatrixOrigin( aTmpOrg ) && aTmpOrg == aOrg )
/*?*/ 							{
/*?*/ 								nC++;
/*?*/ 								aAdr.IncCol();
/*?*/ 							}
/*?*/ 							else
/*?*/ 								bCont = FALSE;
/*?*/ 						} while ( bCont );
/*?*/ 						aAdr = aOrg;
/*?*/ 						aAdr.IncRow();
/*?*/ 						bCont = TRUE;
/*?*/ 						do
/*?*/ 						{
/*?*/ 							pCell = pDocument->GetCell( aAdr );
/*?*/ 							if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA
/*?*/ 							  && ((ScFormulaCell*)pCell)->cMatrixFlag == MM_REFERENCE
/*?*/ 							  && GetMatrixOrigin( aTmpOrg ) && aTmpOrg == aOrg )
/*?*/ 							{
/*?*/ 								nR++;
/*?*/ 								aAdr.IncRow();
/*?*/ 							}
/*?*/ 							else
/*?*/ 								bCont = FALSE;
/*?*/ 						} while ( bCont );
/*?*/ 						pFCell->SetMatColsRows( nC, nR );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ #ifdef DBG_UTIL
/*N*/ 					String aTmp;
/*N*/ 					ByteString aMsg( "broken Matrix, no MatFormula at origin, Pos: " );
/*N*/ 					aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
/*N*/ 					aMsg += ByteString( aTmp, RTL_TEXTENCODING_ASCII_US );
/*N*/ 					aMsg += ", MatOrg: ";
/*N*/ 					aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
/*N*/ 					aMsg += ByteString( aTmp, RTL_TEXTENCODING_ASCII_US );
/*N*/ 					DBG_ERRORFILE( aMsg.GetBuffer() );
/*N*/ #endif
/*N*/ 					return 0;           // bad luck ...
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// here we are, healthy and clean, somewhere in between
/*N*/ 			short dC = aPos.Col() - aOrg.Col();
/*N*/ 			short dR = aPos.Row() - aOrg.Row();
/*N*/ 			USHORT nEdges = 0;
/*N*/ 			if ( dC >= 0 && dR >= 0 && dC < nC && dR < nR )
/*N*/ 			{
/*N*/ 				if ( dC == 0 )
/*N*/ 					nEdges |= 4;            // linke Kante
/*N*/ 				if ( dC+1 == nC )
/*N*/ 					nEdges |= 16;			// rechte Kante
/*N*/ 				if ( dR == 0 )
/*N*/ 					nEdges |= 8;            // obere Kante
/*N*/ 				if ( dR+1 == nR )
/*N*/ 					nEdges |= 2;			// untere Kante
/*N*/ 				if ( !nEdges )
/*N*/ 					nEdges = 1;				// mittendrin
/*N*/ 			}
/*N*/ #ifdef DBG_UTIL
/*N*/ 			else
/*N*/ 			{
/*?*/ 				String aTmp;
/*?*/ 				ByteString aMsg( "broken Matrix, Pos: " );
/*?*/ 				aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
/*?*/ 				aMsg += ByteString( aTmp, RTL_TEXTENCODING_ASCII_US );
/*?*/ 				aMsg += ", MatOrg: ";
/*?*/ 				aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
/*?*/ 				aMsg += ByteString( aTmp, RTL_TEXTENCODING_ASCII_US );
/*?*/ 				aMsg += ", MatCols: ";
/*?*/ 				aMsg += ByteString::CreateFromInt32( nC );
/*?*/ 				aMsg += ", MatRows: ";
/*?*/ 				aMsg += ByteString::CreateFromInt32( nR );
/*?*/ 				aMsg += ", DiffCols: ";
/*?*/ 				aMsg += ByteString::CreateFromInt32( dC );
/*?*/ 				aMsg += ", DiffRows: ";
/*?*/ 				aMsg += ByteString::CreateFromInt32( dR );
/*?*/ 				DBG_ERRORFILE( aMsg.GetBuffer() );
/*N*/ 			}
/*N*/ #endif
/*N*/ 			return nEdges;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		default:
/*N*/ 			return 0;
/*N*/ 	}
/*N*/ }

/*N*/ USHORT ScFormulaCell::GetErrCode()
/*N*/ {
/*N*/ 	if (IsDirtyOrInTableOpDirty() && pDocument->GetAutoCalc())
/*N*/ 		Interpret();
/*N*/ 	return pCode->GetError();
/*N*/ }

/*N*/  BOOL ScFormulaCell::HasRelNameReference() const
/*N*/  {
/*N*/  	pCode->Reset();
/*N*/  	for( ScToken* t = pCode->GetNextReferenceRPN(); t;
/*N*/  				  t = pCode->GetNextReferenceRPN() )
/*N*/  	{
/*N*/  		if ( t->GetSingleRef().IsRelName() ||
/*N*/  				(t->GetType() == svDoubleRef &&
/*N*/  				t->GetDoubleRef().Ref2.IsRelName()) )
/*N*/  			return TRUE;
/*N*/  	}
/*N*/  	return FALSE;
/*N*/  }

/*N*/  void ScFormulaCell::UpdateReference(UpdateRefMode eUpdateRefMode,
/*N*/  									const ScRange& r,
/*N*/  									short nDx, short nDy, short nDz,
/*N*/  									ScDocument* pUndoDoc )
/*N*/  {DBG_BF_ASSERT(0, "STRIP");//STRIP001 
/*N*/  }
 
/*N*/ void ScFormulaCell::UpdateInsertTab(USHORT nTable)
/*N*/ {
/*N*/	BOOL bPosChanged = ( aPos.Tab() >= nTable ? TRUE : FALSE );
/*N*/	pCode->Reset();
/*N*/	if( pCode->GetNextReferenceRPN() && !pDocument->IsClipOrUndo() )
/*N*/	{
/*N*/		EndListeningTo( pDocument );
/*N*/		// IncTab _nach_ EndListeningTo und _vor_ Compiler UpdateInsertTab !
/*N*/		if ( bPosChanged )
/*?*/			aPos.IncTab();
/*N*/		ScRangeData* pRangeData;
/*N*/		ScCompiler aComp(pDocument, aPos, *pCode);
/*N*/		pRangeData = aComp.UpdateInsertTab( nTable, FALSE );
/*N*/		if (pRangeData)						// Shared Formula gegen echte Formel
/*N*/		{									// austauschen
/*?*/			BOOL bChanged;
/*?*/			pDocument->RemoveFromFormulaTree( this );	// update formula count
/*?*/			delete pCode;
/*?*/			pCode = new ScTokenArray( *pRangeData->GetCode() );
/*M*/             pCode->SetReplacedSharedFormula( TRUE );
/*?*/			ScCompiler aComp2(pDocument, aPos, *pCode);
/*?*/			aComp2.MoveRelWrap();
/*?*/			aComp2.UpdateInsertTab( nTable, FALSE );
/*?*/             // If the shared formula contained a named range/formula containing
/*?*/             // an absolute reference to a sheet, those have to be readjusted.
/*?*/			aComp2.UpdateDeleteTab( nTable, FALSE, TRUE, bChanged );
/*?*/			bCompile = TRUE;
/*N*/		}
/*N*/		// kein StartListeningTo weil pTab[nTab] noch nicht existiert!
/*N*/	}
/*N*/	else if ( bPosChanged )
/*?*/		aPos.IncTab();
/*N*/ }

/*N*/  BOOL ScFormulaCell::UpdateDeleteTab(USHORT nTable, BOOL bIsMove)
/*N*/  {
/*N*/  	BOOL bChanged = FALSE;
/*N*/  	BOOL bPosChanged = ( aPos.Tab() > nTable ? TRUE : FALSE );
/*N*/  	pCode->Reset();
/*N*/  	if( pCode->GetNextReferenceRPN() && !pDocument->IsClipOrUndo() )
/*N*/  	{
/*N*/  		EndListeningTo( pDocument );
/*N*/  		// IncTab _nach_ EndListeningTo und _vor_ Compiler UpdateDeleteTab !
/*N*/  		if ( bPosChanged )
/*N*/  			aPos.IncTab(-1);
/*N*/  		ScRangeData* pRangeData;
/*N*/  		ScCompiler aComp(pDocument, aPos, *pCode);
/*N*/  		pRangeData = aComp.UpdateDeleteTab(nTable, bIsMove, FALSE, bChanged);
/*N*/  		if (pRangeData)						// Shared Formula gegen echte Formel
/*N*/  		{									// austauschen
/*N*/  			pDocument->RemoveFromFormulaTree( this );	// update formula count
/*N*/  			delete pCode;
/*N*/  			pCode = pRangeData->GetCode()->Clone();
/*N*/              pCode->SetReplacedSharedFormula( TRUE );
/*N*/  			ScCompiler aComp2(pDocument, aPos, *pCode);
/*N*/  			aComp2.CompileTokenArray();
/*N*/  			aComp2.MoveRelWrap();
/*N*/  			aComp2.UpdateDeleteTab( nTable, FALSE, FALSE, bChanged );
/*N*/              // If the shared formula contained a named range/formula containing
/*N*/              // an absolute reference to a sheet, those have to be readjusted.
/*N*/  			aComp2.UpdateInsertTab( nTable,TRUE );
/*N*/  			// bChanged kann beim letzten UpdateDeleteTab zurueckgesetzt worden sein
/*N*/  			bChanged = TRUE;
/*N*/  			bCompile = TRUE;
/*N*/  		}
/*N*/  		// kein StartListeningTo weil pTab[nTab] noch nicht korrekt!
/*N*/  	}
/*N*/  	else if ( bPosChanged )
/*N*/  		aPos.IncTab(-1);
/*N*/  
/*N*/  	return bChanged;
/*N*/  }

/*N*/ BOOL ScFormulaCell::TestTabRefAbs(USHORT nTable)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
DBG_BF_ASSERT(0, "STRIP"); /*N*/  	if( !pDocument->IsClipOrUndo() )
/*N*/  	{
/*N*/  		pCode->Reset();
/*N*/  		ScToken* p = pCode->GetNextReferenceRPN();
/*N*/  		while( p )
/*N*/  		{
/*N*/  			SingleRefData& rRef1 = p->GetSingleRef();
/*N*/  			if( !rRef1.IsTabRel() )
/*N*/  			{
/*N*/  				if( (short) nTable != rRef1.nTab )
/*N*/  					bRet = TRUE;
/*N*/  				else if (nTable != aPos.Tab())
/*N*/  					rRef1.nTab = aPos.Tab();
/*N*/  			}
/*N*/  			if( p->GetType() == svDoubleRef )
/*N*/  			{
/*N*/  				SingleRefData& rRef2 = p->GetDoubleRef().Ref2;
/*N*/  				if( !rRef2.IsTabRel() )
/*N*/  				{
/*N*/  					if( (short) nTable != rRef2.nTab )
/*N*/  						bRet = TRUE;
/*N*/  					else if (nTable != aPos.Tab())
/*N*/  						rRef2.nTab = aPos.Tab();
/*N*/  				}
/*N*/  			}
/*N*/  			p = pCode->GetNextReferenceRPN();
/*N*/  		}
/*N*/  	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void ScFormulaCell::UpdateCompile( BOOL bForceIfNameInUse )
/*N*/ {
/*N*/ 	if ( bForceIfNameInUse && !bCompile )
/*?*/      { DBG_BF_ASSERT(0, "STRIP");} /*N*/    bCompile = pCode->HasNameOrColRowName();
/*N*/ 	if ( bCompile )
/*?*/ 		pCode->SetError( 0 );	// damit auch wirklich kompiliert wird
/*N*/ 	CompileTokenArray();
/*N*/ }

/*N*/ BOOL lcl_IsRangeNameInUse(USHORT nIndex, ScTokenArray* pCode, ScRangeName* pNames)
/*N*/ {
/*N*/ 	for (ScToken* p = pCode->First(); p; p = pCode->Next())
/*N*/ 	{
/*N*/ 		if (p->GetOpCode() == ocName)
/*N*/ 		{
/*N*/ 			if (p->GetIndex() == nIndex)
/*N*/ 				return TRUE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				//	RangeData kann Null sein in bestimmten Excel-Dateien (#31168#)
/*N*/ 				ScRangeData* pSubName = pNames->FindIndex(p->GetIndex());
/*N*/ 				if (pSubName && lcl_IsRangeNameInUse(nIndex,
/*N*/ 									pSubName->GetCode(), pNames))
/*N*/ 					return TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScFormulaCell::IsRangeNameInUse(USHORT nIndex) const
/*N*/ {
/*N*/ 	return lcl_IsRangeNameInUse( nIndex, pCode, pDocument->GetRangeName() );
/*N*/ }

/*N*/ void ScFormulaCell::CompileDBFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	// zwei Phasen, muessen (!) nacheinander aufgerufen werden:
/*N*/ 	// 1. FormelString mit alten Namen erzeugen
/*N*/ 	// 2. FormelString mit neuen Namen kompilieren
/*N*/ 	if ( bCreateFormulaString )
/*N*/ 	{
/*N*/ 		BOOL bRecompile = FALSE;
/*N*/ 		pCode->Reset();
/*N*/ 		for ( ScToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
/*N*/ 		{
/*N*/ 			switch ( p->GetOpCode() )
/*N*/ 			{
/*N*/ 				case ocBad:				// DB-Bereich evtl. zugefuegt
/*N*/ 				case ocColRowName:		// #36762# falls Namensgleichheit
/*N*/ 				case ocDBArea:			// DB-Bereich
/*N*/ 					bRecompile = TRUE;
/*N*/ 				break;
/*N*/ 				case ocName:
/*?*/ 					if ( p->GetIndex() >= SC_START_INDEX_DB_COLL )
/*?*/ 						bRecompile = TRUE;	// DB-Bereich
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bRecompile )
/*N*/ 		{
/*N*/ 			String aFormula;
/*N*/ 			GetFormula( aFormula );
/*N*/ 			if ( GetMatrixFlag() != MM_NONE && aFormula.Len() )
/*N*/ 			{
/*?*/ 				if ( aFormula.GetChar( aFormula.Len()-1 ) == '}' )
/*?*/ 					aFormula.Erase( aFormula.Len()-1 , 1 );
/*?*/ 				if ( aFormula.GetChar(0) == '{' )
/*?*/ 					aFormula.Erase( 0, 1 );
/*N*/ 			}
/*N*/ 			EndListeningTo( pDocument );
/*N*/ 			pDocument->RemoveFromFormulaTree( this );
/*N*/ 			pCode->Clear();
/*N*/ 			aErgString = aFormula;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( !pCode->GetLen() && aErgString.Len() )
/*N*/ 	{
/*N*/ 		Compile( aErgString );
/*N*/ 		aErgString.Erase();
/*N*/ 		SetDirty();
/*N*/ 	}
/*N*/ }

/*N*/ void ScFormulaCell::CompileNameFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	// zwei Phasen, muessen (!) nacheinander aufgerufen werden:
/*N*/ 	// 1. FormelString mit alten RangeNames erzeugen
/*N*/ 	// 2. FormelString mit neuen RangeNames kompilieren
/*N*/ 	if ( bCreateFormulaString )
/*N*/ 	{
/*N*/ 		BOOL bRecompile = FALSE;
/*N*/ 		pCode->Reset();
/*N*/ 		for ( ScToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
/*N*/ 		{
/*N*/ 			switch ( p->GetOpCode() )
/*N*/ 			{
/*N*/ 				case ocBad:				// RangeName evtl. zugefuegt
/*N*/ 				case ocColRowName:		// #36762# falls Namensgleichheit
/*N*/ 					bRecompile = TRUE;
/*N*/ 				break;
/*N*/ 				default:
/*N*/ 					if ( p->GetType() == svIndex )
/*N*/ 						bRecompile = TRUE;	// RangeName
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bRecompile )
/*N*/ 		{
/*N*/ 			String aFormula;
/*N*/ 			GetFormula( aFormula );
/*N*/ 			if ( GetMatrixFlag() != MM_NONE && aFormula.Len() )
/*N*/ 			{
/*?*/ 				if ( aFormula.GetChar( aFormula.Len()-1 ) == '}' )
/*?*/ 					aFormula.Erase( aFormula.Len()-1 , 1 );
/*?*/ 				if ( aFormula.GetChar(0) == '{' )
/*?*/ 					aFormula.Erase( 0, 1 );
/*N*/ 			}
/*N*/ 			EndListeningTo( pDocument );
/*N*/ 			pDocument->RemoveFromFormulaTree( this );
/*N*/ 			pCode->Clear();
/*N*/ 			aErgString = aFormula;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( !pCode->GetLen() && aErgString.Len() )
/*N*/ 	{
/*N*/ 		Compile( aErgString );
/*N*/ 		aErgString.Erase();
/*N*/ 		SetDirty();
/*N*/ 	}
/*N*/ }

/*N*/ void ScFormulaCell::CompileColRowNameFormula()
/*N*/ {
/*N*/ 	pCode->Reset();
/*N*/ 	for ( ScToken* p = pCode->First(); p; p = pCode->Next() )
/*N*/ 	{
/*N*/ 		if ( p->GetOpCode() == ocColRowName )
/*N*/ 		{
/*N*/ 			bCompile = TRUE;
/*N*/ 			CompileTokenArray();
/*N*/ 			SetDirty();
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ ScValueCell::ScValueCell( SvStream& rStream, USHORT nVer ) :
/*N*/ 	ScBaseCell( CELLTYPE_VALUE )
/*N*/ {
/*N*/ 	if( nVer >= SC_DATABYTES2 )
/*N*/ 	{
/*N*/ 		BYTE cData;
/*N*/ 		rStream >> cData;
/*N*/ 		if( cData & 0x0F )
/*?*/ 			rStream.SeekRel( cData & 0x0F );
/*N*/ 	}
/*N*/ 	rStream >> aValue;
/*N*/ }

/*N*/ void ScValueCell::Save( SvStream& rStream ) const
/*N*/ {
/*N*/ 	rStream << (BYTE) 0x00 << aValue;
/*N*/ }

/*N*/ ScStringCell::ScStringCell( SvStream& rStream, USHORT nVer ) :
/*N*/ 	ScBaseCell( CELLTYPE_STRING )
/*N*/ {
/*N*/ 	if( nVer >= SC_DATABYTES2 )
/*N*/ 	{
/*N*/ 		BYTE cData;
/*N*/ 		rStream >> cData;
/*N*/ 		if( cData & 0x0F )
/*?*/ 			rStream.SeekRel( cData & 0x0F );
/*N*/ 	}
/*N*/ 	rStream.ReadByteString( aString, rStream.GetStreamCharSet() );
/*N*/ }

/*N*/ void ScStringCell::Save( SvStream& rStream, FontToSubsFontConverter hConv ) const
/*N*/ {
/*N*/ 	rStream << (BYTE) 0x00;
/*N*/     if ( !hConv )
/*N*/         rStream.WriteByteString( aString, rStream.GetStreamCharSet() );
/*N*/     else
/*N*/     {
/*N*/         String aTmp( aString );
/*N*/         sal_Unicode* p = aTmp.GetBufferAccess();
/*N*/         sal_Unicode const * const pStop = p + aTmp.Len();
/*N*/         for ( ; p < pStop; ++p )
/*N*/         {
/*N*/             *p = ConvertFontToSubsFontChar( hConv, *p );
/*N*/         }
/*N*/         aTmp.ReleaseBufferAccess();
/*N*/         rStream.WriteByteString( aTmp, rStream.GetStreamCharSet() );
/*N*/     }
/*N*/ }

/*N*/ void ScStringCell::ConvertFont( FontToSubsFontConverter hConv )
/*N*/ {
/*N*/     if ( hConv )
/*N*/     {
/*N*/         sal_Unicode* p = aString.GetBufferAccess();
/*N*/         sal_Unicode const * const pStop = p + aString.Len();
/*N*/         for ( ; p < pStop; ++p )
/*N*/         {
/*N*/             *p = ConvertFontToSubsFontChar( hConv, *p );
/*N*/         }
/*N*/         aString.ReleaseBufferAccess();
/*N*/     }
/*N*/ }

/*N*/ ScNoteCell::ScNoteCell( SvStream& rStream, USHORT nVer ) :
/*N*/ 	ScBaseCell( CELLTYPE_NOTE )
/*N*/ {
/*N*/ 	if( nVer >= SC_DATABYTES2 )
/*N*/ 	{
/*N*/ 		BYTE cData;
/*N*/ 		rStream >> cData;
/*N*/ 		if( cData & 0x0F )
/*?*/ 			rStream.SeekRel( cData & 0x0F );
/*N*/ 	}
/*N*/ }

/*N*/ void ScNoteCell::Save( SvStream& rStream ) const
/*N*/ {
/*N*/ 	rStream << (BYTE) 0x00;
/*N*/ }





}
