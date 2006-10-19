/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadow3dextractor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:33:20 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class Shadow3DExtractingProcessor : public BaseProcessor3D
        {
        private:
            // result holding vector (2D) and target vector for stacking (inited to &maPrimitive2DSequence)
            primitive2d::Primitive2DSequence                maPrimitive2DSequence;
            primitive2d::Primitive2DSequence*               mpPrimitive2DSequence;

            // the 3d transformation stack
            const geometry::Transformation3D                maTransformation3D;

            // prepared data (transformations) for 2D/3D shadow calculations
            basegfx::B3DHomMatrix                           maWorldToEye;
            basegfx::B3DHomMatrix                           maEyeToView;
            basegfx::B3DHomMatrix                           maWorldToView;
            basegfx::B3DVector                              maLightNormal;
            basegfx::B3DVector                              maShadowPlaneNormal;
            basegfx::B3DPoint                               maPlanePoint;
            double                                          mfLightPlaneScalar;

            // the shadow color used for sub-primitives. Can stay at black since
            // the encapsulating 2d shadow primitive will contain the color
            basegfx::BColor                                 maPrimitiveColor;

            // bitfield
            // flag if shadow plane projection preparation leaded to valid results
            unsigned                                        mbShadowProjectionIsValid : 1;

            // flag if conversion is switched on
            unsigned                                        mbConvert : 1;

            // flag if conversion shall use projection
            unsigned                                        mbUseProjection : 1;

            // helpers
            basegfx::B2DPolygon impDoShadowProjection(const basegfx::B3DPolygon& rSource);
            basegfx::B2DPolyPolygon impDoShadowProjection(const basegfx::B3DPolyPolygon& rSource);

        public:
            Shadow3DExtractingProcessor(
                double fTime,
                const geometry::Transformation3D& rTransformation3D,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                const primitive3d::Primitive3DSequence& rPrimitiveVector,
                double fShadowSlant);

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const primitive2d::Primitive2DSequence& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
            const geometry::Transformation3D& getTransformation3D() const { return maTransformation3D; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

#endif //_DRAWINGLAYER_PROCESSOR3D_SHADOW3DEXTRACTOR_HXX

// eof
