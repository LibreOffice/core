/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdecoratedprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2007-08-03 10:43:04 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

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

        class WrongSpellEntry
        {
            sal_uInt32              mnStart;
            sal_uInt32              mnEnd;

        public:
            WrongSpellEntry(sal_uInt32 nS, sal_uInt32 nE)
            :   mnStart(nS),
                mnEnd(nE)
            {}

            // compare operator
            bool operator==(const WrongSpellEntry& rEntry) const
            {
                return (mnStart == rEntry.mnStart && mnEnd == rEntry.mnEnd);
            }

            sal_uInt32 getStart() const { return mnStart; }
            sal_uInt32 getEnd() const { return mnEnd; }
        };

        typedef std::vector< WrongSpellEntry > WrongSpellVector;

        class TextDecoratedPortionPrimitive2D : public TextSimplePortionPrimitive2D
        {
        private:
            basegfx::BColor                             maTextlineColor;
            FontUnderline                               meFontUnderline;
            FontStrikeout                               meFontStrikeout;
            FontEmphasisMark                            meFontEmphasisMark;
            FontRelief                                  meFontRelief;
            WrongSpellVector                            maWrongSpellVector;

            // bitfield
            unsigned                                    mbUnderlineAbove : 1;
            unsigned                                    mbWordLineMode : 1;
            unsigned                                    mbEmphasisMarkAbove : 1;
            unsigned                                    mbEmphasisMarkBelow : 1;
            unsigned                                    mbShadow : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            TextDecoratedPortionPrimitive2D(

                // TextSimplePortionPrimitive2D parameters
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                const ::std::vector< double >& rDXArray,
                const FontAttributes& rFontAttributes,
                const ::com::sun::star::lang::Locale& rLocale,
                const basegfx::BColor& rFontColor,

                // local parameters
                const basegfx::BColor& rTextlineColor,
                FontUnderline eFontUnderline = FONT_UNDERLINE_NONE,
                bool bUnderlineAbove = false,
                FontStrikeout eFontStrikeout = FONT_STRIKEOUT_NONE,
                bool bWordLineMode = false,
                FontEmphasisMark eFontEmphasisMark = FONT_EMPHASISMARK_NONE,
                bool bEmphasisMarkAbove = true,
                bool bEmphasisMarkBelow = false,
                FontRelief eFontRelief = FONT_RELIEF_NONE,
                bool bShadow = false,
                const WrongSpellVector& rWrongSpellVector = WrongSpellVector());

            // get data
            FontUnderline getFontUnderline() const { return meFontUnderline; }
            FontStrikeout getFontStrikeout() const { return meFontStrikeout; }
            FontEmphasisMark getFontEmphasisMark() const { return meFontEmphasisMark; }
            FontRelief getFontRelief() const { return meFontRelief; }
            basegfx::BColor getTextlineColor() const { return maTextlineColor; }
            const WrongSpellVector& getWrongSpellVector() const { return maWrongSpellVector; }

            bool getUnderlineAbove() const { return mbUnderlineAbove; }
            bool getWordLineMode() const { return mbWordLineMode; }
            bool getEmphasisMarkAbove() const { return mbEmphasisMarkAbove; }
            bool getEmphasisMarkBelow() const { return mbEmphasisMarkBelow; }
            bool getShadow() const { return mbShadow; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTDECORATEDPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
