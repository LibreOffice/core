/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <svl/cntwall.hxx>

#define CNTWALLPAPERITEM_STREAM_MAGIC   ( (sal_uInt32)0xfefefefe )
#define CNTWALLPAPERITEM_STREAM_SEEKREL (-( (long)( sizeof( sal_uInt32 ) ) ) )

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( sal_uInt16 which )
    : SfxPoolItem( which ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
}

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( sal_uInt16 which, SvStream& rStream, sal_uInt16 nVersion )
    : SfxPoolItem( which ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
    sal_uInt32 nMagic = 0;
    rStream >> nMagic;
    if ( nMagic == CNTWALLPAPERITEM_STREAM_MAGIC )
    {
        // Okay, data were stored by CntWallpaperItem.

        readUnicodeString(rStream, _aURL, nVersion >= 1);
        // !!! Color stream operators do not work - they discard any
        // transparency info !!!
        _nColor.Read( rStream, sal_True );
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
        return sal_True;
    else
        return sal_False;
}

//============================================================================
// virtual
sal_uInt16 CntWallpaperItem::GetVersion(sal_uInt16) const
{
    return 1; // because it uses SfxPoolItem::read/writeUnicodeString()
}

// -----------------------------------------------------------------------
SfxPoolItem* CntWallpaperItem::Create( SvStream& rStream, sal_uInt16 nVersion) const
{
    return new CntWallpaperItem( Which(), rStream, nVersion );
}

// -----------------------------------------------------------------------
SvStream& CntWallpaperItem::Store( SvStream& rStream, sal_uInt16 ) const
{
    rStream << CNTWALLPAPERITEM_STREAM_MAGIC;
    writeUnicodeString(rStream, _aURL);
    // !!! Color stream operators do not work - they discard any
    // transparency info !!!
    // ??? Why the hell Color::Write(...) isn't const ???
    SAL_CONST_CAST( CntWallpaperItem*, this )->_nColor.Write( rStream, sal_True );
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
sal_Bool CntWallpaperItem::QueryValue( com::sun::star::uno::Any&,sal_uInt8 ) const
{
    DBG_ERROR("Not implemented!");
    return sal_False;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool CntWallpaperItem::PutValue( const com::sun::star::uno::Any&,sal_uInt8 )
{
    DBG_ERROR("Not implemented!");
    return sal_False;
}


