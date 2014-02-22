/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
    
    
    ::basegfx::B2DPolyPolygon aClipPoly;

    
    for( int i=nOffset; i<nNumStrips; i+=2 )
    {
        aClipPoly.append(
            ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle( double(i)/nNumStrips, 0.0,
                                         double(i+1)/nNumStrips, 1.0) ) );

    }

    
    
    const ::basegfx::B2DVector aUpVec(0.0, 1.0);
    basegfx::B2DHomMatrix aMatrix(basegfx::tools::createRotateAroundPoint(0.5, 0.5, aUpVec.angle( rDirection )));

    
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

    
    
    
    
    
    
    

    
    const basegfx::B2DHomMatrix viewTransform( rViewEntry.mpView->getTransformation() );
    const basegfx::B2DPoint pageOrigin( viewTransform * basegfx::B2DPoint() );

    
    
    cppcanvas::CanvasSharedPtr pCanvas( pCanvas_->clone() );
    basegfx::B2DPoint p;

    
    

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
        
        pLeavingBitmap->clip( aClipPolygon1 );
        
        p = basegfx::B2DPoint( pageOrigin + (t * aPushDirection) );
        pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
        pLeavingBitmap->draw( pCanvas );

        
        pLeavingBitmap->clip( aClipPolygon2 );
        
        p = basegfx::B2DPoint( pageOrigin - (t * aPushDirection) );
        pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
        pLeavingBitmap->draw( pCanvas );
    }

    
    

    
    pEnteringBitmap->clip( aClipPolygon1 );
    
    p = basegfx::B2DPoint( pageOrigin + ((t - 1.0) * aPushDirection) );
    pCanvas->setTransformation(basegfx::tools::createTranslateB2DHomMatrix(p.getX(), p.getY()));
    pEnteringBitmap->draw( pCanvas );

    
    pEnteringBitmap->clip( aClipPolygon2 );
    
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

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
