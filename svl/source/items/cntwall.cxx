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


#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <svl/cntwall.hxx>
#include <stringio.hxx>

#define CNTWALLPAPERITEM_STREAM_MAGIC   ( (sal_uInt32)0xfefefefe )
#define CNTWALLPAPERITEM_STREAM_SEEKREL (-( (long)( sizeof( sal_uInt32 ) ) ) )



CntWallpaperItem::CntWallpaperItem( sal_uInt16 which )
    : SfxPoolItem( which ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
}


CntWallpaperItem::CntWallpaperItem( sal_uInt16 which, SvStream& rStream, sal_uInt16 nVersion )
    : SfxPoolItem( which ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
    sal_uInt32 nMagic = 0;
    rStream.ReadUInt32( nMagic );
    if ( nMagic == CNTWALLPAPERITEM_STREAM_MAGIC )
    {
        // Okay, data were stored by CntWallpaperItem.

        _aURL = readUnicodeString(rStream, nVersion >= 1);
        // !!! Color stream operators do not work - they discard any
        // transparency info !!!
        _nColor.Read( rStream );
        rStream.ReadUInt16( _nStyle );
    }
    else
    {
        rStream.SeekRel( CNTWALLPAPERITEM_STREAM_SEEKREL );

        // Data were stored by SfxWallpaperItem ( SO < 6.0 ). The only
        // thing we can do here is to get the URL and to position the stream.

        {
            // "Read" Wallpaper member - The version compat object positions
            // the stream after the wallpaper data in its dtor. We must use
            // this trick here as no VCL must be used here ( No Wallpaper
            // object allowed ).
            VersionCompat aCompat( rStream, StreamMode::READ );
        }

        // Read SfxWallpaperItem's string member _aURL.
        _aURL = readUnicodeString(rStream, false);

        // "Read" SfxWallpaperItem's string member _aFilter.
        read_uInt16_lenPrefixed_uInt8s_ToOString(rStream);
    }
}


CntWallpaperItem::CntWallpaperItem( const CntWallpaperItem& rItem ) :
    SfxPoolItem( rItem ),
    _aURL( rItem._aURL ),
    _nColor( rItem._nColor ),
    _nStyle( rItem._nStyle )
{
}


CntWallpaperItem::~CntWallpaperItem()
{
}


bool CntWallpaperItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    const CntWallpaperItem& rWallItem = static_cast<const CntWallpaperItem&>(rItem);

    return ( rWallItem._nStyle == _nStyle ) &&
        ( rWallItem._nColor == _nColor ) &&
        ( rWallItem._aURL == _aURL );
}

// virtual
sal_uInt16 CntWallpaperItem::GetVersion(sal_uInt16) const
{
    return 1; // because it uses SfxPoolItem::read/writeUnicodeString()
}


SfxPoolItem* CntWallpaperItem::Create( SvStream& rStream, sal_uInt16 nVersion) const
{
    return new CntWallpaperItem( Which(), rStream, nVersion );
}


SvStream& CntWallpaperItem::Store( SvStream& rStream, sal_uInt16 ) const
{
    rStream.WriteUInt32( CNTWALLPAPERITEM_STREAM_MAGIC );
    writeUnicodeString(rStream, _aURL);
    // !!! Color stream operators do not work - they discard any
    // transparency info !!!
    // ??? Why the hell Color::Write(...) isn't const ???
    (const_cast< CntWallpaperItem* >(this))->_nColor.Write( rStream );
    rStream.WriteUInt16( _nStyle );

    return rStream;
}


SfxPoolItem* CntWallpaperItem::Clone( SfxItemPool* ) const
{
    return new CntWallpaperItem( *this );
}

// virtual
bool CntWallpaperItem::QueryValue( css::uno::Any&, sal_uInt8) const
{
    OSL_FAIL("Not implemented!");
    return false;
}

// virtual
bool CntWallpaperItem::PutValue( const css::uno::Any&, sal_uInt8)
{
    OSL_FAIL("Not implemented!");
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
