/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdecoratedprimitive2d.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX

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
        class BaseTextStrikeoutPrimitive2D : public BufferedDecompositionPrimitive2D
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
        class TextCharacterStrikeoutPrimitive2D : public BaseTextStrikeoutPrimitive2D
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
        class TextGeometryStrikeoutPrimitive2D : public BaseTextStrikeoutPrimitive2D
        {
        private:
            double                                  mfHeight;
            double                                  mfOffset;
            FontStrikeout                           meFontStrikeout;

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
                FontStrikeout eFontStrikeout);

            /// data read access
            double getHeight() const { return mfHeight; }
            double getOffset() const { return mfOffset; }
            FontStrikeout getFontStrikeout() const { return meFontStrikeout; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTSTRIKEOUTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
