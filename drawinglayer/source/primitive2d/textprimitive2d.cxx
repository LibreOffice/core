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
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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

namespace drawinglayer
{
    namespace primitive2d
    {
        void TextSimplePortionPrimitive2D::getCorrectedScaleAndFontScale(basegfx::B2DVector& rScale, basegfx::B2DVector& rFontScale) const
        {
            // copy input value
            rFontScale = rScale;

            if(basegfx::fTools::equalZero(rFontScale.getY()))
            {
                // no font height; choose one and adapt scale to get back to original scaling
                static double fDefaultFontScale(100.0);
                rScale.setY(1.0 / fDefaultFontScale);
                rFontScale.setY(fDefaultFontScale);
            }
            else if(basegfx::fTools::less(rFontScale.getY(), 0.0))
            {
                // negative font height; invert and adapt scale to get back to original scaling
                rFontScale.setY(-rFontScale.getY());
                rScale.setY(-1.0);
            }
            else
            {
                // positive font height; adapt scale; scaling will be part of the polygons
                rScale.setY(1.0);
            }

            if(basegfx::fTools::equal(rFontScale.getX(), rFontScale.getY()))
            {
                // adapt scale in X
                rScale.setX(1.0);
            }
            else
            {
                // If font scale is different in X and Y, force font scale to equal
                // in X and Y to get a non-scaled VCL font.
                // Adapt scaling in X accordingly. FontScaleY cannot be zero here.
                rScale.setX(rFontScale.getX()/rFontScale.getY());
                rFontScale.setX(rFontScale.getY());
            }
        }

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
#ifdef WIN32
                    const bool bCorrectScale(!basegfx::fTools::equal(fabs(aScale.getX()), fabs(aScale.getY())));
#endif
                    basegfx::B2DVector aFontScale;
                    getCorrectedScaleAndFontScale(aScale, aFontScale);

                    // prepare textlayoutdevice
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttributes(getFontAttributes(), aFontScale.getX(), aFontScale.getY(), getLocale());
#ifdef WIN32
                    // when under Windows and the font is unequally scaled, need to correct font X-Scaling factor
                    if(bCorrectScale)
                    {
                        const double fFontRelation(aTextLayouter.getCurrentFontRelation());
                        aScale.setX(aScale.getX() * fFontRelation);
                        aFontScale.setX(aFontScale.getX() / fFontRelation);
                    }
#endif
                    // get the text outlines. No DXArray is given (would contain integers equal to unit vector
                    // transformed by object's transformation), let VCL do the job
                    aTextLayouter.getTextOutlines(
                        rTarget, getText(),
                        getTextPosition(),
                        getTextLength(),
                        // #i89784# added support for DXArray for justified text
                        getDXArray(),
                        aFontScale.getX());

                    // create primitives for the outlines
                    const sal_uInt32 nCount(rTarget.size());

                    if(nCount)
                    {
                        // prepare object transformation for polygons
                        rTransformation = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aScale, fShearX, fRotate, aTranslate);
                    }
                }
            }
        }

        Primitive2DSequence TextSimplePortionPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
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
        :   BufferedDecompositionPrimitive2D(),
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
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
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
#ifdef WIN32
                    const bool bCorrectScale(!basegfx::fTools::equal(fabs(aScale.getX()), fabs(aScale.getY())));
#endif
                    basegfx::B2DVector aFontScale;
                    getCorrectedScaleAndFontScale(aScale, aFontScale);

                    // prepare textlayoutdevice
                    TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFontAttributes(getFontAttributes(), aFontScale.getX(), aFontScale.getY(), getLocale());

                    // get basic text range
                    basegfx::B2DRange aNewRange(aTextLayouter.getTextBoundRect(getText(), getTextPosition(), getTextLength()));

                    // #i102556# take empty results into account
                    if(!aNewRange.isEmpty())
                    {
#ifdef WIN32
                        // when under Windows and the font is unequally scaled, need to correct font X-Scaling factor
                        if(bCorrectScale)
                        {
                            aScale.setX(aScale.getX() * aTextLayouter.getCurrentFontRelation());
                        }
#endif
                        // prepare object transformation for range
                        const basegfx::B2DHomMatrix aRangeTransformation(basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aScale, fShearX, fRotate, aTranslate));

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
