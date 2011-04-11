/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svtools.hxx"

#include <salhelper/timer.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <tools/poly.hxx>
#include "grfcache.hxx"

#include <memory>

// -----------
// - Defines -
// -----------

#define RELEASE_TIMEOUT 10000
#define MAX_BMP_EXTENT  4096

// -----------
// - statics -
// -----------

static const char aHexData[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

// -------------
// - GraphicID -
// -------------

class GraphicID
{
private:

    sal_uInt32  mnID1;
    sal_uInt32  mnID2;
    sal_uInt32  mnID3;
    sal_uInt32  mnID4;

                GraphicID();

public:


                GraphicID( const GraphicObject& rObj );
                ~GraphicID() {}

    sal_Bool        operator==( const GraphicID& rID ) const
                {
                    return( rID.mnID1 == mnID1 && rID.mnID2 == mnID2 &&
                            rID.mnID3 == mnID3 && rID.mnID4 == mnID4 );
                }

    ByteString  GetIDString() const;
    sal_Bool        IsEmpty() const { return( 0 == mnID4 ); }
};

// -----------------------------------------------------------------------------

GraphicID::GraphicID( const GraphicObject& rObj )
{
    const Graphic& rGraphic = rObj.GetGraphic();

    mnID1 = ( (sal_uLong) rGraphic.GetType() ) << 28;

    switch( rGraphic.GetType() )
    {
        case( GRAPHIC_BITMAP ):
        {
            if( rGraphic.IsAnimated() )
            {
                const Animation aAnimation( rGraphic.GetAnimation() );

                mnID1 |= ( aAnimation.Count() & 0x0fffffff );
                mnID2 = aAnimation.GetDisplaySizePixel().Width();
                mnID3 = aAnimation.GetDisplaySizePixel().Height();
                mnID4 = rGraphic.GetChecksum();
            }
            else
            {
                const BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                mnID1 |= ( ( ( (sal_uLong) aBmpEx.GetTransparentType() << 8 ) | ( aBmpEx.IsAlpha() ? 1 : 0 ) ) & 0x0fffffff );
                mnID2 = aBmpEx.GetSizePixel().Width();
                mnID3 = aBmpEx.GetSizePixel().Height();
                mnID4 = rGraphic.GetChecksum();
            }
        }
        break;

        case( GRAPHIC_GDIMETAFILE ):
        {
            const GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );

            mnID1 |= ( aMtf.GetActionCount() & 0x0fffffff );
            mnID2 = aMtf.GetPrefSize().Width();
            mnID3 = aMtf.GetPrefSize().Height();
            mnID4 = rGraphic.GetChecksum();
        }
        break;

        default:
            mnID2 = mnID3 = mnID4 = 0;
        break;
    }
}

// -----------------------------------------------------------------------------

ByteString GraphicID::GetIDString() const
{
    ByteString  aHexStr;
    sal_Char*   pStr = aHexStr.AllocBuffer( 32 );
    sal_Int32   nShift;

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        *pStr++ = aHexData[ ( mnID1 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        *pStr++ = aHexData[ ( mnID2 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        *pStr++ = aHexData[ ( mnID3 >> (sal_uInt32) nShift ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        *pStr++ = aHexData[ ( mnID4 >> (sal_uInt32) nShift ) & 0xf ];

    return aHexStr;
}

// ---------------------
// - GraphicCacheEntry -
// ---------------------

class GraphicCacheEntry
{
private:

    List                maGraphicObjectList;
    GraphicID           maID;
    GfxLink             maGfxLink;
    BitmapEx*           mpBmpEx;
    GDIMetaFile*        mpMtf;
    Animation*          mpAnimation;
    sal_Bool                mbSwappedAll;

    sal_Bool                ImplInit( const GraphicObject& rObj );
    void                ImplFillSubstitute( Graphic& rSubstitute );

public:

                        GraphicCacheEntry( const GraphicObject& rObj );
                        ~GraphicCacheEntry();

    const GraphicID&    GetID() const { return maID; }

    void                AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute );
    sal_Bool                ReleaseGraphicObjectReference( const GraphicObject& rObj );
    sal_uLong               GetGraphicObjectReferenceCount() { return maGraphicObjectList.Count(); }
    sal_Bool                HasGraphicObjectReference( const GraphicObject& rObj );

    void                TryToSwapIn();
    void                GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    sal_Bool                FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                GraphicObjectWasSwappedIn( const GraphicObject& rObj );
};

// -----------------------------------------------------------------------------

GraphicCacheEntry::GraphicCacheEntry( const GraphicObject& rObj ) :
    maID            ( rObj ),
    mpBmpEx         ( NULL ),
    mpMtf           ( NULL ),
    mpAnimation     ( NULL ),
    mbSwappedAll    ( !ImplInit( rObj ) )
{
    maGraphicObjectList.Insert( (void*) &rObj, LIST_APPEND );
}

// -----------------------------------------------------------------------------

GraphicCacheEntry::~GraphicCacheEntry()
{
    DBG_ASSERT( !maGraphicObjectList.Count(), "GraphicCacheEntry::~GraphicCacheEntry(): Not all GraphicObjects are removed from this entry" );

    delete mpBmpEx;
    delete mpMtf;
    delete mpAnimation;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCacheEntry::ImplInit( const GraphicObject& rObj )
{
    sal_Bool bRet;

    if( !rObj.IsSwappedOut() )
    {
        const Graphic& rGraphic = rObj.GetGraphic();

        if( mpBmpEx )
            delete mpBmpEx, mpBmpEx = NULL;

        if( mpMtf )
            delete mpMtf, mpMtf = NULL;

        if( mpAnimation )
            delete mpAnimation, mpAnimation = NULL;

        switch( rGraphic.GetType() )
        {
            case( GRAPHIC_BITMAP ):
            {
                if( rGraphic.IsAnimated() )
                    mpAnimation = new Animation( rGraphic.GetAnimation() );
                else
                    mpBmpEx = new BitmapEx( rGraphic.GetBitmapEx() );
            }
            break;

            case( GRAPHIC_GDIMETAFILE ):
            {
                mpMtf = new GDIMetaFile( rGraphic.GetGDIMetaFile() );
            }
            break;

            default:
                DBG_ASSERT( GetID().IsEmpty(), "GraphicCacheEntry::ImplInit: Could not initialize graphic! (=>KA)" );
            break;
        }

        if( rGraphic.IsLink() )
            maGfxLink = ( (Graphic&) rGraphic ).GetLink();
        else
            maGfxLink = GfxLink();

        bRet = sal_True;
    }
    else
        bRet = sal_False;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::ImplFillSubstitute( Graphic& rSubstitute )
{
    // create substitute for graphic;
    const Size          aPrefSize( rSubstitute.GetPrefSize() );
    const MapMode       aPrefMapMode( rSubstitute.GetPrefMapMode() );
    const Link          aAnimationNotifyHdl( rSubstitute.GetAnimationNotifyHdl() );
    const String        aDocFileName( rSubstitute.GetDocFileName() );
    const sal_uLong         nDocFilePos = rSubstitute.GetDocFilePos();
    const GraphicType   eOldType = rSubstitute.GetType();
    const sal_Bool          bDefaultType = ( rSubstitute.GetType() == GRAPHIC_DEFAULT );

    if( rSubstitute.IsLink() && ( GFX_LINK_TYPE_NONE == maGfxLink.GetType() ) )
        maGfxLink = rSubstitute.GetLink();

    if( mpBmpEx )
        rSubstitute = *mpBmpEx;
    else if( mpAnimation )
        rSubstitute = *mpAnimation;
    else if( mpMtf )
        rSubstitute = *mpMtf;
    else
        rSubstitute.Clear();

    if( eOldType != GRAPHIC_NONE )
    {
        rSubstitute.SetPrefSize( aPrefSize );
        rSubstitute.SetPrefMapMode( aPrefMapMode );
        rSubstitute.SetAnimationNotifyHdl( aAnimationNotifyHdl );
        rSubstitute.SetDocFileName( aDocFileName, nDocFilePos );
    }

    if( GFX_LINK_TYPE_NONE != maGfxLink.GetType() )
        rSubstitute.SetLink( maGfxLink );

    if( bDefaultType )
        rSubstitute.SetDefaultType();
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );

    ImplFillSubstitute( rSubstitute );
    maGraphicObjectList.Insert( (void*) &rObj, LIST_APPEND );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCacheEntry::ReleaseGraphicObjectReference( const GraphicObject& rObj )
{
    sal_Bool bRet = sal_False;

    for( void* pObj = maGraphicObjectList.First(); !bRet && pObj; pObj = maGraphicObjectList.Next() )
    {
        if( &rObj == (GraphicObject*) pObj )
        {
            maGraphicObjectList.Remove( pObj );
            bRet = sal_True;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCacheEntry::HasGraphicObjectReference( const GraphicObject& rObj )
{
    sal_Bool bRet = sal_False;

    for( void* pObj = maGraphicObjectList.First(); !bRet && pObj; pObj = maGraphicObjectList.Next() )
        if( &rObj == (GraphicObject*) pObj )
            bRet = sal_True;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::TryToSwapIn()
{
    if( mbSwappedAll && maGraphicObjectList.Count() )
        ( (GraphicObject*) maGraphicObjectList.First() )->FireSwapInRequest();
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::GraphicObjectWasSwappedOut( const GraphicObject& /*rObj*/ )
{
    mbSwappedAll = sal_True;

    for( void* pObj = maGraphicObjectList.First(); mbSwappedAll && pObj; pObj = maGraphicObjectList.Next() )
        if( !( (GraphicObject*) pObj )->IsSwappedOut() )
            mbSwappedAll = sal_False;

    if( mbSwappedAll )
    {
        delete mpBmpEx, mpBmpEx = NULL;
        delete mpMtf, mpMtf = NULL;
        delete mpAnimation, mpAnimation = NULL;
    }
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCacheEntry::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    sal_Bool bRet;

    if( !mbSwappedAll && rObj.IsSwappedOut() )
    {
        ImplFillSubstitute( rSubstitute );
        bRet = sal_True;
    }
    else
        bRet = sal_False;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    if( mbSwappedAll )
        mbSwappedAll = !ImplInit( rObj );
}

// ----------------------------
// - GraphicDisplayCacheEntry -
// ----------------------------

class GraphicDisplayCacheEntry
{
private:

    ::salhelper::TTimeValue     maReleaseTime;
    const GraphicCacheEntry*    mpRefCacheEntry;
    GDIMetaFile*                mpMtf;
    BitmapEx*                   mpBmpEx;
    GraphicAttr                 maAttr;
    Size                        maOutSizePix;
    sal_uLong                       mnCacheSize;
    sal_uLong                       mnOutDevDrawMode;
    sal_uInt16                      mnOutDevBitCount;

public:

    static sal_uLong                GetNeededSize( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr );

public:

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const BitmapEx& rBmpEx ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( NULL ), mpBmpEx( new BitmapEx( rBmpEx ) ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ),
                                    mnOutDevDrawMode( pOut->GetDrawMode() ),
                                    mnOutDevBitCount( pOut->GetBitCount() )
                                    {
                                    }

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const GDIMetaFile& rMtf ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( new GDIMetaFile( rMtf ) ), mpBmpEx( NULL ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ),
                                    mnOutDevDrawMode( pOut->GetDrawMode() ),
                                    mnOutDevBitCount( pOut->GetBitCount() )
                                    {
                                    }


                                ~GraphicDisplayCacheEntry();

    const GraphicAttr&          GetAttr() const { return maAttr; }
    const Size&                 GetOutputSizePixel() const { return maOutSizePix; }
    sal_uLong                   GetCacheSize() const { return mnCacheSize; }
    const GraphicCacheEntry*    GetReferencedCacheEntry() const { return mpRefCacheEntry; }
    sal_uLong                   GetOutDevDrawMode() const { return mnOutDevDrawMode; }
    sal_uInt16              GetOutDevBitCount() const { return mnOutDevBitCount; }

    void                        SetReleaseTime( const ::salhelper::TTimeValue& rReleaseTime ) { maReleaseTime = rReleaseTime; }
    const ::salhelper::TTimeValue&    GetReleaseTime() const { return maReleaseTime; }

    sal_Bool                        Matches( OutputDevice* pOut, const Point& /*rPtPixel*/, const Size& rSzPixel,
                                         const GraphicCacheEntry* pCacheEntry, const GraphicAttr& rAttr ) const
                                {
                                    // #i46805# Additional match
                                    // criteria: outdev draw mode and
                                    // bit count. One cannot reuse
                                    // this cache object, if it's
                                    // e.g. generated for
                                    // DRAWMODE_GRAYBITMAP.
                                    return( ( pCacheEntry == mpRefCacheEntry ) &&
                                            ( maAttr == rAttr ) &&
                                            ( ( maOutSizePix == rSzPixel ) || ( !maOutSizePix.Width() && !maOutSizePix.Height() ) ) &&
                                            ( pOut->GetBitCount() == mnOutDevBitCount ) &&
                                            ( pOut->GetDrawMode() == mnOutDevDrawMode ) );
                                }

    void                        Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const;
};

// -----------------------------------------------------------------------------

sal_uLong GraphicDisplayCacheEntry::GetNeededSize( OutputDevice* pOut, const Point& /*rPt*/, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Graphic&      rGraphic = rObj.GetGraphic();
    const GraphicType   eType = rGraphic.GetType();
    sal_uLong               nNeededSize;

    if( GRAPHIC_BITMAP == eType )
    {
        const Size aOutSizePix( pOut->LogicToPixel( rSz ) );
        const long nBitCount = pOut->GetBitCount();

        if( ( aOutSizePix.Width() > MAX_BMP_EXTENT ) ||
            ( aOutSizePix.Height() > MAX_BMP_EXTENT ) )
        {
            nNeededSize = ULONG_MAX;
        }
        else if( nBitCount )
        {
            nNeededSize = aOutSizePix.Width() * aOutSizePix.Height() * nBitCount / 8;

            if( rObj.IsTransparent() || ( rAttr.GetRotation() % 3600 ) )
                nNeededSize += nNeededSize / nBitCount;
        }
        else
        {
             OSL_FAIL( "GraphicDisplayCacheEntry::GetNeededSize(): pOut->GetBitCount() == 0" );
            nNeededSize = 256000;
        }
    }
    else if( GRAPHIC_GDIMETAFILE == eType )
        nNeededSize = rGraphic.GetSizeBytes();
    else
        nNeededSize = 0;

    return nNeededSize;
}

// -----------------------------------------------------------------------------

GraphicDisplayCacheEntry::~GraphicDisplayCacheEntry()
{
    if( mpMtf )
        delete mpMtf;

    if( mpBmpEx )
        delete mpBmpEx;
}

// -----------------------------------------------------------------------------

void GraphicDisplayCacheEntry::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const
{
    if( mpMtf )
        GraphicManager::ImplDraw( pOut, rPt, rSz, *mpMtf, maAttr );
    else if( mpBmpEx )
    {
        if( maAttr.IsRotated() )
        {
            Polygon aPoly( Rectangle( rPt, rSz ) );

            aPoly.Rotate( rPt, maAttr.GetRotation() % 3600 );
            const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
            pOut->DrawBitmapEx( aRotBoundRect.TopLeft(), aRotBoundRect.GetSize(), *mpBmpEx );
        }
        else
            pOut->DrawBitmapEx( rPt, rSz, *mpBmpEx );
    }
}

// -----------------------
// - GraphicCache -
// -----------------------

GraphicCache::GraphicCache( GraphicManager& rMgr, sal_uLong nDisplayCacheSize, sal_uLong nMaxObjDisplayCacheSize ) :
    mrMgr                   ( rMgr ),
    mnReleaseTimeoutSeconds ( 0UL ),
    mnMaxDisplaySize        ( nDisplayCacheSize ),
    mnMaxObjDisplaySize     ( nMaxObjDisplayCacheSize ),
    mnUsedDisplaySize       ( 0UL )
{
    maReleaseTimer.SetTimeoutHdl( LINK( this, GraphicCache, ReleaseTimeoutHdl ) );
    maReleaseTimer.SetTimeout( RELEASE_TIMEOUT );
    maReleaseTimer.Start();
}

// -----------------------------------------------------------------------------

GraphicCache::~GraphicCache()
{
    DBG_ASSERT( !maGraphicCache.Count(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in cache" );
    DBG_ASSERT( !maDisplayCache.Count(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in display cache" );
}

// -----------------------------------------------------------------------------

void GraphicCache::AddGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute,
                                     const ByteString* pID, const GraphicObject* pCopyObj )
{
    sal_Bool bInserted = sal_False;

    if( !rObj.IsSwappedOut() &&
        ( pID || ( pCopyObj && ( pCopyObj->GetType() != GRAPHIC_NONE ) ) || ( rObj.GetType() != GRAPHIC_NONE ) ) )
    {
        if( pCopyObj )
        {
            GraphicCacheEntry* pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.First() );

            while( !bInserted && pEntry )
            {
                if( pEntry->HasGraphicObjectReference( *pCopyObj ) )
                {
                    pEntry->AddGraphicObjectReference( rObj, rSubstitute );
                    bInserted = sal_True;
                }
                else
                {
                    pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.Next() );
                }
            }
        }

        if( !bInserted )
        {
            GraphicCacheEntry* pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.First() );
            ::std::auto_ptr< GraphicID > apID;

            if( !pID )
            {
                apID.reset( new GraphicID( rObj ) );
            }

            while( !bInserted && pEntry )
            {
                const GraphicID& rEntryID = pEntry->GetID();

                if( pID )
                {
                    if( rEntryID.GetIDString() == *pID )
                    {
                        pEntry->TryToSwapIn();

                        // since pEntry->TryToSwapIn can modify our current list, we have to
                        // iterate from beginning to add a reference to the appropriate
                        // CacheEntry object; after this, quickly jump out of the outer iteration
                        for( pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.First() );
                             !bInserted && pEntry;
                             pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.Next() ) )
                        {
                            const GraphicID& rID = pEntry->GetID();

                            if( rID.GetIDString() == *pID )
                            {
                                pEntry->AddGraphicObjectReference( rObj, rSubstitute );
                                bInserted = sal_True;
                            }
                        }

                        if( !bInserted )
                        {
                            maGraphicCache.Insert( new GraphicCacheEntry( rObj ), LIST_APPEND );
                            bInserted = sal_True;
                        }
                    }
                }
                else
                {
                    if( rEntryID == *apID )
                    {
                        pEntry->AddGraphicObjectReference( rObj, rSubstitute );
                        bInserted = sal_True;
                    }
                }

                if( !bInserted )
                    pEntry = static_cast< GraphicCacheEntry* >( maGraphicCache.Next() );
            }
        }
    }

    if( !bInserted )
        maGraphicCache.Insert( new GraphicCacheEntry( rObj ), LIST_APPEND );
}

// -----------------------------------------------------------------------------

void GraphicCache::ReleaseGraphicObject( const GraphicObject& rObj )
{
    // Release cached object
    GraphicCacheEntry*  pEntry = (GraphicCacheEntry*) maGraphicCache.First();
    sal_Bool                bRemoved = sal_False;

    while( !bRemoved && pEntry )
    {
        bRemoved = pEntry->ReleaseGraphicObjectReference( rObj );

        if( bRemoved )
        {
            if( 0 == pEntry->GetGraphicObjectReferenceCount() )
            {
                // if graphic cache entry has no more references,
                // the corresponding display cache object can be removed
                GraphicDisplayCacheEntry* pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.First();

                while( pDisplayEntry )
                {
                    if( pDisplayEntry->GetReferencedCacheEntry() == pEntry )
                    {
                        mnUsedDisplaySize -= pDisplayEntry->GetCacheSize();
                        maDisplayCache.Remove( pDisplayEntry );
                        delete pDisplayEntry;
                        pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.GetCurObject();
                    }
                    else
                        pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
                }

                // delete graphic cache entry
                maGraphicCache.Remove( (void*) pEntry );
                delete pEntry;
            }
        }
        else
            pEntry = (GraphicCacheEntry*) maGraphicCache.Next();
    }

    DBG_ASSERT( bRemoved, "GraphicCache::ReleaseGraphicObject(...): GraphicObject not found in cache" );
}

// -----------------------------------------------------------------------------

void GraphicCache::GraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    // notify cache that rObj is swapped out (and can thus be pruned
    // from the cache)
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
        pEntry->GraphicObjectWasSwappedOut( rObj );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( !pEntry )
        return sal_False;

    return pEntry->FillSwappedGraphicObject( rObj, rSubstitute );
}

// -----------------------------------------------------------------------------

void GraphicCache::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( pEntry )
    {
        if( pEntry->GetID().IsEmpty() )
        {
            ReleaseGraphicObject( rObj );
            AddGraphicObject( rObj, (Graphic&) rObj.GetGraphic(), NULL, NULL );
        }
        else
            pEntry->GraphicObjectWasSwappedIn( rObj );
    }
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxDisplayCacheSize( sal_uLong nNewCacheSize )
{
    mnMaxDisplaySize = nNewCacheSize;

    if( GetMaxDisplayCacheSize() < GetUsedDisplayCacheSize() )
        ImplFreeDisplayCacheSpace( GetUsedDisplayCacheSize() - GetMaxDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxObjDisplayCacheSize( sal_uLong nNewMaxObjSize, sal_Bool bDestroyGreaterCached )
{
    const sal_Bool bDestroy = ( bDestroyGreaterCached && ( nNewMaxObjSize < mnMaxObjDisplaySize ) );

    mnMaxObjDisplaySize = Min( nNewMaxObjSize, mnMaxDisplaySize );

    if( bDestroy )
    {
        GraphicDisplayCacheEntry* pCacheObj = (GraphicDisplayCacheEntry*) maDisplayCache.First();

        while( pCacheObj )
        {
            if( pCacheObj->GetCacheSize() > mnMaxObjDisplaySize )
            {
                mnUsedDisplaySize -= pCacheObj->GetCacheSize();
                maDisplayCache.Remove( pCacheObj );
                delete pCacheObj;
                pCacheObj = (GraphicDisplayCacheEntry*) maDisplayCache.GetCurObject();
            }
            else
                pCacheObj = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
        }
    }
}

// -----------------------------------------------------------------------------

void GraphicCache::SetCacheTimeout( sal_uLong nTimeoutSeconds )
{
    if( mnReleaseTimeoutSeconds != nTimeoutSeconds )
    {
        GraphicDisplayCacheEntry*   pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.First();
        ::salhelper::TTimeValue           aReleaseTime;

        if( ( mnReleaseTimeoutSeconds = nTimeoutSeconds ) != 0 )
        {
            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( nTimeoutSeconds, 0 ) );
        }

        while( pDisplayEntry )
        {
            pDisplayEntry->SetReleaseTime( aReleaseTime );
            pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
        }
    }
}

// -----------------------------------------------------------------------------

void GraphicCache::ClearDisplayCache()
{
    for( void* pObj = maDisplayCache.First(); pObj; pObj = maDisplayCache.Next() )
        delete (GraphicDisplayCacheEntry*) pObj;

    maDisplayCache.Clear();
    mnUsedDisplaySize = 0UL;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                       const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    return( GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) <=
            GetMaxObjDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                     const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );
    //GraphicDisplayCacheEntry* pDisplayEntry = (GraphicDisplayCacheEntry*) ( (GraphicCache*) this )->maDisplayCache.First(); // -Wall removed ....
    sal_Bool                        bFound = sal_False;

    if( pCacheEntry )
    {
        for( long i = 0, nCount = maDisplayCache.Count(); !bFound && ( i < nCount ); i++ )
            if( ( (GraphicDisplayCacheEntry*) maDisplayCache.GetObject( i ) )->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
                bFound = sal_True;
    }

    return bFound;
}

// -----------------------------------------------------------------------------

ByteString GraphicCache::GetUniqueID( const GraphicObject& rObj ) const
{
    ByteString          aRet;
    GraphicCacheEntry*  pEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );

    // ensure that the entry is correctly initialized (it has to be read at least once)
    if( pEntry && pEntry->GetID().IsEmpty() )
        pEntry->TryToSwapIn();

    // do another call to ImplGetCacheEntry in case of modified entry list
    pEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );

    if( pEntry )
        aRet = pEntry->GetID().GetIDString();

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const BitmapEx& rBmpEx )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    sal_Bool        bRet = sal_False;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rBmpEx );

        if( GetCacheTimeout() )
        {
            ::salhelper::TTimeValue aReleaseTime;

            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            pNewEntry->SetReleaseTime( aReleaseTime );
        }

        maDisplayCache.Insert( pNewEntry, LIST_APPEND );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const GDIMetaFile& rMtf )
{
    const sal_uLong nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    sal_Bool        bRet = sal_False;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rMtf );

        if( GetCacheTimeout() )
        {
            ::salhelper::TTimeValue aReleaseTime;

            osl_getSystemTime( &aReleaseTime );
            aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            pNewEntry->SetReleaseTime( aReleaseTime );
        }

        maDisplayCache.Insert( pNewEntry, LIST_APPEND );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                        const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ImplGetCacheEntry( rObj );
    GraphicDisplayCacheEntry*   pDisplayCacheEntry = (GraphicDisplayCacheEntry*) maDisplayCache.First();
    sal_Bool                        bRet = sal_False;

    while( !bRet && pDisplayCacheEntry )
    {
        if( pDisplayCacheEntry->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
        {
            ::salhelper::TTimeValue aReleaseTime;

            // put found object at last used position
            maDisplayCache.Insert( maDisplayCache.Remove( pDisplayCacheEntry ), LIST_APPEND );

            if( GetCacheTimeout() )
            {
                osl_getSystemTime( &aReleaseTime );
                aReleaseTime.addTime( ::salhelper::TTimeValue( GetCacheTimeout(), 0 ) );
            }

            pDisplayCacheEntry->SetReleaseTime( aReleaseTime );
            bRet = sal_True;
        }
        else
            pDisplayCacheEntry = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
    }

    if( bRet )
        pDisplayCacheEntry->Draw( pOut, rPt, rSz );

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicCache::ImplFreeDisplayCacheSpace( sal_uLong nSizeToFree )
{
    sal_uLong nFreedSize = 0UL;

    if( nSizeToFree )
    {
        void* pObj = maDisplayCache.First();

        if( nSizeToFree > mnUsedDisplaySize )
            nSizeToFree = mnUsedDisplaySize;

        while( pObj )
        {
            GraphicDisplayCacheEntry* pCacheObj = (GraphicDisplayCacheEntry*) pObj;

            nFreedSize += pCacheObj->GetCacheSize();
            mnUsedDisplaySize -= pCacheObj->GetCacheSize();
            maDisplayCache.Remove( pObj );
            delete pCacheObj;

            if( nFreedSize >= nSizeToFree )
                break;
            else
                pObj = maDisplayCache.GetCurObject();
        }
    }

    return( nFreedSize >= nSizeToFree );
}

// -----------------------------------------------------------------------------

GraphicCacheEntry* GraphicCache::ImplGetCacheEntry( const GraphicObject& rObj )
{
    GraphicCacheEntry* pRet = NULL;

    for( void* pObj = maGraphicCache.First(); !pRet && pObj; pObj = maGraphicCache.Next() )
        if( ( (GraphicCacheEntry*) pObj )->HasGraphicObjectReference( rObj ) )
            pRet = (GraphicCacheEntry*) pObj;

    return pRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GraphicCache, ReleaseTimeoutHdl, Timer*, pTimer )
{
    pTimer->Stop();

    ::salhelper::TTimeValue           aCurTime;
    GraphicDisplayCacheEntry*   pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.First();

    osl_getSystemTime( &aCurTime );

    while( pDisplayEntry )
    {
        const ::salhelper::TTimeValue& rReleaseTime = pDisplayEntry->GetReleaseTime();

        if( !rReleaseTime.isEmpty() && ( rReleaseTime < aCurTime ) )
        {
            mnUsedDisplaySize -= pDisplayEntry->GetCacheSize();
            maDisplayCache.Remove( pDisplayEntry );
            delete pDisplayEntry;
            pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.GetCurObject();
        }
        else
            pDisplayEntry = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
    }

    pTimer->Start();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
