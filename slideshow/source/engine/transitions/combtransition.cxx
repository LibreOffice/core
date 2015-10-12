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


#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <cppcanvas/spritecanvas.hxx>

#include "combtransition.hxx"

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
                   [this, &t]( const ViewEntry& rViewEntry )
                   { return this->renderComb( t, rViewEntry ); } );

    getScreenUpdater().notifyUpdate();

    return true;
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
