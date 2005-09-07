/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: combtransition.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:50:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "combtransition.hxx"
#include "canvas/debug.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "basegfx/polygon/b2dpolypolygontools.hxx"

namespace presentation {
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
                ::basegfx::B2DRectangle( (double)i/nNumStrips, 0.0,
                                         (double)(i+1)/nNumStrips, 1.0) ) );

    }

    // rotate polygons, such that the strips are parallel to
    // the given direction vector
    const ::basegfx::B2DVector aUpVec(0.0, 1.0);
    ::basegfx::B2DHomMatrix    aMatrix;

    aMatrix.translate( -0.5, -0.5 );
    aMatrix.rotate( aUpVec.angle( rDirection ) );
    aMatrix.translate( 0.5, 0.5 );

    // blow up clip polygon to slide size
    aMatrix.scale( rSlideSize.getX(),
                   rSlideSize.getY() );

    aClipPoly.transform( aMatrix );

    return aClipPoly;
}

}

CombTransition::CombTransition(
    boost::optional<SlideSharedPtr> const & leavingSlide,
    const SlideSharedPtr& pEnteringSlide,
    const SoundPlayerSharedPtr& pSoundPlayer,
    const ::basegfx::B2DVector& rPushDirection,
    sal_Int32                   nNumStripes )
    : SlideChangeBase( leavingSlide, pEnteringSlide, pSoundPlayer,
                       false /* no leaving sprite */,
                       false /* no entering sprite */ ),
      maPushDirectionUnit( rPushDirection ),
      mnNumStripes( nNumStripes )
{
}

void CombTransition::renderComb(
    double t, UnoViewSharedPtr const & pView ) const
{
    const cppcanvas::CanvasSharedPtr pCanvas_ = pView->getCanvas();

    // calc bitmap offsets. The enter/leaving bitmaps are only
    // as large as the actual slides. For scaled-down
    // presentations, we have to move the left, top edge of
    // those bitmaps to the actual position, governed by the
    // given view transform. The aBitmapPosPixel local
    // variable is already in device coordinate space
    // (i.e. pixel).

    ENSURE_AND_THROW( pCanvas_.get(),
                      "CombTransition::renderComb(): Invalid canvas" );

    // TODO(F2): Properly respect clip here. Might have to be transformed, too.
    const basegfx::B2DHomMatrix viewTransform( pCanvas_->getTransformation() );
    const basegfx::B2DPoint pageOrigin( viewTransform * basegfx::B2DPoint() );

    // change transformation on cloned canvas to be in
    // device pixel
    cppcanvas::CanvasSharedPtr pCanvas( pCanvas_->clone() );
    basegfx::B2DHomMatrix transform;
    basegfx::B2DPoint p;

    // TODO(Q2): Use basegfx bitmaps here
    // TODO(F1): SlideBitmap is not fully portable between different canvases!

    const basegfx::B2DSize enteringSizePixel(
        getEnteringSizePixel(pView) );

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

    SlideBitmapSharedPtr const & pLeavingBitmap = getLeavingBitmap();
    if (pLeavingBitmap.get() != 0) {
        // render odd strips:
        pLeavingBitmap->clip( aClipPolygon1 );
        // don't modify bitmap object (no move!):
        p = basegfx::B2DPoint( pageOrigin + (t * aPushDirection) );
        transform.translate( p.getX(), p.getY() );
        pCanvas->setTransformation( transform );
        pLeavingBitmap->draw( pCanvas );

        // render even strips:
        pLeavingBitmap->clip( aClipPolygon2 );
        // don't modify bitmap object (no move!):
        transform.identity();
        p = basegfx::B2DPoint( pageOrigin - (t * aPushDirection) );
        transform.translate( p.getX(), p.getY() );
        pCanvas->setTransformation( transform );
        pLeavingBitmap->draw( pCanvas );
    }

    // TODO(Q2): Use basegfx bitmaps here
    // TODO(F1): SlideBitmap is not fully portable between different canvases!

    // render odd strips:
    SlideBitmapSharedPtr const & pEnteringBitmap = getEnteringBitmap();
    pEnteringBitmap->clip( aClipPolygon1 );
    // don't modify bitmap object (no move!):
    transform.identity();
    p = basegfx::B2DPoint( pageOrigin + ((t - 1.0) * aPushDirection) );
    transform.translate( p.getX(), p.getY() );
    pCanvas->setTransformation( transform );
    pEnteringBitmap->draw( pCanvas );

    // render even strips:
    pEnteringBitmap->clip( aClipPolygon2 );
    // don't modify bitmap object (no move!):
    transform.identity();
    p = basegfx::B2DPoint( pageOrigin + ((1.0 - t) * aPushDirection) );
    transform.translate( p.getX(), p.getY() );
    pCanvas->setTransformation( transform );
    pEnteringBitmap->draw( pCanvas );
}

bool CombTransition::operator()( double t )
{
    SlideBitmapSharedPtr const & pSlideBitmap = getEnteringBitmap();
    if (pSlideBitmap.get() == 0)
        return false;
    for_each_view( boost::bind( &CombTransition::renderComb, this, t, _1 ) );
    return true;
}

} // namespace internal
} // namespace presentation
