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

#include <memory>
#include <svx/XPropertyTable.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

using namespace com::sun::star;

XColorListRef XColorList::CreateStdColorList()
{
    return XPropertyList::AsColorList(
        XPropertyList::CreatePropertyList(
            XPropertyListType::Color, !utl::ConfigManager::IsFuzzing() ?
                                          SvtPathOptions().GetPalettePath() :
                                          "", ""));
}

XColorListRef XColorList::GetStdColorList()
{
    XColorListRef aTable( CreateStdColorList() );
    return aTable;
}

void XColorList::Replace(long nIndex, std::unique_ptr<XColorEntry> pEntry)
{
    XPropertyList::Replace(std::move(pEntry), nIndex);
}
XColorEntry* XColorList::GetColor(long nIndex) const
{
    return static_cast<XColorEntry*>( XPropertyList::Get(nIndex) );
}

uno::Reference< container::XNameContainer > XColorList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXColorTable_createInstance( this ),
        uno::UNO_QUERY );
}

bool XColorList::Create()
{
    sal_uInt32 a(0);
    sal_uInt32 b(0);

    // <!-- Gray palette from white to black -->
    const OUString aStrGrey( SvxResId( RID_SVXSTR_COLOR_GREY ) );

    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0xff, 0xff ), SvxResId( RID_SVXSTR_COLOR_WHITE ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xee, 0xee, 0xee ), aStrGrey + " 1" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xdd, 0xdd, 0xdd ), aStrGrey + " 2" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xcc, 0xcc, 0xcc ), aStrGrey + " 3" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xb2, 0xb2, 0xb2 ), aStrGrey + " 4" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x99, 0x99, 0x99 ), aStrGrey + " 5" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x80, 0x80, 0x80 ), aStrGrey + " 6" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x66, 0x66, 0x66 ), aStrGrey + " 7" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x33, 0x33, 0x33 ), aStrGrey + " 8" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x1c, 0x1c, 0x1c ), aStrGrey + " 9" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x11, 0x11, 0x11 ), aStrGrey + " 10") );
    Insert( std::make_unique<XColorEntry>( Color( 0x00, 0x00, 0x00 ), SvxResId( RID_SVXSTR_COLOR_BLACK ) ) );

    // <!-- Base colors step 0 to 10 -->
    const sal_uInt32 nNumColorsInGroup(12);
    const sal_uInt32 nNumGroups(11);
    const OUString aStrCol[nNumColorsInGroup] = {
        SvxResId(RID_SVXSTR_COLOR_YELLOW),
        SvxResId(RID_SVXSTR_COLOR_ORANGE),
        SvxResId(RID_SVXSTR_COLOR_RED),
        SvxResId(RID_SVXSTR_COLOR_PINK),
        SvxResId(RID_SVXSTR_COLOR_MAGENTA),
        SvxResId(RID_SVXSTR_COLOR_PURPLE),
        SvxResId(RID_SVXSTR_COLOR_BLUE),
        SvxResId(RID_SVXSTR_COLOR_SKYBLUE),
        SvxResId(RID_SVXSTR_COLOR_CYAN),
        SvxResId(RID_SVXSTR_COLOR_TURQUOISE),
        SvxResId(RID_SVXSTR_COLOR_GREEN),
        SvxResId(RID_SVXSTR_COLOR_YELLOWGREEN) };
    static const sal_uInt32 aStdCol[nNumColorsInGroup * nNumGroups] = {
        0xffff99, 0xff6600, 0xff3333, 0xff00cc, 0xff33ff, 0x9900ff, 0x6666ff, 0x00ccff, 0x66ffff, 0x33ff99, 0x99ff66, 0xccff00,
        0xffff66, 0xffcc00, 0xff9999, 0xff66cc, 0xff99ff, 0xcc66ff, 0x9999ff, 0x9999ff, 0x99ffff, 0x66ff99, 0x99ff99, 0xccff66,
        0xffff00, 0xff9900, 0xff6666, 0xff3399, 0xff66ff, 0x9933ff, 0x3333ff, 0x3399ff, 0x00ffff, 0x00ff66, 0x66ff66, 0x99ff33,
        0xcc9900, 0xff3300, 0xff0000, 0xff0066, 0xff00ff, 0x6600ff, 0x0000ff, 0x0066ff, 0x00cccc, 0x00cc33, 0x00cc00, 0x66ff00,
        0x996600, 0xcc3300, 0xcc0000, 0xcc0066, 0xcc00cc, 0x6600cc, 0x0000cc, 0x0066cc, 0x009999, 0x009933, 0x009900, 0x66cc00,
        0x663300, 0x801900, 0x990000, 0x990066, 0x990099, 0x330099, 0x000099, 0x006699, 0x006666, 0x007826, 0x006600, 0x669900,
        0x333300, 0x461900, 0x330000, 0x330033, 0x660066, 0x000033, 0x000066, 0x000080, 0x003333, 0x00331a, 0x003300, 0x193300,
        0x666633, 0x661900, 0x663333, 0x660033, 0x663366, 0x330066, 0x333366, 0x003366, 0x336666, 0x006633, 0x336633, 0x336600,
        0x999966, 0x996633, 0x996666, 0x993366, 0x996699, 0x663399, 0x666699, 0x336699, 0x669999, 0x339966, 0x669966, 0x669933,
        0xcccc99, 0xcc9966, 0xcc9999, 0xcc6699, 0xcc99cc, 0x9966cc, 0x9999cc, 0x6699cc, 0x99cccc, 0x66cc99, 0x99cc99, 0x99cc66,
        0xffffcc, 0xffcc99, 0xffcccc, 0xff99cc, 0xffccff, 0xcc99ff, 0xccccff, 0x99ccff, 0xccffff, 0x99ffcc, 0xccffcc, 0xccff99 };

    for(a = 0; a < nNumGroups; a++)
    {
        OUString aSuffix;
        if (a > 0) aSuffix = OUString::number(a);

        const sal_uInt32 nOffset(a * nNumColorsInGroup);

        for(b = 0; b < nNumColorsInGroup; b++)
        {
            Insert( std::make_unique<XColorEntry>( Color(aStdCol[nOffset + b]), aStrCol[b] + aSuffix ) );
        }
    }

    // <!-- use some 'nice' colors from original palette -->
    Insert( std::make_unique<XColorEntry>( Color( 0xe6, 0xe6, 0xff ), SvxResId( RID_SVXSTR_COLOR_BLUEGREY) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xCF, 0xE7, 0xF5  ), SvxResId( RID_SVXSTR_COLOR_BLUE_CLASSIC ) ) );

    // <!-- add 'pale' colors from original palette -->
    Insert( std::make_unique<XColorEntry>( Color( 0x99, 0x99, 0xff ), SvxResId( RID_SVXSTR_COLOR_VIOLET ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x99, 0x33, 0x66 ), SvxResId( RID_SVXSTR_COLOR_BORDEAUX ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0xff, 0xcc ), SvxResId( RID_SVXSTR_COLOR_PALE_YELLOW ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xcc, 0xff, 0xff ), SvxResId( RID_SVXSTR_COLOR_PALE_GREEN ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x66, 0x00, 0x66 ), SvxResId( RID_SVXSTR_COLOR_DARKVIOLET ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0x80, 0x80 ), SvxResId( RID_SVXSTR_COLOR_SALMON ) ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x00, 0x66, 0xcc ), SvxResId( RID_SVXSTR_COLOR_SEABLUE ) ) );

    // <!-- add Chart colors from original palette (also 12, coincidence?) -->
    const OUString aStrChart( SvxResId( RID_SVXSTR_COLOR_CHART ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x00, 0x45, 0x86 ), aStrChart + " 1" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0x42, 0x0e ), aStrChart + " 2" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0xd3, 0x20 ), aStrChart + " 3" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x57, 0x9d, 0x1c ), aStrChart + " 4" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x7e, 0x00, 0x21 ), aStrChart + " 5" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x83, 0xca, 0xff ), aStrChart + " 6" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x31, 0x40, 0x04 ), aStrChart + " 7" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xae, 0xcf, 0x00 ), aStrChart + " 8" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x4b, 0x1f, 0x6f ), aStrChart + " 9" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xff, 0x95, 0x0e ), aStrChart + " 10" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0xc5, 0x00, 0x0b ), aStrChart + " 11" ) );
    Insert( std::make_unique<XColorEntry>( Color( 0x00, 0x84, 0xd1 ), aStrChart + " 12" ) );

    return(165 == Count());
}

BitmapEx XColorList::CreateBitmapForUI( long /*nIndex*/ )
{
    return BitmapEx();
}

long XColorList::GetIndexOfColor( const Color& rColor ) const
{
    for( long i = 0, n = maList.size(); i < n; ++i )
    {
        const Color aColor = static_cast<XColorEntry*>( maList[i].get() )->GetColor();

        if (aColor == rColor )
            return i;
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
