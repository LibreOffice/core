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

#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <bf_svtools/pathoptions.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <bf_svx/forbiddencharacterstable.hxx>
#endif

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST 	SID_LINEEND_LIST
#ifndef _SVX_DRAWITEM_HXX
#include <bf_svx/drawitem.hxx>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ const String GetPalettePath()
/*N*/ {
/*N*/ 	SvtPathOptions aPathOpt;
/*N*/ 	return aPathOpt.GetPalettePath();
/*N*/ }

/*N*/ SwDrawDocument::SwDrawDocument( SwDoc* pD ) :
    /*N*/ 	FmFormModel( ::binfilter::GetPalettePath(), &pD->GetAttrPool(),
/*N*/ 				 pD->GetDocShell(), TRUE ),
/*N*/ 	pDoc( pD )
/*N*/ {
/*N*/ 	SetScaleUnit( MAP_TWIP );
/*N*/ 	SetSwapGraphics( TRUE );
/*N*/ 
/*N*/ 	SwDocShell* pDocSh = pDoc->GetDocShell();
/*N*/ 	if ( pDocSh )
/*N*/ 	{
/*N*/ 		SetObjectShell( pDocSh );
/*N*/ 		SvxColorTableItem* pColItem = ( SvxColorTableItem* )
/*N*/ 								( pDocSh->GetItem( ITEMID_COLOR_TABLE ) );
/*N*/ 		XColorTable *pXCol = pColItem ? pColItem->GetColorTable() :
/*N*/ 										OFF_APP()->GetStdColorTable();
/*N*/ 		SetColorTable( pXCol );
/*N*/ 
/*N*/ 		if ( !pColItem )
/*N*/ 			pDocSh->PutItem( SvxColorTableItem( pXCol ) );
/*N*/ 
/*N*/ 		pDocSh->PutItem( SvxGradientListItem( GetGradientList() ));
/*N*/ 		pDocSh->PutItem( SvxHatchListItem( GetHatchList() ) );
/*N*/ 		pDocSh->PutItem( SvxBitmapListItem( GetBitmapList() ) );
/*N*/ 		pDocSh->PutItem( SvxDashListItem( GetDashList() ) );
/*N*/ 		pDocSh->PutItem( SvxLineEndListItem( GetLineEndList() ) );
/*N*/ 		pDocSh->PutItem( SfxUInt16Item(SID_ATTR_LINEEND_WIDTH_DEFAULT, 111) );
/*N*/ 		SetObjectShell( pDocSh );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		SetColorTable( OFF_APP()->GetStdColorTable() );
/*N*/ 
/*N*/ 	// copy all the default values to the SdrModel
/*N*/ 	SfxItemPool* pSdrPool = pD->GetAttrPool().GetSecondaryPool();
/*N*/ 	if( pSdrPool )
/*N*/ 	{
/*N*/ 		const USHORT aWhichRanges[] =
/*N*/ 			{
/*N*/ 				RES_CHRATR_BEGIN, RES_CHRATR_END,
/*N*/ 				RES_PARATR_BEGIN, RES_PARATR_END,
/*N*/ 				0
/*N*/ 			};
/*N*/ 
/*N*/ 		SfxItemPool& rDocPool = pD->GetAttrPool();
/*N*/ 		USHORT nEdtWhich, nSlotId;
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		for( const USHORT* pRangeArr = aWhichRanges;
/*N*/ 			*pRangeArr; pRangeArr += 2 )
/*N*/ 			for( USHORT nW = *pRangeArr, nEnd = *(pRangeArr+1);
/*N*/ 					nW < nEnd; ++nW )
/*N*/ 				if( 0 != (pItem = rDocPool.GetPoolDefaultItem( nW )) &&
/*N*/ 					0 != (nSlotId = rDocPool.GetSlotId( nW ) ) &&
/*N*/ 					nSlotId != nW &&
/*N*/ 					0 != (nEdtWhich = pSdrPool->GetWhich( nSlotId )) &&
/*N*/ 					nSlotId != nEdtWhich )
/*N*/ 				{
/*N*/ 					SfxPoolItem* pCpy = pItem->Clone();
/*N*/ 					pCpy->SetWhich( nEdtWhich );
/*N*/ 					pSdrPool->SetPoolDefaultItem( *pCpy );
/*N*/ 					delete pCpy;
/*N*/ 				}
/*N*/ 	}
/*N*/ 
/*N*/ 	SetForbiddenCharsTable( pD->GetForbiddenCharacterTbl() );
/*N*/ 	// #87795# Implementation for asian compression
/*N*/ 	SetCharCompressType( pD->GetCharCompressType() );
/*N*/ }

/*************************************************************************
|*
|* Konstruktor, fuer einfuegen Document
|*
\************************************************************************/


SwDrawDocument::SwDrawDocument( SfxItemPool *pPool, SwDocShell *pDocSh )
    : FmFormModel( GetPalettePath(), pPool, pDocSh, TRUE ),
    pDoc( pDocSh->GetDoc() )
{
    SetScaleUnit( MAP_TWIP );
    SetDefaultFontHeight( 240 );
    SetSwapGraphics( TRUE );

    ASSERT( pDocSh, "DocShell not found" );
    SvxColorTableItem* pColItem = ( SvxColorTableItem* )
                                ( pDocSh->GetItem( ITEMID_COLOR_TABLE ) );
    XColorTable *pXCol = pColItem ? pColItem->GetColorTable() :
                                    OFF_APP()->GetStdColorTable();
    SetColorTable( pXCol );

    if ( !pColItem )
        pDocSh->PutItem( SvxColorTableItem( pXCol ) );

    // Bug 35371:
    //  fuers "Datei einfuegen" NIE die anderen Items an der DocShell setzen!!!
    // Diese zeigen sonst immer in das temporaere SdrModel !
    SetObjectShell( pDocSh );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/


/*N*/ SwDrawDocument::~SwDrawDocument()
/*N*/ {
/*N*/ 	Broadcast(SdrHint(HINT_MODELCLEARED));
/*N*/ 
/*N*/ 	Clear();
/*N*/ }

/*************************************************************************
|*
|* Diese Methode erzeugt eine neue Seite (SdPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/


/*N*/ SdrPage* SwDrawDocument::AllocPage(FASTBOOL bMasterPage)
/*N*/ {
/*N*/ 	SwDPage* pPage = new SwDPage(*this, bMasterPage);
/*N*/ 	pPage->SetName( String::CreateFromAscii(
/*N*/ 									RTL_CONSTASCII_STRINGPARAM( "Controls" )) );
/*N*/ 	return pPage;
/*N*/ }


/*N*/ SvStream* SwDrawDocument::GetDocumentStream( SdrDocumentStreamInfo& rInfo ) const
/*N*/ {
/*N*/ 	SvStream* pRet = NULL;
/*N*/ 	SvStorageRef xRoot( pDoc->GetDocStorage() );
/*N*/ 	String sDrawStrmNm( String::CreateFromAscii(
/*N*/ 					RTL_CONSTASCII_STRINGPARAM( DRAWING_STREAM_NAME )));
/*N*/ 
/*N*/  	if( xRoot.Is() && SVSTREAM_OK == xRoot->GetError() )
/*N*/     {
/*N*/ 	    if( rInfo.maUserData.Len() &&
/*N*/ 		    ( rInfo.maUserData.GetToken( 0, ':' ) ==
/*N*/ 		      String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
/*N*/ 	    {
/*?*/ 		    const String aPicturePath( rInfo.maUserData.GetToken( 1, ':' ) );
/*?*/ 
/*?*/ 		    // graphic from picture stream in picture storage in XML package
/*?*/ 		    if( aPicturePath.GetTokenCount( '/' ) == 2 )
/*?*/ 		    {
/*?*/ 			    SvStorageRef    xPictureStorage;
/*?*/ 			    const String    aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );
/*?*/                 const String    aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );
/*?*/ 
/*?*/ 			    if( xRoot->IsContained( aPictureStorageName ) &&
/*?*/ 					xRoot->IsStorage( aPictureStorageName )  )
/*?*/ 				{
/*?*/ 					xPictureStorage = xRoot->OpenUCBStorage( aPictureStorageName, 
/*?*/                                                              STREAM_READ | 
/*?*/                                                              STREAM_SHARE_DENYWRITE | 
/*?*/                                                              STREAM_NOCREATE );
/*?*/ 				}
/*?*/ 
/*?*/ 			    if( xPictureStorage.Is() &&
/*?*/ 				    xPictureStorage->IsContained( aPictureStreamName ) &&
/*?*/ 				    xPictureStorage->IsStream( aPictureStreamName ) )
/*?*/ 			    {
/*?*/ 				    pRet = xPictureStorage->OpenSotStream( aPictureStreamName, 
/*?*/                                                            STREAM_READ | 
/*?*/                                                            STREAM_SHARE_DENYWRITE | 
/*?*/                                                            STREAM_NOCREATE );
/*?*/ 
/*?*/ 				    if( pRet )
/*?*/ 				    {
/*?*/ 					    pRet->SetVersion( xPictureStorage->GetVersion() );
/*?*/ 					    pRet->SetKey( xPictureStorage->GetKey() );
/*?*/ 
/*?*/                         rInfo.mbDeleteAfterUse = TRUE;
/*?*/                         rInfo.mpStorageRef = new SvStorageRef( xPictureStorage );
/*?*/ 				    }
/*?*/ 			    }
/*?*/ 		    }
/*N*/ 	    }
/*N*/         else if( xRoot->IsStream( sDrawStrmNm ) )
/*N*/ 	    {
/*N*/ 		    long nFFVersion = xRoot->GetVersion();
/*N*/ 		    ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31 ||
/*N*/ 				    nFFVersion == SOFFICE_FILEFORMAT_40 ||
/*N*/ 				    nFFVersion == SOFFICE_FILEFORMAT_50,
/*N*/ 				    "Am Root-Storage ist keine FF-Version gesetzt!" );
/*N*/ 
/*N*/ 		    // Wenn eine 3.1-Clipboard-ID gesetzt ist, die Fileformat-Version
/*N*/ 		    // auf 3.1 setzten.
/*N*/ 		    if( SOT_FORMATSTR_ID_STARWRITER_30 == xRoot->GetFormat() &&
/*N*/ 			    nFFVersion != SOFFICE_FILEFORMAT_31 )
/*N*/ 		    {
/*?*/ 			    ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31,
/*?*/ 					    "Fileformat-Version auf 3.1 umgesetzt" );
/*?*/ 			    xRoot->SetVersion( nFFVersion = SOFFICE_FILEFORMAT_31 );
/*N*/ 		    }
/*N*/ 		    else if( ( SOT_FORMATSTR_ID_STARWRITER_40 == xRoot->GetFormat() ||
/*N*/ 				       SOT_FORMATSTR_ID_STARWRITERWEB_40 == xRoot->GetFormat() ||
/*N*/ 				       SOT_FORMATSTR_ID_STARWRITERGLOB_40 == xRoot->GetFormat() ) &&
/*N*/ 				     nFFVersion != SOFFICE_FILEFORMAT_40 )
/*N*/ 		    {
/*?*/ 			    ASSERT( nFFVersion == SOFFICE_FILEFORMAT_40,
/*?*/ 					    "Fileformat-Version auf 4.0 umgesetzt" );
/*?*/ 			    xRoot->SetVersion( nFFVersion = SOFFICE_FILEFORMAT_40 );
/*N*/ 		    }
/*N*/ 		    else if( ( SOT_FORMATSTR_ID_STARWRITER_50 == xRoot->GetFormat() ||
/*N*/ 				       SOT_FORMATSTR_ID_STARWRITERWEB_50 == xRoot->GetFormat() ||
/*N*/ 				       SOT_FORMATSTR_ID_STARWRITERGLOB_50 == xRoot->GetFormat() ) &&
/*N*/ 				     nFFVersion != SOFFICE_FILEFORMAT_50 )
/*N*/ 		    {
/*?*/ 			    ASSERT( nFFVersion == SOFFICE_FILEFORMAT_50,
/*?*/ 					    "Fileformat-Version auf 4.0 umgesetzt" );
/*?*/ 			    xRoot->SetVersion( nFFVersion = SOFFICE_FILEFORMAT_50 );
/*N*/ 		    }
/*N*/ 
/*N*/ 		    pRet = xRoot->OpenStream( sDrawStrmNm,
/*N*/ 					    STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
/*N*/ 
/*N*/             if( pRet )
/*N*/                 rInfo.mbDeleteAfterUse = TRUE;
/*N*/ 	    }
/*N*/ 
/*N*/     }
/*N*/ 	return pRet;
/*N*/ }





}
