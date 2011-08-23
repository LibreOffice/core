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

#ifdef OS2
#endif


#include <sot/formats.hxx>
#include <bf_so3/lnkbase.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_sfx2/progress.hxx>
#include <bf_sfx2/interno.hxx>

#include "fileobj.hxx"
#include "linkmgr.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "impgrf.hxx"
namespace binfilter {

using namespace ::com::sun::star::uno;


/*N*/ #define FILETYPE_TEXT		1
/*N*/ #define FILETYPE_GRF		2

/*N*/ struct Impl_DownLoadData
/*N*/ {
/*N*/ 	Graphic aGrf;
/*N*/ 	Timer aTimer;
/*N*/ 
/*N*/ 	Impl_DownLoadData( const Link& rLink )
/*N*/ 	{
/*N*/ 		aTimer.SetTimeout( 100 );
/*N*/ 		aTimer.SetTimeoutHdl( rLink  );
/*N*/ 		aGrf.SetDefaultType();
/*N*/ 	}
/*N*/ 	~Impl_DownLoadData()
/*N*/ 	{
/*N*/ 		aTimer.Stop();
/*N*/ 	}
/*N*/ };

// --------------------------------------------------------------------------


/*N*/ SvFileObject::SvFileObject()
/*N*/ 	: nType( FILETYPE_TEXT ), pDownLoadData( 0 )
/*N*/ {
/*N*/ 	bLoadAgain = bMedUseCache = TRUE;
/*N*/ 	bSynchron = bLoadError = bWaitForData = bDataReady = bNativFormat =
/*N*/ 	bClearMedium = bStateChangeCalled = bInCallDownLoad = FALSE;
/*N*/ }


/*N*/ SvFileObject::~SvFileObject()
/*N*/ {
/*N*/ 	if ( xMed.Is() )
/*N*/ 	{
/*?*/ 		xMed->SetDataAvailableLink( Link() );
/*?*/ 		xMed->SetDoneLink( Link() );
/*?*/ 		xMed.Clear();
/*N*/ 	}
/*N*/ 	delete pDownLoadData;
/*N*/ }


/*N*/ BOOL SvFileObject::GetData( ::com::sun::star::uno::Any & rData,
/*N*/ 								const String & rMimeType,
/*N*/ 								BOOL bGetSynchron )
/*N*/ {
/*N*/ 	ULONG nFmt = SotExchange::GetFormatStringId( rMimeType );
/*N*/ 	switch( nType )
/*N*/ 	{
/*N*/ 	case FILETYPE_TEXT:
/*N*/ 		if( FORMAT_FILE == nFmt )
/*N*/ 		{
/*N*/ 			// das Medium muss in der Applikation geoffnet werden, um die
/*N*/ 			// relativen Datei Links aufzuloesen!!!! Wird ueber den
/*N*/ 			// LinkManager und damit von dessen Storage erledigt.
/*N*/ 			rData <<= ::rtl::OUString( sFileNm );
/*N*/ 
/*
===========================================================================
JP 28.02.96: noch eine Baustelle:
                Idee: hier das Medium und die DocShell anlegen, Doc laden
                      und ueber OLE-SS (GetObj(...)) den Bereich als
                      PseudoObject erfragen. Dieses mit den Daten oder
                      dessen Daten verschicken.

===========================================================================

            SfxMedium aMed( aFileNm.GetFull(), STREAM_READ, TRUE );
            aMed.DownLoad();		// nur mal das Medium anfassen (DownLoaden)

            if( aMed.IsStorage() )
                pSvData->SetData( SvStorageRef( aMed.GetStorage() ),
                                    TRANSFER_COPY );
            else
            {
                SvStream* pStream = aMed.GetInStream();
                if( !pStream )
                    return FALSE;

                UINT32 nLen = pStream->Seek( STREAM_SEEK_TO_END );
                pStream->Seek( STREAM_SEEK_TO_BEGIN );

                void* pData = SvMemAlloc( nLen );
                pStream->Read( pData, nLen );
                pSvData->SetData( pData, nLen, TRANSFER_MOVE );
            }
*/
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FILETYPE_GRF:
/*?*/ 		if( !bLoadError )
/*?*/ 		{
/*?*/ 			SfxMediumRef xTmpMed;
/*?*/ 
/*?*/ 			if( FORMAT_GDIMETAFILE == nFmt || FORMAT_BITMAP == nFmt ||
/*?*/ 				SOT_FORMATSTR_ID_SVXB == nFmt )
/*?*/ 			{
/*?*/ 				Graphic aGrf;
/*?*/ 
/*?*/ 				//JP 15.07.98: Bug 52959
/*?*/ 				//		falls das Nativformat doch erwuenscht ist, muss am
/*?*/ 				//		Ende das Flag zurueckgesetzt werden.
/*?*/ // wird einzig und allein im sw/ndgrf.cxx benutzt, wenn der Link vom
/*?*/ // GraphicNode entfernt wird.
/*?*/ 				BOOL bOldNativFormat = bNativFormat;
/*?*/ //!!??				bNativFormat = 0 != (ASPECT_ICON & pSvData->GetAspect());
/*?*/ 
/*?*/ 				// falls gedruckt werden soll, warten wir bis die
/*?*/ 				// Daten vorhanden sind
/*?*/ 				if( bGetSynchron )
/*?*/ 				{
/*?*/ 					// testhalber mal ein LoadFile rufen um das nach-
/*?*/ 					// laden ueberahaupt anzustossen
/*?*/ 					if( !xMed.Is() )
/*?*/ 					{
/*?*/ 						LoadFile_Impl();
/*?*/ 						if( xMed.Is() )
/*?*/ 							// dann mit der hoechsten Prioritaet
/*?*/ 							xMed->SetTransferPriority( SFX_TFPRIO_VISIBLE_LOWRES_GRAPHIC );
/*?*/ 					}
/*?*/ 
/*?*/ 					if( !bInCallDownLoad )
/*?*/ 					{
/*?*/ 						xTmpMed = xMed;
/*?*/ 						while( bWaitForData )
/*?*/ 							Application::Reschedule();
/*?*/ 
/*?*/ 						xMed = xTmpMed;
/*?*/ 						bClearMedium = TRUE;
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				if( pDownLoadData ||
/*?*/ 					( !bWaitForData && ( xMed.Is() || 		// wurde als URL geladen
/*?*/ 						( bSynchron && LoadFile_Impl() && xMed.Is() ) )) )
/*?*/ 				{
/*?*/ 					// falls
/*?*/ 
/*?*/ 					// falls es uebers Internet gesogen wurde, nicht
/*?*/ 					// wieder versuchen
/*?*/ 					if( !bGetSynchron )
/*?*/ 						bLoadAgain = !xMed->IsRemote();
/*?*/ 					bLoadError = !GetGraphic_Impl( aGrf, xMed->GetInStream() );
/*?*/ 				}
/*?*/ 				else if( !LoadFile_Impl() ||
/*?*/ 						!GetGraphic_Impl( aGrf, xMed.Is() ? xMed->GetInStream() : 0 ))
/*?*/ 				{
/*?*/ 					if( !xMed.Is() )
/*?*/ 						break;
/*?*/ 					aGrf.SetDefaultType();
/*?*/ 				}
/*?*/ 
/*?*/ 				if( SOT_FORMATSTR_ID_SVXB != nFmt )
/*?*/ 					nFmt = (bLoadError || GRAPHIC_BITMAP == aGrf.GetType())
/*?*/ 								? FORMAT_BITMAP
/*?*/ 								: FORMAT_GDIMETAFILE;
/*?*/ 
/*?*/ 				SvMemoryStream aMemStm( 0, 65535 );
/*?*/ 				switch ( nFmt )
/*?*/ 				{
/*?*/ 				case SOT_FORMATSTR_ID_SVXB:
/*?*/ 					if( GRAPHIC_NONE != aGrf.GetType() )
/*?*/ 					{
/*?*/ 						aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
/*?*/ 						aMemStm << aGrf;
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case  FORMAT_BITMAP:
/*?*/ 					if( !aGrf.GetBitmap().IsEmpty())
/*?*/ 						aMemStm << aGrf.GetBitmap();
/*?*/ 					break;
/*?*/ 
/*?*/ 				default:
/*?*/ 					if( aGrf.GetGDIMetaFile().GetActionCount() )
/*?*/ 					{
/*?*/ 						GDIMetaFile aMeta( aGrf.GetGDIMetaFile() );
/*?*/ 						aMeta.Write( aMemStm );
/*?*/ 					}
/*?*/ 				}
/*?*/ 				rData <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(),
/*?*/ 										aMemStm.Seek( STREAM_SEEK_TO_END ) );
/*?*/ 
/*?*/ 				bNativFormat = bOldNativFormat;
/*?*/ 
/*?*/ 				// alles fertig?
/*?*/ 				if( xMed.Is() && !bSynchron && bClearMedium )
/*?*/ 				{
/*?*/ 					xMed.Clear();
/*?*/ 					bClearMedium = FALSE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	}
/*N*/ 	return sal_True/*0 != aTypeList.Count()*/;
/*N*/ }




/*N*/ BOOL SvFileObject::Connect( ::binfilter::SvBaseLink* pLink )
/*N*/ {
/*N*/ 	if( !pLink || !pLink->GetLinkManager() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	// teste doch mal, ob nicht ein anderer Link mit der gleichen
/*N*/ 	// Verbindung schon existiert
/*N*/ 	pLink->GetLinkManager()->GetDisplayNames( pLink, 0, &sFileNm, 0, &sFilter );
/*N*/ 
/*N*/ 	if( OBJECT_CLIENT_GRF == pLink->GetObjType() )
/*N*/ 	{
/*?*/ 		if( !pLink->IsUseCache() )
/*?*/ 			bMedUseCache = FALSE;
/*?*/ 
/*?*/ 		// Reload-Erkennung ???
/*?*/ 		SvInPlaceObjectRef aRef( pLink->GetLinkManager()->GetPersist() );
/*?*/ 		if( aRef.Is() )
/*?*/ 		{
/*?*/ 			SfxObjectShell* pShell = ((SfxInPlaceObject*)&aRef)->GetObjectShell();
/*?*/ 			if( pShell->IsAbortingImport() )
/*?*/ 				return FALSE;
/*?*/ 
/*?*/ 			if( pShell->IsReloading() )
/*?*/ 				bMedUseCache = FALSE;
/*?*/ 
/*?*/ 			if( pShell->GetMedium() )
/*?*/ 				sReferer = pShell->GetMedium()->GetName();
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	switch( pLink->GetObjType() )
/*N*/ 	{
/*N*/ 	case OBJECT_CLIENT_GRF:
/*?*/ 		nType = FILETYPE_GRF;
/*?*/ 		bSynchron = pLink->IsSynchron();
/*?*/ 		break;
/*N*/ 
/*N*/ 	case OBJECT_CLIENT_FILE:
/*N*/ 		nType = FILETYPE_TEXT;
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetUpdateTimeout( 0 );
/*N*/ 
/*N*/ 	// und jetzt bei diesem oder gefundenem Pseudo-Object anmelden
/*N*/ 	AddDataAdvise( pLink, SotExchange::GetFormatMimeType( pLink->GetContentType() ), 0 );
/*N*/ 	return TRUE;
/*N*/ }


/*?*/ BOOL SvFileObject::LoadFile_Impl()
/*?*/ {
/*?*/ 	// wir sind noch im Laden!!
/*?*/ 	if( bWaitForData || !bLoadAgain || xMed.Is() || pDownLoadData )
/*?*/ 		return FALSE;
/*?*/ 
/*?*/ 	// z.Z. nur auf die aktuelle DocShell
/*?*/ 	xMed = new SfxMedium( sFileNm, STREAM_STD_READ, TRUE );
/*?*/ 	// Keinen Eintrag im Roter Button Menu
/*?*/ 	xMed->SetDontCreateCancellable();
/*?*/ 	xMed->SetUsesCache( bMedUseCache );
/*?*/ 	if( sReferer.Len() )
/*?*/ 		xMed->SetReferer( sReferer );
/*?*/ 	// erstmal mit der niedrigsten Prioritaet
/*?*/ 	xMed->SetTransferPriority( SFX_TFPRIO_INVISIBLE_HIGHRES_GRAPHIC );
/*?*/ 
/*?*/ 	if( !bSynchron )
/*?*/ 	{
/*?*/ 		bLoadAgain = bDataReady = bInNewData = FALSE;
/*?*/ 		bWaitForData = TRUE;
/*?*/ 
/*?*/ 		SfxMediumRef xTmpMed = xMed;
/*?*/ 		xMed->SetDataAvailableLink( STATIC_LINK( this, SvFileObject, LoadGrfNewData_Impl ) );
/*?*/ 		bInCallDownLoad = TRUE;
/*?*/ 		xMed->DownLoad( STATIC_LINK( this, SvFileObject, LoadGrfReady_Impl ) );
/*?*/ 		bInCallDownLoad = FALSE;
/*?*/ 
/*?*/ 		bClearMedium = !xMed.Is();
/*?*/ 		if( bClearMedium )
/*?*/ 			xMed = xTmpMed;		// falls gleich im DownLoad schon schluss ist
/*?*/ 		return bDataReady;
/*?*/ 	}
/*?*/ 
/*?*/ 	bWaitForData = TRUE;
/*?*/ 	bDataReady = bInNewData = FALSE;
/*?*/ 	xMed->DownLoad();
/*?*/ 	bLoadAgain = !xMed->IsRemote();
/*?*/ 	bWaitForData = FALSE;
/*?*/ 
/*?*/ 	// Grafik ist fertig, also DataChanged von der Statusaederung schicken:
/*?*/ 	SendStateChg_Impl( xMed->GetInStream() && xMed->GetInStream()->GetError()
/*?*/ 						? STATE_LOAD_ERROR : STATE_LOAD_OK );
/*?*/ 
/*?*/ 	return TRUE;
/*?*/ }


/*?*/ BOOL SvFileObject::GetGraphic_Impl( Graphic& rGrf, SvStream* pStream )
/*?*/ {
/*?*/ 	GraphicFilter* pGF = GetGrfFilter();
/*?*/ 	const int nFilter = sFilter.Len() && pGF->GetImportFormatCount()
/*?*/ 							? pGF->GetImportFormatNumber( sFilter )
/*?*/ 							: GRFILTER_FORMAT_DONTKNOW;
/*?*/ 
/*?*/ 	String aEmptyStr;
/*?*/ 	int nRes;
/*?*/ 
/*?*/ 	// vermeiden, dass ein native Link angelegt wird
/*?*/ 	if( ( !pStream || !pDownLoadData ) && !rGrf.IsLink() &&
/*?*/ 		!rGrf.GetContext() && !bNativFormat )
/*?*/ 		rGrf.SetLink( GfxLink() );
/*?*/ 
/*?*/ 	if( !pStream )
/*?*/ 		nRes = xMed.Is() ? GRFILTER_OPENERROR
/*?*/ 						 : pGF->ImportGraphic( rGrf, INetURLObject(sFileNm), nFilter );
/*?*/ 	else if( !pDownLoadData )
/*?*/ 	{
/*?*/ 		pStream->Seek( STREAM_SEEK_TO_BEGIN );
/*?*/ 		nRes = pGF->ImportGraphic( rGrf, aEmptyStr, *pStream, nFilter );
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		nRes = pGF->ImportGraphic( pDownLoadData->aGrf, aEmptyStr,
/*?*/ 									*pStream, nFilter );
/*?*/ 
/*?*/ 		if( pDownLoadData )
/*?*/ 		{
/*?*/ 			rGrf = pDownLoadData->aGrf;
/*?*/ 			if( GRAPHIC_NONE == rGrf.GetType() )
/*?*/ 				rGrf.SetDefaultType();
/*?*/ 
/*?*/ 
/*?*/ 			if( !pDownLoadData->aGrf.GetContext() )
/*?*/ 			{
/*?*/ 				xMed->SetDataAvailableLink( Link() );
/*?*/ //				xMed->SetDoneLink( Link() );
/*?*/ 				delete pDownLoadData, pDownLoadData = 0;
/*?*/ 				bDataReady = TRUE;
/*?*/ 				bWaitForData = FALSE;
/*?*/ 			}
/*?*/ 			else if( FALSE )
/*?*/ 			{
/*?*/ 				// Timer aufsetzen, um zurueck zukehren
/*?*/ 				pDownLoadData->aTimer.Start();
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	if( pStream && ERRCODE_IO_PENDING == pStream->GetError() )
/*?*/ 		pStream->ResetError();
/*?*/ 
/*?*/ #ifdef DBG_UTIL
/*?*/ 	if( nRes )
/*?*/ 	{
/*?*/ 		if( xMed.Is() && !pStream )
/*?*/ 		{
/*?*/ 			DBG_WARNING3( "GrafikFehler [%d] - [%s] URL[%s]",
/*?*/ 							nRes,
/*?*/ 							xMed->GetPhysicalName().GetBuffer(),
/*?*/ 							sFileNm.GetBuffer() );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			DBG_WARNING2( "GrafikFehler [%d] - [%s]",
/*?*/ 							nRes, sFileNm.GetBuffer() );
/*?*/ 		}
/*?*/ 	}
/*?*/ #endif
/*?*/ 
/*?*/ 	return GRFILTER_OK == nRes;
/*?*/ }




/*?*/ IMPL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void*, EMPTYARG )
/*?*/ {
/*?*/ 	// wenn wir von hier kommen, kann es kein Fehler mehr sein
/*?*/ 	pThis->bLoadError = FALSE;
/*?*/ 	pThis->bWaitForData = FALSE;
/*?*/ 	pThis->bInCallDownLoad = FALSE;
/*?*/ 
/*?*/ 	if( !pThis->bInNewData && !pThis->bDataReady )
/*?*/ 	{
/*?*/ 			// Grafik ist fertig, also DataChanged von der Status-
/*?*/ 			// aederung schicken:
/*?*/ 		pThis->bDataReady = TRUE;
/*?*/ 		pThis->SendStateChg_Impl( STATE_LOAD_OK );
/*?*/ 
/*?*/ 			// und dann nochmal die Daten senden
/*?*/ 		pThis->NotifyDataChanged();
/*?*/ 	}
/*?*/ 
/*?*/ 	if( pThis->bDataReady )
/*?*/ 	{
/*?*/ 		pThis->bLoadAgain = TRUE;
/*?*/ 		if( pThis->xMed.Is() )
/*?*/ 		{
/*?*/ 			pThis->xMed->SetDataAvailableLink( Link() );
/*?*/ 			pThis->xMed->SetDoneLink( Link() );
/*?*/ 
/*?*/ 			Application::PostUserEvent(
/*?*/ 						STATIC_LINK( pThis, SvFileObject, DelMedium_Impl ),
/*?*/ 						new SfxMediumRef( pThis->xMed ));
/*?*/ 			pThis->xMed.Clear();
/*?*/ 		}
/*?*/ 		if( pThis->pDownLoadData )
/*?*/ 			delete pThis->pDownLoadData, pThis->pDownLoadData = 0;
/*?*/ 	}
/*?*/ 
/*?*/ 	return 0;
/*?*/ }

/*?*/ IMPL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef*, pDelMed )
/*?*/ {
/*?*/ 	delete pDelMed;
/*?*/ 	return 0;
/*?*/ }

/*?*/ IMPL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void*, EMPTYARG )
/*?*/ {
/*?*/ 	// wenn wir von hier kommen, kann es kein Fehler mehr sein
/*?*/ 	if( pThis->bInNewData )
/*?*/ 		return 0;
/*?*/ 
/*?*/ 	pThis->bInNewData = TRUE;
/*?*/ 	pThis->bLoadError = FALSE;
/*?*/ 
/*?*/ 	if( !pThis->pDownLoadData )
/*?*/ 	{
/*?*/ 		pThis->pDownLoadData = new Impl_DownLoadData(
/*?*/ 						STATIC_LINK( pThis, SvFileObject, LoadGrfNewData_Impl ) );
/*?*/ 
/*?*/ 		// Null-Link setzen, damit keine temporaeren Grafiken
/*?*/ 		// rausgeswapt werden; der Filter prueft, ob schon
/*?*/ 		// ein Link gesetzt ist => falls dies zutrifft, wird
/*?*/ 		// _kein_ neuer Link gesetzt; der Link muss hier gesetzt werden,
/*?*/ 		// (bevor das erste Mal gefiltert wird), um zu verhindern,
/*?*/ 		// dass der Kontext zurueckgesetzt wird (aynchrones Laden)
/*?*/ 		if( !pThis->bNativFormat )
/*?*/ 		{
/*?*/ 			static GfxLink aDummyLink;
/*?*/ 			pThis->pDownLoadData->aGrf.SetLink( aDummyLink );
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	pThis->NotifyDataChanged();
/*?*/ 
/*?*/ 	SvStream* pStrm = pThis->xMed.Is() ? pThis->xMed->GetInStream() : 0;
/*?*/ 	if( pStrm && pStrm->GetError() )
/*?*/ 	{
/*?*/ 		if( ERRCODE_IO_PENDING == pStrm->GetError() )
/*?*/ 			pStrm->ResetError();
/*?*/ 
/*?*/ 		// im DataChanged ein DataReady?
/*?*/ 		else if( pThis->bWaitForData && pThis->pDownLoadData )
/*?*/ 		{
/*?*/ 			pThis->bLoadError = TRUE;
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	if( pThis->bDataReady )
/*?*/ 	{
/*?*/ 		// Grafik ist fertig, also DataChanged von der Status-
/*?*/ 		// aederung schicken:
/*?*/ 		pThis->SendStateChg_Impl( pStrm->GetError() ? STATE_LOAD_ERROR
/*?*/ 													: STATE_LOAD_OK );
/*?*/ 	}
/*?*/ 
/*?*/ 	pThis->bInNewData = FALSE;
/*?*/ 	return 0;
/*?*/ }


/*	[Beschreibung]

    Die Methode stellt fest, ob aus einem DDE-Object die Daten gelesen
    werden kann.
    Zurueckgegeben wird:
        ERRCODE_NONE 			wenn sie komplett gelesen wurde
        ERRCODE_SO_PENDING		wenn sie noch nicht komplett gelesen wurde
        ERRCODE_SO_FALSE		sonst
*/



/*N*/ void SvFileObject::CancelTransfers()
/*N*/ {
/*N*/ 	if( xMed.Is() )
/*?*/ 		xMed->CancelTransfers();
/*N*/ 
/*N*/ 	// und aus dem Cache austragen, wenn man mitten im Laden ist
/*N*/ 	if( !bDataReady )
/*N*/ 	{
/*N*/ 		// nicht noch mal aufsetzen
/*N*/ 		bLoadAgain = bMedUseCache = FALSE;
/*N*/ 		bDataReady = bLoadError = bWaitForData = TRUE;
/*N*/ 		SendStateChg_Impl( STATE_LOAD_ABORT );
/*N*/ 	}
/*N*/ }




/*N*/ void SvFileObject::SendStateChg_Impl( USHORT nState )
/*N*/ {
/*N*/ 	if( !bStateChangeCalled && HasDataLinks() )
/*N*/ 	{
/*N*/ 		Any aAny;
/*N*/ 		aAny <<= ::rtl::OUString::valueOf( (sal_Int32)nState );
/*N*/ 		DataChanged( SotExchange::GetFormatName(
/*N*/ 						SvxLinkManager::RegisterStatusInfoId()), aAny );
/*N*/ 		bStateChangeCalled = TRUE;
/*N*/ 	}
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
