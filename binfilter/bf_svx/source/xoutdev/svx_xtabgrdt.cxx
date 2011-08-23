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

#ifndef _SVX_XPROPERTYTABLE_HXX
#include "XPropertyTable.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX 
#include <unotools/ucbstreamhelper.hxx>
#endif

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

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <xflgrit.hxx>
#endif
namespace binfilter {

#define GLOBALOVERFLOW

using namespace ::com::sun::star;
using namespace rtl;

sal_Unicode const pszExtGradient[]	= {'s','o','g'};

char const aChckGradient[]  = { 0x04, 0x00, 'S','O','G','L'};	// < 5.2
char const aChckGradient0[] = { 0x04, 0x00, 'S','O','G','0'};	// = 5.2
char const aChckXML[]       = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// ---------------------
// class XGradientTable
// ---------------------

/************************************************************************/

/*N*/ XGradientTable::~XGradientTable()
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientTable::Replace(long nIndex, XGradientEntry* pEntry )
/*N*/ {
/*N*/ 	return (XGradientEntry*) XPropertyTable::Replace(nIndex, pEntry);
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientTable::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XGradientEntry*) XPropertyTable::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientTable::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XGradientEntry*) XPropertyTable::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientTable::Load()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientTable::Save()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientTable::Create()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientTable::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XGradientTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	return( NULL );
/*N*/ }

// --------------------
// class XGradientList
// --------------------

/*************************************************************************
|*
|* XGradientList::XGradientList()
|*
*************************************************************************/

/*N*/ XGradientList::XGradientList( const String& rPath,
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

/*N*/ XGradientList::~XGradientList()
/*N*/ {
/*N*/ 	if( pVD )    delete pVD;
/*N*/ 	if( pXOut )  delete pXOut;
/*N*/ 	if( pXFSet ) delete pXFSet;
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
/*N*/ {
/*N*/ 	return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientList::Remove(long nIndex)
/*N*/ {
/*N*/ 	return( (XGradientEntry*) XPropertyList::Remove( nIndex, 0 ) );
/*N*/ }

/************************************************************************/

/*N*/ XGradientEntry* XGradientList::Get(long nIndex) const
/*N*/ {
/*N*/ 	return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientList::Load()
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
/*N*/ 			aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );
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
/*N*/ 			if( memcmp( aCheck, aChckGradient, sizeof( aChckGradient ) ) == 0 ||
/*N*/ 				memcmp( aCheck, aChckGradient0, sizeof( aChckGradient0 ) ) == 0 )
/*N*/ 			{
/*N*/ 				ImpRead( *pStream );
/*N*/ 				return( pStream->GetError() == SVSTREAM_OK );
/*N*/ 			}
/*N*/ 			else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
/*N*/ 			{
/*N*/ 				return FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
/*N*/ 		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
/*N*/ 
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientList::Save()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return false;

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckGradient0, 4 );
    pStream->WriteByteString(String( pszChckGradient0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
*/
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientList::Create()
/*N*/ {
/*N*/ 	XubString aStr( SVX_RES( RID_SVXSTR_GRADIENT ) );
/*N*/ 	xub_StrLen nLen;
/*N*/ 
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLACK  ),RGB_Color(COL_WHITE	),XGRAD_LINEAR	  ,    0,10,10, 0,100,100),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLUE	 ),RGB_Color(COL_RED	),XGRAD_AXIAL	  ,  300,20,20,10,100,100),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_RED	 ),RGB_Color(COL_YELLOW ),XGRAD_RADIAL	  ,  600,30,30,20,100,100),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_YELLOW ),RGB_Color(COL_GREEN	),XGRAD_ELLIPTICAL,  900,40,40,30,100,100),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_GREEN  ),RGB_Color(COL_MAGENTA),XGRAD_SQUARE	  , 1200,50,50,40,100,100),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(new XGradientEntry(XGradient(RGB_Color(COL_MAGENTA),RGB_Color(COL_YELLOW ),XGRAD_RECT	  , 1900,60,60,50,100,100),aStr));
/*N*/ 
/*N*/ 	return( TRUE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XGradientList::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	for( long i = 0; i < Count(); i++)
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
/*N*/ 		DBG_ASSERT( pBmp, "XGradientList: Bitmap(UI) konnte nicht erzeugt werden!" );
/*N*/ 
/*N*/ 		if( pBmp )
/*N*/ 			pBmpList->Insert( pBmp, i );
/*N*/ 	}
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 	if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 	if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XGradientList::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	if( !pVD ) // und pXOut und pXFSet
/*N*/ 	{
/*N*/ 		pVD = new VirtualDevice;
/*N*/ 		DBG_ASSERT( pVD, "XGradientList: Konnte kein VirtualDevice erzeugen!" );
/*N*/ 		pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );
/*N*/ 
/*N*/ 		pXOut = new XOutputDevice( pVD );
/*N*/ 		DBG_ASSERT( pVD, "XGradientList: Konnte kein XOutDevice erzeugen!" );
/*N*/ 
/*N*/ 		pXFSet = new XFillAttrSetItem( pXPool );
/*N*/ 		DBG_ASSERT( pVD, "XGradientList: Konnte kein XFillAttrSetItem erzeugen!" );
/*N*/ 		pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_GRADIENT ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	pXFSet->GetItemSet().Put(
/*N*/ 		XFillGradientItem( pXPool, Get( nIndex )->GetGradient() ) );
/*N*/ //-/	pXOut->SetFillAttr( *pXFSet );
/*N*/ 	pXOut->SetFillAttr( pXFSet->GetItemSet() );
/*N*/ 
/*N*/ 	// #73550#
/*N*/ 	pXOut->OverrideLineColor( Color( COL_BLACK ) );
/*N*/ 
/*N*/ 	Size aVDSize = pVD->GetOutputSizePixel();
/*N*/ 	pXOut->DrawRect( Rectangle( Point(), aVDSize ) );
/*N*/ 	Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( Point(), aVDSize ) );
/*N*/ 
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( bDelete )
/*N*/ 	{
/*N*/ 		if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 		if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 		if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 	}
/*N*/ 	return( pBitmap );
/*N*/ }

/************************************************************************/

/*N*/ XubString& XGradientList::ConvertName( XubString& rStrName )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	for( USHORT i=0; i<(RID_SVXSTR_GRDT_DEF_END-RID_SVXSTR_GRDT_DEF_START+1) && !bFound; i++ )
/*N*/ 	{
/*N*/ 		XubString aStrDefName =	SVX_RESSTR( RID_SVXSTR_GRDT_DEF_START + i );
/*N*/ 		if( rStrName.Search( aStrDefName ) == 0 )
/*N*/ 		{
/*N*/ 			rStrName.Replace( 0, aStrDefName.Len(),	SVX_RESSTR( RID_SVXSTR_GRDT_START + i ) );
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rStrName;
/*N*/ }

/************************************************************************/

/*N*/ SvStream& XGradientList::ImpRead( SvStream& rIn )
/*N*/ {
/*N*/ 	// Lesen
/*N*/ 	rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
/*N*/ 
/*N*/ 	delete pBmpList;
/*N*/ 	pBmpList = new List( 16, 16 );
/*N*/ 
/*N*/ 	XGradientEntry* pEntry = NULL;
/*N*/ 	long		nCheck;
/*N*/ 	long		nCount;
/*N*/ 	XubString		aName;
/*N*/ 
/*N*/ 	long	nStyle;
/*N*/ 	USHORT	nRed;
/*N*/ 	USHORT	nGreen;
/*N*/ 	USHORT	nBlue;
/*N*/ 	Color	aStart;
/*N*/ 	Color	aEnd;
/*N*/ 	long	nAngle;
/*N*/ 	sal_uInt32 nBorder;
/*N*/ 	sal_uInt32 nXOfs;
/*N*/ 	sal_uInt32 nYOfs;
/*N*/ 	sal_uInt32 nStartIntens;
/*N*/ 	sal_uInt32 nEndIntens;
/*N*/ 
/*N*/ 	// Kennung oder Anzahl
/*N*/ 	rIn >> nCheck;
/*N*/ 
/*N*/ 	if( nCheck >= 0 )
/*N*/ 	{
/*N*/ 		nCount = nCheck;
/*N*/ 
/*N*/ 		for (long nIndex = 0; nIndex < nCount; nIndex++)
/*N*/ 		{
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 
/*N*/ 			aName = ConvertName( aName );
/*N*/ 			rIn >> nStyle;
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			aStart = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			aEnd = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 
/*N*/ 			rIn >> nAngle;
/*N*/ 			rIn >> nBorder;
/*N*/ 			rIn >> nXOfs;
/*N*/ 			rIn >> nYOfs;
/*N*/ 
/*N*/ 			nStartIntens = 100L;
/*N*/ 			nEndIntens = 100L;
/*N*/ 
/*N*/ 			XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
/*N*/ 								 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
/*N*/ 								 (USHORT) nStartIntens, (USHORT) nEndIntens );
/*N*/ 			pEntry = new XGradientEntry (aGradient, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( nCheck == -1L )
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 		for (long nIndex = 0; nIndex < nCount; nIndex++)
/*N*/ 		{
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 
/*N*/ 			aName = ConvertName( aName );
/*N*/ 			rIn >> nStyle;
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			aStart = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			aEnd = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 
/*N*/ 			rIn >> nAngle;
/*N*/ 			rIn >> nBorder;
/*N*/ 			rIn >> nXOfs;
/*N*/ 			rIn >> nYOfs;
/*N*/ 
/*N*/ 			rIn >> nStartIntens;
/*N*/ 			rIn >> nEndIntens;
/*N*/ 
/*N*/ 			XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
/*N*/ 								 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
/*N*/ 								 (USHORT) nStartIntens, (USHORT) nEndIntens );
/*N*/ 			pEntry = new XGradientEntry (aGradient, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else // ab 3.00a
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 		for (long nIndex = 0; nIndex < nCount; nIndex++)
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
/*N*/ 			aStart = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 			aEnd = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 
/*N*/ 			rIn >> nAngle;
/*N*/ 			rIn >> nBorder;
/*N*/ 			rIn >> nXOfs;
/*N*/ 			rIn >> nYOfs;
/*N*/ 
/*N*/ 			rIn >> nStartIntens;
/*N*/ 			rIn >> nEndIntens;
/*N*/ 
/*N*/ 			if (aIOC.GetVersion() > 0)
/*N*/ 			{
/*N*/ 				// lesen neuer Daten ...
/*N*/ 			}
/*N*/ 
/*N*/ 			XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
/*N*/ 								 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
/*N*/ 								 (USHORT) nStartIntens, (USHORT) nEndIntens );
/*N*/ 			pEntry = new XGradientEntry (aGradient, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return( rIn );
/*N*/ }


}
