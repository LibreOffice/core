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

// INCLUDE ---------------------------------------------------------

#include <bf_sfx2/app.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_svtools/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "arealink.hxx"

#include "tablink.hxx"
#include "docsh.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include "hints.hxx"

#include "attrib.hxx"			// raus, wenn ResetAttrib am Dokument
#include "patattr.hxx"			// raus, wenn ResetAttrib am Dokument
#include "docpool.hxx"			// raus, wenn ResetAttrib am Dokument
namespace binfilter {

/*N*/ TYPEINIT1(ScAreaLink,::binfilter::SvBaseLink);

//------------------------------------------------------------------------

/*N*/ ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const String& rFile,
/*N*/ 						const String& rFilter, const String& rOpt,
/*N*/ 						const String& rArea, const ScRange& rDest,
/*N*/ 						ULONG nRefresh ) :
/*N*/ 	::binfilter::SvBaseLink(::binfilter::LINKUPDATE_ONCALL,FORMAT_FILE),
/*N*/ 	ScRefreshTimer	( nRefresh ),
/*N*/ 	pDocShell		((ScDocShell*)pShell),
/*N*/ 	aFileName		(rFile),
/*N*/ 	aFilterName		(rFilter),
/*N*/ 	aOptions		(rOpt),
/*N*/ 	aSourceArea		(rArea),
/*N*/ 	aDestArea		(rDest),
/*N*/ 	bAddUndo		(TRUE),
/*N*/ 	bInCreate		(FALSE),
/*N*/ 	bDoInsert		(TRUE)
/*N*/ {
/*N*/ 	DBG_ASSERT(pShell->ISA(ScDocShell), "ScAreaLink mit falscher ObjectShell");
/*N*/ 	SetRefreshHandler( LINK( this, ScAreaLink, RefreshHdl ) );
/*N*/ 	SetRefreshControl( pDocShell->GetDocument()->GetRefreshTimerControlAddress() );
/*N*/ }

/*N*/ __EXPORT ScAreaLink::~ScAreaLink()
/*N*/ {
/*N*/ 	StopRefreshTimer();
/*N*/ }


/*N*/  void __EXPORT ScAreaLink::DataChanged( const String&,
/*N*/  									   const ::com::sun::star::uno::Any& )
/*N*/ {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 	//	bei bInCreate nichts tun, damit Update gerufen werden kann, um den Status im
/*N*/ }


/*N*/ void ScAreaLink::SetDestArea(const ScRange& rNew)
/*N*/ {
/*N*/ 	aDestArea = rNew;			// fuer Undo
/*N*/ }



// find a range with name >rAreaName< in >pSrcDoc<, return it in >rRange<
/*N*/ BOOL ScAreaLink::FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const String& rAreaName )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	ScRangeName* pNames = pSrcDoc->GetRangeName();
/*N*/ 	USHORT nPos;
/*N*/ 	if (pNames)			// benannte Bereiche
/*N*/ 	{
/*N*/ 		if (pNames->SearchName( rAreaName, nPos ))
/*N*/ 			if ( (*pNames)[nPos]->IsReference( rRange ) )
/*N*/ 				bFound = TRUE;
/*N*/ 	}
/*N*/ 	if (!bFound)		// Datenbankbereiche
/*N*/ 	{
/*N*/ 		ScDBCollection*	pDBColl = pSrcDoc->GetDBCollection();
/*N*/ 		if (pDBColl)
/*N*/ 			if (pDBColl->SearchName( rAreaName, nPos ))
/*N*/ 			{
/*N*/ 				USHORT nTab,nCol1,nRow1,nCol2,nRow2;
/*N*/ 				(*pDBColl)[nPos]->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
/*N*/ 				rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
/*N*/ 				bFound = TRUE;
/*N*/ 			}
/*N*/ 	}
/*N*/ 	if (!bFound)		// direct reference (range or cell)
/*N*/ 	{
/*N*/ 		if ( rRange.ParseAny( rAreaName, pSrcDoc ) & SCA_VALID )
/*N*/ 			bFound = TRUE;
/*N*/ 	}
/*N*/ 	return bFound;
/*N*/ }

//	ausfuehren:

/*N*/ BOOL ScAreaLink::Refresh( const String& rNewFile, const String& rNewFilter,
/*N*/ 							const String& rNewArea, ULONG nNewRefresh )
/*N*/ {
/*N*/ 	//	Dokument laden - wie TabLink
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
/*N*/ 
/*N*/ 	BOOL bUndo (pDoc->IsUndoEnabled());
/*N*/ 	pDoc->SetInLinkUpdate( TRUE );
/*N*/ 
/*N*/ 	//	wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
/*N*/ 	if ( rNewFilter != aFilterName )
/*N*/ 		aOptions.Erase();
/*N*/ 
/*N*/ 	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
/*N*/ 	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
/*N*/ 	if ( aOptions.Len() )
/*N*/ 		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
/*N*/ 
/*N*/ 	SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, FALSE, pFilter);
/*N*/ 
/*N*/ 	ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
/*N*/ 	SvEmbeddedObjectRef aRef = pSrcShell;
/*N*/ 	pSrcShell->DoLoad(pMed);
/*N*/ 
/*N*/ 	ScDocument* pSrcDoc = pSrcShell->GetDocument();
/*N*/ 
/*N*/ 	// Optionen koennten gesetzt worden sein
/*N*/ 	String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
/*N*/ 	if (!aNewOpt.Len())
/*N*/ 		aNewOpt = aOptions;
/*N*/ 
/*N*/ 	// correct source range name list for web query import
/*N*/ 	String aTempArea;
/*N*/ 
/*N*/ 	if( rNewFilter == ScDocShell::GetWebQueryFilterName() )
/*?*/			{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		aTempArea = ScHTMLImport::GetHTMLRangeNameList( pSrcDoc, rNewArea );
/*N*/ 	else
/*N*/ 		aTempArea = rNewArea;
/*N*/ 
/*N*/ 	// find total size of source area
/*N*/ 	USHORT nWidth = 0;
/*N*/ 	USHORT nHeight = 0;
/*N*/ 	xub_StrLen nTokenCnt = aTempArea.GetTokenCount( ';' );
/*N*/ 	xub_StrLen nStringIx = 0;
/*N*/ 	xub_StrLen nToken;
/*N*/ 
/*N*/ 	for( nToken = 0; nToken < nTokenCnt; nToken++ )
/*N*/ 	{
/*N*/ 		String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
/*N*/ 		ScRange aTokenRange;
/*N*/ 		if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
/*N*/ 		{
/*N*/ 			// columns: find maximum
/*N*/ 			nWidth = Max( nWidth, (USHORT)(aTokenRange.aEnd.Col() - aTokenRange.aStart.Col() + 1) );
/*N*/ 			// rows: add row range + 1 empty row
/*N*/ 			nHeight += aTokenRange.aEnd.Row() - aTokenRange.aStart.Row() + 2;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// remove the last empty row
/*N*/ 	if( nHeight )
/*N*/ 		nHeight--;
/*N*/ 
/*N*/ 	//	alte Daten loeschen / neue kopieren
/*N*/ 
/*N*/ 	ScAddress aDestPos = aDestArea.aStart;
/*N*/ 	USHORT nDestTab = aDestPos.Tab();
/*N*/ 	ScRange aOldRange = aDestArea;
/*N*/ 	ScRange aNewRange = aDestArea;			// alter Bereich, wenn Datei nicht gefunden o.ae.
/*N*/ 	if (nWidth && nHeight)
/*N*/ 	{
/*N*/ 		aNewRange.aEnd.SetCol( aNewRange.aStart.Col() + nWidth - 1 );
/*N*/ 		aNewRange.aEnd.SetRow( aNewRange.aStart.Row() + nHeight - 1 );
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bCanDo = pDoc->CanFitBlock( aOldRange, aNewRange );	//! nach bDoInsert unterscheiden
/*N*/ 	if (bCanDo)
/*N*/ 	{
/*N*/ 		ScDocShellModificator aModificator( *pDocShell );
/*N*/ 
/*N*/ 		USHORT nStartX = aDestPos.Col();
/*N*/ 		USHORT nStartY = aDestPos.Row();
/*N*/ 		USHORT nOldEndX = aOldRange.aEnd.Col();
/*N*/ 		USHORT nOldEndY = aOldRange.aEnd.Row();
/*N*/ 		USHORT nNewEndX = aNewRange.aEnd.Col();
/*N*/ 		USHORT nNewEndY = aNewRange.aEnd.Row();
/*N*/ 		ScRange aMaxRange( aDestPos,
/*N*/ 					ScAddress(Max(nOldEndX,nNewEndX), Max(nOldEndY,nNewEndY), nDestTab) );
/*N*/ 
/*N*/ 		//	Undo initialisieren
/*N*/ 
/*N*/ 		ScDocument* pUndoDoc = NULL;
/*N*/ 		ScDocument* pRedoDoc = NULL;
/*N*/ 		if ( bAddUndo && bUndo )
/*N*/ 		{
/*N*/ 			pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
/*N*/ 			if ( bDoInsert )
/*N*/ 			{
/*N*/ 				if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )				// Bereich veraendert?
/*N*/ 				{
/*N*/ 					pUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1 );
/*N*/ 					pDoc->CopyToDocument( 0,0,0,MAXCOL,MAXROW,MAXTAB,
/*N*/ 											IDF_FORMULA, FALSE, pUndoDoc );		// alle Formeln
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );				// nur Zieltabelle
/*N*/ 				pDoc->CopyToDocument( aOldRange, IDF_ALL, FALSE, pUndoDoc );
/*N*/ 			}
/*N*/ 			else		// ohne Einfuegen
/*N*/ 			{
/*N*/ 				pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );				// nur Zieltabelle
/*N*/ 				pDoc->CopyToDocument( aMaxRange, IDF_ALL, FALSE, pUndoDoc );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//	Zellen einfuegen / loeschen
/*N*/ 		//	DeleteAreaTab loescht auch MERGE_FLAG Attribute
/*N*/ 
/*N*/ 		if (bDoInsert)
/*N*/ 			pDoc->FitBlock( aOldRange, aNewRange );			// incl. loeschen
/*N*/ 		else
/*N*/ 			pDoc->DeleteAreaTab( aMaxRange, IDF_ALL );
/*N*/ 
/*N*/ 		//	Daten kopieren
/*N*/ 
/*N*/ 		if (nWidth && nHeight)
/*N*/ 		{
/*N*/ 			ScDocument aClipDoc( SCDOCMODE_CLIP );
/*N*/ 			ScRange aNewTokenRange( aNewRange.aStart );
/*N*/ 			nStringIx = 0;
/*N*/ 			for( nToken = 0; nToken < nTokenCnt; nToken++ )
/*N*/ 			{
/*N*/ 				String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
/*N*/ 				ScRange aTokenRange;
/*N*/ 				if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
/*N*/ 				{
/*N*/ 					USHORT nSrcTab = aTokenRange.aStart.Tab();
/*N*/ 					ScMarkData aSourceMark;
/*N*/ 					aSourceMark.SelectOneTable( nSrcTab );		// selektieren fuer CopyToClip
/*N*/ 					aSourceMark.SetMarkArea( aTokenRange );
/*N*/ 
/*N*/ 					pSrcDoc->CopyToClip( aTokenRange.aStart.Col(), aTokenRange.aStart.Row(),
/*N*/ 										 aTokenRange.aEnd.Col(), aTokenRange.aEnd.Row(),
/*N*/ 										 FALSE, &aClipDoc, FALSE, &aSourceMark );
/*N*/ 
/*N*/ 					if ( aClipDoc.HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
/*N*/ 											HASATTR_MERGED | HASATTR_OVERLAPPED ) )
/*N*/ 					{
/*N*/ 						//!	ResetAttrib am Dokument !!!
/*N*/ 
/*N*/ 						ScPatternAttr aPattern( pSrcDoc->GetPool() );
/*N*/ 						aPattern.GetItemSet().Put( ScMergeAttr() );				// Defaults
/*N*/ 						aPattern.GetItemSet().Put( ScMergeFlagAttr() );
/*N*/ 						aClipDoc.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
/*N*/ 					}
/*N*/ 
/*N*/ 					aNewTokenRange.aEnd.SetCol( aNewTokenRange.aStart.Col() + (aTokenRange.aEnd.Col() - aTokenRange.aStart.Col()) );
/*N*/ 					aNewTokenRange.aEnd.SetRow( aNewTokenRange.aStart.Row() + (aTokenRange.aEnd.Row() - aTokenRange.aStart.Row()) );
/*N*/ 					ScMarkData aDestMark;
/*N*/ 					aDestMark.SelectOneTable( nDestTab );
/*N*/ 					aDestMark.SetMarkArea( aNewTokenRange );
/*N*/ 					pDoc->CopyFromClip( aNewTokenRange, aDestMark, IDF_ALL, NULL, &aClipDoc, FALSE );
/*N*/ 					aNewTokenRange.aStart.SetRow( aNewTokenRange.aEnd.Row() + 2 );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aErr = ScGlobal::GetRscString(STR_LINKERROR);
/*N*/ 			pDoc->SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
/*N*/ 		}
/*N*/ 
/*N*/ 		//	Undo eintragen
/*N*/ 
/*N*/ 		if ( bAddUndo && bUndo)
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
/*N*/ 		}
/*N*/ 
/*N*/ 		//	neue Einstellungen merken
/*N*/ 
/*N*/ 		if ( bNewUrlName )
/*N*/ 			aFileName = aNewUrl;
/*N*/ 		if ( rNewFilter != aFilterName )
/*N*/ 			aFilterName = rNewFilter;
/*N*/ 		if ( rNewArea != aSourceArea )
/*N*/ 			aSourceArea = rNewArea;
/*N*/ 		if ( aNewOpt != aOptions )
/*N*/ 			aOptions = aNewOpt;
/*N*/ 
/*N*/ 		if ( aNewRange != aDestArea )
/*N*/ 			aDestArea = aNewRange;
/*N*/ 
/*N*/ 		if ( nNewRefresh != GetRefreshDelay() )
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SetRefreshDelay( nNewRefresh );
/*N*/ 
/*N*/ 		USHORT nPaintEndX = Max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
/*N*/ 		USHORT nPaintEndY = Max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );
/*N*/ 
/*N*/ 		if ( aOldRange.aEnd.Col() != aNewRange.aEnd.Col() )
/*N*/ 			nPaintEndX = MAXCOL;
/*N*/ 		if ( aOldRange.aEnd.Row() != aNewRange.aEnd.Row() )
/*N*/ 			nPaintEndY = MAXROW;
/*N*/ 
/*N*/ 		if ( !pDocShell->AdjustRowHeight( aDestPos.Row(), nPaintEndY, nDestTab ) )
/*N*/ 			pDocShell->PostPaint( aDestPos.Col(),aDestPos.Row(),nDestTab,
/*N*/ 									nPaintEndX,nPaintEndY,nDestTab, PAINT_GRID );
/*N*/ 		aModificator.SetDocumentModified();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	CanFitBlock FALSE -> Probleme mit zusammengefassten Zellen
/*N*/ 		//						 oder Tabellengrenze erreicht!
/*N*/ 		//!	Zellschutz ???
/*N*/ 
/*N*/ 		//!	Link-Dialog muss Default-Parent setzen
/*N*/ 		//	"kann keine Zeilen einfuegen"
/*N*/ 		InfoBox aBox( Application::GetDefDialogParent(),
/*N*/ 						ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_2 ) );
/*N*/ 		aBox.Execute();
/*N*/ 	}
/*N*/ 
/*N*/ 	//	aufraeumen
/*N*/ 
/*N*/ 	aRef->DoClose();
/*N*/ 
/*N*/ 	pDoc->SetInLinkUpdate( FALSE );
/*N*/ 
/*N*/ 	if (bCanDo)
/*N*/ 	{
/*N*/ 		//	notify Uno objects (for XRefreshListener)
/*N*/ 		//!	also notify Uno objects if file name was changed!
/*N*/ 		ScLinkRefreshedHint aHint;
/*N*/ 		aHint.SetAreaLink( aDestPos );
/*N*/ 		pDoc->BroadcastUno( aHint );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bCanDo;
/*N*/ }


/*N*/ IMPL_LINK( ScAreaLink, RefreshHdl, ScAreaLink*, pCaller )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 long nRes = Refresh( aFileName, aFilterName, aSourceArea,
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
