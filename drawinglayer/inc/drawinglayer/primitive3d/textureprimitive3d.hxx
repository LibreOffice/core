/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-05-12 11:45:15 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE_LISTPRIMITIVE_HXX
#include <drawinglayer/primitive/listprimitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_FILLATTRIBUTE_HXX
#include <drawinglayer/primitive/fillattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/primitive/fillbitmapattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_PRIMITIVELIST_HXX
#include <drawinglayer/primitive/primitivelist.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class texturePrimitive3D : public listPrimitive
        {
        protected:
            ::basegfx::B2DVector                        maTextureSize;

            // bitfield
            // flag if texture shall be modulated with white interpolated color
            unsigned                                    mbModulate : 1;

            // flag if texture shall be filtered
            unsigned                                    mbFilter : 1;

        public:
            texturePrimitive3D(
                const primitiveList& rPrimitiveList,
                const ::basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~texturePrimitive3D();

            // get data
            const ::basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class simpleTransparenceTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            double                                      mfTransparence;

            //  create decomposition
            virtual void decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation);

        public:
            simpleTransparenceTexturePrimitive3D(
                double fTransparence,
                const primitiveList& rPrimitiveList);
            virtual ~simpleTransparenceTexturePrimitive3D();

            // get data
            double getTransparence() const { return mfTransparence; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // clone operator
            virtual basePrimitive* createNewClone() const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class gradientTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            fillGradientAttribute                       maGradient;

            //  create decomposition
            virtual void decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation);

        public:
            gradientTexturePrimitive3D(
                const fillGradientAttribute& rGradient,
                const primitiveList& rPrimitiveList,
                const ::basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~gradientTexturePrimitive3D();

            // get data
            const fillGradientAttribute& getGradient() const { return maGradient; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // clone operator
            virtual basePrimitive* createNewClone() const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class bitmapTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            fillBitmapAttribute                         maBitmap;

            //  create decomposition
            virtual void decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation);

        public:
            bitmapTexturePrimitive3D(
                const fillBitmapAttribute& rBitmap,
                const primitiveList& rPrimitiveList,
                const ::basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~bitmapTexturePrimitive3D();

            // get data
            const fillBitmapAttribute& getBitmap() const { return maBitmap; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // clone operator
            virtual basePrimitive* createNewClone() const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class transparenceTexturePrimitive3D : public gradientTexturePrimitive3D
        {
        public:
            transparenceTexturePrimitive3D(
                const fillGradientAttribute& rGradient,
                const primitiveList& rPrimitiveList,
                const ::basegfx::B2DVector& rTextureSize);
            virtual ~transparenceTexturePrimitive3D();

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // clone operator
            virtual basePrimitive* createNewClone() const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX

// eof
