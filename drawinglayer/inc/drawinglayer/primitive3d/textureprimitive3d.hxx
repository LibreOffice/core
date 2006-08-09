/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.hxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE3D_VECTORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/vectorprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class texturePrimitive3D : public vectorPrimitive3D
        {
        protected:
            basegfx::B2DVector                      maTextureSize;

            // bitfield
            // flag if texture shall be modulated with white interpolated color
            unsigned                                    mbModulate : 1;

            // flag if texture shall be filtered
            unsigned                                    mbFilter : 1;

        public:
            texturePrimitive3D(
                const primitiveVector3D& rPrimitiveVector,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~texturePrimitive3D();

            // get data
            const basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class simpleTransparenceTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            double                                      mfTransparence;

            //  create decomposition
            virtual void decompose(primitiveVector3D& rTarget);

        public:
            simpleTransparenceTexturePrimitive3D(
                double fTransparence,
                const primitiveVector3D& rPrimitiveVector);
            virtual ~simpleTransparenceTexturePrimitive3D();

            // get data
            double getTransparence() const { return mfTransparence; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class gradientTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            attribute::fillGradientAttribute        maGradient;

            //  create decomposition
            virtual void decompose(primitiveVector3D& rTarget);

        public:
            gradientTexturePrimitive3D(
                const attribute::fillGradientAttribute& rGradient,
                const primitiveVector3D& rPrimitiveVector,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~gradientTexturePrimitive3D();

            // get data
            const attribute::fillGradientAttribute& getGradient() const { return maGradient; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class bitmapTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            attribute::fillBitmapAttribute      maBitmap;

            //  create decomposition
            virtual void decompose(primitiveVector3D& rTarget);

        public:
            bitmapTexturePrimitive3D(
                const attribute::fillBitmapAttribute& rBitmap,
                const primitiveVector3D& rPrimitiveVector,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~bitmapTexturePrimitive3D();

            // get data
            const attribute::fillBitmapAttribute& getBitmap() const { return maBitmap; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class transparenceTexturePrimitive3D : public gradientTexturePrimitive3D
        {
        public:
            transparenceTexturePrimitive3D(
                const attribute::fillGradientAttribute& rGradient,
                const primitiveVector3D& rPrimitiveVector,
                const basegfx::B2DVector& rTextureSize);
            virtual ~transparenceTexturePrimitive3D();

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX

// eof
