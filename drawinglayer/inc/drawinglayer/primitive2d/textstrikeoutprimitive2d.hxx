/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;
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

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

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

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
