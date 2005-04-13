/*************************************************************************
 *
 *  $RCSfile: cntwall.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:14:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>

#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif

#include "cntwall.hxx"

#define CNTWALLPAPERITEM_STREAM_MAGIC   ( (UINT32)0xfefefefe )
#define CNTWALLPAPERITEM_STREAM_SEEKREL (-( (long)( sizeof( UINT32 ) ) ) )

TYPEINIT1( CntWallpaperItem, SfxPoolItem );

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( USHORT nWhich )
    : SfxPoolItem( nWhich ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
{
}

// -----------------------------------------------------------------------
CntWallpaperItem::CntWallpaperItem( USHORT nWhich, SvStream& rStream, USHORT nVersion )
    : SfxPoolItem( nWhich ), _nColor( COL_TRANSPARENT ), _nStyle( 0 )
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
BOOL CntWallpaperItem::QueryValue( com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId ) const
{
    DBG_ERROR("Not implemented!");
    return FALSE;
}

//----------------------------------------------------------------------------
// virtual
BOOL CntWallpaperItem::PutValue( const com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId )
{
    DBG_ERROR("Not implemented!");
    return FALSE;
}


