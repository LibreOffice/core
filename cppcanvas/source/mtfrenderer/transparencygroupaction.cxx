/*************************************************************************
 *
 *  $RCSfile: transparencygroupaction.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:57:31 $
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

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <transparencygroupaction.hxx>
#include <outdevstate.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XBitmap.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif

#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <mtftools.hxx>
#include <cppcanvas/vclfactory.hxx>


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        // free support functions
        // ======================
        namespace
        {
            /** Setup transformation such that the next render call is
                moved rPoint away, and scaled according to the ratio
                given by src and dst size.
            */
            void implSetupTransform( rendering::RenderState&    rRenderState,
                                     const Point&               rDstPoint   )
            {
                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.translate( rDstPoint.X(),
                                                rDstPoint.Y() );
                ::canvas::tools::appendToRenderState( rRenderState,
                                                      aLocalTransformation );
            }
        }

        TransparencyGroupAction::TransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                          const Renderer::Parameters&   rParms,
                                                          const ::Point&                rDstPoint,
                                                          const ::Size&                 rDstSize,
                                                          double                        nAlpha,
                                                          const CanvasSharedPtr&        rCanvas,
                                                          const OutDevState&            rState ) :
            mpGroupMtf( rGroupMtf ),
            mpAlphaGradient(),
            maParms( rParms ),
            maDstSize( rDstSize ),
            mxBufferBitmap(),
            maLastTransformation(),
            mpCanvas( rCanvas ),
            maState(),
            mnAlpha( nAlpha )
        {
            tools::initRenderState(maState,rState);
            implSetupTransform( maState, rDstPoint );
        }

        TransparencyGroupAction::TransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                          GradientAutoPtr&              rAlphaGradient,
                                                          const Renderer::Parameters&   rParms,
                                                          const ::Point&                rDstPoint,
                                                          const ::Size&                 rDstSize,
                                                          const CanvasSharedPtr&        rCanvas,
                                                          const OutDevState&            rState ) :
            mpGroupMtf( rGroupMtf ),
            mpAlphaGradient( rAlphaGradient ),
            maParms( rParms ),
            maDstSize( rDstSize ),
            mxBufferBitmap(),
            maLastTransformation(),
            mpCanvas( rCanvas ),
            maState(),
            mnAlpha( 1.0 )
        {
            tools::initRenderState(maState,rState);
            implSetupTransform( maState, rDstPoint );
        }

        TransparencyGroupAction::~TransparencyGroupAction()
        {
            // outline, because of incomplete types in header
        }

        // TODO(P3): The whole float transparency handling is a mess,
        // this should be refactored. What's more, the old idea of
        // having only internal 'metaactions', and not the original
        // GDIMetaFile now looks a lot less attractive. Try to move
        // into the direction of having a direct GDIMetaFile2XCanvas
        // renderer, and maybe a separate metafile XCanvas
        // implementation.
        bool TransparencyGroupAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TransparencyGroupAction::render()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TransparencyGroupAction: 0x%X", this );

            // determine overall transformation matrix (render, view,
            // and passed transformation)
            ::basegfx::B2DHomMatrix aTransform;
            ::canvas::tools::getRenderStateTransform( aTransform, maState );
            aTransform = rTransformation * aTransform;

            ::basegfx::B2DHomMatrix aTotalTransform;
            ::canvas::tools::getViewStateTransform( aTotalTransform, mpCanvas->getViewState() );
            aTotalTransform = aTotalTransform * aTransform;

            // since pure translational changes to the transformation
            // does not matter, remove them before comparing
            aTotalTransform.set( 0, 2, 0.0 );
            aTotalTransform.set( 1, 2, 0.0 );

            // as soon as the total transformation changes, we've got
            // to re-render the bitmap
            if( aTotalTransform != maLastTransformation)
            {
                DBG_TESTSOLARMUTEX();

                // determine total scaling factor of the
                // transformation matrix - need to make the bitmap
                // large enough
                ::basegfx::B2DTuple aScale;
                ::basegfx::B2DTuple aTranslate;
                double              nRotate;
                double              nShearX;
                if( !aTotalTransform.decompose( aScale,
                                                aTranslate,
                                                nRotate,
                                                nShearX ) )
                {
                    OSL_ENSURE( false,
                                "TransparencyGroupAction::render(): non-decomposable transformation" );
                    return false;
                }

                ::Size aSize( ::basegfx::fround( aScale.getX() * maDstSize.Width() ),
                              ::basegfx::fround( aScale.getY() * maDstSize.Height() ) );

                // render our content into an appropriately sized
                // VirtualDevice with alpha channel
                VirtualDevice aVDev(
                    *::Application::GetDefaultDevice(), 0, 0 );
                aVDev.SetOutputSizePixel( aSize );
                aVDev.SetMapMode();

                ::Point aEmptyPoint;
                aVDev.DrawTransparent( *mpGroupMtf,
                                       aEmptyPoint,
                                       aSize,
                                       *mpAlphaGradient );


                // update buffered bitmap and transformation
                BitmapSharedPtr aBmp( VCLFactory::getInstance().createBitmap(
                                          mpCanvas,
                                          aVDev.GetBitmapEx(
                                              aEmptyPoint,
                                              aSize ) ) );
                mxBufferBitmap = aBmp->getUNOBitmap();
                maLastTransformation = aTotalTransform;
            }

            // determine target transformation (we can't simply pass
            // aTotalTransform as assembled above, since we must take
            // the canvas' view state as is, it might contain clipping
            // (which, in turn, is relative to the view
            // transformation))

            // given that aTotalTransform is the identity
            // transformation, we could simply render our bitmap
            // as-is. Now, since the mxBufferBitmap content already
            // accounts for scale changes in the overall
            // transformation, we must factor this out
            // before. Generally, the transformation matrix should be
            // structured like this:
            // Translation*Rotation*Shear*Scale. Thus, to neutralize
            // the contained scaling, we've got to right-multiply with
            // the inverse.
            ::basegfx::B2ISize aBmpSize(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBufferBitmap->getSize() ) );

            ::basegfx::B2DHomMatrix aScaleCorrection;
            aScaleCorrection.scale( (double)maDstSize.Width() / aBmpSize.getX(),
                                    (double)maDstSize.Height() / aBmpSize.getY() );
            aTransform = aTransform * aScaleCorrection;

            rendering::RenderState aLocalState( maState );
            ::canvas::tools::setRenderStateTransform(aLocalState, aTransform);

            if( ::rtl::math::approxEqual(mnAlpha, 1.0) )
            {
                // no further alpha changes necessary -> draw directly
                mpCanvas->getUNOCanvas()->drawBitmap( mxBufferBitmap,
                                                      mpCanvas->getViewState(),
                                                      aLocalState );
            }
            else
            {
                // add alpha modulation value to DeviceColor
                aLocalState.DeviceColor.realloc(4);
                aLocalState.DeviceColor[0] = 1.0;
                aLocalState.DeviceColor[1] = 1.0;
                aLocalState.DeviceColor[2] = 1.0;
                aLocalState.DeviceColor[3] = mnAlpha;
                mpCanvas->getUNOCanvas()->drawBitmapModulated( mxBufferBitmap,
                                                               mpCanvas->getViewState(),
                                                               aLocalState );
            }

            return true;
        }

    }
}
