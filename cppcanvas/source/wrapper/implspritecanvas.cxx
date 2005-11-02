/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implspritecanvas.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:44:26 $
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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_INTERPOLATIONMODE_HPP_
#include <com/sun/star/rendering/InterpolationMode.hpp>
#endif

#include <implspritecanvas.hxx>
#include <implcustomsprite.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplSpriteCanvas::TransformationArbiter::TransformationArbiter() :
            maTransformation()
        {
        }

        void ImplSpriteCanvas::TransformationArbiter::setTransformation( const ::basegfx::B2DHomMatrix& rViewTransform )
        {
            maTransformation = rViewTransform;
        }

        ::basegfx::B2DHomMatrix ImplSpriteCanvas::TransformationArbiter::getTransformation() const
        {
            return maTransformation;
        }


        ImplSpriteCanvas::ImplSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& rCanvas ) :
            ImplCanvas( uno::Reference< rendering::XCanvas >(rCanvas,
                                                             uno::UNO_QUERY) ),
            ImplBitmapCanvas( uno::Reference< rendering::XBitmapCanvas >(rCanvas,
                                                                         uno::UNO_QUERY) ),
            mxSpriteCanvas( rCanvas ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas(): Invalid canvas" );
        }

        ImplSpriteCanvas::ImplSpriteCanvas(const ImplSpriteCanvas& rOrig) :
            ImplCanvas( rOrig ),
            ImplBitmapCanvas( rOrig ),
            mxSpriteCanvas( rOrig.getUNOSpriteCanvas() ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas( const ImplSpriteCanvas& ): Invalid canvas" );

            mpTransformArbiter->setTransformation( getTransformation() );
        }

        ImplSpriteCanvas::~ImplSpriteCanvas()
        {
        }

        void ImplSpriteCanvas::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            mpTransformArbiter->setTransformation( rMatrix );

            ImplCanvas::setTransformation( rMatrix );
        }

        bool ImplSpriteCanvas::updateScreen( bool bUpdateAll ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::updateScreen(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return false;

            return mxSpriteCanvas->updateScreen( bUpdateAll );
        }

        CustomSpriteSharedPtr ImplSpriteCanvas::createCustomSprite( const ::basegfx::B2DSize& rSize ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::createCustomSprite(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return CustomSpriteSharedPtr();

            return CustomSpriteSharedPtr(
                new ImplCustomSprite( mxSpriteCanvas,
                                      mxSpriteCanvas->createCustomSprite( ::basegfx::unotools::size2DFromB2DSize(rSize) ),
                                      mpTransformArbiter ) );
        }

        SpriteSharedPtr ImplSpriteCanvas::createClonedSprite( const SpriteSharedPtr& rSprite ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::createCustomSprite(): Invalid canvas" );
            OSL_ENSURE( rSprite.get() != NULL && rSprite->getUNOSprite().is(),
                        "ImplSpriteCanvas::createCustomSprite(): Invalid sprite" );

            if( !mxSpriteCanvas.is() ||
                rSprite.get() == NULL ||
                !rSprite->getUNOSprite().is() )
            {
                return SpriteSharedPtr();
            }

            return SpriteSharedPtr(
                new ImplSprite( mxSpriteCanvas,
                                mxSpriteCanvas->createClonedSprite( rSprite->getUNOSprite() ),
                                mpTransformArbiter ) );
        }

        SpriteSharedPtr ImplSpriteCanvas::createSpriteFromBitmaps( const uno::Sequence< uno::Reference< rendering::XBitmap > >& rAnimationBitmaps,
                                                                   sal_Int8                                                     nInterpolationMode )
        {
            return SpriteSharedPtr( new internal::ImplSprite( mxSpriteCanvas,
                                                              mxSpriteCanvas->createSpriteFromBitmaps( rAnimationBitmaps,
                                                                                                       rendering::InterpolationMode::NEAREST_NEIGHBOR ),
                                                              mpTransformArbiter ) );
        }

        CanvasSharedPtr ImplSpriteCanvas::clone() const
        {
            return SpriteCanvasSharedPtr( new ImplSpriteCanvas( *this ) );
        }

        uno::Reference< rendering::XSpriteCanvas > ImplSpriteCanvas::getUNOSpriteCanvas() const
        {
            return mxSpriteCanvas;
        }

    }
}
