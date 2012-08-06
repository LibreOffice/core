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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/string.hxx>
#include <vcl/font.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vector>
#include <com/sun/star/lang/Locale.hpp>
#include <drawinglayer/attribute/fontattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DPolyPolygon;
    typedef ::std::vector< B2DPolyPolygon > B2DPolyPolygonVector;
}

class OutputDevice;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextSimplePortionPrimitive2D class

            This is the basic primitive for representing a text portion. It contains
            all needed information. If it is not handled by a renderer, it's decomposition
            will provide the text PolyPolygon outlines as filled polygons, correctly
            transformed.

            To get better text quality, it is suggested to handle tis primitive directly
            in a renderer. In that case, e.g. hintings on the system can be supported.

            @param maTextTransform
            The text transformation contains the text start position (always baselined)
            as translation, the FontSize as scale (where width relative to height defines
            font scaling and width == height means no font scaling) and the font rotation
            and shear.
            When shear is used and a renderer does not support it, it may be better to use
            the decomposition which will do everything correctly. Same is true for mirroring
            which would be expressed as negative scalings.

            @param rText
            The text to be used. Only a part may be used, but a bigger part of the string
            may be necessary for correct layouting (e.g. international)

            @param aTextPosition
            The index to the first character to use from rText

            @param aTextLength
            The number of characters to use from rText

            @param rDXArray
            The distances between the characters. This parameter may be empty, in that case
            the renderer is responsible to do something useful. If it is given, it has to be of
            the size aTextLength. Its values are in logical coordinates and describe the
            distance for each character to use. This is independent from the font width which
            is given with maTextTransform. The first value is the offset to use from the start
            point in FontCoordinateSystem X-Direction (given by maTextTransform) to the start
            point of the second character

            @param rFontAttribute
            The font definition

            @param rLocale
            The locale to use

            @param rFontColor
            The font color to use
         */
        class DRAWINGLAYER_DLLPUBLIC TextSimplePortionPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// text transformation (FontCoordinateSystem)
            basegfx::B2DHomMatrix                   maTextTransform;

            /// The text, used from maTextPosition up to maTextPosition + maTextLength
            String                                  maText;

            /// The index from where on maText is used
            xub_StrLen                              maTextPosition;

            /// The length for maText usage, starting from maTextPosition
            xub_StrLen                              maTextLength;

            /// The DX array in logic units
            ::std::vector< double >                 maDXArray;

            /// The font definition
            attribute::FontAttribute                maFontAttribute;

            /// The Locale for the text
            ::com::sun::star::lang::Locale          maLocale;

            /// font color
            basegfx::BColor                         maFontColor;

            /// #i96669# internal: add simple range buffering for this primitive
            basegfx::B2DRange                       maB2DRange;
            bool                                    mbFilled;           // Whether to fill a given width with the text
            long                                    mnWidthToFill;      // the width to fill

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            TextSimplePortionPrimitive2D(
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const attribute::FontAttribute& rFontAttribute,
                const ::com::sun::star::lang::Locale& rLocale,
                const basegfx::BColor& rFontColor,
                bool bFilled = false,
                long nWidthToFill = 0);

            /// helpers
            /** get text outlines as polygons and their according ObjectTransformation. Handles all
                the necessary VCL outline extractins, scaling adaptions and other stuff.
             */
            void getTextOutlinesAndTransformation(basegfx::B2DPolyPolygonVector& rTarget, basegfx::B2DHomMatrix& rTransformation) const;

            /// data read access
            const basegfx::B2DHomMatrix& getTextTransform() const { return maTextTransform; }
            const String& getText() const { return maText; }
            xub_StrLen getTextPosition() const { return maTextPosition; }
            xub_StrLen getTextLength() const { return maTextLength; }
            const ::std::vector< double >& getDXArray() const { return maDXArray; }
            const attribute::FontAttribute& getFontAttribute() const { return maFontAttribute; }
            const ::com::sun::star::lang::Locale& getLocale() const { return  maLocale; }
            const basegfx::BColor& getFontColor() const { return maFontColor; }
            bool isFilled() const { return mbFilled; }
            long getWidthToFill() const { return mnWidthToFill; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };

        /// small helper to have a compare operator for Locale
        bool DRAWINGLAYER_DLLPUBLIC LocalesAreEqual(const ::com::sun::star::lang::Locale& rA, const ::com::sun::star::lang::Locale& rB);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
