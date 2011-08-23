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

#ifdef WIN
// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
#define _BASEDLGS_HXX
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX ***

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX ***
//#define _SFXDOCINF_HXX
#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
#define _SFXDOCFILE_HXX
#define _VIEWFAC_HXX
#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX ***
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX

#endif	//WIN

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/langitem.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svtools/zforlist.hxx>
#include <vcl/svapp.hxx>
#include "document.hxx"
#include "attrib.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "docpool.hxx"
#include "poolhelp.hxx"
#include "refupdat.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "tablink.hxx"
#include "conditio.hxx"
#include "detdata.hxx"
#include "scmod.hxx"   		// SC_MOD
#include "inputopt.hxx" 	// GetExpandRefs
#include "bf_sc.hrc"			// SID_LINK
#include "hints.hxx"
#include "dpobject.hxx"

namespace binfilter {
using namespace ::com::sun::star;

//------------------------------------------------------------------------

/*N*/ ScRangeName* ScDocument::GetRangeName()
/*N*/ {
/*N*/ 	return pRangeName;
/*N*/ }

/*N*/ void ScDocument::SetRangeName( ScRangeName* pNewRangeName )
/*N*/ {
/*N*/ 	if (pRangeName)
/*N*/ 		delete pRangeName;
/*N*/ 	pRangeName = pNewRangeName;
/*N*/ }



/*N*/ ScDBCollection* ScDocument::GetDBCollection() const
/*N*/ {
/*N*/ 	return pDBCollection;
/*N*/ }


/*N*/ ScDBData* ScDocument::GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const
/*N*/ {
/*N*/ 	if (pDBCollection)
/*N*/ 		return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, bStartOnly);
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ ScDBData* ScDocument::GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const
/*N*/ {
/*N*/ 	if (pDBCollection)
/*N*/ 		return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ ScDPCollection* ScDocument::GetDPCollection()
/*N*/ {
/*N*/ 	if (!pDPCollection)
/*N*/ 		pDPCollection = new ScDPCollection(this);
/*N*/ 	return pDPCollection;
/*N*/ }








/*N*/ void ScDocument::SetScenario( USHORT nTab, BOOL bFlag )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetScenario(bFlag);
/*N*/ }

/*N*/ BOOL ScDocument::IsScenario( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->IsScenario();
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScDocument::SetScenarioData( USHORT nTab, const String& rComment,
/*N*/ 										const Color& rColor, USHORT nFlags )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab] && pTab[nTab]->IsScenario())
/*N*/ 	{
/*N*/ 		pTab[nTab]->SetScenarioComment( rComment );
/*N*/ 		pTab[nTab]->SetScenarioColor( rColor );
/*N*/ 		pTab[nTab]->SetScenarioFlags( nFlags );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::GetScenarioData( USHORT nTab, String& rComment,
/*N*/ 										Color& rColor, USHORT& rFlags ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab] && pTab[nTab]->IsScenario())
/*N*/ 	{
/*N*/ 		pTab[nTab]->GetScenarioComment( rComment );
/*N*/ 		rColor = pTab[nTab]->GetScenarioColor();
/*N*/ 		rFlags = pTab[nTab]->GetScenarioFlags();
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDocument::IsLinked( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->IsLinked();
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::GetLinkMode( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkMode();
/*N*/ 	return SC_LINK_NONE;
/*N*/ }

/*N*/ const String& ScDocument::GetLinkDoc( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkDoc();
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ const String& ScDocument::GetLinkFlt( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkFlt();
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ const String& ScDocument::GetLinkOpt( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkOpt();
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ const String& ScDocument::GetLinkTab( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkTab();
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ ULONG ScDocument::GetLinkRefreshDelay( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetLinkRefreshDelay();
/*N*/ 	return 0;
/*N*/ }

/*N*/ void ScDocument::SetLink( USHORT nTab, BYTE nMode, const String& rDoc,
/*N*/ 							const String& rFilter, const String& rOptions,
/*N*/ 							const String& rTabName, ULONG nRefreshDelay )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetLink( nMode, rDoc, rFilter, rOptions, rTabName, nRefreshDelay );
/*N*/ }

/*N*/ BOOL ScDocument::HasLink( const String& rDoc,
/*N*/ 							const String& rFilter, const String& rOptions ) const
/*N*/ {
/*N*/ 	USHORT nCount = GetTableCount();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		if (pTab[i]->IsLinked()
/*N*/ 				&& pTab[i]->GetLinkDoc() == rDoc
/*N*/ 				&& pTab[i]->GetLinkFlt() == rFilter
/*N*/ 				&& pTab[i]->GetLinkOpt() == rOptions)
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScDocument::LinkExternalTab( USHORT& rTab, const String& aDocTab,
/*N*/ 		const String& aFileName, const String& aTabName )
/*N*/ {
/*N*/ 	if ( IsClipboard() )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "LinkExternalTab in Clipboard" );
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	rTab = 0;
/*N*/ 	String	aFilterName;		// wird vom Loader gefuellt
/*N*/ 	String	aOptions;		// Filter-Optionen
/*N*/ 	ScDocumentLoader aLoader( aFileName, aFilterName, aOptions,
/*N*/ 		pExtDocOptions ? pExtDocOptions->nLinkCnt + 1 : 1 );
/*N*/ 	if ( aLoader.IsError() )
/*N*/ 		return FALSE;
/*N*/ 	ScDocument* pSrcDoc = aLoader.GetDocument();
/*N*/ 
/*N*/ 	//	Tabelle kopieren
/*N*/ 	USHORT nSrcTab;
/*N*/ 	if ( pSrcDoc->GetTable( aTabName, nSrcTab ) )
/*N*/ 	{
/*N*/ 		if ( !InsertTab( SC_TAB_APPEND, aDocTab, TRUE ) )
/*N*/ 		{
/*N*/ 			DBG_ERRORFILE("can't insert external document table");
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		rTab = GetTableCount() - 1;
/*N*/ 		// nicht neu einfuegen, nur Ergebnisse
/*N*/ 		TransferTab( pSrcDoc, nSrcTab, rTab, FALSE, TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ULONG nRefreshDelay = 0;
/*N*/ 
/*N*/ 	BOOL bWasThere = HasLink( aFileName, aFilterName, aOptions );
/*N*/ 	SetLink( rTab, SC_LINK_VALUE, aFileName, aFilterName, aOptions, aTabName, nRefreshDelay );
/*N*/ 	if ( !bWasThere )		// Link pro Quelldokument nur einmal eintragen
/*N*/ 	{
/*N*/ 		ScTableLink* pLink = new ScTableLink( pShell, aFileName, aFilterName, aOptions, nRefreshDelay );
/*N*/ 		pLink->SetInCreate( TRUE );
/*N*/ 		pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName,
/*N*/ 										&aFilterName );
/*N*/ 		pLink->Update();
/*N*/ 		pLink->SetInCreate( FALSE );
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ ScOutlineTable* ScDocument::GetOutlineTable( USHORT nTab, BOOL bCreate )
/*N*/ {
/*N*/ 	ScOutlineTable* pVal = NULL;
/*N*/ 
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 		{
/*N*/ 			pVal = pTab[nTab]->GetOutlineTable();
/*N*/ 			if (!pVal)
/*N*/ 				if (bCreate)
/*N*/ 				{
/*N*/ 					pTab[nTab]->StartOutlineTable();
/*N*/ 					pVal = pTab[nTab]->GetOutlineTable();
/*N*/ 				}
/*N*/ 		}
/*N*/ 
/*N*/ 	return pVal;
/*N*/ }







//	kopiert aus diesem Dokument die Zellen von Positionen, an denen in pPosDoc
//	auch Zellen stehen, nach pDestDoc





/*N*/ const ScRangeList* ScDocument::GetScenarioRanges( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetScenarioRanges();
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }

/*N*/ BOOL ScDocument::IsActiveScenario( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->IsActiveScenario();
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScDocument::SetActiveScenario( USHORT nTab, BOOL bActive )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->SetActiveScenario( bActive );
/*N*/ }


/*N*/ void ScDocument::AddUnoObject( SfxListener& rObject )
/*N*/ {
/*N*/ 	if (!pUnoBroadcaster)
/*N*/ 		pUnoBroadcaster = new SfxBroadcaster;
/*N*/ 
/*N*/ 	rObject.StartListening( *pUnoBroadcaster );
/*N*/ }

/*N*/ void ScDocument::RemoveUnoObject( SfxListener& rObject )
/*N*/ {
/*N*/ 	if (pUnoBroadcaster)
/*N*/ 	{
/*N*/ 		rObject.EndListening( *pUnoBroadcaster );
/*N*/ 
/*N*/ 		if ( bInUnoBroadcast )
/*N*/ 		{
/*?*/ 			//	#107294# Broadcasts from ScDocument::BroadcastUno are the only way that
/*?*/ 			//	uno object methods are called without holding a reference.
/*?*/ 			//
/*?*/ 			//	If RemoveUnoObject is called from an object dtor in the finalizer thread
/*?*/ 			//	while the main thread is calling BroadcastUno, the dtor thread must wait
/*?*/ 			//	(or the object's Notify might try to access a deleted object).
/*?*/ 			//	The SolarMutex can't be locked here because if a component is called from
/*?*/ 			//	a VCL event, the main thread has the SolarMutex locked all the time.
/*?*/ 			//
/*?*/ 			//	This check is done after calling EndListening, so a later BroadcastUno call
/*?*/ 			//	won't touch this object.
/*?*/ 
/*?*/ 			osl::SolarMutex& rSolarMutex = Application::GetSolarMutex();
/*?*/ 			if ( rSolarMutex.tryToAcquire() )
/*?*/ 			{
/*?*/ 				//	BroadcastUno is always called with the SolarMutex locked, so if it
/*?*/ 				//	can be acquired, this is within the same thread (should not happen)
/*?*/ 				DBG_ERRORFILE( "RemoveUnoObject called from BroadcastUno" );
/*?*/ 				rSolarMutex.release();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				//	let the thread that called BroadcastUno continue
/*?*/ 				while ( bInUnoBroadcast )
/*?*/ 				{
/*?*/ 					osl::Thread::yield();
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		DBG_ERROR("No Uno broadcaster");
/*N*/ }

/*N*/ void ScDocument::BroadcastUno( const SfxHint &rHint )
/*N*/ {
/*N*/ 	if (pUnoBroadcaster)
/*N*/ 	{
/*N*/ 		bInUnoBroadcast = TRUE;
/*N*/ 		pUnoBroadcaster->Broadcast( rHint );
/*N*/ 		bInUnoBroadcast = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 									USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 									USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 									short nDx, short nDy, short nDz,
/*N*/ 									ScDocument* pUndoDoc, BOOL bIncludeDraw )
/*N*/ {
/*N*/ 	PutInOrder( nCol1, nCol2 );
/*N*/ 	PutInOrder( nRow1, nRow2 );
/*N*/ 	PutInOrder( nTab1, nTab2 );
/*N*/ 	if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
/*N*/ 	{
/*N*/ 		BOOL bExpandRefsOld = IsExpandRefs();
/*N*/ 		if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
/*N*/ 			SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
/*N*/ 		USHORT i;
/*N*/ 		USHORT iMax;
/*N*/ 		if ( eUpdateRefMode == URM_COPY )
/*N*/ 		{
/*N*/ 			i = nTab1;
/*N*/ 			iMax = nTab2;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 			xColNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
/*N*/ 			xRowNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
/*N*/ 			pDBCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
/*N*/ 			pRangeName->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
/*N*/ 			if (pPivotCollection)
/*N*/ 				pPivotCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
/*N*/ 			if ( pDPCollection )
/*N*/ 				pDPCollection->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
/*N*/ 			UpdateChartRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
/*N*/ 			UpdateRefAreaLinks( eUpdateRefMode, aRange, nDx, nDy, nDz );
/*N*/ 			if ( pCondFormList )
/*N*/ 				pCondFormList->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
/*N*/ 			if ( pDetOpList )
/*N*/ 				pDetOpList->UpdateReference( this, eUpdateRefMode, aRange, nDx, nDy, nDz );
/*N*/ 			if ( pUnoBroadcaster )
/*N*/ 				pUnoBroadcaster->Broadcast( ScUpdateRefHint(
/*N*/ 									eUpdateRefMode, aRange, nDx, nDy, nDz ) );
/*N*/ 			i = 0;
/*N*/ 			iMax = MAXTAB;
/*N*/ 		}
/*N*/ 		for ( ; i<=iMax; i++)
/*N*/ 			if (pTab[i])
/*N*/ 				pTab[i]->UpdateReference(
/*N*/ 					eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
/*N*/ 					nDx, nDy, nDz, pUndoDoc, bIncludeDraw );
/*N*/ 
/*N*/ 		if ( bIsEmbedded )
/*N*/ 		{
/*N*/ 			USHORT theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
/*N*/ 			theCol1 = aEmbedRange.aStart.Col();
/*N*/ 			theRow1 = aEmbedRange.aStart.Row();
/*N*/ 			theTab1 = aEmbedRange.aStart.Tab();
/*N*/ 			theCol2 = aEmbedRange.aEnd.Col();
/*N*/ 			theRow2 = aEmbedRange.aEnd.Row();
/*N*/ 			theTab2 = aEmbedRange.aEnd.Tab();
/*N*/ 			if ( ScRefUpdate::Update( this, eUpdateRefMode, nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
/*N*/ 										nDx,nDy,nDz, theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
/*N*/ 			{
/*N*/ 				aEmbedRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		SetExpandRefs( bExpandRefsOld );
         }
/*N*/ }



/*N*/ void ScDocument::Fill(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, const ScMarkData& rMark,
/*N*/ 						USHORT nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
/*N*/ 						double nStepValue, double nMaxValue)
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 PutInOrder( nCol1, nCol2 );
/*N*/ }


/*N*/ void ScDocument::AutoFormat( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 									USHORT nFormatNo, const ScMarkData& rMark )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 PutInOrder( nStartCol, nEndCol );
/*N*/ }


//	Outline anpassen
 
/*N*/ BOOL ScDocument::UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, USHORT nTab, BOOL bShow )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->UpdateOutlineCol( nStartCol, nEndCol, bShow );
/*N*/ 
/*N*/ 	DBG_ERROR("missing tab");
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bShow )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->UpdateOutlineRow( nStartRow, nEndRow, bShow );
/*N*/ 
/*N*/ 	DBG_ERROR("missing tab");
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScDocument::Sort(USHORT nTab, const ScSortParam& rSortParam, BOOL bKeepQuery)
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ }

/*N*/ USHORT ScDocument::Query(USHORT nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub)
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 	return 0;
/*N*/ }





/*N*/ BOOL ScDocument::CreateQueryParam(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nTab, ScQueryParam& rQueryParam)
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pTab[nTab] )
/*N*/ 		return pTab[nTab]->CreateQueryParam(nCol1, nRow1, nCol2, nRow2, rQueryParam);
/*N*/ 
/*N*/ 	DBG_ERROR("missing tab");
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDocument::HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 									USHORT nTab )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (VALIDTAB(nTab))
/*N*/ 	return FALSE;
/*N*/ }

//	Funktionen werden als 1 schon vom InputHandler eingefuegt
#define SC_STRTYPE_NAMES		2
#define SC_STRTYPE_DBNAMES		3
#define SC_STRTYPE_HEADERS		4



/*N*/ BOOL ScDocument::IsEmbedded() const
/*N*/ {
/*N*/ 	return bIsEmbedded;
/*N*/ }





/*N*/ ScRange ScDocument::GetRange( USHORT nTab, const Rectangle& rMMRect )
/*N*/ {
/*N*/ 	ScTable* pTable = pTab[nTab];
/*N*/ 	if (!pTable)
/*N*/ 	{
/*N*/ 		DBG_ERROR("GetRange ohne Tabelle");
/*N*/ 		return ScRange();
/*N*/ 	}
/*N*/ 
/*N*/ 	long nSize;
/*N*/ 	long nTwips;
/*N*/ 	long nAdd;
/*N*/ 	BOOL bEnd;
/*N*/ 
/*N*/ 	nSize = 0;
/*N*/ 	nTwips = (long) (rMMRect.Left() / HMM_PER_TWIPS);
/*N*/ 
/*N*/ 	USHORT nX1 = 0;
/*N*/ 	bEnd = FALSE;
/*N*/ 	while (!bEnd)
/*N*/ 	{
/*N*/ 		nAdd = (long) pTable->GetColWidth(nX1);
/*N*/ 		if (nSize+nAdd <= nTwips+1 && nX1<MAXCOL)
/*N*/ 		{
/*N*/ 			nSize += nAdd;
/*N*/ 			++nX1;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bEnd = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	nTwips = (long) (rMMRect.Right() / HMM_PER_TWIPS);
/*N*/ 
/*N*/ 	USHORT nX2 = nX1;
/*N*/ 	bEnd = FALSE;
/*N*/ 	while (!bEnd)
/*N*/ 	{
/*N*/ 		nAdd = (long) pTable->GetColWidth(nX2);
/*N*/ 		if (nSize+nAdd < nTwips && nX2<MAXCOL)
/*N*/ 		{
/*N*/ 			nSize += nAdd;
/*N*/ 			++nX2;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bEnd = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	nSize = 0;
/*N*/ 	nTwips = (long) (rMMRect.Top() / HMM_PER_TWIPS);
/*N*/ 
/*N*/ 	USHORT nY1 = 0;
/*N*/ 	bEnd = FALSE;
/*N*/ 	while (!bEnd)
/*N*/ 	{
/*N*/ 		nAdd = (long) pTable->GetRowHeight(nY1);
/*N*/ 		if (nSize+nAdd <= nTwips+1 && nY1<MAXROW)
/*N*/ 		{
/*N*/ 			nSize += nAdd;
/*N*/ 			++nY1;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bEnd = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	nTwips = (long) (rMMRect.Bottom() / HMM_PER_TWIPS);
/*N*/ 
/*N*/ 	USHORT nY2 = nY1;
/*N*/ 	bEnd = FALSE;
/*N*/ 	while (!bEnd)
/*N*/ 	{
/*N*/ 		nAdd = (long) pTable->GetRowHeight(nY2);
/*N*/ 		if (nSize+nAdd < nTwips && nY2<MAXROW)
/*N*/ 		{
/*N*/ 			nSize += nAdd;
/*N*/ 			++nY2;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bEnd = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return ScRange( nX1,nY1,nTab, nX2,nY2,nTab );
/*N*/ }


//	VisArea auf Zellgrenzen anpassen

/*N*/ void lcl_SnapHor( ScTable* pTable, long& rVal, USHORT& rStartCol )
/*N*/ {
/*N*/ 	USHORT nCol = 0;
/*N*/ 	long nTwips = (long) (rVal / HMM_PER_TWIPS);
/*N*/ 	long nSnap = 0;
/*N*/ 	while ( nCol<MAXCOL )
/*N*/ 	{
/*N*/ 		long nAdd = pTable->GetColWidth(nCol);
/*N*/ 		if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
/*N*/ 		{
/*N*/ 			nSnap += nAdd;
/*N*/ 			++nCol;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	rVal = (long) ( nSnap * HMM_PER_TWIPS );
/*N*/ 	rStartCol = nCol;
/*N*/ }

/*N*/ void lcl_SnapVer( ScTable* pTable, long& rVal, USHORT& rStartRow )
/*N*/ {
/*N*/ 	USHORT nRow = 0;
/*N*/ 	long nTwips = (long) (rVal / HMM_PER_TWIPS);
/*N*/ 	long nSnap = 0;
/*N*/ 	while ( nRow<MAXROW )
/*N*/ 	{
/*N*/ 		long nAdd = pTable->GetRowHeight(nRow);
/*N*/ 		if ( nSnap + nAdd/2 < nTwips || nRow < rStartRow )
/*N*/ 		{
/*N*/ 			nSnap += nAdd;
/*N*/ 			++nRow;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	rVal = (long) ( nSnap * HMM_PER_TWIPS );
/*N*/ 	rStartRow = nRow;
/*N*/ }

/*N*/ void ScDocument::SnapVisArea( Rectangle& rRect ) const
/*N*/ {
/*N*/ 	ScTable* pTable = pTab[nVisibleTab];
/*N*/ 	if (!pTable)
/*N*/ 	{
/*N*/ 		DBG_ERROR("SetEmbedded ohne Tabelle");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nCol = 0;
/*N*/ 	lcl_SnapHor( pTable, rRect.Left(), nCol );
/*N*/ 	++nCol;											// mindestens eine Spalte
/*N*/ 	lcl_SnapHor( pTable, rRect.Right(), nCol );
/*N*/ 
/*N*/ 	USHORT nRow = 0;
/*N*/ 	lcl_SnapVer( pTable, rRect.Top(), nRow );
/*N*/ 	++nRow;											// mindestens eine Zeile
/*N*/ 	lcl_SnapVer( pTable, rRect.Bottom(), nRow );
/*N*/ }

/*N*/ void ScDocument::SetDocProtection( BOOL bProtect, const uno::Sequence<sal_Int8>& rPasswd )
/*N*/ {
/*N*/ 	bProtected = bProtect;
/*N*/ 	aProtectPass = rPasswd;
/*N*/ }

/*N*/ void ScDocument::SetTabProtection( USHORT nTab, BOOL bProtect, const uno::Sequence<sal_Int8>& rPasswd )
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			pTab[nTab]->SetProtection( bProtect, rPasswd );
/*N*/ }

/*N*/ BOOL ScDocument::IsDocProtected() const
/*N*/ {
/*N*/ 	return bProtected;
/*N*/ }

/*N*/ BOOL ScDocument::IsDocEditable() const
/*N*/ {
/*N*/ 	// import into read-only document is possible - must be extended if other filters use api
/*N*/ 
/*N*/ 	return !bProtected && ( !pShell || !pShell->IsReadOnly() || bImportingXML );
/*N*/ }

/*N*/ BOOL ScDocument::IsTabProtected( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->IsProtected();
/*N*/ 
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ const uno::Sequence<sal_Int8>& ScDocument::GetDocPassword() const
/*N*/ {
/*N*/ 	return aProtectPass;
/*N*/ }

/*N*/ const uno::Sequence<sal_Int8>& ScDocument::GetTabPassword( USHORT nTab ) const
/*N*/ {
/*N*/ 	if (VALIDTAB(nTab))
/*N*/ 		if (pTab[nTab])
/*N*/ 			return pTab[nTab]->GetPassword();
/*N*/ 
/*N*/ 	DBG_ERROR("Falsche Tabellennummer");
/*N*/ 	return aProtectPass;
/*N*/ }

/*N*/ const ScDocOptions& ScDocument::GetDocOptions() const
/*N*/ {
/*N*/ 	DBG_ASSERT( pDocOptions, "No DocOptions! :-(" );
/*N*/ 	return *pDocOptions;
/*N*/ }

/*N*/ void ScDocument::SetDocOptions( const ScDocOptions& rOpt )
/*N*/ {
/*N*/ 	USHORT d,m,y;
/*N*/ 
/*N*/ 	DBG_ASSERT( pDocOptions, "No DocOptions! :-(" );
/*N*/ 	*pDocOptions = rOpt;
/*N*/ 	rOpt.GetDate( d,m,y );
/*N*/ 
/*N*/ 	SvNumberFormatter* pFormatter = xPoolHelper->GetFormTable();
/*N*/ 	pFormatter->ChangeNullDate( d,m,y );
/*N*/ 	pFormatter->ChangeStandardPrec( (USHORT)rOpt.GetStdPrecision() );
/*N*/ 	pFormatter->SetYear2000( rOpt.GetYear2000() );
/*N*/ }

/*N*/ const ScViewOptions& ScDocument::GetViewOptions() const
/*N*/ {
/*N*/ 	DBG_ASSERT( pViewOptions, "No ViewOptions! :-(" );
/*N*/ 	return *pViewOptions;
/*N*/ }

/*N*/ void ScDocument::SetViewOptions( const ScViewOptions& rOpt )
/*N*/ {
/*N*/ 	DBG_ASSERT( pViewOptions, "No ViewOptions! :-(" );
/*N*/ 	*pViewOptions = rOpt;
/*N*/ }

/*N*/ void ScDocument::GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const
/*N*/ {
/*N*/ 	rLatin = eLanguage;
/*N*/ 	rCjk = eCjkLanguage;
/*N*/ 	rCtl = eCtlLanguage;
/*N*/ }

/*N*/ void ScDocument::SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl )
/*N*/ {
/*N*/ 	eLanguage = eLatin;
/*N*/ 	eCjkLanguage = eCjk;
/*N*/ 	eCtlLanguage = eCtl;
/*N*/ 	if ( xPoolHelper.is() )
/*N*/ 	{
/*N*/ 		ScDocumentPool* pPool = xPoolHelper->GetDocPool();
/*N*/ 		pPool->SetPoolDefaultItem( SvxLanguageItem( eLanguage, ATTR_FONT_LANGUAGE ) );
/*N*/ 		pPool->SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, ATTR_CJK_FONT_LANGUAGE ) );
/*N*/ 		pPool->SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, ATTR_CTL_FONT_LANGUAGE ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	UpdateDrawLanguages();		// set edit engine defaults in drawing layer pool
/*N*/ }

/*N*/ Rectangle ScDocument::GetMMRect( USHORT nStartCol, USHORT nStartRow,
/*N*/ 								USHORT nEndCol, USHORT nEndRow, USHORT nTab )
/*N*/ {
/*N*/ 	if (nTab > MAXTAB || !pTab[nTab])
/*N*/ 	{
/*N*/ 		DBG_ERROR("GetMMRect: falsche Tabelle");
/*N*/ 		return Rectangle(0,0,0,0);
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	Rectangle aRect;
/*N*/ 
/*N*/ 	for (i=0; i<nStartCol; i++)
/*N*/ 		aRect.Left() += GetColWidth(i,nTab);
/*N*/ 	for (i=0; i<nStartRow; i++)
/*N*/ 		aRect.Top() += FastGetRowHeight(i,nTab);
/*N*/ 
/*N*/ 	aRect.Right()  = aRect.Left();
/*N*/ 	aRect.Bottom() = aRect.Top();
/*N*/ 
/*N*/ 	for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 		aRect.Right() += GetColWidth(i,nTab);
/*N*/ 	for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 		aRect.Bottom() += FastGetRowHeight(i,nTab);
/*N*/ 
/*N*/ 	aRect.Left()	= (long)(aRect.Left()	* HMM_PER_TWIPS);
/*N*/ 	aRect.Right()	= (long)(aRect.Right()	* HMM_PER_TWIPS);
/*N*/ 	aRect.Top()		= (long)(aRect.Top()	* HMM_PER_TWIPS);
/*N*/ 	aRect.Bottom()	= (long)(aRect.Bottom()	* HMM_PER_TWIPS);
/*N*/ 
/*N*/ 	return aRect;
/*N*/ }

/*N*/ void ScDocument::DoMerge( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
/*N*/ 									USHORT nEndCol, USHORT nEndRow )
/*N*/ {
/*N*/ 	ScMergeAttr aAttr( nEndCol-nStartCol+1, nEndRow-nStartRow+1 );
/*N*/ 	ApplyAttr( nStartCol, nStartRow, nTab, aAttr );
/*N*/ 
/*N*/ 	if ( nEndCol > nStartCol )
/*N*/ 		ApplyFlagsTab( nStartCol+1, nStartRow, nEndCol, nStartRow, nTab, SC_MF_HOR );
/*N*/ 	if ( nEndRow > nStartRow )
/*N*/ 		ApplyFlagsTab( nStartCol, nStartRow+1, nStartCol, nEndRow, nTab, SC_MF_VER );
/*N*/ 	if ( nEndCol > nStartCol && nEndRow > nStartRow )
/*N*/ 		ApplyFlagsTab( nStartCol+1, nStartRow+1, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );
/*N*/ }


/*N*/ void ScDocument::ExtendPrintArea( OutputDevice* pDev, USHORT nTab,
/*N*/ 					USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->ExtendPrintArea( pDev, nStartCol, nStartRow, rEndCol, nEndRow );
/*N*/ }

/*N*/ void ScDocument::IncSizeRecalcLevel( USHORT nTab )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->IncRecalcLevel();
/*N*/ }

/*N*/ void ScDocument::DecSizeRecalcLevel( USHORT nTab )
/*N*/ {
/*N*/ 	if ( nTab <= MAXTAB && pTab[nTab] )
/*N*/ 		pTab[nTab]->DecRecalcLevel();
/*N*/ }




} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
