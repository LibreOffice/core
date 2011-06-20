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
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <cppcanvas/spritecanvas.hxx>

#include "combtransition.hxx"

#include <boost/bind.hpp>


namespace slideshow {
namespace internal {

namespace {

basegfx::B2DPolyPolygon createClipPolygon(
    const ::basegfx::B2DVector& rDirection,
    const ::basegfx::B2DSize& rSlideSize,
    int nNumStrips, int nOffset )
{
    // create clip polygon in standard orientation (will later
    // be rotated to match direction vector)
    ::basegfx::B2DPolyPolygon aClipPoly;

    // create nNumStrips/2 vertical strips
    for( int i=nOffset; i<nNumStrips; i+=2 )
    {
        aClipPoly.append(
            ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle( double(i)/nNumStrips, 0.0,
                                         double(i+1)/nNumStrips, 1.0) ) );

    }

    // rotate polygons, such that the strips are parallel to
    // the given direction vector
    const ::basegfx::B2DVector aUpVec(0.0, 1.0);
    basegfx::B2DHomMatrix aMatrix(basegfx::tools::createRotateAroundPoint(0.5, 0.5, aUpVec.angle( rDirection )));

    // blow up clip polygon to slide size
    aMatrix.scale( rSlideSize.getX(),
                   rSlideSize.getY() );

    aClipPoly.transform( aMatrix );

    return aClipPoly;
}

}

CombTransition::CombTransition(
    boost::optional<SlideSharedPtr> const & leavingSlide,
    const SlideSharedPtr&                   pEnteringSlide,
    const SoundPlayerSharedPtr&             pSoundPlayer,
    const UnoViewContainer&                 rViewContainer,
    ScreenUpdater&                          rScreenUpdater,
    EventMultiplexer&                       rEventMultiplexer,
    const ::basegfx::B2DVector&             rPushDirection,
    sal_Int32                               nNumStripes )
    : SlideChangeBase( leavingSlide, pEnteringSlide, pSoundPlayer,
                       rViewContainer, rScreenUpdater, rEventMultiplexer,
                       false /* no leaving sprite */,
                       false /* no entering sprite */ ),
      maPushDirectionUnit( rPushDirection ),
      mnNumStripes( nNumStripes )
{
}

void CombTransition::renderComb( double           t,
                                 const ViewEntry& rViewEntry ) const
{
    const SlideBitmapSharedPtr& pEnteringBitmap = getEnteringBitmap(rViewEntry);
    const cppcanvas::CanvasSharedPtr pCanvas_ = rViewEntry.mpView->getCanvas();

    if( !pEnteringBitmap || !pCanvas_ )
        return;

    // calc bitmap offsets. The enter/leaving bitmaps are only
    // as large as the actual slides. For scaled-down
    // presentations, we have to move the left, top edge of
    // those bitmaps to the actual position, governed by the
    // given view transform. The aBitmapPosPixel local
    // variable is already in device coordinate space
    // (i.e. pixel).

    // TODO(F2): Properly respect clip here. Might have to be transformed, too.
    const basegfx::B2DHomMatrix viewTransform( rViewEntry.mpView->getTransformation() );
    const basegfx::B2DPoint pageOrigin( viewTransform * basegfx::B2DPoint() );

    // change transformation on cloned canvas to be in
    // device pixel
    cppcanvas::CanvasSharedPtr pCanvas( pCanvas_->clone() );
    basegfx::B2DPoint p;

    // TODO(Q2): Use basegfx bitmaps here
    // TODO(F1): SlideBitmap is not fully portable between different canvases!

    const basegfx::B2DSize enteringSizePixel(
        getEnteringSlideSizePixel( rViewEntry.mpView) );

    const basegfx::B2DVector aPushDirection = basegfx::B2DVector(
        enteringSizePixel * maPushDirectionUnit );
    const basegfx::B2DPolyPolygon aClipPolygon1 = basegfx::B2DPolyPolygon(
        createClipPolygon( maPushDirectionUnit,
                           enteringSizePixel,
                           mnNumStripes, 0 ) );
    const basegfx::B2DPolyPolygon aClipPolygon2 = basegfx::B2DPolyPolygon(
        createClipPolygon( maPushDirectionUnit,
                           enteringSizePixel,
                           mnNumStripes, 1 ) );

    SlideBitmapSharedPtr const & pLeavingBitmap = getLeavingBitmap(rViewEntry);
    if( pLeavingBitmap )
    {
        // render odd strips:
        pLeavingBitmap->clip( aClipPolygon1 );
        // don't modify bitmap object (no move!):
        p = basegfx::B2DPoint( pageOrigin + (t * aPushDirection) );
        pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
        pLeavingBitmap->draw( pCanvas );

        // render even strips:
        pLeavingBitmap->clip( aClipPolygon2 );
        // don't modify bitmap object (no move!):
        p = basegfx::B2DPoint( pageOrigin - (t * aPushDirection) );
        pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
        pLeavingBitmap->draw( pCanvas );
    }

    // TODO(Q2): Use basegfx bitmaps here
    // TODO(F1): SlideBitmap is not fully portable between different canvases!

    // render odd strips:
    pEnteringBitmap->clip( aClipPolygon1 );
    // don't modify bitmap object (no move!):
    p = basegfx::B2DPoint( pageOrigin + ((t - 1.0) * aPushDirection) );
    pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
    pEnteringBitmap->draw( pCanvas );

    // render even strips:
    pEnteringBitmap->clip( aClipPolygon2 );
    // don't modify bitmap object (no move!):
    p = basegfx::B2DPoint( pageOrigin + ((1.0 - t) * aPushDirection) );
    pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
    pEnteringBitmap->draw( pCanvas );
}

bool CombTransition::operator()( double t )
{
    std::for_each( beginViews(),
                   endViews(),
                   boost::bind( &CombTransition::renderComb,
                                this,
                                t,
                                _1 ));

    getScreenUpdater().notifyUpdate();

    return true;
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
