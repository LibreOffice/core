/*************************************************************************
 *
 *  $RCSfile: glyphcache.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:31:08 $
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

GlyphCache::GlyphCache( ULONG nMaxSize )
:   mnMaxSize(nMaxSize),
    mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0),
    mnGlyphCount(0),
    mpCurrentGCFont(NULL),
    mpFtManager(NULL)
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

#ifndef IRIX
inline
#endif
size_t GlyphCache::IFSD_Hash::operator()( const ImplFontSelectData& rFontSelData ) const
{
    // TODO: does it pay much to improve this hash function?
    if( !rFontSelData.mpFontData )
        return 0;
    size_t nHash = rFontSelData.mpFontData->GetFontId() << 8;
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    nHash   += rFontSelData.mbVertical;
    return nHash;
}

// -----------------------------------------------------------------------

bool GlyphCache::IFSD_Equal::operator()( const ImplFontSelectData& rA, const ImplFontSelectData& rB) const
{
    if( (rA.mpFontData == NULL)
    ||  (rB.mpFontData == NULL)
    ||  (rA.mpFontData->GetFontId() != rB.mpFontData->GetFontId()) )
        return false;

    if( (rA.mnHeight         != rB.mnHeight)
    ||  (rA.mnOrientation    != rB.mnOrientation)
    ||  (rA.mbVertical       != rB.mbVertical)
    ||  (rA.mbNonAntialiased != rB.mbNonAntialiased) )
        return false;

    if( (rA.mnWidth != rB.mnWidth)
    && ((rA.mnHeight != rB.mnWidth) || (rA.mnWidth != 0)) )
        return false;

    return true;
}

// -----------------------------------------------------------------------

GlyphCache& GlyphCache::GetInstance()
{
    static GlyphCache aGlyphCache( 1500000 );
    return aGlyphCache;
}

// -----------------------------------------------------------------------

void GlyphCache::EnsureInstance( GlyphCachePeer& rPeer, bool bInitFonts )
{
    GlyphCache& rGlyphCache = GetInstance();
    if( rGlyphCache.mpPeer == &rPeer )
        return;

    rGlyphCache.mpPeer = &rPeer;

    if( bInitFonts )
    {
        if( const char* pFontPath = ::getenv( "SAL_FONTPATH_PRIVATE" ) )
            rGlyphCache.AddFontPath( String::CreateFromAscii( pFontPath ) );
        const String& rFontPath = Application::GetFontPath();
        if( rFontPath.Len() > 0 )
            rGlyphCache.AddFontPath( rFontPath );
    }
}

// -----------------------------------------------------------------------

// this gets called when the upper layers want to remove the related ImplFontData
void GlyphCache::RemoveFont( int nFontId )
{
    FontList::iterator it_next = maFontList.begin();
    while( it_next != maFontList.end() )
    {
        FontList::iterator it = it_next++;
        ImplFontData* pFontFace = it->first.mpFontData;
        if( nFontId != pFontFace->GetFontId() )
            continue;

        // found matching pFontData => remove entry if not referenced
        ServerFont* pSF = it->second;
        if( pSF && (pSF->GetRefCount() <= 0) )
        {
            if( pSF == mpCurrentGCFont )
                mpCurrentGCFont = NULL;
            delete pSF;
        }
        maFontList.erase( it );
    }

    // when current GC font has been destroyed get another one
    if( !mpCurrentGCFont )
    {
        it_next = maFontList.begin();
        if( it_next != maFontList.end() )
            mpCurrentGCFont = it_next->second;
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
    int nFontId, const ImplDevFontAttributes& rDFA, const ExtraKernInfo* pExtraKern )
{
    if( mpFtManager )
        mpFtManager->AddFontFile( rNormalizedName, nFaceNum, nFontId, rDFA, pExtraKern );
}

// -----------------------------------------------------------------------

void GlyphCache::AnnounceFonts( ImplDevFontList* pList ) const
{
    if( mpFtManager )
        mpFtManager->AnnounceFonts( pList );
    // VirtDevServerFont::AnnounceFonts( pList );
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
    if( mpFtManager )
        pNew = mpFtManager->CreateFont( rFontSelData );
    // TODO: pNew = VirtDevServerFont::CreateFont( rFontSelData );

    maFontList[ rFontSelData ] = pNew;
    if( pNew )
    {
        mnBytesUsed += pNew->GetByteCount();

        // enable garbage collection for new font
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
    if( (pFont->Release() <= 0)
    &&  (mnMaxSize <= (mnBytesUsed + mpPeer->GetByteCount())) )
    {
        mpCurrentGCFont = pFont;
        GarbageCollect();
    }
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
    if( !mpCurrentGCFont )
        return;

    // prepare advance to next font for garbage collection
    ServerFont* const pServerFont = mpCurrentGCFont;
    mpCurrentGCFont = pServerFont->mpNextGCFont;

    if( (pServerFont == mpCurrentGCFont)    // no other fonts
    ||  (pServerFont->GetRefCount() > 0) )  // font still used
    {
        // try to save at least a few bytes
        pServerFont->GarbageCollect( mnLruIndex - mnGlyphCount/2 );

        // when there is a lot of memory pressure also tighten maFontList
        if( maFontList.size() >= 100 )
        {
            // remove unreferenced fonts
            FontList::iterator it_next = maFontList.begin();
            while( it_next != maFontList.end() )
            {
                FontList::iterator it = it_next++;
                ServerFont* pSF = it->second;
                if( (pSF != NULL)
                &&  (pSF->GetRefCount() <= 0)
                &&  (pSF != mpCurrentGCFont) )
                {
                    maFontList.erase( it );
                    delete pSF;
                }
            }
        }
    }
    else // current GC font is unreferenced
    {
        DBG_ASSERT( (pServerFont->GetRefCount() == 0),
            "GlyphCache::GC detected RefCount underflow" );

        pServerFont->GarbageCollect( mnLruIndex+0x10000000 );
        const ImplFontSelectData& rIFSD = pServerFont->GetFontSelData();
        maFontList.erase( rIFSD );
        mpPeer->RemovingFont( *pServerFont );
        mnBytesUsed -= pServerFont->GetByteCount();
        delete pServerFont;
    }
}

// -----------------------------------------------------------------------

inline void GlyphCache::UsingGlyph( ServerFont&, GlyphData& rGlyphData )
{
    rGlyphData.SetLruValue( mnLruIndex++ );
}

// -----------------------------------------------------------------------

inline void GlyphCache::AddedGlyph( ServerFont& rServerFont, GlyphData& rGlyphData )
{
    ++mnGlyphCount;
    mnBytesUsed += sizeof( rGlyphData );
    UsingGlyph( rServerFont, rGlyphData );
    GrowNotify();
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
    mpPrevGCFont( NULL ),
    mpNextGCFont( NULL ),
    mnCos( 0x10000),
    mnSin( 0)
{
    // TODO: move update of mpFontEntry into FontEntry class when
    // it becomes reponsible for the ServerFont instantiation
    ((ImplServerFontEntry*)rFSD.mpFontEntry)->SetServerFont( this );

    if( rFSD.mnOrientation != 0 )
    {
        const double dRad = rFSD.mnOrientation * ( F_2PI / 3600.0 );
        mnCos = static_cast<long>( 0x10000 * cos( dRad ) + 0.5 );
        mnSin = static_cast<long>( 0x10000 * sin( dRad ) + 0.5 );
    }
}

// -----------------------------------------------------------------------

ServerFont::~ServerFont()
{
   // remove from GC list
    ServerFont* pPrev = mpPrevGCFont;
    ServerFont* pNext = mpNextGCFont;
    if( pPrev ) pPrev->mpNextGCFont = pNext;
    if( pNext ) pNext->mpPrevGCFont = pPrev;
}

// -----------------------------------------------------------------------

long ServerFont::Release() const
{
    DBG_ASSERT( mnRefCount > 0, "ServerFont: RefCount underflow" );
    return --mnRefCount;
}

// -----------------------------------------------------------------------

GlyphData& ServerFont::GetGlyphData( int nGlyphIndex )
{
    // usually the GlyphData is cached
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
    GlyphList::iterator it_next = maGlyphList.begin(), it;
    while( it_next != maGlyphList.end() )
    {
        it = it_next++;
        GlyphData& rGD = it->second;
        if( (nMinLruIndex - rGD.GetLruValue()) > 0 )
        {
            mnBytesUsed -= sizeof( GlyphData );
            GlyphCache::GetInstance().RemovingGlyph( *this, rGD, it->first );
            maGlyphList.erase( it );
        }
    }

    if( mnBytesUsed < 0 )
        mnBytesUsed = 0;    // shouldn't happen
}

// -----------------------------------------------------------------------

Point ServerFont::TransformPoint( const Point& rPoint ) const
{
    if( mnCos == 0x10000 )
        return rPoint;
    // TODO: use 32x32=>64bit intermediate
    const double dCos = mnCos * (1.0 / 0x10000);
    const double dSin = mnSin * (1.0 / 0x10000);
    long nX = (long)(rPoint.X() * dCos + rPoint.Y() * dSin);
    long nY = (long)(rPoint.Y() * dCos - rPoint.X() * dSin);
    return Point( nX, nY );
}

// =======================================================================

ImplServerFontEntry::ImplServerFontEntry( ImplFontSelectData& rFSD )
:   ImplFontEntry( rFSD ),
    mpServerFont( NULL )
{}

// -----------------------------------------------------------------------

ImplServerFontEntry::~ImplServerFontEntry()
{
    // TODO: remove the ServerFont here instead of in the GlyphCache
}

// =======================================================================

ExtraKernInfo::ExtraKernInfo( int nFontId )
:   mnFontId( nFontId ),
    mbInitialized( false )
{}

//--------------------------------------------------------------------------

bool ExtraKernInfo::HasKernPairs() const
{
    if( !mbInitialized )
        Initialize();
    return !maUnicodeKernPairs.empty();
}

//--------------------------------------------------------------------------

int ExtraKernInfo::GetUnscaledKernPairs( ImplKernPairData** ppKernPairs ) const
{
    if( !mbInitialized )
        Initialize();

    // return early if no kerning available
    if( maUnicodeKernPairs.empty() )
        return 0;

    // allocate kern pair table
    int nKernCount = maUnicodeKernPairs.size();
    *ppKernPairs = new ImplKernPairData[ nKernCount ];

    // fill in unicode kern pairs with the kern value scaled to the font width
    ImplKernPairData* pKernData = *ppKernPairs;
    UnicodeKernPairs::const_iterator it =  it = maUnicodeKernPairs.begin();
    for(; it != maUnicodeKernPairs.end(); ++it )
        *(pKernData++) = *it;

    return nKernCount;
}

//--------------------------------------------------------------------------

int ExtraKernInfo::GetUnscaledKernValue( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    if( !mbInitialized )
        Initialize();

    if( maUnicodeKernPairs.empty() )
        return 0;

    ImplKernPairData aKernPair = { cLeft, cRight, 0 };
    UnicodeKernPairs::const_iterator it = maUnicodeKernPairs.find( aKernPair );
    if( it == maUnicodeKernPairs.end() )
        return 0;

    int nUnscaledValue = (*it).mnKern;
    return nUnscaledValue;
}

// =======================================================================
