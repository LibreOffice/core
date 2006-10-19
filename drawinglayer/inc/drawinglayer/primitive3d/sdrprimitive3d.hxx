/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrprimitive3d.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:32:40 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class SdrPrimitive3D : public BasePrimitive3D
        {
        private:
            basegfx::B3DHomMatrix                       maTransform;
            basegfx::B2DVector                          maTextureSize;
            attribute::SdrLineFillShadowAttribute       maSdrLFSAttribute;
            attribute::Sdr3DObjectAttribute             maSdr3DObjectAttribute;

        protected:
            // Standard implementation for primitive3D which
            // will use maTransform as range and expand by evtl. line width / 2
            basegfx::B3DRange getStandard3DRange() const;

            // implementation for primitive3D which
            // will use given Slice3Ds and expand by evtl. line width / 2
            basegfx::B3DRange get3DRangeFromSlices(const Slice3DVector& rSlices) const;

        public:
            SdrPrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute);

            // data access
            const basegfx::B3DHomMatrix& getTransform() const { return maTransform; }
            const basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            const attribute::SdrLineFillShadowAttribute& getSdrLFSAttribute() const { return maSdrLFSAttribute; }
            const attribute::Sdr3DObjectAttribute getSdr3DObjectAttribute() const { return maSdr3DObjectAttribute; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
