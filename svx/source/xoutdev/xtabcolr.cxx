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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include "XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include <unotools/pathoptions.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;

using ::rtl::OUString;

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

        uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( FALSE );
}

/************************************************************************/

BOOL XColorTable::Save()
{
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
}

/************************************************************************/

BOOL XColorTable::Create()
{
    XubString aStr;
    xub_StrLen nLen;
    ResMgr& rRes = DIALOG_MGR();

    static USHORT const aResId[] =
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

    static ColorData const aColTab[] =
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
                                    String( ResId( aResId[ n ], rRes )) ) );
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

    // Chart default colors
    aStr = SVX_RESSTR( RID_SVXSTR_COLOR_CHART );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( 92, new XColorEntry( Color( 0x00, 0x45, 0x86 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( 93, new XColorEntry( Color( 0xff, 0x42, 0x0e ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( 94, new XColorEntry( Color( 0xff, 0xd3, 0x20 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( 95, new XColorEntry( Color( 0x57, 0x9d, 0x1c ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( 96, new XColorEntry( Color( 0x7e, 0x00, 0x21 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( 97, new XColorEntry( Color( 0x83, 0xca, 0xff ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( 98, new XColorEntry( Color( 0x31, 0x40, 0x04 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( 99, new XColorEntry( Color( 0xae, 0xcf, 0x00 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('9'));
    Insert( 100, new XColorEntry( Color( 0x4b, 0x1f, 0x6f ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('1'));
    aStr.AppendAscii("0");
    nLen = aStr.Len() - 1;
    Insert( 101, new XColorEntry( Color( 0xff, 0x95, 0x0e ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('1'));
    Insert( 102, new XColorEntry( Color( 0xc5, 0x00, 0x0b ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( 103, new XColorEntry( Color( 0x00, 0x84, 0xd1 ), aStr ) );

    return( Count() == 104 );
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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
