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

#include <com/sun/star/awt/XBitmap.hpp>
#include <vcl/virdev.hxx>
#include <toolkit/unohlp.hxx>

#ifndef _SFXSTYLE_HXX
#include <bf_svtools/style.hxx>
#endif

#include "dialogs.hrc"

#ifndef _XDEF_HXX
#include <bf_svx/xdef.hxx>
#endif

#include "xattr.hxx"
#include "xtable.hxx"
#include "unoprnms.hxx"

#include "unoapi.hxx"

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif
namespace binfilter {

#define GLOBALOVERFLOW

// ---------------
// class XOBitmap
// ---------------

/*************************************************************************
|*
|*    XOBitmap::XOBitmap()
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ XOBitmap::XOBitmap() :
/*N*/ 	eType			( XBITMAP_NONE ),
/*N*/ 	eStyle			( XBITMAP_STRETCH ),
/*N*/ 	pPixelArray		( NULL ),
/*N*/ 	bGraphicDirty	( FALSE )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ XOBitmap::XOBitmap( const Bitmap& rBmp, XBitmapStyle eInStyle ) :
/*N*/ 	eType			( XBITMAP_IMPORT ),
/*N*/ 	eStyle			( eInStyle ),
/*N*/ 	aGraphicObject	( rBmp ),
/*N*/ 	pPixelArray		( NULL ),
/*N*/ 	bGraphicDirty	( FALSE )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ XOBitmap::XOBitmap( const BfGraphicObject& rGraphicObject, XBitmapStyle eInStyle ) :
/*N*/ 	eType			( XBITMAP_IMPORT ),
/*N*/ 	eStyle			( eInStyle ),
/*N*/ 	aGraphicObject	( rGraphicObject ),
/*N*/ 	pPixelArray		( NULL ),
/*N*/ 	bGraphicDirty	( FALSE )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( USHORT* pArray, const Color& aPixelColor,
|*          const Color& aBckgrColor, const Size& rSize = Size( 8, 8 ),
|*          XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*?*/ XOBitmap::XOBitmap( const USHORT* pArray, const Color& rPixelColor,
/*?*/ 			const Color& rBckgrColor, const Size& rSize,
/*?*/ 			XBitmapStyle eInStyle ) :
/*?*/ 	eStyle			( eInStyle ),
/*?*/ 	pPixelArray		( NULL ),
/*?*/ 	aArraySize		( rSize ),
/*?*/ 	aPixelColor		( rPixelColor ),
/*?*/ 	aBckgrColor		( rBckgrColor ),
/*?*/ 	bGraphicDirty	( TRUE )
/*?*/ 
/*?*/ {
/*?*/ 	if( aArraySize.Width() == 8 && aArraySize.Height() == 8 )
/*?*/ 	{
/*?*/ 		eType = XBITMAP_8X8;
/*?*/ 		pPixelArray = new USHORT[ 64 ];
/*?*/ 
/*?*/ 		for( USHORT i = 0; i < 64; i++ )
/*?*/ 			*( pPixelArray + i ) = *( pArray + i );
/*?*/ 	}
/*?*/ 	else
/*?*/ 		DBG_ASSERT( 0, "Nicht unterstuetzte Bitmapgroesse" );
/*?*/ }

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( const XOBitmap& rXBmp )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ XOBitmap::XOBitmap( const XOBitmap& rXBmp ) :
/*N*/ 	pPixelArray ( NULL )
/*N*/ {
/*N*/ 	eType = rXBmp.eType;
/*N*/ 	eStyle = rXBmp.eStyle;
/*N*/ 	aGraphicObject = rXBmp.aGraphicObject;
/*N*/ 	aArraySize = rXBmp.aArraySize;
/*N*/ 	aPixelColor = rXBmp.aPixelColor;
/*N*/ 	aBckgrColor = rXBmp.aBckgrColor;
/*N*/ 	bGraphicDirty = rXBmp.bGraphicDirty;
/*N*/ 
/*N*/ 	if( rXBmp.pPixelArray )
/*N*/ 	{
/*N*/ 		if( eType == XBITMAP_8X8 )
/*N*/ 		{
/*N*/ 			pPixelArray = new USHORT[ 64 ];
/*N*/ 
/*N*/ 			for( USHORT i = 0; i < 64; i++ )
/*N*/ 				*( pPixelArray + i ) = *( rXBmp.pPixelArray + i );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*    XOBitmap::XOBitmap( Bitmap aBitmap, XBitmapStyle eStyle = XBITMAP_TILE )
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ XOBitmap::~XOBitmap()
/*N*/ {
/*N*/ 	if( pPixelArray )
/*N*/ 		delete []pPixelArray;
/*N*/ }

/*************************************************************************
|*
|*    XOBitmap& XOBitmap::operator=( const XOBitmap& rXBmp )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ XOBitmap& XOBitmap::operator=( const XOBitmap& rXBmp )
/*N*/ {
/*N*/ 	eType = rXBmp.eType;
/*N*/ 	eStyle = rXBmp.eStyle;
/*N*/ 	aGraphicObject = rXBmp.aGraphicObject;
/*N*/ 	aArraySize = rXBmp.aArraySize;
/*N*/ 	aPixelColor = rXBmp.aPixelColor;
/*N*/ 	aBckgrColor = rXBmp.aBckgrColor;
/*N*/ 	bGraphicDirty = rXBmp.bGraphicDirty;
/*N*/ 
/*N*/ 	if( rXBmp.pPixelArray )
/*N*/ 	{
/*?*/ 		if( eType == XBITMAP_8X8 )
/*?*/ 		{
/*?*/ 			pPixelArray = new USHORT[ 64 ];
/*?*/ 
/*?*/ 			for( USHORT i = 0; i < 64; i++ )
/*?*/ 				*( pPixelArray + i ) = *( rXBmp.pPixelArray + i );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return( *this );
/*N*/ }

/*************************************************************************
|*
|*    int XOBitmap::operator==( const XOBitmap& rXOBitmap ) const
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ int XOBitmap::operator==( const XOBitmap& rXOBitmap ) const
/*N*/ {
/*N*/ 	if( eType != rXOBitmap.eType      ||
/*N*/ 		eStyle != rXOBitmap.eStyle         ||
/*N*/ 		!(aGraphicObject == rXOBitmap.aGraphicObject) ||
/*N*/ 		aArraySize != rXOBitmap.aArraySize     ||
/*N*/ 		aPixelColor != rXOBitmap.aPixelColor ||
/*N*/ 		aBckgrColor != rXOBitmap.aBckgrColor ||
/*N*/ 		bGraphicDirty != rXOBitmap.bGraphicDirty )
/*N*/ 	{
/*N*/ 		return( FALSE );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pPixelArray && rXOBitmap.pPixelArray )
/*N*/ 	{
/*N*/ 		USHORT nCount = (USHORT) ( aArraySize.Width() * aArraySize.Height() );
/*N*/ 		for( USHORT i = 0; i < nCount; i++ )
/*N*/ 		{
/*?*/ 			if( *( pPixelArray + i ) != *( rXOBitmap.pPixelArray + i ) )
/*?*/ 				return( FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return( TRUE );
/*N*/ }

/*************************************************************************
|*
|*    void SetPixelArray( const USHORT* pArray )
|*
|*    Beschreibung
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ void XOBitmap::SetPixelArray( const USHORT* pArray )
/*N*/ {
/*N*/ 	if( eType == XBITMAP_8X8 )
/*N*/ 	{
/*N*/ 		if( pPixelArray )
/*?*/ 			delete []pPixelArray;
/*N*/ 
/*N*/ 		pPixelArray = new USHORT[ 64 ];
/*N*/ 
/*N*/ 		for( USHORT i = 0; i < 64; i++ )
/*N*/ 			*( pPixelArray + i ) = *( pArray + i );
/*N*/ 
/*N*/ 		bGraphicDirty = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ASSERT( 0, "Nicht unterstuetzter Bitmaptyp" );
/*N*/ }

/*************************************************************************
|*
|*    Bitmap XOBitmap::GetBitmap()
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ Bitmap XOBitmap::GetBitmap() const
/*N*/ {
/*N*/ 	return GetGraphicObject().GetGraphic().GetBitmap();
/*N*/ }

/*************************************************************************
|*
|*    Bitmap XOBitmap::GetGraphicObject()
|*
|*    Beschreibung
|*    Ersterstellung
|*    Letzte Aenderung
|*
*************************************************************************/

/*N*/ const BfGraphicObject& XOBitmap::GetGraphicObject() const
/*N*/ {
/*N*/ 	if( bGraphicDirty )
/*N*/ 		( (XOBitmap*) this )->Array2Bitmap();
/*N*/ 
/*N*/ 	return aGraphicObject;
/*N*/ }

/*************************************************************************
|*
|*    void XOBitmap::Bitmap2Array()
|*
|*    Beschreibung      Umwandlung der Bitmap in Array, Hinter- u.
|*                      Vordergrundfarbe
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ void XOBitmap::Bitmap2Array()
/*N*/ {
/*N*/ 	VirtualDevice   aVD;
/*N*/ 	BOOL            bPixelColor = FALSE;
/*N*/ 	const Bitmap	aBitmap( GetBitmap() );
/*N*/ 	const USHORT	nLines = 8; // von Type abhaengig
/*N*/ 
/*N*/ 	if( !pPixelArray )
/*N*/ 		pPixelArray = new USHORT[ nLines * nLines ];
/*N*/ 
/*N*/ 	aVD.SetOutputSizePixel( aBitmap.GetSizePixel() );
/*N*/ 	aVD.DrawBitmap( Point(), aBitmap );
/*N*/ 	aPixelColor = aBckgrColor = aVD.GetPixel( Point() );
/*N*/ 
/*N*/ 	// Aufbau des Arrays und Ermittlung der Vorder-, bzw.
/*N*/ 	// Hintergrundfarbe
/*N*/ 	for( USHORT i = 0; i < nLines; i++ )
/*N*/ 	{
/*N*/ 		for( USHORT j = 0; j < nLines; j++ )
/*N*/ 		{
/*N*/ 			if ( aVD.GetPixel( Point( j, i ) ) == aBckgrColor )
/*N*/ 				*( pPixelArray + j + i * nLines ) = 0;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				*( pPixelArray + j + i * nLines ) = 1;
/*N*/ 				if( !bPixelColor )
/*N*/ 				{
/*N*/ 					aPixelColor = aVD.GetPixel( Point( j, i ) );
/*N*/ 					bPixelColor = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*    void XOBitmap::Array2Bitmap()
|*
|*    Beschreibung      Umwandlung des Arrays, Hinter- u.
|*                      Vordergrundfarbe in eine Bitmap
|*    Ersterstellung    27.07.95
|*    Letzte Aenderung  27.07.95
|*
*************************************************************************/

/*N*/ void XOBitmap::Array2Bitmap()
/*N*/ {
/*N*/ 	VirtualDevice   aVD;
/*N*/ 	BOOL            bPixelColor = FALSE;
/*N*/ 	USHORT          nLines = 8; // von Type abhaengig
/*N*/ 
/*N*/ 	if( !pPixelArray )
/*N*/ 		return;
/*N*/ 
/*N*/ 	aVD.SetOutputSizePixel( Size( nLines, nLines ) );
/*N*/ 
/*N*/ 	// Aufbau der Bitmap
/*N*/ 	for( USHORT i = 0; i < nLines; i++ )
/*N*/ 	{
/*N*/ 		for( USHORT j = 0; j < nLines; j++ )
/*N*/ 		{
/*N*/ 			if( *( pPixelArray + j + i * nLines ) == 0 )
/*N*/ 				aVD.DrawPixel( Point( j, i ), aBckgrColor );
/*N*/ 			else
/*N*/ 				aVD.DrawPixel( Point( j, i ), aPixelColor );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	aGraphicObject = BfGraphicObject( aVD.GetBitmap( Point(), Size( nLines, nLines ) ) );
/*N*/ 	bGraphicDirty = FALSE;
/*N*/ }

// -----------------------
// class XFillBitmapItem
// -----------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillBitmapItem, NameOrIndex);

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(const XubString& rName,
|*                                 const Bitmap& rTheBitmap)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

/*N*/ XFillBitmapItem::XFillBitmapItem(const XubString& rName,
/*N*/ 							   const XOBitmap& rTheBitmap) :
/*N*/ 	NameOrIndex( XATTR_FILLBITMAP, rName ),
/*N*/ 	aXOBitmap( rTheBitmap )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

/*N*/ XFillBitmapItem::XFillBitmapItem(const XFillBitmapItem& rItem) :
/*N*/ 	NameOrIndex( rItem ),
/*N*/ 	aXOBitmap( rItem.aXOBitmap )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillBitmapItem::XFillBitmapItem(SvStream& rIn)
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

/*N*/ XFillBitmapItem::XFillBitmapItem( SvStream& rIn, USHORT nVer ) :
/*N*/ 	NameOrIndex( XATTR_FILLBITMAP, rIn )
/*N*/ {
/*N*/ 	if( nVer == 0 )
/*N*/ 	{
/*?*/ 		if (!IsIndex())
/*?*/ 		{
/*?*/ 			// Behandlung der alten Bitmaps
/*?*/ 			Bitmap aBmp;
/*?*/ 
/*?*/ 			rIn >> aBmp;
/*?*/ 
/*?*/ 			aXOBitmap.SetBitmap( aBmp );
/*?*/ 			aXOBitmap.SetBitmapStyle( XBITMAP_TILE );
/*?*/ 
/*?*/ 			if( aBmp.GetSizePixel().Width() == 8 &&
/*?*/ 				aBmp.GetSizePixel().Height() == 8 )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 			}
/*?*/ 			else
/*?*/ 				aXOBitmap.SetBitmapType( XBITMAP_IMPORT );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if( nVer == 1 )
/*N*/ 	{
/*N*/ 		if (!IsIndex())
/*N*/ 		{
/*N*/ 			INT16 iTmp;
/*N*/ 			rIn >> iTmp;
/*N*/ 			aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
/*N*/ 			rIn >> iTmp;
/*N*/ 			aXOBitmap.SetBitmapType( (XBitmapType) iTmp );
/*N*/ 
/*N*/ 			if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
/*N*/ 			{
/*N*/ 				Bitmap aBmp;
/*N*/ 				rIn >> aBmp;
/*N*/ 				aXOBitmap.SetBitmap( aBmp );
/*N*/ 			}
/*N*/ 			else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
/*N*/ 			{
/*N*/ 				USHORT* pArray = new USHORT[ 64 ];
/*N*/ 				Color   aColor;
/*N*/ 
/*N*/ 				for( USHORT i = 0; i < 64; i++ )
/*N*/ 					rIn >> *( pArray + i );
/*N*/ 				aXOBitmap.SetPixelArray( pArray );
/*N*/ 
/*N*/ 				rIn >> aColor;
/*N*/ 				aXOBitmap.SetPixelColor( aColor );
/*N*/ 				rIn >> aColor;
/*N*/ 				aXOBitmap.SetBackgroundColor( aColor );
/*N*/ 
/*N*/ 				delete []pArray;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// #81908# force bitmap to exist
/*N*/ 	aXOBitmap.GetBitmap();
/*N*/ }

//*************************************************************************

/*N*/ XFillBitmapItem::XFillBitmapItem( SfxItemPool* pPool, const XOBitmap& rTheBitmap )
/*N*/ : 	NameOrIndex( XATTR_FILLBITMAP, -1 ),
/*N*/ 	aXOBitmap( rTheBitmap )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*    XFillBitmapItem::Clone(SfxItemPool* pPool) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBitmapItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillBitmapItem(*this);
/*N*/ }

/*************************************************************************
|*
|*    int XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ int XFillBitmapItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			 aXOBitmap == ((const XFillBitmapItem&) rItem).aXOBitmap );
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XFillBitmapItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  17.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillBitmapItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillBitmapItem( rIn, nVer );
/*N*/ }

/*************************************************************************
|*
|*    SfxPoolItem* XFillBitmapItem::Store(SvStream& rOut) const
|*
|*    Beschreibung
|*    Ersterstellung    17.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

/*N*/ SvStream& XFillBitmapItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		rOut << (INT16) aXOBitmap.GetBitmapStyle();
/*N*/ 		if( !aXOBitmap.GetBitmap() )
/*N*/ 			rOut << (INT16) XBITMAP_NONE;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			rOut << (INT16) aXOBitmap.GetBitmapType();
/*N*/ 			if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
/*N*/ 			{
/*N*/ 				const USHORT    nOldComprMode = rOut.GetCompressMode();
/*N*/ 				USHORT          nNewComprMode = nOldComprMode;
/*N*/ 
/*N*/ 				if( rOut.GetVersion() >= SOFFICE_FILEFORMAT_50 )
/*N*/ 					nNewComprMode |= COMPRESSMODE_ZBITMAP;
/*N*/ 				else
/*N*/ 					nNewComprMode &= ~COMPRESSMODE_ZBITMAP;
/*N*/ 
/*N*/ 				rOut.SetCompressMode( nNewComprMode );
/*N*/ 				rOut << aXOBitmap.GetBitmap();
/*N*/ 				rOut.SetCompressMode( nOldComprMode );
/*N*/ 			}
/*N*/ 			else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
/*N*/ 			{
/*N*/ 				USHORT* pArray = aXOBitmap.GetPixelArray();
/*N*/ 				for( USHORT i = 0; i < 64; i++ )
/*N*/ 					rOut << (USHORT) *( pArray + i );
/*N*/ 
/*N*/ 				rOut << aXOBitmap.GetPixelColor();
/*N*/ 				rOut << aXOBitmap.GetBackgroundColor();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*    const Bitmap& XFillBitmapItem::GetValue(const XBitmapTable* pTable) const
|*
|*    Beschreibung
|*    Ersterstellung    15.11.94
|*    Letzte Aenderung  26.07.94
|*
*************************************************************************/

/*N*/ const XOBitmap& XFillBitmapItem::GetValue(const XBitmapTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aXOBitmap;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetXBitmap();
/*N*/ }


/*************************************************************************
|*
|*    USHORT XFillBitmapItem::GetVersion() const
|*
|*    Beschreibung
|*    Ersterstellung    26.07.95
|*    Letzte Aenderung  26.07.95
|*
*************************************************************************/

/*N*/ USHORT XFillBitmapItem::GetVersion( USHORT nFileFormatVersion ) const
/*N*/ {
/*N*/ 	// 2. Version
/*N*/ 	return( 1 );
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ sal_Bool XFillBitmapItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*N*/ 		::rtl::OUString aApiName;
/*N*/ 		SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 		rVal <<= aApiName;
/*N*/ 	}
/*N*/ 	else if( nMemberId == MID_GRAFURL )
/*N*/ 	{
/*N*/ 		XOBitmap aXOBitmap( GetValue() );
/*N*/ 		::rtl::OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
/*N*/ 		aURL += ::rtl::OUString::createFromAscii( aXOBitmap.GetGraphicObject().GetUniqueID().GetBuffer() );
/*N*/ 		rVal <<= aURL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		XOBitmap aXOBitmap( GetValue() );
/*?*/ 		Bitmap aBmp( aXOBitmap.GetBitmap() );
/*?*/ 		BitmapEx aBmpEx( aBmp );
/*?*/ 
/*?*/ 		::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp(
/*?*/ 			VCLUnoHelper::CreateBitmap( aBmpEx ) );
/*?*/ 
/*?*/ 		rVal <<= xBmp;
/*N*/ 	}
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ sal_Bool XFillBitmapItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*?*/ 		::rtl::OUString aName;
/*?*/ 		if(rVal >>= aName)
/*?*/ 		{
/*?*/ 			SetName( aName );
/*?*/ 			return sal_True;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if( nMemberId == MID_GRAFURL )
/*N*/ 	{
/*N*/ 		::rtl::OUString aURL;
/*N*/ 		if(rVal >>= aURL)
/*N*/ 		{
/*N*/ 			BfGraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
/*N*/ 			XOBitmap aBMP( aGrafObj );
/*N*/ 			SetValue( aBMP );
/*N*/ 			return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBmp;
/*?*/ 		if( rVal >>= xBmp)
/*?*/ 		{
/*?*/ 			BitmapEx aInputEx( VCLUnoHelper::GetBitmap( xBmp ) );
/*?*/ 			Bitmap aInput( aInputEx.GetBitmap() );
/*?*/ 
/*?*/ 			// Bitmap einsetzen
/*?*/ 			aXOBitmap.SetBitmap( aInput );
/*?*/ 			aXOBitmap.SetBitmapType(XBITMAP_IMPORT);
/*?*/ 
/*?*/ 			if(aInput.GetSizePixel().Width() == 8
/*?*/ 				&& aInput.GetSizePixel().Height() == 8
/*?*/ 				&& aInput.GetColorCount() == 2)
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 			}
/*?*/ 			return sal_True;
/*?*/ 		}
/*?*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*N*/ BOOL XFillBitmapItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
/*N*/ {
/*N*/ 	return ((XFillBitmapItem*)p1)->GetValue().GetGraphicObject().GetUniqueID() ==
/*N*/ 		   ((XFillBitmapItem*)p2)->GetValue().GetGraphicObject().GetUniqueID();
/*N*/ }

/*N*/ XFillBitmapItem* XFillBitmapItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		const String aUniqueName = NameOrIndex::CheckNamedItem(	this,
/*N*/ 																XATTR_FILLBITMAP,
/*N*/ 																&pModel->GetItemPool(),
/*N*/ 																pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
/*N*/ 																XFillBitmapItem::CompareValueFunc,
/*N*/ 																RID_SVXSTR_BMP21,
/*N*/ 																pModel->GetBitmapList() );
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() )
/*N*/ 		{
/*N*/ 			return new XFillBitmapItem( aUniqueName, aXOBitmap );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XFillBitmapItem*)this;
/*N*/ }
}
