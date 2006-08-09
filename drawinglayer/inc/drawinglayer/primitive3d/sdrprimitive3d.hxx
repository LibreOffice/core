/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrprimitive3d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:38:13 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE3D_PRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/primitive3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class sdrPrimitive3D : public basePrimitive3D
        {
        protected:
            basegfx::B3DHomMatrix                       maTransform;
            basegfx::B2DVector                      maTextureSize;
            attribute::sdrLineFillShadowAttribute       maSdrLFSAttribute;
            attribute::sdr3DObjectAttribute             maSdr3DObjectAttribute;

            // Standard implementation for primitive3D which
            // will use maTransform as range and expand by evtl. line width / 2
            basegfx::B3DRange getStandard3DRange() const;

            // implementation for primitive3D which
            // will use given slices and expand by evtl. line width / 2
            basegfx::B3DRange get3DRangeFromSlices(const sliceVector& rSlices) const;

        public:
            sdrPrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::sdrLineFillShadowAttribute& rSdrLFSAttribute,
                const attribute::sdr3DObjectAttribute& rSdr3DObjectAttribute);
            virtual ~sdrPrimitive3D();

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // data access
            const basegfx::B3DHomMatrix& getTransform() const { return maTransform; }
            const basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            const attribute::sdrLineFillShadowAttribute& getSdrLFSAttribute() const { return maSdrLFSAttribute; }
            const attribute::sdr3DObjectAttribute getSdr3DObjectAttribute() const { return maSdr3DObjectAttribute; }
        };
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PRIMITIVE3D_SDRCUBEPRIMITIVE3D_HXX

// eof
