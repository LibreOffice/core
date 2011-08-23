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


#define ROLBCK_HISTORY_ONLY 	// Der Kampf gegen die CLOOK's

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HDL_
#include <com/sun/star/i18n/ForbiddenCharacters.hdl>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <bf_sfx2/docinf.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <bf_svtools/macitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef _SVXLINKMGR_HXX
#include <bf_svx/linkmgr.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <bf_svtools/zforlist.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <bf_svx/forbiddencharacterstable.hxx>
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFORDR_HXX
#include <fmtfordr.hxx>
#endif
#ifndef _PVPRTDAT_HXX
#include <pvprtdat.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#endif
#ifndef _LAYOUTER_HXX
#include <layouter.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SW_PRINTDATA_HXX
#include <printdata.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>          // fuer die Pool-Vorlage
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VISITURL_HXX
#include <visiturl.hxx>			// fuer die URL-Change Benachrichtigung
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _EXTINPUT_HXX
#include <extinput.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _UNOCLBCK_HXX
#include <unoclbck.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _LAYCACHE_HXX
#include <laycache.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>              // fuer den dflt - Printer in SetJob
#endif

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif

namespace binfilter {
const sal_Char __FAR_DATA sFrmFmtStr[] = "Frameformat";
const sal_Char __FAR_DATA sEmptyPageStr[] = "Empty Page";
const sal_Char __FAR_DATA sColumnCntStr[] = "Columncontainer";
const sal_Char __FAR_DATA sCharFmtStr[] = "Zeichenformat";
const sal_Char __FAR_DATA sTxtCollStr[] = "Textformatvorlage";
const sal_Char __FAR_DATA sGrfCollStr[] = "Graphikformatvorlage";

/*N*/ SV_IMPL_PTRARR( SwNumRuleTbl, SwNumRulePtr)
/*N*/ SV_IMPL_PTRARR( SwTxtFmtColls, SwTxtFmtCollPtr)
/*N*/ SV_IMPL_PTRARR( SwGrfFmtColls, SwGrfFmtCollPtr)


/*
 * interne Funktionen
 */

/*N*/ BOOL lcl_DelFmtIndizes( const SwFrmFmtPtr& rpFmt, void* )
/*N*/ {
/*N*/ 	SwFmtCntnt &rFmtCntnt = (SwFmtCntnt&)rpFmt->GetCntnt();
/*N*/ 	if ( rFmtCntnt.GetCntntIdx() )
/*N*/ 		rFmtCntnt.SetNewCntntIdx( 0 );
/*N*/ 	SwFmtAnchor &rFmtAnchor = (SwFmtAnchor&)rpFmt->GetAnchor();
/*N*/ 	if ( rFmtAnchor.GetCntntAnchor() )
/*N*/ 		rFmtAnchor.SetAnchor( 0 );
/*N*/ 	return TRUE;
/*N*/ }

/*
 * exportierte Methoden
 */

/*N*/ SwDoc::SwDoc() :
/*N*/ 	aAttrPool( this ),
/*N*/ 	aNodes( this ),
/*N*/ 	pFrmFmtTbl( new SwFrmFmts() ),
/*N*/ 	pCharFmtTbl( new SwCharFmts() ),
/*N*/ 	pSpzFrmFmtTbl( new SwSpzFrmFmts() ),
/*N*/ 	pTblFrmFmtTbl( new SwFrmFmts() ),
/*N*/ 	pDfltFrmFmt( new SwFrmFmt( aAttrPool, sFrmFmtStr, 0 ) ),
/*N*/ 	pEmptyPageFmt( new SwFrmFmt( aAttrPool, sEmptyPageStr, pDfltFrmFmt ) ),
/*N*/ 	pColumnContFmt( new SwFrmFmt( aAttrPool, sColumnCntStr, pDfltFrmFmt ) ),
/*N*/ 	pDfltCharFmt( new SwCharFmt( aAttrPool, sCharFmtStr, 0 ) ),
/*N*/ 	pDfltTxtFmtColl( new SwTxtFmtColl( aAttrPool, sTxtCollStr ) ),
/*N*/ 	pTxtFmtCollTbl( new SwTxtFmtColls() ),
/*N*/ 	pDfltGrfFmtColl( new SwGrfFmtColl( aAttrPool, sGrfCollStr ) ),
/*N*/ 	pGrfFmtCollTbl( new SwGrfFmtColls() ),
/*N*/ 	pSectionFmtTbl( new SwSectionFmts() ),
/*N*/ 	pFldTypes( new SwFldTypes() ),
/*N*/ 	pBookmarkTbl( new SwBookmarks( 0, 16 ) ),
/*N*/ 	pTOXTypes( new SwTOXTypes() ),
/*N*/ 	pDefTOXBases( new SwDefTOXBase_Impl() ),
/*N*/ 	nLinkCt( 0 ),
/*N*/ 	pGlossaryDoc( 0 ),
/*N*/ 	pOutlineRule( 0 ),
/*N*/ 	pLayout( 0 ),					// Rootframe des spezifischen Layouts.
/*N*/ 	pPrt( 0 ),
/*N*/     pPrtData( 0 ),
/*N*/ 	pExtInputRing( 0 ),
/*N*/ 	pLayouter( 0 ),
/*N*/     pLayoutCache( 0 ),
/*N*/ 	nLockExpFld( 0 ),
/*N*/ 	pDocShell( 0 ),
/*N*/ 	pDrawModel( 0 ),
/*N*/ 	pUpdtFlds( new SwDocUpdtFld() ),
/*N*/ 	pLinkMgr( new SvxLinkManager( 0 ) ),
/*N*/ 	pSwgInfo( 0 ),
/*N*/ 	pDocShRef( 0 ),
/*N*/ 	pACEWord( 0 ),
/*N*/ 	pURLStateChgd( 0 ),
/*N*/ 	pNumberFormatter( 0 ),
/*N*/ 	pFtnInfo( new SwFtnInfo ),
/*N*/ 	pEndNoteInfo( new SwEndNoteInfo ),
/*N*/ 	pLineNumberInfo( new SwLineNumberInfo ),
/*N*/ 	pFtnIdxs( new SwFtnIdxs ),
/*N*/ 	pDocStat( new SwDocStat ),
/*N*/ 	pNumRuleTbl( new SwNumRuleTbl ),
/*N*/ 	eRedlineMode( SwRedlineMode(REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) ),
/*N*/ 	pRedlineTbl( new SwRedlineTbl ),
/*N*/ 	pUnoCrsrTbl( new SwUnoCrsrTbl( 0, 16 ) ),
/*N*/ 	pPgPViewPrtData( 0 ),
/*N*/ 	pAutoFmtRedlnComment( 0 ),
/*N*/ 	pUnoCallBack(new SwUnoCallBack(0)),
/*N*/ 	nAutoFmtRedlnCommentNo( 0 ),
/*N*/ 	eChrCmprType( CHARCOMPRESS_NONE ),
/*N*/     n32Dummy1( 0 ), n32Dummy2( 0 ), n8Dummy1( 0x80 ), n8Dummy2( 0 ),
/*N*/ 	nLinkUpdMode( GLOBALSETTING ),
/*N*/ 	nFldUpdMode( AUTOUPD_GLOBALSETTING ),
/*N*/ 	bReadlineChecked(sal_False)
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::SwDoc" );
/*N*/
/*N*/ 	bGlossDoc =
/*N*/ 	bModified =
/*N*/ 	bDtor =
/*N*/ 	bPageNums =
/*N*/ 	bLoaded =
/*N*/ 	bUpdateExpFld =
/*N*/ 	bNewDoc =
/*N*/ 	bCopyIsMove =
/*N*/ 	bBrowseMode =
/*N*/ 	bInReading =
/*N*/ 	bUpdateTOX =
/*N*/ 	bInLoadAsynchron =
/*N*/ 	bHTMLMode =
/*N*/ 	bHeadInBrowse =
/*N*/ 	bFootInBrowse =
/*N*/ 	bInCallModified =
/*N*/ 	bIsGlobalDoc =
/*N*/ 	bGlblDocSaveLinks =
/*N*/ 	bIsLabelDoc =
/*N*/ 	bIsAutoFmtRedline =
/*N*/ 	bOLEPrtNotifyPending =
/*N*/ 	bAllOLENotify =
/*N*/ 	bIsRedlineMove =
/*N*/ 	bInsOnlyTxtGlssry =
/*N*/ 	bContains_MSVBasic =
/*N*/ 	bKernAsianPunctuation =
/*N*/ #ifdef DBG_UTIL
/*N*/ 	bXMLExport =
/*N*/ #endif
/*N*/ 								FALSE;
/*N*/
/*N*/ 	bNewFldLst =
/*N*/ 	bVisibleLinks =
/*N*/ 	bFrmBeepEnabled =
/*N*/ 	bPurgeOLE =
/*N*/ 								TRUE;
/*N*/
/*N*/ 	pMacroTable = new SvxMacroTableDtor;
/*N*/
    /*
     * Builds and sets the virtual device
     */
/*N*/     pVirDev = new VirtualDevice( 1 );
/*N*/     pVirDev->SetReferenceDevice(VirtualDevice::REFDEV_MODE_MSO1);
/*N*/     MapMode aMapMode( pVirDev->GetMapMode() );
/*N*/     aMapMode.SetMapUnit( MAP_TWIP );
/*N*/     pVirDev->SetMapMode( aMapMode );

    /*
     * Defaultformate und DefaultFormatsammlungen (FmtColl)
     * werden an der Position 0 in das jeweilige Array eingetragen.
     * Die Formate der FmtColls sind von den Defaultformaten
      * abgeleitet und stehen auch in der Liste.
     */
/*N*/ 	/* Formate */
/*N*/ 	pFrmFmtTbl->Insert(pDfltFrmFmt, 0 );
/*N*/ 	pCharFmtTbl->Insert(pDfltCharFmt, 0 );
/*N*/
/*N*/ 	/* FmtColls */
/*N*/ 	// TXT
/*N*/ 	pTxtFmtCollTbl->Insert(pDfltTxtFmtColl, 0 );
/*N*/ 	// aFtnInfo.SetFtnTxtColl(aDfltTxtFmtColl); // jetzt oben in der Liste
/*N*/ 	// GRF
/*N*/ 	pGrfFmtCollTbl->Insert(pDfltGrfFmtColl, 0 );
/*N*/
/*N*/ 	// PageDesc, EmptyPageFmt und ColumnFmt anlegen
/*N*/ 	if ( !aPageDescs.Count() )
/*N*/ 		GetPageDescFromPool( RES_POOLPAGE_STANDARD );
/*N*/
/*N*/ 		//Leere Seite Einstellen.
/*N*/ 	pEmptyPageFmt->SetAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
/*N*/ 		//BodyFmt fuer Spalten Einstellen.
/*N*/ 	pColumnContFmt->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
/*N*/
/*N*/ 	_InitFieldTypes();
/*N*/
/*N*/ 	// lege (fuer die Filter) eine Default-OullineNumRule an
/*N*/ 	pOutlineRule = new SwNumRule( String::CreateFromAscii(
/*N*/ 										SwNumRule::GetOutlineRuleName() ),
/*N*/ 									OUTLINE_RULE );
/*N*/
/*N*/   // new SwTxtNode( SwNodeIndex( aUndoNodes.GetEndOfContent() ), pDfltTxtFmtColl );
/*N*/ 	new SwTxtNode( SwNodeIndex( aNodes.GetEndOfContent() ),
/*N*/ 					GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
/*N*/
/*N*/ 	// den eigenen IdleTimer setzen
/*N*/ 	aIdleTimer.SetTimeout( 600 );
/*N*/ 	aIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );
/*N*/ 	aIdleTimer.Start();
/*N*/
/*N*/ 	// den CharTimer setzen
/*N*/ 	aChartTimer.SetTimeout( 2000 );
/*N*/ 	aChartTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateAllCharts ));
/*N*/
/*N*/ 	aOLEModifiedTimer.SetTimeout( 1000 );
/*N*/ 	aOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));
/*N*/
/*N*/ 	// DBMgr anlegen
/*N*/ 	pNewDBMgr = new SwNewDBMgr;
/*N*/
/*N*/ 	// create TOXTypes
/*N*/ 	ShellResource* pShellRes = ViewShell::GetShellRes();
/*N*/
/*N*/ 	SwTOXType * pNew = new SwTOXType(TOX_CONTENT, 	pShellRes->aTOXContentName		  );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_INDEX, 				pShellRes->aTOXIndexName  );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_USER, 					pShellRes->aTOXUserName  );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_ILLUSTRATIONS, 		pShellRes->aTOXIllustrationsName );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_OBJECTS, 				pShellRes->aTOXObjectsName       );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_TABLES, 				pShellRes->aTOXTablesName        );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/ 	pNew = new SwTOXType(TOX_AUTHORITIES, 			pShellRes->aTOXAuthoritiesName   );
/*N*/ 	pTOXTypes->Insert( pNew, pTOXTypes->Count() );
/*N*/
/*N*/ 	ResetModified();
/*N*/ }

/*
 * Besonderheiten: an der Position 0 des Arrays der Formate und
 * der GDI-Objekte befindet sich ein Member der Klasse SwDoc.
 * Dieser darf also keinesfalls durch delete geloescht
 * werden!!!!!!!!!!
 */


/*N*/ SwDoc::~SwDoc()
/*N*/ {
/*N*/     //!! needs to be done to destroy a possible SwFmtDrop format that may
/*N*/     //!! be connected to a char format which may not otherwise be removed
/*N*/     //!! and thus would leave a unremoved SwFmt object. (TL)
/*N*/     //!! (this is case is not possible via UI but via API...)
/*N*/     SwFmtDrop aDrop;
/*N*/     SetDefault(aDrop);
/*N*/     //!! same for SwFmtCharFmt
/*N*/     SwFmtCharFmt aCharFmt(NULL);
/*N*/     SetDefault(aCharFmt);
/*N*/
/*N*/ 	aIdleTimer.Stop();	// den Idltimer abschalten
/*N*/
/*N*/ 	delete pUnoCallBack, pUnoCallBack = 0;
/*N*/ 	delete pURLStateChgd;
/*N*/
/*N*/ 	delete pLayouter;
/*N*/
/*N*/ 	// Undo-Benachrichtigung vom Draw abschalten
/*N*/ 	if( pDrawModel )
/*N*/ 	{
/*N*/ 		ClrContourCache();
/*N*/ 	}
/*N*/
/*N*/ 	delete pPgPViewPrtData;
/*N*/
/*N*/ 	// damit die Fussnotenattribute die Fussnotennodes in Frieden lassen.
/*N*/ 	bDtor = TRUE;
/*N*/
/*N*/ 	DELETEZ( pLayout );
/*N*/ 	DELETEZ( pOutlineRule );
/*N*/
/*N*/ 	delete pRedlineTbl;
/*N*/ 	delete pUnoCrsrTbl;
/*N*/ 	delete pAutoFmtRedlnComment;
/*N*/
/*N*/ 	if( pUpdtFlds )
/*N*/ 		delete pUpdtFlds;
/*N*/
/*N*/ 	// die BaseLinks freigeben.
/*N*/ 	{
/*N*/ 		for( USHORT n = pLinkMgr->GetServers().Count(); n; )
/*N*/ 			pLinkMgr->GetServers()[ --n ]->Closed();
/*N*/
/*N*/ 		if( pLinkMgr->GetLinks().Count() )
/*N*/ 			pLinkMgr->Remove( 0, pLinkMgr->GetLinks().Count() );
/*N*/ 	}
/*N*/
/*N*/ 	// die KapitelNummern / Nummern muessen vor den Vorlage geloescht werden
/*N*/ 	// ansonsten wird noch staendig geupdatet !!!
/*N*/ 	aNodes.pOutlineNds->Remove( USHORT(0), aNodes.pOutlineNds->Count() );
/*N*/   // aUndoNodes.pOutlineNds->Remove( USHORT(0), aUndoNodes.pOutlineNds->Count() );
/*N*/
/*N*/ 	pFtnIdxs->Remove( USHORT(0), pFtnIdxs->Count() );
/*N*/
/*N*/ 	// in den BookMarks sind Indizies auf den Content. Diese muessen vorm
/*N*/ 	// loesche der Nodes geloescht werden.
/*N*/ 	pBookmarkTbl->DeleteAndDestroy( 0, pBookmarkTbl->Count() );
/*N*/ 	DELETEZ( pMacroTable );
/*N*/
/*N*/ 	if( pExtInputRing )
/*N*/ 	{
/*?*/ 		Ring* pTmp = pExtInputRing;
/*?*/ 		pExtInputRing = 0;
/*?*/ 		while( pTmp->GetNext() != pTmp )
/*?*/ 			delete pTmp->GetNext();
/*?*/ 		delete pTmp;
/*N*/ 	}

//JP: alt - loeschen ohne Flag ist teuer; Modify wird verschickt!
//	aTOXTypes.DeleteAndDestroy( 0, aTOXTypes.Count() );
    {
/*N*/ 		for( USHORT n = pTOXTypes->Count(); n; )
/*N*/ 		{
/*N*/ 			(*pTOXTypes)[ --n ]->SetInDocDTOR();
/*N*/ 			delete (*pTOXTypes)[ n ];
/*N*/ 		}
/*N*/ 		pTOXTypes->Remove( 0, pTOXTypes->Count() );
/*N*/ 	}
/*N*/ 	delete pDefTOXBases;

    //Im einen oder anderen FrmFormat koennen noch Indizes angemeldet sein,
    //Diese muessen spaetestens jetzt zerstoert werden.
/*N*/ 	pFrmFmtTbl->ForEach( &lcl_DelFmtIndizes, this );
/*N*/ 	pSpzFrmFmtTbl->ForEach( &lcl_DelFmtIndizes, this );
/*N*/ 	((SwFrmFmts&)*pSectionFmtTbl).ForEach( &lcl_DelFmtIndizes, this );
/*N*/
/*N*/ 	//Die Formate, die hier hinter stehen sind von den DefaultFormaten
/*N*/ 	//abhaengig. Erst nach dem Loeschen der FmtIndizes weil der Inhalt von
/*N*/ 	//Kopf-/Fussbereichen geloescht wird. Wenn dort noch Indizes von Flys
/*N*/ 	//angemeldet sind gibts was an die Ohren.
/*N*/ 	aPageDescs.DeleteAndDestroy( 0, aPageDescs.Count() );
/*N*/
/*N*/ 	// Inhaltssections loeschen
/*N*/ 	// nicht erst durch den SwNodes-DTOR, damit Formate
/*N*/ 	// keine Abhaengigen mehr haben.
/*N*/ 	aNodes.DelNodes( SwNodeIndex( aNodes ), aNodes.Count() );
/*N*/   // aUndoNodes.DelNodes( SwNodeIndex( aUndoNodes ), aUndoNodes.Count() );
/*N*/
/*N*/ 	// Formate loeschen, spaeter mal permanent machen.
/*N*/
/*N*/ 	// Delete fuer Collections
/*N*/ 	// damit die Abhaengigen wech sind
/*N*/ 	SwTxtFmtColl *pFtnColl = pFtnInfo->GetFtnTxtColl();
/*N*/ 	if ( pFtnColl ) pFtnColl->Remove(pFtnInfo);
/*N*/ 	pFtnColl = pEndNoteInfo->GetFtnTxtColl();
/*N*/ 	if ( pFtnColl ) pFtnColl->Remove(pEndNoteInfo);
/*N*/
/*N*/ 	ASSERT( pDfltTxtFmtColl == (*pTxtFmtCollTbl)[0],
/*N*/ 			"Default-Text-Collection muss immer am Anfang stehen" );
/*N*/
/*N*/ 	// JP 27.01.98: opt.: ausgehend davon, das Standard als 2. im Array
/*N*/ 	// 				steht, sollte das als letztes geloescht werden, damit
/*N*/ 	//				die ganze Umhaengerei der Formate vermieden wird!
/*N*/ 	if( 2 < pTxtFmtCollTbl->Count() )
/*N*/ 		pTxtFmtCollTbl->DeleteAndDestroy( 2, pTxtFmtCollTbl->Count()-2 );
/*N*/ 	pTxtFmtCollTbl->DeleteAndDestroy( 1, pTxtFmtCollTbl->Count()-1 );
/*N*/ 	delete pTxtFmtCollTbl;
/*N*/
/*N*/ 	ASSERT( pDfltGrfFmtColl == (*pGrfFmtCollTbl)[0],
/*N*/ 			"Default-Grf-Collection muss immer am Anfang stehen" );
/*N*/
/*N*/ 	pGrfFmtCollTbl->DeleteAndDestroy( 1, pGrfFmtCollTbl->Count()-1 );
/*N*/ // ergibt sich automatisch - kein _DEL Array!
/*N*/ //	pGrfFmtCollTbl->Remove( 0, n );
/*N*/ 	delete pGrfFmtCollTbl;
/*N*/
    /*
      * Defaultformate und DefaultFormatsammlungen (FmtColl)
     * sind an der Position 0 der jeweiligen Arrays eingetragen.
     * Damit sie nicht vom DTOR der Array's zum 2.mal geloescht werden,
     * nehme sie aus dem Array.
     */
/*N*/ 	pFrmFmtTbl->Remove( 0 );
/*N*/ 	pCharFmtTbl->Remove( 0 );
/*N*/
/*N*/ 	// Delete fuer pPrt
/*N*/ 	DELETEZ( pPrt );
/*N*/ 	DELETEZ( pSwgInfo );
/*N*/ 	DELETEZ( pNewDBMgr );
/*N*/
/*N*/ 	// Alle Flys muessen vor dem Drawing Model zerstoert werden,
/*N*/ 	// da Flys noch DrawContacts enthalten koennen, wenn wegen
/*N*/ 	// eines Lesefehlers kein Layout aufgebaut wurde.
/*N*/ 	pSpzFrmFmtTbl->DeleteAndDestroy( 0, pSpzFrmFmtTbl->Count() );
/*N*/
/*N*/ 	//Erst jetzt das Model zerstoeren, die Zeichenobjekte - die ja auch
/*N*/ 	//im Undo herumlungern - wollen noch ihre Attribute beim Model entfernen.
/*N*/ 	//Ausserdem koennen vorher noch DrawContacts existieren.
/*N*/ 	ReleaseDrawModel();
/*N*/ 	//JP 28.01.99: DrawModel vorm LinkManager zerstoeren, da am DrawModel
/*N*/ 	// 			dieser immer gesetzt ist.
/*N*/ 	DELETEZ( pLinkMgr );
/*N*/
/*N*/ 	//Tables vor dem loeschen der Defaults leeren, sonst GPF wegen Def-Abhaengigen.
/*N*/ 	//Die Arrays sollten (wegen includes) bei Gelegenheit auch zu Pointern werden.
/*N*/ 	delete pFrmFmtTbl;
/*N*/ 	delete pSpzFrmFmtTbl;
/*N*/ 	delete pCharFmtTbl;
/*N*/ 	delete pSectionFmtTbl;
/*N*/ 	delete pTblFrmFmtTbl;
/*N*/ 	delete pDfltTxtFmtColl;
/*N*/ 	delete pDfltGrfFmtColl;
/*N*/ 	delete pNumRuleTbl;
/*N*/
/*N*/ 	delete pPrtData;
/*N*/ 	delete pBookmarkTbl;
/*N*/ 	delete pNumberFormatter;
/*N*/ 	delete pFtnInfo;
/*N*/ 	delete pEndNoteInfo;
/*N*/ 	delete pLineNumberInfo;
/*N*/ 	delete pFtnIdxs;
/*N*/ 	delete pFldTypes;
/*N*/ 	delete pTOXTypes;
/*N*/ 	delete pDocStat;
/*N*/ 	delete pEmptyPageFmt;
/*N*/ 	delete pColumnContFmt;
/*N*/ 	delete pDfltCharFmt;
/*N*/ 	delete pDfltFrmFmt;
/*N*/     delete pLayoutCache;
/*N*/     delete pVirDev;
/*N*/ }


//---------------------------------------------------


/*N*/ void SwDoc::SetJobsetup( const JobSetup &rJobSetup )
/*N*/ {
/*N*/ 	BOOL bCheckPageDescs = 0 == pPrt;
/*N*/ 	BOOL bDataChanged = FALSE;
/*N*/
/*N*/ 	if ( pPrt )
/*N*/ 	{
/*N*/ 		if ( pPrt->GetName() == rJobSetup.GetPrinterName() )
/*N*/ 		{
/*N*/ 			if ( pPrt->GetJobSetup() != rJobSetup )
/*N*/ 			{
/*?*/ 				pPrt->SetJobSetup( rJobSetup );
/*?*/ 				bDataChanged = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			delete pPrt, pPrt = 0;
/*N*/ 	}
/*N*/
/*N*/ 	if( !pPrt )
/*N*/ 	{
/*N*/ 		//Das ItemSet wird vom Sfx geloescht!
/*?*/ 		SfxItemSet *pSet = new SfxItemSet( aAttrPool,
/*?*/ 						FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
/*?*/ 						SID_HTML_MODE,	SID_HTML_MODE,
/*?*/ 						SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
/*?*/ 						SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
/*?*/ 						0 );
/*?*/ 		SfxPrinter *p = new SfxPrinter( pSet, rJobSetup );
/*?*/ 		if ( bCheckPageDescs )
/*?*/ 			SetPrt( p );
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pPrt = p;
/*?*/ 			bDataChanged = TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	    if ( !IsUseVirtualDevice() && bDataChanged )
/*?*/ 		PrtDataChanged();
/*N*/ }

//---------------------------------------------------

/*N*/ OutputDevice& SwDoc::GetRefDev() const
/*N*/ {
/*N*/     if ( ! IsUseVirtualDevice() )
/*N*/     {
/*N*/         SfxPrinter& rPrt = *GetPrt( sal_True );
/*N*/         if ( rPrt.IsValid() )
/*N*/             return rPrt;
/*N*/     }
/*N*/
/*N*/     return *GetVirDev( sal_True );
/*N*/ }

//---------------------------------------------------

/*N*/ OutputDevice* SwDoc::_GetRefDev() const
/*N*/ {
/*N*/     if ( IsUseVirtualDevice() )
/*N*/         return pVirDev;
/*N*/     return pPrt;
/*N*/ }

//---------------------------------------------------

/*N*/ VirtualDevice& SwDoc::_GetVirDev() const
/*N*/ {
/*?*/     VirtualDevice* pNewVir = new VirtualDevice( 1 );
/*?*/     pNewVir->SetReferenceDevice(VirtualDevice::REFDEV_MODE_MSO1);
/*?*/     MapMode aMapMode( pNewVir->GetMapMode() );
/*?*/     aMapMode.SetMapUnit( MAP_TWIP );
/*?*/     pNewVir->SetMapMode( aMapMode );
/*?*/
/*?*/     ((SwDoc*)this)->SetVirDev( pNewVir, sal_True );
/*?*/     return *pVirDev;
/*N*/ }

//---------------------------------------------------

/*N*/ SfxPrinter& SwDoc::_GetPrt() const
/*N*/ {
/*N*/     ASSERT( ! pPrt, "Do not call _GetPrt(), call GetPrt() instead" )
/*N*/
/*N*/     // wir erzeugen einen default SfxPrinter.
/*N*/ 	// Das ItemSet wird vom Sfx geloescht!
/*N*/ 	SfxItemSet *pSet = new SfxItemSet( ((SwDoc*)this)->GetAttrPool(),
/*N*/ 					FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
/*N*/ 					SID_HTML_MODE,	SID_HTML_MODE,
/*N*/ 					SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
/*N*/ 					SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
/*N*/ 					0 );
/*N*/     SfxPrinter* pNewPrt = new SfxPrinter( pSet );
/*N*/ 	((SwDoc*)this)->SetPrt( pNewPrt, sal_True );
/*N*/     return *pPrt;
/*N*/ }
//---------------------------------------------------

/*N*/ SwPrintData*    SwDoc::GetPrintData() const
/*N*/ {
/*N*/     return pPrtData;
/*N*/ }
//---------------------------------------------------
/*N*/ void  SwDoc::SetPrintData(SwPrintData& rPrtData)
/*N*/ {
/*N*/     if(!pPrtData)
/*N*/         pPrtData = new SwPrintData;
/*N*/     *pPrtData = rPrtData;
/*N*/ }
//---------------------------------------------------


/*N*/ void SwDoc::SetDocShell( SwDocShell* pDSh )
/*N*/ {
/*N*/ 	if( pDocShell != pDSh )
/*N*/ 	{
/*N*/ 		pDocShell = pDSh;
/*N*/ 		pLinkMgr->SetPersist( pDocShell );
/*N*/ 		//JP 27.08.98: Bug 55570 - DocShell Pointer auch am DrawModel setzen
/*N*/ 		if( pDrawModel )
/*N*/ 		{
/*N*/ 			((SwDrawDocument*)pDrawModel)->SetObjectShell( pDocShell );
/*N*/ 			pDrawModel->SetPersist( pDocShell );
/*N*/ 			ASSERT( pDrawModel->GetPersist() == GetPersist(),
/*N*/ 					"draw model's persist is out of sync" );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


// Convenience-Methode, um uebermaessige Includes von docsh.hxx
// zu vermeiden


/*N*/ SvStorage* SwDoc::GetDocStorage()
/*N*/ {
/*N*/ 	if( pDocShell )
/*N*/ 		return pDocShell->GetStorage();
/*?*/ 	if( pLinkMgr->GetPersist() )
/*?*/ 		return pLinkMgr->GetPersist()->GetStorage();
/*?*/ 	return NULL;
/*N*/ }



/*N*/ SvPersist* SwDoc::GetPersist() const
/*N*/ {
/*N*/ 	return pDocShell ? pDocShell : pLinkMgr->GetPersist();
/*N*/ }


/*N*/ const SfxDocumentInfo* SwDoc::GetInfo()
/*N*/ {
/*N*/ 	if( !pSwgInfo )
/*N*/ 		// Pointer-Members initialisieren
/*N*/ 		pSwgInfo  = new SfxDocumentInfo;
/*N*/ 	return pSwgInfo;
/*N*/ }


/*N*/ void SwDoc::SetPreViewPrtData( const SwPagePreViewPrtData* pNew )
/*N*/ {
/*N*/ 	if( pNew )
/*N*/ 	{
/*N*/ 		if( pPgPViewPrtData )
/*N*/ 			*pPgPViewPrtData = *pNew;
/*N*/ 		else
/*N*/ 			pPgPViewPrtData = new SwPagePreViewPrtData( *pNew );
/*N*/ 	}
/*N*/ 	else if( pPgPViewPrtData )
/*N*/ 		DELETEZ( pPgPViewPrtData );
/*N*/ 	SetModified();
/*N*/ }
/* -----------------------------06.01.00 14:03--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwModify*	SwDoc::GetUnoCallBack() const
/*N*/ {
/*N*/ 	return pUnoCallBack;
/*N*/ }

/*N*/ vos::ORef < SvxForbiddenCharactersTable > & SwDoc::GetForbiddenCharacterTbl()
/*N*/ {
/*N*/ 	if( !xForbiddenCharsTable.isValid() )
/*N*/ 	{
/*N*/ 		::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::lang::XMultiServiceFactory > xMSF =
/*N*/ 									::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 		xForbiddenCharsTable = new SvxForbiddenCharactersTable( xMSF );
/*N*/ 	}
/*N*/ 	return xForbiddenCharsTable;
/*N*/ }
/* ------------------------------------------------------------------------*/

/*N*/ const ::com::sun::star::i18n::
/*N*/ 	ForbiddenCharacters* SwDoc::GetForbiddenCharacters( USHORT nLang,
/*N*/ 							BOOL bLocaleData ) const
/*N*/ {
/*N*/ 	const ::com::sun::star::i18n::ForbiddenCharacters* pRet = 0;
/*N*/ 	if( xForbiddenCharsTable.isValid() )
/*N*/ 		pRet = xForbiddenCharsTable->GetForbiddenCharacters( nLang, FALSE );
/*N*/ 	if( bLocaleData && !pRet && pBreakIt )
/*N*/ 		pRet = &pBreakIt->GetForbidden( (LanguageType)nLang );
/*N*/ 	return pRet;
/*N*/ }



/*N*/ void SwDoc::SetCharCompressType( SwCharCompressType n )
/*N*/ {
/*N*/ 	if( eChrCmprType != n )
/*N*/ 	{
/*N*/ 		eChrCmprType = n;
/*N*/ 		if( pDrawModel )
/*N*/ 		{
/*N*/ 			pDrawModel->SetCharCompressType( n );
/*N*/ 			if( !bInReading )
/*N*/ 				pDrawModel->ReformatAllTextObjects();
/*N*/ 		}
/*N*/
/*N*/ 		if( pLayout && !bInReading )
/*N*/ 		{
/*N*/ 			pLayout->StartAllAction();
/*N*/ 			pLayout->InvalidateAllCntnt();
/*N*/ 			pLayout->EndAllAction();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*-----------------28.5.2001 10:06------------------
 * SwDoc:
 *  Reading and writing of the layout cache.
 *--------------------------------------------------*/

/*N*/ void SwDoc::ReadLayoutCache( SvStream& rStream )
/*N*/ {
/*N*/     if( !pLayoutCache )
/*N*/         pLayoutCache = new SwLayoutCache();
/*N*/     if( !pLayoutCache->IsLocked() )
/*N*/     {
/*N*/         pLayoutCache->GetLockCount() |= 0x8000;
/*N*/         pLayoutCache->Read( rStream );
/*N*/         pLayoutCache->GetLockCount() &= 0x7fff;
/*N*/     }
/*N*/ }

/*N*/ void SwDoc::WriteLayoutCache( SvStream& rStream )
/*N*/ {
/*N*/     pLayoutCache->Write( rStream, *this );
/*N*/ }

}
