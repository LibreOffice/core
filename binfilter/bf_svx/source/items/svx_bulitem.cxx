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

#include <vcl/outdev.hxx>

#define _SVX_BULITEM_CXX

#include "bulitem.hxx"

// #90477#
#include <tools/tenccvt.hxx>
namespace binfilter {

#define BULITEM_VERSION		((USHORT)2)

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1(SvxBulletItem,SfxPoolItem);

// -----------------------------------------------------------------------

/*N*/ void SvxBulletItem::StoreFont( SvStream& rStream, const Font& rFont )
/*N*/ {
/*N*/ 	USHORT nTemp;
/*N*/ 
/*N*/ 	rStream << rFont.GetColor();
/*N*/ 	nTemp = (USHORT)rFont.GetFamily(); rStream << nTemp;
/*N*/ 
/*N*/ 	// #90477# nTemp = (USHORT)GetStoreCharSet( rFont.GetCharSet(), rStream.GetVersion() );
/*N*/ 	nTemp = (USHORT)GetSOStoreTextEncoding((rtl_TextEncoding)rFont.GetCharSet(), (sal_uInt16)rStream.GetVersion());
/*N*/ 	rStream << nTemp;
/*N*/ 
/*N*/ 	nTemp = (USHORT)rFont.GetPitch(); rStream << nTemp;
/*N*/ 	nTemp = (USHORT)rFont.GetAlign(); rStream << nTemp;
/*N*/ 	nTemp = (USHORT)rFont.GetWeight(); rStream << nTemp;
/*N*/ 	nTemp = (USHORT)rFont.GetUnderline(); rStream << nTemp;
/*N*/ 	nTemp = (USHORT)rFont.GetStrikeout(); rStream << nTemp;
/*N*/ 	nTemp = (USHORT)rFont.GetItalic(); rStream << nTemp;
/*N*/ 
/*N*/ 	// UNICODE: rStream << rFont.GetName();
/*N*/ 	rStream.WriteByteString(rFont.GetName());
/*N*/ 
/*N*/ 	rStream << rFont.IsOutline();
/*N*/ 	rStream << rFont.IsShadow();
/*N*/ 	rStream << rFont.IsTransparent();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ Font SvxBulletItem::CreateFont( SvStream& rStream, USHORT nVer )
/*N*/ {
/*N*/ 	Font aFont;
/*N*/ 	Color aColor;
/*N*/ 	rStream >> aColor;    aFont.SetColor( aColor );
/*N*/ 	USHORT nTemp;
/*N*/ 	rStream >> nTemp; aFont.SetFamily((FontFamily)nTemp);
/*N*/ 
/*N*/ 	// #90477#
/*N*/ 	rStream >> nTemp; 
/*N*/ 	nTemp = (sal_uInt16)GetSOLoadTextEncoding((rtl_TextEncoding)nTemp, (sal_uInt16)rStream.GetVersion());
/*N*/ 	aFont.SetCharSet((rtl_TextEncoding)nTemp);
/*N*/ 	
/*N*/ 	rStream >> nTemp; aFont.SetPitch((FontPitch)nTemp);
/*N*/ 	rStream >> nTemp; aFont.SetAlign((FontAlign)nTemp);
/*N*/ 	rStream >> nTemp; aFont.SetWeight((FontWeight)nTemp);
/*N*/ 	rStream >> nTemp; aFont.SetUnderline((FontUnderline)nTemp);
/*N*/ 	rStream >> nTemp; aFont.SetStrikeout((FontStrikeout)nTemp);
/*N*/ 	rStream >> nTemp; aFont.SetItalic((FontItalic)nTemp);
/*N*/ 
/*N*/ 	// UNICODE: rStream >> aName; aFont.SetName( aName );
/*N*/ 	String aName;
/*N*/ 	rStream.ReadByteString(aName);
/*N*/ 	aFont.SetName( aName );
/*N*/ 
/*N*/ 	if( nVer == 1 )
/*N*/ 	{
/*N*/ 		long nHeight, nWidth;
/*?*/ 		rStream >> nHeight; rStream >> nWidth; Size aSize( nWidth, nHeight );
/*?*/ 		aFont.SetSize( aSize );
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bTemp;
/*N*/ 	rStream >> bTemp; aFont.SetOutline( bTemp );
/*N*/ 	rStream >> bTemp; aFont.SetShadow( bTemp );
/*N*/ 	rStream >> bTemp; aFont.SetTransparent( bTemp );
/*N*/ 	return aFont;
/*N*/ }


// -----------------------------------------------------------------------

/*N*/ SvxBulletItem::SvxBulletItem( USHORT nWhich ) : SfxPoolItem( nWhich )
/*N*/ {
/*N*/ 	SetDefaultFont_Impl();
/*N*/ 	SetDefaults_Impl();
/*N*/ 	nValidMask = 0xFFFF;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SvxBulletItem::SvxBulletItem( SvStream& rStrm, USHORT nWhich ) : 
/*N*/     SfxPoolItem( nWhich ), 
/*N*/     pGraphicObject( NULL )
/*N*/ {
/*N*/ 	rStrm >> nStyle;
/*N*/ 
/*N*/ 	if( nStyle != BS_BMP )
/*N*/ 		aFont = CreateFont( rStrm, BULITEM_VERSION );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Sicheres Laden mit Test auf leere Bitmap
/*?*/ 		Bitmap			aBmp;
/*?*/ 		const UINT32	nOldPos = rStrm.Tell();
/*?*/         // #69345# Errorcode beim Bitmap lesen ignorieren,
/*?*/ 		// siehe Kommentar #67581# in SvxBulletItem::Store()
/*?*/ 		BOOL bOldError = rStrm.GetError() ? TRUE : FALSE;
/*?*/ 		rStrm >> aBmp;
/*?*/ 		if ( !bOldError && rStrm.GetError() )
/*?*/ 		{
/*?*/ 			rStrm.ResetError();
/*?*/ 			// #71493# Keine Warnung: Das BulletItem interessiert seit 5.0 im Dateiformat nicht mehr.
/*?*/ 			// rStrm.SetError(ERRCODE_CLASS_READ | ERRCODE_SVX_BULLETITEM_NOBULLET | ERRCODE_WARNING_MASK);
/*?*/ 		}
/*?*/ 
/*?*/ 		if( aBmp.IsEmpty() )
/*?*/ 		{
/*?*/ 			rStrm.Seek( nOldPos );
/*?*/ 			nStyle = BS_NONE;
/*?*/ 		}
/*?*/ 		else
/*?*/             pGraphicObject = new BfGraphicObject( aBmp );
/*N*/ 	}
/*N*/ 	
/*N*/     rStrm >> nWidth;
/*N*/ 	rStrm >> nStart;
/*N*/ 	rStrm >> nJustify;
/*N*/ 
/*N*/ 	char cTmpSymbol;
/*N*/ 	rStrm >> cTmpSymbol;
/*N*/ 	cSymbol = ByteString::ConvertToUnicode( cTmpSymbol, aFont.GetCharSet() );
/*N*/ 
/*N*/ 	rStrm >> nScale;
/*N*/ 
/*N*/ 	// UNICODE: rStrm >> aPrevText;
/*N*/ 	rStrm.ReadByteString(aPrevText);
/*N*/ 
/*N*/ 	// UNICODE: rStrm >> aFollowText;
/*N*/ 	rStrm.ReadByteString(aFollowText);
/*N*/ 
/*N*/ 	nValidMask = 0xFFFF;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBulletItem::SvxBulletItem( const SvxBulletItem& rItem) : SfxPoolItem( rItem )
/*N*/ {
/*N*/ 	aFont			= rItem.aFont;
/*N*/     pGraphicObject	= ( rItem.pGraphicObject ? new BfGraphicObject( *rItem.pGraphicObject ) : NULL );
/*N*/ 	aPrevText		= rItem.aPrevText;
/*N*/ 	aFollowText		= rItem.aFollowText;
/*N*/ 	nStart			= rItem.nStart;
/*N*/ 	nStyle			= rItem.nStyle;
/*N*/ 	nWidth			= rItem.nWidth;
/*N*/ 	nScale			= rItem.nScale;
/*N*/ 	cSymbol			= rItem.cSymbol;
/*N*/ 	nJustify		= rItem.nJustify;
/*N*/ 	nValidMask		= rItem.nValidMask;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxBulletItem::~SvxBulletItem()
/*N*/ {
/*N*/     if( pGraphicObject )
/*?*/         delete pGraphicObject;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBulletItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxBulletItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxBulletItem::Create( SvStream& rStrm, USHORT nVersion ) const
/*N*/ {
/*N*/ 	return new SvxBulletItem( rStrm, Which() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBulletItem::SetDefaultFont_Impl()
/*N*/ {
/*N*/ 	aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_SYSTEM, 0 );
/*N*/ 	aFont.SetAlign( ALIGN_BOTTOM);
/*N*/ 	aFont.SetTransparent( TRUE );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxBulletItem::SetDefaults_Impl()
/*N*/ {
/*N*/     pGraphicObject  = NULL;
/*N*/ 	nWidth		    = 1200;  // 1.2cm
/*N*/ 	nStart		    = 1;
/*N*/ 	nStyle		    = BS_123;
/*N*/ 	nJustify	    = BJ_HLEFT | BJ_VCENTER;
/*N*/ 	cSymbol		    = sal_Unicode(' ');
/*N*/ 	nScale 		    = 75;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SvxBulletItem::GetVersion( USHORT nVersion ) const
/*N*/ {
/*N*/ 	return BULITEM_VERSION;
/*N*/ }

// -----------------------------------------------------------------------



// -----------------------------------------------------------------------

/*N*/ int SvxBulletItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(rItem.ISA(SvxBulletItem),"operator==Types not matching");
/*N*/ 	const SvxBulletItem& rBullet = (const SvxBulletItem&)rItem;
/*N*/ 	// ValidMask mitvergleichen, da sonst kein Putten in ein AttrSet moeglich,
/*N*/ 	// wenn sich das Item nur in der ValidMask von einem existierenden unterscheidet.
/*N*/ 	if(	nValidMask != rBullet.nValidMask 	||
/*N*/ 		nStyle != rBullet.nStyle 			||
/*N*/ 		nScale != rBullet.nScale			||
/*N*/ 		nJustify != rBullet.nJustify 		||
/*N*/ 		nWidth != rBullet.nWidth 			||
/*N*/ 		nStart != rBullet.nStart 			||
/*N*/ 		cSymbol != rBullet.cSymbol 			||
/*N*/ 		aPrevText != rBullet.aPrevText 		||
/*N*/ 		aFollowText != rBullet.aFollowText )
/*N*/ 			return 0;
/*N*/ 
/*N*/ 	if( ( nStyle != BS_BMP ) && ( aFont != rBullet.aFont ) )
/*N*/ 		return 0;
/*N*/ 	
/*N*/     if( nStyle == BS_BMP )
/*N*/     { 
/*?*/         if( ( pGraphicObject && !rBullet.pGraphicObject ) || ( !pGraphicObject && rBullet.pGraphicObject ) )
/*?*/             return 0;
/*?*/  
/*?*/         if( ( pGraphicObject && rBullet.pGraphicObject ) &&
/*?*/             ( ( !(*pGraphicObject == *rBullet.pGraphicObject) ) ||
/*?*/               ( pGraphicObject->GetPrefSize() != rBullet.pGraphicObject->GetPrefSize() ) ) )
/*?*/         {
/*?*/             return 0;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	return 1;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxBulletItem::Store( SvStream& rStrm, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	// Korrektur bei leerer Bitmap
/*N*/ 	if( ( nStyle == BS_BMP ) && 
/*N*/         ( !pGraphicObject || ( GRAPHIC_NONE == pGraphicObject->GetType() ) || ( GRAPHIC_DEFAULT == pGraphicObject->GetType() ) ) )
/*N*/ 	{
/*?*/         if( pGraphicObject )
/*?*/         {
/*?*/             delete( const_cast< SvxBulletItem* >( this )->pGraphicObject );
/*?*/             const_cast< SvxBulletItem* >( this )->pGraphicObject = NULL;
/*?*/         }
/*?*/ 		
/*?*/         const_cast< SvxBulletItem* >( this )->nStyle = BS_NONE;
/*N*/ 	}
/*N*/ 
/*N*/ 	rStrm << nStyle;
/*N*/ 
/*N*/ 	if( nStyle != BS_BMP )
/*N*/ 		StoreFont( rStrm, aFont );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		ULONG nStart = rStrm.Tell();
/*?*/ 
/*?*/ 		// Kleine Vorab-Schaetzung der Groesse...
/*?*/ 		USHORT nFac = ( rStrm.GetCompressMode() != COMPRESSMODE_NONE ) ? 3 : 1;
/*?*/ 		const Bitmap aBmp( pGraphicObject->GetGraphic().GetBitmap() );
/*?*/ 		ULONG nBytes = aBmp.GetSizeBytes();
/*?*/ 		if ( nBytes < ULONG(0xFF00*nFac) )
/*?*/ 			rStrm << aBmp;
/*?*/ 
/*?*/ 		ULONG nEnd = rStrm.Tell();
/*?*/ 		// #67581# Item darf mit Overhead nicht mehr als 64K schreiben,
/*?*/ 		// sonst platzt der SfxMultiRecord
/*?*/ 		// Dann lieber auf die Bitmap verzichten, ist nur fuer Outliner
/*?*/ 		// und auch nur fuer <= 5.0 wichtig.
/*?*/ 		// Beim Einlesen merkt der Stream-Operator der Bitmap, dass dort keine steht.
/*?*/ 		// Hiermit funktioniert jetzt der Fall das die grosse Bitmap aus einem anderen
/*?*/ 		// Fileformat entstanden ist, welches keine 64K belegt, aber wenn eine
/*?*/ 		// Bitmap > 64K verwendet wird, hat das SvxNumBulletItem beim Laden ein Problem,
/*?*/ 		// stuerzt aber nicht ab.
/*?*/ 
/*?*/ 		if ( (nEnd-nStart) > 0xFF00 )
/*?*/ 			rStrm.Seek( nStart );
/*N*/ 	}
/*N*/ 	rStrm << nWidth;
/*N*/ 	rStrm << nStart;
/*N*/ 	rStrm << nJustify;
/*N*/ 	rStrm << (char)ByteString::ConvertFromUnicode( cSymbol, aFont.GetCharSet() );
/*N*/ 	rStrm << nScale;
/*N*/ 
/*N*/ 	// UNICODE: rStrm << aPrevText;
/*N*/ 	rStrm.WriteByteString(aPrevText);
/*N*/ 
/*N*/ 	// UNICODE: rStrm << aFollowText;
/*N*/ 	rStrm.WriteByteString(aFollowText);
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
