/*************************************************************************
 *
 *  $RCSfile: textaction.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-10 13:26:35 $
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

#include <textaction.hxx>
#include <outdevstate.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/rendering/XCanvas.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#include <mtftools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        void TextAction::init( const ::Point&                                                   rStartPoint,
                               const OutDevState&                                               rState,
                               const CanvasSharedPtr&                                           rCanvas,
                               const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&    rTextTransform       )
        {
            // ensure that mxFont is valid. It is possible that text actions
            // are generated without previously setting a font. Then, just
            // take a default font
            if( !mxFont.is() )
            {
                // Use completely default FontRequest
                const rendering::FontRequest aFontRequest;

                geometry::Matrix2D aFontMatrix;
                ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

                mxFont =
                    mpCanvas->getUNOCanvas()->createFont( aFontRequest,
                                                          uno::Sequence< beans::PropertyValue >(),
                                                          aFontMatrix );
            }

            tools::initRenderState(maState,rState);

            // #i36950# Offset clip back to origin (as it's also moved
            // by rStartPoint!)
            ::Point aEmptyPoint;
            if( rStartPoint != aEmptyPoint )
            {
                if( rState.clip.count() )
                {
                    ::basegfx::B2DPolyPolygon aLocalClip( rState.clip );
                    ::basegfx::B2DHomMatrix   aTranslate;

                    aTranslate.translate( -rStartPoint.X(),
                                          -rStartPoint.Y() );
                    aLocalClip.transform( aTranslate );

                    maState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        aLocalClip );
                }
                else if( !rState.clipRect.IsEmpty() )
                {
                    ::Rectangle aLocalClipRect( rState.clipRect );

                    aLocalClipRect.Move( -rStartPoint.X(),
                                         -rStartPoint.Y() );

                    maState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                ::basegfx::B2DRectangle( aLocalClipRect.Left(),
                                                         aLocalClipRect.Top(),
                                                         aLocalClipRect.Right(),
                                                         aLocalClipRect.Bottom() ) ) ) );
                }
            }

            // TODO(F3): Also inversely-transform clip with
            // rTextTransform!
            if( rTextTransform.isValid() )
            {
                // prepend extra font transform to render state
                // (prepend it, because it's interpreted in the unit
                // rect coordinate space)
                ::canvas::tools::prependToRenderState( maState,
                                                       rTextTransform.getValue() );
            }

            ::basegfx::B2DHomMatrix aLocalTransformation( rState.fontTransform );

            aLocalTransformation.translate( rStartPoint.X(),
                                            rStartPoint.Y() );
            ::canvas::tools::appendToRenderState( maState,
                                                  aLocalTransformation );

            // append text transform, if given

            maState.DeviceColor = rState.textColor;

            if( maOffsets.getLength() )
            {
                mxTextLayout = mxFont->createTextLayout( maStringContext, rState.textDirection, 0 );

                if( mxTextLayout.is() )
                    mxTextLayout->applyLogicalAdvancements( maOffsets );
            }
        }

        TextAction::TextAction( const ::Point&                                                  rStartPoint,
                                const ::rtl::OUString&                                          rText,
                                sal_Int32                                                       nStartPos,
                                sal_Int32                                                       nLen,
                                const CanvasSharedPtr&                                          rCanvas,
                                const OutDevState&                                              rState,
                                const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&   rTextTransform  ) :
            mxFont( rState.xFont ),
            maStringContext( rText, nStartPos, nLen ),
            maOffsets(),
            mpCanvas( rCanvas ),
            maState(),
            maTextDirection( rState.textDirection )
        {
            init( rStartPoint, rState, rCanvas, rTextTransform );
        }

        TextAction::TextAction( const ::Point&                                                  rStartPoint,
                                const ::rtl::OUString&                                          rText,
                                sal_Int32                                                       nStartPos,
                                sal_Int32                                                       nLen,
                                ::com::sun::star::uno::Sequence< double >                       aOffsets,
                                const CanvasSharedPtr&                                          rCanvas,
                                const OutDevState&                                              rState,
                                const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&   rTextTransform  ) :
            mxFont( rState.xFont ),
            maStringContext( rText, nStartPos, nLen ),
            maOffsets( aOffsets ),
            mpCanvas( rCanvas ),
            maState(),
            maTextDirection( rState.textDirection )
        {
            init( rStartPoint, rState, rCanvas, rTextTransform );
        }

        TextAction::~TextAction()
        {
        }

        bool TextAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TextAction::render()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TextAction: 0x%X", this );

            rendering::RenderState aLocalState( maState );
            ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

            if( mxTextLayout.is() )
                mpCanvas->getUNOCanvas()->drawTextLayout( mxTextLayout, mpCanvas->getViewState(), aLocalState );
            else
                mpCanvas->getUNOCanvas()->drawText( maStringContext, mxFont,
                                                    mpCanvas->getViewState(), aLocalState, maTextDirection );

            return true;
        }

    }
}
