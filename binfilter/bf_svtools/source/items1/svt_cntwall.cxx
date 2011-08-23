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


#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>

#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif

#include <bf_svtools/cntwall.hxx>

#define CNTWALLPAPERITEM_STREAM_MAGIC   ( (UINT32)0xfefefefe )
#define CNTWALLPAPERITEM_STREAM_SEEKREL (-( (long)( sizeof( UINT32 ) ) ) )

namespace binfilter
{

TYPEINIT1( CntWallpaperItem, SfxPoolItem );

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( USHORT which, SvStream& rStream, USHORT nVersion )
    : SfxPoolItem( which ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
    UINT32 nMagic = 0;
    rStream >> nMagic;
    if ( nMagic == CNTWALLPAPERITEM_STREAM_MAGIC )
    {
        // Okay, data were stored by CntWallpaperItem.

        readUnicodeString(rStream, _aURL, nVersion >= 1);
        // !!! Color stream operators do not work - they discard any
        // transparency info !!!
        _nColor.Read( rStream, TRUE );
        rStream >> _nStyle;
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
            VersionCompat aCompat( rStream, STREAM_READ );
        }

        // Read SfxWallpaperItem's string member _aURL.
        readUnicodeString(rStream, _aURL, false);

        // "Read" SfxWallpaperItem's string member _aFilter.
        ByteString aDummy;
        rStream.ReadByteString(aDummy);
    }
}

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( const CntWallpaperItem& rItem ) :
    SfxPoolItem( rItem ),
    _aURL( rItem._aURL ),
    _nColor( rItem._nColor ),
    _nStyle( rItem._nStyle )
{
}

// -----------------------------------------------------------------------
CntWallpaperItem::~CntWallpaperItem()
{
}

// -----------------------------------------------------------------------
int CntWallpaperItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    const CntWallpaperItem& rWallItem = (const CntWallpaperItem&)rItem;

    if( ( rWallItem._nStyle == _nStyle ) &&
        ( rWallItem._nColor == _nColor ) &&
        ( rWallItem._aURL == _aURL ) )
        return TRUE;
    else
        return FALSE;
}

//============================================================================
// virtual
USHORT CntWallpaperItem::GetVersion(USHORT) const
{
    return 1; // because it uses SfxPoolItem::read/writeUnicodeString()
}

// -----------------------------------------------------------------------
SfxPoolItem* CntWallpaperItem::Create( SvStream& rStream, USHORT nVersion) const
{
    return new CntWallpaperItem( Which(), rStream, nVersion );
}

// -----------------------------------------------------------------------
SvStream& CntWallpaperItem::Store( SvStream& rStream, USHORT ) const
{
    rStream << CNTWALLPAPERITEM_STREAM_MAGIC;
    writeUnicodeString(rStream, _aURL);
    // !!! Color stream operators do not work - they discard any
    // transparency info !!!
    // ??? Why the hell Color::Write(...) isn't const ???
    SAL_CONST_CAST( CntWallpaperItem*, this )->_nColor.Write( rStream, TRUE );
    rStream << _nStyle;

    return rStream;
}

// -----------------------------------------------------------------------
SfxPoolItem* CntWallpaperItem::Clone( SfxItemPool* ) const
{
    return new CntWallpaperItem( *this );
}

//----------------------------------------------------------------------------
// virtual
BOOL CntWallpaperItem::QueryValue( com::sun::star::uno::Any&,BYTE ) const
{
    DBG_ERROR("Not implemented!");
    return FALSE;
}

//----------------------------------------------------------------------------
// virtual
BOOL CntWallpaperItem::PutValue( const com::sun::star::uno::Any&,BYTE )
{
    DBG_ERROR("Not implemented!");
    return FALSE;
}


}
