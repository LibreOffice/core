/*************************************************************************
 *
 *  $RCSfile: glyphcache.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: cp $ $Date: 2001-04-06 08:18:23 $
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <salbtype.hxx>
#include <gcach_vdev.hxx>
#include <gcach_ftyp.hxx>

#include <svapp.hxx>
#include <bitmap.hxx>
#include <outfont.hxx>

#include <rtl/ustring>      // used only for string=>hashvalue
#include <osl/file.hxx>
#include <tools/debug.hxx>


// =======================================================================
// GlyphCache
// =======================================================================

GlyphCache* GlyphCache::pSingleton = 0;

// -----------------------------------------------------------------------

GlyphCache::GlyphCache( ULONG _nMaxSize )
:   mnMaxSize(_nMaxSize),
    mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0),
    pFtManager(NULL),
    pCurrentGCFont(NULL)
{
    pFtManager = new FreetypeManager;
}

// -----------------------------------------------------------------------

GlyphCache::~GlyphCache()
{
// TODO:
//  for( FontList::iterator it = aFontList.begin(); it != aFontList.end(); ++it )
//      delete const_cast<ServerFont*>( it->second );
    if( pFtManager )
        delete pFtManager;
}

// -----------------------------------------------------------------------

inline size_t std::hash<ImplFontSelectData>::operator()( const ImplFontSelectData& rFontSelData ) const
{
    // TODO: does it pay much to improve this hash function?
    size_t nHash = size_t( rFontSelData.mpFontData->mpSysData );
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    nHash   += rFontSelData.mbVertical;
    return nHash;

}

// -----------------------------------------------------------------------

bool operator==( const ImplFontSelectData& rA, const ImplFontSelectData& rB )
{
    if( (rA.mpFontData != NULL) && (rB.mpFontData != NULL)
    &&  (rA.mpFontData->mpSysData == rB.mpFontData->mpSysData )
    &&  ((rA.mnWidth==rB.mnWidth) || (!rA.mnWidth && (rA.mnHeight==rB.mnWidth)))
    &&  (rA.mnHeight        == rB.mnHeight)
    &&  (rA.mnOrientation   == rB.mnOrientation)
    &&  (rA.mbVertical      == rB.mbVertical) )
        return true;
    return false;
}

// -----------------------------------------------------------------------

void GlyphCache::EnsureInstance( GlyphCachePeer& rPeer, bool bInitFonts )
{
    if( pSingleton )
        return;

    static GlyphCache aGlyphCache( 750000 );
    aGlyphCache.pPeer = &rPeer;

    if( bInitFonts )
    {
        if( const char* pFontPath = ::getenv( "SAL_FONTPATH_PRIVATE" ) )
            aGlyphCache.AddFontPath( String::CreateFromAscii( pFontPath ) );
        const String& rFontPath = Application::GetFontPath();
        if( rFontPath.Len() > 0 )
            aGlyphCache.AddFontPath( rFontPath );
    }

    pSingleton = &aGlyphCache;
}

// -----------------------------------------------------------------------

void GlyphCache::ClearFontPath()
{
    if( pFtManager )
        pFtManager->ClearFontList();
}

// -----------------------------------------------------------------------

void GlyphCache::AddFontPath( const String& rFontPath )
{
    if( !pFtManager )
        return;

    for( xub_StrLen nBreaker1 = 0, nBreaker2 = 0; nBreaker2 != STRING_LEN; nBreaker1 = nBreaker2 + 1 )
    {
        nBreaker2 = rFontPath.Search( ';', nBreaker1 );
        if( nBreaker2 == STRING_NOTFOUND )
            nBreaker2 = STRING_LEN;

        ::rtl::OUString aNormalizedName;
        osl::FileBase::normalizePath( rFontPath.Copy( nBreaker1, nBreaker2 ), aNormalizedName );
        pFtManager->AddFontDir( aNormalizedName );
    }
}

// -----------------------------------------------------------------------

void GlyphCache::AddFontFile( const String& rNormalizedName, int nFaceNum,
    int nFontId, const ImplFontData* pFontData )
{
    if( pFtManager )
        pFtManager->AddFontFile( rNormalizedName, nFaceNum, nFontId, pFontData );
}

// -----------------------------------------------------------------------

long GlyphCache::FetchFontList( ImplDevFontList* pList ) const
{
    long nCount = 0;
    if( pFtManager )
        nCount += pFtManager->FetchFontList( pList );
    //  nCount += VirtDevServerFont::FetchFontList( pList );
    return nCount;
}

// -----------------------------------------------------------------------

void*
GlyphCache::GetFontHandle (int nFontId)
{
    if ( pFtManager != NULL )
        return pFtManager->GetFontHandle (nFontId);
    return NULL;
}

// -----------------------------------------------------------------------

ServerFont* GlyphCache::CacheFont( const ImplFontSelectData& rFontSelData )
{
    if( rFontSelData.mpFontData == NULL )
        return NULL;

    FontList::iterator it = aFontList.find( rFontSelData );
    if( it != aFontList.end() )
    {
        ServerFont* pFound = it->second;
        pFound->AddRef();
        return pFound;
    }

    // font not cached yet => create new font item
    ServerFont* pNew = NULL;
    // TODO: pNew = VirtDevServerFont::CreateFont( rFontSelData );
    if( !pNew && pFtManager)
        pNew = pFtManager->CreateFont( rFontSelData );

    if( pNew )
    {
        aFontList[ rFontSelData ] = pNew;
        mnBytesUsed += pNew->GetByteCount();

        // schedule it for garbage collection some time later
        if( !pCurrentGCFont )
        {
            pCurrentGCFont = pNew;
            pNew->pPrevGCFont = pNew;
        }
        pNew->pNextGCFont = pCurrentGCFont;
        pNew->pPrevGCFont = pCurrentGCFont->pPrevGCFont;
        pCurrentGCFont->pPrevGCFont->pNextGCFont = pNew;
        pCurrentGCFont->pPrevGCFont = pNew;
    }

    return pNew;
}

// -----------------------------------------------------------------------

void GlyphCache::UncacheFont( ServerFont& rServerFont )
{
    // the interface for rServerFont must be const because a
    // user who wants to release it only got const ServerFonts.
    // The caching algorithm needs a non-const object
    ServerFont* pFont = const_cast<ServerFont*>( &rServerFont );
    if( pFont->Release() <= 0 )
        // lazy release
        pFont->GarbageCollect( mnLruIndex );
}

// -----------------------------------------------------------------------

ULONG GlyphCache::CalcByteCount() const
{
    ULONG nCacheSize = sizeof(*this);
    for( FontList::const_iterator it = aFontList.begin(); it != aFontList.end(); ++it )
        nCacheSize += it->second->GetByteCount();
    // TODO: also account something for hashtable management
    return nCacheSize;
}

// -----------------------------------------------------------------------

void GlyphCache::GarbageCollect()
{
    // prepare advance to next font for garbage collection
    ServerFont* const pServerFont = pCurrentGCFont;
    pCurrentGCFont = pCurrentGCFont->pNextGCFont;

    ULONG nBytesCollected;
    if( pServerFont->GetRefCount() > 0 )
    {
        // try to save at least a few bytes
        pServerFont->GarbageCollect( mnLruIndex );
    }
    else
    {
        DBG_ASSERT( (pServerFont->GetRefCount() == 0), "GlyphCache::GC detected RefCount underflow" );

        // now its time to remove the unreferenced font
        ServerFont* const pPrev = pServerFont->pPrevGCFont;
        ServerFont* const pNext = pServerFont->pNextGCFont;
        pPrev->pNextGCFont = pNext;
        pNext->pPrevGCFont = pPrev;

        if( pCurrentGCFont == pServerFont ) // no fonts left
            pCurrentGCFont = NULL;

        pServerFont->GarbageCollect( ~mnLruIndex );
        pPeer->RemovingFont( *pServerFont );
        mnBytesUsed -= pServerFont->GetByteCount();
        aFontList.erase( pServerFont->GetFontSelData() );
        delete pServerFont;
    }
}

// -----------------------------------------------------------------------

inline void GlyphCache::UsingGlyph( ServerFont&, GlyphData& rGlyphData )
{
    rGlyphData.SetLruValue( ++mnLruIndex );
}

// -----------------------------------------------------------------------

inline void GlyphCache::AddedGlyph( ServerFont& rServerFont, GlyphData& rGlyphData )
{
    UsingGlyph( rServerFont, rGlyphData );
    mnBytesUsed += sizeof( rGlyphData );
    GrowNotify();
}

// -----------------------------------------------------------------------

void GlyphCache::GrowNotify()
{
    if( (mnBytesUsed + pPeer->GetByteCount()) > mnMaxSize )
        GarbageCollect();
}

// -----------------------------------------------------------------------

inline void GlyphCache::RemovingGlyph( ServerFont& rSF, GlyphData& rGD, int nGlyphIndex )
{
    pPeer->RemovingGlyph( rSF, rGD, nGlyphIndex );
    mnBytesUsed -= sizeof( GlyphData );
}

// =======================================================================
// ServerFont
// =======================================================================

ServerFont::ServerFont( const ImplFontSelectData& rFSD )
:   maFontSelData(rFSD),
    mnExtInfo(0),
    mnRefCount(1),
    mnBytesUsed( sizeof(ServerFont) ),
    nCos( 0x10000),
    nSin( 0)
{
    if( rFSD.mnOrientation != 0 )
    {
        const double dRad = rFSD.mnOrientation * ( F_2PI / 3600.0 );
        nCos = (long)( 0x10000 * cos( dRad ) + 0.5 );
        nSin = (long)( 0x10000 * sin( dRad ) + 0.5 );
    }
}

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

GlyphData& ServerFont::GetGlyphData( int nGlyphIndex )
{
    // usually it is cached
    GlyphList::iterator it = aGlyphList.find( nGlyphIndex );
    if( it != aGlyphList.end() ) {
        GlyphData& rGlyphData = it->second;
        GlyphCache::GetInstance().UsingGlyph( *this, rGlyphData );
        return rGlyphData;
    }

    // sometimes not => we need to create and initialize it ourselves
    GlyphData& rGlyphData = aGlyphList[ nGlyphIndex ];
    InitGlyphData( nGlyphIndex, rGlyphData );
    GlyphCache::GetInstance().AddedGlyph( *this, rGlyphData );
    return rGlyphData;
}

// -----------------------------------------------------------------------

void ServerFont::GarbageCollect( long nLruIndex )
{
    for( GlyphList::iterator it = aGlyphList.begin(); it != aGlyphList.end(); )
    {
        GlyphData& rGD = it->second;
        if( (ULONG)(nLruIndex - rGD.GetLruValue()) > 200 )  // TODO: change constant
        {
            mnBytesUsed -= sizeof( GlyphData );
            GlyphCache::GetInstance().RemovingGlyph( *this, rGD, it->first );
            aGlyphList.erase( it++ );
        }
        else
            ++it;
    }
}

// -----------------------------------------------------------------------

Point ServerFont::TransformPoint( const Point& rPoint ) const
{
    if( !nSin)
        return rPoint;
    // TODO: use 32x32=>64bit intermediate
    const double dCos = nCos * (1.0 / 0x10000);
    const double dSin = nSin * (1.0 / 0x10000);
    long nX = (long)(rPoint.X() * dCos + rPoint.Y() * dSin);
    long nY = (long)(rPoint.Y() * dCos - rPoint.X() * dSin);
    return Point( nX, nY );
}

// =======================================================================
