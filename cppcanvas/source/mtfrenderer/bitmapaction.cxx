/*************************************************************************
 *
 *  $RCSfile: bitmapaction.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:54:03 $
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

#include <bitmapaction.hxx>
#include <outdevstate.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XBitmap.hpp>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <mtftools.hxx>


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
                moved rPoint away.
            */
            void implSetupTransform( rendering::RenderState&    rRenderState,
                                     const Point&               rPoint          )
            {
                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.translate( rPoint.X(),
                                                rPoint.Y() );
                ::canvas::tools::appendToRenderState( rRenderState,
                                                      aLocalTransformation );
            }

            /** Setup transformation such that the next render call is
                moved rPoint away, and scaled according to the ratio
                given by src and dst size.
            */
            void implSetupTransform( rendering::RenderState&    rRenderState,
                                     const Point&               rPoint,
                                     const Size&                rSrcSize,
                                     const Size&                rDstSize        )
            {
                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.scale( static_cast<double>(rDstSize.Width()) / rSrcSize.Width(),
                                            static_cast<double>(rDstSize.Height()) / rSrcSize.Height() );
                aLocalTransformation.translate( rPoint.X(),
                                                rPoint.Y() );
                ::canvas::tools::appendToRenderState( rRenderState,
                                                      aLocalTransformation );
            }

            /** Setup transformation such that the next render call
                paints the content given by the src area into the dst
                area. No clipping is set whatsoever.
            */
            void implSetupTransform( rendering::RenderState&    rRenderState,
                                     const Point&               rSrcPoint,
                                     const Size&                rSrcSize,
                                     const Point&               rDstPoint,
                                     const Size&                rDstSize        )
            {
                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.scale( static_cast<double>(rDstSize.Width()) / rSrcSize.Width(),
                                            static_cast<double>(rDstSize.Height()) / rSrcSize.Height() );
                aLocalTransformation.translate( rDstPoint.X() - rSrcPoint.X(),
                                                rDstPoint.Y() - rSrcPoint.Y() );
                ::canvas::tools::appendToRenderState( rRenderState,
                                                      aLocalTransformation );
            }
        }

        BitmapAction::BitmapAction( const ::BitmapEx&       rBmpEx,
                                    const ::Point&          rDstPoint,
                                    const CanvasSharedPtr&  rCanvas,
                                    const OutDevState&      rState ) :
            mxBitmap( ::vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                            rBmpEx ) ),
            mpCanvas( rCanvas ),
            maState()
        {
            tools::initRenderState(maState,rState);
            implSetupTransform( maState, rDstPoint );
        }

        BitmapAction::BitmapAction( const ::BitmapEx&       rBmpEx,
                                    const ::Point&          rDstPoint,
                                    const ::Size&           rDstSize,
                                    const CanvasSharedPtr&  rCanvas,
                                    const OutDevState&      rState      ) :
            mxBitmap( ::vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                            rBmpEx ) ),
            mpCanvas( rCanvas ),
            maState()
        {
            tools::initRenderState(maState,rState);
            implSetupTransform( maState, rDstPoint, rBmpEx.GetSizePixel(), rDstSize );
        }

        BitmapAction::BitmapAction( const ::BitmapEx&       rBmpEx,
                                    const ::Point&          rSrcPoint,
                                    const ::Size&           rSrcSize,
                                    const ::Point&          rDstPoint,
                                    const ::Size&           rDstSize,
                                    const CanvasSharedPtr&  rCanvas,
                                    const OutDevState&      rState      ) :
            mxBitmap( ::vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                            rBmpEx ) ),
            mpCanvas( rCanvas ),
            maState()
        {
            tools::initRenderState(maState,rState);

            // TODO(F2): setup clipping/extract only part of the bitmap
            implSetupTransform( maState, rSrcPoint, rSrcSize, rDstPoint, rDstSize );
        }

        BitmapAction::~BitmapAction()
        {
        }

        bool BitmapAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::BitmapAction::render()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::BitmapAction: 0x%X", this );

            rendering::RenderState aLocalState( maState );
            ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

            mpCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                  mpCanvas->getViewState(),
                                                  aLocalState );

            return true;
        }

    }
}
