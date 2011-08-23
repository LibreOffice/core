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
#ifndef _SV_SVAPP_HXX 
#include <vcl/svapp.hxx>
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

#ifndef _SVX_XLINEIT0_HXX //autogen
#include <xlineit0.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SVX_XLNDSIT_HXX //autogen
#include <xlndsit.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace rtl;

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[] 	= {'s','o','d'};
char const aChckDash[]  = { 0x04, 0x00, 'S','O','D','L'};	// < 5.2
char const aChckDash0[] = { 0x04, 0x00, 'S','O','D','0'};	// = 5.2
char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// -----------------
// class XDashTable
// -----------------

/************************************************************************/

/*N*/ XDashTable::~XDashTable()
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashTable::Replace(long nIndex, XDashEntry* pEntry )
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyTable::Replace(nIndex, pEntry);
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashTable::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyTable::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashTable::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyTable::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashTable::Load()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashTable::Save()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashTable::Create()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashTable::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XDashTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	return( NULL );
/*N*/ }

// ----------------
// class XDashList
// ----------------

/*************************************************************************
|*
|* XDashList::XDashList()
|*
*************************************************************************/

/*N*/ XDashList::XDashList( const String& rPath,
/*N*/ 							XOutdevItemPool* pInPool,
/*N*/ 							USHORT nInitSize, USHORT nReSize ) :
/*N*/ 				XPropertyList	( rPath, pInPool, nInitSize, nReSize),
/*N*/ 				pVD				( NULL ),
/*N*/ 				pXOut			( NULL ),
/*N*/ 				pXFSet			( NULL ),
/*N*/ 				pXLSet			( NULL )
/*N*/ {
/*N*/ 	pBmpList = new List( nInitSize, nReSize );
/*N*/ }

/************************************************************************/

/*N*/ XDashList::~XDashList()
/*N*/ {
/*N*/ 	if( pVD )    delete pVD;
/*N*/ 	if( pXOut )  delete pXOut;
/*N*/ 	if( pXFSet ) delete pXFSet;
/*N*/ 	if( pXLSet ) delete pXLSet;
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyList::Replace(pEntry, nIndex);
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashList::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyList::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XDashEntry* XDashList::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XDashEntry*) XPropertyList::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashList::Load()
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
/*N*/ 			aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );
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
/*N*/ 			if( memcmp( aCheck, aChckDash, sizeof( aChckDash ) ) == 0 ||
/*N*/ 				memcmp( aCheck, aChckDash0, sizeof( aChckDash0 ) ) == 0 )
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
/*N*/ 		uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
/*N*/ 		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashList::Save()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return false;


/*


    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckDash0, 4 );
    pStream->WriteByteString(String( pszChckDash0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
*/
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashList::Create()
/*N*/ {
/*N*/ 	XubString aStr( SVX_RES( RID_SVXSTR_LINESTYLE ) );
/*N*/ 	xub_StrLen nLen;
/*N*/ 
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(new XDashEntry(XDash(XDASH_RECT,1, 50,1, 50, 50),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(new XDashEntry(XDash(XDASH_RECT,1,500,1,500,500),aStr));
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(new XDashEntry(XDash(XDASH_RECT,2, 50,3,250,120),aStr));
/*N*/ 
/*N*/ 	return( TRUE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XDashList::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	for( long i = 0; i < Count(); i++)
/*N*/ 	{
/*N*/ 		Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
/*N*/ 		DBG_ASSERT( pBmp, "XDashList: Bitmap(UI) konnte nicht erzeugt werden!" );
/*N*/ 
/*N*/ 		if( pBmp )
/*N*/ 			pBmpList->Insert( pBmp, i );
/*N*/ 	}
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 	if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 	if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 	if( pXLSet ){ delete pXLSet; pXLSet = NULL; }
/*N*/ 
/*N*/ 	return( TRUE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XDashList::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	Point	aZero;
/*N*/ 
/*N*/ 	if( !pVD ) // und pXOut und pXFSet und pXLSet
/*N*/ 	{
/*N*/ 		pVD = new VirtualDevice;
/*N*/ 		DBG_ASSERT( pVD, "XDashList: Konnte kein VirtualDevice erzeugen!" );
/*N*/ 		pVD->SetMapMode( MAP_100TH_MM );
/*N*/ 		pVD->SetOutputSize( pVD->PixelToLogic( Size( BITMAP_WIDTH * 2, BITMAP_HEIGHT ) ) );
/*N*/ 
/*N*/ 		const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
/*N*/ 		pVD->SetFillColor( rStyles.GetFieldColor() );
/*N*/ 		pVD->SetLineColor( rStyles.GetFieldColor() );
/*N*/ 
/*N*/ 		pXOut = new XOutputDevice( pVD );
/*N*/ 		DBG_ASSERT( pVD, "XDashList: Konnte kein XOutDevice erzeugen!" );
/*N*/ 
/*N*/ 		pXFSet = new XFillAttrSetItem( pXPool );
/*N*/ 		DBG_ASSERT( pVD, "XDashList: Konnte kein XFillAttrSetItem erzeugen!" );
/*N*/ 
/*N*/ 		pXLSet = new XLineAttrSetItem( pXPool );
/*N*/ 		DBG_ASSERT( pVD, "XDashList: Konnte kein XLineAttrSetItem erzeugen!" );
/*N*/ 		pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_DASH ) );
/*N*/ 		pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( rStyles.GetFieldTextColor().GetColor() ) ) );
/*N*/ 		pXLSet->GetItemSet().Put( XLineWidthItem( 30 ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	Size aVDSize = pVD->GetOutputSize();
/*N*/ 	pVD->DrawRect( Rectangle( aZero, aVDSize ) );
/*N*/ 	pXLSet->GetItemSet().Put( XLineDashItem( String(), Get( nIndex )->GetDash() ) );
/*N*/ 
/*N*/ //-/	pXOut->SetLineAttr( *pXLSet );
/*N*/ 	pXOut->SetLineAttr( pXLSet->GetItemSet() );
/*N*/ 	
/*N*/ 	pXOut->DrawLine( Point( 0, aVDSize.Height() / 2 ),
/*N*/ 					 Point( aVDSize.Width(), aVDSize.Height() / 2 ) );
/*N*/ 
/*N*/ 	Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, aVDSize ) );
/*N*/ 
/*N*/ 	// Loeschen, da JOE den Pool vorm Dtor entfernt!
/*N*/ 	if( bDelete )
/*N*/ 	{
/*N*/ 		if( pVD )	{ delete pVD;	pVD = NULL;     }
/*N*/ 		if( pXOut ) { delete pXOut;	pXOut = NULL;   }
/*N*/ 		if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
/*N*/ 		if( pXLSet ){ delete pXLSet; pXLSet = NULL; }
/*N*/ 	}
/*N*/ 	return( pBitmap );
/*N*/ }

/************************************************************************/

/*N*/ XubString& XDashList::ConvertName( XubString& rStrName )
/*N*/ {
/*N*/ 	static USHORT __READONLY_DATA aDefResId[] =
/*N*/ 	{
/*N*/ 		RID_SVXSTR_DASH5_DEF,
/*N*/ 		RID_SVXSTR_DASH0_DEF,
/*N*/ 		RID_SVXSTR_DASH1_DEF,
/*N*/ 		RID_SVXSTR_DASH2_DEF,
/*N*/ 		RID_SVXSTR_DASH3_DEF,
/*N*/ 		RID_SVXSTR_DASH4_DEF,
/*N*/ 		RID_SVXSTR_DASH6_DEF,
/*N*/ 		RID_SVXSTR_DASH7_DEF,
/*N*/ 		RID_SVXSTR_DASH8_DEF,
/*N*/ 		RID_SVXSTR_DASH9_DEF,
/*N*/ 		RID_SVXSTR_DASH10_DEF
/*N*/ 	};
/*N*/ 	static USHORT __READONLY_DATA aResId[] =
/*N*/ 	{
/*N*/ 		RID_SVXSTR_DASH5,
/*N*/ 		RID_SVXSTR_DASH0,
/*N*/ 		RID_SVXSTR_DASH1,
/*N*/ 		RID_SVXSTR_DASH2,
/*N*/ 		RID_SVXSTR_DASH3,
/*N*/ 		RID_SVXSTR_DASH4,
/*N*/ 		RID_SVXSTR_DASH6,
/*N*/ 		RID_SVXSTR_DASH7,
/*N*/ 		RID_SVXSTR_DASH8,
/*N*/ 		RID_SVXSTR_DASH9,
/*N*/ 		RID_SVXSTR_DASH10
/*N*/ 	};
/*N*/ 
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	for( int i=0; i<(sizeof(aDefResId) / sizeof(USHORT)) && !bFound; i++ )
/*N*/ 	{
/*N*/ 		XubString aStrDefName = SVX_RESSTR( aDefResId[i] );
/*N*/ 		if( rStrName.Search( aStrDefName ) == 0 )
/*N*/ 		{
/*N*/ 			rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( aResId[i] ) );
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rStrName;
/*N*/ }

/************************************************************************/

/*N*/ SvStream& XDashList::ImpRead( SvStream& rIn )
/*N*/ {
/*N*/ 	// Lesen
/*N*/ 	rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
/*N*/ 
/*N*/ 	delete pBmpList;
/*N*/ 	pBmpList = new List( 16, 16 );
/*N*/ 
/*N*/ 	XDashEntry* pEntry = NULL;
/*N*/ 	long		nCount;
/*N*/ 	XubString	aName;
/*N*/ 
/*N*/ 	long		nStyle;
/*N*/ 	long		nDots;
/*N*/ 	sal_uInt32	nDotLen;
/*N*/ 	long		nDashes;
/*N*/ 	sal_uInt32	nDashLen;
/*N*/ 	sal_uInt32	nDistance;
/*N*/ 
/*N*/ 	rIn >> nCount;
/*N*/ 
/*N*/ 	if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
/*N*/ 	{
/*N*/ 		for (long nIndex = 0; nIndex < nCount; nIndex++)
/*N*/ 		{
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 	
/*N*/ 			aName = ConvertName( aName );
/*N*/ 			rIn >> nStyle;
/*N*/ 			rIn >> nDots;
/*N*/ 			rIn >> nDotLen;
/*N*/ 			rIn >> nDashes;
/*N*/ 			rIn >> nDashLen;
/*N*/ 			rIn >> nDistance;
/*N*/ 			XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
/*N*/ 						(BYTE)nDashes, nDashLen, nDistance);
/*N*/ 			pEntry = new XDashEntry (aDash, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else // ab 3.00a
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 
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
/*N*/ 			rIn >> nDots;
/*N*/ 			rIn >> nDotLen;
/*N*/ 			rIn >> nDashes;
/*N*/ 			rIn >> nDashLen;
/*N*/ 			rIn >> nDistance;
/*N*/ 
/*N*/ 			if (aIOC.GetVersion() > 0)
/*N*/ 			{
/*N*/ 				// lesen neuer Daten ...
/*N*/ 			}
/*N*/ 
/*N*/ 			XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
/*N*/ 						(BYTE)nDashes, nDashLen, nDistance);
/*N*/ 			pEntry = new XDashEntry (aDash, aName);
/*N*/ 			Insert (pEntry, nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return( rIn );
/*N*/ }



}
