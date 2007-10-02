/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: aw $ $Date: 2007-10-02 16:55:00 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTEFFECTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        bool FontAttributes::operator==(const FontAttributes& rCompare) const
        {
            return (getFamilyName() == rCompare.getFamilyName()
                && getStyleName() == rCompare.getStyleName()
                && getWeight() == rCompare.getWeight()
                && getSymbol() == rCompare.getSymbol()
                && getVertical() == rCompare.getVertical()
                && getItalic() == rCompare.getItalic()
                && getOutline() == rCompare.getOutline()
                && getRTL() == rCompare.getRTL()
                && getBiDiStrong() == rCompare.getBiDiStrong());
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

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
                rFontAttributes.getFamilyName(),
                rFontAttributes.getStyleName(),
                Size(nWidth, nHeight));

            if(!basegfx::fTools::equalZero(fFontRotation))
            {
                sal_Int16 aRotate10th((sal_Int16)(fFontRotation * (-1800.0/F_PI)));
                aRetval.SetOrientation(aRotate10th % 3600);
            }

            aRetval.SetAlign(ALIGN_BASELINE);
            aRetval.SetCharSet(rFontAttributes.getSymbol() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
            aRetval.SetVertical(rFontAttributes.getVertical() ? TRUE : FALSE);
            aRetval.SetWeight(static_cast<FontWeight>(rFontAttributes.getWeight()));
            aRetval.SetItalic(rFontAttributes.getItalic() ? ITALIC_NORMAL : ITALIC_NONE);
            aRetval.SetOutline(rFontAttributes.getOutline());

            return aRetval;
        }

        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont, bool bRTL, bool bBiDiStrong)
        {
            FontAttributes aRetval(
                rFont.GetName(),
                rFont.GetStyleName(),
                static_cast<sal_uInt16>(rFont.GetWeight()),
                RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet(),
                rFont.IsVertical(),
                ITALIC_NONE != rFont.GetItalic(),
                rFont.IsOutline(),
                bRTL,
                bBiDiStrong);
            // TODO: eKerning

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
            Primitive2DSequence aRetval;

            if(getTextLength())
            {
                // get integer DXArray for getTextOutlines call (ATM uses vcl)
                ::std::vector< sal_Int32 > aNewIntegerDXArray;
                getIntegerDXArray(aNewIntegerDXArray);

                // prepare transformation matrices
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                // unscaled transform is needed for transformations since the scale is the font height/width already
                basegfx::B2DHomMatrix aUnscaledTransform;
                aUnscaledTransform.rotate( fRotate );
                aUnscaledTransform.shearX( fShearX );
                aUnscaledTransform.translate( aTranslate.getX(), aTranslate.getY() );

                // unrotated transform is needed for text layouter. He works always with
                // X-Axis aligned text
                basegfx::B2DHomMatrix aUnrotatedTransform(getTextTransform());
                aUnrotatedTransform.rotate( -fRotate );

                // prepare textlayoutdevice
                TextLayouterDevice aTextLayouter;
                aTextLayouter.setFontAttributes(getFontAttributes(), aUnrotatedTransform );

                // get the text outlines
                basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
                aTextLayouter.getTextOutlines(aB2DPolyPolyVector, getText(), getTextPosition(), getTextLength(), aNewIntegerDXArray);

                // create primitives for the outlines
                const sal_uInt32 nCount(aB2DPolyPolyVector.size());

                if(nCount)
                {
                    // alloc space for the primitives
                    aRetval.realloc(nCount);

                    // color-filled polypolygons
                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        // prepare polypolygon
                        basegfx::B2DPolyPolygon& rPolyPolygon = aB2DPolyPolyVector[a];
                        rPolyPolygon.transform(aUnscaledTransform);
                        aRetval[a] = new PolyPolygonColorPrimitive2D(rPolyPolygon, getFontColor());
                    }

                    if(getFontAttributes().getOutline())
                    {
                        // create outline text effect with current content and replace
                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aTranslate,
                            fRotate,
                            TEXTEFFECTSTYLE2D_OUTLINE));

                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }
                }
            }

            return aRetval;
        }

        TextSimplePortionPrimitive2D::TextSimplePortionPrimitive2D(
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            xub_StrLen aTextPosition,
            xub_StrLen aTextLength,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const ::com::sun::star::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor)
        :   BasePrimitive2D(),
            maTextTransform(rNewTransform),
            maText(rText),
            maTextPosition(aTextPosition),
            maTextLength(aTextLength),
            maDXArray(rDXArray),
            maFontAttributes(rFontAttributes),
            maLocale(rLocale),
            maFontColor(rFontColor)
        {
#ifdef DBG_UTIL
            const xub_StrLen aStringLength(getText().Len());
            OSL_ENSURE(aStringLength >= getTextPosition() && aStringLength >= getTextPosition() + getTextLength(),
                "TextSimplePortionPrimitive2D with text out of range (!)");
#endif
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

        bool impLocalesAreEqual(const ::com::sun::star::lang::Locale& rA, const ::com::sun::star::lang::Locale& rB)
        {
            return (rA.Language == rB.Language
                && rA.Country == rB.Country
                && rA.Variant == rB.Variant);
        }

        bool TextSimplePortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const TextSimplePortionPrimitive2D& rCompare = (TextSimplePortionPrimitive2D&)rPrimitive;

                return (getTextTransform() == rCompare.getTextTransform()
                    && getText() == rCompare.getText()
                    && getTextPosition() == rCompare.getTextPosition()
                    && getTextLength() == rCompare.getTextLength()
                    && getDXArray() == rCompare.getDXArray()
                    && getFontAttributes() == rCompare.getFontAttributes()
                    && impLocalesAreEqual(getLocale(), rCompare.getLocale())
                    && getFontColor() == rCompare.getFontColor());
            }

            return false;
        }

        basegfx::B2DRange TextSimplePortionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval;

            if(getTextLength())
            {
                // get TextBoundRect as base size
                TextLayouterDevice aTextLayouter;
                aTextLayouter.setFontAttributes(getFontAttributes(), getTextTransform());
                aRetval = aTextLayouter.getTextBoundRect(getText(), getTextPosition(), getTextLength());

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
        ImplPrimitrive2DIDBlock(TextSimplePortionPrimitive2D, PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
