/*************************************************************************
 *
 *  $RCSfile: combtransition.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:08:29 $
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

#include <canvas/debug.hxx>
#include <combtransition.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>


namespace presentation
{
    namespace internal
    {
        namespace
        {
            ::basegfx::B2DPolyPolygon createClipPolygon( const ::basegfx::B2DVector& rDirection,
                                                         const ::basegfx::B2DSize&   rSlideSize,
                                                         int                         nNumStrips,
                                                         int                         nOffset )
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

        CombTransition::CombTransition( const SlideBitmapSharedPtr& rLeavingBitmap,
                                        const SlideBitmapSharedPtr& rEnteringBitmap,
                                        const ::basegfx::B2DVector& rPushDirection,
                                        sal_Int32                   nNumStripes,
                                        const SoundPlayerSharedPtr& rSoundPlayer ) :
            maViews(),
            mpLeavingBitmap( rLeavingBitmap ),
            mpEnteringBitmap( rEnteringBitmap ),
            maBitmapSize( getBitmapSize() ),
            maClipPolygon1( createClipPolygon( rPushDirection,
                                               maBitmapSize,
                                               nNumStripes,
                                               0 ) ),
            maClipPolygon2( createClipPolygon( rPushDirection,
                                               maBitmapSize,
                                               nNumStripes,
                                               1 ) ),
            maPushDirection( maBitmapSize *
                             rPushDirection ),
            mpSoundPlayer( rSoundPlayer )
        {
            ENSURE_AND_THROW(
                rEnteringBitmap.get(),
                "CombTransition::CombTransition(): Invalid entering bitmap" );
        }

        ::basegfx::B2DSize CombTransition::getBitmapSize() const
        {
            return ::basegfx::B2DSize(
                ::basegfx::B2DTuple( mpEnteringBitmap->getSize() ) );
        }

        void CombTransition::start( const AnimatableShapeSharedPtr&,
                                    const ShapeAttributeLayerSharedPtr& )
        {
            // TODO(F1): Maybe we've got to create separate bitmaps
            // for every view, should the canvas not allow for
            // cross-canvas bitmap rendering
            if( mpSoundPlayer.get() )
                mpSoundPlayer->startPlayback();
        }

        void CombTransition::end()
        {
            if( mpSoundPlayer.get() )
                mpSoundPlayer->stopPlayback();

            // drop all references
            mpLeavingBitmap.reset();
            mpEnteringBitmap.reset();

            maViews.clear();
        }

        bool CombTransition::operator()( double t )
        {
            if( !mpEnteringBitmap.get() )
            {
                return false;
            }

            for( ::std::size_t i=0, nEntries=maViews.size(); i<nEntries; ++i )
            {
                // calc bitmap offsets. The enter/leaving bitmaps are only
                // as large as the actual slides. For scaled-down
                // presentations, we have to move the left, top edge of
                // those bitmaps to the actual position, governed by the
                // given view transform. The aBitmapPosPixel local
                // variable is already in device coordinate space
                // (i.e. pixel).

                ::cppcanvas::CanvasSharedPtr pCanvas( maViews[i]->getCanvas() );

                ENSURE_AND_THROW( pCanvas.get(),
                                  "CombTransition::operator(): Invalid canvas" );

                // TODO(F2): Properly respect clip here. Might have to be transformed, too.
                const ::basegfx::B2DHomMatrix   aViewTransform( pCanvas->getTransformation() );
                const ::basegfx::B2DPoint       aPageOrigin( aViewTransform * ::basegfx::B2DPoint() );

                // change transformation on cloned canvas to be in
                // device pixel
                pCanvas = pCanvas->clone();
                pCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                // TODO(Q2): Use basegfx bitmaps here
                // TODO(F1): SlideBitmap is not fully portable between different canvases!

                if( mpLeavingBitmap.get() )
                {
                    // render odd strips
                    mpLeavingBitmap->move( aPageOrigin + t*maPushDirection );
                    mpLeavingBitmap->clip( maClipPolygon1 );
                    mpLeavingBitmap->draw( pCanvas );

                    // render even strips
                    mpLeavingBitmap->move( aPageOrigin - t*maPushDirection );
                    mpLeavingBitmap->clip( maClipPolygon2 );
                    mpLeavingBitmap->draw( pCanvas );
                }

                // TODO(Q2): Use basegfx bitmaps here
                // TODO(F1): SlideBitmap is not fully portable between different canvases!

                // render odd strips
                mpEnteringBitmap->move( aPageOrigin + (t-1.0)*maPushDirection );
                mpEnteringBitmap->clip( maClipPolygon1 );
                mpEnteringBitmap->draw( pCanvas );

                // render even strips
                mpEnteringBitmap->move( aPageOrigin + (1.0-t)*maPushDirection );
                mpEnteringBitmap->clip( maClipPolygon2 );
                mpEnteringBitmap->draw( pCanvas );
            }

            return true;
        }

        double CombTransition::getUnderlyingValue() const
        {
            return 0.0;     // though this should be used in concert with
                            // ActivitiesFactory::createSimpleActivity, better
                            // explicitely name our start value.
                            // Permissible range for operator() above is [0,1]
        }

        void CombTransition::addView( const ViewSharedPtr& rView )
        {
            // TODO(Q2): Try to use UnoViewContainer here.
            maViews.push_back( rView );
        }

        bool CombTransition::removeView( const ViewSharedPtr& rView )
        {
            // remove locally
            const ViewVector::iterator aEnd( maViews.end() );
            ViewVector::iterator aIter;
            if( (aIter=::std::remove( maViews.begin(),
                                      aEnd,
                                      rView)) == aEnd )
            {
                // view seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViews.erase( aIter, aEnd );

            return true;
        }

    }
}
