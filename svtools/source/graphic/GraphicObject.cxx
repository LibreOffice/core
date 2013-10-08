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

#include "sal/config.h"

#include <algorithm>

#include <officecfg/Office/Common.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svtools/GraphicObject.hxx>
#include <svtools/GraphicManager.hxx>
#include <comphelper/servicedecl.hxx>
#include <vcl/pdfextoutdevdata.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using css::beans::XPropertySet;
using css::container::XNameContainer;
using css::uno::Reference;
using css::uno::RuntimeException;
using css::uno::Sequence;
using css::uno::UNO_QUERY;
using css::uno::XInterface;

struct GrfSimpleCacheObj
{
    Graphic     maGraphic;
    GraphicAttr maAttr;

                GrfSimpleCacheObj( const Graphic& rGraphic, const GraphicAttr& rAttr ) :
                    maGraphic( rGraphic ), maAttr( rAttr ) {}
};

TYPEINIT1_AUTOFACTORY( GraphicObject, SvDataCopyStream );

struct ImplTileInfo
{
    ImplTileInfo() : aTileTopLeft(), aNextTileTopLeft(), aTileSizePixel(), nTilesEmptyX(0), nTilesEmptyY(0) {}

    Point aTileTopLeft;     // top, left position of the rendered tile
    Point aNextTileTopLeft; // top, left position for next recursion
                            // level's tile
    Size  aTileSizePixel;   // size of the generated tile (might
                            // differ from
                            // aNextTileTopLeft-aTileTopLeft, because
                            // this is nExponent*prevTileSize. The
                            // generated tile is always nExponent
                            // times the previous tile, such that it
                            // can be used in the next stage. The
                            // required area coverage is often
                            // less. The extraneous area covered is
                            // later overwritten by the next stage)
    int   nTilesEmptyX;     // number of original tiles empty right of
                            // this tile. This counts from
                            // aNextTileTopLeft, i.e. the additional
                            // area covered by aTileSizePixel is not
                            // considered here. This is for
                            // unification purposes, as the iterative
                            // calculation of the next level's empty
                            // tiles has to be based on this value.
    int   nTilesEmptyY;     // as above, for Y
};


GraphicObject::GraphicObject( void) :
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( );
}

GraphicObject::GraphicObject( const Graphic& rGraphic ) :
    maGraphic   ( rGraphic ),
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager(  );
}

GraphicObject::GraphicObject( const GraphicObject& rGraphicObj) :
    maGraphic   ( rGraphicObj.GetGraphic() ),
    maAttr      ( rGraphicObj.maAttr ),
    mpLink      ( rGraphicObj.mpLink ? ( new OUString( *rGraphicObj.mpLink ) ) : NULL ),
    mpUserData  ( rGraphicObj.mpUserData ? ( new OUString( *rGraphicObj.mpUserData ) ) : NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( NULL, &rGraphicObj );
}

GraphicObject::GraphicObject( const GraphicObject* pGraphicObj) :
    maGraphic   ( pGraphicObj->GetGraphic() ),
    maAttr      ( pGraphicObj->maAttr ),
    mpLink      ( pGraphicObj->mpLink ? ( new OUString( *(pGraphicObj->mpLink) ) ) : NULL ),
    mpUserData  ( pGraphicObj->mpUserData ? ( new OUString( *(pGraphicObj->mpUserData) ) ) : NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( NULL, pGraphicObj );
}

GraphicObject::GraphicObject( const GraphicObject* pGraphicObj, const Graphic& rGraphic) :
    maGraphic   ( rGraphic ),
    maAttr      ( pGraphicObj->maAttr ),
    mpLink      ( pGraphicObj->mpLink ? ( new OUString( *(pGraphicObj->mpLink) ) ) : NULL ),
    mpUserData  ( pGraphicObj->mpUserData ? ( new OUString( *(pGraphicObj->mpUserData) ) ) : NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( NULL, pGraphicObj );
}

GraphicObject::GraphicObject( const OString& rUniqueID ) :
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();

    // assign default properties
    ImplAssignGraphicData();

    ImplSetGraphicManager(&rUniqueID );

    // update properties
    ImplAssignGraphicData();
}

GraphicObject::GraphicObject( css::uno::Sequence< css::uno::Any > const & args,
                              css::uno::Reference< css::uno::XComponentContext > const & )
    throw( RuntimeException ) :
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();

    if ( args.getLength() == 1 )
    {
        OUString sId;
        if ( !( args[ 0 ] >>= sId ) || sId.isEmpty() )
            throw css::lang::IllegalArgumentException();

        OString bsId ( OUStringToOString ( sId, RTL_TEXTENCODING_UTF8 ) );
        ImplSetGraphicManager( &bsId );
    }
    else
        ImplSetGraphicManager();
}

css::uno::Reference< css::graphic::XGraphic > SAL_CALL GraphicObject::getGraphic()
    throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    return GetGraphic().GetXGraphic();
}

void SAL_CALL GraphicObject::setGraphic( const css::uno::Reference< css::graphic::XGraphic >& /*xGraphic*/ )
    throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    assert(false);
//    Graphic aGraphic( xGraphic );
//    SetGraphic( aGraphic );
}

OUString SAL_CALL GraphicObject::getUniqueID()
    throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;

    return OStringToOUString(GetUniqueID(), RTL_TEXTENCODING_ASCII_US);
}

GraphicObject::~GraphicObject()
{
    GraphicManager::GetGlobalManager()->UnregisterObject(this);
    delete mpSwapOutTimer;
    delete mpSwapStreamHdl;
    delete mpLink;
    delete mpUserData;
    delete mpSimpleCache;
}

void GraphicObject::ImplConstruct()
{
    mpSwapStreamHdl = NULL;
    mpSwapOutTimer = NULL;
    mpSimpleCache = NULL;
    mnAnimationLoopCount = 0;
    mbAutoSwapped = false;
    mbIsInSwapIn = false;
    mbIsInSwapOut = false;
}

void GraphicObject::ImplAssignGraphicData()
{
    maPrefSize = maGraphic.GetPrefSize();
    maPrefMapMode = maGraphic.GetPrefMapMode();
    mnSizeBytes = maGraphic.GetSizeBytes();
    meType = maGraphic.GetType();
    mbTransparent = maGraphic.IsTransparent();
    mbAlpha = maGraphic.IsAlpha();
    mbAnimated = maGraphic.IsAnimated();
    mbEPS = maGraphic.IsEPS();
    mnAnimationLoopCount = ( mbAnimated ? maGraphic.GetAnimationLoopCount() : 0 );
}


void GraphicObject::ImplSetGraphicManager(const OString* pID, const GraphicObject* pCopyObject )
{
    GraphicManager::GetGlobalManager()->RegisterObject(this, maGraphic, pID, pCopyObject);
}

void GraphicObject::ImplAutoSwapIn()
{
    if( IsSwappedOut() )
    {
        if( GraphicManager::GetGlobalManager()->FillSwappedGraphicObject( this, maGraphic ) )
        {
            mbAutoSwapped = false;
        }
        else
        {
            mbIsInSwapIn = true;

            if( maGraphic.SwapIn() )
                mbAutoSwapped = false;
            else
            {
                SvStream* pStream = GetSwapStream();

                if( GRFMGR_AUTOSWAPSTREAM_NONE != pStream )
                {
                    if( GRFMGR_AUTOSWAPSTREAM_LINK == pStream )
                    {
                        if( HasLink() )
                        {
                            OUString aURLStr;

                            if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( GetLink(), aURLStr ) )
                            {
                                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_READ );

                                if( pIStm )
                                {
                                    (*pIStm) >> maGraphic;
                                    mbAutoSwapped = ( maGraphic.GetType() != GRAPHIC_NONE );
                                    delete pIStm;
                                }
                            }
                        }
                    }
                    else if( GRFMGR_AUTOSWAPSTREAM_TEMP == pStream )
                        mbAutoSwapped = !maGraphic.SwapIn();
                    else if( GRFMGR_AUTOSWAPSTREAM_LOADED == pStream )
                        mbAutoSwapped = maGraphic.IsSwapOut();
                    else
                    {
                        mbAutoSwapped = !maGraphic.SwapIn( pStream );
                        delete pStream;
                    }
                }
                else
                {
                    DBG_ASSERT( ( GRAPHIC_NONE == meType ) || ( GRAPHIC_DEFAULT == meType ),
                                "GraphicObject::ImplAutoSwapIn: could not get stream to swap in graphic! (=>KA)" );
                }
            }

            mbIsInSwapIn = false;

            if( !mbAutoSwapped)
            {
                GraphicManager::GetGlobalManager()->GraphicObjectWasSwappedIn( this );
            }
        }
    }
}

bool GraphicObject::ImplGetCropParams( OutputDevice* pOut, Point& rPt, Size& rSz, const GraphicAttr* pAttr,
                                       PolyPolygon& rClipPolyPoly, bool& bRectClipRegion ) const
{
    bool bRet = false;

    if( GetType() != GRAPHIC_NONE )
    {
        Polygon         aClipPoly( Rectangle( rPt, rSz ) );
        const sal_uInt16    nRot10 = pAttr->GetRotation() % 3600;
        const Point     aOldOrigin( rPt );
        const MapMode   aMap100( MAP_100TH_MM );
        Size            aSize100;
        long            nTotalWidth, nTotalHeight;

        if( nRot10 )
        {
            aClipPoly.Rotate( rPt, nRot10 );
            bRectClipRegion = false;
        }
        else
        {
            bRectClipRegion = true;
        }
        rClipPolyPoly = aClipPoly;

        if( maGraphic.GetPrefMapMode() == MAP_PIXEL )
            aSize100 = Application::GetDefaultDevice()->PixelToLogic( maGraphic.GetPrefSize(), aMap100 );
        else
        {
            MapMode m(maGraphic.GetPrefMapMode());
            aSize100 = pOut->LogicToLogic( maGraphic.GetPrefSize(), &m, &aMap100 );
        }

        nTotalWidth = aSize100.Width() - pAttr->GetLeftCrop() - pAttr->GetRightCrop();
        nTotalHeight = aSize100.Height() - pAttr->GetTopCrop() - pAttr->GetBottomCrop();

        if( aSize100.Width() > 0 && aSize100.Height() > 0 && nTotalWidth > 0 && nTotalHeight > 0 )
        {
            double fScale = (double) aSize100.Width() / nTotalWidth;
            const long nNewLeft = -FRound( ( ( pAttr->GetMirrorFlags() & BMP_MIRROR_HORZ ) ? pAttr->GetRightCrop() : pAttr->GetLeftCrop() ) * fScale );
            const long nNewRight = nNewLeft + FRound( aSize100.Width() * fScale ) - 1;

            fScale = (double) rSz.Width() / aSize100.Width();
            rPt.X() += FRound( nNewLeft * fScale );
            rSz.Width() = FRound( ( nNewRight - nNewLeft + 1 ) * fScale );

            fScale = (double) aSize100.Height() / nTotalHeight;
            const long nNewTop = -FRound( ( ( pAttr->GetMirrorFlags() & BMP_MIRROR_VERT ) ? pAttr->GetBottomCrop() : pAttr->GetTopCrop() ) * fScale );
            const long nNewBottom = nNewTop + FRound( aSize100.Height() * fScale ) - 1;

            fScale = (double) rSz.Height() / aSize100.Height();
            rPt.Y() += FRound( nNewTop * fScale );
            rSz.Height() = FRound( ( nNewBottom - nNewTop + 1 ) * fScale );

            if( nRot10 )
            {
                Polygon aOriginPoly( 1 );

                aOriginPoly[ 0 ] = rPt;
                aOriginPoly.Rotate( aOldOrigin, nRot10 );
                rPt = aOriginPoly[ 0 ];
            }

            bRet = true;
        }
    }

    return bRet;
}

/*
GraphicObject& GraphicObject::operator=( const GraphicObject& rGraphicObj )
{
    if( &rGraphicObj != this )
    {
        GraphicManager::GetGlobalManager()->UnregisterObject( *this );

        delete mpSwapStreamHdl, mpSwapStreamHdl = NULL;
        delete mpSimpleCache, mpSimpleCache = NULL;
        delete mpLink;
        delete mpUserData;

        maGraphic = rGraphicObj.GetGraphic();
        maAttr = rGraphicObj.maAttr;
        mpLink = rGraphicObj.mpLink ? new String( *rGraphicObj.mpLink ) : NULL;
        mpUserData = rGraphicObj.mpUserData ? new String( *rGraphicObj.mpUserData ) : NULL;
        ImplAssignGraphicData();
        mbAutoSwapped = false;

        GraphicManager::GetGlobalManager()->RegisterObject( *this, maGraphic, NULL, &rGraphicObj );
    }

    return *this;
}
*/

bool GraphicObject::operator==( const GraphicObject& rGraphicObj ) const
{
    return( ( rGraphicObj.maGraphic == maGraphic ) &&
            ( rGraphicObj.maAttr == maAttr ) &&
            ( rGraphicObj.GetLink() == GetLink() ) );
}

OString GraphicObject::GetUniqueID() const
{
    if ( !IsInSwapIn() && IsEPS() )
        const_cast<GraphicObject*>(this)->FireSwapInRequest();

    OString aRet;

    aRet = GraphicManager::GetGlobalManager()->GetUniqueID( this );

    return aRet;
}

SvStream* GraphicObject::GetSwapStream() const
{
    return( HasSwapStreamHdl() ? (SvStream*) mpSwapStreamHdl->Call( (void*) this ) : GRFMGR_AUTOSWAPSTREAM_NONE );
}

void GraphicObject::SetAttr( const GraphicAttr& rAttr )
{
    maAttr = rAttr;

    if( mpSimpleCache && ( mpSimpleCache->maAttr != rAttr ) )
        delete mpSimpleCache, mpSimpleCache = NULL;
}

void GraphicObject::SetLink()
{
    if( mpLink )
        delete mpLink, mpLink = NULL;
}

void GraphicObject::SetLink( const OUString& rLink )
{
    delete mpLink, mpLink = new OUString( rLink );
}

OUString GraphicObject::GetLink() const
{
    if( mpLink )
        return *mpLink;
    else
        return OUString();
}

void GraphicObject::SetUserData()
{
    if( mpUserData )
        delete mpUserData, mpUserData = NULL;
}

void GraphicObject::SetUserData( const OUString& rUserData )
{
    delete mpUserData, mpUserData = new OUString( rUserData );
}

OUString GraphicObject::GetUserData() const
{
    if( mpUserData )
        return *mpUserData;
    else
        return OUString();
}

void GraphicObject::SetSwapStreamHdl()
{
    if( mpSwapStreamHdl )
    {
        delete mpSwapOutTimer, mpSwapOutTimer = NULL;
        delete mpSwapStreamHdl, mpSwapStreamHdl = NULL;
    }
}

void GraphicObject::SetSwapStreamHdl( const Link& rHdl, const sal_uLong nSwapOutTimeout )
{
    delete mpSwapStreamHdl, mpSwapStreamHdl = new Link( rHdl );

    if( nSwapOutTimeout )
    {
        if( !mpSwapOutTimer )
        {
            mpSwapOutTimer = new Timer;
            mpSwapOutTimer->SetTimeoutHdl( LINK( this, GraphicObject, ImplAutoSwapOutHdl ) );
        }

        mpSwapOutTimer->SetTimeout( nSwapOutTimeout );
        mpSwapOutTimer->Start();
    }
    else
        delete mpSwapOutTimer, mpSwapOutTimer = NULL;
}

void GraphicObject::FireSwapInRequest()
{
    ImplAutoSwapIn();
}

void GraphicObject::FireSwapOutRequest()
{
    ImplAutoSwapOutHdl( NULL );
}

void GraphicObject::GraphicManagerDestroyed()
{
}

bool GraphicObject::IsCached( OutputDevice* pOut, const Point& rPoint, const Size& rSize,
                              const GraphicAttr* pAttr, sal_uLong nFlags ) const
{
    bool bRet;

    if( nFlags & GRFMGR_DRAW_CACHED )
    {
        Point aPoint( rPoint );
        Size aSize( rSize );
        if ( pAttr->IsCropped() )
        {
            PolyPolygon aClipPolyPoly;
            bool        bRectClip;
            ImplGetCropParams( pOut, aPoint, aSize, pAttr, aClipPolyPoly, bRectClip );
        }
        bRet = GraphicManager::GetGlobalManager()->IsInCache( pOut, aPoint, aSize, this, ( pAttr ? *pAttr : GetAttr() ) );
    }
    else
    {
        bRet = false;
    }
    return bRet;
}

void GraphicObject::ReleaseFromCache()
{

    GraphicManager::GetGlobalManager()->ReleaseFromCache( *this );
}

bool GraphicObject::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                          const GraphicAttr* pAttr, sal_uLong nFlags )
{
    GraphicAttr         aAttr( pAttr ? *pAttr : GetAttr() );
    Point               aPt( rPt );
    Size                aSz( rSz );
    const sal_uInt32    nOldDrawMode = pOut->GetDrawMode();
    bool                bCropped = aAttr.IsCropped();
    bool                bCached = false;
    bool bRet;

    // #i29534# Provide output rects for PDF writer
    Rectangle           aCropRect;

    if( !( GRFMGR_DRAW_USE_DRAWMODE_SETTINGS & nFlags ) )
        pOut->SetDrawMode( nOldDrawMode & ( ~( DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT ) ) );

    // mirrored horizontically
    if( aSz.Width() < 0L )
    {
        aPt.X() += aSz.Width() + 1;
        aSz.Width() = -aSz.Width();
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BMP_MIRROR_HORZ );
    }

    // mirrored vertically
    if( aSz.Height() < 0L )
    {
        aPt.Y() += aSz.Height() + 1;
        aSz.Height() = -aSz.Height();
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BMP_MIRROR_VERT );
    }

    if( bCropped )
    {
        PolyPolygon aClipPolyPoly;
        bool        bRectClip;
        const bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

        pOut->Push( PUSH_CLIPREGION );

        if( bCrop )
        {
            if( bRectClip )
            {
                // #i29534# Store crop rect for later forwarding to
                // PDF writer
                aCropRect = aClipPolyPoly.GetBoundRect();
                pOut->IntersectClipRegion( aCropRect );
            }
            else
            {
                pOut->IntersectClipRegion(Region(aClipPolyPoly));
            }
        }
    }

    bRet = GraphicManager::GetGlobalManager()->DrawObj( pOut, aPt, aSz, this, aAttr, nFlags, bCached );

    if( bCropped )
        pOut->Pop();

    pOut->SetDrawMode( nOldDrawMode );

    // #i29534# Moved below OutDev restoration, to avoid multiple swap-ins
    // (code above needs to call GetGraphic twice)
    if( bCached )
    {
        if( mpSwapOutTimer )
            mpSwapOutTimer->Start();
        else
            FireSwapOutRequest();
    }

    return bRet;
}

// #i105243#
bool GraphicObject::DrawWithPDFHandling( OutputDevice& rOutDev,
                                         const Point& rPt, const Size& rSz,
                                         const GraphicAttr* pGrfAttr,
                                         const sal_uLong nFlags )
{
    const GraphicAttr aGrfAttr( pGrfAttr ? *pGrfAttr : GetAttr() );

    // Notify PDF writer about linked graphic (if any)
    bool bWritingPdfLinkedGraphic( false );
    Point aPt( rPt );
    Size aSz( rSz );
    Rectangle aCropRect;
    vcl::PDFExtOutDevData* pPDFExtOutDevData =
            dynamic_cast<vcl::PDFExtOutDevData*>(rOutDev.GetExtOutDevData());
    if( pPDFExtOutDevData )
    {
        // only delegate image handling to PDF, if no special treatment is necessary
        if( GetGraphic().IsLink() &&
            rSz.Width() > 0L &&
            rSz.Height() > 0L &&
            !aGrfAttr.IsSpecialDrawMode() &&
            !aGrfAttr.IsMirrored() &&
            !aGrfAttr.IsRotated() &&
            !aGrfAttr.IsAdjusted() )
        {
            bWritingPdfLinkedGraphic = true;

            if( aGrfAttr.IsCropped() )
            {
                PolyPolygon aClipPolyPoly;
                bool bRectClip;
                const bool bCrop = ImplGetCropParams( &rOutDev,
                                                      aPt, aSz,
                                                      &aGrfAttr,
                                                      aClipPolyPoly,
                                                      bRectClip );
                if ( bCrop && bRectClip )
                {
                    aCropRect = aClipPolyPoly.GetBoundRect();
                }
            }

            pPDFExtOutDevData->BeginGroup();
        }
    }

    bool bRet = Draw( &rOutDev, rPt, rSz, &aGrfAttr, nFlags );

    // Notify PDF writer about linked graphic (if any)
    if( bWritingPdfLinkedGraphic )
    {
        pPDFExtOutDevData->EndGroup( const_cast< Graphic& >(GetGraphic()),
                                     aGrfAttr.GetTransparency(),
                                     Rectangle( aPt, aSz ),
                                     aCropRect );
    }

    return bRet;
}

bool GraphicObject::DrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSize,
                               const Size& rOffset, const GraphicAttr* pAttr, sal_uLong nFlags, int nTileCacheSize1D )
{
    if( pOut == NULL || rSize.Width() == 0 || rSize.Height() == 0 )
        return false;

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    const MapMode   aMapMode( aOutMapMode.GetMapUnit(), Point(), aOutMapMode.GetScaleX(), aOutMapMode.GetScaleY() );
    // #106258# Clamp size to 1 for zero values. This is okay, since
    // logical size of zero is handled above already
    const Size      aOutTileSize( ::std::max( 1L, pOut->LogicToPixel( rSize, aOutMapMode ).Width() ),
                                  ::std::max( 1L, pOut->LogicToPixel( rSize, aOutMapMode ).Height() ) );

    //#i69780 clip final tile size to a sane max size
    while (((sal_Int64)rSize.Width() * nTileCacheSize1D) > SAL_MAX_UINT16)
        nTileCacheSize1D /= 2;
    while (((sal_Int64)rSize.Height() * nTileCacheSize1D) > SAL_MAX_UINT16)
        nTileCacheSize1D /= 2;

    return ImplDrawTiled( pOut, rArea, aOutTileSize, rOffset, pAttr, nFlags, nTileCacheSize1D );
}

bool GraphicObject::StartAnimation( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                    long nExtraData, const GraphicAttr* pAttr, sal_uLong /*nFlags*/,
                                    OutputDevice* pFirstFrameOutDev )
{
    bool bRet = false;

    GetGraphic();

    if( !IsSwappedOut() )
    {
        const GraphicAttr aAttr( pAttr ? *pAttr : GetAttr() );

        if( mbAnimated )
        {
            Point   aPt( rPt );
            Size    aSz( rSz );
            bool    bCropped = aAttr.IsCropped();

            if( bCropped )
            {
                PolyPolygon aClipPolyPoly;
                bool        bRectClip;
                const bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

                pOut->Push( PUSH_CLIPREGION );

                if( bCrop )
                {
                    if( bRectClip )
                        pOut->IntersectClipRegion( aClipPolyPoly.GetBoundRect() );
                    else
                        pOut->IntersectClipRegion(Region(aClipPolyPoly));
                }
            }

            if( !mpSimpleCache || ( mpSimpleCache->maAttr != aAttr ) || pFirstFrameOutDev )
            {
                if( mpSimpleCache )
                    delete mpSimpleCache;

                mpSimpleCache = new GrfSimpleCacheObj( GetTransformedGraphic( &aAttr ), aAttr );
                mpSimpleCache->maGraphic.SetAnimationNotifyHdl( GetAnimationNotifyHdl() );
            }

            mpSimpleCache->maGraphic.StartAnimation( pOut, aPt, aSz, nExtraData, pFirstFrameOutDev );

            if( bCropped )
                pOut->Pop();

            bRet = true;
        }
        else
            bRet = Draw( pOut, rPt, rSz, &aAttr, GRFMGR_DRAW_STANDARD );
    }

    return bRet;
}

void GraphicObject::StopAnimation( OutputDevice* pOut, long nExtraData )
{
    if( mpSimpleCache )
        mpSimpleCache->maGraphic.StopAnimation( pOut, nExtraData );
}

const Graphic& GraphicObject::GetGraphic() const
{
    if( mbAutoSwapped )
        ( (GraphicObject*) this )->ImplAutoSwapIn();

    return maGraphic;
}

Graphic GraphicObject::GetTransformedGraphic( const Size& rDestSize, const MapMode& rDestMap, const GraphicAttr& rAttr ) const
{
    // #104550# Extracted from svx/source/svdraw/svdograf.cxx
    Graphic             aTransGraphic( maGraphic );
    const GraphicType   eType = GetType();
    const Size          aSrcSize( aTransGraphic.GetPrefSize() );

    // #104115# Convert the crop margins to graphic object mapmode
    const MapMode aMapGraph( aTransGraphic.GetPrefMapMode() );
    const MapMode aMap100( MAP_100TH_MM );

    Size aCropLeftTop;
    Size aCropRightBottom;

    if( GRAPHIC_GDIMETAFILE == eType )
    {
        GDIMetaFile aMtf( aTransGraphic.GetGDIMetaFile() );

        if( aMapGraph == MAP_PIXEL )
        {
            aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel( Size( rAttr.GetLeftCrop(),
                                                                                rAttr.GetTopCrop() ),
                                                                          aMap100 );
            aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel( Size( rAttr.GetRightCrop(),
                                                                                    rAttr.GetBottomCrop() ),
                                                                              aMap100 );
        }
        else
        {
            aCropLeftTop = OutputDevice::LogicToLogic( Size( rAttr.GetLeftCrop(),
                                                             rAttr.GetTopCrop() ),
                                                       aMap100,
                                                       aMapGraph );
            aCropRightBottom = OutputDevice::LogicToLogic( Size( rAttr.GetRightCrop(),
                                                                 rAttr.GetBottomCrop() ),
                                                           aMap100,
                                                           aMapGraph );
        }

        // #104115# If the metafile is cropped, give it a special
        // treatment: clip against the remaining area, scale up such
        // that this area later fills the desired size, and move the
        // origin to the upper left edge of that area.
        if( rAttr.IsCropped() )
        {
            const MapMode aMtfMapMode( aMtf.GetPrefMapMode() );

            Rectangle aClipRect( aMtfMapMode.GetOrigin().X() + aCropLeftTop.Width(),
                                 aMtfMapMode.GetOrigin().Y() + aCropLeftTop.Height(),
                                 aMtfMapMode.GetOrigin().X() + aSrcSize.Width() - aCropRightBottom.Width(),
                                 aMtfMapMode.GetOrigin().Y() + aSrcSize.Height() - aCropRightBottom.Height() );

            // #104115# To correctly crop rotated metafiles, clip by view rectangle
            aMtf.AddAction( new MetaISectRectClipRegionAction( aClipRect ), 0 );

            // #104115# To crop the metafile, scale larger than the output rectangle
            aMtf.Scale( (double)rDestSize.Width() / (aSrcSize.Width() - aCropLeftTop.Width() - aCropRightBottom.Width()),
                        (double)rDestSize.Height() / (aSrcSize.Height() - aCropLeftTop.Height() - aCropRightBottom.Height()) );

            // #104115# Adapt the pref size by hand (scale changes it
            // proportionally, but we want it to be smaller than the
            // former size, to crop the excess out)
            aMtf.SetPrefSize( Size( (long)((double)rDestSize.Width() *  (1.0 + (aCropLeftTop.Width() + aCropRightBottom.Width()) / aSrcSize.Width())  + .5),
                                    (long)((double)rDestSize.Height() * (1.0 + (aCropLeftTop.Height() + aCropRightBottom.Height()) / aSrcSize.Height()) + .5) ) );

            // #104115# Adapt the origin of the new mapmode, such that it
            // is shifted to the place where the cropped output starts
            Point aNewOrigin( (long)((double)aMtfMapMode.GetOrigin().X() + rDestSize.Width() * aCropLeftTop.Width() / (aSrcSize.Width() - aCropLeftTop.Width() - aCropRightBottom.Width()) + .5),
                              (long)((double)aMtfMapMode.GetOrigin().Y() + rDestSize.Height() * aCropLeftTop.Height() / (aSrcSize.Height() - aCropLeftTop.Height() - aCropRightBottom.Height()) + .5) );
            MapMode aNewMap( rDestMap );
            aNewMap.SetOrigin( OutputDevice::LogicToLogic(aNewOrigin, aMtfMapMode, rDestMap) );
            aMtf.SetPrefMapMode( aNewMap );
        }
        else
        {
            aMtf.Scale( Fraction( rDestSize.Width(), aSrcSize.Width() ), Fraction( rDestSize.Height(), aSrcSize.Height() ) );
            aMtf.SetPrefMapMode( rDestMap );
        }

        aTransGraphic = aMtf;
    }
    else if( GRAPHIC_BITMAP == eType )
    {
        BitmapEx aBitmapEx( aTransGraphic.GetBitmapEx() );
        Rectangle aCropRect;

        // convert crops to pixel (crops are always in GraphicObject units)
        if(rAttr.IsCropped())
        {
            aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel(
                Size(rAttr.GetLeftCrop(), rAttr.GetTopCrop()),
                aMapGraph);
            aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel(
                Size(rAttr.GetRightCrop(), rAttr.GetBottomCrop()),
                aMapGraph);

            // convert from prefmapmode to pixel
            Size aSrcSizePixel(
                Application::GetDefaultDevice()->LogicToPixel(
                    aSrcSize,
                    aMapGraph));

            if(rAttr.IsCropped()
                && (aSrcSizePixel.Width() != aBitmapEx.GetSizePixel().Width() || aSrcSizePixel.Height() != aBitmapEx.GetSizePixel().Height())
                && aSrcSizePixel.Width())
            {
                // the size in pixels calculated from Graphic's internal MapMode (aTransGraphic.GetPrefMapMode())
                // and it's internal size (aTransGraphic.GetPrefSize()) is different from it's real pixel size.
                // This can be interpreted as this values to be set wrong, but needs to be corrected since e.g.
                // existing cropping is calculated based on this logic values already.
                // aBitmapEx.Scale(aSrcSizePixel);

                // another possibility is to adapt the values created so far with a factor; this
                // will keep the original Bitmap untouched and thus quality will not change
                const double fFactorX(aBitmapEx.GetSizePixel().Width() / aSrcSizePixel.Width());
                const double fFactorY(aBitmapEx.GetSizePixel().Height() / aSrcSizePixel.Height());

                aCropLeftTop.Width() = basegfx::fround(aCropLeftTop.Width() * fFactorX);
                aCropLeftTop.Height() = basegfx::fround(aCropLeftTop.Height() * fFactorY);
                aCropRightBottom.Width() = basegfx::fround(aCropRightBottom.Width() * fFactorX);
                aCropRightBottom.Height() = basegfx::fround(aCropRightBottom.Height() * fFactorY);

                aSrcSizePixel = aBitmapEx.GetSizePixel();
            }

            // setup crop rectangle in pixel
            aCropRect = Rectangle( aCropLeftTop.Width(), aCropLeftTop.Height(),
                                 aSrcSizePixel.Width() - aCropRightBottom.Width(),
                                 aSrcSizePixel.Height() - aCropRightBottom.Height() );
        }

        // #105641# Also crop animations
        if( aTransGraphic.IsAnimated() )
        {
            sal_uInt16 nFrame;
            Animation aAnim( aTransGraphic.GetAnimation() );

            for( nFrame=0; nFrame<aAnim.Count(); ++nFrame )
            {
                AnimationBitmap aAnimBmp( aAnim.Get( nFrame ) );

                if( !aCropRect.IsInside( Rectangle(aAnimBmp.aPosPix, aAnimBmp.aSizePix) ) )
                {
                    // setup actual cropping (relative to frame position)
                    Rectangle aCropRectRel( aCropRect );
                    aCropRectRel.Move( -aAnimBmp.aPosPix.X(),
                                       -aAnimBmp.aPosPix.Y() );

                    // cropping affects this frame, apply it then
                    // do _not_ apply enlargement, this is done below
                    ImplTransformBitmap( aAnimBmp.aBmpEx, rAttr, Size(), Size(),
                                         aCropRectRel, rDestSize, false );

                    aAnim.Replace( aAnimBmp, nFrame );
                }
                // else: bitmap completely within crop area,
                // i.e. nothing is cropped away
            }

            // now, apply enlargement (if any) through global animation size
            if( aCropLeftTop.Width() < 0 ||
                aCropLeftTop.Height() < 0 ||
                aCropRightBottom.Width() < 0 ||
                aCropRightBottom.Height() < 0 )
            {
                Size aNewSize( aAnim.GetDisplaySizePixel() );
                aNewSize.Width() += aCropRightBottom.Width() < 0 ? -aCropRightBottom.Width() : 0;
                aNewSize.Width() += aCropLeftTop.Width() < 0 ? -aCropLeftTop.Width() : 0;
                aNewSize.Height() += aCropRightBottom.Height() < 0 ? -aCropRightBottom.Height() : 0;
                aNewSize.Height() += aCropLeftTop.Height() < 0 ? -aCropLeftTop.Height() : 0;
                aAnim.SetDisplaySizePixel( aNewSize );
            }

            // if topleft has changed, we must move all frames to the
            // right and bottom, resp.
            if( aCropLeftTop.Width() < 0 ||
                aCropLeftTop.Height() < 0 )
            {
                Point aPosOffset( aCropLeftTop.Width() < 0 ? -aCropLeftTop.Width() : 0,
                                  aCropLeftTop.Height() < 0 ? -aCropLeftTop.Height() : 0 );

                for( nFrame=0; nFrame<aAnim.Count(); ++nFrame )
                {
                    AnimationBitmap aAnimBmp( aAnim.Get( nFrame ) );

                    aAnimBmp.aPosPix += aPosOffset;

                    aAnim.Replace( aAnimBmp, nFrame );
                }
            }

            aTransGraphic = aAnim;
        }
        else
        {
            ImplTransformBitmap( aBitmapEx, rAttr, aCropLeftTop, aCropRightBottom,
                                 aCropRect, rDestSize, true );

            aTransGraphic = aBitmapEx;
        }

        aTransGraphic.SetPrefSize( rDestSize );
        aTransGraphic.SetPrefMapMode( rDestMap );
    }

    GraphicObject aGrfObj( aTransGraphic );
    aTransGraphic = aGrfObj.GetTransformedGraphic( &rAttr );

    return aTransGraphic;
}

Graphic GraphicObject::GetTransformedGraphic( const GraphicAttr* pAttr ) const // TODO: Change to Impl
{
    GetGraphic();

    Graphic     aGraphic;
    GraphicAttr aAttr( pAttr ? *pAttr : GetAttr() );

    if( maGraphic.IsSupportedGraphic() && !maGraphic.IsSwapOut() )
    {
        if( aAttr.IsSpecialDrawMode() || aAttr.IsAdjusted() || aAttr.IsMirrored() || aAttr.IsRotated() || aAttr.IsTransparent() )
        {
            if( GetType() == GRAPHIC_BITMAP )
            {
                if( IsAnimated() )
                {
                    Animation aAnimation( maGraphic.GetAnimation() );
                    GraphicManager::Adjust( aAnimation, aAttr, ADJUSTMENT_ALL );
                    aAnimation.SetLoopCount( mnAnimationLoopCount );
                    aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( maGraphic.GetBitmapEx() );
                    GraphicManager::Adjust( aBmpEx, aAttr, ADJUSTMENT_ALL );
                    aGraphic = aBmpEx;
                }
            }
            else
            {
                GDIMetaFile aMtf( maGraphic.GetGDIMetaFile() );
                GraphicManager::Adjust( aMtf, aAttr, ADJUSTMENT_ALL );
                aGraphic = aMtf;
            }
        }
        else
        {
            if( ( GetType() == GRAPHIC_BITMAP ) && IsAnimated() )
            {
                Animation aAnimation( maGraphic.GetAnimation() );
                aAnimation.SetLoopCount( mnAnimationLoopCount );
                aGraphic = aAnimation;
            }
            else
                aGraphic = maGraphic;
        }
    }

    return aGraphic;
}

bool GraphicObject::SwapOut()
{
    bool bRet = ( !mbAutoSwapped ? maGraphic.SwapOut() : false );

    if( bRet )
    {
        GraphicManager::GetGlobalManager()->GraphicObjectWasSwappedOut( this );
    }
    return bRet;
}

bool GraphicObject::SwapOut( SvStream* pOStm )
{
    bool bRet = ( !mbAutoSwapped ? maGraphic.SwapOut( pOStm ) : false );

    if( bRet )
    {
        GraphicManager::GetGlobalManager()->GraphicObjectWasSwappedOut( this );
    }

    return bRet;
}

bool GraphicObject::SwapIn()
{
    bool bRet;

    if( mbAutoSwapped )
    {
        ImplAutoSwapIn();
        bRet = true;
    }
    else if( GraphicManager::GetGlobalManager()->FillSwappedGraphicObject( this, maGraphic ) )
    {
        bRet = true;
    }
    else
    {
        bRet = maGraphic.SwapIn();

        if( bRet)
        {
            GraphicManager::GetGlobalManager()->GraphicObjectWasSwappedIn( this );
        }
    }

    if( bRet )
        ImplAssignGraphicData();

    return bRet;
}

void GraphicObject::SetSwapState()
{
    if( !IsSwappedOut() )
    {
        mbAutoSwapped = true;

        GraphicManager::GetGlobalManager()->GraphicObjectWasSwappedOut( this );
    }
}

IMPL_LINK_NOARG(GraphicObject, ImplAutoSwapOutHdl)
{
    if( !IsSwappedOut() )
    {
        mbIsInSwapOut = true;

        SvStream* pStream = GetSwapStream();

        if( GRFMGR_AUTOSWAPSTREAM_NONE != pStream )
        {
            if( GRFMGR_AUTOSWAPSTREAM_LINK == pStream )
                mbAutoSwapped = SwapOut( NULL );
            else
            {
                if( GRFMGR_AUTOSWAPSTREAM_TEMP == pStream )
                    mbAutoSwapped = SwapOut();
                else
                {
                    mbAutoSwapped = SwapOut( pStream );
                    delete pStream;
                }
            }
        }

        mbIsInSwapOut = false;
    }

    if( mpSwapOutTimer )
        mpSwapOutTimer->Start();

    return 0L;
}

#if 0
SvStream& operator>>( SvStream& rIStm, GraphicObject& rGraphicObj )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    Graphic         aGraphic;
    GraphicAttr     aAttr;
    sal_Bool        bLink;

    rIStm >> aGraphic >> aAttr >> bLink;

    rGraphicObj.SetGraphic( aGraphic );
    rGraphicObj.SetAttr( aAttr );

    if( bLink )
    {
        OUString aLink = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStm, RTL_TEXTENCODING_UTF8);
        rGraphicObj.SetLink(aLink);
    }
    else
        rGraphicObj.SetLink();

    rGraphicObj.SetSwapStreamHdl();

    return rIStm;
}

SvStream& operator<<( SvStream& rOStm, const GraphicObject& rGraphicObj )
{
    VersionCompat   aCompat( rOStm, STREAM_WRITE, 1 );
    const sal_Bool  bLink =  rGraphicObj.HasLink() ? sal_True : sal_False;

    rOStm << rGraphicObj.GetGraphic() << rGraphicObj.GetAttr() << bLink;

    if( bLink )
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStm, rGraphicObj.GetLink(), RTL_TEXTENCODING_UTF8);

    return rOStm;
}
#endif

#define UNO_NAME_GRAPHOBJ_URLPREFIX "vnd.sun.star.GraphicObject:"

rtl::Reference<GraphicObject> GraphicObject::CreateGraphicObjectFromURL( const OUString &rURL )
{
    const OUString aURL( rURL );
    const OUString aPrefix(UNO_NAME_GRAPHOBJ_URLPREFIX);
    if( aURL.indexOf( aPrefix ) == 0 )
    {
        // graphic manager url
        OString aUniqueID(OUStringToOString(rURL.copy(sizeof(UNO_NAME_GRAPHOBJ_URLPREFIX) - 1), RTL_TEXTENCODING_UTF8));
        return GraphicObject::Create( aUniqueID );
    }
    else
    {
        Graphic     aGraphic;
        if ( !aURL.isEmpty() )
        {
            SvStream*   pStream = utl::UcbStreamHelper::CreateStream( aURL, STREAM_READ );
            if( pStream )
            {
                GraphicConverter::Import( *pStream, aGraphic );
            }
        }

        return GraphicObject::Create( aGraphic );
    }
}

void GraphicObject::InspectForGraphicObjectImageURL( const Reference< XInterface >& xIf,  std::vector< OUString >& rvEmbedImgUrls )
{
    static OUString sImageURL( "ImageURL" );
    Reference< XPropertySet > xProps( xIf, UNO_QUERY );
    if ( xProps.is() )
    {

        if ( xProps->getPropertySetInfo()->hasPropertyByName( sImageURL ) )
        {
            OUString sURL;
            xProps->getPropertyValue( sImageURL ) >>= sURL;
            if ( !sURL.isEmpty() && sURL.startsWith( UNO_NAME_GRAPHOBJ_URLPREFIX ) )
                rvEmbedImgUrls.push_back( sURL );
        }
    }
    Reference< XNameContainer > xContainer( xIf, UNO_QUERY );
    if ( xContainer.is() )
    {
        Sequence< OUString > sNames = xContainer->getElementNames();
        sal_Int32 nContainees = sNames.getLength();
        for ( sal_Int32 index = 0; index < nContainees; ++index )
        {
            Reference< XInterface > xCtrl;
            xContainer->getByName( sNames[ index ] ) >>= xCtrl;
            InspectForGraphicObjectImageURL( xCtrl, rvEmbedImgUrls );
        }
    }
}

// calculate scalings between real image size and logic object size. This
// is necessary since the crop values are relative to original bitmap size
basegfx::B2DVector GraphicObject::calculateCropScaling(
    double fWidth,
    double fHeight,
    double fLeftCrop,
    double fTopCrop,
    double fRightCrop,
    double fBottomCrop) const
{
    const MapMode aMapMode100thmm(MAP_100TH_MM);
    Size aBitmapSize(GetPrefSize());
    double fFactorX(1.0);
    double fFactorY(1.0);

    if(MAP_PIXEL == GetPrefMapMode().GetMapUnit())
    {
        aBitmapSize = Application::GetDefaultDevice()->PixelToLogic(aBitmapSize, aMapMode100thmm);
    }
    else
    {
        aBitmapSize = Application::GetDefaultDevice()->LogicToLogic(aBitmapSize, GetPrefMapMode(), aMapMode100thmm);
    }

    const double fDivX(aBitmapSize.Width() - fLeftCrop - fRightCrop);
    const double fDivY(aBitmapSize.Height() - fTopCrop - fBottomCrop);

    if(!basegfx::fTools::equalZero(fDivX))
    {
        fFactorX = fabs(fWidth) / fDivX;
    }

    if(!basegfx::fTools::equalZero(fDivY))
    {
        fFactorY = fabs(fHeight) / fDivY;
    }

    return basegfx::B2DVector(fFactorX,fFactorY);
}

bool GraphicObject::ImplRenderTempTile( VirtualDevice& rVDev, int nExponent,
                                        int nNumTilesX, int nNumTilesY,
                                        const Size& rTileSizePixel,
                                        const GraphicAttr* pAttr, sal_uLong nFlags )
{
    if( nExponent <= 1 )
        return false;

    // determine MSB factor
    int nMSBFactor( 1 );
    while( nNumTilesX / nMSBFactor != 0 ||
           nNumTilesY / nMSBFactor != 0 )
    {
        nMSBFactor *= nExponent;
    }

    // one less
    nMSBFactor /= nExponent;

    ImplTileInfo aTileInfo;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    bool bOldMap( rVDev.IsMapModeEnabled() );
    rVDev.EnableMapMode( false );

    bool bRet( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor, nNumTilesX, nNumTilesY,
                                        nNumTilesX, nNumTilesY, rTileSizePixel, pAttr, nFlags, aTileInfo ) );

    rVDev.EnableMapMode( bOldMap );

    return bRet;
}

// define for debug drawings
//#define DBG_TEST

// see header comment. this works similar to base conversion of a
// number, i.e. if the exponent is 10, then the number for every tile
// size is given by the decimal place of the corresponding decimal
// representation.
bool GraphicObject::ImplRenderTileRecursive( VirtualDevice& rVDev, int nExponent, int nMSBFactor,
                                             int nNumOrigTilesX, int nNumOrigTilesY,
                                             int nRemainderTilesX, int nRemainderTilesY,
                                             const Size& rTileSizePixel, const GraphicAttr* pAttr,
                                             sal_uLong nFlags, ImplTileInfo& rTileInfo )
{
    // gets loaded with our tile bitmap
    rtl::Reference<GraphicObject> aTmpGraphic = GraphicObject::Create(*this);

    // stores a flag that renders the zero'th tile position
    // (i.e. (0,0)+rCurrPos) only if we're at the bottom of the
    // recursion stack. All other position already have that tile
    // rendered, because the lower levels painted their generated tile
    // there.
    bool bNoFirstTileDraw( false );

    // what's left when we're done with our tile size
    const int nNewRemainderX( nRemainderTilesX % nMSBFactor );
    const int nNewRemainderY( nRemainderTilesY % nMSBFactor );

    // gets filled out from the recursive call with info of what's
    // been generated
    ImplTileInfo aTileInfo;

    // current output position while drawing
    Point aCurrPos;
    int nX, nY;

    // check for recursion's end condition: LSB place reached?
    if( nMSBFactor == 1 )
    {
        // set initial tile size -> orig size
        aTileInfo.aTileSizePixel = rTileSizePixel;
        aTileInfo.nTilesEmptyX = nNumOrigTilesX;
        aTileInfo.nTilesEmptyY = nNumOrigTilesY;
    }
    else if( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor/nExponent,
                                      nNumOrigTilesX, nNumOrigTilesY,
                                      nNewRemainderX, nNewRemainderY,
                                      rTileSizePixel, pAttr, nFlags, aTileInfo ) )
    {
        // extract generated tile -> see comment on the first loop below
        BitmapEx aTileBitmap( rVDev.GetBitmap( aTileInfo.aTileTopLeft, aTileInfo.aTileSizePixel ) );

        aTmpGraphic = GraphicObject::Create( aTileBitmap );

        // fill stripes left over from upstream levels:
        //
        //  x0000
        //  0
        //  0
        //  0
        //  0
        //
        // where x denotes the place filled by our recursive predecessors

        // check whether we have to fill stripes here. Although not
        // obvious, there is one case where we can skip this step: if
        // the previous recursion level (the one who filled our
        // aTileInfo) had zero area to fill, then there are no white
        // stripes left, naturally. This happens if the digit
        // associated to that level has a zero, and can be checked via
        // aTileTopLeft==aNextTileTopLeft.
        if( aTileInfo.aTileTopLeft != aTileInfo.aNextTileTopLeft )
        {
            // now fill one row from aTileInfo.aNextTileTopLeft.X() all
            // the way to the right
            aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aTileTopLeft.Y();
            for( nX=0; nX < aTileInfo.nTilesEmptyX; nX += nMSBFactor )
            {
                if( !aTmpGraphic->Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
            }

#ifdef DBG_TEST
//          rVDev.SetFillColor( COL_WHITE );
            rVDev.SetFillColor();
            rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
            rVDev.DrawEllipse( Rectangle(aTileInfo.aNextTileTopLeft.X(), aTileInfo.aTileTopLeft.Y(),
                                         aTileInfo.aNextTileTopLeft.X() - 1 + (aTileInfo.nTilesEmptyX/nMSBFactor)*aTileInfo.aTileSizePixel.Width(),
                                         aTileInfo.aTileTopLeft.Y() + aTileInfo.aTileSizePixel.Height() - 1) );
#endif

            // now fill one column from aTileInfo.aNextTileTopLeft.Y() all
            // the way to the bottom
            aCurrPos.X() = aTileInfo.aTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aNextTileTopLeft.Y();
            for( nY=0; nY < aTileInfo.nTilesEmptyY; nY += nMSBFactor )
            {
                if( !aTmpGraphic->Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
            }

#ifdef DBG_TEST
            rVDev.DrawEllipse( Rectangle(aTileInfo.aTileTopLeft.X(), aTileInfo.aNextTileTopLeft.Y(),
                                         aTileInfo.aTileTopLeft.X() + aTileInfo.aTileSizePixel.Width() - 1,
                                         aTileInfo.aNextTileTopLeft.Y() - 1 + (aTileInfo.nTilesEmptyY/nMSBFactor)*aTileInfo.aTileSizePixel.Height()) );
#endif
        }
        else
        {
            // Thought that aTileInfo.aNextTileTopLeft tile has always
            // been drawn already, but that's wrong: typically,
            // _parts_ of that tile have been drawn, since the
            // previous level generated the tile there. But when
            // aTileInfo.aNextTileTopLeft!=aTileInfo.aTileTopLeft, the
            // difference between these two values is missing in the
            // lower right corner of this first tile. So, can do that
            // only here.
            bNoFirstTileDraw = true;
        }
    }
    else
    {
        return false;
    }

    // calc number of original tiles in our drawing area without
    // remainder
    nRemainderTilesX -= nNewRemainderX;
    nRemainderTilesY -= nNewRemainderY;

    // fill tile info for calling method
    rTileInfo.aTileTopLeft     = aTileInfo.aNextTileTopLeft;
    rTileInfo.aNextTileTopLeft = Point( rTileInfo.aTileTopLeft.X() + rTileSizePixel.Width()*nRemainderTilesX,
                                        rTileInfo.aTileTopLeft.Y() + rTileSizePixel.Height()*nRemainderTilesY );
    rTileInfo.aTileSizePixel   = Size( rTileSizePixel.Width()*nMSBFactor*nExponent,
                                       rTileSizePixel.Height()*nMSBFactor*nExponent );
    rTileInfo.nTilesEmptyX     = aTileInfo.nTilesEmptyX - nRemainderTilesX;
    rTileInfo.nTilesEmptyY     = aTileInfo.nTilesEmptyY - nRemainderTilesY;

    // init output position
    aCurrPos = aTileInfo.aNextTileTopLeft;

    // fill our drawing area. Fill possibly more, to create the next
    // bigger tile size -> see bitmap extraction above. This does no
    // harm, since everything right or below our actual area is
    // overdrawn by our caller. Just in case we're in the last level,
    // we don't draw beyond the right or bottom border.
    for( nY=0; nY < aTileInfo.nTilesEmptyY && nY < nExponent*nMSBFactor; nY += nMSBFactor )
    {
        aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();

        for( nX=0; nX < aTileInfo.nTilesEmptyX && nX < nExponent*nMSBFactor; nX += nMSBFactor )
        {
            if( bNoFirstTileDraw )
                bNoFirstTileDraw = false; // don't draw first tile position
            else if( !aTmpGraphic->Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                return false;

            aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
        }

        aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
    }

#ifdef DBG_TEST
//  rVDev.SetFillColor( COL_WHITE );
    rVDev.SetFillColor();
    rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
    rVDev.DrawRect( Rectangle((rTileInfo.aTileTopLeft.X())*rTileSizePixel.Width(),
                              (rTileInfo.aTileTopLeft.Y())*rTileSizePixel.Height(),
                              (rTileInfo.aNextTileTopLeft.X())*rTileSizePixel.Width()-1,
                              (rTileInfo.aNextTileTopLeft.Y())*rTileSizePixel.Height()-1) );
#endif

    return true;
}

bool GraphicObject::ImplDrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSizePixel,
                                   const Size& rOffset, const GraphicAttr* pAttr, sal_uLong nFlags, int nTileCacheSize1D )
{
    // how many tiles to generate per recursion step
    enum{ SubdivisionExponent=2 };

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    const MapMode   aMapMode( aOutMapMode.GetMapUnit(), Point(), aOutMapMode.GetScaleX(), aOutMapMode.GetScaleY() );
    bool            bRet( false );

    // #i42643# Casting to Int64, to avoid integer overflow for
    // huge-DPI output devices
    if( GetGraphic().GetType() == GRAPHIC_BITMAP &&
        static_cast<sal_Int64>(rSizePixel.Width()) * rSizePixel.Height() <
        static_cast<sal_Int64>(nTileCacheSize1D)*nTileCacheSize1D )
    {
        // First combine very small bitmaps into a larger tile
        // ===================================================

        VirtualDevice   aVDev;
        const int       nNumTilesInCacheX( (nTileCacheSize1D + rSizePixel.Width()-1) / rSizePixel.Width() );
        const int       nNumTilesInCacheY( (nTileCacheSize1D + rSizePixel.Height()-1) / rSizePixel.Height() );

        aVDev.SetOutputSizePixel( Size( nNumTilesInCacheX*rSizePixel.Width(),
                                        nNumTilesInCacheY*rSizePixel.Height() ) );
        aVDev.SetMapMode( aMapMode );

        // draw bitmap content
        if( ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
        {
            BitmapEx aTileBitmap( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) );

            // draw alpha content, if any
            if( IsTransparent() )
            {
                rtl::Reference<GraphicObject> aAlphaGraphic;

                if( GetGraphic().IsAlpha() )
                {
                    aAlphaGraphic = GraphicObject::Create( GetGraphic().GetBitmapEx().GetAlpha().GetBitmap() );
                }
                else
                {
                    aAlphaGraphic = GraphicObject::Create( GetGraphic().GetBitmapEx().GetMask() );
                }
                if( aAlphaGraphic->ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                                      nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
                {
                    // Combine bitmap and alpha/mask
                    if( GetGraphic().IsAlpha() )
                    {
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                AlphaMask( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) ) );
                    }
                    else
                    {
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ).CreateMask( Color(COL_WHITE) ) );
                    }
                }
            }

            // paint generated tile
            rtl::Reference<GraphicObject> aTmpGraphic = GraphicObject::Create(aTileBitmap);
            bRet = aTmpGraphic->ImplDrawTiled( pOut, rArea,
                                               aTileBitmap.GetSizePixel(),
                                               rOffset, pAttr, nFlags, nTileCacheSize1D );
        }
    }
    else
    {
        const Size      aOutOffset( pOut->LogicToPixel( rOffset, aOutMapMode ) );
        const Rectangle aOutArea( pOut->LogicToPixel( rArea, aOutMapMode ) );

        // number of invisible (because out-of-area) tiles
        int nInvisibleTilesX;
        int nInvisibleTilesY;

        // round towards -infty for negative offset
        if( aOutOffset.Width() < 0 )
            nInvisibleTilesX = (aOutOffset.Width() - rSizePixel.Width() + 1) / rSizePixel.Width();
        else
            nInvisibleTilesX = aOutOffset.Width() / rSizePixel.Width();

        // round towards -infty for negative offset
        if( aOutOffset.Height() < 0 )
            nInvisibleTilesY = (aOutOffset.Height() - rSizePixel.Height() + 1) / rSizePixel.Height();
        else
            nInvisibleTilesY = aOutOffset.Height() / rSizePixel.Height();

        // origin from where to 'virtually' start drawing in pixel
        const Point aOutOrigin( pOut->LogicToPixel( Point( rArea.Left() - rOffset.Width(),
                                                           rArea.Top() - rOffset.Height() ) ) );
        // position in pixel from where to really start output
        const Point aOutStart( aOutOrigin.X() + nInvisibleTilesX*rSizePixel.Width(),
                               aOutOrigin.Y() + nInvisibleTilesY*rSizePixel.Height() );

        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rArea );

        // Paint all tiles
        // ===============

        bRet = ImplDrawTiled( *pOut, aOutStart,
                              (aOutArea.GetWidth() + aOutArea.Left() - aOutStart.X() + rSizePixel.Width() - 1) / rSizePixel.Width(),
                              (aOutArea.GetHeight() + aOutArea.Top() - aOutStart.Y() + rSizePixel.Height() - 1) / rSizePixel.Height(),
                              rSizePixel, pAttr, nFlags );

        pOut->Pop();
    }

    return bRet;
}

bool GraphicObject::ImplDrawTiled( OutputDevice& rOut, const Point& rPosPixel,
                                   int nNumTilesX, int nNumTilesY,
                                   const Size& rTileSizePixel, const GraphicAttr* pAttr, sal_uLong nFlags )
{
    Point   aCurrPos( rPosPixel );
    Size    aTileSizeLogic( rOut.PixelToLogic( rTileSizePixel ) );
    int     nX, nY;

    // #107607# Use logical coordinates for metafile playing, too
    bool    bDrawInPixel( rOut.GetConnectMetaFile() == NULL && GRAPHIC_BITMAP == GetType() );
    bool    bRet = false;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    bool bOldMap( rOut.IsMapModeEnabled() );

    if( bDrawInPixel )
        rOut.EnableMapMode( false );

    for( nY=0; nY < nNumTilesY; ++nY )
    {
        aCurrPos.X() = rPosPixel.X();

        for( nX=0; nX < nNumTilesX; ++nX )
        {
            // #105229# work with pixel coordinates here, mapping is disabled!
            // #104004# don't disable mapping for metafile recordings
            // #108412# don't quit the loop if one draw fails

            // update return value. This method should return true, if
            // at least one of the looped Draws succeeded.
            bRet |= Draw( &rOut,
                          bDrawInPixel ? aCurrPos : rOut.PixelToLogic( aCurrPos ),
                          bDrawInPixel ? rTileSizePixel : aTileSizeLogic,
                          pAttr, nFlags );

            aCurrPos.X() += rTileSizePixel.Width();
        }

        aCurrPos.Y() += rTileSizePixel.Height();
    }

    if( bDrawInPixel )
        rOut.EnableMapMode( bOldMap );

    return bRet;
}

void GraphicObject::ImplTransformBitmap( BitmapEx&          rBmpEx,
                                         const GraphicAttr& rAttr,
                                         const Size&        rCropLeftTop,
                                         const Size&        rCropRightBottom,
                                         const Rectangle&   rCropRect,
                                         const Size&        rDstSize,
                                         bool               bEnlarge ) const
{
    // #107947# Extracted from svdograf.cxx

    // #104115# Crop the bitmap
    if( rAttr.IsCropped() )
    {
        rBmpEx.Crop( rCropRect );

        // #104115# Negative crop sizes mean: enlarge bitmap and pad
        if( bEnlarge && (
            rCropLeftTop.Width() < 0 ||
            rCropLeftTop.Height() < 0 ||
            rCropRightBottom.Width() < 0 ||
            rCropRightBottom.Height() < 0 ) )
        {
            Size aBmpSize( rBmpEx.GetSizePixel() );
            sal_Int32 nPadLeft( rCropLeftTop.Width() < 0 ? -rCropLeftTop.Width() : 0 );
            sal_Int32 nPadTop( rCropLeftTop.Height() < 0 ? -rCropLeftTop.Height() : 0 );
            sal_Int32 nPadTotalWidth( aBmpSize.Width() + nPadLeft + (rCropRightBottom.Width() < 0 ? -rCropRightBottom.Width() : 0) );
            sal_Int32 nPadTotalHeight( aBmpSize.Height() + nPadTop + (rCropRightBottom.Height() < 0 ? -rCropRightBottom.Height() : 0) );

            BitmapEx aBmpEx2;

            if( rBmpEx.IsTransparent() )
            {
                if( rBmpEx.IsAlpha() )
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetAlpha() );
                else
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetMask() );
            }
            else
            {
                // #104115# Generate mask bitmap and init to zero
                Bitmap aMask( aBmpSize, 1 );
                aMask.Erase( Color(0,0,0) );

                // #104115# Always generate transparent bitmap, we need the border transparent
                aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), aMask );

                // #104115# Add opaque mask to source bitmap, otherwise the destination remains transparent
                rBmpEx = aBmpEx2;
            }

            aBmpEx2.SetSizePixel( Size(nPadTotalWidth, nPadTotalHeight) );
            aBmpEx2.Erase( Color(0xFF,0,0,0) );
            aBmpEx2.CopyPixel( Rectangle( Point(nPadLeft, nPadTop), aBmpSize ), Rectangle( Point(0, 0), aBmpSize ), &rBmpEx );
            rBmpEx = aBmpEx2;
        }
    }

    const Size  aSizePixel( rBmpEx.GetSizePixel() );

    if( rAttr.GetRotation() != 0 && !IsAnimated() )
    {
        if( aSizePixel.Width() && aSizePixel.Height() && rDstSize.Width() && rDstSize.Height() )
        {
            double fSrcWH = (double) aSizePixel.Width() / aSizePixel.Height();
            double fDstWH = (double) rDstSize.Width() / rDstSize.Height();
            double fScaleX = 1.0, fScaleY = 1.0;

            // always choose scaling to shrink bitmap
            if( fSrcWH < fDstWH )
                fScaleY = aSizePixel.Width() / ( fDstWH * aSizePixel.Height() );
            else
                fScaleX = fDstWH * aSizePixel.Height() / aSizePixel.Width();

            rBmpEx.Scale( fScaleX, fScaleY );
        }
    }
}

#if 0
namespace unographic {
    namespace sdecl = comphelper::service_decl;
    sdecl::class_<GraphicObject, sdecl::with_args<true> > serviceBI;
    extern sdecl::ServiceDecl const serviceDecl( serviceBI,
                                                 "com.sun.star.graphic.GraphicObject",
                                                 "com.sun.star.graphic.GraphicObject" );
}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
