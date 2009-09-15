/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.hxx,v $
 *
 *  $Revision: 1.13 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/string.hxx>
#include <vcl/font.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vector>
#include <com/sun/star/lang/Locale.hpp>

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
        class FontAttributes
        {
        private:
            // core data
            String                                      maFamilyName;       // Font Family Name
            String                                      maStyleName;        // Font Style Name
            sal_uInt16                                  mnWeight;           // Font weight

            // bitfield
            unsigned                                    mbSymbol : 1;       // Symbol Font Flag
            unsigned                                    mbVertical : 1;     // Vertical Text Flag
            unsigned                                    mbItalic : 1;       // Italic Flag
            unsigned                                    mbOutline : 1;      // Outline Flag
            unsigned                                    mbRTL : 1;          // RTL Flag
            unsigned                                    mbBiDiStrong : 1;   // BiDi Flag
            // TODO: pair kerning and CJK kerning

        public:
            FontAttributes(
                const String& rFamilyName,
                const String& rStyleName,
                sal_uInt16 nWeight,
                bool bSymbol = false,
                bool bVertical = false,
                bool bItalic = false,
                bool bOutline = false,
                bool bRTL = false,
                bool bBiDiStrong = false)
            :   maFamilyName(rFamilyName),
                maStyleName(rStyleName),
                mnWeight(nWeight),
                mbSymbol(bSymbol),
                mbVertical(bVertical),
                mbItalic(bItalic),
                mbOutline(bOutline),
                mbRTL(bRTL),
                mbBiDiStrong(bBiDiStrong)
            {
            }

            // compare operator
            bool operator==(const FontAttributes& rCompare) const;

            // data access
            const String& getFamilyName() const { return maFamilyName; }
            const String& getStyleName() const { return maStyleName; }
            sal_uInt16 getWeight() const { return mnWeight; }
            bool getSymbol() const { return mbSymbol; }
            bool getVertical() const { return mbVertical; }
            bool getItalic() const { return mbItalic; }
            bool getOutline() const { return mbOutline; }
            bool getRTL() const { return mbRTL; }
            bool getBiDiStrong() const { return mbBiDiStrong; }
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class TextSimplePortionPrimitive2D : public BufDecPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                   maTextTransform;    // text range transformation from unit range ([0.0 .. 1.0]) to text range
            String                                  maText;             // the text, used from maTextPosition up to maTextPosition + maTextLength
            xub_StrLen                              maTextPosition;     // the index from where on maText is used
            xub_StrLen                              maTextLength;       // the length for maText usage, starting from maTextPosition
            ::std::vector< double >                 maDXArray;          // the DX array scale-independent in unit coordinates
            FontAttributes                          maFontAttributes;   // the font to use
            ::com::sun::star::lang::Locale          maLocale;           // the Locale for the text
            basegfx::BColor                         maFontColor;        // font color

            // #i96669# add simple range buffering for this primitive
            basegfx::B2DRange                       maB2DRange;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocal2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            TextSimplePortionPrimitive2D(
                const basegfx::B2DHomMatrix& rNewTransform,
                const String& rText,
                xub_StrLen aTextPosition,
                xub_StrLen aTextLength,
                const ::std::vector< double >& rDXArray,
                const FontAttributes& rFontAttributes,
                const ::com::sun::star::lang::Locale& rLocale,
                const basegfx::BColor& rFontColor);

            // helpers
            // get text outlines as polygons and their according ObjectTransformation. Handles all
            // the necessary VCL outline extractins, scaling adaptions and other stuff.
            void getTextOutlinesAndTransformation(basegfx::B2DPolyPolygonVector& rTarget, basegfx::B2DHomMatrix& rTransformation) const;

            // adapts fontScale for usage with TextLayouter. Input is rScale which is the extracted
            // scale from a text transformation. A copy goes to rFontScale and is modified so that
            // it contains only positive scalings and XY-equal scalings to allow to get a non-X-scaled
            // Vcl-Font for TextLayouter. rScale is adapted accordingly to contain the corrected scale
            // which would need to be applied to e.g. outlines received from TextLayouter under
            // usage of fontScale. This includes Y-Scale, X-Scale-correction and mirrorings.
            void getCorrectedScaleAndFontScale(basegfx::B2DVector& rScale, basegfx::B2DVector& rFontScale) const;

            // get data
            const basegfx::B2DHomMatrix& getTextTransform() const { return maTextTransform; }
            const String& getText() const { return maText; }
            xub_StrLen getTextPosition() const { return maTextPosition; }
            xub_StrLen getTextLength() const { return maTextLength; }
            const ::std::vector< double >& getDXArray() const { return maDXArray; }
            const FontAttributes& getFontAttributes() const { return maFontAttributes; }
            const ::com::sun::star::lang::Locale& getLocale() const { return  maLocale; }
            const basegfx::BColor& getFontColor() const { return maFontColor; }

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

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
