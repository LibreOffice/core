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

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/boxitem.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/editeng.hxx>
#include <bf_svtools/poolcach.hxx>
#include <bf_svtools/saveopt.hxx>
#include <bf_svtools/zforlist.hxx>
#include <tools/tenccvt.hxx>

#include "attarray.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "globstr.hrc"
#include "rechead.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "chartlis.hxx"
#include "drwlayer.hxx"
#include "validat.hxx"
#include "prnsave.hxx"
#include "chgtrack.hxx"
#include "bf_sc.hrc"
#include "scresid.hxx"
#include "hints.hxx"
#include "detdata.hxx"
#include "cell.hxx"
#include "dpobject.hxx"
#include "indexmap.hxx"
#include "detfunc.hxx"		// for UpdateAllComments
#include "scmod.hxx"
#include "dociter.hxx"
#include "progress.hxx"
#ifndef __SGI_STL_SET
#include <set>
#endif
namespace binfilter {

/*N*/ struct ScDefaultAttr
/*N*/ {
/*N*/ 	const ScPatternAttr*	pAttr;
/*N*/ 	USHORT					nFirst;
/*N*/ 	USHORT					nCount;
/*N*/ 	ScDefaultAttr(const ScPatternAttr* pPatAttr) : pAttr(pPatAttr), nFirst(0), nCount(0) {}
/*N*/ };

/*N*/ struct ScLessDefaultAttr
/*N*/ {
/*N*/ 	sal_Bool operator() (const ScDefaultAttr& rValue1, const ScDefaultAttr& rValue2) const
/*N*/ 	{
/*N*/ 		return rValue1.pAttr < rValue2.pAttr;
/*N*/ 	}
/*N*/ };

/*N*/ typedef std::set<ScDefaultAttr, ScLessDefaultAttr>	ScDefaultAttrSet;

/*N*/ void ScDocument::MakeTable( USHORT nTab )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && !pTab[nTab] )
/*N*/ 	{
/*N*/ 		String aString = ScGlobal::GetRscString(STR_TABLE_DEF); //"Tabelle"
/*N*/ 		aString += String::CreateFromInt32(nTab+1);
/*N*/ 		CreateValidTabName( aString );	// keine doppelten
/*N*/
/*N*/ 		pTab[nTab] = new ScTable(this, nTab, aString);
/*N*/ 		++nMaxTableNumber;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScDocument::HasTable( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return TRUE;
/*N*/
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::GetName( USHORT nTab, String& rName ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			pTab[nTab]->GetName( rName );
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	rName.Erase();
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::GetTable( const String& rName, USHORT& rTab ) const
/*N*/ {
/*N*/ 	String aUpperName = rName;
/*N*/ 	ScGlobal::pCharClass->toUpper(aUpperName);
/*N*/ 	String aCompName;
/*N*/
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 		{
/*N*/ 			pTab[i]->GetName( aCompName );
/*N*/ 			ScGlobal::pCharClass->toUpper(aCompName);
/*N*/ 			if (aUpperName == aCompName)
/*N*/ 			{
/*N*/ 				rTab = i;
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	rTab = 0;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::ValidTabName( const String& rName ) const
/*N*/ {
    /*  If changed, ScfTools::ConvertToScSheetName (sc/source/filter/ftools/ftools.cxx)
        needs to be changed too. */
/*N*/ 	using namespace ::com::sun::star::i18n;
/*N*/ 	sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
/*N*/ 		KParseTokens::ASC_UNDERSCORE;
/*N*/ 	sal_Int32 nContFlags = nStartFlags;
/*N*/ 	String aContChars( RTL_CONSTASCII_USTRINGPARAM(" ") );
/*N*/ 	ParseResult rRes = ScGlobal::pCharClass->parsePredefinedToken( KParseType::IDENTNAME, rName, 0,
/*N*/ 		nStartFlags, EMPTY_STRING, nContFlags, aContChars );
/*N*/ 	return (rRes.TokenType & KParseType::IDENTNAME) && rRes.EndPos == rName.Len();
/*N*/ }


/*N*/ BOOL ScDocument::ValidNewTabName( const String& rName ) const
/*N*/ {
/*N*/ 	BOOL bValid = ValidTabName(rName);
/*N*/ 	for (USHORT i=0; (i<=MAXTAB) && bValid; i++)
/*N*/ 		if (pTab[i])
/*N*/ 		{
/*N*/ 			String aOldName;
/*N*/ 			pTab[i]->GetName(aOldName);
/*N*/             bValid = !ScGlobal::pTransliteration->isEqual( rName, aOldName );
/*N*/ 		}
/*N*/ 	return bValid;
/*N*/ }


/*N*/ void ScDocument::CreateValidTabName(String& rName) const
/*N*/ {
/*N*/ 	if ( !ValidTabName(rName) )
/*N*/ 	{
/*?*/ 		// neu erzeugen
/*?*/
/*?*/ 		const String aStrTable( ScResId(SCSTR_TABLE) );
/*?*/ 		BOOL		 bOk   = FALSE;
/*?*/
/*?*/ 		//	vorneweg testen, ob der Prefix als gueltig erkannt wird
/*?*/ 		//	wenn nicht, nur doppelte vermeiden
/*?*/ 		BOOL bPrefix = ValidTabName( aStrTable );
/*?*/ 		DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
/*?*/ 		USHORT nDummy;
/*?*/
/*?*/ 		USHORT nLoops = 0;		// "zur Sicherheit"
/*?*/ 		for ( USHORT i = nMaxTableNumber+1; !bOk && nLoops <= MAXTAB; i++ )
/*?*/ 		{
/*?*/ 			rName  = aStrTable;
/*?*/ 			rName += String::CreateFromInt32(i);
/*?*/ 			if (bPrefix)
/*?*/ 				bOk = ValidNewTabName( rName );
/*?*/ 			else
/*?*/ 				bOk = !GetTable( rName, nDummy );
/*?*/ 			++nLoops;
/*?*/ 		}
/*?*/
/*?*/ 		DBG_ASSERT(bOk, "kein gueltiger Tabellenname gefunden");
/*?*/ 		if ( !bOk )
/*?*/ 			rName = aStrTable;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// uebergebenen Namen ueberpruefen
/*N*/
/*N*/ 		if ( !ValidNewTabName(rName) )
/*N*/ 		{
/*N*/ 			USHORT i = 1;
/*N*/ 			String aName;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				i++;
/*N*/ 				aName = rName;
/*N*/ 				aName += '_';
/*N*/ 				aName += String::CreateFromInt32(i);
/*N*/ 			}
/*N*/ 			while (!ValidNewTabName(aName) && (i < MAXTAB+1));
/*N*/ 			rName = aName;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScDocument::InsertTab( USHORT nPos, const String& rName,
/*N*/ 			BOOL bExternalDocument )
/*N*/ {
/*N*/ 	USHORT	nTabCount = GetTableCount();
/*N*/ 	BOOL	bValid = (nTabCount <= MAXTAB);
/*N*/ 	if ( !bExternalDocument )	// sonst rName == "'Doc'!Tab", vorher pruefen
/*N*/ 		bValid = (bValid && ValidNewTabName(rName));
/*N*/ 	if (bValid)
/*N*/ 	{
/*N*/ 		if (nPos == SC_TAB_APPEND || nPos == nTabCount)
/*N*/ 		{
/*?*/ 			pTab[nTabCount] = new ScTable(this, nTabCount, rName);
/*?*/ 			++nMaxTableNumber;
/*?*/ 			if ( bExternalDocument )
/*?*/ 				pTab[nTabCount]->SetVisible( FALSE );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (VALIDTAB(nPos) && (nPos < nTabCount))
/*N*/ 			{
/*N*/ 				ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
/*N*/ 				xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
/*N*/ 				xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
/*N*/ 				pRangeName->UpdateTabRef( nPos, 1 );
/*N*/ 				pDBCollection->UpdateReference(
/*N*/ 									URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
/*N*/ 				if (pPivotCollection)
/*N*/ 					pPivotCollection->UpdateReference(
/*N*/ 									URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
/*N*/ 				if (pDPCollection)
/*?*/ 					pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
/*N*/ 				if (pDetOpList)
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,1 );
/*N*/ 				UpdateChartRef( URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
/*N*/ 				UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,1 );
/*N*/ 				if ( pUnoBroadcaster )
/*N*/ 					pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,1 ) );
/*N*/
/*N*/ 				USHORT i;
/*N*/ 				for (i = 0; i <= MAXTAB; i++)
/*N*/ 					if (pTab[i])
/*N*/ 						pTab[i]->UpdateInsertTab(nPos);
/*N*/ 				for (i = nTabCount; i > nPos; i--)
/*N*/ 					pTab[i] = pTab[i - 1];
/*N*/ 				pTab[nPos] = new ScTable(this, nPos, rName);
/*N*/ 				++nMaxTableNumber;
/*N*/ 				for (i = 0; i <= MAXTAB; i++)
/*N*/ 					if (pTab[i])
/*N*/ 						pTab[i]->UpdateCompile();
/*N*/ 				for (i = 0; i <= MAXTAB; i++)
/*N*/ 					if (pTab[i])
/*N*/ 						pTab[i]->StartAllListeners();
/*N*/
/*N*/ 				//	update conditional formats after table is inserted
/*N*/ 				if ( pCondFormList )
/*N*/ 					pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
/*N*/ 				// #81844# sheet names of references are not valid until sheet is inserted
/*N*/ 				if ( pChartListenerCollection )
/*N*/ 					pChartListenerCollection->UpdateScheduledSeriesRanges();
/*N*/
/*N*/ 				SetDirty();
/*N*/ 				bValid = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bValid = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bValid;
/*N*/ }


/*N*/ BOOL ScDocument::DeleteTab( USHORT nTab, ScDocument* pRefUndoDoc )
/*N*/ {
/*N*/ 	BOOL bValid = FALSE;
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 	{
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			USHORT nTabCount = GetTableCount();
/*N*/ 			if (nTabCount > 1)
/*N*/ 			{
/*N*/ 				BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 				SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 				ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
/*N*/ 				DelBroadcastAreasInRange( aRange );
/*N*/
/*N*/ 				aRange.aEnd.SetTab( MAXTAB );
/*N*/ 				xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
/*N*/ 				xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
/*N*/ 				pRangeName->UpdateTabRef( nTab, 2 );
/*N*/ 				pDBCollection->UpdateReference(
/*N*/ 									URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
/*N*/ 				if (pPivotCollection)
/*N*/ 					pPivotCollection->UpdateReference(
/*N*/ 									URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
/*N*/ 				if (pDPCollection)
/*N*/ 					pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
/*N*/ 				if (pDetOpList)
/*N*/ 					pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1 );
/*N*/ 				UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
/*N*/ 				UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,-1 );
/*N*/ 				if ( pCondFormList )
/*N*/ 					pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
/*N*/ 				if ( pUnoBroadcaster )
/*N*/ 					pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1 ) );
/*N*/
/*N*/ 				USHORT i;
/*N*/ 				for (i=0; i<=MAXTAB; i++)
/*N*/ 					if (pTab[i])
/*N*/ 						pTab[i]->UpdateDeleteTab(nTab,FALSE,
/*N*/ 									pRefUndoDoc ? pRefUndoDoc->pTab[i] : 0);
/*N*/ 				delete pTab[nTab];
/*N*/ 				for (i=nTab + 1; i < nTabCount; i++)
/*N*/ 					pTab[i - 1] = pTab[i];
/*N*/ 				pTab[nTabCount - 1] = NULL;
/*N*/ 				--nMaxTableNumber;
/*N*/ 				for (i = 0; i <= MAXTAB; i++)
/*N*/ 					if (pTab[i])
/*N*/ 						pTab[i]->UpdateCompile();
/*N*/ 				// Excel-Filter loescht einige Tables waehrend des Ladens,
/*N*/ 				// Listener werden erst nach dem Laden aufgesetzt
/*N*/ 				if ( !bInsertingFromOtherDoc )
/*N*/ 				{
/*N*/ 					for (i = 0; i <= MAXTAB; i++)
/*N*/ 						if (pTab[i])
/*N*/ 							pTab[i]->StartAllListeners();
/*N*/ 					SetDirty();
/*N*/ 				}
/*N*/ 				// #81844# sheet names of references are not valid until sheet is deleted
/*N*/ 				pChartListenerCollection->UpdateScheduledSeriesRanges();
/*N*/ 				SetAutoCalc( bOldAutoCalc );
/*N*/ 				bValid = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bValid;
/*N*/ }


/*N*/ BOOL ScDocument::RenameTab( USHORT nTab, const String& rName, BOOL bUpdateRef,
/*N*/ 		BOOL bExternalDocument )
/*N*/ {
/*N*/ 	BOOL	bValid = FALSE;
/*N*/ 	USHORT	i;
/*N*/ 	if VALIDTAB(nTab)
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			if ( bExternalDocument )
/*N*/ 				bValid = TRUE;		// zusammengesetzter Name
/*N*/ 			else
/*N*/ 				bValid = ValidTabName(rName);
/*N*/ 			for (i=0; (i<=MAXTAB) && bValid; i++)
/*N*/ 				if (pTab[i] && (i != nTab))
/*N*/ 				{
/*N*/ 					String aOldName;
/*N*/ 					pTab[i]->GetName(aOldName);
/*N*/                     bValid = !ScGlobal::pTransliteration->isEqual( rName, aOldName );
/*N*/ 				}
/*N*/ 			if (bValid)
/*N*/ 			{
/*N*/ 				pTab[nTab]->SetName(rName);
/*N*/ 				if ( pChartListenerCollection )
/*N*/ 					pChartListenerCollection->UpdateSeriesRangesContainingTab( nTab );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	return bValid;
/*N*/ }


/*N*/ void ScDocument::SetVisible( USHORT nTab, BOOL bVisible )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->SetVisible(bVisible);
/*N*/ }


/*N*/ BOOL ScDocument::IsVisible( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->IsVisible();
/*N*/
/*N*/ 	return FALSE;
/*N*/ }

/* ----------------------------------------------------------------------------
    benutzten Bereich suchen:

    GetCellArea	 - nur Daten
    GetTableArea - Daten / Attribute
    GetPrintArea - beruecksichtigt auch Zeichenobjekte,
                    streicht Attribute bis ganz rechts / unten
---------------------------------------------------------------------------- */


/*N*/ BOOL ScDocument::GetCellArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->GetCellArea( rEndCol, rEndRow );
/*N*/
/*N*/ 	rEndCol = 0;
/*N*/ 	rEndRow = 0;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::GetTableArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->GetTableArea( rEndCol, rEndRow );
/*N*/
/*N*/ 	rEndCol = 0;
/*N*/ 	rEndRow = 0;
/*N*/ 	return FALSE;
/*N*/ }


//	zusammenhaengender Bereich

/*N*/ void ScDocument::GetDataArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
/*N*/ 								USHORT& rEndCol, USHORT& rEndRow, BOOL bIncludeOld )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->GetDataArea( rStartCol, rStartRow, rEndCol, rEndRow, bIncludeOld );
/*N*/ }






/*N*/ BOOL ScDocument::CanInsertRow( const ScRange& rRange ) const
/*N*/ {
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nEndTab = rRange.aEnd.Tab();
/*N*/ 	PutInOrder( nStartCol, nEndCol );
/*N*/ 	PutInOrder( nStartRow, nEndRow );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 	USHORT nSize = nEndRow - nStartRow + 1;
/*N*/
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 	for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );
/*N*/
/*N*/ 	return bTest;
/*N*/ }


/*N*/ BOOL ScDocument::InsertRow( USHORT nStartCol, USHORT nStartTab,
/*N*/ 							USHORT nEndCol,   USHORT nEndTab,
/*N*/ 							USHORT nStartRow, USHORT nSize, ScDocument* pRefUndoDoc )
/*N*/ {
/*N*/ 	PutInOrder( nStartCol, nEndCol );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	USHORT i = 0;
/*N*/ 	for (i=nStartTab; i<=nEndTab && bTest; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );
/*N*/ 	if (bTest)
/*N*/ 	{
/*N*/ 		// UpdateBroadcastAreas muss vor UpdateReference gerufen werden, damit nicht
/*N*/ 		// Eintraege verschoben werden, die erst bei UpdateReference neu erzeugt werden
/*N*/
/*N*/ 		UpdateBroadcastAreas( URM_INSDEL, ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( nEndCol, MAXROW, nEndTab )), 0, nSize, 0 );
/*N*/ 		UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
/*N*/ 						 nEndCol, MAXROW, nEndTab,
/*N*/ 						 0, nSize, 0, pRefUndoDoc, FALSE );		// without drawing objects
/*N*/ 		for (i=nStartTab; i<=nEndTab; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );
/*N*/
/*N*/ 		//	#82991# UpdateRef for drawing layer must be after inserting,
/*N*/ 		//	when the new row heights are known.
/*N*/ 		for (i=nStartTab; i<=nEndTab; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->UpdateDrawRef( URM_INSDEL,
/*N*/ 							nStartCol, nStartRow, nStartTab, nEndCol, MAXROW, nEndTab,
/*N*/ 							0, nSize, 0 );
/*N*/
/*N*/ 		if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
/*N*/ 		{	// durch Restaurierung von Referenzen auf geloeschte Bereiche ist
/*N*/ 			// ein neues Listening faellig, bisherige Listener wurden in
/*N*/ 			// FormulaCell UpdateReference abgehaengt
/*N*/ 			StartAllListeners();
/*N*/ 		}
/*N*/ 		else
/*N*/         {   // RelName listeners have been removed in UpdateReference
/*N*/ 			for (i=0; i<=MAXTAB; i++)
/*N*/ 				if (pTab[i])
/*N*/                     pTab[i]->StartNameListeners( TRUE );
/*N*/             // #69592# at least all cells using range names pointing relative
/*N*/             // to the moved range must recalculate
/*N*/ 			for (i=0; i<=MAXTAB; i++)
/*N*/ 				if (pTab[i])
/*N*/ 					pTab[i]->SetRelNameDirty();
/*N*/ 		}
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ 	if ( bRet )
/*N*/ 		pChartListenerCollection->UpdateDirtyCharts();
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL ScDocument::InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc )
/*N*/ {
/*N*/ 	return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
/*N*/ 					  rRange.aEnd.Col(),   rRange.aEnd.Tab(),
/*N*/ 					  rRange.aStart.Row(), rRange.aEnd.Row()-rRange.aStart.Row()+1,
/*N*/ 					  pRefUndoDoc );
/*N*/ }


/*N*/ void ScDocument::DeleteRow( USHORT nStartCol, USHORT nStartTab,
/*N*/ 							USHORT nEndCol,   USHORT nEndTab,
/*N*/ 							USHORT nStartRow, USHORT nSize,
/*N*/ 							ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	PutInOrder( nStartCol, nEndCol );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/
/*N*/ 	if ( nStartRow+nSize <= MAXROW )
/*N*/ 	{
/*N*/ 		DelBroadcastAreasInRange( ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( nEndCol, nStartRow+nSize-1, nEndTab ) ) );
/*N*/ 		UpdateBroadcastAreas( URM_INSDEL, ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow+nSize, nStartTab ),
/*N*/ 			ScAddress( nEndCol, MAXROW, nEndTab )), 0, -(short) nSize, 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DelBroadcastAreasInRange( ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( nEndCol, MAXROW, nEndTab ) ) );
/*N*/
/*N*/ 	if ( nStartRow+nSize <= MAXROW )
/*N*/ 	{
/*N*/ 		UpdateReference( URM_INSDEL, nStartCol, nStartRow+nSize, nStartTab,
/*N*/ 						 nEndCol, MAXROW, nEndTab,
/*N*/ 						 0, -(short) nSize, 0, pRefUndoDoc );
/*N*/ 	}
/*N*/
/*N*/ 	if (pUndoOutline)
/*N*/ 		*pUndoOutline = FALSE;
/*N*/
/*N*/ 	USHORT i=0;
/*N*/ 	for (i=nStartTab; i<=nEndTab; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			pTab[i]->DeleteRow( nStartCol, nEndCol, nStartRow, nSize, pUndoOutline );
/*N*/
/*N*/ 	if ( nStartRow+nSize <= MAXROW )
/*N*/     {   // Name listeners have been removed in UpdateReference
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i])
/*N*/                 pTab[i]->StartNameListeners( FALSE );
/*N*/         // #69592# at least all cells using range names pointing relative to
/*N*/         // the moved range must recalculate
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->SetRelNameDirty();
/*N*/ 	}
/*N*/
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ 	pChartListenerCollection->UpdateDirtyCharts();
/*N*/ }


/*N*/ void ScDocument::DeleteRow( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
/*N*/ 			   rRange.aEnd.Col(),   rRange.aEnd.Tab(),
/*N*/ 			   rRange.aStart.Row(), rRange.aEnd.Row()-rRange.aStart.Row()+1,
/*N*/ 			   pRefUndoDoc, pUndoOutline );
/*N*/ }


/*N*/ BOOL ScDocument::CanInsertCol( const ScRange& rRange ) const
/*N*/ {
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	USHORT nEndTab = rRange.aEnd.Tab();
/*N*/ 	PutInOrder( nStartCol, nEndCol );
/*N*/ 	PutInOrder( nStartRow, nEndRow );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 	USHORT nSize = nEndCol - nStartCol + 1;
/*N*/
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 	for (USHORT i=nStartTab; i<=nEndTab && bTest; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );
/*N*/
/*N*/ 	return bTest;
/*N*/ }


/*N*/ BOOL ScDocument::InsertCol( USHORT nStartRow, USHORT nStartTab,
/*N*/ 							USHORT nEndRow,   USHORT nEndTab,
/*N*/ 							USHORT nStartCol, USHORT nSize, ScDocument* pRefUndoDoc )
/*N*/ {
/*N*/ 	PutInOrder( nStartRow, nEndRow );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	USHORT i=0;
/*N*/ 	for (i=nStartTab; i<=nEndTab && bTest; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );
/*N*/ 	if (bTest)
/*N*/ 	{
/*N*/ 		UpdateBroadcastAreas( URM_INSDEL, ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( MAXCOL, nEndRow, nEndTab )), nSize, 0, 0 );
/*N*/ 		UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
/*N*/ 						 MAXCOL, nEndRow, nEndTab,
/*N*/ 						 nSize, 0, 0, pRefUndoDoc );
/*N*/ 		for (i=nStartTab; i<=nEndTab; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->InsertCol( nStartCol, nStartRow, nEndRow, nSize );
/*N*/
/*N*/ 		if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
/*N*/ 		{	// durch Restaurierung von Referenzen auf geloeschte Bereiche ist
/*N*/ 			// ein neues Listening faellig, bisherige Listener wurden in
/*N*/ 			// FormulaCell UpdateReference abgehaengt
/*N*/ 			StartAllListeners();
/*N*/ 		}
/*N*/ 		else
/*N*/         {   // RelName listeners have been removed in UpdateReference
/*N*/ 			for (i=0; i<=MAXTAB; i++)
/*N*/ 				if (pTab[i])
/*N*/                     pTab[i]->StartNameListeners( TRUE );
/*N*/             // #69592# at least all cells using range names pointing relative
/*N*/             // to the moved range must recalculate
/*N*/ 			for (i=0; i<=MAXTAB; i++)
/*N*/ 				if (pTab[i])
/*N*/ 					pTab[i]->SetRelNameDirty();
/*N*/ 		}
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ 	if ( bRet )
/*N*/ 		pChartListenerCollection->UpdateDirtyCharts();
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL ScDocument::InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc )
/*N*/ {
/*N*/ 	return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
/*N*/ 					  rRange.aEnd.Row(),   rRange.aEnd.Tab(),
/*N*/ 					  rRange.aStart.Col(), rRange.aEnd.Col()-rRange.aStart.Col()+1,
/*N*/ 					  pRefUndoDoc );
/*N*/ }


/*N*/ void ScDocument::DeleteCol(USHORT nStartRow, USHORT nStartTab, USHORT nEndRow, USHORT nEndTab,
/*N*/ 								USHORT nStartCol, USHORT nSize, ScDocument* pRefUndoDoc,
/*N*/ 								BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	PutInOrder( nStartRow, nEndRow );
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/
/*N*/ 	if ( nStartCol+nSize <= MAXCOL )
/*N*/ 	{
/*N*/ 		DelBroadcastAreasInRange( ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( nStartCol+nSize-1, nEndRow, nEndTab ) ) );
/*N*/ 		UpdateBroadcastAreas( URM_INSDEL, ScRange(
/*N*/ 			ScAddress( nStartCol+nSize, nStartRow, nStartTab ),
/*N*/ 			ScAddress( MAXCOL, nEndRow, nEndTab )), -(short) nSize, 0, 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DelBroadcastAreasInRange( ScRange(
/*N*/ 			ScAddress( nStartCol, nStartRow, nStartTab ),
/*N*/ 			ScAddress( MAXCOL, nEndRow, nEndTab ) ) );
/*N*/
/*N*/ 	if ( nStartCol+nSize <= MAXCOL )
/*N*/ 	{
/*N*/ 		UpdateReference( URM_INSDEL, nStartCol+nSize, nStartRow, nStartTab,
/*N*/ 						 MAXCOL, nEndRow, nEndTab,
/*N*/ 						 -(short) nSize, 0, 0, pRefUndoDoc );
/*N*/ 	}
/*N*/
/*N*/ 	if (pUndoOutline)
/*N*/ 		*pUndoOutline = FALSE;
/*N*/
/*N*/ 	USHORT i=0;
/*N*/ 	for (i=nStartTab; i<=nEndTab; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			pTab[i]->DeleteCol( nStartCol, nStartRow, nEndRow, nSize, pUndoOutline );
/*N*/
/*N*/ 	if ( nStartCol+nSize <= MAXCOL )
/*N*/     {   // Name listeners have been removed in UpdateReference
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i])
/*N*/                 pTab[i]->StartNameListeners( FALSE );
/*N*/         // #69592# at least all cells using range names pointing relative to
/*N*/         // the moved range must recalculate
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->SetRelNameDirty();
/*N*/ 	}
/*N*/
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ 	pChartListenerCollection->UpdateDirtyCharts();
/*N*/ }


/*N*/ void ScDocument::DeleteCol( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
/*N*/ 			   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
/*N*/ 			   rRange.aStart.Col(), rRange.aEnd.Col()-rRange.aStart.Col()+1,
/*N*/ 			   pRefUndoDoc, pUndoOutline );
/*N*/ }


//	fuer Area-Links: Zellen einuegen/loeschen, wenn sich der Bereich veraendert
//	(ohne Paint)






/*N*/ BOOL ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 if ( rOld == rNew )
/*N*/ }


/*N*/ void ScDocument::FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (bClear)
/*N*/ }


/*N*/ void ScDocument::DeleteArea(USHORT nCol1, USHORT nRow1,
/*N*/ 							USHORT nCol2, USHORT nRow2,
/*N*/ 							const ScMarkData& rMark, USHORT nDelFlag)
/*N*/ {
/*N*/ 	PutInOrder( nCol1, nCol2 );
/*N*/ 	PutInOrder( nRow1, nRow2 );
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i = 0; i <= MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			if ( rMark.GetTableSelect(i) || bIsUndo )
/*N*/ 				pTab[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScDocument::DeleteAreaTab(USHORT nCol1, USHORT nRow1,
/*N*/ 								USHORT nCol2, USHORT nRow2,
/*N*/ 								USHORT nTab, USHORT nDelFlag)
/*N*/ {
/*N*/ 	PutInOrder( nCol1, nCol2 );
/*N*/ 	PutInOrder( nRow1, nRow2 );
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 	{
/*N*/ 		BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 		SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 		pTab[nTab]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
/*N*/ 		SetAutoCalc( bOldAutoCalc );
/*N*/ 	}
/*N*/ }


/*N*/ void ScDocument::DeleteAreaTab( const ScRange& rRange, USHORT nDelFlag )
/*N*/ {
/*N*/ 	for ( USHORT nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); nTab++ )
/*N*/ 		DeleteAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
/*N*/ 					   rRange.aEnd.Col(),   rRange.aEnd.Row(),
/*N*/ 					   nTab, nDelFlag );
/*N*/ }


/*N*/ void ScDocument::InitUndo( ScDocument* pSrcDoc, USHORT nTab1, USHORT nTab2,
/*N*/ 								BOOL bColInfo, BOOL bRowInfo )
/*N*/ {
/*N*/ 	if (bIsUndo)
/*N*/ 	{
/*N*/ 		Clear();
/*N*/
/*N*/ 		xPoolHelper = pSrcDoc->xPoolHelper;
/*N*/
/*N*/ 		String aString;
/*N*/ 		for (USHORT nTab = nTab1; nTab <= nTab2; nTab++)
/*N*/ 			pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);
/*N*/
/*N*/ 		nMaxTableNumber = nTab2 + 1;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("InitUndo");
/*N*/ }

/*N*/ void ScDocument::CopyToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 							USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 							USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
/*N*/ 							const ScMarkData* pMarks, BOOL bColRowFlags )
/*N*/ {
/*N*/ 	PutInOrder( nCol1, nCol2 );
/*N*/ 	PutInOrder( nRow1, nRow2 );
/*N*/ 	PutInOrder( nTab1, nTab2 );
/*N*/ 	if( !pDestDoc->aDocName.Len() )
/*N*/ 		pDestDoc->aDocName = aDocName;
/*N*/ 	if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
/*N*/ 	{
/*N*/ 		BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
/*N*/ 		pDestDoc->SetAutoCalc( FALSE );		// Mehrfachberechnungen vermeiden
/*N*/ 		for (USHORT i = nTab1; i <= nTab2; i++)
/*N*/ 		{
/*N*/ 			if (pTab[i] && pDestDoc->pTab[i])
/*N*/ 				pTab[i]->CopyToTable( nCol1, nRow1, nCol2, nRow2, nFlags,
/*N*/ 									  bOnlyMarked, pDestDoc->pTab[i], pMarks,
/*N*/ 									  FALSE, bColRowFlags );
/*N*/ 		}
/*N*/ 		pDestDoc->SetAutoCalc( bOldAutoCalc );
/*N*/ 	}
/*N*/ }




/*N*/ void ScDocument::CopyToDocument(const ScRange& rRange,
/*N*/ 							USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
/*N*/ 							const ScMarkData* pMarks, BOOL bColRowFlags)
/*N*/ {
/*N*/ 	ScRange aNewRange = rRange;
/*N*/ 	aNewRange.Justify();
/*N*/
/*N*/ 	if( !pDestDoc->aDocName.Len() )
/*N*/ 		pDestDoc->aDocName = aDocName;
/*N*/ 	BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
/*N*/ 	pDestDoc->SetAutoCalc( FALSE );		// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab(); i++)
/*N*/ 		if (pTab[i] && pDestDoc->pTab[i])
/*N*/ 			pTab[i]->CopyToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
/*N*/ 								 aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
/*N*/ 								 nFlags, bOnlyMarked, pDestDoc->pTab[i],
/*N*/ 								 pMarks, FALSE, bColRowFlags);
/*N*/ 	pDestDoc->SetAutoCalc( bOldAutoCalc );
/*N*/ }




/*N*/ void ScDocument::CopyToClip(USHORT nCol1, USHORT nRow1,
/*N*/ 							USHORT nCol2, USHORT nRow2,
/*N*/ 							BOOL bCut, ScDocument* pClipDoc,
/*N*/ 							BOOL bAllTabs, const ScMarkData* pMarks,
/*N*/ 							BOOL bKeepScenarioFlags, BOOL bIncludeObjects)
/*N*/ {
/*N*/ 	DBG_ASSERT( bAllTabs || pMarks, "CopyToClip: ScMarkData fehlt" );
/*N*/
/*N*/ 	if (!bIsClip)
/*N*/ 	{
/*N*/ 		PutInOrder( nCol1, nCol2 );
/*N*/ 		PutInOrder( nRow1, nRow2 );
/*N*/ 		if (!pClipDoc)
/*N*/ 		{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			DBG_ERROR("CopyToClip: no ClipDoc");
/*N*/ 		}
/*N*/
/*N*/ 		pClipDoc->aDocName = aDocName;
/*N*/ 		pClipDoc->aClipRange = ScRange( nCol1,nRow1,0, nCol2,nRow2,0 );
/*N*/ 		pClipDoc->ResetClip( this, pMarks );
/*N*/ 		USHORT i, j;
/*N*/ 		pClipDoc->pRangeName->FreeAll();
/*N*/ 		for (i = 0; i < pRangeName->GetCount(); i++)		//! DB-Bereiche Pivot-Bereiche auch !!!
/*N*/ 		{
/*N*/ 			USHORT nIndex = ((ScRangeData*)((*pRangeName)[i]))->GetIndex();
/*N*/ 			BOOL bInUse = FALSE;
/*N*/ 			for (j = 0; !bInUse && (j <= MAXTAB); j++)
/*N*/ 			{
/*N*/ 				if (pTab[j])
/*N*/ 					bInUse = pTab[j]->IsRangeNameInUse(nCol1, nRow1, nCol2, nRow2,
/*N*/ 													   nIndex);
/*N*/ 			}
/*N*/ 			if (bInUse)
/*N*/ 			{
/*N*/ 				ScRangeData* pData = new ScRangeData(*((*pRangeName)[i]));
/*N*/ 				if (!pClipDoc->pRangeName->Insert(pData))
/*N*/ 					delete pData;
/*N*/ 				else
/*N*/ 					pData->SetIndex(nIndex);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		for (i = 0; i <= MAXTAB; i++)
/*N*/ 			if (pTab[i] && pClipDoc->pTab[i])
/*N*/ 				if ( bAllTabs || !pMarks || pMarks->GetTableSelect(i) )
/*N*/ 				{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 					pTab[i]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[i], bKeepScenarioFlags);
/*N*/
/*N*/ 					if ( pDrawLayer && bIncludeObjects )
/*N*/ 					{
/*N*/ 						//	also copy drawing objects
/*N*/
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 						Rectangle aObjRect = GetMMRect( nCol1, nRow1, nCol2, nRow2, i );
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 		pClipDoc->bCutMode = bCut;
/*N*/ 	}
/*N*/ }

void ScDocument::StartListeningFromClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, const ScMarkData& rMark, USHORT nInsFlag )
{
}


/*N*/ void ScDocument::BroadcastFromClip( USHORT nCol1, USHORT nRow1,
/*N*/ 								USHORT nCol2, USHORT nRow2,
/*N*/ 									const ScMarkData& rMark, USHORT nInsFlag )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (nInsFlag & IDF_CONTENTS)
/*N*/ }


/*N*/ void ScDocument::CopyBlockFromClip( USHORT nCol1, USHORT nRow1,
/*N*/ 									USHORT nCol2, USHORT nRow2,
/*N*/ 									const ScMarkData& rMark,
/*N*/ 									short nDx, short nDy,
/*N*/ 									const ScCopyBlockFromClipParams* pCBFCP )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	ScTable** ppClipTab = pCBFCP->pClipDoc->pTab;
/*N*/ }


/*N*/ void ScDocument::CopyNonFilteredFromClip( USHORT nCol1, USHORT nRow1,
/*N*/ 									USHORT nCol2, USHORT nRow2,
/*N*/ 									const ScMarkData& rMark,
/*N*/ 									short nDx, short nDy,
/*N*/ 									const ScCopyBlockFromClipParams* pCBFCP )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	//	call CopyBlockFromClip for ranges of consecutive non-filtered rows
/*N*/ }


/*N*/ void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
/*N*/ 								USHORT nInsFlag,
/*N*/ 								ScDocument* pRefUndoDoc, ScDocument* pClipDoc, BOOL bResetCut,
/*N*/ 								BOOL bAsLink, BOOL bIncludeFiltered, BOOL bSkipAttrForEmpty )
/*N*/ {
/*N*/ 	if (!bIsClip && pClipDoc)
/*N*/ 	{
/*N*/ 		if (pClipDoc->bIsClip && pClipDoc->GetTableCount())
/*N*/ 		{
/*N*/ 			BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 			SetAutoCalc( FALSE );	// avoid multiple recalculations
/*N*/
/*N*/ 			SvNumberFormatter* pThisFormatter = xPoolHelper->GetFormTable();
/*N*/ 			SvNumberFormatter* pOtherFormatter = pClipDoc->xPoolHelper->GetFormTable();
/*N*/ 			if (pOtherFormatter && pOtherFormatter != pThisFormatter)
/*N*/ 			{
/*N*/ 				SvNumberFormatterIndexTable* pExchangeList =
/*N*/ 						 pThisFormatter->MergeFormatter(*(pOtherFormatter));
/*N*/ 				if (pExchangeList->Count() > 0)
/*N*/ 					pFormatExchangeList = pExchangeList;
/*N*/ 			}
/*N*/
/*N*/ 			USHORT nClipRangeNames = pClipDoc->pRangeName->GetCount();
/*N*/ 			// array containing range names which might need update of indices
/*N*/ 			ScRangeData** pClipRangeNames = nClipRangeNames ? new ScRangeData* [nClipRangeNames] : NULL;
/*N*/ 			// the index mapping thereof
/*N*/ 			ScIndexMap aClipRangeMap( nClipRangeNames );
/*N*/ 			BOOL bRangeNameReplace = FALSE;
/*N*/
/*N*/ 			USHORT i, k;
/*N*/ 			for (i = 0; i < nClipRangeNames; i++)		//! DB-Bereiche Pivot-Bereiche auch
/*N*/ 			{
                /*	Copy only if the name doesn't exist in this document.
                 If it exists we use the already existing name instead,
                    another possibility could be to create new names if
                    documents differ.
                    A proper solution would ask the user how to proceed.
                    The adjustment of the indices in the formulas is done later.
                */
/*N*/ 				ScRangeData* pClipData = (*pClipDoc->pRangeName)[i];
/*N*/ 				if ( pRangeName->SearchName( pClipData->GetName(), k ) )
/*N*/ 				{
/*N*/ 					pClipRangeNames[i] = NULL;	// range name not inserted
/*N*/ 					USHORT nOldIndex = pClipData->GetIndex();
/*N*/ 					USHORT nNewIndex = ((*pRangeName)[k])->GetIndex();
/*N*/ 					aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
/*N*/ 					if ( !bRangeNameReplace )
/*N*/ 						bRangeNameReplace = ( nOldIndex != nNewIndex );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					ScRangeData* pData = new ScRangeData( *pClipData );
/*N*/ 					pData->SetDocument(this);
/*N*/ 					if ( pRangeName->FindIndex( pData->GetIndex() ) )
/*N*/ 						pData->SetIndex(0);		// need new index, done in Insert
/*N*/ 					if ( pRangeName->Insert( pData ) )
/*N*/ 					{
/*N*/ 						pClipRangeNames[i] = pData;
/*N*/ 						USHORT nOldIndex = pClipData->GetIndex();
/*N*/ 						USHORT nNewIndex = pData->GetIndex();
/*N*/ 						aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
/*N*/ 						if ( !bRangeNameReplace )
/*N*/ 							bRangeNameReplace = ( nOldIndex != nNewIndex );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{	// must be an overflow
/*N*/ 						delete pData;
/*N*/ 						pClipRangeNames[i] = NULL;
/*N*/ 						aClipRangeMap.SetPair( i, pClipData->GetIndex(), 0 );
/*N*/ 						bRangeNameReplace = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			USHORT nCol1 = rDestRange.aStart.Col();
/*N*/ 			USHORT nRow1 = rDestRange.aStart.Row();
/*N*/ 			USHORT nCol2 = rDestRange.aEnd.Col();
/*N*/ 			USHORT nRow2 = rDestRange.aEnd.Row();
/*N*/
/*N*/ 			USHORT nXw = pClipDoc->aClipRange.aEnd.Col();
/*N*/ 			USHORT nYw = pClipDoc->aClipRange.aEnd.Row();
/*N*/ 			pClipDoc->ExtendMerge( pClipDoc->aClipRange.aStart.Col(),
/*N*/ 									pClipDoc->aClipRange.aStart.Row(),
/*N*/ 									nXw, nYw, 0 );
/*N*/ 			nXw -= pClipDoc->aClipRange.aEnd.Col();
/*N*/ 			nYw -= pClipDoc->aClipRange.aEnd.Row();			// only extra value from ExtendMerge
/*N*/ 			USHORT nDestAddX, nDestAddY;
/*N*/ 			pClipDoc->GetClipArea( nDestAddX, nDestAddY, bIncludeFiltered );
/*N*/ 			nXw += nDestAddX;
/*N*/ 			nYw += nDestAddY;								// ClipArea, plus ExtendMerge value
/*N*/
/*N*/ 			//	Inhalte entweder komplett oder gar nicht loeschen:
/*N*/ 			USHORT nDelFlag = IDF_NONE;
/*N*/ 			if ( nInsFlag & IDF_CONTENTS )
/*N*/ 				nDelFlag |= IDF_CONTENTS;
/*N*/ 			//	With bSkipAttrForEmpty, don't remove attributes, copy
/*N*/ 			//	on top of existing attributes instead.
/*N*/ 			if ( ( nInsFlag & IDF_ATTRIB ) && !bSkipAttrForEmpty )
/*N*/ 				nDelFlag |= IDF_ATTRIB;
/*N*/ 			DeleteArea(nCol1, nRow1, nCol2, nRow2, rMark, nDelFlag);
/*N*/
/*N*/ 			bInsertingFromOtherDoc = TRUE;	// kein Broadcast/Listener aufbauen bei Insert
/*N*/ 			USHORT nC1 = nCol1;
/*N*/ 			USHORT nR1 = nRow1;
/*N*/ 			USHORT nC2 = nC1 + nXw;
/*N*/ 			USHORT nR2 = nR1 + nYw;
/*N*/ 			USHORT nClipStartCol = pClipDoc->aClipRange.aStart.Col();
/*N*/ 			USHORT nClipStartRow = pClipDoc->aClipRange.aStart.Row();
/*N*/
/*N*/ 			ScCopyBlockFromClipParams aCBFCP;
/*N*/ 			aCBFCP.pRefUndoDoc = pRefUndoDoc;
/*N*/ 			aCBFCP.pClipDoc = pClipDoc;
/*N*/ 			aCBFCP.nInsFlag = nInsFlag;
/*N*/ 			aCBFCP.bAsLink	= bAsLink;
/*N*/ 			aCBFCP.bSkipAttrForEmpty = bSkipAttrForEmpty;
/*N*/ 			aCBFCP.nTabStart = MAXTAB;		// wird in der Schleife angepasst
/*N*/ 			aCBFCP.nTabEnd = 0;				// wird in der Schleife angepasst
/*N*/
/*N*/ 			//	Inc/DecRecalcLevel einmal aussen, damit nicht fuer jeden Block
/*N*/ 			//	die Draw-Seitengroesse neu berechnet werden muss
/*N*/ 			//!	nur wenn ganze Zeilen/Spalten kopiert werden?
/*N*/
/*N*/ 			for (i = 0; i <= MAXTAB; i++)
/*N*/ 				if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 				{
/*N*/ 					if ( i < aCBFCP.nTabStart )
/*N*/ 						aCBFCP.nTabStart = i;
/*N*/ 					aCBFCP.nTabEnd = i;
/*N*/ 					pTab[i]->IncRecalcLevel();
/*N*/ 				}
/*N*/
/*N*/ 			// bei mindestens 64 Zeilen wird in ScColumn::CopyFromClip voralloziert
/*N*/ 			BOOL bDoDouble = ( nYw < 64 && nRow2 - nRow1 > 64);
/*N*/ 			BOOL bOldDouble = ScColumn::bDoubleAlloc;
/*N*/ 			if (bDoDouble)
/*N*/ 				ScColumn::bDoubleAlloc = TRUE;
/*N*/
/*N*/ 			do
/*N*/ 			{
/*N*/ 				do
/*N*/ 				{
/*N*/ 					short nDx = ((short)nC1) - nClipStartCol;
/*N*/ 					short nDy = ((short)nR1) - nClipStartRow;
/*N*/ 					if ( bIncludeFiltered )
/*N*/ 						CopyBlockFromClip( nC1, nR1, nC2, nR2, rMark, nDx, nDy, &aCBFCP );
/*N*/ 					else
/*N*/ 						CopyNonFilteredFromClip( nC1, nR1, nC2, nR2, rMark, nDx, nDy, &aCBFCP );
/*N*/ 					nC1 = nC2 + 1;
/*N*/ 					nC2 = Min((USHORT)(nC1 + nXw), nCol2);
/*N*/ 				}
/*N*/ 				while (nC1 <= nCol2);
/*N*/ 				nC1 = nCol1;
/*N*/ 				nC2 = nC1 + nXw;
/*N*/ 				nR1 = nR2 + 1;
/*N*/ 				nR2 = Min((USHORT)(nR1 + nYw), nRow2);
/*N*/ 			}
/*N*/ 			while (nR1 <= nRow2);
/*N*/
/*N*/ 			ScColumn::bDoubleAlloc = bOldDouble;
/*N*/
/*N*/ 			for (i = 0; i <= MAXTAB; i++)
/*N*/ 				if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 					pTab[i]->DecRecalcLevel();
/*N*/
/*N*/ 			bInsertingFromOtherDoc = FALSE;
/*N*/ 			pFormatExchangeList = NULL;
/*N*/ 			if ( bRangeNameReplace )
/*N*/ 			{
/*N*/ 				// first update all inserted named formulas if they contain other
/*N*/ 				// range names and used indices changed
/*N*/ 				for (i = 0; i < nClipRangeNames; i++)		//! DB-Bereiche Pivot-Bereiche auch
/*N*/ 				{
/*N*/ 					if ( pClipRangeNames[i] )
/*N*/ 						pClipRangeNames[i]->ReplaceRangeNamesInUse( aClipRangeMap );
/*N*/ 				}
/*N*/ 				// then update the formulas, they might need the just updated range names
/*N*/ 				USHORT nC1 = nCol1;
/*N*/ 				USHORT nR1 = nRow1;
/*N*/ 				USHORT nC2 = nC1 + nXw;
/*N*/ 				USHORT nR2 = nR1 + nYw;
/*N*/ 				do
/*N*/ 				{
/*N*/ 					do
/*N*/ 					{
/*N*/ 						for (k = 0; k <= MAXTAB; k++)
/*N*/ 						{
/*N*/ 							if ( pTab[k] && rMark.GetTableSelect(k) )
/*N*/ 								pTab[k]->ReplaceRangeNamesInUse(nC1, nR1,
/*N*/ 									nC2, nR2, aClipRangeMap );
/*N*/ 						}
/*N*/ 						nC1 = nC2 + 1;
/*N*/ 						nC2 = Min((USHORT)(nC1 + nXw), nCol2);
/*N*/ 					} while (nC1 <= nCol2);
/*N*/ 					nC1 = nCol1;
/*N*/ 					nC2 = nC1 + nXw;
/*N*/ 					nR1 = nR2 + 1;
/*N*/ 					nR2 = Min((USHORT)(nR1 + nYw), nRow2);
/*N*/ 				} while (nR1 <= nRow2);
/*N*/ 			}
/*N*/ 			if ( pClipRangeNames )
/*N*/ 				delete [] pClipRangeNames;
/*N*/ 			// Listener aufbauen nachdem alles inserted wurde
/*N*/ 			StartListeningFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
/*N*/ 			// nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
/*N*/ 			BroadcastFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
/*N*/ 			if (bResetCut)
/*N*/ 				pClipDoc->bCutMode = FALSE;
/*N*/ 			SetAutoCalc( bOldAutoCalc );
/*N*/ 		}
/*N*/ 	}
/*N*/ }




/*N*/ void ScDocument::GetClipArea(USHORT& nClipX, USHORT& nClipY, BOOL bIncludeFiltered)
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (bIsClip)
/*N*/ }












/*N*/ void ScDocument::PutCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell, BOOL bForceTab )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 	{
/*N*/ 		if ( bForceTab && !pTab[nTab] )
/*N*/ 		{
/*N*/ 			BOOL bExtras = !bIsUndo;		// Spaltenbreiten, Zeilenhoehen, Flags
/*N*/
/*N*/ 			pTab[nTab] = new ScTable(this, nTab,
/*N*/ 							String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
/*N*/ 							bExtras, bExtras);
/*N*/ 			++nMaxTableNumber;
/*N*/ 		}
/*N*/
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->PutCell( nCol, nRow, pCell );
/*N*/ 	}
/*N*/ }


/*N*/ void ScDocument::PutCell( const ScAddress& rPos, ScBaseCell* pCell, BOOL bForceTab )
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( bForceTab && !pTab[nTab] )
/*N*/ 	{
/*?*/ 		BOOL bExtras = !bIsUndo;		// Spaltenbreiten, Zeilenhoehen, Flags
/*?*/
/*?*/ 		pTab[nTab] = new ScTable(this, nTab,
/*?*/ 						String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
/*?*/ 						bExtras, bExtras);
/*?*/ 		++nMaxTableNumber;
/*N*/ 	}
/*N*/
/*N*/ 	if (pTab[nTab])
/*N*/ 		pTab[nTab]->PutCell( rPos, pCell );
/*N*/ }


/*N*/ BOOL ScDocument::SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->SetString( nCol, nRow, nTab, rString );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ void ScDocument::SetValue( USHORT nCol, USHORT nRow, USHORT nTab, const double& rVal )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->SetValue( nCol, nRow, rVal );
/*N*/ }


/*N*/ void ScDocument::SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->SetNote( nCol, nRow, rNote );
/*N*/ }


/*N*/ void ScDocument::GetString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString )
/*N*/ {
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 		pTab[nTab]->GetString( nCol, nRow, rString );
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }


/*N*/ void ScDocument::GetInputString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString )
/*N*/ {
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 		pTab[nTab]->GetInputString( nCol, nRow, rString );
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }


/*N*/ void ScDocument::GetValue( USHORT nCol, USHORT nRow, USHORT nTab, double& rValue )
/*N*/ {
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 		rValue = pTab[nTab]->GetValue( nCol, nRow );
/*N*/ 	else
/*N*/ 		rValue = 0.0;
/*N*/ }


/*N*/ double ScDocument::GetValue( const ScAddress& rPos )
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetValue( rPos );
/*N*/ 	return 0.0;
/*N*/ }


/*N*/ void ScDocument::GetNumberFormat( USHORT nCol, USHORT nRow, USHORT nTab,
/*N*/ 								  sal_uInt32& rFormat )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			rFormat = pTab[nTab]->GetNumberFormat( nCol, nRow );
/*N*/ 			return ;
/*N*/ 		}
/*N*/ 	rFormat = 0;
/*N*/ }


/*N*/ ULONG ScDocument::GetNumberFormat( const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetNumberFormat( rPos );
/*N*/ 	return 0;
/*N*/ }


/*N*/ void ScDocument::GetNumberFormatInfo( short& nType, ULONG& nIndex,
/*N*/ 			const ScAddress& rPos, const ScFormulaCell& rFCell ) const
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 	{
/*N*/ 		nIndex = pTab[nTab]->GetNumberFormat( rPos );
/*N*/ 		if ( (nIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*N*/ 			rFCell.GetFormatInfo( nType, nIndex );
/*N*/ 		else
/*N*/ 			nType = GetFormatTable()->GetType( nIndex );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nType = NUMBERFORMAT_UNDEFINED;
/*N*/ 		nIndex = 0;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScDocument::GetNote( USHORT nCol, USHORT nRow, USHORT nTab, ScPostIt& rNote )
/*N*/ {
/*N*/ 	BOOL bHasNote = FALSE;
/*N*/
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 		bHasNote = pTab[nTab]->GetNote( nCol, nRow, rNote );
/*N*/ 	else
/*?*/ 		rNote.Clear();
/*N*/
/*N*/ 	return bHasNote;
/*N*/ }


/*N*/ CellType ScDocument::GetCellType( const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetCellType( rPos );
/*N*/ 	return CELLTYPE_NONE;
/*N*/ }


/*N*/ void ScDocument::GetCellType( USHORT nCol, USHORT nRow, USHORT nTab,
/*N*/ 		CellType& rCellType ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		rCellType = pTab[nTab]->GetCellType( nCol, nRow );
/*N*/ 	else
/*N*/ 		rCellType = CELLTYPE_NONE;
/*N*/ }


/*N*/ void ScDocument::GetCell( USHORT nCol, USHORT nRow, USHORT nTab,
/*N*/ 		ScBaseCell*& rpCell ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		rpCell = pTab[nTab]->GetCell( nCol, nRow );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR("GetCell ohne Tabelle");
/*N*/ 		rpCell = NULL;
/*N*/ 	}
/*N*/ }


/*N*/ ScBaseCell* ScDocument::GetCell( const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetCell( rPos );
/*N*/
/*N*/ 	DBG_ERROR("GetCell ohne Tabelle");
/*N*/ 	return NULL;
/*N*/ }


/*N*/ BOOL ScDocument::HasStringData( USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 			return pTab[nTab]->HasStringData( nCol, nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::HasValueData( USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( VALIDTAB(nTab) && pTab[nTab] )
/*N*/ 			return pTab[nTab]->HasValueData( nCol, nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }






/*N*/ void ScDocument::SetDirty()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	bAutoCalc = FALSE;		// keine Mehrfachberechnung
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->SetDirty();
/*N*/
/*N*/ 	//	Charts werden zwar auch ohne AutoCalc im Tracking auf Dirty gesetzt,
/*N*/ 	//	wenn alle Formeln dirty sind, werden die Charts aber nicht mehr erwischt
/*N*/ 	//	(#45205#) - darum alle Charts nochmal explizit
/*N*/ 	if (pChartListenerCollection)
/*N*/ 		pChartListenerCollection->SetDirty();
/*N*/
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScDocument::SetDirty( const ScRange& rRange )
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	bAutoCalc = FALSE;		// keine Mehrfachberechnung
/*N*/ 	USHORT nTab2 = rRange.aEnd.Tab();
/*N*/ 	for (USHORT i=rRange.aStart.Tab(); i<=nTab2; i++)
/*N*/ 		if (pTab[i]) pTab[i]->SetDirty( rRange );
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/  void ScDocument::SetTableOpDirty( const ScRange& rRange )
/*N*/  {
/*N*/  	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/  	bAutoCalc = FALSE;		// no multiple recalculation
/*N*/  	USHORT nTab2 = rRange.aEnd.Tab();
/*N*/  	for (USHORT i=rRange.aStart.Tab(); i<=nTab2; i++)
/*N*/  		if (pTab[i]) pTab[i]->SetTableOpDirty( rRange );
/*N*/  	SetAutoCalc( bOldAutoCalc );
/*N*/  }


/*N*/  void ScDocument::AddTableOpFormulaCell( ScFormulaCell* pCell )
/*N*/  {
/*N*/      ScInterpreterTableOpParams* p = aTableOpList.Last();
/*N*/      if ( p && p->bCollectNotifications )
/*N*/      {
/*N*/          if ( p->bRefresh )
/*N*/          {   // refresh pointers only
/*N*/              p->aNotifiedFormulaCells.push_back( pCell );
/*N*/          }
/*N*/          else
/*N*/          {   // init both, address and pointer
/*N*/              p->aNotifiedFormulaCells.push_back( pCell );
/*N*/              p->aNotifiedFormulaPos.push_back( pCell->aPos );
/*N*/          }
/*N*/      }
/*N*/  }


/*N*/ void ScDocument::CalcAll()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( TRUE );
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->SetDirtyVar();
/*N*/ 	for (i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->CalcAll();
/*N*/ 	ClearFormulaTree();
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ }




/*N*/ void ScDocument::CompileXML()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = GetAutoCalc();
/*N*/ 	SetAutoCalc( FALSE );
/*N*/     ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(
/*N*/                 STR_PROGRESS_CALCULATING ), GetXMLImportedFormulaCount() );
/*N*/
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->CompileXML( aProgress );
/*N*/
/*N*/ 	if ( pCondFormList )
/*N*/ 		pCondFormList->CompileXML();
/*N*/ 	if ( pValidationList )
/*N*/ 		pValidationList->CompileXML();
/*N*/
/*N*/ 	SetDirty();
/*N*/ 	SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScDocument::CalcAfterLoad()
/*N*/ {
/*N*/ 	if (bIsClip)	// Excel-Dateien werden aus dem Clipboard in ein Clip-Doc geladen
/*N*/ 		return;		// dann wird erst beim Einfuegen in das richtige Doc berechnet
/*N*/
/*N*/ 	bCalcingAfterLoad = TRUE;
/*N*/ 	USHORT i=0;
/*N*/ 	for (i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->CalcAfterLoad();
/*N*/ 	for (i=0; i<=MAXTAB; i++)
/*N*/ 		if (pTab[i]) pTab[i]->SetDirtyAfterLoad();
/*N*/ 	bCalcingAfterLoad = FALSE;
/*N*/
/*N*/ 	SetDetectiveDirty(FALSE);	// noch keine wirklichen Aenderungen
/*N*/ }


/*N*/ USHORT ScDocument::GetErrCode( const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	USHORT nTab = rPos.Tab();
/*N*/ 	if ( pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetErrCode( rPos );
/*N*/ 	return 0;
/*N*/ }


/*N*/ void ScDocument::ResetChanged( const ScRange& rRange )
/*N*/ {
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndTab = rRange.aEnd.Tab();
/*N*/ 	for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->ResetChanged( rRange );
/*N*/ }

//
//	Spaltenbreiten / Zeilenhoehen	--------------------------------------
//


/*N*/ void ScDocument::SetColWidth( USHORT nCol, USHORT nTab, USHORT nNewWidth )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetColWidth( nCol, nNewWidth );
/*N*/ }




/*N*/ void ScDocument::SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nNewHeight )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetRowHeightRange
/*N*/ 			( nStartRow, nEndRow, nNewHeight, 1.0, 1.0 );
/*N*/ }


/*N*/ void ScDocument::SetManualHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bManual )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetManualHeight( nStartRow, nEndRow, bManual );
/*N*/ }


/*N*/ USHORT ScDocument::GetColWidth( USHORT nCol, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetColWidth( nCol );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }


/*N*/ USHORT ScDocument::GetOriginalWidth( USHORT nCol, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetOriginalWidth( nCol );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }




/*N*/ USHORT ScDocument::GetOriginalHeight( USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetOriginalHeight( nRow );
/*N*/ 	DBG_ERROR("Wrong table number");
/*N*/ 	return 0;
/*N*/ }


/*N*/ USHORT ScDocument::GetRowHeight( USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetRowHeight( nRow );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }


/*N*/ USHORT ScDocument::GetHiddenRowCount( USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetHiddenRowCount( nRow );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }






/*N*/ USHORT ScDocument::GetOptimalColWidth( USHORT nCol, USHORT nTab, OutputDevice* pDev,
/*N*/ 										double nPPTX, double nPPTY,
/*N*/ 										const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 										BOOL bFormula, const ScMarkData* pMarkData,
/*N*/ 										BOOL bSimpleTextImport )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetOptimalColWidth( nCol, pDev, nPPTX, nPPTY,
/*N*/ 			rZoomX, rZoomY, bFormula, pMarkData, bSimpleTextImport );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }




/*N*/ BOOL ScDocument::SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nExtra,
/*N*/ 									OutputDevice* pDev,
/*N*/ 									double nPPTX, double nPPTY,
/*N*/ 									const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 									BOOL bShrink )
/*N*/ {
/*N*/ //!	MarkToMulti();
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->SetOptimalHeight( nStartRow, nEndRow, nExtra,
/*N*/ 												pDev, nPPTX, nPPTY, rZoomX, rZoomY, bShrink );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return FALSE;
/*N*/ }


//
//	Spalten-/Zeilen-Flags	----------------------------------------------
//

/*N*/ void ScDocument::ShowCol(USHORT nCol, USHORT nTab, BOOL bShow)
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->ShowCol( nCol, bShow );
/*N*/ }


/*N*/ void ScDocument::ShowRows(USHORT nRow1, USHORT nRow2, USHORT nTab, BOOL bShow)
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->ShowRows( nRow1, nRow2, bShow );
/*N*/ }




/*N*/ void ScDocument::SetRowFlags( USHORT nRow, USHORT nTab, BYTE nNewFlags )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetRowFlags( nRow, nNewFlags );
/*N*/ }


/*N*/ BYTE ScDocument::GetColFlags( USHORT nCol, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetColFlags( nCol );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }

/*N*/ BYTE ScDocument::GetRowFlags( USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetRowFlags( nRow );
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return 0;
/*N*/ }





/*N*/ USHORT ScDocument::GetLastChangedCol( USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/         return pTab[nTab]->GetLastChangedCol();
/*N*/ 	return 0;
/*N*/ }

/*N*/ USHORT ScDocument::GetLastChangedRow( USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/         return pTab[nTab]->GetLastChangedRow();
/*N*/ 	return 0;
/*N*/ }


/*N*/ USHORT ScDocument::GetNextDifferentChangedCol( USHORT nTab, USHORT nStart) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 	{
/*N*/ 		BYTE nStartFlags = pTab[nTab]->GetColFlags(nStart);
/*N*/ 		USHORT nStartWidth = pTab[nTab]->GetOriginalWidth(nStart);
/*N*/ 		for (USHORT nCol = nStart + 1; nCol <= MAXCOL; nCol++)
/*N*/ 		{
/*N*/ 			if (((nStartFlags & CR_MANUALBREAK) != (pTab[nTab]->GetColFlags(nCol) & CR_MANUALBREAK)) ||
/*N*/ 				(nStartWidth != pTab[nTab]->GetOriginalWidth(nCol)) ||
/*N*/ 				((nStartFlags & CR_HIDDEN) != (pTab[nTab]->GetColFlags(nCol) & CR_HIDDEN)) )
/*N*/ 				return nCol;
/*N*/ 		}
/*N*/ 		return MAXCOL;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ USHORT ScDocument::GetNextDifferentChangedRow( USHORT nTab, USHORT nStart, bool bCareManualSize) const
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 	{
/*N*/ 		BYTE nStartFlags = pTab[nTab]->GetRowFlags(nStart);
/*N*/ 		USHORT nStartHeight = pTab[nTab]->GetOriginalHeight(nStart);
/*N*/ 		for (USHORT nRow = nStart + 1; nRow <= MAXROW; nRow++)
/*N*/ 		{
/*N*/ 			if (((nStartFlags & CR_MANUALBREAK) != (pTab[nTab]->GetRowFlags(nRow) & CR_MANUALBREAK)) ||
/*N*/ 				((nStartFlags & CR_MANUALSIZE) != (pTab[nTab]->GetRowFlags(nRow) & CR_MANUALSIZE)) ||
/*N*/ 				(bCareManualSize && (nStartFlags & CR_MANUALSIZE) && (nStartHeight != pTab[nTab]->GetOriginalHeight(nRow))) ||
/*N*/                 (!bCareManualSize && ((nStartHeight != pTab[nTab]->GetOriginalHeight(nRow)))))
/*N*/ 				return nRow;
/*N*/ 		}
/*N*/ 		return MAXROW;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ BOOL ScDocument::GetColDefault( USHORT nTab, USHORT nCol, USHORT nLastRow, USHORT& nDefault)
/*N*/ {
/*N*/ 	BOOL bRet(FALSE);
/*N*/ 	nDefault = 0;
/*N*/ 	ScDocAttrIterator aDocAttrItr(this, nTab, nCol, 0, nCol, nLastRow);
/*N*/ 	USHORT nColumn, nStartRow, nEndRow;
/*N*/ 	const ScPatternAttr* pAttr = aDocAttrItr.GetNext(nColumn, nStartRow, nEndRow);
/*N*/ 	if (nEndRow < nLastRow)
/*N*/ 	{
/*N*/ 		ScDefaultAttrSet aSet;
/*N*/ 		ScDefaultAttrSet::iterator aItr = aSet.end();
/*N*/ 		while (pAttr)
/*N*/ 		{
/*N*/ 			ScDefaultAttr aAttr(pAttr);
/*N*/ 			aItr = aSet.find(aAttr);
/*N*/ 			if (aItr == aSet.end())
/*N*/ 			{
/*N*/ 				aAttr.nCount = nEndRow - nStartRow + 1;
/*N*/ 				aAttr.nFirst = nStartRow;
/*N*/ 				aSet.insert(aAttr);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aAttr.nCount = aItr->nCount + nEndRow - nStartRow + 1;
/*N*/ 				aAttr.nFirst = aItr->nFirst;
/*N*/ 				aSet.erase(aItr);
/*N*/ 				aSet.insert(aAttr);
/*N*/ 			}
/*N*/ 			pAttr = aDocAttrItr.GetNext(nColumn, nStartRow, nEndRow);
/*N*/ 		}
/*N*/ 		ScDefaultAttrSet::iterator aDefaultItr = aSet.begin();
/*N*/ 		aItr = aDefaultItr;
/*N*/ 		aItr++;
/*N*/ 		while (aItr != aSet.end())
/*N*/ 		{
/*N*/ 			if (aItr->nCount > aDefaultItr->nCount)
/*N*/ 				aDefaultItr = aItr;
/*N*/ 			aItr++;
/*N*/ 		}
/*N*/ 		nDefault = aDefaultItr->nFirst;
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bRet = TRUE;
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL ScDocument::GetRowDefault( USHORT nTab, USHORT nRow, USHORT nLastCol, USHORT& nDefault)
/*N*/ {
/*N*/ 	BOOL bRet(FALSE);
/*N*/ 	return bRet;
/*N*/ }

//
//	Attribute	----------------------------------------------------------
//

/*N*/ const SfxPoolItem* ScDocument::GetAttr( USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pTemp = pTab[nTab]->GetAttr( nCol, nRow, nWhich );
/*N*/ 		if (pTemp)
/*N*/ 			return pTemp;
/*N*/ 		else
/*N*/ 			DBG_ERROR( "Attribut Null" );
/*N*/ 	}
/*N*/ 	return &xPoolHelper->GetDocPool()->GetDefaultItem( nWhich );
/*N*/ }


/*N*/ const ScPatternAttr* ScDocument::GetPattern( USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetPattern( nCol, nRow );
/*N*/ 	return NULL;
/*N*/ }


/*N*/ void ScDocument::ApplyAttr( USHORT nCol, USHORT nRow, USHORT nTab, const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->ApplyAttr( nCol, nRow, rAttr );
/*N*/ }




/*N*/ void ScDocument::ApplyPatternArea( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow,
/*N*/ 						const ScMarkData& rMark,
/*N*/ 						const ScPatternAttr& rAttr )
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			if (rMark.GetTableSelect(i))
/*N*/ 				pTab[i]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
/*N*/ }


/*N*/ void ScDocument::ApplyPatternAreaTab( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow, USHORT nTab, const ScPatternAttr& rAttr )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
/*N*/ }





/*N*/ void ScDocument::ApplyStyleArea( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow,
/*N*/ 						const ScMarkData& rMark,
/*N*/ 						const ScStyleSheet& rStyle)
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			if (rMark.GetTableSelect(i))
/*N*/ 				pTab[i]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
/*N*/ }


/*N*/ void ScDocument::ApplyStyleAreaTab( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow, USHORT nTab, const ScStyleSheet& rStyle)
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
/*N*/ }


/*N*/ void ScDocument::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
/*N*/ {
/*N*/ 	// ApplySelectionStyle needs multi mark
/*N*/ 	if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		rMark.GetMarkArea( aRange );
/*N*/ 		ApplyStyleArea( aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 						  aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rStyle );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 			if ( pTab[i] && rMark.GetTableSelect(i) )
/*N*/ 					pTab[i]->ApplySelectionStyle( rStyle, rMark );
/*N*/ 	}
/*N*/ }

/*N*/ const ScStyleSheet*	ScDocument::GetSelectionStyle( const ScMarkData& rMark ) const
/*N*/ {
/*N*/ 	BOOL	bEqual = TRUE;
/*N*/ 	BOOL	bFound;
/*N*/ 	USHORT	i;
/*N*/
/*N*/ 	const ScStyleSheet* pStyle = NULL;
/*N*/ 	const ScStyleSheet* pNewStyle;
/*N*/
/*N*/ 	if ( rMark.IsMultiMarked() )
/*N*/ 		for (i=0; i<=MAXTAB && bEqual; i++)
/*N*/ 			if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 			{
/*N*/ 				pNewStyle = pTab[i]->GetSelectionStyle( rMark, bFound );
/*N*/ 				if (bFound)
/*N*/ 				{
/*N*/ 					if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 						bEqual = FALSE;												// unterschiedliche
/*N*/ 					pStyle = pNewStyle;
/*N*/ 				}
/*N*/ 			}
/*N*/ 	if ( rMark.IsMarked() )
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		rMark.GetMarkArea( aRange );
/*N*/ 		for (i=aRange.aStart.Tab(); i<=aRange.aEnd.Tab() && bEqual; i++)
/*N*/ 			if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 			{
/*N*/ 				pNewStyle = pTab[i]->GetAreaStyle( bFound,
/*N*/ 										aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 										aRange.aEnd.Col(),   aRange.aEnd.Row()   );
/*N*/ 				if (bFound)
/*N*/ 				{
/*N*/ 					if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 						bEqual = FALSE;												// unterschiedliche
/*N*/ 					pStyle = pNewStyle;
/*N*/ 				}
/*N*/ 			}
/*N*/ 	}
/*N*/
/*N*/ 	return bEqual ? pStyle : NULL;
/*N*/ }


/*N*/ void ScDocument::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
/*N*/ 									OutputDevice* pDev,
/*N*/ 									double nPPTX, double nPPTY,
/*N*/ 									const Fraction& rZoomX, const Fraction& rZoomY )
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXTAB; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			pTab[i]->StyleSheetChanged
/*N*/ 				( pStyleSheet, bRemoved, pDev, nPPTX, nPPTY, rZoomX, rZoomY );
/*N*/
/*N*/ 	if ( pStyleSheet && pStyleSheet->GetName() == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 	{
/*N*/ 		//	update attributes for all note objects
/*N*/
/*N*/ 		ScDetectiveFunc aFunc( this, 0 );
/*N*/ 		aFunc.UpdateAllComments();
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScDocument::IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001
/*N*/ }





/*N*/ BOOL ScDocument::ApplyFlagsTab( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow, USHORT nTab, INT16 nFlags )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );
/*N*/
/*N*/ 	DBG_ERROR("ApplyFlags: falsche Tabelle");
/*N*/ 	return FALSE;
/*N*/ }




/*N*/ BOOL ScDocument::RemoveFlagsTab( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						USHORT nEndCol, USHORT nEndRow, USHORT nTab, INT16 nFlags )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return FALSE;
/*N*/ }






/*N*/ ScPatternAttr* ScDocument::CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep )
/*N*/ {
/*N*/ 	SfxItemSet* pSet = NULL;
/*N*/ 	USHORT i;
/*N*/
/*N*/ 	if ( rMark.IsMultiMarked() )								// multi selection
/*N*/ 	{
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 				pTab[i]->MergeSelectionPattern( &pSet, rMark, bDeep );
/*N*/ 	}
/*N*/ 	if ( rMark.IsMarked() )										// simle selection
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		rMark.GetMarkArea(aRange);
/*N*/ 		for (i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 				pTab[i]->MergePatternArea( &pSet,
/*N*/ 								aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 								aRange.aEnd.Col(), aRange.aEnd.Row(), bDeep );
/*N*/ 	}
/*N*/
/*N*/ 	DBG_ASSERT( pSet, "SelectionPattern Null" );
/*N*/ 	if (pSet)
/*N*/ 		return new ScPatternAttr( pSet );
/*N*/ 	else
/*?*/ 		return new ScPatternAttr( GetPool() );		// empty
/*N*/ }


/*N*/ void ScDocument::GetSelectionFrame( const ScMarkData& rMark,
/*N*/ 									SvxBoxItem&		rLineOuter,
/*N*/ 									SvxBoxInfoItem&	rLineInner )
/*N*/ {
/*N*/ 	rLineOuter.SetLine(NULL, BOX_LINE_TOP);
/*N*/ 	rLineOuter.SetLine(NULL, BOX_LINE_BOTTOM);
/*N*/ 	rLineOuter.SetLine(NULL, BOX_LINE_LEFT);
/*N*/ 	rLineOuter.SetLine(NULL, BOX_LINE_RIGHT);
/*N*/ 	rLineOuter.SetDistance(0);
/*N*/
/*N*/ 	rLineInner.SetLine(NULL, BOXINFO_LINE_HORI);
/*N*/ 	rLineInner.SetLine(NULL, BOXINFO_LINE_VERT);
/*N*/ 	rLineInner.SetTable(TRUE);
/*N*/ 	rLineInner.SetDist((BOOL)FALSE);
/*N*/ 	rLineInner.SetMinDist(FALSE);
/*N*/
/*N*/ 	ScLineFlags aFlags;
/*N*/
/*N*/ 	if (rMark.IsMarked())
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		rMark.GetMarkArea(aRange);
/*N*/ 		rLineInner.SetTable(aRange.aStart!=aRange.aEnd);
/*N*/ 		for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 				pTab[i]->MergeBlockFrame( &rLineOuter, &rLineInner, aFlags,
/*N*/ 										  aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 										  aRange.aEnd.Col(),   aRange.aEnd.Row() );
/*N*/ 	}
/*N*/
/*N*/ 		//	Don't care Status auswerten
/*N*/
/*N*/ 	rLineInner.SetValid( VALID_LEFT,   ( aFlags.nLeft != SC_LINE_DONTCARE ) );
/*N*/ 	rLineInner.SetValid( VALID_RIGHT,  ( aFlags.nRight != SC_LINE_DONTCARE ) );
/*N*/ 	rLineInner.SetValid( VALID_TOP,    ( aFlags.nTop != SC_LINE_DONTCARE ) );
/*N*/ 	rLineInner.SetValid( VALID_BOTTOM, ( aFlags.nBottom != SC_LINE_DONTCARE ) );
/*N*/ 	rLineInner.SetValid( VALID_HORI,   ( aFlags.nHori != SC_LINE_DONTCARE ) );
/*N*/ 	rLineInner.SetValid( VALID_VERT,   ( aFlags.nVert != SC_LINE_DONTCARE ) );
/*N*/ }


/*N*/ BOOL ScDocument::HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 							USHORT nCol2, USHORT nRow2, USHORT nTab2, USHORT nMask )
/*N*/ {
/*N*/ 	if ( nMask & HASATTR_ROTATE )
/*N*/ 	{
/*N*/ 		//	Attribut im Dokument ueberhaupt verwendet?
/*N*/ 		//	(wie in fillinfo)
/*N*/
/*N*/ 		ScDocumentPool* pPool = xPoolHelper->GetDocPool();
/*N*/
/*N*/ 		BOOL bAnyItem = FALSE;
/*N*/ 		USHORT nRotCount = pPool->GetItemCount( ATTR_ROTATE_VALUE );
/*N*/ 		for (USHORT nItem=0; nItem<nRotCount; nItem++)
/*N*/ 			if (pPool->GetItem( ATTR_ROTATE_VALUE, nItem ))
/*N*/ 			{
/*N*/ 				bAnyItem = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		if (!bAnyItem)
/*N*/ 			nMask &= ~HASATTR_ROTATE;
/*N*/ 	}
/*N*/
/*N*/ 	if ( nMask & HASATTR_RTL )
/*N*/ 	{
/*?*/ 		//	first check if right-to left is in the pool at all
/*?*/ 		//	(the same item is used in cell and page format)
/*?*/
/*?*/ 		ScDocumentPool* pPool = xPoolHelper->GetDocPool();
/*?*/
/*?*/ 		BOOL bHasRtl = FALSE;
/*?*/ 		USHORT nDirCount = pPool->GetItemCount( ATTR_WRITINGDIR );
/*?*/ 		for (USHORT nItem=0; nItem<nDirCount; nItem++)
/*?*/ 		{
/*?*/ 			const SfxPoolItem* pItem = pPool->GetItem( ATTR_WRITINGDIR, nItem );
/*?*/ 			if ( pItem && ((const SvxFrameDirectionItem*)pItem)->GetValue() == FRMDIR_HORI_RIGHT_TOP )
/*?*/ 			{
/*?*/ 				bHasRtl = TRUE;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if (!bHasRtl)
/*?*/ 			nMask &= ~HASATTR_RTL;
/*N*/ 	}
/*N*/
/*N*/ 	if (!nMask)
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	for (USHORT i=nTab1; i<=nTab2 && !bFound; i++)
/*N*/ 		if (pTab[i])
/*N*/ 		{
/*N*/ 			if ( nMask & HASATTR_RTL )
/*N*/ 			{
/*?*/ 				if ( GetEditTextDirection(i) == EE_HTEXTDIR_R2L )		// sheet default
/*?*/ 					bFound = TRUE;
/*N*/ 			}
/*N*/
/*N*/ 			bFound |= pTab[i]->HasAttrib( nCol1, nRow1, nCol2, nRow2, nMask );
/*N*/ 		}
/*N*/
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScDocument::HasAttrib( const ScRange& rRange, USHORT nMask )
/*N*/ {
/*N*/ 	return HasAttrib( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
/*N*/ 					  rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
/*N*/ 					  nMask );
/*N*/ }

/*N*/ BOOL ScDocument::IsBlockEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
/*N*/ 										USHORT nEndCol, USHORT nEndRow ) const
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (VALIDTAB(nTab))
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void ScDocument::LockTable(USHORT nTab)
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->LockTable();
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Tabellennummer");
/*N*/ }


/*N*/ void ScDocument::UnlockTable(USHORT nTab)
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->UnlockTable();
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Tabellennummer");
/*N*/ }


/*N*/ BOOL ScDocument::IsBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
/*N*/ 										USHORT nEndCol, USHORT nEndRow,
/*N*/ 										BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
/*N*/ {
/*N*/ 	// import into read-only document is possible - must be extended if other filters use api
/*N*/ 	if ( pShell && pShell->IsReadOnly() && !bImportingXML )
/*N*/ 	{
/*N*/ 		if ( pOnlyNotBecauseOfMatrix )
/*N*/ 			*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol,
/*N*/ 				nEndRow, pOnlyNotBecauseOfMatrix );
/*N*/
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	if ( pOnlyNotBecauseOfMatrix )
/*N*/ 		*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::IsSelectionEditable( const ScMarkData& rMark,
/*N*/ 			BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
/*N*/ {
/*N*/ 	// import into read-only document is possible - must be extended if other filters use api
/*N*/ 	if ( pShell && pShell->IsReadOnly() && !bImportingXML )
/*N*/ 	{
/*N*/ 		if ( pOnlyNotBecauseOfMatrix )
/*N*/ 			*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	ScRange aRange;
/*N*/ 	rMark.GetMarkArea(aRange);
/*N*/
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	BOOL bMatrix = ( pOnlyNotBecauseOfMatrix != NULL );
/*N*/ 	for ( USHORT i=0; i<=MAXTAB && (bOk || bMatrix); i++ )
/*N*/ 	{
/*N*/ 		if ( pTab[i] && rMark.GetTableSelect(i) )
/*N*/ 		{
/*N*/ 			if (rMark.IsMarked())
/*N*/ 			{
/*N*/ 				if ( !pTab[i]->IsBlockEditable( aRange.aStart.Col(),
/*N*/ 						aRange.aStart.Row(), aRange.aEnd.Col(),
/*N*/ 						aRange.aEnd.Row(), pOnlyNotBecauseOfMatrix ) )
/*N*/ 				{
/*N*/ 					bOk = FALSE;
/*N*/ 					if ( pOnlyNotBecauseOfMatrix )
/*N*/ 						bMatrix = *pOnlyNotBecauseOfMatrix;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (rMark.IsMultiMarked())
/*N*/ 			{
/*N*/ 				if ( !pTab[i]->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix ) )
/*N*/ 				{
/*N*/ 					bOk = FALSE;
/*N*/ 					if ( pOnlyNotBecauseOfMatrix )
/*N*/ 						bMatrix = *pOnlyNotBecauseOfMatrix;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if ( pOnlyNotBecauseOfMatrix )
/*N*/ 		*pOnlyNotBecauseOfMatrix = ( !bOk && bMatrix );
/*N*/
/*N*/ 	return bOk;
/*N*/ }








/*N*/ BOOL ScDocument::GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix )
/*N*/ {
/*N*/ 	//	if rCell is part of a matrix formula, return its complete range
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	ScBaseCell* pCell = GetCell( rCellPos );
/*N*/ 	if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 	{
/*N*/ 		ScAddress aOrigin = rCellPos;
/*N*/ 		if ( ((ScFormulaCell*)pCell)->GetMatrixOrigin( aOrigin ) )
/*N*/ 		{
/*N*/ 			if ( aOrigin != rCellPos )
/*N*/ 				pCell = GetCell( aOrigin );
/*N*/ 			if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			{
/*N*/ 				USHORT nSizeX, nSizeY;
/*N*/ 				((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
/*N*/ 				if ( !(nSizeX && nSizeY) )
/*N*/ 				{
/*?*/ 					// GetMatrixEdge computes also dimensions of the matrix
/*?*/ 					// if not already done (may occur if document is loaded
/*?*/ 					// from old file format).
/*?*/ 					// Needs an "invalid" initialized address.
/*?*/ 					aOrigin = UINT32(0xffffffff);
/*?*/ 					((ScFormulaCell*)pCell)->GetMatrixEdge(aOrigin);
/*?*/ 					((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
/*N*/ 				}
/*N*/ 				if ( nSizeX && nSizeY )
/*N*/ 				{
/*N*/ 					ScAddress aEnd( aOrigin.Col() + nSizeX - 1,
/*N*/ 									aOrigin.Row() + nSizeY - 1,
/*N*/ 									aOrigin.Tab() );
/*N*/
/*N*/ 					rMatrix.aStart = aOrigin;
/*N*/ 					rMatrix.aEnd = aEnd;
/*N*/ 					bRet = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL ScDocument::ExtendOverlapped( USHORT& rStartCol, USHORT& rStartRow,
/*N*/ 								USHORT nEndCol, USHORT nEndRow, USHORT nTab )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	if ( ValidColRow(rStartCol,rStartRow) && ValidColRow(nEndCol,nEndRow) && nTab<=MAXTAB )
/*N*/ 	{
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			USHORT nCol;
/*N*/ 			USHORT nOldCol = rStartCol;
/*N*/ 			USHORT nOldRow = rStartRow;
/*N*/ 			for (nCol=nOldCol; nCol<=nEndCol; nCol++)
/*N*/ 				while (((ScMergeFlagAttr*)GetAttr(nCol,rStartRow,nTab,ATTR_MERGE_FLAG))->
/*N*/ 							IsVerOverlapped())
/*N*/ 					--rStartRow;
/*N*/
/*N*/ 			//!		weiterreichen ?
/*N*/
/*N*/ 			ScAttrArray* pAttrArray = pTab[nTab]->aCol[nOldCol].pAttrArray;
/*N*/ 			short nIndex;
/*N*/ 			pAttrArray->Search( nOldRow, nIndex );
/*N*/ 			USHORT nAttrPos = nOldRow;
/*N*/ 			while (nAttrPos<=nEndRow)
/*N*/ 			{
/*N*/ 				DBG_ASSERT( nIndex < (short) pAttrArray->nCount, "Falscher Index im AttrArray" );
/*N*/
/*N*/ 				if (((ScMergeFlagAttr&)pAttrArray->pData[nIndex].pPattern->
/*N*/ 						GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped())
/*N*/ 				{
/*N*/ 					USHORT nLoopEndRow = Min( nEndRow, pAttrArray->pData[nIndex].nRow );
/*N*/ 					for (USHORT nAttrRow = nAttrPos; nAttrRow <= nLoopEndRow; nAttrRow++)
/*N*/ 					{
/*N*/ 						USHORT nTempCol = nOldCol;
/*N*/ 						do
/*N*/ 							--nTempCol;
/*N*/ 						while (((ScMergeFlagAttr*)GetAttr(nTempCol,nAttrRow,nTab,ATTR_MERGE_FLAG))
/*N*/ 								->IsHorOverlapped());
/*N*/ 						if (nTempCol < rStartCol)
/*N*/ 							rStartCol = nTempCol;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				nAttrPos = pAttrArray->pData[nIndex].nRow + 1;
/*N*/ 				++nIndex;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("ExtendOverlapped: falscher Bereich");
/*N*/
/*N*/ 	return bFound;
/*N*/ }


/*N*/ BOOL ScDocument::ExtendMerge( USHORT nStartCol, USHORT nStartRow,
/*N*/ 							  USHORT& rEndCol,  USHORT& rEndRow,
/*N*/ 							  USHORT nTab, BOOL bRefresh, BOOL bAttrs )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	if ( ValidColRow(nStartCol,nStartRow) && ValidColRow(rEndCol,rEndRow) && nTab<=MAXTAB )
/*N*/ 	{
/*N*/ 		if (pTab[nTab])
/*N*/ 			bFound = pTab[nTab]->ExtendMerge( nStartCol, nStartRow, rEndCol, rEndRow, bRefresh, bAttrs );
/*N*/
/*N*/ 		if (bRefresh)
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			RefreshAutoFilter( nStartCol, nStartRow, rEndCol, rEndRow, nTab );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("ExtendMerge: falscher Bereich");
/*N*/
/*N*/ 	return bFound;
/*N*/ }


/*N*/ BOOL ScDocument::ExtendMerge( ScRange& rRange, BOOL bRefresh, BOOL bAttrs )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndTab   = rRange.aEnd.Tab();
/*N*/ 	USHORT nEndCol   = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow   = rRange.aEnd.Row();
/*N*/
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 	for (USHORT nTab = nStartTab; nTab <= nEndTab; nTab++ )
/*N*/ 	{
/*N*/ 		USHORT nExtendCol = rRange.aEnd.Col();
/*N*/ 		USHORT nExtendRow = rRange.aEnd.Row();
/*N*/ 		if (ExtendMerge( rRange.aStart.Col(), rRange.aStart.Row(),
/*N*/ 						 nExtendCol,          nExtendRow,
/*N*/ 						 nTab, bRefresh, bAttrs ) )
/*N*/ 		{
/*N*/ 			bFound = TRUE;
/*N*/ 			if (nExtendCol > nEndCol) nEndCol = nExtendCol;
/*N*/ 			if (nExtendRow > nEndRow) nEndRow = nExtendRow;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rRange.aEnd.SetCol(nEndCol);
/*N*/ 	rRange.aEnd.SetRow(nEndRow);
/*N*/
/*N*/ 	return bFound;
/*N*/ }


/*N*/ BOOL ScDocument::ExtendOverlapped( ScRange& rRange )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT nStartTab = rRange.aStart.Tab();
/*N*/ 	USHORT nEndTab   = rRange.aEnd.Tab();
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 	for (USHORT nTab = nStartTab; nTab <= nEndTab; nTab++ )
/*N*/ 	{
/*N*/ 		USHORT nExtendCol = rRange.aStart.Col();
/*N*/ 		USHORT nExtendRow = rRange.aStart.Row();
/*N*/ 		ExtendOverlapped( nExtendCol, nExtendRow,
/*N*/ 								rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
/*N*/ 		if (nExtendCol < nStartCol)
/*N*/ 		{
/*N*/ 			nStartCol = nExtendCol;
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 		if (nExtendRow < nStartRow)
/*N*/ 		{
/*N*/ 			nStartRow = nExtendRow;
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rRange.aStart.SetCol(nStartCol);
/*N*/ 	rRange.aStart.SetRow(nStartRow);
/*N*/
/*N*/ 	return bFound;
/*N*/ }



/*N*/ void ScDocument::SetAutoFilterFlags()
/*N*/ {
/*N*/ 	USHORT nCount = pDBCollection->GetCount();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScDBData* pData = (*pDBCollection)[i];
/*N*/ 		USHORT nDBTab;
/*N*/ 		USHORT nDBStartCol;
/*N*/ 		USHORT nDBStartRow;
/*N*/ 		USHORT nDBEndCol;
/*N*/ 		USHORT nDBEndRow;
/*N*/ 		pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
/*N*/ 		pData->SetAutoFilter( HasAttrib( nDBStartCol,nDBStartRow,nDBTab,
/*N*/ 								nDBEndCol,nDBStartRow,nDBTab, HASATTR_AUTOFILTER ) );
/*N*/ 	}
/*N*/ }








/*N*/ void ScDocument::ApplySelectionFrame( const ScMarkData& rMark,
/*N*/ 									  const SvxBoxItem* pLineOuter,
/*N*/ 									  const SvxBoxInfoItem* pLineInner )
/*N*/ {
/*N*/ 	if (rMark.IsMarked())
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		rMark.GetMarkArea(aRange);
/*N*/ 		for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				if (rMark.GetTableSelect(i))
/*N*/ 					pTab[i]->ApplyBlockFrame( pLineOuter, pLineInner,
/*N*/ 												aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 												aRange.aEnd.Col(),   aRange.aEnd.Row() );
/*N*/ 	}
/*N*/ }




/*N*/ void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark )
/*N*/ {
/*N*/ 	const SfxItemSet* pSet = &rAttr.GetItemSet();
/*N*/ 	BOOL bSet = FALSE;
/*N*/ 	USHORT i;
/*N*/ 	for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
/*N*/ 		if (pSet->GetItemState(i) == SFX_ITEM_SET)
/*N*/ 			bSet = TRUE;
/*N*/
/*N*/ 	if (bSet)
/*N*/ 	{
/*N*/ 		// ApplySelectionCache needs multi mark
/*N*/ 		if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
/*N*/ 		{
/*N*/ 			ScRange aRange;
/*N*/ 			rMark.GetMarkArea( aRange );
/*N*/ 			ApplyPatternArea( aRange.aStart.Col(), aRange.aStart.Row(),
/*N*/ 							  aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rAttr );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SfxItemPoolCache aCache( xPoolHelper->GetDocPool(), pSet );
/*N*/ 			for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 				if (pTab[i])
/*N*/ 					if (rMark.GetTableSelect(i))
/*N*/ 						pTab[i]->ApplySelectionCache( &aCache, rMark );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScDocument::ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ }


/*N*/ void ScDocument::ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ }






/*N*/ ScPatternAttr* ScDocument::GetDefPattern() const
/*N*/ {
/*N*/ 	return (ScPatternAttr*) &xPoolHelper->GetDocPool()->GetDefaultItem(ATTR_PATTERN);
/*N*/ }


/*N*/ ScDocumentPool* ScDocument::GetPool()
/*N*/ {
/*N*/ 	return xPoolHelper->GetDocPool();
/*N*/ }



/*N*/ ScStyleSheetPool* ScDocument::GetStyleSheetPool() const
/*N*/ {
/*N*/ 	return xPoolHelper->GetStylePool();
/*N*/ }






/*N*/ void ScDocument::GetNextPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY,
/*N*/ 								BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
/*N*/ {
/*N*/ 	DBG_ASSERT( !nMovX || !nMovY, "GetNextPos: nur X oder Y" );
/*N*/
/*N*/ 	ScMarkData aCopyMark = rMark;
/*N*/ 	aCopyMark.SetMarking(FALSE);
/*N*/ 	aCopyMark.MarkToMulti();
/*N*/
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->GetNextPos( rCol, rRow, nMovX, nMovY, bMarked, bUnprotected, aCopyMark );
/*N*/ }

//
//	Datei-Operationen
//


/*N*/ void ScDocument::UpdStlShtPtrsFrmNms()
/*N*/ {
/*N*/ 	ScPatternAttr::pDoc = this;
/*N*/
/*N*/ 	ScDocumentPool* pPool = xPoolHelper->GetDocPool();
/*N*/
/*N*/ 	USHORT nCount = pPool->GetItemCount(ATTR_PATTERN);
/*N*/ 	ScPatternAttr* pPattern;
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		pPattern = (ScPatternAttr*)pPool->GetItem(ATTR_PATTERN, i);
/*N*/ 		if (pPattern)
/*N*/ 			pPattern->UpdateStyleSheet();
/*N*/ 	}
/*N*/ 	((ScPatternAttr&)pPool->GetDefaultItem(ATTR_PATTERN)).UpdateStyleSheet();
/*N*/ }




/*N*/ void lcl_RemoveMergeFromStyles( ScStyleSheetPool* pStylePool )
/*N*/ {
/*N*/ 	pStylePool->SetSearchMask( SFX_STYLE_FAMILY_ALL );
/*N*/
/*N*/ 	USHORT nCount = pStylePool->Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		//	in alten Versionen wurden statt SFXSTYLEBIT_USERDEF alle Bits gesetzt
/*N*/ 		SfxStyleSheetBase* pStyle = (*pStylePool)[i];
/*N*/ 		if ( pStyle->GetMask() & SFXSTYLEBIT_READONLY )
/*N*/ 			pStyle->SetMask( pStyle->GetMask() & ~SFXSTYLEBIT_READONLY );
/*N*/
/*N*/ 		SfxItemSet& rSet = pStyle->GetItemSet();
/*N*/ 		rSet.ClearItem( ATTR_MERGE );
/*N*/ 		rSet.ClearItem( ATTR_MERGE_FLAG );
/*N*/
/*N*/ 		//	Das SvxBoxInfoItem wurde bis zur 358 falsch geladen, so dass
/*N*/ 		//	Seitenvorlagen falsche Items mit bDist = FALSE enthalten koennen
/*N*/ 		if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PAGE )
/*N*/ 		{
/*N*/ 			const SvxBoxInfoItem& rPageInfo = (const SvxBoxInfoItem&)rSet.Get(ATTR_BORDER_INNER);
/*N*/ 			if ( !rPageInfo.IsDist() )
/*N*/ 			{
/*N*/ 				DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
/*N*/ 				SvxBoxInfoItem aNew( rPageInfo );
/*N*/ 				aNew.SetDist( TRUE );
/*N*/ 				rSet.Put( aNew );
/*N*/ 			}
/*N*/ 			//	Das gilt fuer alle Hdr/Ftr-SetItems, darum kann das SetItem auch
/*N*/ 			//	direkt im Pool geaendert werden (const weggecastet):
/*N*/ 			SfxItemSet& rHdrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_HEADERSET)).GetItemSet();
/*N*/ 			const SvxBoxInfoItem& rHdrInfo = (const SvxBoxInfoItem&)rHdrSet.Get(ATTR_BORDER_INNER);
/*N*/ 			if ( !rHdrInfo.IsDist() )
/*N*/ 			{
/*N*/ 				DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
/*N*/ 				SvxBoxInfoItem aNew( rHdrInfo );
/*N*/ 				aNew.SetDist( TRUE );
/*N*/ 				rHdrSet.Put( aNew );
/*N*/ 			}
/*N*/ 			SfxItemSet& rFtrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_FOOTERSET)).GetItemSet();
/*N*/ 			const SvxBoxInfoItem& rFtrInfo = (const SvxBoxInfoItem&)rFtrSet.Get(ATTR_BORDER_INNER);
/*N*/ 			if ( !rFtrInfo.IsDist() )
/*N*/ 			{
/*N*/ 				DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
/*N*/ 				SvxBoxInfoItem aNew( rFtrInfo );
/*N*/ 				aNew.SetDist( TRUE );
/*N*/ 				rFtrSet.Put( aNew );
/*N*/ 			}
/*N*/ 			const SfxUInt16Item& rScaleItem = (const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALE);
/*N*/ 			USHORT nScale = rScaleItem.GetValue();
/*N*/ 			//! Extra-Konstanten fuer Seitenformat?
/*N*/ 			//	0 ist erlaubt (wird gesetzt bei Scale To Pages)
/*N*/ 			if ( nScale != 0 && ( nScale < MINZOOM || nScale > MAXZOOM ) )
/*N*/ 			{
/*?*/ 				//	konnte anscheinend mal irgendwie kaputtgehen (#34508#)
/*?*/ 				DBG_WARNING("kaputter Zoom im Seitenformat muss korrigiert werden");
/*?*/ 				rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScDocument::LoadPool( SvStream& rStream, BOOL bLoadRefCounts )
/*N*/ {
/*N*/ 	//	bLoadingDone wird beim Laden des StylePools (ScStyleSheet::GetItemSet) gebraucht
/*N*/ 	bLoadingDone = FALSE;
/*N*/
/*N*/ 	USHORT nOldBufSize = rStream.GetBufferSize();
/*N*/ 	rStream.SetBufferSize( 32768 );
/*N*/ 	CharSet eOldSet = rStream.GetStreamCharSet();
/*N*/
/*N*/ 	SetPrinter( NULL );
/*N*/
/*N*/ 	ScPatternAttr::pDoc = this;
/*N*/
/*N*/ 	if ( xPoolHelper.is() && !bIsClip )
/*N*/ 		xPoolHelper->SourceDocumentGone();
/*N*/
/*N*/ 	xPoolHelper = new ScPoolHelper( this );
/*N*/
/*N*/ 	xPoolHelper->GetDocPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
/*N*/ 	BOOL bStylesFound = FALSE;
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	USHORT nID;
/*N*/ 	rStream >> nID;
/*N*/ 	if (nID == SCID_POOLS || nID == SCID_NEWPOOLS)
/*N*/ 	{
/*N*/ 		ScReadHeader aHdr( rStream );
/*N*/ 		while (aHdr.BytesLeft())
/*N*/ 		{
/*N*/ 			USHORT nSubID;
/*N*/ 			rStream >> nSubID;
/*N*/ 			ScReadHeader aSubHdr( rStream );
/*N*/ 			switch (nSubID)
/*N*/ 			{
/*N*/ 				case SCID_CHARSET:
/*N*/ 					{
/*N*/ 						BYTE cSet, cGUI;	// cGUI is dummy, old GUIType
/*N*/ 						rStream >> cGUI >> cSet;
/*N*/ 						eSrcSet = (CharSet) cSet;
/*N*/                         rStream.SetStreamCharSet( ::GetSOLoadTextEncoding(
/*N*/                             eSrcSet, (USHORT)rStream.GetVersion() ) );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case SCID_DOCPOOL:
/*N*/ 					xPoolHelper->GetDocPool()->Load( rStream );
/*N*/ 					break;
/*N*/ 				case SCID_STYLEPOOL:
/*N*/ 					{
/*N*/ 						//	StylePool konvertiert beim Laden selber
/*N*/ 						CharSet eOld = rStream.GetStreamCharSet();
/*N*/ 						rStream.SetStreamCharSet( gsl_getSystemTextEncoding() );	//! ???
/*N*/ 						xPoolHelper->GetStylePool()->Load( rStream );
/*N*/ 						rStream.SetStreamCharSet( eOld );
/*N*/ 						lcl_RemoveMergeFromStyles( xPoolHelper->GetStylePool() );	// setzt auch ReadOnly zurueck
/*N*/ 						bStylesFound = TRUE;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case SCID_EDITPOOL :
/*N*/ 					xPoolHelper->GetEditPool()->Load( rStream );
/*N*/ 					break;
/*?*/ 				default:
/*?*/ 					DBG_ERROR("unbekannter Sub-Record in ScDocument::LoadPool");
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		UpdStlShtPtrsFrmNms();
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	else
/*?*/ 		DBG_ERROR("LoadPool: SCID_POOLS nicht gefunden");
/*N*/
/*N*/ 	if (!bStylesFound)
/*?*/ 		xPoolHelper->GetStylePool()->CreateStandardStyles();
/*N*/
/*N*/ 	rStream.SetStreamCharSet( eOldSet );
/*N*/ 	rStream.SetBufferSize( nOldBufSize );
/*N*/
/*N*/ 	bLoadingDone = TRUE;
/*N*/
/*N*/ 	//	Das Uno-Objekt merkt sich einen Pointer auf den NumberFormatter
/*N*/ 	//	-> mitteilen, dass der alte Pointer ungueltig geworden ist
/*N*/ 	BroadcastUno( ScPointerChangedHint(SC_POINTERCHANGED_NUMFMT) );
/*N*/
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL ScDocument::SavePool( SvStream& rStream ) const
/*N*/ {
/*N*/ 	xPoolHelper->GetDocPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
/*N*/
/*N*/ 	USHORT nOldBufSize = rStream.GetBufferSize();
/*N*/ 	rStream.SetBufferSize( 32768 );
/*N*/ 	CharSet eOldSet = rStream.GetStreamCharSet();
/*N*/     CharSet eStoreCharSet = ::GetSOStoreTextEncoding(
/*N*/         gsl_getSystemTextEncoding(), (USHORT)rStream.GetVersion() );
/*N*/     rStream.SetStreamCharSet( eStoreCharSet );
/*N*/
/*N*/ 	//	::com::press-Mode fuer Grafiken in Brush-Items (Hintergrund im Seitenformat)
/*N*/
/*N*/ 	USHORT nComprMode = rStream.GetCompressMode() & ~(COMPRESSMODE_ZBITMAP | COMPRESSMODE_NATIVE);
/*N*/   BOOL bNative = FALSE;
/*N*/   BOOL bCompr = FALSE;
/*N*/
/*N*/ 	if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_40 && bCompr )
/*N*/ 		nComprMode |= COMPRESSMODE_ZBITMAP;				//	komprimiert ab 4.0
/*N*/ 	if ( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 && bNative )
/*N*/ 		nComprMode |= COMPRESSMODE_NATIVE;				//	Originalformat ab 5.0
/*N*/ 	rStream.SetCompressMode( nComprMode );
/*N*/
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_NEWPOOLS;
/*N*/ 		ScWriteHeader aHdr( rStream );
/*N*/
/*N*/ 		{
/*N*/ 			rStream << (USHORT) SCID_CHARSET;
/*N*/ 			ScWriteHeader aSetHdr( rStream, 2 );
/*N*/ 			rStream << (BYTE) 0		// dummy, old System::GetGUIType()
/*N*/                     << (BYTE) eStoreCharSet;
/*N*/ 		}
/*N*/
/*N*/ 		//	Force the default style's name to be "Standard" for all languages in the file.
/*N*/ 		//	This is needed for versions up to 5.1, to find the default pattern's style in
/*N*/ 		//	the UpdateStyleSheet call.
/*N*/ 		//	#89078# this has to be set for the DocPool save, too, so the default style name
/*N*/ 		//	is adjusted for the patterns, or a wrong style would be used if other styles
/*N*/ 		//	match the default style's name after CharacterSet conversion.
/*N*/
/*N*/ 		String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
/*N*/ 		if ( aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
/*N*/ 			xPoolHelper->GetStylePool()->SetForceStdName( &aFileStdName );
/*N*/
/*N*/ 		{
/*N*/ 			rStream << (USHORT) SCID_DOCPOOL;
/*N*/ 			ScWriteHeader aDocPoolHdr( rStream );
/*N*/ 			xPoolHelper->GetDocPool()->Store( rStream );
/*N*/ 		}
/*N*/
/*N*/ 		{
/*N*/ 			rStream << (USHORT) SCID_STYLEPOOL;
/*N*/ 			ScWriteHeader aStylePoolHdr( rStream );
/*N*/ 			xPoolHelper->GetStylePool()->SetSearchMask( SFX_STYLE_FAMILY_ALL );
/*N*/
/*N*/ 			xPoolHelper->GetStylePool()->Store( rStream, FALSE );
/*N*/ 		}
/*N*/
/*N*/ 		xPoolHelper->GetStylePool()->SetForceStdName( NULL );
/*N*/
/*N*/ 		if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_50 )
/*N*/ 		{
/*N*/ 			rStream << (USHORT) SCID_EDITPOOL;
/*N*/ 			ScWriteHeader aEditPoolHdr( rStream );
/*N*/ 			xPoolHelper->GetEditPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
/*N*/ 			xPoolHelper->GetEditPool()->Store( rStream );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rStream.SetStreamCharSet( eOldSet );
/*N*/ 	rStream.SetBufferSize( nOldBufSize );
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ long ScDocument::GetCellCount() const
/*N*/ {
/*N*/ 	long nCellCount = 0L;
/*N*/
/*N*/ 	for ( USHORT nTab=0; nTab<=MAXTAB; nTab++ )
/*N*/ 		if ( pTab[nTab] )
/*N*/ 			nCellCount += pTab[nTab]->GetCellCount();
/*N*/
/*N*/ 	return nCellCount;
/*N*/ }


/*N*/ long ScDocument::GetWeightedCount() const
/*N*/ {
/*N*/ 	long nCellCount = 0L;
/*N*/
/*N*/ 	for ( USHORT nTab=0; nTab<=MAXTAB; nTab++ )
/*N*/ 		if ( pTab[nTab] )
/*N*/ 			nCellCount += pTab[nTab]->GetWeightedCount();
/*N*/
/*N*/ 	return nCellCount;
/*N*/ }




/*N*/ void ScDocument::SetPageStyle( USHORT nTab, const String& rName )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetPageStyle( rName );
/*N*/ }


/*N*/ const String& ScDocument::GetPageStyle( USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetPageStyle();
/*N*/
/*N*/ 	return EMPTY_STRING;
/*N*/ }


/*N*/ void ScDocument::SetPageSize( USHORT nTab, const Size& rSize )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetPageSize( rSize );
/*N*/ }

/*N*/ Size ScDocument::GetPageSize( USHORT nTab ) const
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->GetPageSize();
/*N*/
/*N*/ 	DBG_ERROR("falsche Tab");
/*N*/ 	return Size();
/*N*/ }


/*N*/ void ScDocument::SetRepeatArea( USHORT nTab, USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->SetRepeatArea( nStartCol, nEndCol, nStartRow, nEndRow );
/*N*/ }

/*N*/ void ScDocument::UpdatePageBreaks( USHORT nTab, const ScRange* pUserArea )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->UpdatePageBreaks( pUserArea );
/*N*/ }

/*N*/ void ScDocument::RemoveManualBreaks( USHORT nTab )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->RemoveManualBreaks();
/*N*/ }





/*N*/ BOOL ScDocument::HasPrintRange()
/*N*/ {
/*N*/ 	BOOL bResult = FALSE;
/*N*/
/*N*/ 	for ( USHORT i=0; !bResult && i<nMaxTableNumber; i++ )
/*N*/ 		if ( pTab[i] )
/*N*/ 			bResult = ( pTab[i]->GetPrintRangeCount() > 0 );
/*N*/
/*N*/ 	return bResult;
/*N*/ }


/*N*/ USHORT ScDocument::GetPrintRangeCount( USHORT nTab )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetPrintRangeCount();
/*N*/
/*N*/ 	return 0;
/*N*/ }


/*N*/ const ScRange* ScDocument::GetPrintRange( USHORT nTab, USHORT nPos )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetPrintRange(nPos);
/*N*/
/*N*/ 	return NULL;
/*N*/ }


/*N*/ const ScRange* ScDocument::GetRepeatColRange( USHORT nTab )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetRepeatColRange();
/*N*/
/*N*/ 	return NULL;
/*N*/ }


/*N*/ const ScRange* ScDocument::GetRepeatRowRange( USHORT nTab )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetRepeatRowRange();
/*N*/
/*N*/ 	return NULL;
/*N*/ }


// #42845# zeroptimiert
/*N*/ #if defined(WIN) && defined(MSC)
/*N*/ #pragma optimize("",off)
/*N*/ #endif
/*N*/ void ScDocument::SetPrintRangeCount( USHORT nTab, USHORT nNew )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetPrintRangeCount( nNew );
/*N*/ }
/*N*/ #if defined(WIN) && defined(MSC)
/*N*/ #pragma optimize("",on)
/*N*/ #endif


/*N*/ void ScDocument::SetPrintRange( USHORT nTab, USHORT nPos, const ScRange& rNew )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetPrintRange( nPos, rNew );
/*N*/ }


/*N*/ void ScDocument::SetRepeatColRange( USHORT nTab, const ScRange* pNew )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetRepeatColRange( pNew );
/*N*/ }


/*N*/ void ScDocument::SetRepeatRowRange( USHORT nTab, const ScRange* pNew )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetRepeatRowRange( pNew );
/*N*/ }


/*N*/ ScPrintRangeSaver* ScDocument::CreatePrintRangeSaver() const
/*N*/ {
/*N*/ 	USHORT nCount = GetTableCount();
/*N*/ 	ScPrintRangeSaver* pNew = new ScPrintRangeSaver( nCount );
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		if (pTab[i])
/*N*/ 			pTab[i]->FillPrintSaver( pNew->GetTabData(i) );
/*N*/ 	return pNew;
/*N*/ }







}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
