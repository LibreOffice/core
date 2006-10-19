/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrextrudeprimitive3d.hxx,v $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDEPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDEPRIMITIVE3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class SdrExtrudePrimitive3D : public SdrPrimitive3D
        {
        private:
            // geometry helper for slices
            basegfx::B2DPolyPolygon                     maCorrectedPolyPolygon;
            Slice3DVector                               maSlices;

            // primitive data
            basegfx::B2DPolyPolygon                     maPolyPolygon;
            double                                      mfDepth;
            double                                      mfDiagonal;
            double                                      mfBackScale;

            // bitfield
            unsigned                                    mbSmoothNormals : 1; // Plane self
            unsigned                                    mbSmoothHorizontalNormals : 1; // always
            unsigned                                    mbSmoothLids : 1; // Front/back
            unsigned                                    mbCharacterMode : 1;
            unsigned                                    mbCloseFront : 1;
            unsigned                                    mbCloseBack : 1;

            // create slices
            void impCreateSlices();

            // get (evtl. create) slices
            const Slice3DVector& getSlices() const;

        protected:
            // local decomposition.
            virtual Primitive3DSequence createLocalDecomposition(double fTime) const;

        public:
            SdrExtrudePrimitive3D(
                const basegfx::B3DHomMatrix& rTransform,
                const basegfx::B2DVector& rTextureSize,
                const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
                const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                double fDepth,
                double fDiagonal,
                double fBackScale,
                bool bSmoothNormals,
                bool bSmoothHorizontalNormals,
                bool bSmoothLids,
                bool bCharacterMode,
                bool bCloseFront,
                bool bCloseBack);

            // data access
            const basegfx::B2DPolyPolygon& getPolyPolygon() const { return maPolyPolygon; }
            double getDepth() const { return mfDepth; }
            double getDiagonal() const { return mfDiagonal; }
            double getBackScale() const { return mfBackScale; }
            bool getSmoothNormals() const { return mbSmoothNormals; }
            bool getSmoothHorizontalNormals() const { return mbSmoothHorizontalNormals; }
            bool getSmoothLids() const { return mbSmoothLids; }
            bool getCharacterMode() const { return mbCharacterMode; }
            bool getCloseFront() const { return mbCloseFront; }
            bool getCloseBack() const { return mbCloseBack; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            // get range
            virtual basegfx::B3DRange getB3DRange(double fTime) const;

            // provide unique ID
            virtual sal_uInt32 getPrimitiveID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDEPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
