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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <bf_svtools/urihelper.hxx>
#include <bf_svtools/fstathelper.hxx>
#include <bf_svtools/moduleoptions.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/linkmgr.hxx>
#include <bf_svx/paperinf.hxx>

#include <errhdl.hxx>

#include <docary.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <editsh.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <fltini.hxx>
#include <docsh.hxx>
#include <redline.hxx>
#include <linkenum.hxx>
#include <swerror.h>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////

/*N*/ ULONG SwReader::Read( const Reader& rOptions )
/*N*/ {
/*N*/ 	// Variable uebertragen
/*N*/ 	Reader* po = (Reader*) &rOptions;
/*N*/ 	po->pStrm = pStrm;
/*N*/ 	po->pStg  = pStg;
/*N*/ 	po->bInsertMode = 0 != pCrsr;
/*N*/
/*N*/ 	// ist ein Medium angegeben, dann aus diesem die Streams besorgen
/*N*/   if( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 po->SetReadUTF8( FALSE );
/*?*/      return ERR_SWG_FILE_FORMAT_ERROR;
/*N*/ 	}
/*N*/
/*N*/ 	ULONG nError = 0L;
/*N*/
/*N*/ 	GetDoc();
/*N*/
/*N*/ 	// am Sw3-Reader noch den pIo-Pointer "loeschen"
/*N*/ 	if( po == ReadSw3 && pDoc->GetDocShell() &&
/*N*/ 		((Sw3Reader*)po)->GetSw3Io() != pDoc->GetDocShell()->GetIoSystem() )
/*N*/ 			((Sw3Reader*)po)->SetSw3Io( pDoc->GetDocShell()->GetIoSystem() );
/*N*/
/*N*/ 	// waehrend des einlesens kein OLE-Modified rufen
/*N*/ 	Link aOLELink( pDoc->GetOle2Link() );
/*N*/ 	pDoc->SetOle2Link( Link() );
/*N*/
/*N*/ 	pDoc->bInReading = TRUE;
/*N*/
/*N*/ 	SwPaM *pPam;
/*N*/ 	if( pCrsr )
/*?*/ 		pPam = pCrsr;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Wenn der Reader nicht mit einem Shell konstruiert wurde,
/*N*/ 		// selber einen Pam machen.
/*N*/ 		SwNodeIndex nNode( pDoc->GetNodes().GetEndOfContent(), -1 );
/*N*/ 		pPam = new SwPaM( nNode );
/*N*/ 		// Bei Web-Dokumenten wird die Default-Vorlage schon im InitNew
/*N*/ 		// gesetzt und braucht deshalb nicht nochmal gesetzt zu werden.
/*N*/ 		// Das gilt natuerlich nicht, wenn der Filter nicht der HTML-Filter
/*N*/ 		// ist oder im ConvertFrom zuvor ein SetTemplateName gerufen
/*N*/ 		// wurde.
/*N*/ 		if( !pDoc->IsHTMLMode() || ReadHTML != po || !po->pTemplate  )
/*N*/ 			po->SetTemplate( *pDoc );
/*N*/ 	}
/*N*/
/*N*/ 	// Pams sind ringfoermig verkettet. Aufhoeren, wenn man wieder beim
/*N*/ 	// ersten ist.
/*N*/ 	SwPaM *pEnd = pPam;
/*N*/
/*N*/ 	BOOL bReadPageDescs = FALSE;
/*N*/
/*N*/ 	SwNodeIndex aSplitIdx( pDoc->GetNodes() );
/*N*/
/*N*/ 	SwRedlineMode eOld = pDoc->GetRedlineMode();
/*N*/ 	pDoc->SetRedlineMode_intern( REDLINE_IGNORE );
/*N*/
/*N*/ 	// Array von FlyFormaten
/*N*/ 	SwSpzFrmFmts aFlyFrmArr;
/*N*/ 	// only read templates? then ignore multi selection!
/*N*/ 	BOOL bFmtsOnly = po->aOpt.IsFmtsOnly();
/*N*/
/*N*/ 	while( TRUE )
/*N*/ 	{
/*N*/       if( pCrsr )
/*N*/ 		{
/*?*/ 			// Pam auf den Node davor setzen damit er nicht mit verschoben wird
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
/*N*/ 		}
/*N*/
/*N*/ 		// Speicher mal alle Fly's
/*N*/ 		if( pCrsr )
/*?*/ 			aFlyFrmArr.Insert( pDoc->GetSpzFrmFmts(), 0L );
/*N*/
/*N*/ 		xub_StrLen nSttCntnt = pPam->GetPoint()->nContent.GetIndex();
/*N*/
/*N*/ 		// damit fuer alle Reader die Ende-Position immer stimmt, hier
/*N*/ 		// pflegen.
/*N*/ 		SwCntntNode* pCNd = pPam->GetCntntNode();
/*N*/ 		xub_StrLen nEndCntnt = pCNd ? pCNd->Len() - nSttCntnt : 0;
/*N*/ 		SwNodeIndex aEndPos( pPam->GetPoint()->nNode, 1 );
/*N*/
/*N*/ 		nError = po->Read( *pDoc, *pPam, aFileName );
/*N*/
/*N*/ 		if( !IsError( nError )) 	// dann setzen wir das Ende mal richtig
/*N*/ 		{
/*N*/ 			aEndPos--;
/*N*/ 			pCNd = aEndPos.GetNode().GetCntntNode();
/*N*/ 			if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &aEndPos ) ))
/*?*/ 				pCNd = pDoc->GetNodes().GoNext( &aEndPos );
/*N*/
/*N*/ 			pPam->GetPoint()->nNode = aEndPos;
/*N*/ 			xub_StrLen nLen = pCNd->Len();
/*N*/ 			if( nLen < nEndCntnt )
/*?*/ 				nEndCntnt = 0;
/*N*/ 			else
/*N*/ 				nEndCntnt = nLen - nEndCntnt;
/*N*/ 			pPam->GetPoint()->nContent.Assign( pCNd, nEndCntnt );
/*N*/ 		}
/*N*/
/*N*/ 		if( pCrsr )
/*N*/ 		{
/*?*/           DBG_BF_ASSERT(0, "STRIP");
/*N*/ 		}
/*N*/
/*N*/ 		pPam = (SwPaM *) pPam->GetNext();
/*N*/ 		if( pPam == pEnd )
/*N*/ 			break;
/*N*/
/*N*/ 		// only read templates? then ignore multi selection! Bug 68593
/*?*/ 		if( bFmtsOnly )
/*?*/ 			break;
/*?*/
        /*
         * !!! man muss selbst den Status vom Stream zuruecksetzen. !!!
         *	   Beim seekg wird der akt. Status, eof- und bad-Bit
         *	   gesetzt, warum weiss keiner
         */
/*?*/ 		if( pStrm )
/*?*/ 		{
/*?*/ 			pStrm->Seek(0);
/*?*/ 			pStrm->ResetError();
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	pDoc->bInReading = FALSE;
/*N*/ 	pDoc->SetAllUniqueFlyNames();
/*N*/
/*N*/
/*N*/ 	// Wenn der Pam nur fuers Lesen konstruiert wurde, jetzt zerstoeren.
/*N*/ 	if( !pCrsr )
/*N*/ 	{
/*N*/ 		delete pPam;		  // ein neues aufgemacht.
/*N*/ 		eOld = (SwRedlineMode)(pDoc->GetRedlineMode() & ~REDLINE_IGNORE);
/*N*/ 		pDoc->SetFieldsDirty( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	pDoc->SetRedlineMode_intern( eOld );
/*N*/ 	pDoc->SetOle2Link( aOLELink );
/*N*/
/*N*/ 	if( pCrsr )					// das Doc ist jetzt modifiziert
/*N*/ 		pDoc->SetModified();
/*N*/
/*N*/ 	if( po == ReadSw3 ) 		// am Sw3-Reader noch den pIo-Pointer "loeschen"
/*N*/ 		((Sw3Reader*)po)->SetSw3Io( 0 );
/*N*/
/*N*/ 	po->SetReadUTF8( FALSE );
/*N*/ 	po->SetBlockMode( FALSE );
/*N*/ 	po->SetOrganizerMode( FALSE );
/*N*/     po->SetIgnoreHTMLComments( FALSE );
/*N*/ 	return nError;
/*N*/ }


/*
 * Konstruktoren, Destruktor
 */

// Initiales Einlesben


/*N*/ SwReader::SwReader( SvStorage& rStg, const String& rFileName, SwDoc *pDoc )
/*N*/ 	: SwDocFac( pDoc ),
/*N*/ 	pStrm( 0 ),
/*N*/ 	pStg( &rStg ),
/*N*/ 	pMedium( 0 ),
/*N*/ 	aFileName( rFileName ),
/*N*/ 	pCrsr( 0 )
/*N*/ {
/*N*/ }


 SwReader::SwReader( SfxMedium& rMedium, const String& rFileName, SwDoc *pDoc )
    : SwDocFac( pDoc ),
    pStrm( 0 ),
    pStg( 0 ),
    pMedium( &rMedium ),
    aFileName( rFileName ),
    pCrsr( 0 )
 {
 }

// In ein existierendes Dokument einlesen

 SwReader::SwReader( SfxMedium& rMedium, const String& rFileName, SwPaM& rPam )
    : SwDocFac( rPam.GetDoc() ),
    aFileName( rFileName ),
    pStg( 0 ),
    pStrm( 0 ),
    pMedium( &rMedium ),
    pCrsr( &rPam )
 {
 }
/*N*/ Reader::Reader()
/*N*/ 	: pStrm(0), pStg(0), pMedium(0), pTemplate(0),
/*N*/ 	bTmplBrowseMode( FALSE ), bInsertMode( FALSE ),
/*N*/ 	bReadUTF8( FALSE ), bBlockMode( FALSE ), bOrganizerMode( FALSE ),
/*N*/     bHasAskTemplateName( FALSE ), bIgnoreHTMLComments( FALSE )
/*N*/ {
/*N*/ }

/*N*/ Reader::~Reader()
/*N*/ {
/*N*/ 	delete pTemplate;
/*N*/ }

/*N*/ String Reader::GetTemplateName() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

// Die Filter-Vorlage laden, setzen und wieder freigeben
/*N*/ SwDoc* Reader::GetTemplateDoc()
/*N*/ {
/*N*/ 	if( !bHasAskTemplateName )
/*N*/ 	{
/*N*/ 		SetTemplateName( GetTemplateName() );
/*N*/ 		bHasAskTemplateName = TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	if( !aTemplateNm.Len() )
/*N*/ 		ClearTemplate();
/*N*/ 	else
/*N*/ 	{
/*?*/ 		INetURLObject aTDir( ::binfilter::StaticBaseUrl::SmartRelToAbs(aTemplateNm) );
/*?*/ 		DateTime aCurrDateTime;
/*?*/ 		BOOL bLoad = FALSE;
/*?*/
/*?*/ 		// Wenn das Template schon mal geladen wurde, nur einmal pro
/*?*/ 		// Minute nachschauen, ob es geaendert wurde.
/*?*/ 		if( !pTemplate || aCurrDateTime >= aChkDateTime )
/*?*/ 		{
/*?*/ 			Date aTstDate;
/*?*/ 			Time aTstTime;
/*?*/ 			if( ::binfilter::GetModifiedDateTimeOfFile(
/*?*/ 							aTDir.GetMainURL( INetURLObject::NO_DECODE ),
/*?*/ 							&aTstDate, &aTstTime ) &&
/*?*/ 				( !pTemplate || aDStamp != aTstDate || aTStamp != aTstTime ))
/*?*/ 			{
/*?*/ 				bLoad = TRUE;
/*?*/ 				aDStamp = aTstDate;
/*?*/ 				aTStamp = aTstTime;
/*?*/ 			}
/*?*/
/*?*/ 			// Erst in einer Minute wieder mal nachschauen, ob sich die
/*?*/ 			// Vorlage geaendert hat.
/*?*/ 			aChkDateTime = aCurrDateTime;
/*?*/ 			aChkDateTime += Time( 0L, 1L );
/*?*/ 		}
/*?*/
/*?*/ 		if( bLoad )
/*?*/ 		{
/*?*/ 			ClearTemplate();
/*?*/ 			ASSERT( !pTemplate, "Who holds the template doc?" );
/*?*/
/*?*/ 			SvStorageRef xStor( new SvStorage( aTDir.GetFull(), STREAM_READ ));
/*?*/ 			ULONG nFormat = xStor->GetFormat();
/*?*/ 			long nVersion = SOFFICE_FILEFORMAT_60;
/*?*/ 			switch( nFormat )
/*?*/ 			{
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITER_50:
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITERGLOB_50:
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITERWEB_50:
/*?*/ 				nVersion = SOFFICE_FILEFORMAT_50;
/*?*/ 				break;
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITER_40:
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITERGLOB_40:
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITERWEB_40:
/*?*/ 				nVersion = SOFFICE_FILEFORMAT_40;
/*?*/ 				break;
/*?*/ 			case SOT_FORMATSTR_ID_STARWRITER_30:
/*?*/ 				nVersion = SOFFICE_FILEFORMAT_31;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			if( nVersion >= SOFFICE_FILEFORMAT_60 )
/*?*/ 			{
/*?*/ 				// #95605#: If the writer module is not installed,
/*?*/ 				// we cannot create a SwDocShell. We could create a
/*?*/ 				// SwWebDocShell however, because this exists always
/*?*/ 				// for the help.
/*?*/ 				SvtModuleOptions aModuleOptions;
/*?*/ 				if( aModuleOptions.IsWriter() )
/*?*/ 				{
/*?*/ 					SwDocShell *pDocSh =
/*?*/ 						new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
/*?*/ 					SvEmbeddedObjectRef xDocSh = pDocSh;
/*?*/ 					if( pDocSh->DoInitNew( 0 ) )
/*?*/ 					{
/*?*/ 						pTemplate = pDocSh->GetDoc();
/*?*/ 						pTemplate->SetOle2Link( Link() );
/*?*/ 						pTemplate->SetBrowseMode( bTmplBrowseMode );
/*?*/ 						pTemplate->RemoveAllFmtLanguageDependencies();
/*?*/
/*?*/ 						ReadXML->SetOrganizerMode( TRUE );
/*?*/ 						SwReader aRdr( *xStor, aEmptyStr, pTemplate );
/*?*/ 						aRdr.Read( *ReadXML );
/*?*/ 						ReadXML->SetOrganizerMode( FALSE );
/*?*/
/*?*/ 						pTemplate->AddLink();
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pTemplate = new SwDoc;
/*?*/ 			}
/*?*/ 		}
/*?*/
/*?*/ 		ASSERT( !pTemplate || ::binfilter::IsDocument(
/*?*/ 				aTDir.GetMainURL( INetURLObject::NO_DECODE ) ) ||
/*?*/ 				aTemplateNm.EqualsAscii( "$$Dummy$$" ),
/*?*/ 				"TemplatePtr but no template exist!" );
/*N*/ 	}
/*N*/
/*N*/ 	return pTemplate;
/*N*/ }

/*N*/ BOOL Reader::SetTemplate( SwDoc& rDoc )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/
/*N*/ 	GetTemplateDoc();
/*N*/ 	if( pTemplate )
/*N*/ 	{
/*?*/ 		rDoc.RemoveAllFmtLanguageDependencies();
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 rDoc.ReplaceStyles( *pTemplate );
/*N*/ 	}
/*N*/
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void Reader::ClearTemplate()
/*N*/ {
/*N*/ 	if( pTemplate )
/*N*/ 	{
/*?*/ 		if( 0 == pTemplate->RemoveLink() )
/*?*/ 			delete pTemplate,
/*?*/ 		pTemplate = 0;
/*N*/ 	}
/*N*/ }

/*N*/ void Reader::SetTemplateName( const String& rDir )
/*N*/ {
/*N*/ 	if( rDir.Len() && aTemplateNm != rDir )
/*N*/ 	{
/*?*/ 		ClearTemplate();
/*?*/ 		aTemplateNm = rDir;
/*N*/ 	}
/*N*/ }


// alle die die Streams / Storages nicht geoeffnet brauchen,
// muessen die Methode ueberladen
int Reader::SetStrmStgPtr()
{
   ASSERT( pMedium, "Wo ist das Medium??" );

   if( pMedium->IsStorage() )
   {
       if( SW_STORAGE_READER & GetReaderType() )
       {
           pStg = pMedium->GetStorage();
           return TRUE;
       }
   }
   else if( SW_STREAM_READER & GetReaderType() )
   {
       pStrm = pMedium->GetInStream();
       return TRUE;
   }
   return FALSE;
}


 int Reader::GetReaderType()
 {
    return SW_STREAM_READER;
 }


 void Reader::SetFltName( const String& )
 {
 }


void Reader::SetNoOutlineNum( SwDoc& rDoc )
{
}


void Reader::ResetFrmFmtAttrs( SfxItemSet &rFrmSet )
{
    rFrmSet.Put( SvxLRSpaceItem() );
    rFrmSet.Put( SvxULSpaceItem() );
    rFrmSet.Put( SvxBoxItem() );
}


void Reader::ResetFrmFmts( SwDoc& rDoc )
{
    for( USHORT i=0; i<3; i++ )
    {
        USHORT nPoolId;
        switch( i )
        {
        case 0: nPoolId = RES_POOLFRM_FRAME;    break;
        case 1: nPoolId = RES_POOLFRM_GRAPHIC;  break;
        case 2: nPoolId = RES_POOLFRM_OLE;      break;
        }

        SwFrmFmt *pFrmFmt = rDoc.GetFrmFmtFromPool( nPoolId );

        pFrmFmt->ResetAttr( RES_LR_SPACE );
        pFrmFmt->ResetAttr( RES_UL_SPACE );
        pFrmFmt->ResetAttr( RES_BOX );
    }
}

    // read the sections of the document, which is equal to the medium.
    // returns the count of it

// ------------------------------------------------




// ------------------------------------------------





/*
 * Writer
 */

/*
 * Konstruktoren, Destruktoren sind inline (inc/shellio.hxx).
 */






/*N*/ SwWriter::SwWriter( SvStream& rStrm, SwPaM& rPam, BOOL bWriteAll )
/*N*/ 	: pStrm( &rStrm ),
/*N*/ 	pStg( 0 ),
/*N*/ 	pMedium( 0 ),
/*N*/ 	pShell( 0 ),
/*N*/ 	pOutPam( &rPam ),
/*N*/ 	rDoc( *rPam.GetDoc() ),
/*N*/ 	bWriteAll( bWriteAll )
/*N*/ {
/*N*/ }

/*

SwWriter::SwWriter( SvStorage& rStg, SwCrsrShell &rShell, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    pShell( &rShell ),
    pOutPam( 0 ),
    rDoc( *rShell.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/


/*N*/ SwWriter::SwWriter(SvStorage& rStg,SwDoc &rDoc)
/*N*/ 	:pStrm( 0 ),
/*N*/ 	pStg( &rStg ),
/*N*/ 	pMedium( 0 ),
/*N*/ 	pShell( 0 ),
/*N*/ 	pOutPam( 0 ),
/*N*/ 	rDoc( rDoc ),
/*N*/ 	bWriteAll( TRUE )
/*N*/ {
/*N*/ }
/*

SwWriter::SwWriter( SvStorage& rStg, SwPaM& rPam, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( &rStg ),
    pMedium( 0 ),
    pShell( 0 ),
    pOutPam( &rPam ),
    rDoc( *rPam.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/




/*

SwWriter::SwWriter( SfxMedium& rMedium, SwPaM& rPam, BOOL bWriteAll )
    : pStrm( 0 ),
    pStg( 0 ),
    pShell( 0 ),
    pMedium( &rMedium ),
    pOutPam( &rPam ),
    rDoc( *rPam.GetDoc() ),
    bWriteAll( bWriteAll )
{
}
*/


/*N*/ ULONG SwWriter::Write( WriterRef& rxWriter, const String* pRealFileName )
/*N*/ {
/*N*/ 	BOOL bHasMark = FALSE;
/*N*/ 	SwPaM * pPam;
/*N*/
/*N*/ 	SwDoc *pDoc = 0L;
/*N*/     SvEmbeddedObjectRef* pRefForDocSh = 0;
/*N*/
/*N*/ 	if ( pShell && !bWriteAll && pShell->IsTableMode() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bWriteAll = TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	if( !bWriteAll && ( pShell || pOutPam ))
/*N*/ 	{
/*N*/ 		if( pShell )
/*?*/ 			pPam = pShell->GetCrsr();
/*N*/ 		else
/*N*/ 			pPam = pOutPam;
/*N*/
/*N*/ 		SwPaM *pEnd = pPam;
/*N*/
/*N*/ 		// Erste Runde: Nachsehen, ob eine Selektion besteht.
/*N*/ 		while(TRUE)
/*N*/ 		{
/*N*/ 			bHasMark = bHasMark || pPam->HasMark();
/*N*/ 			pPam = (SwPaM *) pPam->GetNext();
/*N*/ 			if(bHasMark || pPam == pEnd)
/*N*/ 				break;
/*N*/ 		}
/*N*/
/*N*/ 		// Wenn keine Selektion besteht, eine ueber das ganze Dokument aufspannen.
/*N*/ 		if(!bHasMark)
/*N*/ 		{
/*?*/ 			if( pShell )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pShell->Push();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				pPam = new SwPaM( *pPam );
/*?*/ 				pPam->Move( fnMoveBackward, fnGoDoc );
/*?*/ 				pPam->SetMark();
/*?*/ 				pPam->Move( fnMoveForward, fnGoDoc );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		// pPam ist immer noch der akt. Cursor !!
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// keine Shell oder alles schreiben -> eigenen Pam erzeugen
/*N*/ 		SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;
/*N*/ 		pPam = new SwPaM( pOutDoc->GetNodes().GetEndOfContent() );
/*N*/ 		pPam->Move( fnMoveBackward, fnGoDoc );
/*N*/ 		pPam->SetMark();
/*N*/ 		pPam->Move( fnMoveForward, fnGoDoc );
/*N*/ 	}
/*N*/
/*N*/ 	rxWriter->bWriteAll = bWriteAll;
/*N*/ 	SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;
/*N*/
/*N*/ 	// falls der Standart PageDesc. immer noch auf initalen Werten steht
/*N*/ 	// (wenn z.B. kein Drucker gesetzt wurde) dann setze jetzt auf DIN A4
/*N*/ 	if( !pOutDoc->GetPrt() )
/*N*/ 	{
/*?*/ 		const SwPageDesc& rPgDsc = pOutDoc->GetPageDesc( 0L );
/*?*/ 		//const SwPageDesc& rPgDsc = *pOutDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );;
/*?*/ 		const SwFmtFrmSize& rSz = rPgDsc.GetMaster().GetFrmSize();
/*?*/ 		// Clipboard-Dokument wird immer ohne Drucker angelegt, so ist
/*?*/ 		// der Std.PageDesc immer aug LONG_MAX !! Mappe dann auf DIN A4
/*?*/ 		if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
/*?*/ 		{
/*?*/ 			SwPageDesc aNew( rPgDsc );
/*?*/ 			SwFmtFrmSize aNewSz( rSz );
/*?*/ 			aNewSz.SetHeight( lA4Height );
/*?*/ 			aNewSz.SetWidth( lA4Width );
/*?*/ 			aNew.GetMaster().SetAttr( aNewSz );
/*?*/ 			pOutDoc->ChgPageDesc( 0, aNew );
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SwEditShell* pESh = pOutDoc->GetEditShell();
/*N*/ 	if( pESh )
/*N*/ 		pESh->StartAllAction();
/*N*/
/*N*/ 	BOOL bWasPurgeOle = pOutDoc->IsPurgeOLE();
/*N*/ 	pOutDoc->SetPurgeOLE( FALSE );
/*N*/
/*N*/ 	ULONG nError = 0;
/*N*/ 	if( pMedium )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	nError = rxWriter->Write( *pPam, *pMedium, pRealFileName );
/*N*/ 	else if( pStg )
/*N*/ 		nError = rxWriter->Write( *pPam, *pStg, pRealFileName );
/*N*/ 	else if( pStrm )
/*N*/ 		nError = rxWriter->Write( *pPam, *pStrm, pRealFileName );
/*N*/
/*N*/ 	pOutDoc->SetPurgeOLE( bWasPurgeOle );
/*N*/ 	if( pESh )
/*N*/ 		pESh->EndAllAction();
/*N*/
/*N*/ 	// Falls nur zum Schreiben eine Selektion aufgespannt wurde, vor der
/*N*/ 	// Rueckkehr den alten Crsr wieder herstellen.
/*N*/ 	if( !bWriteAll && ( pShell || pOutPam ))
/*N*/ 	{
/*N*/ 		if(!bHasMark)
/*N*/ 		{
/*?*/ 			if( pShell )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pShell->Pop( FALSE );
/*?*/ 			else
/*?*/ 				delete pPam;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		delete pPam;			// loesche den hier erzeugten Pam
/*N*/ 		// Alles erfolgreich geschrieben? Sag' das dem Dokument!
/*N*/ 		if( !IsError( nError ) && !pDoc )
/*N*/ 			rDoc.ResetModified();
/*N*/ 	}
/*N*/
/*N*/ 	if ( pDoc )
/*N*/ 	{
/*N*/         delete pRefForDocSh;
/*?*/ 		if ( !pDoc->RemoveLink() )
/*?*/ 			delete pDoc;
/*?*/ 		bWriteAll = FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	return nError;
/*N*/ }


/*  */

// ----------------------------------------------------------------------


/*N*/ BOOL SetHTMLTemplate( SwDoc & rDoc )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 //STRIP001 	// Vorlagennamen von den Sfx-HTML-Filter besorgen!!!
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
