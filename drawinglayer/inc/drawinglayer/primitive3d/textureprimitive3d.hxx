/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:21 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** TexturePrimitive3D class

            This 3D grouping primitive is used to define a texture for
            3d geometry by embedding it. It is used as bae class for
            extended texture definitions
         */
        class TexturePrimitive3D : public GroupPrimitive3D
        {
        private:
            /// texture geometry definition
            basegfx::B2DVector                          maTextureSize;

            /// bitfield
            /// flag if texture shall be modulated with white interpolated color
            unsigned                                    mbModulate : 1;

            /// flag if texture shall be filtered
            unsigned                                    mbFilter : 1;

        public:
            /// constructor
            TexturePrimitive3D(
                const Primitive3DSequence& rChildren,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);

            /// data read access
            const basegfx::B2DVector& getTextureSize() const { return maTextureSize; }
            bool getModulate() const { return mbModulate; }
            bool getFilter() const { return mbFilter; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** UnifiedAlphaTexturePrimitive3D class

            This 3D primitive expands TexturePrimitive3D to a unified
            alpha (transparence) texture definition. All 3D primitives
            embedded here will be shown with the given transparency.
         */
        class UnifiedAlphaTexturePrimitive3D : public TexturePrimitive3D
        {
        private:
            /// transparency definition
            double                                      mfTransparence;

        public:
            /// constructor
            UnifiedAlphaTexturePrimitive3D(
                double fTransparence,
                const Primitive3DSequence& rChildren);

            /// data read access
            double getTransparence() const { return mfTransparence; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// local decomposition.
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** GradientTexturePrimitive3D class

            This 3D primitive expands TexturePrimitive3D to a gradient texture
            definition. All 3D primitives embedded here will be shown with the
            defined gradient.
         */
        class GradientTexturePrimitive3D : public TexturePrimitive3D
        {
        private:
            /// the gradient definition
            attribute::FillGradientAttribute        maGradient;

        public:
            /// constructor
            GradientTexturePrimitive3D(
                const attribute::FillGradientAttribute& rGradient,
                const Primitive3DSequence& rChildren,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);

            /// data read access
            const attribute::FillGradientAttribute& getGradient() const { return maGradient; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** BitmapTexturePrimitive3D class

            This 3D primitive expands TexturePrimitive3D to a bitmap texture
            definition. All 3D primitives embedded here will be shown with the
            defined bitmap (maybe tiled if defined).
         */
        class BitmapTexturePrimitive3D : public TexturePrimitive3D
        {
        private:
            /// bitmap fill attribute
            attribute::FillBitmapAttribute      maFillBitmapAttribute;

        public:
            /// constructor
            BitmapTexturePrimitive3D(
                const attribute::FillBitmapAttribute& rFillBitmapAttribute,
                const Primitive3DSequence& rChildren,
                const basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);

            /// data read access
            const attribute::FillBitmapAttribute& getFillBitmapAttribute() const { return maFillBitmapAttribute; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** AlphaTexturePrimitive3D class

            This 3D primitive expands TexturePrimitive3D to a alpha texture
            definition. For alpha definition, a gradient is used. The values in
            that gradient will be interpreted as luminance Alpha-Values. All 3D
            primitives embedded here will be shown with the defined transparence.
         */
        class AlphaTexturePrimitive3D : public GradientTexturePrimitive3D
        {
        public:
            /// constructor
            AlphaTexturePrimitive3D(
                const attribute::FillGradientAttribute& rGradient,
                const Primitive3DSequence& rChildren,
                const basegfx::B2DVector& rTextureSize);

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
