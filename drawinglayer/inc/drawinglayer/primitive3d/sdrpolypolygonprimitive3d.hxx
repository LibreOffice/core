/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpolypolygonprimitive3d.hxx,v $
 *
 *  $Revision: 1.1 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class SdrPolyPolygonPrimitive3D : public SdrPrimitive3D
        {
        private:
            // the polyPolygon evtl with normals and texture coordinates
            basegfx::B3DPolyPolygon                 maPolyPolygon3D;

        protected:
            // local decomposition.
            virtual Primitive3DSequence createLocalDecomposition(double fTime) const;

        public:
            SdrPolyPolygonPrimitive3D(
                const basegfx::B3DPolyPolygon& rPolyPolygon3D,
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute);

            // data access
            const basegfx::B3DPolyPolygon& getPolyPolygon3D() const { return maPolyPolygon3D; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            // provide unique ID
            virtual sal_uInt32 getPrimitiveID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPOLYPOLYGONPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
