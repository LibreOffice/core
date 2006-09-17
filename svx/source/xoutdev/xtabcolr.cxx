/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabcolr.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:25:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_LIGHT

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SVX_XPROPERTYTABLE_HXX
#include "XPropertyTable.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#include <svtools/pathoptions.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtColor[]  = {'s','o','c'};

static char const aChckColor[]  = { 0x04, 0x00, 'S','O','C','L'};   // < 5.2
static char const aChckColor0[] = { 0x04, 0x00, 'S','O','C','0'};   // = 5.2
static char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };      // = 6.0

// ------------------
// class XColorTable
// ------------------

static XColorTable* pTable=0;

/*************************************************************************
|*
|* XColorTable::XColorTable()
|*
*************************************************************************/

XColorTable::XColorTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    // ColorTable braucht keine eigene BmpTable
    // pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XColorTable::~XColorTable()
{
}

XColorTable* XColorTable::GetStdColorTable()
{
    if ( !pTable )
        pTable = new XColorTable( SvtPathOptions().GetPalettePath() );
    return pTable;
}

/************************************************************************/

XColorEntry* XColorTable::Replace(long nIndex, XColorEntry* pEntry )
{
    return (XColorEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XColorEntry* XColorTable::Remove(long nIndex)
{
    return (XColorEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XColorEntry* XColorTable::GetColor(long nIndex) const
{
    return (XColorEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XColorTable::Load()
{
//BFS01#ifndef SVX_LIGHT
    if( bTableDirty )
    {
        bTableDirty = FALSE;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return FALSE;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtColor, 3 ) );

//BFS01     // check if file exists, SfxMedium shows an errorbox else
//BFS01     {
//BFS01         com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xHandler;
//BFS01         SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );
//BFS01
//BFS01         sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);
//BFS01
//BFS01         if( pIStm )
//BFS01             delete pIStm;
//BFS01
//BFS01         if( !bOk )
//BFS01             return sal_False;
//BFS01     }

//BFS01     {
//BFS01         SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE, TRUE );
//BFS01         SvStream* pStream = aMedium.GetInStream();
//BFS01         if( !pStream )
//BFS01             return( FALSE );
//BFS01
//BFS01         char aCheck[6];
//BFS01         pStream->Read( aCheck, 6 );
//BFS01
//BFS01         // Handelt es sich um die gew"unschte Tabelle?
//BFS01         if( memcmp( aCheck, aChckColor, sizeof( aChckColor ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckColor0, sizeof( aChckColor0 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XColorTable::Save()
{
//BFS01#ifndef SVX_LIGHT
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtColor, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

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
//BFS01#else
//BFS01 return FALSE;
//BFS01#endif
}

/************************************************************************/

BOOL XColorTable::Create()
{
    XubString aStr;
    xub_StrLen nLen;
    ResMgr* pRes = DIALOG_MGR();

    static USHORT __READONLY_DATA aResId[] =
    {
        RID_SVXSTR_BLACK,
        RID_SVXSTR_BLUE,
        RID_SVXSTR_GREEN,
        RID_SVXSTR_CYAN,
        RID_SVXSTR_RED,
        RID_SVXSTR_MAGENTA,
        RID_SVXSTR_BROWN,
        RID_SVXSTR_GREY,
        RID_SVXSTR_LIGHTGREY,
        RID_SVXSTR_LIGHTBLUE,
        RID_SVXSTR_LIGHTGREEN,
        RID_SVXSTR_LIGHTCYAN,
        RID_SVXSTR_LIGHTRED,
        RID_SVXSTR_LIGHTMAGENTA,
        RID_SVXSTR_YELLOW,
        RID_SVXSTR_WHITE
    };

    // MT: COL_XXX ist in VCL kein enum mehr!!!
    // COL_WHITE ist seeeehr gross! ( => Zugriff ueber das obige Array hinweg )
    // Mit der unteren Schleife gibt es keinen Absturtz, aber es ist
    // alles schwarz, weil alles kleine Werte.
    // Ausserdem ist die ganze Vorgehensweise laut MM sehr unperformant
    // => lieber gleich Stringlisten laden.

    // BM: ifndef VCL part removed (deprecated)

    static ColorData __READONLY_DATA aColTab[] =
    {
        COL_BLACK,
        COL_BLUE,
        COL_GREEN,
        COL_CYAN,
        COL_RED,
        COL_MAGENTA,
        COL_BROWN,
        COL_GRAY,
        COL_LIGHTGRAY,
        COL_LIGHTBLUE,
        COL_LIGHTGREEN,
        COL_LIGHTCYAN,
        COL_LIGHTRED,
        COL_LIGHTMAGENTA,
        COL_YELLOW,
        COL_WHITE
    };

    for( USHORT n = 0; n < 16; ++n )
    {
        Insert( n, new XColorEntry( Color( aColTab[n] ),
                                    String( ResId( aResId[ n ], pRes )) ) );
    }

    aStr = SVX_RESSTR( RID_SVXSTR_GREY );
    aStr.AppendAscii(" 80%");
    nLen = aStr.Len() - 3;
    Insert(16, new XColorEntry( Color( 51, 51, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(17, new XColorEntry( Color( 76, 76, 76 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(18, new XColorEntry( Color(102,102,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(19, new XColorEntry( Color(153,153,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(20, new XColorEntry( Color(179,179,179 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(21, new XColorEntry( Color(204,204,204 ), aStr ) );
    // BM: new 15%
    aStr.SetChar(nLen, sal_Unicode('1'));
    aStr.SetChar(nLen + 1, sal_Unicode('5'));
    Insert(22, new XColorEntry( Color(217,217,217 ), aStr ) );
    aStr.SetChar(nLen + 1, sal_Unicode('0'));
    Insert(23, new XColorEntry( Color(230,230,230 ), aStr ) );
    Insert(24, new XColorEntry( Color(230,230,255 ), SVX_RESSTR( RID_SVXSTR_BLUEGREY ) ) );

    aStr = SVX_RESSTR( RID_SVXSTR_RED );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(25, new XColorEntry( Color(255, 51,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(26, new XColorEntry( Color(220, 35,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(27, new XColorEntry( Color(184, 71,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(28, new XColorEntry( Color(255, 51, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(29, new XColorEntry( Color(235, 97, 61 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(30, new XColorEntry( Color(184, 71, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(31, new XColorEntry( Color(184,  0, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(32, new XColorEntry( Color(153, 40, 76 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_MAGENTA );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(33, new XColorEntry( Color(148,  0,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(34, new XColorEntry( Color(148, 71,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(35, new XColorEntry( Color(148, 71,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(36, new XColorEntry( Color(153,102,204 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(37, new XColorEntry( Color(107, 71,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(38, new XColorEntry( Color(107, 35,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(39, new XColorEntry( Color(107,  0,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(40, new XColorEntry( Color( 94, 17,166 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BLUE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(41, new XColorEntry( Color( 40,  0,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(42, new XColorEntry( Color( 71,  0,184 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(43, new XColorEntry( Color( 35,  0,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(44, new XColorEntry( Color( 35, 35,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(45, new XColorEntry( Color(  0, 71,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(46, new XColorEntry( Color(  0,153,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(47, new XColorEntry( Color(  0,184,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(48, new XColorEntry( Color(153,204,255 ), aStr ) );
    //Insert(48, new XColorEntry( Color( 46,215,255 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_CYAN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(49, new XColorEntry( Color(  0,220,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(50, new XColorEntry( Color(  0,204,204 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(51, new XColorEntry( Color( 35,184,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(52, new XColorEntry( Color( 71,184,184 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(53, new XColorEntry( Color( 51,163,163 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(54, new XColorEntry( Color( 25,138,138 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(55, new XColorEntry( Color(  0,107,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(56, new XColorEntry( Color(  0, 74, 74 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_GREEN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(57, new XColorEntry( Color( 53, 94,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(58, new XColorEntry( Color( 92,133, 38 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(59, new XColorEntry( Color(125,166, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(60, new XColorEntry( Color(148,189, 94 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(61, new XColorEntry( Color(  0,174,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(62, new XColorEntry( Color( 51,204,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(63, new XColorEntry( Color( 61,235, 61 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(64, new XColorEntry( Color( 35,255, 35 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_YELLOW );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(65, new XColorEntry( Color(230,255,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(66, new XColorEntry( Color(255,255,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(67, new XColorEntry( Color(255,255,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(68, new XColorEntry( Color(230,230, 76 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(69, new XColorEntry( Color(204,204,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(70, new XColorEntry( Color(179,179,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(71, new XColorEntry( Color(128,128, 25 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(72, new XColorEntry( Color(102,102,  0 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BROWN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(73, new XColorEntry( Color( 76, 25,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(74, new XColorEntry( Color(102, 51,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(75, new XColorEntry( Color(128, 76, 25 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(76, new XColorEntry( Color(153,102, 51 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_ORANGE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(77, new XColorEntry( Color(204,102, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(78, new XColorEntry( Color(255,102, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(79, new XColorEntry( Color(255,153,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(80, new XColorEntry( Color(255,204,153 ), aStr ) );

    // new chart colors
    aStr = SVX_RESSTR( RID_SVXSTR_VIOLET );
    Insert( 81, new XColorEntry( Color( 0x99, 0x99, 0xff ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BORDEAUX );
    Insert( 82, new XColorEntry( Color( 0x99, 0x33, 0x66 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_YELLOW );
    Insert( 83, new XColorEntry( Color( 0xff, 0xff, 0xcc ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_GREEN );
    Insert( 84, new XColorEntry( Color( 0xcc, 0xff, 0xff ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_DKVIOLET );
    Insert( 85, new XColorEntry( Color( 0x66, 0x00, 0x66 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_SALMON );
    Insert( 86, new XColorEntry( Color( 0xff, 0x80, 0x80 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_SEABLUE );
    Insert( 87, new XColorEntry( Color( 0x00, 0x66, 0xcc ), aStr ) );

    // Sun colors
    aStr = SVX_RESSTR( RID_SVXSTR_COLOR_SUN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( 88, new XColorEntry( Color( 0x33, 0x33, 0x66 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( 89, new XColorEntry( Color( 0x66, 0x66, 0x99 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( 90, new XColorEntry( Color( 0x99, 0x99, 0xcc ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( 91, new XColorEntry( Color( 0xcc, 0xcc, 0xff ), aStr ) );

    return( Count() == 92 );
}

/************************************************************************/

BOOL XColorTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XColorTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XColorTable::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 // Version statt Tabellentyp, um auch alte Tabellen zu lesen
//BFS01 rOut << (long) -1;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Farben
//BFS01 XColorEntry* pEntry = (XColorEntry*)aTable.First();
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // Versionsverwaltung: Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE:: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     USHORT nCol = pEntry->GetColor().GetRed();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01
//BFS01     nCol = pEntry->GetColor().GetGreen();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01
//BFS01     nCol = pEntry->GetColor().GetBlue();
//BFS01     nCol = nCol << 8;
//BFS01     rOut << nCol;
//BFS01     pEntry = (XColorEntry*)aTable.Next();
//BFS01 }
//BFS01 return( rOut );
//BFS01}

/************************************************************************/

//BFS01XubString& XColorTable::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 static USHORT __READONLY_DATA aDefResId[] =
//BFS01 {
//BFS01     RID_SVXSTR_BLUEGREY_DEF,
//BFS01     RID_SVXSTR_BLACK_DEF,
//BFS01     RID_SVXSTR_BLUE_DEF,
//BFS01     RID_SVXSTR_GREEN_DEF,
//BFS01     RID_SVXSTR_CYAN_DEF,
//BFS01     RID_SVXSTR_RED_DEF,
//BFS01     RID_SVXSTR_MAGENTA_DEF,
//BFS01     RID_SVXSTR_BROWN_DEF,
//BFS01     RID_SVXSTR_GREY_DEF,
//BFS01     RID_SVXSTR_LIGHTGREY_DEF,
//BFS01     RID_SVXSTR_LIGHTBLUE_DEF,
//BFS01     RID_SVXSTR_LIGHTGREEN_DEF,
//BFS01     RID_SVXSTR_LIGHTCYAN_DEF,
//BFS01     RID_SVXSTR_LIGHTRED_DEF,
//BFS01     RID_SVXSTR_LIGHTMAGENTA_DEF,
//BFS01     RID_SVXSTR_YELLOW_DEF,
//BFS01     RID_SVXSTR_WHITE_DEF,
//BFS01     RID_SVXSTR_ORANGE_DEF,
//BFS01     RID_SVXSTR_VIOLET_DEF,
//BFS01     RID_SVXSTR_BORDEAUX_DEF,
//BFS01     RID_SVXSTR_PALE_YELLOW_DEF,
//BFS01     RID_SVXSTR_PALE_GREEN_DEF,
//BFS01     RID_SVXSTR_DKVIOLET_DEF,
//BFS01     RID_SVXSTR_SALMON_DEF,
//BFS01     RID_SVXSTR_SEABLUE_DEF,
//BFS01     RID_SVXSTR_COLOR_SUN_DEF
//BFS01 };
//BFS01 static USHORT __READONLY_DATA aResId[] =
//BFS01 {
//BFS01     RID_SVXSTR_BLUEGREY,
//BFS01     RID_SVXSTR_BLACK,
//BFS01     RID_SVXSTR_BLUE,
//BFS01     RID_SVXSTR_GREEN,
//BFS01     RID_SVXSTR_CYAN,
//BFS01     RID_SVXSTR_RED,
//BFS01     RID_SVXSTR_MAGENTA,
//BFS01     RID_SVXSTR_BROWN,
//BFS01     RID_SVXSTR_GREY,
//BFS01     RID_SVXSTR_LIGHTGREY,
//BFS01     RID_SVXSTR_LIGHTBLUE,
//BFS01     RID_SVXSTR_LIGHTGREEN,
//BFS01     RID_SVXSTR_LIGHTCYAN,
//BFS01     RID_SVXSTR_LIGHTRED,
//BFS01     RID_SVXSTR_LIGHTMAGENTA,
//BFS01     RID_SVXSTR_YELLOW,
//BFS01     RID_SVXSTR_WHITE,
//BFS01     RID_SVXSTR_ORANGE,
//BFS01     RID_SVXSTR_VIOLET,
//BFS01     RID_SVXSTR_BORDEAUX,
//BFS01     RID_SVXSTR_PALE_YELLOW,
//BFS01     RID_SVXSTR_PALE_GREEN,
//BFS01     RID_SVXSTR_DKVIOLET,
//BFS01     RID_SVXSTR_SALMON,
//BFS01     RID_SVXSTR_SEABLUE,
//BFS01     RID_SVXSTR_COLOR_SUN
//BFS01 };
//BFS01
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( int i=0; i<(sizeof(aDefResId) / sizeof(USHORT)) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( aDefResId[i] );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( aResId[i] ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XColorTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XColorEntry* pEntry = NULL;
//BFS01 long         nType;
//BFS01 long         nCount;
//BFS01 long         nIndex;
//BFS01 USHORT       nRed;
//BFS01 USHORT       nGreen;
//BFS01 USHORT       nBlue;
//BFS01 Color        aColor;
//BFS01 XubString    aName;
//BFS01
//BFS01 rIn >> nType;
//BFS01
//BFS01 // gesamte Tabelle?
//BFS01 if (nType == 0)
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01
//BFS01         aColor = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                         (BYTE) ( nGreen >> 8 ),
//BFS01                         (BYTE) ( nBlue  >> 8 ) );
//BFS01         pEntry = new XColorEntry( aColor, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 else // Version ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         if( aIOC.GetVersion() >= 0 )
//BFS01         {
//BFS01             rIn >> nRed;
//BFS01             rIn >> nGreen;
//BFS01             rIn >> nBlue;
//BFS01             aColor = Color( (BYTE) ( nRed   >> 8 ),
//BFS01                             (BYTE) ( nGreen >> 8 ),
//BFS01                             (BYTE) ( nBlue  >> 8 ) );
//BFS01         }
//BFS01         /*
//BFS01         else if( aIOC.GetVersion() >= 1 )
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01         */
//BFS01
//BFS01         pEntry = new XColorEntry( aColor, aName );
//BFS01         Insert( nIndex, pEntry );
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// --------------------
// class XColorList
// --------------------

/*************************************************************************
|*
|* XColorList::XColorList()
|*
*************************************************************************/

XColorList::XColorList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize)
{
    // pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XColorList::~XColorList()
{
}

/************************************************************************/

XColorEntry* XColorList::Replace(XColorEntry* pEntry, long nIndex )
{
    return (XColorEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XColorEntry* XColorList::Remove(long nIndex)
{
    return (XColorEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XColorEntry* XColorList::GetColor(long nIndex) const
{
    return (XColorEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XColorList::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XColorList::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XColorList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Erstmal von XColorTable uebernommen !!!
//BFS01
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 rOut << (long)0;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Farben
//BFS01 XColorEntry* pEntry = (XColorEntry*)aList.First();
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // rOut << (long)aList.GetCurKey();
//BFS01
//BFS01     // UNICODE:rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     rOut << pEntry->GetColor().GetRed();
//BFS01     rOut << pEntry->GetColor().GetGreen();
//BFS01     rOut << pEntry->GetColor().GetBlue();
//BFS01     pEntry = (XColorEntry*)aList.Next();
//BFS01 }
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XColorList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 // Erstmal von XColorTable uebernommen !!!
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01
//BFS01 XColorEntry* pEntry = NULL;
//BFS01 long         nType;
//BFS01 long         nCount;
//BFS01 long         nIndex;
//BFS01 USHORT       nRed;
//BFS01 USHORT       nGreen;
//BFS01 USHORT       nBlue;
//BFS01 XubString    aName;
//BFS01
//BFS01 rIn >> nType;
//BFS01
//BFS01 // gesamte Tabelle?
//BFS01 if (nType == 0)
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         rIn >> nRed;
//BFS01         rIn >> nGreen;
//BFS01         rIn >> nBlue;
//BFS01
//BFS01         pEntry = new XColorEntry (Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue), aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// eof
