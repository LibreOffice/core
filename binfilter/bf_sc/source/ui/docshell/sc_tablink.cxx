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
#pragma optimize("",off)
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------

#include <bf_sfx2/app.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/fcontnr.hxx>
#include <bf_svx/linkmgr.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "tablink.hxx"

#include "docsh.hxx"
#include "globstr.hrc"
#include "hints.hxx"
namespace binfilter {

/*N*/ TYPEINIT1(ScTableLink, ::binfilter::SvBaseLink);

//------------------------------------------------------------------------

/*N*/ ScTableLink::ScTableLink(ScDocShell* pDocSh, const String& rFile,
/*N*/ 							const String& rFilter, const String& rOpt,
/*N*/ 							ULONG nRefresh ):
/*N*/ 	::binfilter::SvBaseLink(::binfilter::LINKUPDATE_ONCALL,FORMAT_FILE),
/*N*/ 	ScRefreshTimer( nRefresh ),
/*N*/ 	pDocShell(pDocSh),
/*N*/ 	aFileName(rFile),
/*N*/ 	aFilterName(rFilter),
/*N*/ 	aOptions(rOpt),
/*N*/ 	bInCreate( FALSE ),
/*M*/ 	bInEdit( FALSE ),
/*N*/ 	bAddUndo( TRUE ),
/*N*/ 	bDoPaint( TRUE )
/*N*/ {
/*N*/ }

/*N*/ ScTableLink::ScTableLink(SfxObjectShell* pShell, const String& rFile,
/*N*/ 							const String& rFilter, const String& rOpt,
/*N*/ 							ULONG nRefresh ):
/*N*/ 	::binfilter::SvBaseLink(::binfilter::LINKUPDATE_ONCALL,FORMAT_FILE),
/*N*/ 	ScRefreshTimer( nRefresh ),
/*N*/ 	pDocShell((ScDocShell*)pShell),
/*N*/ 	aFileName(rFile),
/*N*/ 	aFilterName(rFilter),
/*N*/ 	aOptions(rOpt),
/*N*/ 	bInCreate( FALSE ),
/*M*/ 	bInEdit( FALSE ),
/*N*/ 	bAddUndo( TRUE ),
/*N*/ 	bDoPaint( TRUE )
/*N*/ {
/*N*/ 	SetRefreshHandler( LINK( this, ScTableLink, RefreshHdl ) );
/*N*/ 	SetRefreshControl( pDocShell->GetDocument()->GetRefreshTimerControlAddress() );
/*N*/ }

/*N*/ __EXPORT ScTableLink::~ScTableLink()
/*N*/ {
/*N*/ 	// Verbindung aufheben
/*N*/ 
/*N*/ 	StopRefreshTimer();
/*N*/ 	String aEmpty;
/*N*/ 	ScDocument* pDoc = pDocShell->GetDocument();
/*N*/ 	USHORT nCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nTab=0; nTab<nCount; nTab++)
/*N*/ 		if (pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab)==aFileName)
/*N*/ 			pDoc->SetLink( nTab, SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty, 0 );
/*N*/ }


/*N*/ void __EXPORT ScTableLink::DataChanged( const String&,
/*N*/ 										const ::com::sun::star::uno::Any& )
/*N*/ {
/*N*/ 	SvxLinkManager* pLinkManager=pDocShell->GetDocument()->GetLinkManager();
/*N*/ 	if (pLinkManager!=NULL)
/*N*/ 	{
/*N*/ 		String aFile;
/*N*/ 		String aFilter;
/*N*/ 		pLinkManager->GetDisplayNames( this,0,&aFile,NULL,&aFilter);
/*N*/ 
/*N*/ 		//	the file dialog returns the filter name with the application prefix
/*N*/ 		//	-> remove prefix
/*N*/ 		ScDocumentLoader::RemoveAppPrefix( aFilter );
/*N*/ 
/*N*/ 		if (!bInCreate)
/*N*/ 			Refresh( aFile, aFilter, NULL, GetRefreshDelay() );	// don't load twice
/*N*/ 	}
/*N*/ }



/*N*/ BOOL ScTableLink::Refresh(const String& rNewFile, const String& rNewFilter,
/*N*/ 							const String* pNewOptions, ULONG nNewRefresh )
/*N*/ {
/*N*/ 	//	Dokument laden
/*N*/ 
/*N*/ 	if (!rNewFile.Len() || !rNewFilter.Len())
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pDocShell ) );
/*N*/ 	BOOL bNewUrlName = (aNewUrl != aFileName);
/*N*/ 
/*N*/ 	const SfxFilter* pFilter = SFX_APP()->GetFilter( pDocShell->GetFactory(), rNewFilter );
/*N*/ 	if (!pFilter)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDocument* pDoc = pDocShell->GetDocument();
/*N*/ 	pDoc->SetInLinkUpdate( TRUE );
/*N*/ 
/*N*/ 	//	wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
/*N*/ 	if ( rNewFilter != aFilterName )
/*?*/ 		aOptions.Erase();
/*N*/ 	if ( pNewOptions )					// Optionen hart angegeben?
/*N*/ 		aOptions = *pNewOptions;
/*N*/ 
/*N*/ 	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
/*N*/ 	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
/*N*/ 	if ( aOptions.Len() )
/*N*/ 		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
/*N*/ 
/*N*/ 	SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, FALSE, pFilter, pSet);
/*N*/ 
/*M*/ 	if ( bInEdit )								// only if using the edit dialog,
/*M*/ 		pMed->UseInteractionHandler( TRUE );	// enable the filter options dialog
/*M*/ 
/*N*/ 	ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
/*N*/ 	SvEmbeddedObjectRef aRef = pSrcShell;
/*N*/ 	pSrcShell->DoLoad(pMed);
/*N*/ 
/*N*/ 	// Optionen koennten gesetzt worden sein
/*N*/ 	String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
/*N*/ 	if (!aNewOpt.Len())
/*N*/ 		aNewOpt = aOptions;
/*N*/ 
/*N*/ 	BOOL bFirst = TRUE;
/*N*/ 
/*N*/ 	//	Tabellen kopieren
/*N*/ 
/*N*/ 	ScDocShellModificator aModificator( *pDocShell );
/*N*/ 
/*N*/ 	BOOL bNotFound = FALSE;
/*N*/ 	ScDocument* pSrcDoc = pSrcShell->GetDocument();
/*N*/ 
/*N*/ 	//	#74835# from text filters that don't set the table name,
/*N*/ 	//	use the one table regardless of link table name
/*N*/ 	BOOL bAutoTab = (pSrcDoc->GetTableCount() == 1) &&
/*N*/ 					ScDocShell::HasAutomaticTableName( rNewFilter );
/*N*/ 
/*N*/ 	USHORT nCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nTab=0; nTab<nCount; nTab++)
/*N*/ 	{
/*N*/ 		BYTE nMode = pDoc->GetLinkMode(nTab);
/*N*/ 		if (nMode && pDoc->GetLinkDoc(nTab)==aFileName)
/*N*/ 		{
/*N*/ 			String aTabName = pDoc->GetLinkTab(nTab);
/*N*/ 
/*N*/ 			//	Tabellenname einer ExtDocRef anpassen
/*N*/ 
/*N*/ 			if ( bNewUrlName && nMode == SC_LINK_VALUE )
/*N*/ 			{
/*N*/ 				String aName;
/*N*/ 				pDoc->GetName( nTab, aName );
/*N*/                 if ( ScGlobal::pTransliteration->isEqual(
/*N*/                         ScGlobal::GetDocTabName( aFileName, aTabName ), aName ) )
/*N*/ 				{
/*N*/ 					pDoc->RenameTab( nTab,
/*N*/ 						ScGlobal::GetDocTabName( aNewUrl, aTabName ),
/*N*/ 						FALSE, TRUE );	// kein RefUpdate, kein ValidTabName
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			//	kopieren
/*N*/ 
/*N*/ 			USHORT nSrcTab = 0;
/*N*/ 			BOOL bFound = TRUE;			// kein Tab-Name angegeben: immer die erste
/*N*/ 			if ( aTabName.Len() && !bAutoTab )
/*N*/ 				bFound = pSrcDoc->GetTable( aTabName, nSrcTab );
/*N*/ 			if (bFound)
/*N*/ 				pDoc->TransferTab( pSrcDoc, nSrcTab, nTab, FALSE,		// nicht neu einfuegen
/*N*/ 										(nMode == SC_LINK_VALUE) );		// nur Werte?
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pDoc->DeleteAreaTab( 0,0,MAXCOL,MAXROW, nTab, IDF_ALL );
/*N*/ //				pDoc->ClearDrawPage(nTab);
/*N*/ 				//	Fehler eintragen
/*N*/ 				pDoc->SetString( 0,0,nTab, ScGlobal::GetRscString(STR_LINKERROR) );
/*N*/ 				pDoc->SetString( 0,1,nTab, ScGlobal::GetRscString(STR_LINKERRORFILE) );
/*N*/ 				pDoc->SetString( 1,1,nTab, aNewUrl );
/*N*/ 				pDoc->SetString( 0,2,nTab, ScGlobal::GetRscString(STR_LINKERRORTAB) );
/*N*/ 				pDoc->SetString( 1,2,nTab, aTabName );
/*N*/ 
/*N*/ 				bNotFound = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( bNewUrlName || rNewFilter != aFilterName ||
/*N*/ 					aNewOpt != aOptions || pNewOptions ||
/*N*/ 					nNewRefresh != GetRefreshDelay() )
/*?*/ 				pDoc->SetLink( nTab, nMode, aNewUrl, rNewFilter, aNewOpt,
/*?*/ 					aTabName, nNewRefresh );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	neue Einstellungen merken
/*N*/ 
/*N*/ 	if ( bNewUrlName )
/*N*/ 		aFileName = aNewUrl;
/*N*/ 	if ( rNewFilter != aFilterName )
/*N*/ 		aFilterName = rNewFilter;
/*N*/ 	if ( aNewOpt != aOptions )
/*N*/ 		aOptions = aNewOpt;
/*N*/ 
/*N*/ 	//	aufraeumen
/*N*/ 
/*N*/ //	pSrcShell->DoClose();
/*N*/ 	aRef->DoClose();
/*N*/ 
/*N*/ 	//	Paint (koennen mehrere Tabellen sein)
/*N*/ 
/*N*/ 	if (bDoPaint)
/*N*/ 	{
/*?*/ 		pDocShell->PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
/*?*/ 								PAINT_GRID | PAINT_TOP | PAINT_LEFT );
/*?*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bNotFound)
/*N*/ 	{
/*N*/ 		//!	Fehler ausgeben ?
/*N*/ 	}
/*N*/ 
/*N*/ 	pDoc->SetInLinkUpdate( FALSE );
/*N*/ 
/*N*/ 	//	notify Uno objects (for XRefreshListener)
/*N*/ 	//!	also notify Uno objects if file name was changed!
/*N*/ 	ScLinkRefreshedHint aHint;
/*N*/ 	aHint.SetSheetLink( aFileName );
/*N*/ 	pDoc->BroadcastUno( aHint );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ IMPL_LINK( ScTableLink, RefreshHdl, ScTableLink*, pCaller )
/*N*/ {
/*N*/ 	long nRes = Refresh( aFileName, aFilterName, NULL, GetRefreshDelay() ) != 0;
/*N*/ 	return nRes;
/*N*/ }


// === ScDocumentLoader ==================================================

/*N*/ String ScDocumentLoader::GetOptions( SfxMedium& rMedium )		// static
/*N*/ {
/*N*/ 	SfxItemSet* pSet = rMedium.GetItemSet();
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
/*N*/ 		return ((const SfxStringItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ void ScDocumentLoader::GetFilterName( const String& rFileName,
/*N*/ 									String& rFilter, String& rOptions, BOOL bWithContent )	// static
/*N*/ {
/*N*/ 	TypeId aScType = TYPE(ScDocShell);
/*N*/ 	SfxObjectShell* pDocSh = SfxObjectShell::GetFirst( &aScType );
/*N*/ 	while ( pDocSh )
/*N*/ 	{
/*N*/ 		if ( pDocSh->HasName() )
/*N*/ 		{
/*?*/ 			SfxMedium* pMed = pDocSh->GetMedium();
/*?*/ 			if ( rFileName == pMed->GetName() )
/*?*/ 			{
/*?*/ 				rFilter = pMed->GetFilter()->GetFilterName();
/*?*/ 				rOptions = GetOptions(*pMed);
/*?*/ 				return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pDocSh = SfxObjectShell::GetNext( *pDocSh, &aScType );
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Filter-Detection
/*N*/ 
/*N*/ 	const SfxFilter* pSfxFilter = NULL;
/*N*/ 	SfxMedium* pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE );
/*N*/ 	if ( pMedium->GetError() == ERRCODE_NONE )
/*N*/ 	{
/*N*/ 		SfxFilterContainer* pContainer = ScDocShell::Factory().GetFilterContainer();
/*N*/ 		if ( bWithContent && pContainer )
/*N*/ 		{
/*?*/ 			// look at file content (call DetectFilter)
/*?*/ 			pContainer->GetFilter4Content( *pMedium, &pSfxFilter );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SfxFilterMatcher aMatcher( pContainer );
/*N*/ 			aMatcher.GuessFilter( *pMedium, &pSfxFilter );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pSfxFilter )
/*N*/ 		rFilter = pSfxFilter->GetFilterName();
/*N*/ 	else
/*?*/ 		rFilter = ScDocShell::GetOwnFilterName();		//	sonst Calc-Datei
/*N*/ 
/*N*/ 	delete pMedium;
/*N*/ }

/*N*/ void ScDocumentLoader::RemoveAppPrefix( String& rFilterName )		// static
/*N*/ {
/*N*/ 	String aAppPrefix = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));
/*N*/ 	aAppPrefix.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
/*N*/ 	xub_StrLen nPreLen = aAppPrefix.Len();
/*N*/ 	if ( rFilterName.Copy(0,nPreLen) == aAppPrefix )
/*?*/ 		rFilterName.Erase(0,nPreLen);
/*N*/ }

/*M*/ ScDocumentLoader::ScDocumentLoader( const String& rFileName,
/*M*/ 									String& rFilterName, String& rOptions,
/*M*/ 									UINT32 nRekCnt, BOOL bWithInteraction ) :
/*M*/ 		pDocShell(0),
/*M*/ 		pMedium(0)
/*M*/ {
/*M*/ 	if ( !rFilterName.Len() )
/*M*/ 		GetFilterName( rFileName, rFilterName, rOptions );
/*M*/ 
/*M*/ 	const SfxFilter* pFilter = SFX_APP()->GetFilter(
/*M*/ 								ScDocShell::Factory(), rFilterName );
/*M*/ 
/*M*/ 	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
/*M*/ 	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
/*M*/ 	if ( rOptions.Len() )
/*M*/ 		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, rOptions ) );
/*M*/ 
/*M*/ 	pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
/*M*/ 	if ( pMedium->GetError() != ERRCODE_NONE )
/*M*/ 		return ;
/*M*/ 
/*M*/ 	if ( bWithInteraction )
/*M*/ 		pMedium->UseInteractionHandler( TRUE );	// to enable the filter options dialog
/*M*/ 
/*M*/ 	pDocShell = new ScDocShell( SFX_CREATE_MODE_INTERNAL );
/*M*/ 	aRef = pDocShell;
/*M*/ 
/*M*/ 	ScDocument*	pDoc = pDocShell->GetDocument();
/*M*/ 	if( pDoc )
/*M*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScExtDocOptions*	pExtDocOpt = pDoc->GetExtDocOptions();
/*M*/ 	}
/*M*/ 
/*M*/ 	pDocShell->DoLoad( pMedium );
/*M*/ 
/*M*/ 	String aNew = GetOptions(*pMedium);			// Optionen werden beim Laden per Dialog gesetzt
/*M*/ 	if (aNew.Len() && aNew != rOptions)
/*M*/ 		rOptions = aNew;
/*M*/ }
/*N*/ 
/*N*/ ScDocumentLoader::~ScDocumentLoader()
/*N*/ {
/*	if ( pDocShell )
        pDocShell->DoClose();
*/
/*N*/ 	if ( aRef.Is() )
/*N*/ 		aRef->DoClose();
/*N*/ 	else if ( pMedium )
/*N*/ 		delete pMedium;
/*N*/ }

/*N*/ ScDocument* ScDocumentLoader::GetDocument()
/*N*/ {
/*N*/ 	return pDocShell ? pDocShell->GetDocument() : 0;
/*N*/ }

/*N*/ BOOL ScDocumentLoader::IsError() const
/*N*/ {
/*N*/ 	if ( pDocShell && pMedium )
/*N*/ 		return pMedium->GetError() != ERRCODE_NONE;
/*N*/ 	else
/*N*/ 		return TRUE;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
