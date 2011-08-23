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

#ifndef SVX_LIGHT

#ifndef _SVX_XPROPERTYTABLE_HXX
#include "XPropertyTable.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX 
#include <unotools/ucbstreamhelper.hxx>
#endif


#include "xmlxtimp.hxx"

#endif

#include <bf_sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xiocomp.hxx"
namespace binfilter {

#define GLOBALOVERFLOW

using namespace ::com::sun::star;
using namespace rtl;

sal_Unicode const pszExtColor[]	 = {'s','o','c'};

static char const aChckColor[]  = { 0x04, 0x00, 'S','O','C','L'};	// < 5.2
static char const aChckColor0[] = { 0x04, 0x00, 'S','O','C','0'};	// = 5.2
static char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// ------------------
// class XColorTable
// ------------------

/*************************************************************************
|*
|* XColorTable::XColorTable()
|*
*************************************************************************/

/*N*/ XColorTable::XColorTable( const String& rPath,
/*N*/ 							XOutdevItemPool* pInPool,
/*N*/ 							USHORT nInitSize, USHORT nReSize ) :
/*N*/ 				XPropertyTable( rPath, pInPool, nInitSize, nReSize)
/*N*/ {
/*N*/ 	// ColorTable braucht keine eigene BmpTable
/*N*/ 	// pBmpTable = new Table( nInitSize, nReSize );
/*N*/ }

/************************************************************************/

/*N*/ XColorTable::~XColorTable()
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XColorEntry* XColorTable::Replace(long nIndex, XColorEntry* pEntry )
/*N*/ {
/*N*/ 	return (XColorEntry*) XPropertyTable::Replace(nIndex, pEntry);
/*N*/ }

/************************************************************************/

/*N*/ XColorEntry* XColorTable::Remove(long nIndex)
/*N*/ {
/*N*/ 	return (XColorEntry*) XPropertyTable::Remove(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ XColorEntry* XColorTable::Get(long nIndex) const
/*N*/ {
/*N*/ 	return (XColorEntry*) XPropertyTable::Get(nIndex, 0);
/*N*/ }

/************************************************************************/

/*N*/ BOOL XColorTable::Load()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if( bTableDirty )
/*N*/ 	{
/*N*/ 		bTableDirty = FALSE;
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
/*N*/ 			aURL.setExtension( rtl::OUString( pszExtColor, 3 ) );
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
/*N*/ 			if( memcmp( aCheck, aChckColor, sizeof( aChckColor ) ) == 0 ||
/*N*/ 				memcmp( aCheck, aChckColor0, sizeof( aChckColor0 ) ) == 0 )
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
/*N*/ 		uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
/*N*/ 		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XColorTable::Save()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return false;

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckColor0, 4 );
    pStream->WriteByteString(String( pszChckColor0, 4 ));
    

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
*/
/*N*/ }

/************************************************************************/

/*N*/ BOOL XColorTable::Create()
/*N*/ {
/*N*/ 	XubString aStr;
/*N*/ 	xub_StrLen nLen;
/*N*/ 	ResMgr& rRes = DIALOG_MGR();
/*N*/ 
/*N*/ 	static USHORT __READONLY_DATA aResId[] =
/*N*/ 	{
/*N*/ 		RID_SVXSTR_BLACK,
/*N*/ 		RID_SVXSTR_BLUE,
/*N*/ 		RID_SVXSTR_GREEN,
/*N*/ 		RID_SVXSTR_CYAN,
/*N*/ 		RID_SVXSTR_RED,
/*N*/ 		RID_SVXSTR_MAGENTA,
/*N*/ 		RID_SVXSTR_BROWN,
/*N*/ 		RID_SVXSTR_GREY,
/*N*/ 		RID_SVXSTR_LIGHTGREY,
/*N*/ 		RID_SVXSTR_LIGHTBLUE,
/*N*/ 		RID_SVXSTR_LIGHTGREEN,
/*N*/ 		RID_SVXSTR_LIGHTCYAN,
/*N*/ 		RID_SVXSTR_LIGHTRED,
/*N*/ 		RID_SVXSTR_LIGHTMAGENTA,
/*N*/ 		RID_SVXSTR_YELLOW,
/*N*/ 		RID_SVXSTR_WHITE
/*N*/ 	};
/*N*/ 
/*N*/ 	// MT: COL_XXX ist in VCL kein enum mehr!!!
/*N*/ 	// COL_WHITE ist seeeehr gross! ( => Zugriff ueber das obige Array hinweg )
/*N*/ 	// Mit der unteren Schleife gibt es keinen Absturtz, aber es ist
/*N*/ 	// alles schwarz, weil alles kleine Werte.
/*N*/ 	// Ausserdem ist die ganze Vorgehensweise laut MM sehr unperformant
/*N*/ 	// => lieber gleich Stringlisten laden.
/*N*/ 
/*N*/ 	// BM: ifndef VCL part removed (deprecated)
/*N*/ 
/*N*/ 	static ColorData __READONLY_DATA aColTab[] =
/*N*/ 	{
/*N*/ 		COL_BLACK,
/*N*/ 		COL_BLUE,
/*N*/ 		COL_GREEN,
/*N*/ 		COL_CYAN,
/*N*/ 		COL_RED,
/*N*/ 		COL_MAGENTA,
/*N*/ 		COL_BROWN,
/*N*/ 		COL_GRAY,
/*N*/ 		COL_LIGHTGRAY,
/*N*/ 		COL_LIGHTBLUE,
/*N*/ 		COL_LIGHTGREEN,
/*N*/ 		COL_LIGHTCYAN,
/*N*/ 		COL_LIGHTRED,
/*N*/ 		COL_LIGHTMAGENTA,
/*N*/ 		COL_YELLOW,
/*N*/ 		COL_WHITE
/*N*/ 	};
/*N*/ 
/*N*/ 	for( USHORT n = 0; n < 16; ++n )
/*N*/ 	{
/*N*/ 		Insert( n, new XColorEntry( Color( aColTab[n] ),
/*N*/ 									String( ResId( aResId[ n ], rRes )) ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_GREY );
/*N*/ 	aStr.AppendAscii(" 80%");
/*N*/ 	nLen = aStr.Len() - 3;
/*N*/ 	Insert(16, new XColorEntry( Color( 51, 51, 51 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(17, new XColorEntry( Color( 76, 76, 76 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(18, new XColorEntry( Color(102,102,102 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(19, new XColorEntry( Color(153,153,153 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(20, new XColorEntry( Color(179,179,179 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(21, new XColorEntry( Color(204,204,204 ), aStr ) );
/*N*/ 	// BM: new 15%
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('1'));
/*N*/ 	aStr.SetChar(nLen + 1, sal_Unicode('5'));
/*N*/ 	Insert(22, new XColorEntry( Color(217,217,217 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen + 1, sal_Unicode('0'));
/*N*/ 	Insert(23, new XColorEntry( Color(230,230,230 ), aStr ) );
/*N*/ 	Insert(24, new XColorEntry( Color(230,230,255 ), SVX_RESSTR( RID_SVXSTR_BLUEGREY ) ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_RED );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(25, new XColorEntry( Color(255, 51,102 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(26, new XColorEntry( Color(220, 35,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(27, new XColorEntry( Color(184, 71,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(28, new XColorEntry( Color(255, 51, 51 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(29, new XColorEntry( Color(235, 97, 61 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(30, new XColorEntry( Color(184, 71, 71 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(31, new XColorEntry( Color(184,  0, 71 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(32, new XColorEntry( Color(153, 40, 76 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_MAGENTA );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(33, new XColorEntry( Color(148,  0,107 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(34, new XColorEntry( Color(148, 71,107 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(35, new XColorEntry( Color(148, 71,148 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(36, new XColorEntry( Color(153,102,204 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(37, new XColorEntry( Color(107, 71,148 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(38, new XColorEntry( Color(107, 35,148 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(39, new XColorEntry( Color(107,  0,148 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(40, new XColorEntry( Color( 94, 17,166 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_BLUE );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(41, new XColorEntry( Color( 40,  0,153 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(42, new XColorEntry( Color( 71,  0,184 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(43, new XColorEntry( Color( 35,  0,220 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(44, new XColorEntry( Color( 35, 35,220 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(45, new XColorEntry( Color(  0, 71,255 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(46, new XColorEntry( Color(  0,153,255 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(47, new XColorEntry( Color(  0,184,255 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(48, new XColorEntry( Color(153,204,255 ), aStr ) );
/*N*/ 	//Insert(48, new XColorEntry( Color( 46,215,255 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_CYAN );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(49, new XColorEntry( Color(  0,220,255 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(50, new XColorEntry( Color(  0,204,204 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(51, new XColorEntry( Color( 35,184,220 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(52, new XColorEntry( Color( 71,184,184 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(53, new XColorEntry( Color( 51,163,163 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(54, new XColorEntry( Color( 25,138,138 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(55, new XColorEntry( Color(  0,107,107 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(56, new XColorEntry( Color(  0, 74, 74 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_GREEN );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(57, new XColorEntry( Color( 53, 94,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(58, new XColorEntry( Color( 92,133, 38 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(59, new XColorEntry( Color(125,166, 71 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(60, new XColorEntry( Color(148,189, 94 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(61, new XColorEntry( Color(  0,174,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(62, new XColorEntry( Color( 51,204,102 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(63, new XColorEntry( Color( 61,235, 61 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(64, new XColorEntry( Color( 35,255, 35 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_YELLOW );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(65, new XColorEntry( Color(230,255,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(66, new XColorEntry( Color(255,255,153 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(67, new XColorEntry( Color(255,255,102 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(68, new XColorEntry( Color(230,230, 76 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('5'));
/*N*/ 	Insert(69, new XColorEntry( Color(204,204,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('6'));
/*N*/ 	Insert(70, new XColorEntry( Color(179,179,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('7'));
/*N*/ 	Insert(71, new XColorEntry( Color(128,128, 25 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('8'));
/*N*/ 	Insert(72, new XColorEntry( Color(102,102,  0 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_BROWN );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(73, new XColorEntry( Color( 76, 25,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(74, new XColorEntry( Color(102, 51,  0 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(75, new XColorEntry( Color(128, 76, 25 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(76, new XColorEntry( Color(153,102, 51 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_ORANGE );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert(77, new XColorEntry( Color(204,102, 51 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert(78, new XColorEntry( Color(255,102, 51 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert(79, new XColorEntry( Color(255,153,102 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert(80, new XColorEntry( Color(255,204,153 ), aStr ) );
/*N*/ 
/*N*/ 	// new chart colors
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_VIOLET );
/*N*/ 	Insert( 81, new XColorEntry( Color( 0x99, 0x99, 0xff ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_BORDEAUX );
/*N*/ 	Insert( 82, new XColorEntry( Color( 0x99, 0x33, 0x66 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_PALE_YELLOW );
/*N*/ 	Insert( 83, new XColorEntry( Color( 0xff, 0xff, 0xcc ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_PALE_GREEN );
/*N*/ 	Insert( 84, new XColorEntry( Color( 0xcc, 0xff, 0xff ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_DKVIOLET );
/*N*/ 	Insert( 85, new XColorEntry( Color( 0x66, 0x00, 0x66 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_SALMON );
/*N*/ 	Insert( 86, new XColorEntry( Color( 0xff, 0x80, 0x80 ), aStr ) );
/*N*/ 
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_SEABLUE );
/*N*/ 	Insert( 87, new XColorEntry( Color( 0x00, 0x66, 0xcc ), aStr ) );
/*N*/ 
/*N*/ 	// Sun colors
/*N*/ 	aStr = SVX_RESSTR( RID_SVXSTR_COLOR_SUN );
/*N*/ 	aStr.AppendAscii(" 1");
/*N*/ 	nLen = aStr.Len() - 1;
/*N*/ 	Insert( 88, new XColorEntry( Color( 0x33, 0x33, 0x66 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('2'));
/*N*/ 	Insert( 89, new XColorEntry( Color( 0x66, 0x66, 0x99 ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('3'));
/*N*/ 	Insert( 90, new XColorEntry( Color( 0x99, 0x99, 0xcc ), aStr ) );
/*N*/ 	aStr.SetChar(nLen, sal_Unicode('4'));
/*N*/ 	Insert( 91, new XColorEntry( Color( 0xcc, 0xcc, 0xff ), aStr ) );
/*N*/ 
/*N*/ 	return( Count() == 92 );
/*N*/ }

/************************************************************************/

/*N*/ BOOL XColorTable::CreateBitmapsForUI()
/*N*/ {
/*N*/ 	return( FALSE );
/*N*/ }

/************************************************************************/

/*N*/ Bitmap* XColorTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
/*N*/ {
/*N*/ 	return( NULL );
/*N*/ }

/************************************************************************/

/*N*/ XubString& XColorTable::ConvertName( XubString& rStrName )
/*N*/ {
/*N*/ 	static USHORT __READONLY_DATA aDefResId[] =
/*N*/ 	{
/*N*/ 		RID_SVXSTR_BLUEGREY_DEF,
/*N*/ 		RID_SVXSTR_BLACK_DEF,
/*N*/ 		RID_SVXSTR_BLUE_DEF,
/*N*/ 		RID_SVXSTR_GREEN_DEF,
/*N*/ 		RID_SVXSTR_CYAN_DEF,
/*N*/ 		RID_SVXSTR_RED_DEF,
/*N*/ 		RID_SVXSTR_MAGENTA_DEF,
/*N*/ 		RID_SVXSTR_BROWN_DEF,
/*N*/ 		RID_SVXSTR_GREY_DEF,
/*N*/ 		RID_SVXSTR_LIGHTGREY_DEF,
/*N*/ 		RID_SVXSTR_LIGHTBLUE_DEF,
/*N*/ 		RID_SVXSTR_LIGHTGREEN_DEF,
/*N*/ 		RID_SVXSTR_LIGHTCYAN_DEF,
/*N*/ 		RID_SVXSTR_LIGHTRED_DEF,
/*N*/ 		RID_SVXSTR_LIGHTMAGENTA_DEF,
/*N*/ 		RID_SVXSTR_YELLOW_DEF,
/*N*/ 		RID_SVXSTR_WHITE_DEF,
/*N*/ 		RID_SVXSTR_ORANGE_DEF,
/*N*/ 		RID_SVXSTR_VIOLET_DEF,
/*N*/ 		RID_SVXSTR_BORDEAUX_DEF,
/*N*/ 		RID_SVXSTR_PALE_YELLOW_DEF,
/*N*/ 		RID_SVXSTR_PALE_GREEN_DEF,
/*N*/ 		RID_SVXSTR_DKVIOLET_DEF,
/*N*/ 		RID_SVXSTR_SALMON_DEF,
/*N*/ 		RID_SVXSTR_SEABLUE_DEF,
/*N*/ 		RID_SVXSTR_COLOR_SUN_DEF
/*N*/ 	};
/*N*/ 	static USHORT __READONLY_DATA aResId[] =
/*N*/ 	{
/*N*/ 		RID_SVXSTR_BLUEGREY,
/*N*/ 		RID_SVXSTR_BLACK,
/*N*/ 		RID_SVXSTR_BLUE,
/*N*/ 		RID_SVXSTR_GREEN,
/*N*/ 		RID_SVXSTR_CYAN,
/*N*/ 		RID_SVXSTR_RED,
/*N*/ 		RID_SVXSTR_MAGENTA,
/*N*/ 		RID_SVXSTR_BROWN,
/*N*/ 		RID_SVXSTR_GREY,
/*N*/ 		RID_SVXSTR_LIGHTGREY,
/*N*/ 		RID_SVXSTR_LIGHTBLUE,
/*N*/ 		RID_SVXSTR_LIGHTGREEN,
/*N*/ 		RID_SVXSTR_LIGHTCYAN,
/*N*/ 		RID_SVXSTR_LIGHTRED,
/*N*/ 		RID_SVXSTR_LIGHTMAGENTA,
/*N*/ 		RID_SVXSTR_YELLOW,
/*N*/ 		RID_SVXSTR_WHITE,
/*N*/ 		RID_SVXSTR_ORANGE,
/*N*/ 		RID_SVXSTR_VIOLET,
/*N*/ 		RID_SVXSTR_BORDEAUX,
/*N*/ 		RID_SVXSTR_PALE_YELLOW,
/*N*/ 		RID_SVXSTR_PALE_GREEN,
/*N*/ 		RID_SVXSTR_DKVIOLET,
/*N*/ 		RID_SVXSTR_SALMON,
/*N*/ 		RID_SVXSTR_SEABLUE,
/*N*/ 		RID_SVXSTR_COLOR_SUN
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

/*N*/ SvStream& XColorTable::ImpRead( SvStream& rIn )
/*N*/ {
/*N*/ 	// Lesen
/*N*/ 	rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
/*N*/ 
/*N*/ 	delete pBmpTable;
/*N*/ 	pBmpTable = new Table( 16, 16 );
/*N*/ 
/*N*/ 	XColorEntry* pEntry = NULL;
/*N*/ 	long		 nType;
/*N*/ 	long		 nCount;
/*N*/ 	long		 nIndex;
/*N*/ 	USHORT		 nRed;
/*N*/ 	USHORT		 nGreen;
/*N*/ 	USHORT		 nBlue;
/*N*/ 	Color		 aColor;
/*N*/ 	XubString	 aName;
/*N*/ 
/*N*/ 	rIn >> nType;
/*N*/ 
/*N*/ 	// gesamte Tabelle?
/*N*/ 	if (nType == 0)
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 		for (long nI = 0; nI < nCount; nI++)
/*N*/ 		{
/*N*/ 			rIn >> nIndex;
/*N*/ 
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			aName = ConvertName( aName );
/*N*/ 		
/*N*/ 			rIn >> nRed;
/*N*/ 			rIn >> nGreen;
/*N*/ 			rIn >> nBlue;
/*N*/ 
/*N*/ 			aColor = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 							(BYTE) ( nGreen >> 8 ),
/*N*/ 							(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			pEntry = new XColorEntry( aColor, aName);
/*N*/ 			Insert (nIndex, pEntry);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else // Version ab 3.00a
/*N*/ 	{
/*N*/ 		rIn >> nCount;
/*N*/ 		for (long nI = 0; nI < nCount; nI++)
/*N*/ 		{
/*N*/ 			// Versionsverwaltung
/*N*/ 			XIOCompat aIOC( rIn, STREAM_READ );
/*N*/ 
/*N*/ 			rIn >> nIndex;
/*N*/ 
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			aName = ConvertName( aName );
/*N*/ 
/*N*/ 			if( aIOC.GetVersion() >= 0 )
/*N*/ 			{
/*N*/ 				rIn >> nRed;
/*N*/ 				rIn >> nGreen;
/*N*/ 				rIn >> nBlue;
/*N*/ 				aColor = Color( (BYTE) ( nRed   >> 8 ),
/*N*/ 								(BYTE) ( nGreen >> 8 ),
/*N*/ 								(BYTE) ( nBlue  >> 8 ) );
/*N*/ 			}
            /*
            else if( aIOC.GetVersion() >= 1 )
            {
                // lesen neuer Daten ...
            }
            */
/*N*/ 
/*N*/ 			pEntry = new XColorEntry( aColor, aName );
/*N*/ 			Insert( nIndex, pEntry );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return( rIn );
/*N*/ }

}
