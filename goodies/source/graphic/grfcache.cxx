/*************************************************************************
 *
 *  $RCSfile: grfcache.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:16 $
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
#include <vcl/outdev.hxx>
#include "grfcache.hxx"

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

    BOOL        operator==( const GraphicID& rID ) const
                {
                    return( rID.mnID1 == mnID1 && rID.mnID2 == mnID2 &&
                            rID.mnID3 == mnID3 && rID.mnID4 == mnID4 );
                }

    ByteString  GetIDString() const;
};

// -----------------------------------------------------------------------------

GraphicID::GraphicID( const GraphicObject& rObj )
{
    const Graphic& rGraphic = rObj.GetGraphic();

    mnID1 = ( (ULONG) rGraphic.GetType() ) << 28;

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

                mnID1 |= ( ( ( (ULONG) aBmpEx.GetTransparentType() << 8 ) | ( aBmpEx.IsAlpha() ? 1 : 0 ) ) & 0x0fffffff );
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
    BOOL                mbSwappedAll    : 1;
    BOOL                mbInitialized   : 1;

    BOOL                ImplInit( const GraphicObject& rObj );
    BOOL                ImplMatches( const GraphicObject& rObj ) const { return( GraphicID( rObj ) == maID ); }
    void                ImplFillSubstitute( Graphic& rSubstitute );

public:

                        GraphicCacheEntry( const GraphicObject& rObj );
                        ~GraphicCacheEntry();

    const GraphicID&    GetID() const { return maID; }

    BOOL                AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute );
    BOOL                ReleaseGraphicObjectReference( const GraphicObject& rObj );
    ULONG               GetGraphicObjectReferenceCount() { return maGraphicObjectList.Count(); }
    BOOL                HasGraphicObjectReference( const GraphicObject& rObj );

    void                GraphicObjectWasSwappedOut( const GraphicObject& rObj );
    BOOL                FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute );
    void                GraphicObjectWasSwappedIn( const GraphicObject& rObj );

    BOOL                IsInitialized() const { return mbInitialized; }
};

// -----------------------------------------------------------------------------

GraphicCacheEntry::GraphicCacheEntry( const GraphicObject& rObj ) :
    maID            ( rObj ),
    mpBmpEx         ( NULL ),
    mpMtf           ( NULL ),
    mpAnimation     ( NULL ),
    mbInitialized   ( ImplInit( rObj ) )
{
    mbSwappedAll = !mbInitialized;
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

BOOL GraphicCacheEntry::ImplInit( const GraphicObject& rObj )
{
    BOOL bRet;

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
            break;
        }

        if( rGraphic.IsLink() )
            maGfxLink = ( (Graphic&) rGraphic ).GetLink();
        else
            maGfxLink = GfxLink();

        bRet = TRUE;
    }
    else
        bRet = FALSE;

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
    const ULONG         nDocFilePos = rSubstitute.GetDocFilePos();
    const GraphicType   eOldType = rSubstitute.GetType();
    const BOOL          bDefaultType = ( rSubstitute.GetType() == GRAPHIC_DEFAULT );

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

BOOL GraphicCacheEntry::AddGraphicObjectReference( const GraphicObject& rObj, Graphic& rSubstitute )
{
    BOOL bRet;

    if( !rObj.IsSwappedOut() )
    {
        // append object pointer to list
        maGraphicObjectList.Insert( (void*) &rObj, LIST_APPEND );

        if( mbSwappedAll )
        {
            ImplInit( rObj );
            mbSwappedAll = FALSE;
        }

        ImplFillSubstitute( rSubstitute );
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicCacheEntry::ReleaseGraphicObjectReference( const GraphicObject& rObj )
{
    BOOL bRet = FALSE;

    for( void* pObj = maGraphicObjectList.First(); !bRet && pObj; pObj = maGraphicObjectList.Next() )
    {
        if( &rObj == (GraphicObject*) pObj )
        {
            maGraphicObjectList.Remove( pObj );
            bRet = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicCacheEntry::HasGraphicObjectReference( const GraphicObject& rObj )
{
    BOOL bRet = FALSE;

    for( void* pObj = maGraphicObjectList.First(); !bRet && pObj; pObj = maGraphicObjectList.Next() )
        if( &rObj == (GraphicObject*) pObj )
            bRet = TRUE;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicCacheEntry::GraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    mbSwappedAll = TRUE;

    for( void* pObj = maGraphicObjectList.First(); mbSwappedAll && pObj; pObj = maGraphicObjectList.Next() )
        if( !( (GraphicObject*) pObj )->IsSwappedOut() )
            mbSwappedAll = FALSE;

    if( mbSwappedAll )
    {
        delete mpBmpEx, mpBmpEx = NULL;
        delete mpMtf, mpMtf = NULL;
        delete mpAnimation, mpAnimation = NULL;
    }
}

// -----------------------------------------------------------------------------

BOOL GraphicCacheEntry::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    BOOL bRet;

    if( !mbSwappedAll && rObj.IsSwappedOut() )
    {
        ImplFillSubstitute( rSubstitute );
        bRet = TRUE;
    }
    else
        bRet = FALSE;

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

    const GraphicCacheEntry*    mpRefCacheEntry;
    GDIMetaFile*                mpMtf;
    BitmapEx*                   mpBmpEx;
    GraphicAttr                 maAttr;
    Size                        maOutSizePix;
    ULONG                       mnCacheSize;

public:

    static ULONG                GetNeededSize( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr );

public:

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const BitmapEx& rBmpEx ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( NULL ), mpBmpEx( new BitmapEx( rBmpEx ) ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ) {}

                                GraphicDisplayCacheEntry( const GraphicCacheEntry* pRefCacheEntry,
                                                          OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                                          const GDIMetaFile& rMtf ) :
                                    mpRefCacheEntry( pRefCacheEntry ),
                                    mpMtf( new GDIMetaFile( rMtf ) ), mpBmpEx( NULL ),
                                    maAttr( rAttr ), maOutSizePix( pOut->LogicToPixel( rSz ) ),
                                    mnCacheSize( GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) ) {}

                                ~GraphicDisplayCacheEntry();

    const GraphicAttr&          GetAttr() const { return maAttr; }
    const Size&                 GetOutputSizePixel() const { return maOutSizePix; }
    const long                  GetCacheSize() const { return mnCacheSize; }
    const GraphicCacheEntry*    GetReferencedCacheEntry() const { return mpRefCacheEntry; }

    BOOL                        Matches( OutputDevice* pOut, const Point& rPtPixel, const Size& rSzPixel,
                                         const GraphicCacheEntry* pCacheEntry, const GraphicAttr& rAttr ) const
                                {
                                    return( ( pCacheEntry == mpRefCacheEntry ) &&
                                            ( maAttr == rAttr ) &&
                                            ( ( maOutSizePix == rSzPixel ) || ( !maOutSizePix.Width() && !maOutSizePix.Height() ) ) );
                                }

    void                        Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz ) const;
};

// -----------------------------------------------------------------------------

ULONG GraphicDisplayCacheEntry::GetNeededSize( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                               const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const GraphicType   eType = rObj.GetGraphic().GetType();
    ULONG               nNeededSize;

    if( GRAPHIC_BITMAP == eType )
    {
        const Size aOutSizePix( pOut->LogicToPixel( rSz ) );
        const long nBitCount = pOut->GetBitCount();

        if( nBitCount )
        {
            nNeededSize = aOutSizePix.Width() * aOutSizePix.Height() * nBitCount / 8;

            if( rObj.IsTransparent() || ( rAttr.GetRotation() % 3600 ) )
                nNeededSize += nNeededSize / nBitCount;
        }
        else
        {
             DBG_ERROR( "GraphicDisplayCacheEntry::GetNeededSize(): pOut->GetBitCount() == 0" );
            nNeededSize = 256000;
        }
    }
    else if( GRAPHIC_GDIMETAFILE == eType )
        nNeededSize = 65535;
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
        GraphicManager::ImplDraw( pOut, rPt, rSz, *mpBmpEx, maAttr );
}

// -----------------------
// - GraphicCache -
// -----------------------

GraphicCache::GraphicCache( GraphicManager& rMgr, ULONG nDisplayCacheSize, ULONG nMaxObjDisplayCacheSize ) :
    mrMgr               ( rMgr ),
    mnMaxDisplaySize    ( nDisplayCacheSize ),
    mnMaxObjDisplaySize ( nMaxObjDisplayCacheSize ),
    mnUsedDisplaySize   ( 0UL )
{
}

// -----------------------------------------------------------------------------

GraphicCache::~GraphicCache()
{
    DBG_ASSERT( !maGraphicCache.Count(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in cache" );
    DBG_ASSERT( !maDisplayCache.Count(), "GraphicCache::~GraphicCache(): there are some GraphicObjects in display cache" );
}

// -----------------------------------------------------------------------------

void GraphicCache::AddGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute, const ByteString* pID )
{
    BOOL bInserted = FALSE;

    if( !rObj.IsSwappedOut() )
    {
        GraphicCacheEntry*  pEntry = (GraphicCacheEntry*) maGraphicCache.First();
        const GraphicID     aID( rObj );

        while( !bInserted && pEntry )
        {
            const GraphicID& rID = pEntry->GetID();

            if( pID )
            {
                if( rID.GetIDString() == *pID )
                    bInserted = pEntry->AddGraphicObjectReference( rObj, rSubstitute );
            }
            else
            {
                if( pEntry->GetID() == aID )
                    bInserted = pEntry->AddGraphicObjectReference( rObj, rSubstitute );
            }

            pEntry = (GraphicCacheEntry*) maGraphicCache.Next();
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
    BOOL                bRemoved = FALSE;

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
    ImplGetCacheEntry( rObj )->GraphicObjectWasSwappedOut( rObj );
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::FillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    return( ImplGetCacheEntry( rObj )->FillSwappedGraphicObject( rObj, rSubstitute ) );
}

// -----------------------------------------------------------------------------

void GraphicCache::GraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    GraphicCacheEntry* pEntry = ImplGetCacheEntry( rObj );

    if( !pEntry->IsInitialized() )
    {
        ReleaseGraphicObject( rObj );
        AddGraphicObject( rObj, (Graphic&) rObj.GetGraphic(), NULL );
    }
    else
        pEntry->GraphicObjectWasSwappedIn( rObj );
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxDisplayCacheSize( ULONG nNewCacheSize )
{
    mnMaxDisplaySize = nNewCacheSize;

    if( GetMaxDisplayCacheSize() < GetUsedDisplayCacheSize() )
        ImplFreeDisplayCacheSpace( GetUsedDisplayCacheSize() - GetMaxDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

void GraphicCache::SetMaxObjDisplayCacheSize( ULONG nNewMaxObjSize, BOOL bDestroyGreaterCached )
{
    const BOOL bDestroy = ( bDestroyGreaterCached && ( nNewMaxObjSize < mnMaxObjDisplaySize ) );

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

void GraphicCache::ClearDisplayCache()
{
    for( void* pObj = maDisplayCache.First(); pObj; pObj = maDisplayCache.Next() )
        delete (GraphicDisplayCacheEntry*) pObj;

    maDisplayCache.Clear();
    mnUsedDisplaySize = 0UL;
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::IsDisplayCacheable( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                       const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    return( GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr ) <=
            GetMaxObjDisplayCacheSize() );
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::IsInDisplayCache( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                     const GraphicObject& rObj, const GraphicAttr& rAttr ) const
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ( (GraphicCache*) this )->ImplGetCacheEntry( rObj );
    GraphicDisplayCacheEntry*   pDisplayEntry = (GraphicDisplayCacheEntry*) ( (GraphicCache*) this )->maDisplayCache.First();
    BOOL                        bFound = FALSE;

    for( long i = 0, nCount = maDisplayCache.Count(); !bFound && ( i < nCount ); i++ )
        if( ( (GraphicDisplayCacheEntry*) maDisplayCache.GetObject( i ) )->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
            bFound = TRUE;

    return bFound;
}

// -----------------------------------------------------------------------------

ByteString GraphicCache::GetUniqueID( const GraphicObject& rObj ) const
{
    return( ( (GraphicCache*) this )->ImplGetCacheEntry( rObj )->GetID().GetIDString() );
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const BitmapEx& rBmpEx )
{
    const ULONG nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    BOOL        bRet = FALSE;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rBmpEx );


        maDisplayCache.Insert( pNewEntry, (ULONG) 0 );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::CreateDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                          const GraphicObject& rObj, const GraphicAttr& rAttr,
                                          const GDIMetaFile& rMtf )
{
    const ULONG nNeededSize = GraphicDisplayCacheEntry::GetNeededSize( pOut, rPt, rSz, rObj, rAttr );
    BOOL        bRet = FALSE;

    if( nNeededSize <= GetMaxObjDisplayCacheSize() )
    {
        if( nNeededSize > GetFreeDisplayCacheSize() )
            ImplFreeDisplayCacheSpace( nNeededSize - GetFreeDisplayCacheSize() );

        GraphicDisplayCacheEntry* pNewEntry = new GraphicDisplayCacheEntry( ImplGetCacheEntry( rObj ),
                                                                            pOut, rPt, rSz, rObj, rAttr, rMtf );


        maDisplayCache.Insert( pNewEntry, (ULONG) 0 );
        mnUsedDisplaySize += pNewEntry->GetCacheSize();
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::DrawDisplayCacheObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                        const GraphicObject& rObj, const GraphicAttr& rAttr )
{
    const Point                 aPtPixel( pOut->LogicToPixel( rPt ) );
    const Size                  aSzPixel( pOut->LogicToPixel( rSz ) );
    const GraphicCacheEntry*    pCacheEntry = ImplGetCacheEntry( rObj );
    GraphicDisplayCacheEntry*   pDisplayCacheEntry = (GraphicDisplayCacheEntry*) maDisplayCache.First();
    BOOL                        bRet = FALSE;

    while( !bRet && pDisplayCacheEntry )
    {
        if( pDisplayCacheEntry->Matches( pOut, aPtPixel, aSzPixel, pCacheEntry, rAttr ) )
        {
            // put found object at last used position
            maDisplayCache.Insert( maDisplayCache.Remove( pDisplayCacheEntry ), LIST_APPEND );
            bRet = TRUE;
        }
        else
            pDisplayCacheEntry = (GraphicDisplayCacheEntry*) maDisplayCache.Next();
    }

    if( bRet )
        pDisplayCacheEntry->Draw( pOut, rPt, rSz );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicCache::ImplFreeDisplayCacheSpace( ULONG nSizeToFree )
{
    ULONG nFreedSize = 0UL;

    if( nSizeToFree && ( nSizeToFree <= mnMaxDisplaySize ) )
    {
        void* pObj = maDisplayCache.First();

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
