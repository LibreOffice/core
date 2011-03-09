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

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <algorithm>

#include <tools/vcompat.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/salbtype.hxx>
#include <unotools/cacheoptions.hxx>
#include <svtools/grfmgr.hxx>

// --> OD 2010-01-04 #i105243#
#include <vcl/pdfextoutdevdata.hxx>
// <--

// -----------
// - Defines -
// -----------

#define WATERMARK_LUM_OFFSET                50
#define WATERMARK_CON_OFFSET                -70

// -----------
// - statics -
// -----------

GraphicManager* GraphicObject::mpGlobalMgr = NULL;

// ---------------------
// - GrfDirectCacheObj -
// ---------------------

struct GrfSimpleCacheObj
{
    Graphic     maGraphic;
    GraphicAttr maAttr;

                GrfSimpleCacheObj( const Graphic& rGraphic, const GraphicAttr& rAttr ) :
                    maGraphic( rGraphic ), maAttr( rAttr ) {}
};

// -----------------
// - GraphicObject -
// -----------------

TYPEINIT1_AUTOFACTORY( GraphicObject, SvDataCopyStream );

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const GraphicManager* pMgr ) :
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr ) :
    maGraphic   ( rGraphic ),
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const Graphic& rGraphic, const String& rLink, const GraphicManager* pMgr ) :
    maGraphic   ( rGraphic ),
    mpLink      ( rLink.Len() ? ( new String( rLink ) ) : NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const GraphicObject& rGraphicObj, const GraphicManager* pMgr ) :
    SvDataCopyStream(),
    maGraphic   ( rGraphicObj.GetGraphic() ),
    maAttr      ( rGraphicObj.maAttr ),
    mpLink      ( rGraphicObj.mpLink ? ( new String( *rGraphicObj.mpLink ) ) : NULL ),
    mpUserData  ( rGraphicObj.mpUserData ? ( new String( *rGraphicObj.mpUserData ) ) : NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr, NULL, &rGraphicObj );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const ByteString& rUniqueID, const GraphicManager* pMgr ) :
    mpLink      ( NULL ),
    mpUserData  ( NULL )
{
    ImplConstruct();

    // assign default properties
    ImplAssignGraphicData();

    ImplSetGraphicManager( pMgr, &rUniqueID );

    // update properties
    ImplAssignGraphicData();
}

// -----------------------------------------------------------------------------

GraphicObject::~GraphicObject()
{
    if( mpMgr )
    {
        mpMgr->ImplUnregisterObj( *this );

        if( ( mpMgr == mpGlobalMgr ) && !mpGlobalMgr->ImplHasObjects() )
            delete mpGlobalMgr, mpGlobalMgr = NULL;
    }

    delete mpSwapOutTimer;
    delete mpSwapStreamHdl;
    delete mpLink;
    delete mpUserData;
    delete mpSimpleCache;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplConstruct()
{
    mpMgr = NULL;
    mpSwapStreamHdl = NULL;
    mpSwapOutTimer = NULL;
    mpSimpleCache = NULL;
    mnAnimationLoopCount = 0;
    mbAutoSwapped = sal_False;
    mbIsInSwapIn = sal_False;
    mbIsInSwapOut = sal_False;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplAssignGraphicData()
{
    maPrefSize = maGraphic.GetPrefSize();
    maPrefMapMode = maGraphic.GetPrefMapMode();
    mnSizeBytes = maGraphic.GetSizeBytes();
    meType = maGraphic.GetType();
    mbTransparent = maGraphic.IsTransparent();
    mbAlpha = maGraphic.IsAlpha();
    mbAnimated = maGraphic.IsAnimated();
    mnAnimationLoopCount = ( mbAnimated ? maGraphic.GetAnimationLoopCount() : 0 );

    if( maGraphic.GetType() == GRAPHIC_GDIMETAFILE )
    {
        const GDIMetaFile& rMtf = GetGraphic().GetGDIMetaFile();
        mbEPS = ( rMtf.GetActionCount() >= 1 ) && ( META_EPS_ACTION == rMtf.GetAction( 0 )->GetType() );
    }
    else
        mbEPS = sal_False;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplSetGraphicManager( const GraphicManager* pMgr, const ByteString* pID, const GraphicObject* pCopyObj )
{
    if( !mpMgr || ( pMgr != mpMgr ) )
    {
        if( !pMgr && mpMgr && ( mpMgr == mpGlobalMgr ) )
            return;
        else
        {
            if( mpMgr )
            {
                mpMgr->ImplUnregisterObj( *this );

                if( ( mpMgr == mpGlobalMgr ) && !mpGlobalMgr->ImplHasObjects() )
                    delete mpGlobalMgr, mpGlobalMgr = NULL;
            }

            if( !pMgr )
            {
                if( !mpGlobalMgr )
                {
                    SvtCacheOptions aCacheOptions;

                    mpGlobalMgr = new GraphicManager( aCacheOptions.GetGraphicManagerTotalCacheSize(),
                                                      aCacheOptions.GetGraphicManagerObjectCacheSize() );
                    mpGlobalMgr->SetCacheTimeout( aCacheOptions.GetGraphicManagerObjectReleaseTime() );
                }

                mpMgr = mpGlobalMgr;
            }
            else
                mpMgr = (GraphicManager*) pMgr;

            mpMgr->ImplRegisterObj( *this, maGraphic, pID, pCopyObj );
        }
    }
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplAutoSwapIn()
{
    if( IsSwappedOut() )
    {
        if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
            mbAutoSwapped = sal_False;
        else
        {
            mbIsInSwapIn = sal_True;

            if( maGraphic.SwapIn() )
                mbAutoSwapped = sal_False;
            else
            {
                SvStream* pStream = GetSwapStream();

                if( GRFMGR_AUTOSWAPSTREAM_NONE != pStream )
                {
                    if( GRFMGR_AUTOSWAPSTREAM_LINK == pStream )
                    {
                        if( HasLink() )
                        {
                            String aURLStr;

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

            mbIsInSwapIn = sal_False;

            if( !mbAutoSwapped && mpMgr )
                mpMgr->ImplGraphicObjectWasSwappedIn( *this );
        }
    }
}

// -----------------------------------------------------------------------------
sal_Bool GraphicObject::ImplGetCropParams( OutputDevice* pOut, Point& rPt, Size& rSz, const GraphicAttr* pAttr,
                                       PolyPolygon& rClipPolyPoly, sal_Bool& bRectClipRegion ) const
{
    sal_Bool bRet = sal_False;

    if( GetType() != GRAPHIC_NONE )
    {
        Polygon         aClipPoly( Rectangle( rPt, rSz ) );
        const sal_uInt16    nRot10 = pAttr->GetRotation() % 3600;
        const Point     aOldOrigin( rPt );
        // --> OD 2005-09-30 #i54875# - It's not needed to get the graphic again.
//        const Graphic&  rGraphic = GetGraphic();
        // <--
        const MapMode   aMap100( MAP_100TH_MM );
        Size            aSize100;
        long            nTotalWidth, nTotalHeight;
        long            nNewLeft, nNewTop, nNewRight, nNewBottom;
        double          fScale;

        if( nRot10 )
        {
            aClipPoly.Rotate( rPt, nRot10 );
            bRectClipRegion = sal_False;
        }
        else
            bRectClipRegion = sal_True;

        rClipPolyPoly = aClipPoly;

        // --> OD 2005-09-30 #i54875# - directly access member <maGraphic> to
        // get <PrefSize> and <PrefMapMode>.
//        if( rGraphic.GetPrefMapMode() == MAP_PIXEL )
//            aSize100 = Application::GetDefaultDevice()->PixelToLogic( rGraphic.GetPrefSize(), aMap100 );
//        else
//            aSize100 = pOut->LogicToLogic( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode(), aMap100 );
        if( maGraphic.GetPrefMapMode() == MAP_PIXEL )
            aSize100 = Application::GetDefaultDevice()->PixelToLogic( maGraphic.GetPrefSize(), aMap100 );
        else
        {
            MapMode m(maGraphic.GetPrefMapMode());
            aSize100 = pOut->LogicToLogic( maGraphic.GetPrefSize(), &m, &aMap100 );
        }
        // <--

        nTotalWidth = aSize100.Width() - pAttr->GetLeftCrop() - pAttr->GetRightCrop();
        nTotalHeight = aSize100.Height() - pAttr->GetTopCrop() - pAttr->GetBottomCrop();

        if( aSize100.Width() > 0 && aSize100.Height() > 0 && nTotalWidth > 0 && nTotalHeight > 0 )
        {
            fScale = (double) aSize100.Width() / nTotalWidth;
            nNewLeft = -FRound( ( ( pAttr->GetMirrorFlags() & BMP_MIRROR_HORZ ) ? pAttr->GetRightCrop() : pAttr->GetLeftCrop() ) * fScale );
            nNewRight = nNewLeft + FRound( aSize100.Width() * fScale ) - 1;

            fScale = (double) rSz.Width() / aSize100.Width();
            rPt.X() += FRound( nNewLeft * fScale );
            rSz.Width() = FRound( ( nNewRight - nNewLeft + 1 ) * fScale );

            fScale = (double) aSize100.Height() / nTotalHeight;
            nNewTop = -FRound( ( ( pAttr->GetMirrorFlags() & BMP_MIRROR_VERT ) ? pAttr->GetBottomCrop() : pAttr->GetTopCrop() ) * fScale );
            nNewBottom = nNewTop + FRound( aSize100.Height() * fScale ) - 1;

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

            bRet = sal_True;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

GraphicObject& GraphicObject::operator=( const GraphicObject& rGraphicObj )
{
    if( &rGraphicObj != this )
    {
        mpMgr->ImplUnregisterObj( *this );

        delete mpSwapStreamHdl, mpSwapStreamHdl = NULL;
        delete mpSimpleCache, mpSimpleCache = NULL;
        delete mpLink;
        delete mpUserData;

        maGraphic = rGraphicObj.GetGraphic();
        maAttr = rGraphicObj.maAttr;
        mpLink = rGraphicObj.mpLink ? new String( *rGraphicObj.mpLink ) : NULL;
        mpUserData = rGraphicObj.mpUserData ? new String( *rGraphicObj.mpUserData ) : NULL;
        ImplAssignGraphicData();
        mbAutoSwapped = sal_False;
        mpMgr = rGraphicObj.mpMgr;

        mpMgr->ImplRegisterObj( *this, maGraphic, NULL, &rGraphicObj );
    }

    return *this;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::operator==( const GraphicObject& rGraphicObj ) const
{
    return( ( rGraphicObj.maGraphic == maGraphic ) &&
            ( rGraphicObj.maAttr == maAttr ) &&
            ( rGraphicObj.GetLink() == GetLink() ) );
}

// ------------------------------------------------------------------------

void GraphicObject::Load( SvStream& rIStm )
{
    rIStm >> *this;
}

// ------------------------------------------------------------------------

void GraphicObject::Save( SvStream& rOStm )
{
    rOStm << *this;
}

// ------------------------------------------------------------------------

void GraphicObject::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const GraphicObject& ) rCopyStream;
}

// -----------------------------------------------------------------------------

ByteString GraphicObject::GetUniqueID() const
{
    if ( !IsInSwapIn() && IsEPS() )
        const_cast<GraphicObject*>(this)->FireSwapInRequest();

    ByteString aRet;

    if( mpMgr )
        aRet = mpMgr->ImplGetUniqueID( *this );

    return aRet;
}

// -----------------------------------------------------------------------------

sal_uLong GraphicObject::GetChecksum() const
{
    return( ( maGraphic.IsSupportedGraphic() && !maGraphic.IsSwapOut() ) ? maGraphic.GetChecksum() : 0 );
}

// -----------------------------------------------------------------------------

SvStream* GraphicObject::GetSwapStream() const
{
    return( HasSwapStreamHdl() ? (SvStream*) mpSwapStreamHdl->Call( (void*) this ) : GRFMGR_AUTOSWAPSTREAM_NONE );
}

// -----------------------------------------------------------------------------

// !!! to be removed
sal_uLong GraphicObject::GetReleaseFromCache() const
{
    return 0;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetAttr( const GraphicAttr& rAttr )
{
    maAttr = rAttr;

    if( mpSimpleCache && ( mpSimpleCache->maAttr != rAttr ) )
        delete mpSimpleCache, mpSimpleCache = NULL;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetLink()
{
    if( mpLink )
        delete mpLink, mpLink = NULL;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetLink( const String& rLink )
{
    delete mpLink, mpLink = new String( rLink );
}

// -----------------------------------------------------------------------------

String GraphicObject::GetLink() const
{
    if( mpLink )
        return *mpLink;
    else
        return String();
}

// -----------------------------------------------------------------------------

void GraphicObject::SetUserData()
{
    if( mpUserData )
        delete mpUserData, mpUserData = NULL;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetUserData( const String& rUserData )
{
    delete mpUserData, mpUserData = new String( rUserData );
}

// -----------------------------------------------------------------------------

String GraphicObject::GetUserData() const
{
    if( mpUserData )
        return *mpUserData;
    else
        return String();
}

// -----------------------------------------------------------------------------

void GraphicObject::SetSwapStreamHdl()
{
    if( mpSwapStreamHdl )
    {
        delete mpSwapOutTimer, mpSwapOutTimer = NULL;
        delete mpSwapStreamHdl, mpSwapStreamHdl = NULL;
    }
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

Link GraphicObject::GetSwapStreamHdl() const
{
    if( mpSwapStreamHdl )
        return *mpSwapStreamHdl;
    else
        return Link();
}

// -----------------------------------------------------------------------------

void GraphicObject::FireSwapInRequest()
{
    ImplAutoSwapIn();
}

// -----------------------------------------------------------------------------

void GraphicObject::FireSwapOutRequest()
{
    ImplAutoSwapOutHdl( NULL );
}

// -----------------------------------------------------------------------------

void GraphicObject::GraphicManagerDestroyed()
{
    // we're alive, but our manager doesn't live anymore ==> connect to default manager
    mpMgr = NULL;
    ImplSetGraphicManager( NULL );
}

// -----------------------------------------------------------------------------

void GraphicObject::SetGraphicManager( const GraphicManager& rMgr )
{
    ImplSetGraphicManager( &rMgr );
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::IsCached( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              const GraphicAttr* pAttr, sal_uLong nFlags ) const
{
    sal_Bool bRet;

    if( nFlags & GRFMGR_DRAW_CACHED )
    {
        // --> OD 2005-10-11 #i54875# - Consider cropped graphics.
        // Note: The graphic manager caches a cropped graphic with its
        //       uncropped position and size.
//        bRet = mpMgr->IsInCache( pOut, rPt, rSz, *this, ( pAttr ? *pAttr : GetAttr() ) );
        Point aPt( rPt );
        Size aSz( rSz );
        if ( pAttr->IsCropped() )
        {
            PolyPolygon aClipPolyPoly;
            sal_Bool        bRectClip;
            ImplGetCropParams( pOut, aPt, aSz, pAttr, aClipPolyPoly, bRectClip );
        }
        bRet = mpMgr->IsInCache( pOut, aPt, aSz, *this, ( pAttr ? *pAttr : GetAttr() ) );
    }
    else
        bRet = sal_False;

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicObject::ReleaseFromCache()
{

    mpMgr->ReleaseFromCache( *this );
}

// -----------------------------------------------------------------------------

void GraphicObject::SetAnimationNotifyHdl( const Link& rLink )
{
    maGraphic.SetAnimationNotifyHdl( rLink );
}

// -----------------------------------------------------------------------------

List* GraphicObject::GetAnimationInfoList() const
{
    return maGraphic.GetAnimationInfoList();
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                          const GraphicAttr* pAttr, sal_uLong nFlags )
{
    GraphicAttr         aAttr( pAttr ? *pAttr : GetAttr() );
    Point               aPt( rPt );
    Size                aSz( rSz );
    const sal_uInt32    nOldDrawMode = pOut->GetDrawMode();
    sal_Bool                bCropped = aAttr.IsCropped();
    sal_Bool                bCached = sal_False;
    sal_Bool                bRet;

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
        sal_Bool        bRectClip;
        const sal_Bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

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
                pOut->IntersectClipRegion( aClipPolyPoly );
            }
        }
    }

    bRet = mpMgr->DrawObj( pOut, aPt, aSz, *this, aAttr, nFlags, bCached );

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

// --> OD 2010-01-04 #i105243#
sal_Bool GraphicObject::DrawWithPDFHandling( OutputDevice& rOutDev,
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
                sal_Bool bRectClip;
                const sal_Bool bCrop = ImplGetCropParams( &rOutDev,
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

    sal_Bool bRet = Draw( &rOutDev, rPt, rSz, &aGrfAttr, nFlags );

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
// <--

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::DrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSize,
                               const Size& rOffset, const GraphicAttr* pAttr, sal_uLong nFlags, int nTileCacheSize1D )
{
    if( pOut == NULL || rSize.Width() == 0 || rSize.Height() == 0 )
        return sal_False;

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

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::StartAnimation( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                    long nExtraData, const GraphicAttr* pAttr, sal_uLong /*nFlags*/,
                                    OutputDevice* pFirstFrameOutDev )
{
    sal_Bool bRet = sal_False;

    GetGraphic();

    if( !IsSwappedOut() )
    {
        const GraphicAttr aAttr( pAttr ? *pAttr : GetAttr() );

        if( mbAnimated )
        {
            Point   aPt( rPt );
            Size    aSz( rSz );
            sal_Bool    bCropped = aAttr.IsCropped();

            if( bCropped )
            {
                PolyPolygon aClipPolyPoly;
                sal_Bool        bRectClip;
                const sal_Bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

                pOut->Push( PUSH_CLIPREGION );

                if( bCrop )
                {
                    if( bRectClip )
                        pOut->IntersectClipRegion( aClipPolyPoly.GetBoundRect() );
                    else
                        pOut->IntersectClipRegion( aClipPolyPoly );
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

            bRet = sal_True;
        }
        else
            bRet = Draw( pOut, rPt, rSz, &aAttr, GRFMGR_DRAW_STANDARD );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicObject::StopAnimation( OutputDevice* pOut, long nExtraData )
{
    if( mpSimpleCache )
        mpSimpleCache->maGraphic.StopAnimation( pOut, nExtraData );
}

// -----------------------------------------------------------------------------

const Graphic& GraphicObject::GetGraphic() const
{
    if( mbAutoSwapped )
        ( (GraphicObject*) this )->ImplAutoSwapIn();

    return maGraphic;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetGraphic( const Graphic& rGraphic, const GraphicObject* pCopyObj )
{
    mpMgr->ImplUnregisterObj( *this );

    if( mpSwapOutTimer )
        mpSwapOutTimer->Stop();

    maGraphic = rGraphic;
    mbAutoSwapped = sal_False;
    ImplAssignGraphicData();
    delete mpLink, mpLink = NULL;
    delete mpSimpleCache, mpSimpleCache = NULL;

    mpMgr->ImplRegisterObj( *this, maGraphic, 0, pCopyObj);

    if( mpSwapOutTimer )
        mpSwapOutTimer->Start();
}

// -----------------------------------------------------------------------------

void GraphicObject::SetGraphic( const Graphic& rGraphic, const String& rLink )
{
    SetGraphic( rGraphic );
    mpLink = new String( rLink );
}

// -----------------------------------------------------------------------------

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
        BitmapEx    aBitmapEx( aTransGraphic.GetBitmapEx() );

        // convert crops to pixel
        aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel( Size( rAttr.GetLeftCrop(),
                                                                            rAttr.GetTopCrop() ),
                                                                      aMap100 );
        aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel( Size( rAttr.GetRightCrop(),
                                                                                rAttr.GetBottomCrop() ),
                                                                          aMap100 );

        // convert from prefmapmode to pixel
        const Size aSrcSizePixel( Application::GetDefaultDevice()->LogicToPixel( aSrcSize,
                                                                                 aMapGraph ) );

        // setup crop rectangle in pixel
        Rectangle aCropRect( aCropLeftTop.Width(), aCropLeftTop.Height(),
                             aSrcSizePixel.Width() - aCropRightBottom.Width(),
                             aSrcSizePixel.Height() - aCropRightBottom.Height() );

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
                                         aCropRectRel, rDestSize, sal_False );

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
            BitmapEx aBmpEx( aTransGraphic.GetBitmapEx() );

            ImplTransformBitmap( aBmpEx, rAttr, aCropLeftTop, aCropRightBottom,
                                 aCropRect, rDestSize, sal_True );

            aTransGraphic = aBmpEx;
        }

        aTransGraphic.SetPrefSize( rDestSize );
        aTransGraphic.SetPrefMapMode( rDestMap );
    }

    GraphicObject aGrfObj( aTransGraphic );
    aTransGraphic = aGrfObj.GetTransformedGraphic( &rAttr );

    return aTransGraphic;
}

// -----------------------------------------------------------------------------

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
                    GraphicManager::ImplAdjust( aAnimation, aAttr, ADJUSTMENT_ALL );
                    aAnimation.SetLoopCount( mnAnimationLoopCount );
                    aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( maGraphic.GetBitmapEx() );
                    GraphicManager::ImplAdjust( aBmpEx, aAttr, ADJUSTMENT_ALL );
                    aGraphic = aBmpEx;
                }
            }
            else
            {
                GDIMetaFile aMtf( maGraphic.GetGDIMetaFile() );
                GraphicManager::ImplAdjust( aMtf, aAttr, ADJUSTMENT_ALL );
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

// -----------------------------------------------------------------------------

void GraphicObject::ResetAnimationLoopCount()
{
    if( IsAnimated() && !IsSwappedOut() )
    {
        maGraphic.ResetAnimationLoopCount();

        if( mpSimpleCache )
            mpSimpleCache->maGraphic.ResetAnimationLoopCount();
    }
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::SwapOut()
{
    sal_Bool bRet = ( !mbAutoSwapped ? maGraphic.SwapOut() : sal_False );

    if( bRet && mpMgr )
        mpMgr->ImplGraphicObjectWasSwappedOut( *this );

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::SwapOut( SvStream* pOStm )
{
    sal_Bool bRet = ( !mbAutoSwapped ? maGraphic.SwapOut( pOStm ) : sal_False );

    if( bRet && mpMgr )
        mpMgr->ImplGraphicObjectWasSwappedOut( *this );

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::SwapIn()
{
    sal_Bool bRet;

    if( mbAutoSwapped )
    {
        ImplAutoSwapIn();
        bRet = sal_True;
    }
    else if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
        bRet = sal_True;
    else
    {
        bRet = maGraphic.SwapIn();

        if( bRet && mpMgr )
            mpMgr->ImplGraphicObjectWasSwappedIn( *this );
    }

    if( bRet )
        ImplAssignGraphicData();

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GraphicObject::SwapIn( SvStream* pIStm )
{
    sal_Bool bRet;

    if( mbAutoSwapped )
    {
        ImplAutoSwapIn();
        bRet = sal_True;
    }
    else if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
        bRet = sal_True;
    else
    {
        bRet = maGraphic.SwapIn( pIStm );

        if( bRet && mpMgr )
            mpMgr->ImplGraphicObjectWasSwappedIn( *this );
    }

    if( bRet )
        ImplAssignGraphicData();

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetSwapState()
{
    if( !IsSwappedOut() )
    {
        mbAutoSwapped = sal_True;

        if( mpMgr )
            mpMgr->ImplGraphicObjectWasSwappedOut( *this );
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK( GraphicObject, ImplAutoSwapOutHdl, void*, EMPTYARG )
{
    if( !IsSwappedOut() )
    {
        mbIsInSwapOut = sal_True;

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

        mbIsInSwapOut = sal_False;
    }

    if( mpSwapOutTimer )
        mpSwapOutTimer->Start();

    return 0L;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, GraphicObject& rGraphicObj )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    Graphic         aGraphic;
    GraphicAttr     aAttr;
    ByteString      aLink;
    sal_Bool            bLink;

    rIStm >> aGraphic >> aAttr >> bLink;

    rGraphicObj.SetGraphic( aGraphic );
    rGraphicObj.SetAttr( aAttr );

    if( bLink )
    {
        rIStm >> aLink;
        rGraphicObj.SetLink( UniString( aLink, RTL_TEXTENCODING_UTF8 ) );
    }
    else
        rGraphicObj.SetLink();

    rGraphicObj.SetSwapStreamHdl();

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const GraphicObject& rGraphicObj )
{
    VersionCompat   aCompat( rOStm, STREAM_WRITE, 1 );
    const sal_Bool      bLink =  rGraphicObj.HasLink();

    rOStm << rGraphicObj.GetGraphic() << rGraphicObj.GetAttr() << bLink;

    if( bLink )
        rOStm << ByteString( rGraphicObj.GetLink(), RTL_TEXTENCODING_UTF8 );

    return rOStm;
}

#define UNO_NAME_GRAPHOBJ_URLPREFIX "vnd.sun.star.GraphicObject:"

GraphicObject GraphicObject::CreateGraphicObjectFromURL( const ::rtl::OUString &rURL )
{
    const String aURL( rURL ), aPrefix( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX) );
    if( aURL.Search( aPrefix ) == 0 )
    {
        // graphic manager url
        ByteString aUniqueID( String(rURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 )), RTL_TEXTENCODING_UTF8 );
        return GraphicObject( aUniqueID );
    }
    else
    {
        Graphic     aGraphic;
        if ( aURL.Len() )
        {
            SvStream*   pStream = utl::UcbStreamHelper::CreateStream( aURL, STREAM_READ );
            if( pStream )
                GraphicConverter::Import( *pStream, aGraphic );
        }

        return GraphicObject( aGraphic );
    }
}

