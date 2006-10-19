/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclprocessor2d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:28:25 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_BASEPROCESSOR2D_HXX
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines
class OutputDevice;

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace primitive2d {
    class TextSimplePortionPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class BitmapPrimitive2D;
    class FillBitmapPrimitive2D;
    class PolyPolygonGradientPrimitive2D;
    class PolyPolygonColorPrimitive2D;
    class MetafilePrimitive2D;
    class MaskPrimitive2D;
    class AlphaPrimitive2D;
    class TransformPrimitive2D;
    class MarkerPrimitive2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        class VclProcessor2D : public BaseProcessor2D
        {
        protected:
            // the destination OutDev
            OutputDevice*                                           mpOutputDevice;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                            maBColorModifierStack;

            // the current transformation
            basegfx::B2DHomMatrix                                   maCurrentTransformation;

            // bitfield
            // flag to hold info if output of given OutDev goes to metafile
            unsigned                                                mbOutputToRecordingMetaFile : 1;

            //////////////////////////////////////////////////////////////////////////////
            // rendering support
            void RenderTextSimplePortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate);
            void RenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate);
            void RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
            void RenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapCandidate);
            void RenderPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate);
            void RenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate);
            void RenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rPolygonCandidate);
            void RenderMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate);
            void RenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
            void RenderAlphaPrimitive2D(const primitive2d::AlphaPrimitive2D& rTransCandidate);
            void RenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
            void RenderMarkerPrimitive2D(const primitive2d::MarkerPrimitive2D& rTransCandidate);

        public:
            // constructor/destructor
            VclProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclProcessor2D();

            // the central processing method
            virtual void process(const primitive2d::Primitive2DSequence& rSource);
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPROCESSOR2D_HXX

// eof
