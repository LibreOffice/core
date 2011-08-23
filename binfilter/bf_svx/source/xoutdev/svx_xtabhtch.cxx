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

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include "XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>

#include "xmlxtimp.hxx"

#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xiocomp.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"
#include "dlgutil.hxx"

#include <xflhtit.hxx>

#include <xflclit.hxx>

#include <xfillit0.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;

sal_Unicode const pszExtHatch[]	 = {'s','o','h'};

char const aChckHatch[]  = { 0x04, 0x00, 'S','O','H','L'};	// < 5.2
char const aChckHatch0[] = { 0x04, 0x00, 'S','O','H','0'};	// = 5.2
char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// ------------------
// class XHatchTable
// ------------------

/************************************************************************/

/*N*/ XHatchTable::~XHatchTable()
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchTable::Replace(long nIndex, XHatchEntry* pEntry )
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyTable::Replace(nIndex, pEntry);
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchTable::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyTable::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchTable::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyTable::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchTable::Load()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchTable::Save()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchTable::Create()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchTable::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XHatchTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	return( NULL );
/*N*/ }

// -----------------
// class XHatchList
// -----------------

/*************************************************************************
|*
|* XHatchList::XHatchList()
|*
*************************************************************************/

/*N*/ XHatchList::XHatchList( const String& rPath,
/*N*/ 							XOutdevItemPool* pInPool,
/*N*/ 							USHORT nInitSize, USHORT nReSize ) :
/*N*/ 				XPropertyList	( rPath, pInPool, nInitSize, nReSize),
/*N*/ 				pVD				( NULL ),
/*N*/ 				pXOut			( NULL ),
/*N*/ 				pXFSet			( NULL )
/*N*/ {
/*N*/ 	pBmpList = new List( nInitSize, nReSize );
/*N*/ }

/************************************************************************/

/*N*/ XHatchList::~XHatchList()
/*N*/ {
/*N*/ 	if( pVD )    delete pVD;
/*N*/ 	if( pXOut )  delete pXOut;
/*N*/ 	if( pXFSet ) delete pXFSet;
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchList::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyList::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XHatchEntry* XHatchList::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XHatchEntry*) XPropertyList::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchList::Load()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( bListDirty )
/*N*/ 	{
/*N*/ 		bListDirty = FALSE;
/*N*/ 
/*N*/ 		INetURLObject aURL( aPath );
/*N*/ 
/*N*/ 		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
/*N*/ 		{
/*N*/ 			DBG_ASSERT( !aPath.Len(), "invalid URL" );
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 		aURL.Append( aName );
/*N*/ 
/*N*/ 		if( !aURL.getExtension().getLength() )
/*N*/ 			aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );
/*N*/ 
/*N*/ 		// check if file exists, SfxMedium shows an errorbox else
/*N*/ 		{
/*N*/ 			::com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionHandler > xHandler;
/*N*/ 			SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );
/*N*/ 
/*N*/ 			sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);
/*N*/ 
/*N*/ 			if( pIStm )
/*N*/ 				delete pIStm;
/*N*/ 
/*N*/ 			if( !bOk )
/*N*/ 				return sal_False;
/*N*/ 		}
/*N*/ 
/*N*/ 		{
/*N*/ 			SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE, TRUE );
/*N*/ 			SvStream* pStream = aMedium.GetInStream();
/*N*/ 			if( !pStream )
/*N*/ 				return( FALSE );
/*N*/ 
/*N*/ 			char aCheck[6];
/*N*/ 			pStream->Read( aCheck, 6 );
/*N*/ 
/*N*/ 			// Handelt es sich um die gew"unschte Tabelle?
/*N*/ 			if( memcmp( aCheck, aChckHatch, sizeof( aChckHatch ) ) == 0 ||
/*N*/ 				memcmp( aCheck, aChckHatch0, sizeof( aChckHatch0 ) ) == 0 )
/*N*/ 			{
/*N*/ 				ImpRead( *pStream );
/*N*/ 				return( pStream->GetError() == SVSTREAM_OK );
/*N*/ 			}
/*N*/ 			else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
/*N*/ 			{
/*N*/ 				return FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
/*N*/ 		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchList::Save()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return false;
/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckHatch0, 4 );
    pStream->WriteByteString(String( pszChckHatch0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
*/
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchList::Create()
/*N*/ {
/*N*/ 	XubString aStr( SVX_RES( RID_SVXSTR_HATCH ) );
/*N*/ 	xub_StrLen nLen;
/*N*/ 
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLACK),XHATCH_SINGLE,100,  0),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(new XHatchEntry(XHatch(RGB_Color(COL_RED  ),XHATCH_DOUBLE, 80,450),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLUE ),XHATCH_TRIPLE,120,  0),aStr));
/*N*/ 
/*N*/ 	return( TRUE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XHatchList::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	for( long i = 0; i < Count(); i++)
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
/*N*/ 		DBG_ASSERT( pBmp, "XHatchList: Bitmap(UI) konnte nicht erzeugt werden!" );
/*N*/ 
/*N*/ 		if( pBmp )
/*N*/ 			pBmpList->Insert( pBmp, i );
/*N*/ 	}
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 	if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 	if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 
/*N*/ 	return( TRUE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XHatchList::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	Point	aZero;
/*N*/ 
/*N*/ 	if( !pVD ) // und pXOut und pXFSet
/*N*/ 	{
/*N*/ 		pVD = new VirtualDevice;
/*N*/ 		DBG_ASSERT( pVD, "XHatchList: Konnte kein VirtualDevice erzeugen!" );
/*N*/ 		//pVD->SetMapMode( MAP_100TH_MM );
/*N*/ 		//pVD->SetOutputSize( pVD->PixelToLogic( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) ) );
/*N*/ 		pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );
/*N*/ 
/*N*/ 		pXOut = new XOutputDevice( pVD );
/*N*/ 		DBG_ASSERT( pVD, "XHatchList: Konnte kein XOutDevice erzeugen!" );
/*N*/ 
/*N*/ 		pXFSet = new XFillAttrSetItem( pXPool );
/*N*/ 		DBG_ASSERT( pVD, "XHatchList: Konnte kein XFillAttrSetItem erzeugen!" );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( Application::GetSettings().GetStyleSettings().GetHighContrastMode() != 0 )
/*N*/ 		pVD->SetDrawMode( OUTPUT_DRAWMODE_CONTRAST );
/*N*/ 	else
/*N*/ 		pVD->SetDrawMode( OUTPUT_DRAWMODE_COLOR );
/*N*/ 
/*N*/ 	// Damit die Schraffuren mit Rahmen angezeigt werden:
/*N*/ 	// MapMode-Aenderungen (100th mm <--> Pixel)
/*N*/ 	Size aPixelSize = pVD->GetOutputSizePixel();
/*N*/ 	pVD->SetMapMode( MAP_PIXEL );
/*N*/ 
/*N*/ 	pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
/*N*/ 	pXFSet->GetItemSet().Put( XFillColorItem( String(), RGB_Color( COL_WHITE ) ) );
/*N*/ 
/*N*/ //-/	pXOut->SetFillAttr( *pXFSet );
/*N*/ 	pXOut->SetFillAttr( pXFSet->GetItemSet() );
/*N*/ 
/*N*/ 	// #73550#
/*N*/ 	pXOut->OverrideLineColor( Color( COL_BLACK ) );
/*N*/ 	
/*N*/ 	pXOut->DrawRect( Rectangle( aZero, aPixelSize ) );
/*N*/ 
/*N*/ 	pVD->SetMapMode( MAP_100TH_MM );
/*N*/ 	Size aVDSize = pVD->GetOutputSize();
/*N*/ 	// 1 Pixel (Rahmen) abziehen
/*N*/ 	aVDSize.Width() -= (long) ( aVDSize.Width() / aPixelSize.Width() + 1 );
/*N*/ 	aVDSize.Height() -= (long) ( aVDSize.Height() / aPixelSize.Height() + 1 );
/*N*/ 
/*N*/ 	pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_HATCH ) );
/*N*/ 	pXFSet->GetItemSet().Put( XFillHatchItem( String(), Get( nIndex )->GetHatch() ) );

//-/	pXOut->SetFillAttr( *pXFSet );
/*N*/ 	pXOut->SetFillAttr( pXFSet->GetItemSet() );
/*N*/ 
/*N*/ 	pXOut->DrawRect( Rectangle( aZero, aVDSize ) );
/*N*/ 
/*N*/ 	Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, pVD->GetOutputSize() ) );
/*N*/ 
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( bDelete )
/*N*/ 	{
/*N*/ 		if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 		if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 		if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 	}
/*N*/ 
/*N*/ 	return( pBitmap );
/*N*/ }

/************************************************************************/

/*N*/ XubString& XHatchList::ConvertName( XubString& rStrName )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	for( USHORT i=0; i<(RID_SVXSTR_HATCH_DEF_END-RID_SVXSTR_HATCH_DEF_START+1) && !bFound; i++ )
/*N*/ 	{
/*N*/ 		XubString aStrDefName =	SVX_RESSTR( RID_SVXSTR_HATCH_DEF_START + i );
/*N*/ 		if( rStrName.Search( aStrDefName ) == 0 )
/*N*/ 		{
/*N*/ 			rStrName.Replace( 0, aStrDefName.Len(),	SVX_RESSTR( RID_SVXSTR_HATCH_START + i ) );
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rStrName;
/*N*/ }

/************************************************************************/

/*N*/ SvStream& XHatchList::ImpRead( SvStream& rIn )
/*N*/ {
/*N*/ 	// Lesen
/*N*/ 	rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
/*N*/ 
/*N*/ 	delete pBmpList;
/*N*/ 	pBmpList = new List( 16, 16 );
/*N*/ 
/*N*/ 	XHatchEntry* pEntry = NULL;
/*N*/ 	long		nCount;
/*N*/ 	XubString		aName;
/*N*/ 
/*N*/ 	long		nStyle;
/*N*/ 	USHORT		nRed;
/*N*/ 	USHORT		nGreen;
/*N*/ 	USHORT		nBlue;
/*N*/ 	long		nDistance;
/*N*/ 	long		nAngle;
/*N*/ 	Color		aColor;
/*N*/ 
/*N*/ 	rIn >> nCount;
/*N*/ 
/*N*/ 	if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
/*N*/ 	{
/*N*/ 		for( long nIndex = 0; nIndex < nCount; nIndex++ )
/*N*/ 		{
/*N*/ 			// UNICODE:rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 
/*N*/ 			aName = ConvertName( aName );
/*N*/ 			rIn >> nStyle;
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			rIn >> nDistance;
/*N*/ 			rIn >> nAngle;
/*N*/ 
/*N*/ 			aColor = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
/*N*/ 			pEntry = new XHatchEntry (aHatch, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else // ab 3.00a
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 
/*N*/ 		for( long nIndex = 0; nIndex < nCount; nIndex++ )
/*N*/ 		{
/*N*/ 			// Versionsverwaltung
/*N*/ 			XIOCompat aIOC( rIn, STREAM_READ );
/*N*/ 
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 
/*N*/ 			aName = ConvertName( aName );
/*N*/ 			rIn >> nStyle;
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			rIn >> nDistance;
/*N*/ 			rIn >> nAngle;
/*N*/ 
/*N*/ 			if (aIOC.GetVersion() > 0)
/*N*/ 			{
/*N*/ 				// lesen neuer Daten ...
/*N*/ 			}
/*N*/ 
/*N*/ 			aColor = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
/*N*/ 			pEntry = new XHatchEntry (aHatch, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return( rIn );
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
