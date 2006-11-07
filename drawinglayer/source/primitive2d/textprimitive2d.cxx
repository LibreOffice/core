/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-07 15:49:09 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DHomMatrix& rTransform)
        {
            // decompose matrix to have position and size of text
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rTransform.decompose(aScale, aTranslate, fRotate, fShearX);
            return getVclFontFromFontAttributes(rFontAttributes, aScale, fRotate);
        }

        Font getVclFontFromFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DVector& rFontSize, double fFontRotation)
        {
            sal_uInt32 nWidth(basegfx::fround(fabs(rFontSize.getX())));
            sal_uInt32 nHeight(basegfx::fround(fabs(rFontSize.getY())));

            if(nWidth == nHeight)
            {
                nWidth = 0L;
            }

            Font aRetval(
                rFontAttributes.maFamilyName,
                rFontAttributes.maStyleName,
                Size(nWidth, nHeight));

            if(!basegfx::fTools::equalZero(fFontRotation))
            {
                sal_Int16 aRotate10th((sal_Int16)(fFontRotation * (-1800.0/F_PI)));
                aRetval.SetOrientation(aRotate10th % 3600);
            }

            aRetval.SetAlign(ALIGN_BASELINE);
            aRetval.SetCharSet(rFontAttributes.mbSymbol ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
            aRetval.SetVertical(rFontAttributes.mbVertical ? TRUE : FALSE);
            aRetval.SetWeight(static_cast<FontWeight>(rFontAttributes.mnWeight));
            aRetval.SetItalic(rFontAttributes.mbItalic ? ITALIC_NORMAL : ITALIC_NONE);

            return aRetval;
        }

        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont)
        {
            FontAttributes aRetval;

            aRetval.maFamilyName = rFont.GetName();
            aRetval.maStyleName = rFont.GetStyleName();
            aRetval.mbSymbol = (RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet());
            aRetval.mbVertical = rFont.IsVertical();
            aRetval.mnWeight = static_cast<sal_uInt16>(rFont.GetWeight());
            aRetval.mbItalic = (rFont.GetItalic() != ITALIC_NONE);

            const sal_Int32 nWidth(rFont.GetSize().getWidth());
            const sal_Int32 nHeight(rFont.GetSize().getHeight());
            rSize.setX(nWidth ? nWidth : nHeight);
            rSize.setY(nHeight);

            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextSimplePortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // get integer DXArray for getTextOutlines call (ATM uses vcl)
            ::std::vector< sal_Int32 > aNewIntegerDXArray;
            getIntegerDXArray(aNewIntegerDXArray);

            // get outlines
            TextLayouterDevice aTextLayouter;
            aTextLayouter.setFontAttributes(getFontAttributes(), getTextTransform());
            PolyPolyVector aPolyPolyVector;
            aTextLayouter.getTextOutlines(aPolyPolyVector, getText(), 0L, getText().Len(), aNewIntegerDXArray);

            // get result count
            const sal_uInt32 nCount(aPolyPolyVector.size());

            if(nCount)
            {
                // outlines already have scale and rotate included, so build new transformation
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);
                basegfx::B2DHomMatrix aNewTransform;
                const bool bShearUsed(!basegfx::fTools::equalZero(fShearX));
                const bool bRotateUsed(!basegfx::fTools::equalZero(fRotate));

                if(bShearUsed)
                {
                    // The order would be wrong when just adding shear, so rotate back, apply shear, rotate again
                    if(bRotateUsed)
                    {
                        aNewTransform.rotate(-fRotate);
                    }

                    // add shear (before rotate)
                    aNewTransform.shearX(fShearX);

                    if(bRotateUsed)
                    {
                        aNewTransform.rotate(fRotate);
                    }
                }

                // add translation
                aNewTransform.translate(aTranslate.getX(), aTranslate.getY());

                // prepare retval
                Primitive2DSequence aRetval(nCount);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    // prepare polygon
                    basegfx::B2DPolyPolygon aPolyPolygon(aPolyPolyVector[a].getB2DPolyPolygon());
                    aPolyPolygon.transform(aNewTransform);

                    // create primitive
                    const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(aPolyPolygon, getFontColor()));
                    aRetval[a] = xRef;
                }

                return aRetval;
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        TextSimplePortionPrimitive2D::TextSimplePortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const basegfx::BColor& rFontColor)
        :   BasePrimitive2D(),
            maTextTransform(rNewTransform),
            maText(rText),
            maDXArray(rDXArray),
            maFontAttributes(rFontAttributes),
            maFontColor(rFontColor)
        {
        }

        void TextSimplePortionPrimitive2D::getIntegerDXArray(::std::vector< sal_Int32 >& rDXArray) const
        {
            rDXArray.clear();

            if(getDXArray().size())
            {
                rDXArray.reserve(getDXArray().size());
                const basegfx::B2DVector aPixelVector(getTextTransform() * basegfx::B2DVector(1.0, 0.0));
                const double fPixelVectorLength(aPixelVector.getLength());

                for(::std::vector< double >::const_iterator aStart(getDXArray().begin()); aStart != getDXArray().end(); aStart++)
                {
                    rDXArray.push_back(basegfx::fround((*aStart) * fPixelVectorLength));
                }
            }
        }

        bool TextSimplePortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const TextSimplePortionPrimitive2D& rCompare = (TextSimplePortionPrimitive2D&)rPrimitive;

                return (getTextTransform() == rCompare.getTextTransform()
                    && getText() == rCompare.getText()
                    && getDXArray() == rCompare.getDXArray()
                    && getFontAttributes() == rCompare.getFontAttributes()
                    && getFontColor() == rCompare.getFontColor());
            }

            return false;
        }

        basegfx::B2DRange TextSimplePortionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const xub_StrLen aStrLen(getText().Len());
            basegfx::B2DRange aRetval;

            if(aStrLen)
            {
                // get TextBoundRect as base size
                TextLayouterDevice aTextLayouter;
                aTextLayouter.setFontAttributes(getFontAttributes(), getTextTransform());
                aRetval = aTextLayouter.getTextBoundRect(getText(), 0L, aStrLen);

                // apply textTransform to it, but without scaling. The scale defines the font size
                // which is already part of the fetched textRange
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                basegfx::B2DHomMatrix aTextTransformWithoutScale;

                getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);
                aTextTransformWithoutScale.shearX(fShearX);
                aTextTransformWithoutScale.rotate(fRotate);
                aTextTransformWithoutScale.translate(aTranslate.getX(), aTranslate.getY());
                aRetval.transform(aTextTransformWithoutScale);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextSimplePortionPrimitive2D, '2','T','S','i')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence TextComplexPortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // TODO: need to take care of
            // -underline
            // -strikethrough
            // -emphasis mark
            // -relief (embosses/engraved)
            // -shadow
            // -outline

            // ATM: Just create a simple text primitive and ignore other attributes
            const Primitive2DReference xRef(new TextSimplePortionPrimitive2D(getTextTransform(), getText(), getDXArray(), getFontAttributes(), getFontColor()));
            return Primitive2DSequence(&xRef, 1L);
        }

        TextComplexPortionPrimitive2D::TextComplexPortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const basegfx::BColor& rFontColor,
            FontUnderline eFontUnderline,
            bool bUnderlineAbove,
            FontStrikeout eFontStrikeout,
            bool bWordLineMode,
            FontEmphasisMark eFontEmphasisMark,
            bool bEmphasisMarkAbove,
            bool bEmphasisMarkBelow,
            FontRelief eFontRelief,
            bool bShadow,
            bool bOutline)
        :   TextSimplePortionPrimitive2D(rNewTransform, rText, rDXArray, rFontAttributes, rFontColor),
            meFontUnderline(eFontUnderline),
            meFontStrikeout(eFontStrikeout),
            meFontRelief(eFontRelief),
            mbUnderlineAbove(bUnderlineAbove),
            mbWordLineMode(bWordLineMode),
            meFontEmphasisMark(eFontEmphasisMark),
            mbEmphasisMarkAbove(bEmphasisMarkAbove),
            mbEmphasisMarkBelow(bEmphasisMarkBelow),
            mbShadow(bShadow),
            mbOutline(bOutline)
        {
        }

        bool TextComplexPortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(TextSimplePortionPrimitive2D::operator==(rPrimitive))
            {
                const TextComplexPortionPrimitive2D& rCompare = (TextComplexPortionPrimitive2D&)rPrimitive;

                return (getFontUnderline() == rCompare.getFontUnderline()
                    && getFontStrikeout() == rCompare.getFontStrikeout()
                    && getUnderlineAbove() == rCompare.getUnderlineAbove()
                    && getWordLineMode() == rCompare.getWordLineMode()
                    && getFontEmphasisMark() == rCompare.getFontEmphasisMark()
                    && getEmphasisMarkAbove() == rCompare.getEmphasisMarkAbove()
                    && getEmphasisMarkBelow() == rCompare.getEmphasisMarkBelow()
                    && getFontRelief() == rCompare.getFontRelief()
                    && getShadow() == rCompare.getShadow()
                    && getOutline() == rCompare.getOutline());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextComplexPortionPrimitive2D, '2','T','C','o')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
