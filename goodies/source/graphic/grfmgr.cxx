/*************************************************************************
 *
 *  $RCSfile: grfmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-11 15:17:49 $
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

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/vcompat.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/salbtype.hxx>
#include "grfmgr.hxx"

// -----------
// - Defines -
// -----------

#define GRFMGR_CACHESIZE_STANDARD           10000000UL
#define GRFMGR_OBJCACHESIZE_STANDARD        2500000UL
#define GRFMGR_CACHESIZE_APPSERVER          10000000UL
#define GRFMGR_OBJCACHESIZE_APPSERVER       2500000UL
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
    mpLink  ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const Graphic& rGraphic, const GraphicManager* pMgr ) :
    maGraphic   ( rGraphic ),
    mpLink      ( NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const Graphic& rGraphic, const String& rLink, const GraphicManager* pMgr ) :
    maGraphic   ( rGraphic ),
    mpLink      ( rLink.Len() ? ( new String( rLink ) ) : NULL )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const GraphicObject& rGraphicObj, const GraphicManager* pMgr ) :
    maGraphic   ( rGraphicObj.GetGraphic() ),
    mpLink      ( rGraphicObj.mpLink ? ( new String( *rGraphicObj.mpLink ) ) : NULL ),
    maAttr      ( rGraphicObj.maAttr )
{
    ImplConstruct();
    ImplAssignGraphicData();
    ImplSetGraphicManager( pMgr );
}

// -----------------------------------------------------------------------------

GraphicObject::GraphicObject( const ByteString& rUniqueID, const GraphicManager* pMgr ) :
    mpLink  ( NULL )
{
    ImplConstruct();
    ImplSetGraphicManager( pMgr, &rUniqueID );
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
    delete mpSimpleCache;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplConstruct()
{
    mpMgr = NULL;
    mpSwapStreamHdl = NULL;
    mpSwapOutTimer = NULL;
    mpSimpleCache = NULL;
    mbAutoSwapped = FALSE;
    mbIsInSwapIn = FALSE;
    mbIsInSwapOut = FALSE;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplAssignGraphicData()
{
    maPrefSize = maGraphic.GetPrefSize();
    maPrefMapMode = maGraphic.GetPrefMapMode();
    mnSizeBytes = maGraphic.GetSizeBytes();
    meType = maGraphic.GetType();
    mbTransparent = maGraphic.IsTransparent();
    mbAnimated = maGraphic.IsAnimated();

    if( maGraphic.GetType() == GRAPHIC_GDIMETAFILE )
    {
        const GDIMetaFile& rMtf = GetGraphic().GetGDIMetaFile();
        mbEPS = ( rMtf.GetActionCount() == 1 ) && ( META_EPS_ACTION == rMtf.GetAction( 0 )->GetType() );
    }
    else
        mbEPS = FALSE;
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplSetGraphicManager( const GraphicManager* pMgr, const ByteString* pID )
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
                    if( Application::IsRemoteServer() )
                        mpGlobalMgr = new GraphicManager( GRFMGR_CACHESIZE_APPSERVER, GRFMGR_OBJCACHESIZE_APPSERVER );
                    else
                        mpGlobalMgr = new GraphicManager( GRFMGR_CACHESIZE_STANDARD, GRFMGR_OBJCACHESIZE_STANDARD );
                }

                mpMgr = mpGlobalMgr;
            }
            else
                mpMgr = (GraphicManager*) pMgr;

            mpMgr->ImplRegisterObj( *this, maGraphic, pID );
        }
    }
}

// -----------------------------------------------------------------------------

void GraphicObject::ImplAutoSwapIn( BOOL bIgnoreSwapState )
{
    if( bIgnoreSwapState || IsSwappedOut() )
    {
        if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
            mbAutoSwapped = FALSE;
        else
        {
            mbIsInSwapIn = TRUE;

            SvStream* pStream = GetSwapStream();

            if( GRFMGR_AUTOSWAPSTREAM_NONE != pStream )
            {
                if( GRFMGR_AUTOSWAPSTREAM_LINK == pStream )
                {
                    if( HasLink() )
                    {
                        SvFileStream aIStm( GetLink(), STREAM_READ );
                        mbAutoSwapped = !maGraphic.SwapIn( &aIStm );
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

            mbIsInSwapIn = FALSE;

            if( !mbAutoSwapped && mpMgr )
                mpMgr->ImplGraphicObjectWasSwappedIn( *this );
        }
    }
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::ImplGetCropParams( OutputDevice* pOut, Point& rPt, Size& rSz, const GraphicAttr* pAttr,
                                       PolyPolygon& rClipPolyPoly, BOOL& bRectClipRegion ) const
{
    BOOL bRet = FALSE;

    if( GetType() != GRAPHIC_NONE )
    {
        Polygon         aClipPoly( Rectangle( rPt, rSz ) );
        const USHORT    nRot10 = pAttr->GetRotation() % 3600;
        const Point     aOldOrigin( rPt );
        const Graphic&  rGraphic = GetGraphic();
        const MapMode   aMap100( MAP_100TH_MM );
        Size            aSize100;
        long            nTotalWidth, nTotalHeight;
        long            nNewLeft, nNewTop, nNewRight, nNewBottom;
        double          fScale;

        if( nRot10 )
        {
            aClipPoly.Rotate( rPt, nRot10 );
            bRectClipRegion = FALSE;
        }
        else
            bRectClipRegion = TRUE;

        rClipPolyPoly = aClipPoly;

        if( rGraphic.GetPrefMapMode() == MAP_PIXEL )
            aSize100 = pOut->PixelToLogic( rGraphic.GetPrefSize(), aMap100 );
        else
            aSize100 = pOut->LogicToLogic( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode(), aMap100 );

        nTotalWidth = aSize100.Width() - pAttr->GetLeftCrop() - pAttr->GetRightCrop();
        nTotalHeight = aSize100.Height() - pAttr->GetTopCrop() - pAttr->GetBottomCrop();

        if( aSize100.Width() && aSize100.Height() && nTotalWidth && nTotalHeight )
        {
            fScale = (double) aSize100.Width() / nTotalWidth;
            nNewLeft = -FRound( pAttr->GetLeftCrop() * fScale );
            nNewRight = nNewLeft + FRound( aSize100.Width() * fScale ) - 1;

            fScale = (double) rSz.Width() / aSize100.Width();
            rPt.X() += FRound( nNewLeft * fScale );
            rSz.Width() = FRound( ( nNewRight - nNewLeft + 1 ) * fScale );

            fScale = (double) aSize100.Height() / nTotalHeight;
            nNewTop = -FRound( pAttr->GetTopCrop() * fScale );
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

            bRet = TRUE;
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

        maGraphic = rGraphicObj.GetGraphic();
        maAttr = rGraphicObj.maAttr;
        mpLink = rGraphicObj.mpLink ? new String( *rGraphicObj.mpLink ) : NULL;
        ImplAssignGraphicData();
        mbAutoSwapped = FALSE;
        mpMgr = rGraphicObj.mpMgr;

        mpMgr->ImplRegisterObj( *this, maGraphic, NULL );
    }

    return *this;
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::operator==( const GraphicObject& rGraphicObj ) const
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
    ByteString aRet;

    if( mpMgr )
        aRet = mpMgr->ImplGetUniqueID( *this );

    return aRet;
}

// -----------------------------------------------------------------------------

ULONG GraphicObject::GetChecksum() const
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
ULONG GraphicObject::GetReleaseFromCache() const
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

void GraphicObject::SetSwapStreamHdl()
{
    if( mpSwapStreamHdl )
    {
        delete mpSwapOutTimer, mpSwapOutTimer = NULL;
        delete mpSwapStreamHdl, mpSwapStreamHdl = NULL;
    }
}

// -----------------------------------------------------------------------------

void GraphicObject::SetSwapStreamHdl( const Link& rHdl, const ULONG nSwapOutTimeout )
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
    ImplAutoSwapIn( TRUE );
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

BOOL GraphicObject::IsCached( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              const GraphicAttr* pAttr, ULONG nFlags ) const
{
    BOOL bRet;

    if( nFlags & GRFMGR_DRAW_CACHED )
        bRet = mpMgr->IsInCache( pOut, rPt, rSz, *this, ( pAttr ? *pAttr : GetAttr() ) );
    else
        bRet = FALSE;

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

BOOL GraphicObject::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                          const GraphicAttr* pAttr, ULONG nFlags )
{
    const GraphicAttr&  rActAttr = ( pAttr ? *pAttr : GetAttr() );
    const Point*        pPt;
    const Size*         pSz;
    const BOOL          bCropped = rActAttr.IsCropped();
    BOOL                bCached = FALSE;
    BOOL                bRet;

    if( bCropped )
    {
        pPt = new Point( rPt );
        pSz = new Size( rSz );

        PolyPolygon aClipPolyPoly;
        BOOL        bRectClip;
        const BOOL  bCrop = ImplGetCropParams( pOut, (Point&) *pPt, (Size&) *pSz,
                                               &rActAttr, aClipPolyPoly, bRectClip );

        pOut->Push( PUSH_CLIPREGION );

        if( bCrop )
        {
            if( bRectClip )
                pOut->IntersectClipRegion( aClipPolyPoly.GetBoundRect() );
            else
                pOut->IntersectClipRegion( aClipPolyPoly );
        }
    }
    else
    {
        pPt = &rPt;
        pSz = &rSz;
    }

    if( IsAnimated() || !( nFlags & GRFMGR_DRAW_CACHED ) )
    {
        GetGraphic();

        if( maGraphic.IsSupportedGraphic() && !maGraphic.IsSwapOut() )
        {
            Graphic aGraphic( GetTransformedGraphic( &rActAttr ) );

            if( aGraphic.IsSupportedGraphic() )
            {
                aGraphic.Draw( pOut, *pPt, *pSz );
                bRet = TRUE;
            }
            else
                bRet = FALSE;
        }
        else
            bRet = FALSE;
    }
    else
        bRet = mpMgr->DrawObj( pOut, *pPt, *pSz, *this, rActAttr, nFlags, bCached );

    if( bCached )
    {
        if( mpSwapOutTimer )
            mpSwapOutTimer->Start();
        else
            FireSwapOutRequest();
    }

    if( bCropped )
    {
        pOut->Pop();
        delete (Point*) pPt;
        delete (Size*) pSz;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::StartAnimation( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                    long nExtraData, const GraphicAttr* pAttr, ULONG nFlags,
                                    OutputDevice* pFirstFrameOutDev )
{
    const GraphicAttr&  rActAttr = ( pAttr ? *pAttr : GetAttr() );
    BOOL                bRet = FALSE;

    GetGraphic();

    if( !IsSwappedOut() )
    {
        if( mbAnimated )
        {
            const BOOL      bCropped = rActAttr.IsCropped();
            const Point*    pPt;
            const Size*     pSz;

            if( bCropped )
            {
                pPt = new Point( rPt );
                pSz = new Size( rSz );

                PolyPolygon aClipPolyPoly;
                BOOL        bRectClip;
                const BOOL  bCrop = ImplGetCropParams( pOut, (Point&) *pPt, (Size&) *pSz,
                                                       &rActAttr, aClipPolyPoly, bRectClip );

                pOut->Push( PUSH_CLIPREGION );

                if( bCrop )
                {
                    if( bRectClip )
                        pOut->IntersectClipRegion( aClipPolyPoly.GetBoundRect() );
                    else
                        pOut->IntersectClipRegion( aClipPolyPoly );
                }
            }
            else
            {
                pPt = &rPt;
                pSz = &rSz;
            }

            if( !mpSimpleCache || ( mpSimpleCache->maAttr != rActAttr ) || pFirstFrameOutDev )
            {
                if( mpSimpleCache )
                    delete mpSimpleCache;

                mpSimpleCache = new GrfSimpleCacheObj( GetTransformedGraphic( &rActAttr ), rActAttr );
                mpSimpleCache->maGraphic.SetAnimationNotifyHdl( GetAnimationNotifyHdl() );
            }

            mpSimpleCache->maGraphic.StartAnimation( pOut, *pPt, *pSz, nExtraData, pFirstFrameOutDev );

            if( bCropped )
            {
                pOut->Pop();
                delete (Point*) pPt;
                delete (Size*) pSz;
            }

            bRet = TRUE;
        }
        else
            bRet = Draw( pOut, rPt, rSz, &rActAttr, GRFMGR_DRAW_STANDARD );
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
        ( (GraphicObject*) this )->ImplAutoSwapIn( FALSE );

    return maGraphic;
}

// -----------------------------------------------------------------------------

void GraphicObject::SetGraphic( const Graphic& rGraphic )
{
    mpMgr->ImplUnregisterObj( *this );

    if( mpSwapOutTimer )
        mpSwapOutTimer->Stop();

    maGraphic = rGraphic;
    mbAutoSwapped = FALSE;
    ImplAssignGraphicData();
    delete mpLink, mpLink = NULL;
    delete mpSimpleCache, mpSimpleCache = NULL;

    mpMgr->ImplRegisterObj( *this, maGraphic, NULL );
}

// -----------------------------------------------------------------------------

void GraphicObject::SetGraphic( const Graphic& rGraphic, const String& rLink )
{
    SetGraphic( rGraphic );
    mpLink = new String( rLink );
}

// -----------------------------------------------------------------------------

Graphic GraphicObject::GetTransformedGraphic( const GraphicAttr* pAttr ) const
{
    Graphic     aGraphic;
    GraphicAttr aActAttr;

    GetGraphic();
    aActAttr = ( pAttr ? *pAttr : GetAttr() );

    if( maGraphic.IsSupportedGraphic() && !maGraphic.IsSwapOut() )
    {
        if( aActAttr.IsSpecialDrawMode() || aActAttr.IsAdjusted() ||
            aActAttr.IsMirrored() || aActAttr.IsRotated() || aActAttr.IsTransparent() )
        {
            if( GetType() == GRAPHIC_BITMAP )
            {
                if( IsAnimated() )
                {
                    Animation aAnimation( maGraphic.GetAnimation() );
                    GraphicManager::ImplAdjust( aAnimation, aActAttr, ADJUSTMENT_ALL );
                    aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( maGraphic.GetBitmapEx() );
                    GraphicManager::ImplAdjust( aBmpEx, aActAttr, ADJUSTMENT_ALL );
                    aGraphic = aBmpEx;
                }
            }
            else
            {
                GDIMetaFile aMtf( maGraphic.GetGDIMetaFile() );
                GraphicManager::ImplAdjust( aMtf, aActAttr, ADJUSTMENT_ALL );
                aGraphic = aMtf;
            }
        }
        else
            aGraphic = maGraphic;
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

BOOL GraphicObject::SwapOut()
{
    BOOL bRet = ( !mbAutoSwapped ? maGraphic.SwapOut() : FALSE );

    if( bRet && mpMgr )
        mpMgr->ImplGraphicObjectWasSwappedOut( *this );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::SwapOut( SvStream* pOStm )
{
    BOOL bRet = ( !mbAutoSwapped ? maGraphic.SwapOut( pOStm ) : FALSE );

    if( bRet && mpMgr )
        mpMgr->ImplGraphicObjectWasSwappedOut( *this );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::SwapIn()
{
    BOOL bRet;

    if( mbAutoSwapped )
    {
        ImplAutoSwapIn( FALSE );
        bRet = TRUE;
    }
    else if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
        bRet = TRUE;
    else
    {
        bRet = maGraphic.SwapIn();

        if( bRet && mpMgr )
            mpMgr->ImplGraphicObjectWasSwappedIn( *this );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicObject::SwapIn( SvStream* pIStm )
{
    BOOL bRet;

    if( mbAutoSwapped )
    {
        ImplAutoSwapIn( FALSE );
        bRet = TRUE;
    }
    else if( mpMgr && mpMgr->ImplFillSwappedGraphicObject( *this, maGraphic ) )
        bRet = TRUE;
    else
    {
        bRet = maGraphic.SwapIn( pIStm );

        if( bRet && mpMgr )
            mpMgr->ImplGraphicObjectWasSwappedIn( *this );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( GraphicObject, ImplAutoSwapOutHdl, void*, p )
{
    if( !mbAutoSwapped && !IsSwappedOut() )
    {
        mbIsInSwapOut = TRUE;

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

        mbIsInSwapOut = FALSE;
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
    BOOL            bLink;

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
    const BOOL      bLink =  rGraphicObj.HasLink();

    rOStm << rGraphicObj.GetGraphic() << rGraphicObj.GetAttr() << bLink;

    if( bLink )
        rOStm << ByteString( rGraphicObj.GetLink(), RTL_TEXTENCODING_UTF8 );

    return rOStm;
}
