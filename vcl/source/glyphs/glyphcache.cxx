/*************************************************************************
 *
 *  $RCSfile: glyphcache.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:58:13 $
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

GlyphCache* GlyphCache::mpSingleton = 0;

// -----------------------------------------------------------------------

GlyphCache::GlyphCache( ULONG nMaxSize )
:   mnMaxSize(nMaxSize),
    mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0),
    mnGlyphCount(0),
    mpFtManager(NULL),
    mpCurrentGCFont(NULL)
{
    mpFtManager = new FreetypeManager;
}

// -----------------------------------------------------------------------

GlyphCache::~GlyphCache()
{
// TODO:
//  for( FontList::iterator it = maFontList.begin(); it != maFontList.end(); ++it )
//      delete const_cast<ServerFont*>( it->second );
    if( mpFtManager )
        delete mpFtManager;
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
    &&  (rA.mbVertical      == rB.mbVertical)
    &&  (rA.mbNonAntialiased== rB.mbNonAntialiased) )
        return true;
    return false;
}

// -----------------------------------------------------------------------

void GlyphCache::EnsureInstance( GlyphCachePeer& rPeer, bool bInitFonts )
{
    if( mpSingleton )
        return;

    static GlyphCache aGlyphCache( 1500000 );
    aGlyphCache.mpPeer = &rPeer;

    if( bInitFonts )
    {
        if( const char* pFontPath = ::getenv( "SAL_FONTPATH_PRIVATE" ) )
            aGlyphCache.AddFontPath( String::CreateFromAscii( pFontPath ) );
        const String& rFontPath = Application::GetFontPath();
        if( rFontPath.Len() > 0 )
            aGlyphCache.AddFontPath( rFontPath );
    }

    mpSingleton = &aGlyphCache;
}

// -----------------------------------------------------------------------

// this gets called when the upper layers want to remove the related ImplFontData
void GlyphCache::RemoveFont( const ImplFontData* pFontData )
{
    FontList::iterator it = maFontList.begin();
    while( it != maFontList.end() )
    {
        if( pFontData != it->first.mpFontData )
            ++it;
        else
        {
            const ServerFont* pSF = it->second;
            if( pSF )
                mnBytesUsed -= pSF->GetByteCount();
            maFontList.erase( it++ );
        }
    }
}

// -----------------------------------------------------------------------

void GlyphCache::ClearFontPath()
{
    if( mpFtManager )
        mpFtManager->ClearFontList();
}

// -----------------------------------------------------------------------

void GlyphCache::AddFontPath( const String& rFontPath )
{
    if( !mpFtManager )
        return;

    for( xub_StrLen nBreaker1 = 0, nBreaker2 = 0; nBreaker2 != STRING_LEN; nBreaker1 = nBreaker2 + 1 )
    {
        nBreaker2 = rFontPath.Search( ';', nBreaker1 );
        if( nBreaker2 == STRING_NOTFOUND )
            nBreaker2 = STRING_LEN;

        ::rtl::OUString aUrlName;
        osl::FileBase::getFileURLFromSystemPath( rFontPath.Copy( nBreaker1, nBreaker2 ), aUrlName );
        mpFtManager->AddFontDir( aUrlName );
    }
}

// -----------------------------------------------------------------------

void GlyphCache::AddFontFile( const rtl::OString& rNormalizedName, int nFaceNum,
    int nFontId, const ImplFontData* pFontData )
{
    if( mpFtManager )
        mpFtManager->AddFontFile( rNormalizedName, nFaceNum, nFontId, pFontData );
}

// -----------------------------------------------------------------------

long GlyphCache::FetchFontList( ImplDevFontList* pList ) const
{
    long nCount = 0;
    if( mpFtManager )
        nCount += mpFtManager->FetchFontList( pList );
    // nCount += VirtDevServerFont::FetchFontList( pList );
    return nCount;
}

// -----------------------------------------------------------------------

ServerFont* GlyphCache::CacheFont( const ImplFontSelectData& rFontSelData )
{
    if( rFontSelData.mpFontData == NULL )
        return NULL;

    FontList::iterator it = maFontList.find( rFontSelData );
    if( it != maFontList.end() )
    {
        ServerFont* pFound = it->second;
        if( pFound )
            pFound->AddRef();
        return pFound;
    }

    // font not cached yet => create new font item
    ServerFont* pNew = NULL;
    // TODO: pNew = VirtDevServerFont::CreateFont( rFontSelData );
    if( !pNew && mpFtManager )
        pNew = mpFtManager->CreateFont( rFontSelData );

    maFontList[ rFontSelData ] = pNew;
    if( pNew )
    {
        mnBytesUsed += pNew->GetByteCount();

        // schedule it for garbage collection some time later
        if( !mpCurrentGCFont )
        {
            mpCurrentGCFont = pNew;
            pNew->mpNextGCFont = pNew;
            pNew->mpPrevGCFont = pNew;
        }
        else
        {
            pNew->mpNextGCFont = mpCurrentGCFont;
            pNew->mpPrevGCFont = mpCurrentGCFont->mpPrevGCFont;
            pNew->mpPrevGCFont->mpNextGCFont = pNew;
            mpCurrentGCFont->mpPrevGCFont = pNew;
        }
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
#if 1 // TODO: decide on method depending on performance impact
        {}
#else
        pFont->GarbageCollect( mnLruIndex - mnGlyphCount/2 );
#endif
}

// -----------------------------------------------------------------------

ULONG GlyphCache::CalcByteCount() const
{
    ULONG nCacheSize = sizeof(*this);
    for( FontList::const_iterator it = maFontList.begin(); it != maFontList.end(); ++it )
    {
        const ServerFont* pSF = it->second;
        if( pSF )
            nCacheSize += pSF->GetByteCount();
    }
    // TODO: also account something for hashtable management
    return nCacheSize;
}

// -----------------------------------------------------------------------

void GlyphCache::GarbageCollect()
{
    // prepare advance to next font for garbage collection
    ServerFont* const pServerFont = mpCurrentGCFont;
    mpCurrentGCFont = mpCurrentGCFont->mpNextGCFont;

    if( !pServerFont )
        return;

    if( (pServerFont == mpCurrentGCFont)    // no other fonts
    ||  (pServerFont->GetRefCount() > 0) )  // font still used
    {
        // try to save at least a few bytes
        pServerFont->GarbageCollect( mnLruIndex - mnGlyphCount/2 );

        // when there is a lot of memory pressure also tighten maFontList
        if( maFontList.size() >= 200 )
        {
            FontList::iterator it = maFontList.begin();
            while( it != maFontList.end() )
            {
                if( !it->second )
                    maFontList.erase( it++ );
                else
                    ++it;
            }
        }
    }
    else
    {
        DBG_ASSERT( (pServerFont->GetRefCount() == 0),
            "GlyphCache::GC detected RefCount underflow" );

        // now its time to remove the unreferenced font
        ServerFont* const pPrev = pServerFont->mpPrevGCFont;
        ServerFont* const pNext = pServerFont->mpNextGCFont;
        pPrev->mpNextGCFont = pNext;
        pNext->mpPrevGCFont = pPrev;

        pServerFont->GarbageCollect( mnLruIndex+0x10000000 );
        mpPeer->RemovingFont( *pServerFont );
        mnBytesUsed -= pServerFont->GetByteCount();
        maFontList.erase( pServerFont->GetFontSelData() );
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
    ++mnGlyphCount;
}

// -----------------------------------------------------------------------

void GlyphCache::GrowNotify()
{
    if( (mnBytesUsed + mpPeer->GetByteCount()) > mnMaxSize )
        GarbageCollect();
}

// -----------------------------------------------------------------------

inline void GlyphCache::RemovingGlyph( ServerFont& rSF, GlyphData& rGD, int nGlyphIndex )
{
    mpPeer->RemovingGlyph( rSF, rGD, nGlyphIndex );
    mnBytesUsed -= sizeof( GlyphData );
    --mnGlyphCount;
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
    GlyphList::iterator it = maGlyphList.find( nGlyphIndex );
    if( it != maGlyphList.end() ) {
        GlyphData& rGlyphData = it->second;
        GlyphCache::GetInstance().UsingGlyph( *this, rGlyphData );
        return rGlyphData;
    }

    // sometimes not => we need to create and initialize it ourselves
    GlyphData& rGlyphData = maGlyphList[ nGlyphIndex ];
    mnBytesUsed += sizeof( GlyphData );
    InitGlyphData( nGlyphIndex, rGlyphData );
    GlyphCache::GetInstance().AddedGlyph( *this, rGlyphData );
    return rGlyphData;
}

// -----------------------------------------------------------------------

void ServerFont::GarbageCollect( long nMinLruIndex )
{
    GlyphList::iterator it = maGlyphList.begin();
    while( it != maGlyphList.end() )
    {
        GlyphData& rGD = it->second;
        if( (nMinLruIndex - rGD.GetLruValue()) >= 0 )
        {
            mnBytesUsed -= sizeof( GlyphData );
            GlyphCache::GetInstance().RemovingGlyph( *this, rGD, it->first );
            maGlyphList.erase( it++ );
        }
        else
            ++it;
    }

    if( mnBytesUsed < 0 )
        mnBytesUsed = 0;    // shouldn't happen
}

// -----------------------------------------------------------------------

Point ServerFont::TransformPoint( const Point& rPoint ) const
{
    if( nCos == 0x10000 )
        return rPoint;
    // TODO: use 32x32=>64bit intermediate
    const double dCos = nCos * (1.0 / 0x10000);
    const double dSin = nSin * (1.0 / 0x10000);
    long nX = (long)(rPoint.X() * dCos + rPoint.Y() * dSin);
    long nY = (long)(rPoint.Y() * dCos - rPoint.X() * dSin);
    return Point( nX, nY );
}

// =======================================================================
