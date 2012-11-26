/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <com/sun/star/lang/Locale.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class DRAWINGLAYER_DLLPUBLIC BaseTextStrikeoutPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// geometric definitions
            basegfx::B2DHomMatrix                   maObjectTransformation;
            double                                  mfWidth;

            /// decoration definitions
            basegfx::BColor                         maFontColor;

        public:
            /// constructor
            BaseTextStrikeoutPrimitive2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                double fWidth,
                const basegfx::BColor& rFontColor);

            /// data read access
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            double getWidth() const { return mfWidth; }
            const basegfx::BColor& getFontColor() const { return maFontColor; }
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class DRAWINGLAYER_DLLPUBLIC TextCharacterStrikeoutPrimitive2D : public BaseTextStrikeoutPrimitive2D
        {
        private:
            sal_Unicode                             maStrikeoutChar;
            attribute::FontAttribute                maFontAttribute;
            ::com::sun::star::lang::Locale          maLocale;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            TextCharacterStrikeoutPrimitive2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                double fWidth,
                const basegfx::BColor& rFontColor,
                sal_Unicode aStrikeoutChar,
                const attribute::FontAttribute& rFontAttribute,
                const ::com::sun::star::lang::Locale& rLocale);

            /// data read access
            sal_Unicode getStrikeoutChar() const { return maStrikeoutChar; }
            const attribute::FontAttribute& getFontAttribute() const { return maFontAttribute; }
            const ::com::sun::star::lang::Locale& getLocale() const { return maLocale; }

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class DRAWINGLAYER_DLLPUBLIC TextGeometryStrikeoutPrimitive2D : public BaseTextStrikeoutPrimitive2D
        {
        private:
            double                                  mfHeight;
            double                                  mfOffset;
            TextStrikeout                           meTextStrikeout;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            TextGeometryStrikeoutPrimitive2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                double fWidth,
                const basegfx::BColor& rFontColor,
                double fHeight,
                double fOffset,
                TextStrikeout eTextStrikeout);

            /// data read access
            double getHeight() const { return mfHeight; }
            double getOffset() const { return mfOffset; }
            TextStrikeout getTextStrikeout() const { return meTextStrikeout; }

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
