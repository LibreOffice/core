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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx { namespace tools {
    class B2DHomMatrixBufferedOnDemandDecompose;
}}

namespace com { namespace sun { namespace star { namespace i18n {
    struct Boundary;
}}}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TextDecoratedPortionPrimitive2D class

            This primitive expands the TextSimplePortionPrimitive2D by common
            decorations used in the office. It can be decomposed and will create
            a TextSimplePortionPrimitive2D and all the contained decorations (if used)
            as geometry.
         */
        class TextDecoratedPortionPrimitive2D : public TextSimplePortionPrimitive2D
        {
        private:
            /// decoration definitions
            basegfx::BColor                             maOverlineColor;
            basegfx::BColor                             maTextlineColor;
            TextLine                                    meFontOverline;
            TextLine                                    meFontUnderline;
            FontStrikeout                               meFontStrikeout;
            FontEmphasisMark                            meFontEmphasisMark;
            FontRelief                                  meFontRelief;

            /// bitfield
            unsigned                                    mbUnderlineAbove : 1;
            unsigned                                    mbWordLineMode : 1;
            unsigned                                    mbEmphasisMarkAbove : 1;
            unsigned                                    mbEmphasisMarkBelow : 1;
            unsigned                                    mbShadow : 1;

            /// helper methods
            void impCreateGeometryContent(
                std::vector< Primitive2DReference >& rTarget,
                basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans,
                const String& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const attribute::FontAttribute& rFontAttribute) const;

            void impCorrectTextBoundary(
                ::com::sun::star::i18n::Boundary& rNextWordBoundary) const;

            void impSplitSingleWords(
                std::vector< Primitive2DReference >& rTarget,
                basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans) const;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            TextDecoratedPortionPrimitive2D(

                /// TextSimplePortionPrimitive2D parameters
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const attribute::FontAttribute& rFontAttribute,
                const ::com::sun::star::lang::Locale& rLocale,
                const basegfx::BColor& rFontColor,

                /// local parameters
                const basegfx::BColor& rOverlineColor,
                const basegfx::BColor& rTextlineColor,
                TextLine eFontOverline = TEXT_LINE_NONE,
                TextLine eFontUnderline = TEXT_LINE_NONE,
                bool bUnderlineAbove = false,
                FontStrikeout eFontStrikeout = FONT_STRIKEOUT_NONE,
                bool bWordLineMode = false,
                FontEmphasisMark eFontEmphasisMark = FONT_EMPHASISMARK_NONE,
                bool bEmphasisMarkAbove = true,
                bool bEmphasisMarkBelow = false,
                FontRelief eFontRelief = FONT_RELIEF_NONE,
                bool bShadow = false);

            /// data read access
            TextLine getFontOverline() const { return meFontOverline; }
            TextLine getFontUnderline() const { return meFontUnderline; }
            FontStrikeout getFontStrikeout() const { return meFontStrikeout; }
            FontEmphasisMark getFontEmphasisMark() const { return meFontEmphasisMark; }
            FontRelief getFontRelief() const { return meFontRelief; }
            basegfx::BColor getOverlineColor() const { return maOverlineColor; }
            basegfx::BColor getTextlineColor() const { return maTextlineColor; }
            bool getUnderlineAbove() const { return mbUnderlineAbove; }
            bool getWordLineMode() const { return mbWordLineMode; }
            bool getEmphasisMarkAbove() const { return mbEmphasisMarkAbove; }
            bool getEmphasisMarkBelow() const { return mbEmphasisMarkBelow; }
            bool getShadow() const { return mbShadow; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
