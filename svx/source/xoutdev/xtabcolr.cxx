/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "svx/XPropertyTable.hxx"
#include <unotools/pathoptions.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

using namespace com::sun::star;


XColorListRef XColorList::CreateStdColorList()
{
    return XPropertyList::CreatePropertyList(
        XCOLOR_LIST, SvtPathOptions().GetPalettePath() )->AsColorList();
}

XColorListRef XColorList::GetStdColorList()
{
    static XColorListRef aTable( CreateStdColorList() );
    return aTable;
}

XColorEntry* XColorList::Replace(long nIndex, XColorEntry* pEntry )
{
    return (XColorEntry*)XPropertyList::Replace( pEntry, nIndex );
}

XColorEntry* XColorList::Remove(long nIndex)
{
    return (XColorEntry*) XPropertyList::Remove(nIndex);
}

XColorEntry* XColorList::GetColor(long nIndex) const
{
    return (XColorEntry*) XPropertyList::Get(nIndex, 0);
}

uno::Reference< container::XNameContainer > XColorList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXColorTable_createInstance( this ),
        uno::UNO_QUERY );
}

sal_Bool XColorList::Create()
{
    XubString aStr;
    xub_StrLen nLen;
    ResMgr& rRes = DIALOG_MGR();

    static sal_uInt16 aResId[] =
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

    for( sal_uInt16 n = 0; n < 16; ++n )
    {
        Insert( new XColorEntry(
                    Color( aColTab[n] ),
                    String( ResId( aResId[ n ], rRes ) )
                ),
                n
        );
    }

    aStr = SVX_RESSTR( RID_SVXSTR_GREY );
    aStr.AppendAscii(" 80%");
    nLen = aStr.Len() - 3;
    Insert( new XColorEntry( Color( 51, 51, 51 ), aStr ), 16 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color( 76, 76, 76 ), aStr ), 17 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color(102,102,102 ), aStr ), 18 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(153,153,153 ), aStr ), 19 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(179,179,179 ), aStr ), 20 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(204,204,204 ), aStr ), 21 );
    // BM: new 15%
    aStr.SetChar(nLen, sal_Unicode('1'));
    aStr.SetChar(nLen + 1, sal_Unicode('5'));
    Insert( new XColorEntry( Color(217,217,217 ), aStr ), 22 );
    aStr.SetChar(nLen + 1, sal_Unicode('0'));
    Insert( new XColorEntry( Color(230,230,230 ), aStr ), 23 );
    Insert( new XColorEntry( Color(230,230,255 ), SVX_RESSTR( RID_SVXSTR_BLUEGREY ) ), 24 );

    aStr = SVX_RESSTR( RID_SVXSTR_RED );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color(255, 51,102 ), aStr ), 25 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(220, 35,  0 ), aStr ), 26 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(184, 71,  0 ), aStr ), 27 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(255, 51, 51 ), aStr ), 28 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color(235, 97, 61 ), aStr ), 29 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color(184, 71, 71 ), aStr ), 30 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color(184,  0, 71 ), aStr ), 31 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color(153, 40, 76 ), aStr ), 32 );

    aStr = SVX_RESSTR( RID_SVXSTR_MAGENTA );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color(148,  0,107 ), aStr ), 33 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(148, 71,107 ), aStr ), 34 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(148, 71,148 ), aStr ), 35 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(153,102,204 ), aStr ), 36 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color(107, 71,148 ), aStr ), 37 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color(107, 35,148 ), aStr ), 38 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color(107,  0,148 ), aStr ), 39 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color( 94, 17,166 ), aStr ), 40 );

    aStr = SVX_RESSTR( RID_SVXSTR_BLUE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 40,  0,153 ), aStr ), 41 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 71,  0,184 ), aStr ), 42 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color( 35,  0,220 ), aStr ), 43 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color( 35, 35,220 ), aStr ), 44 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color(  0, 71,255 ), aStr ), 45 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color(  0,153,255 ), aStr ), 46 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color(  0,184,255 ), aStr ), 47 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color(153,204,255 ), aStr ), 48 );

    aStr = SVX_RESSTR( RID_SVXSTR_CYAN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color(  0,220,255 ), aStr ), 49 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(  0,204,204 ), aStr ), 50 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color( 35,184,220 ), aStr ), 51 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color( 71,184,184 ), aStr ), 52 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color( 51,163,163 ), aStr ), 53 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color( 25,138,138 ), aStr ), 54 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color(  0,107,107 ), aStr ), 55 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color(  0, 74, 74 ), aStr ), 56 );

    aStr = SVX_RESSTR( RID_SVXSTR_GREEN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 53, 94,  0 ), aStr ), 57 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 92,133, 38 ), aStr ), 58 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(125,166, 71 ), aStr ), 59 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(148,189, 94 ), aStr ), 60 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color(  0,174,  0 ), aStr ), 61 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color( 51,204,102 ), aStr ), 62 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color( 61,235, 61 ), aStr ), 63 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color( 35,255, 35 ), aStr ), 64 );

    aStr = SVX_RESSTR( RID_SVXSTR_YELLOW );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color(230,255,  0 ), aStr ), 65 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(255,255,153 ), aStr ), 66 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(255,255,102 ), aStr ), 67 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(230,230, 76 ), aStr ), 68 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color(204,204,  0 ), aStr ), 69 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color(179,179,  0 ), aStr ), 70 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color(128,128, 25 ), aStr ), 71 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color(102,102,  0 ), aStr ), 72 );

    aStr = SVX_RESSTR( RID_SVXSTR_BROWN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 76, 25,  0 ), aStr ), 73 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(102, 51,  0 ), aStr ), 74 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(128, 76, 25 ), aStr ), 75 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(153,102, 51 ), aStr ), 76 );

    aStr = SVX_RESSTR( RID_SVXSTR_ORANGE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color(204,102, 51 ), aStr ), 77 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color(255,102, 51 ), aStr ), 78 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color(255,153,102 ), aStr ), 79 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color(255,204,153 ), aStr ), 80 );

    // new chart colors
    aStr = SVX_RESSTR( RID_SVXSTR_VIOLET );
    Insert( new XColorEntry( Color( 0x99, 0x99, 0xff ), aStr ), 81 );

    aStr = SVX_RESSTR( RID_SVXSTR_BORDEAUX );
    Insert( new XColorEntry( Color( 0x99, 0x33, 0x66 ), aStr ), 82 );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_YELLOW );
    Insert( new XColorEntry( Color( 0xff, 0xff, 0xcc ), aStr ), 83 );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_GREEN );
    Insert( new XColorEntry( Color( 0xcc, 0xff, 0xff ), aStr ), 84 );

    aStr = SVX_RESSTR( RID_SVXSTR_DKVIOLET );
    Insert( new XColorEntry( Color( 0x66, 0x00, 0x66 ), aStr ), 85 );

    aStr = SVX_RESSTR( RID_SVXSTR_SALMON );
    Insert( new XColorEntry( Color( 0xff, 0x80, 0x80 ), aStr ), 86 );

    aStr = SVX_RESSTR( RID_SVXSTR_SEABLUE );
    Insert( new XColorEntry( Color( 0x00, 0x66, 0xcc ), aStr ), 87 );

    // Sun colors
    aStr = SVX_RESSTR( RID_SVXSTR_COLOR_SUN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 0x33, 0x33, 0x66 ), aStr ), 88 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 0x66, 0x66, 0x99 ), aStr ), 89 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color( 0x99, 0x99, 0xcc ), aStr ), 90 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color( 0xcc, 0xcc, 0xff ), aStr ), 91 );

    // Chart default colors
    aStr = SVX_RESSTR( RID_SVXSTR_COLOR_CHART );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 0x00, 0x45, 0x86 ), aStr ), 92 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 0xff, 0x42, 0x0e ), aStr ), 93 );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XColorEntry( Color( 0xff, 0xd3, 0x20 ), aStr ), 94 );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XColorEntry( Color( 0x57, 0x9d, 0x1c ), aStr ), 95 );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert( new XColorEntry( Color( 0x7e, 0x00, 0x21 ), aStr ), 96 );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert( new XColorEntry( Color( 0x83, 0xca, 0xff ), aStr ), 97 );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert( new XColorEntry( Color( 0x31, 0x40, 0x04 ), aStr ), 98 );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert( new XColorEntry( Color( 0xae, 0xcf, 0x00 ), aStr ), 99 );
    aStr.SetChar(nLen, sal_Unicode('9'));
    Insert( new XColorEntry( Color( 0x4b, 0x1f, 0x6f ), aStr ), 100 );
    aStr.SetChar(nLen, sal_Unicode('1'));
    aStr.AppendAscii("0");
    nLen = aStr.Len() - 1;
    Insert( new XColorEntry( Color( 0xff, 0x95, 0x0e ), aStr ), 101 );
    aStr.SetChar(nLen, sal_Unicode('1'));
    Insert( new XColorEntry( Color( 0xc5, 0x00, 0x0b ), aStr ), 102 );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 0x00, 0x84, 0xd1 ), aStr ), 103 );

    return Count() == 104;
}

Bitmap XColorList::CreateBitmapForUI( long /*nIndex*/ )
{
    return Bitmap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
