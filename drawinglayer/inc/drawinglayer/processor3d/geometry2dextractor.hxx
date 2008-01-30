/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: geometry2dextractor.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-01-30 12:26:48 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_BASEPROCESSOR3D_HXX
#include <drawinglayer/processor3d/baseprocessor3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLORMODIFIER_HXX
#include <basegfx/color/bcolormodifier.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class Geometry2DExtractingProcessor : public BaseProcessor3D
        {
        private:
            // result holding vector (2D)
            primitive2d::Primitive2DSequence                maPrimitive2DSequence;

            // object transformation for scene for 2d definition
            basegfx::B2DHomMatrix                           maObjectTransformation;

            // prepared data (transformations) for 2D/3D transformation calculations
            // will be used as transformation stack during processing (changed)
            basegfx::B3DHomMatrix                           maWorldToView;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack                    maBColorModifierStack;

        public:
            Geometry2DExtractingProcessor(
                double fTime,
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const basegfx::B3DHomMatrix& rWorldToView);

            // the central processing method
            virtual void process(const primitive3d::Primitive3DSequence& rSource);

            // data access
            const primitive2d::Primitive2DSequence& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const basegfx::B3DHomMatrix& getWorldToView() const { return maWorldToView; }
            const basegfx::BColorModifierStack& getBColorModifierStack() const { return maBColorModifierStack; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_GEOMETRY2DEXTRACTOR_HXX

// eof
