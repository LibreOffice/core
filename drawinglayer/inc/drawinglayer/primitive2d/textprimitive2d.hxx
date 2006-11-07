/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-07 15:49:05 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        struct FontAttributes
        {
            String                                      maFamilyName;
            String                                      maStyleName;
            sal_uInt16                                  mnWeight;

            // bitfield
            unsigned                                    mbSymbol : 1;
            unsigned                                    mbVertical : 1;
            unsigned                                    mbItalic : 1;

            // compare operator
            bool operator==(const FontAttributes& rCompare) const
            {
                return (maFamilyName == rCompare.maFamilyName
                    && maStyleName == rCompare.maStyleName
                    && mnWeight == rCompare.mnWeight
                    && mbSymbol == rCompare.mbSymbol
                    && mbVertical == rCompare.mbVertical
                    && mbItalic == rCompare.mbItalic);
            }
        };

        // helper methods for vcl font
        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DVector& rFontSize, double fFontRotation);
        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DHomMatrix& rTransform);
        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class TextSimplePortionPrimitive2D : public BasePrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                   maTextTransform;    // text range transformation from unit range ([0.0 .. 1.0]) to text range
            String                                  maText;             // the text
            ::std::vector< double >                 maDXArray;          // the DX array scale-independent in unit coordinates
            FontAttributes                          maFontAttributes;   // the font to use
            basegfx::BColor                         maFontColor;        // font color

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            TextSimplePortionPrimitive2D(
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                const ::std::vector< double >& rDXArray,
                const FontAttributes& rFontAttributes,
                const basegfx::BColor& rFontColor);

            // get data
            const basegfx::B2DHomMatrix& getTextTransform() const { return maTextTransform; }
            const String& getText() const { return maText; }
            const ::std::vector< double >& getDXArray() const { return maDXArray; }
            const FontAttributes& getFontAttributes() const { return maFontAttributes; }
            const basegfx::BColor& getFontColor() const { return maFontColor; }

            // helper to have a central conversion to font-size-scaled integer DXArray
            void getIntegerDXArray(::std::vector< sal_Int32 >& rDXArray) const;

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        enum FontUnderline
        {
            FONT_UNDERLINE_NONE,
            FONT_UNDERLINE_SINGLE,
            FONT_UNDERLINE_DOUBLE,
            FONT_UNDERLINE_DOTTED,
            FONT_UNDERLINE_DASH,
            FONT_UNDERLINE_LONGDASH,
            FONT_UNDERLINE_DASHDOT,
            FONT_UNDERLINE_DASHDOTDOT,
            FONT_UNDERLINE_SMALLWAVE,
            FONT_UNDERLINE_WAVE,
            FONT_UNDERLINE_DOUBLEWAVE,
            FONT_UNDERLINE_BOLD,
            FONT_UNDERLINE_BOLDDOTTED,
            FONT_UNDERLINE_BOLDDASH,
            FONT_UNDERLINE_BOLDLONGDASH,
            FONT_UNDERLINE_BOLDDASHDOT,
            FONT_UNDERLINE_BOLDDASHDOTDOT,
            FONT_UNDERLINE_BOLDWAVE
        };

        enum FontStrikeout
        {
            FONT_STRIKEOUT_NONE,
            FONT_STRIKEOUT_SINGLE,
            FONT_STRIKEOUT_DOUBLE,
            FONT_STRIKEOUT_BOLD,
            FONT_STRIKEOUT_SLASH,
            FONT_STRIKEOUT_X
        };

        enum FontEmphasisMark
        {
            FONT_EMPHASISMARK_NONE,
            FONT_EMPHASISMARK_DOT,
            FONT_EMPHASISMARK_CIRCLE,
            FONT_EMPHASISMARK_DISC,
            FONT_EMPHASISMARK_ACCENT
        };

        enum FontRelief
        {
            FONT_RELIEF_NONE,
            FONT_RELIEF_EMBOSSED,
            FONT_RELIEF_ENGRAVED
        };

        class TextComplexPortionPrimitive2D : public TextSimplePortionPrimitive2D
        {
        private:
            FontUnderline                               meFontUnderline;
            FontStrikeout                               meFontStrikeout;
            FontEmphasisMark                            meFontEmphasisMark;
            FontRelief                                  meFontRelief;

            // bitfield
            unsigned                                    mbUnderlineAbove : 1;
            unsigned                                    mbWordLineMode : 1;
            unsigned                                    mbEmphasisMarkAbove : 1;
            unsigned                                    mbEmphasisMarkBelow : 1;
            unsigned                                    mbShadow : 1;
            unsigned                                    mbOutline : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            TextComplexPortionPrimitive2D(
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                const ::std::vector< double >& rDXArray,
                const FontAttributes& rFontAttributes,
                const basegfx::BColor& rFontColor,
                FontUnderline eFontUnderline = FONT_UNDERLINE_NONE,
                bool bUnderlineAbove = false,
                FontStrikeout eFontStrikeout = FONT_STRIKEOUT_NONE,
                bool bWordLineMode = false,
                FontEmphasisMark eFontEmphasisMark = FONT_EMPHASISMARK_NONE,
                bool bEmphasisMarkAbove = true,
                bool bEmphasisMarkBelow = false,
                FontRelief eFontRelief = FONT_RELIEF_NONE,
                bool bShadow = false,
                bool bOutline = false);

            // get data
            FontUnderline getFontUnderline() const { return meFontUnderline; }
            FontStrikeout getFontStrikeout() const { return meFontStrikeout; }
            FontEmphasisMark getFontEmphasisMark() const { return meFontEmphasisMark; }
            FontRelief getFontRelief() const { return meFontRelief; }
            bool getUnderlineAbove() const { return mbUnderlineAbove; }
            bool getWordLineMode() const { return mbWordLineMode; }
            bool getEmphasisMarkAbove() const { return mbEmphasisMarkAbove; }
            bool getEmphasisMarkBelow() const { return mbEmphasisMarkBelow; }
            bool getShadow() const { return mbShadow; }
            bool getOutline() const { return mbOutline; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE_HXX

// eof
