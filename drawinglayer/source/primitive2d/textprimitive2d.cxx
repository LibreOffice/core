/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textprimitive2d.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

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

namespace
{
    // adapts fontScale for usage with TextLayouter. Input is rScale which is the extracted
    // scale from a text transformation. A copy is modified so that it contains only positive
    // scalings and XY-equal scalings to allow to get a non-X-scaled Vcl-Font for TextLayouter.
    // rScale is adapted accordingly to contain the corrected scale which would need to be
    // applied to e.g. outlines received from TextLayouter under usage of fontScale. This
    // includes Y-Scale, X-Scale-correction and mirrorings.
    basegfx::B2DVector getCorrectedScaleAndFontScale(basegfx::B2DVector& rScale)
    {
        // copy input value
        basegfx::B2DVector aFontScale(rScale);

        // correct FontHeight settings
        if(basegfx::fTools::equalZero(aFontScale.getY()))
        {
            // no font height; choose one and adapt scale to get back to original scaling
            static double fDefaultFontScale(100.0);
            rScale.setY(1.0 / fDefaultFontScale);
            aFontScale.setY(fDefaultFontScale);
        }
        else if(basegfx::fTools::less(aFontScale.getY(), 0.0))
        {
            // negative font height; invert and adapt scale to get back to original scaling
            aFontScale.setY(-aFontScale.getY());
            rScale.setY(-1.0);
        }
        else
        {
            // positive font height; adapt scale; scaling will be part of the polygons
            rScale.setY(1.0);
        }

        // correct FontWidth settings
        if(basegfx::fTools::equal(aFontScale.getX(), aFontScale.getY()))
        {
            // no FontScale, adapt scale
            rScale.setX(1.0);
        }
        else
        {
            // If FontScale is used, force to no FontScale to get a non-scaled VCL font.
            // Adapt scaling in X accordingly.
            rScale.setX(aFontScale.getX() / aFontScale.getY());
            aFontScale.setX(aFontScale.getY());
        }

        return aFontScale;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void TextSimplePortionPrimitive2D::getTextOutlinesAndTransformation(basegfx::B2DPolyPolygonVector& rTarget, basegfx::B2DHomMatrix& rTransformation) const
        {
            if(getTextLength())
            {
                // decompose object transformation to single values
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;

                // if decomposition returns false, create no geometry since e.g. scaling may
                // be zero
                if(getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    // handle special case: If scale is negative in (x,y) (3rd quadrant), it can
                    // be expressed as rotation by PI
                    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
                    {
                        aScale = basegfx::absolute(aScale);
                        fRotate += F_PI;
                    }

                    // for the TextLayouterDevice, it is necessary to have a scaling representing
                    // the font size. Since we want to extract polygons here, it is okay to
                    // work just with scaling and to ignore shear, rotation and translation,
                    // all that can be applied to the polygons later
                    const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

                    // prepare textlayoutdevice
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttributes(
                        getFontAttributes(),
                        aFontScale.getX(),
                        aFontScale.getY(),
                        getLocale());

                    // When getting outlines from stretched text (aScale.getX() != 1.0) it
                    // is necessary to inverse-scale the DXArray (if used) to not get the
                    // outlines already aligned to given, but wrong DXArray
                    if(getDXArray().size() && !basegfx::fTools::equal(aScale.getX(), 1.0))
                    {
                        ::std::vector< double > aScaledDXArray = getDXArray();
                        const double fDXArrayScale(1.0 / aScale.getX());

                        for(sal_uInt32 a(0); a < aScaledDXArray.size(); a++)
                        {
                            aScaledDXArray[a] *= fDXArrayScale;
                        }

                        // get the text outlines
                        aTextLayouter.getTextOutlines(
                            rTarget,
                            getText(),
                            getTextPosition(),
                            getTextLength(),
                            aScaledDXArray);
                    }
                    else
                    {
                        // get the text outlines
                        aTextLayouter.getTextOutlines(
                            rTarget,
                            getText(),
                            getTextPosition(),
                            getTextLength(),
                            getDXArray());
                    }

                    // create primitives for the outlines
                    const sal_uInt32 nCount(rTarget.size());

                    if(nCount)
                    {
                        // prepare object transformation for polygons
                        rTransformation.identity();
                        rTransformation.scale(aScale.getX(), aScale.getY());
                        rTransformation.shearX(fShearX);
                        rTransformation.rotate(fRotate);
                        rTransformation.translate(aTranslate.getX(), aTranslate.getY());
                    }
                }
            }
        }

        Primitive2DSequence TextSimplePortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(getTextLength())
            {
                basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
                basegfx::B2DHomMatrix aPolygonTransform;

                // get text outlines and their object transformation
                getTextOutlinesAndTransformation(aB2DPolyPolyVector, aPolygonTransform);

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
                        rPolyPolygon.transform(aPolygonTransform);
                        aRetval[a] = new PolyPolygonColorPrimitive2D(rPolyPolygon, getFontColor());
                    }

                    if(getFontAttributes().getOutline())
                    {
                        // decompose polygon transformation to single values
                        basegfx::B2DVector aScale, aTranslate;
                        double fRotate, fShearX;
                        aPolygonTransform.decompose(aScale, aTranslate, fRotate, fShearX);

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
            maFontColor(rFontColor),
            maB2DRange()
        {
#ifdef DBG_UTIL
            const xub_StrLen aStringLength(getText().Len());
            OSL_ENSURE(aStringLength >= getTextPosition() && aStringLength >= getTextPosition() + getTextLength(),
                "TextSimplePortionPrimitive2D with text out of range (!)");
#endif
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
            if(maB2DRange.isEmpty() && getTextLength())
            {
                // get TextBoundRect as base size
                // decompose object transformation to single values
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;

                if(getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    // for the TextLayouterDevice, it is necessary to have a scaling representing
                    // the font size. Since we want to extract polygons here, it is okay to
                    // work just with scaling and to ignore shear, rotation and translation,
                    // all that can be applied to the polygons later
                    const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

                    // prepare textlayoutdevice
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttributes(
                        getFontAttributes(),
                        aFontScale.getX(),
                        aFontScale.getY(),
                        getLocale());

                    // get basic text range
                    basegfx::B2DRange aNewRange(aTextLayouter.getTextBoundRect(getText(), getTextPosition(), getTextLength()));

                    // #i104432#, #i102556# take empty results into account
                    if(!aNewRange.isEmpty())
                    {
                        // prepare object transformation for range
                        basegfx::B2DHomMatrix aRangeTransformation;

                        aRangeTransformation.scale(aScale.getX(), aScale.getY());
                        aRangeTransformation.shearX(fShearX);
                        aRangeTransformation.rotate(fRotate);
                        aRangeTransformation.translate(aTranslate.getX(), aTranslate.getY());

                        // apply range transformation to it
                        aNewRange.transform(aRangeTransformation);

                        // assign to buffered value
                        const_cast< TextSimplePortionPrimitive2D* >(this)->maB2DRange = aNewRange;
                    }
                }
            }

            return maB2DRange;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextSimplePortionPrimitive2D, PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
