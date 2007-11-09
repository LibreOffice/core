/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidebitmap.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:16:18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <slidebitmap.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>

#include <canvas/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {

        SlideBitmap::SlideBitmap( const ::cppcanvas::BitmapSharedPtr& rBitmap ) :
            maOutputPos(),
            maClipPoly(),
            mxBitmap()
        {
            if( rBitmap )
                mxBitmap = rBitmap->getUNOBitmap();

            ENSURE_AND_THROW( mxBitmap.is(), "SlideBitmap::SlideBitmap(): Invalid bitmap" );
        }

        bool SlideBitmap::draw( const ::cppcanvas::CanvasSharedPtr& rCanvas ) const
        {
            ENSURE_AND_RETURN( rCanvas && rCanvas->getUNOCanvas().is(),
                               "SlideBitmap::draw(): Invalid canvas" );

            // selectively only copy the transformation from current viewstate,
            // don't want no clipping here.
            rendering::ViewState aViewState;
            aViewState.AffineTransform = rCanvas->getViewState().AffineTransform;

            rendering::RenderState aRenderState;
            ::canvas::tools::initRenderState( aRenderState );

            ::basegfx::B2DHomMatrix aTranslation;
            aTranslation.translate( maOutputPos.getX(),
                                    maOutputPos.getY() );
            ::canvas::tools::setRenderStateTransform( aRenderState, aTranslation );

            try
            {
                if( maClipPoly.count() )
                {
                    // TODO(P1): Buffer the clip polygon
                    aRenderState.Clip =
                        ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            rCanvas->getUNOCanvas()->getDevice(),
                            maClipPoly );
                }

                rCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                     aViewState,
                                                     aRenderState );
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( false,
                            rtl::OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                return false;
            }

            return true;
        }

        ::basegfx::B2ISize SlideBitmap::getSize() const
        {
            return ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBitmap->getSize() );
        }

        void SlideBitmap::move( const ::basegfx::B2DPoint& rNewPos )
        {
            maOutputPos = rNewPos;
        }

        void SlideBitmap::clip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            maClipPoly = rClipPoly;
        }

        ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >    SlideBitmap::getXBitmap()
        {
        return mxBitmap;
        }

    }
}
