/*************************************************************************
 *
 *  $RCSfile: glyphcache.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: hdu $ $Date: 2000-11-10 17:21:28 $
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

#include <gcach_vdev.hxx>
#include <gcach_ftyp.hxx>

#include <svapp.hxx>
#include <bitmap.hxx>
#include <outfont.hxx>

#include <rtl/ustring>      // used only for string=>hashvalue
#include <osl/file.hxx>
#include <tools/debug.hxx>

#include <stdlib.h>

// =======================================================================
// GlyphCache
// =======================================================================

GlyphCache GlyphCache::aSingleton( 250000 );
static class FreetypeManager* pFtManager = NULL;

// -----------------------------------------------------------------------

GlyphCache::GlyphCache( ULONG _nMaxSize )
:   mnMaxSize(_nMaxSize), mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0), mnLastGC(0), mnMinGCInterval(200)
{
#ifndef NO_FREETYPE_FONTS
    if( const char* pFontPath = getenv( "SAL_FONTPATH_PRIVATE" ) )
    {
        String aEnv = String::CreateFromAscii( pFontPath );
        ::rtl::OUString aNormalizedName;
        osl::FileBase::normalizePath( aEnv, aNormalizedName );
        pFtManager = new FreetypeManager;
        pFtManager->AddFontDir( aNormalizedName );
    }
#endif // NO_FREETYPE_FONTS
}

// -----------------------------------------------------------------------

GlyphCache::~GlyphCache()
{
// TODO:
//  for( FontList::iterator it = aFontList.begin(); it != aFontList.end(); ++it )
//      delete const_cast<ServerFont*>( it->second );
#ifndef NO_FREETYPE_FONTS
    if( pFtManager )
        delete pFtManager;
#endif // NO_FREETYPE_FONTS
}

// -----------------------------------------------------------------------

size_t std::hash<const ImplFontSelectData>::operator()( const ImplFontSelectData& rFontSelData ) const
{
    // TODO: does it pay much to improve this hash function?
    size_t nHash = ::rtl::OUString( rFontSelData.maName ).hashCode();
    nHash   += ::rtl::OUString( rFontSelData.maStyleName ).hashCode();
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    return nHash;
}

// -----------------------------------------------------------------------

bool std::equal_to<const ImplFontSelectData>::operator()(
    const ImplFontSelectData& rA, const ImplFontSelectData& rB ) const
{
    if( (rA.maName          == rB.maName)
    &&  (rA.maStyleName     == rB.maStyleName)
    &&  (rA.mnHeight        == rB.mnHeight)
    &&  (rA.mnWidth         == rB.mnWidth)
    &&  (rA.mnOrientation   == rB.mnOrientation)
    &&  (rA.mbVertical      == rB.mbVertical) )
        return true;
    return false;
}

// -----------------------------------------------------------------------

long GlyphCache::FetchFontList( ImplDevFontList* pList )
{
    long nCount = VirtDevServerFont::FetchFontList( pList );
#ifndef NO_FREETYPE_FONTS
    nCount += pFtManager->FetchFontList( pList );
#endif // NO_FREETYPE_FONTS
    return nCount;
}

// -----------------------------------------------------------------------

const ServerFont* GlyphCache::CacheFont( const ImplFontSelectData& rFontSelData )
{
    FontList::const_iterator it = aFontList.find( rFontSelData );
    if( it != aFontList.end() )
    {
        const ServerFont* pFound = it->second;
        pFound->AddRef();
        return pFound;
    }

    // font not cached yet => create new font item
    const ServerFont* pNew = NULL;
    // TODO:
    // pNew = VirtDevServerFont::CreateFont( rFontSelData );
#ifndef NO_FREETYPE_FONTS
    if( !pNew && pFtManager)
        pNew = pFtManager->CreateFont( rFontSelData );
#endif // NO_FREETYPE_FONTS

    if( pNew )
    {
        aFontList[ rFontSelData ] = pNew;
        mnBytesUsed += pNew->GetByteCount();
    }

    return pNew;
}

// -----------------------------------------------------------------------

void GlyphCache::UncacheFont( const ServerFont& rServerFont )
{
    // the interface for rServerFont must be const because a
    // user who wants to release it only got const ServerFonts.
    // The caching algorithm needs a non-const object though
    ServerFont* pFont = const_cast<ServerFont*>( &rServerFont );
    if( pFont->Release() <= 0)
        // lazy release
        mnBytesUsed -= pFont->GarbageCollect( mnLruIndex );
}

// -----------------------------------------------------------------------

ULONG GlyphCache::CalcByteCount() const
{
    ULONG nCacheSize = sizeof(*this);
    for( FontList::const_iterator it = aFontList.begin(); it != aFontList.end(); ++it )
        nCacheSize += it->second->GetByteCount();
    return nCacheSize;
}

// -----------------------------------------------------------------------

ULONG GlyphCache::GarbageCollect()
{
    if( mnLruIndex - mnLastGC < mnMinGCInterval )
        return 0;

    mnLastGC = mnLruIndex;

    // TODO: improve garbage collection scalability
    ULONG nBytesCollected = 0;
    for( FontList::iterator it = aFontList.begin(); it != aFontList.end(); )
    {
        ServerFont& rServerFont = const_cast<ServerFont&>( *(it->second) );
        if( rServerFont.GetRefCount() > 0)
        {
            nBytesCollected += rServerFont.GarbageCollect( mnLruIndex );
            ++it;
        }
        else
        {
            // this is a good time to remove unreferenced fonts
            nBytesCollected += rServerFont.GetByteCount();
            delete &rServerFont;
            aFontList.erase( it++ );
        }
    }

    return nBytesCollected;
}

// -----------------------------------------------------------------------

const GlyphMetric& GlyphCache::GetGlyphMetric( const ServerFont& rServerFont, sal_Unicode aChar ) const
{
    const int nGlyphIndex = rServerFont.GetGlyphIndex( aChar );
    const GlyphData* pGD = rServerFont.FindGlyphData( nGlyphIndex );

    if( !pGD )  // GlyphData not found => create new one
    {
        if( mnBytesUsed > mnMaxSize )
            mnBytesUsed -= const_cast<GlyphCache*>(this) -> GarbageCollect();

        ULONG nOldSize = rServerFont.GetByteCount();
        pGD = rServerFont.CacheGlyphData( nGlyphIndex, false );
        mnBytesUsed += rServerFont.GetByteCount() - nOldSize;
    }

    pGD->SetLruValue( ++mnLruIndex );
    return pGD->GetMetric();
}

// -----------------------------------------------------------------------

const Bitmap& GlyphCache::GetGlyphBitmap( const ServerFont& rServerFont, sal_Unicode aChar ) const
{
    const int nGlyphIndex = rServerFont.GetGlyphIndex( aChar );
    const GlyphData* pGD = rServerFont.FindGlyphData( nGlyphIndex );

    if( !pGD || !pGD->GetBitmap() ) // GlyphData not found => create new one
    {
        if( mnBytesUsed > mnMaxSize )
            mnBytesUsed -= const_cast<GlyphCache*>(this) -> GarbageCollect();

        ULONG nOldSize = rServerFont.GetByteCount();
        pGD = rServerFont.CacheGlyphData( nGlyphIndex, true );
        mnBytesUsed += rServerFont.GetByteCount() - nOldSize;
    }

    pGD->SetLruValue( ++mnLruIndex );
    return *(pGD->GetBitmap());
}

// -----------------------------------------------------------------------

bool GlyphCache::GetGlyphOutline( const ServerFont& rServerFont, sal_Unicode aChar, bool bOptimize, PolyPolygon& rPolyPoly ) const
{
    const int nGlyphIndex = rServerFont.GetGlyphIndex( aChar );
    return rServerFont.GetGlyphOutline( nGlyphIndex, bOptimize, rPolyPoly );
}

// =======================================================================
// ServerFont
// =======================================================================

ServerFont::ServerFont( const ImplFontSelectData& rFSD )
:   maFontSelData(rFSD),    mnRefCount(1), mnBytesUsed( sizeof(ServerFont) )
{}

// -----------------------------------------------------------------------

ServerFont::~ServerFont()
{}

// -----------------------------------------------------------------------

long ServerFont::Release() const
{
    DBG_ASSERT( mnRefCount > 0, "ServerFont: RefCount underflow" );
    return --mnRefCount;
}

// -----------------------------------------------------------------------

const GlyphData* ServerFont::FindGlyphData( int nGlyphIndex ) const
{
    GlyphList::const_iterator it = aGlyphList.find( nGlyphIndex );
    if( it == aGlyphList.end() )
        return NULL;
    return &(it->second);
}

// -----------------------------------------------------------------------

const GlyphData* ServerFont::CacheGlyphData( int nGlyphIndex, bool bWithBitmap ) const
{
    GlyphData& pGD = const_cast<GlyphList&>(aGlyphList)[ nGlyphIndex ];
    SetGlyphData( nGlyphIndex, bWithBitmap, pGD );
    mnBytesUsed += pGD.GetByteCount();
    return &pGD;
}

// -----------------------------------------------------------------------

ULONG ServerFont::GarbageCollect( long nLruIndex )
{
    ULONG nOldByteCount = mnBytesUsed;

    for( GlyphList::iterator it = aGlyphList.begin(); it != aGlyphList.end(); )
    {
        GlyphData& pGD = it->second;
        // TODO: improve cache hit ratio
        if( (nLruIndex - pGD.GetLruValue()) > 200 ) // TODO: change constant
        {
            mnBytesUsed -= pGD.GetByteCount();
            pGD.DeleteBitmap();
            aGlyphList.erase( it++ );
        }
            else ++it;
    }

    return (nOldByteCount - mnBytesUsed);
}

// =======================================================================
// GlyphData
// =======================================================================

GlyphData::GlyphData()
:   mpBitmap(0)
{}

// -----------------------------------------------------------------------

GlyphData::~GlyphData()
{
    DeleteBitmap();
}

// -----------------------------------------------------------------------

void GlyphData::DeleteBitmap()
{
    if( mpBitmap)
    {
        delete mpBitmap;
        mpBitmap = 0;
    }
}

// -----------------------------------------------------------------------

ULONG GlyphData::GetByteCount() const
{
    ULONG nItemSize = sizeof(GlyphData);
    if( mpBitmap)
        nItemSize += mpBitmap->GetSizeBytes();
    return nItemSize;
}

// =======================================================================
