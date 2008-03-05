/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdecoratedprimitive2d.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTDECORATEDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIXTOOLS_HXX
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTEFFECTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void TextDecoratedPortionPrimitive2D::impCreateGeometryContent(
            std::vector< Primitive2DReference >& rTarget,
            basegfx::DecomposedB2DHomMatrixContainer& rDecTrans,
            const String& rText,
            xub_StrLen aTextPosition,
            xub_StrLen aTextLength,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes) const
        {
            // create the SimpleTextPrimitive needed in any case
            rTarget.push_back(Primitive2DReference(new TextSimplePortionPrimitive2D(
                rDecTrans.getB2DHomMatrix(),
                rText,
                aTextPosition,
                aTextLength,
                rDXArray,
                rFontAttributes,
                getLocale(),
                getFontColor())));

            // see if something else needs to be done
            const bool bUnderlineUsed(FONT_UNDERLINE_NONE != getFontUnderline());
            const bool bStrikeoutUsed(FONT_STRIKEOUT_NONE != getFontStrikeout());

            if(bUnderlineUsed || bStrikeoutUsed)
            {
                // common preparations
                basegfx::B2DHomMatrix aUnscaledTransform;
                TextLayouterDevice aTextLayouter;

                // unscaled is needed since scale contains already the font size
                aUnscaledTransform.shearX(rDecTrans.getShearX());
                aUnscaledTransform.rotate(rDecTrans.getRotate());
                aUnscaledTransform.translate(rDecTrans.getTranslate().getX(), rDecTrans.getTranslate().getY());

                // TextLayouterDevice is needed to get metrics for text decorations like
                // underline/strikeout/emphasis marks from it. For setup, the font size is needed
                aTextLayouter.setFontAttributes(getFontAttributes(), rDecTrans.getScale().getX(), rDecTrans.getScale().getY());

                // get text width
                double fTextWidth(0.0);

                if(rDXArray.empty())
                {
                    fTextWidth = aTextLayouter.getTextWidth(rText, aTextPosition, aTextLength);
                }
                else
                {
                    fTextWidth = rDXArray.back() * rDecTrans.getScale().getX();
                }

                if(bUnderlineUsed)
                {
                    // create primitive geometry for underline
                    bool bDoubleLine(false);
                    bool bWaveLine(false);
                    bool bBoldLine(false);
                    const int* pDotDashArray(0);
                    basegfx::B2DLineJoin eLineJoin(basegfx::B2DLINEJOIN_NONE);
                    double fUnderlineOffset(aTextLayouter.getUnderlineOffset());
                    double fUnderlineHeight(aTextLayouter.getUnderlineHeight());

                    static const int aDottedArray[]     = { 1, 1, 0};               // DOTTED LINE
                    static const int aDotDashArray[]    = { 1, 1, 4, 1, 0};         // DASHDOT
                    static const int aDashDotDotArray[] = { 1, 1, 1, 1, 4, 1, 0};   // DASHDOTDOT
                    static const int aDashedArray[]     = { 5, 2, 0};               // DASHED LINE
                    static const int aLongDashArray[]   = { 7, 2, 0};               // LONGDASH

                    switch(getFontUnderline())
                    {
                        default: // case FONT_UNDERLINE_SINGLE:
                        {
                            break;
                        }
                        case FONT_UNDERLINE_DOUBLE:
                        {
                            bDoubleLine = true;
                            break;
                        }
                        case FONT_UNDERLINE_DOTTED:
                        {
                            pDotDashArray = aDottedArray;
                            break;
                        }
                        case FONT_UNDERLINE_DASH:
                        {
                            pDotDashArray = aDashedArray;
                            break;
                        }
                        case FONT_UNDERLINE_LONGDASH:
                        {
                            pDotDashArray = aLongDashArray;
                            break;
                        }
                        case FONT_UNDERLINE_DASHDOT:
                        {
                            pDotDashArray = aDotDashArray;
                            break;
                        }
                        case FONT_UNDERLINE_DASHDOTDOT:
                        {
                            pDotDashArray = aDashDotDotArray;
                            break;
                        }
                        case FONT_UNDERLINE_SMALLWAVE:
                        {
                            bWaveLine = true;
                            break;
                        }
                        case FONT_UNDERLINE_WAVE:
                        {
                            bWaveLine = true;
                            break;
                        }
                        case FONT_UNDERLINE_DOUBLEWAVE:
                        {
                            bDoubleLine = true;
                            bWaveLine = true;
                            break;
                        }
                        case FONT_UNDERLINE_BOLD:
                        {
                            bBoldLine = true;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDDOTTED:
                        {
                            bBoldLine = true;
                            pDotDashArray = aDottedArray;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDDASH:
                        {
                            bBoldLine = true;
                            pDotDashArray = aDashedArray;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDLONGDASH:
                        {
                            bBoldLine = true;
                            pDotDashArray = aLongDashArray;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDDASHDOT:
                        {
                            bBoldLine = true;
                            pDotDashArray = aDotDashArray;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDDASHDOTDOT:
                        {
                            bBoldLine = true;
                            pDotDashArray = aDashDotDotArray;
                            break;
                        }
                        case FONT_UNDERLINE_BOLDWAVE:
                        {
                            bWaveLine = true;
                            bBoldLine = true;
                            break;
                        }
                    }

                    if(bBoldLine)
                    {
                        fUnderlineHeight *= 2.0;
                    }

                    if(bDoubleLine)
                    {
                        fUnderlineOffset -= 0.50 * fUnderlineHeight;
                        fUnderlineHeight *= 0.64;
                    }

                    if(bWaveLine)
                    {
                        eLineJoin = basegfx::B2DLINEJOIN_ROUND;
                        fUnderlineHeight *= 0.5;
                    }

                    // prepare Line and Stroke Attributes
                    const attribute::LineAttribute aLineAttribute(getTextlineColor(), fUnderlineHeight, eLineJoin);
                    attribute::StrokeAttribute aStrokeAttribute;

                    if(pDotDashArray)
                    {
                        ::std::vector< double > aDoubleArray;

                        for(const int* p = pDotDashArray; *p; ++p)
                        {
                            aDoubleArray.push_back((double)(*p) * fUnderlineHeight);
                        }

                        aStrokeAttribute = attribute::StrokeAttribute(aDoubleArray);
                    }

                    // create base polygon and new primitive
                    basegfx::B2DPolygon aUnderline;
                    Primitive2DReference aNewPrimitive;

                    aUnderline.append(basegfx::B2DPoint(0.0, fUnderlineOffset));
                    aUnderline.append(basegfx::B2DPoint(fTextWidth, fUnderlineOffset));
                    aUnderline.transform(aUnscaledTransform);

                    if(bWaveLine)
                    {
                        double fWaveWidth(4.0 * fUnderlineHeight);

                        if(primitive2d::FONT_UNDERLINE_SMALLWAVE == getFontUnderline())
                        {
                            fWaveWidth *= 0.7;
                        }
                        else if(primitive2d::FONT_UNDERLINE_WAVE == getFontUnderline())
                        {
                            // extra multiply to get the same WaveWidth as with the bold version
                            fWaveWidth *= 2.0;
                        }

                        aNewPrimitive = Primitive2DReference(new PolygonWavePrimitive2D(aUnderline, aLineAttribute, aStrokeAttribute, fWaveWidth, 0.5 * fWaveWidth));
                    }
                    else
                    {
                        aNewPrimitive = Primitive2DReference(new PolygonStrokePrimitive2D(aUnderline, aLineAttribute, aStrokeAttribute));
                    }

                    // add primitive
                    rTarget.push_back(aNewPrimitive);

                    if(bDoubleLine)
                    {
                        // double line, create 2nd primitive with offset using TransformPrimitive based on
                        // already created NewPrimitive
                        const double fLineDist((bWaveLine ? 3.0 : 2.0) * fUnderlineHeight);
                        basegfx::B2DHomMatrix aTransform;

                        // move base point of text to 0.0 and de-rotate
                        aTransform.translate(-rDecTrans.getTranslate().getX(), -rDecTrans.getTranslate().getY());
                        aTransform.rotate(-rDecTrans.getRotate());

                        // translate in Y by offset
                        aTransform.translate(0.0, fLineDist);

                        // move back and rotate
                        aTransform.rotate(rDecTrans.getRotate());
                        aTransform.translate(rDecTrans.getTranslate().getX(), rDecTrans.getTranslate().getY());

                        // add transform primitive
                        const Primitive2DSequence aContent(&aNewPrimitive, 1);
                        rTarget.push_back(Primitive2DReference(new TransformPrimitive2D(aTransform, aContent)));
                    }
                }

                if(bStrikeoutUsed)
                {
                    // create primitive geometry for strikeout
                    if(FONT_STRIKEOUT_SLASH == getFontStrikeout() || FONT_STRIKEOUT_X == getFontStrikeout())
                    {
                        // strikeout with character
                        const sal_Unicode aStrikeoutChar(FONT_STRIKEOUT_SLASH == getFontStrikeout() ? '/' : 'X');
                        const String aSingleCharString(aStrikeoutChar);
                        const double fStrikeCharWidth(aTextLayouter.getTextWidth(aSingleCharString, 0, 1));
                        const double fStrikeCharCount(fabs(fTextWidth/fStrikeCharWidth));
                        const sal_uInt32 nStrikeCharCount(static_cast< sal_uInt32 >(fStrikeCharCount + 0.9));
                        const double fScaleX(rDecTrans.getScale().getX());
                        const double fStrikeCharWidthUnscaled(basegfx::fTools::equalZero(fScaleX) ? fStrikeCharWidth : fStrikeCharWidth/fScaleX);

                        std::vector<double> aDXArray(nStrikeCharCount);
                        String aStrikeoutString;

                        for(sal_uInt32 a(0); a < nStrikeCharCount; a++)
                        {
                            aStrikeoutString += aSingleCharString;
                            aDXArray[a] = (a + 1) * fStrikeCharWidthUnscaled;
                        }

                        rTarget.push_back(Primitive2DReference(new TextSimplePortionPrimitive2D(
                            rDecTrans.getB2DHomMatrix(),
                            aStrikeoutString,
                            0,
                            aStrikeoutString.Len(),
                            aDXArray,
                            rFontAttributes,
                            getLocale(),
                            getFontColor())));
                    }
                    else
                    {
                        // strikeout with geometry
                        double fStrikeoutHeight(aTextLayouter.getUnderlineHeight());
                        double fStrikeoutOffset(aTextLayouter.getStrikeoutOffset());
                        bool bDoubleLine(false);

                        // set Underline attribute
                        switch(getFontStrikeout())
                        {
                            default : // case primitive2d::FONT_STRIKEOUT_SINGLE:
                            {
                                break;
                            }
                            case primitive2d::FONT_STRIKEOUT_DOUBLE:
                            {
                                bDoubleLine = true;
                                break;
                            }
                            case primitive2d::FONT_STRIKEOUT_BOLD:
                            {
                                fStrikeoutHeight *= 2.0;
                                break;
                            }
                        }

                        if(bDoubleLine)
                        {
                            fStrikeoutOffset -= 0.50 * fStrikeoutHeight;
                            fStrikeoutHeight *= 0.64;
                        }

                        // create base polygon and new primitive
                        basegfx::B2DPolygon aStrikeoutLine;

                        aStrikeoutLine.append(basegfx::B2DPoint(0.0, -fStrikeoutOffset));
                        aStrikeoutLine.append(basegfx::B2DPoint(fTextWidth, -fStrikeoutOffset));
                        aStrikeoutLine.transform(aUnscaledTransform);

                        const attribute::LineAttribute aLineAttribute(getFontColor(), fStrikeoutHeight, basegfx::B2DLINEJOIN_NONE);
                        Primitive2DReference aNewPrimitive(new PolygonStrokePrimitive2D(aStrikeoutLine, aLineAttribute));

                        // add primitive
                        rTarget.push_back(aNewPrimitive);

                        if(bDoubleLine)
                        {
                            // double line, create 2nd primitive with offset using TransformPrimitive based on
                            // already created NewPrimitive
                            const double fLineDist(2.0 * fStrikeoutHeight);
                            basegfx::B2DHomMatrix aTransform;

                            // move base point of text to 0.0 and de-rotate
                            aTransform.translate(-rDecTrans.getTranslate().getX(), -rDecTrans.getTranslate().getY());
                            aTransform.rotate(-rDecTrans.getRotate());

                            // translate in Y by offset
                            aTransform.translate(0.0, -fLineDist);

                            // move back and rotate
                            aTransform.rotate(rDecTrans.getRotate());
                            aTransform.translate(rDecTrans.getTranslate().getX(), rDecTrans.getTranslate().getY());

                            // add transform primitive
                            const Primitive2DSequence aContent(&aNewPrimitive, 1);
                            rTarget.push_back(Primitive2DReference(new TransformPrimitive2D(aTransform, aContent)));
                        }
                    }
                }
            }

            // TODO: Handle Font Emphasis Above/Below
        }

        void TextDecoratedPortionPrimitive2D::impCorrectTextBoundary(::com::sun::star::i18n::Boundary& rNextWordBoundary) const
        {
            // truncate aNextWordBoundary to min/max possible values. This is necessary since the word start may be
            // before/after getTextPosition() when a long string is the content and getTextPosition()
            // is right inside a word. Same for end.
            const sal_Int32 aMinPos(static_cast< sal_Int32 >(getTextPosition()));
            const sal_Int32 aMaxPos(aMinPos + static_cast< sal_Int32 >(getTextLength()));

            if(rNextWordBoundary.startPos < aMinPos)
            {
                rNextWordBoundary.startPos = aMinPos;
            }
            else if(rNextWordBoundary.startPos > aMaxPos)
            {
                rNextWordBoundary.startPos = aMaxPos;
            }

            if(rNextWordBoundary.endPos < aMinPos)
            {
                rNextWordBoundary.endPos = aMinPos;
            }
            else if(rNextWordBoundary.endPos > aMaxPos)
            {
                rNextWordBoundary.endPos = aMaxPos;
            }
        }

        void TextDecoratedPortionPrimitive2D::impSplitSingleWords(
            std::vector< Primitive2DReference >& rTarget,
            basegfx::DecomposedB2DHomMatrixContainer& rDecTrans) const
        {
            // break iterator support
            // made static so it only needs to be fetched once, even with many single
            // constructed VclMetafileProcessor2D. It's still incarnated on demand,
            // but exists for OOo runtime now by purpose.
            static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xLocalBreakIterator;

            if(!xLocalBreakIterator.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF(::comphelper::getProcessServiceFactory());
                xLocalBreakIterator.set(xMSF->createInstance(rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator")), ::com::sun::star::uno::UNO_QUERY);
            }

            if(xLocalBreakIterator.is())
            {
                // init word iterator, get first word and truncate to possibilities
                ::com::sun::star::i18n::Boundary aNextWordBoundary(xLocalBreakIterator->getWordBoundary(
                    getText(), getTextPosition(), getLocale(), ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True));
                impCorrectTextBoundary(aNextWordBoundary);

                // prepare new font attributes WITHOUT outline
                const FontAttributes aNewFontAttributes(
                    getFontAttributes().getFamilyName(),
                    getFontAttributes().getStyleName(),
                    getFontAttributes().getWeight(),
                    getFontAttributes().getSymbol(),
                    getFontAttributes().getVertical(),
                    getFontAttributes().getItalic(),
                    false,             // no outline anymore, handled locally
                    getFontAttributes().getRTL(),
                    getFontAttributes().getBiDiStrong());

                if(aNextWordBoundary.startPos == getTextPosition() && aNextWordBoundary.endPos == getTextLength())
                {
                    // it IS only a single word, handle as one word
                    impCreateGeometryContent(rTarget, rDecTrans, getText(), getTextPosition(), getTextLength(), getDXArray(), aNewFontAttributes);
                }
                else
                {
                    // do iterate over single words
                    while(aNextWordBoundary.startPos != aNextWordBoundary.endPos)
                    {
                        // prepare values for new portion
                        const xub_StrLen nNewTextStart(static_cast< xub_StrLen >(aNextWordBoundary.startPos));
                        const xub_StrLen nNewTextEnd(static_cast< xub_StrLen >(aNextWordBoundary.endPos));

                        // prepare transform for the single word
                        basegfx::B2DHomMatrix aNewTransform;
                        double fDistance(0.0);
                        const bool bNewStartIsNotOldStart(nNewTextStart > getTextPosition());

                        if(bNewStartIsNotOldStart)
                        {
                            // needs to be moved to a new start position (get from DXArray)
                            const sal_uInt32 nIndex(static_cast< sal_uInt32 >(nNewTextStart - getTextPosition()));
                            fDistance = getDXArray()[nIndex - 1];
                            aNewTransform.translate(fDistance, 0.0);
                        }

                        aNewTransform *= rDecTrans.getB2DHomMatrix();

                        // prepare new DXArray for the single word
                        ::std::vector< double > aNewDXArray(
                            getDXArray().begin() + static_cast< sal_uInt32 >(nNewTextStart - getTextPosition()),
                            getDXArray().begin() + static_cast< sal_uInt32 >(nNewTextEnd - getTextPosition()));

                        if(bNewStartIsNotOldStart)
                        {
                            // DXArray values need to be corrected
                            const sal_uInt32 nArraySize(aNewDXArray.size());

                            for(sal_uInt32 a(0); a < nArraySize; a++)
                            {
                                aNewDXArray[a] -= fDistance;
                            }
                        }

                        // create geometry content for the single word
                        basegfx::DecomposedB2DHomMatrixContainer aDecTrans(aNewTransform);
                        impCreateGeometryContent(rTarget, aDecTrans, getText(), nNewTextStart,
                            nNewTextEnd - nNewTextStart, aNewDXArray, aNewFontAttributes);

                        // prepare next word and truncate to possibilities
                        aNextWordBoundary = xLocalBreakIterator->nextWord(
                            getText(), aNextWordBoundary.endPos, getLocale(),
                            ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES);
                        impCorrectTextBoundary(aNextWordBoundary);
                    }
                }
            }
        }

        Primitive2DSequence TextDecoratedPortionPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            std::vector< Primitive2DReference > aNewPrimitives;
            basegfx::DecomposedB2DHomMatrixContainer aDecTrans(getTextTransform());
            Primitive2DSequence aRetval;

            // create basic geometry such as SimpleTextPrimitive, Underline,
            // Strikeuot, etc...
            if(getWordLineMode())
            {
                // support for single word mode
                impSplitSingleWords(aNewPrimitives, aDecTrans);
            }
            else
            {
                // prepare new font attributes WITHOUT outline
                const FontAttributes aNewFontAttributes(
                    getFontAttributes().getFamilyName(),
                    getFontAttributes().getStyleName(),
                    getFontAttributes().getWeight(),
                    getFontAttributes().getSymbol(),
                    getFontAttributes().getVertical(),
                    getFontAttributes().getItalic(),
                    false,             // no outline anymore, handled locally
                    getFontAttributes().getRTL(),
                    getFontAttributes().getBiDiStrong());

                // handle as one word
                impCreateGeometryContent(aNewPrimitives, aDecTrans, getText(), getTextPosition(), getTextLength(), getDXArray(), aNewFontAttributes);
            }

            // convert to Primitive2DSequence
            const sal_uInt32 nMemberCount(aNewPrimitives.size());

            if(nMemberCount)
            {
                aRetval.realloc(nMemberCount);

                for(sal_uInt32 a(0); a < nMemberCount; a++)
                {
                    aRetval[a] = aNewPrimitives[a];
                }
            }

            // Handle Shadow, Outline and FontRelief
            if(aRetval.hasElements())
            {
                // outline AND shadow depend on NO FontRelief (see dialog)
                const bool bHasFontRelief(FONT_RELIEF_NONE != getFontRelief());
                const bool bHasShadow(!bHasFontRelief && getShadow());
                const bool bHasOutline(!bHasFontRelief && getFontAttributes().getOutline());

                if(bHasShadow || bHasFontRelief || bHasOutline)
                {
                    Primitive2DReference aShadow;

                    if(bHasShadow)
                    {
                        // create shadow with current content (in aRetval). Text shadow
                        // is constant, relative to font size, rotated with the text and has a
                        // constant color.
                        // shadow parameter values
                        static double fFactor(1.0 / 24.0);
                        const double fTextShadowOffset(aDecTrans.getScale().getY() * fFactor);
                        static basegfx::BColor aShadowColor(0.3, 0.3, 0.3);

                        // preapare shadow transform matrix
                        basegfx::B2DHomMatrix aShadowTransform;
                        aShadowTransform.translate(fTextShadowOffset, fTextShadowOffset);

                        // create shadow primitive
                        aShadow = Primitive2DReference(new ShadowPrimitive2D(
                            aShadowTransform,
                            aShadowColor,
                            aRetval));
                    }

                    if(bHasFontRelief)
                    {
                        // create emboss using an own helper primitive since this will
                        // be view-dependent
                        const basegfx::BColor aBBlack(0.0, 0.0, 0.0);
                        const bool bDefaultTextColor(aBBlack == getFontColor());
                        TextEffectStyle2D aTextEffectStyle2D(TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED);

                        if(bDefaultTextColor)
                        {
                            if(FONT_RELIEF_ENGRAVED == getFontRelief())
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT;
                            }
                            else
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT;
                            }
                        }
                        else
                        {
                            if(FONT_RELIEF_ENGRAVED == getFontRelief())
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED;
                            }
                            else
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED;
                            }
                        }

                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            aTextEffectStyle2D));
                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }
                    else if(bHasOutline)
                    {
                        // create outline using an own helper primitive since this will
                        // be view-dependent
                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            TEXTEFFECTSTYLE2D_OUTLINE));
                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }

                    if(aShadow.is())
                    {
                        // put shadow in front if there is one to paint timely before
                        // but placed behind content
                        const Primitive2DSequence aContent(aRetval);
                        aRetval = Primitive2DSequence(&aShadow, 1);
                        appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aContent);
                    }
                }
            }

            return aRetval;
        }

        TextDecoratedPortionPrimitive2D::TextDecoratedPortionPrimitive2D(

            // TextSimplePortionPrimitive2D parameters
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            xub_StrLen aTextPosition,
            xub_StrLen aTextLength,
            const ::std::vector< double >& rDXArray,
            const FontAttributes& rFontAttributes,
            const ::com::sun::star::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor,

            // local parameters
            const basegfx::BColor& rTextlineColor,
            FontUnderline eFontUnderline,
            bool bUnderlineAbove,
            FontStrikeout eFontStrikeout,
            bool bWordLineMode,
            FontEmphasisMark eFontEmphasisMark,
            bool bEmphasisMarkAbove,
            bool bEmphasisMarkBelow,
            FontRelief eFontRelief,
            bool bShadow)
        :   TextSimplePortionPrimitive2D(rNewTransform, rText, aTextPosition, aTextLength, rDXArray, rFontAttributes, rLocale, rFontColor),
            maTextlineColor(rTextlineColor),
            meFontUnderline(eFontUnderline),
            meFontStrikeout(eFontStrikeout),
            meFontEmphasisMark(eFontEmphasisMark),
            meFontRelief(eFontRelief),
            mbUnderlineAbove(bUnderlineAbove),
            mbWordLineMode(bWordLineMode),
            mbEmphasisMarkAbove(bEmphasisMarkAbove),
            mbEmphasisMarkBelow(bEmphasisMarkBelow),
            mbShadow(bShadow)
        {
        }

        bool TextDecoratedPortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(TextSimplePortionPrimitive2D::operator==(rPrimitive))
            {
                const TextDecoratedPortionPrimitive2D& rCompare = (TextDecoratedPortionPrimitive2D&)rPrimitive;

                return (getTextlineColor() == rCompare.getTextlineColor()
                    && getFontUnderline() == rCompare.getFontUnderline()
                    && getFontStrikeout() == rCompare.getFontStrikeout()
                    && getFontEmphasisMark() == rCompare.getFontEmphasisMark()
                    && getFontRelief() == rCompare.getFontRelief()
                    && getUnderlineAbove() == rCompare.getUnderlineAbove()
                    && getWordLineMode() == rCompare.getWordLineMode()
                    && getEmphasisMarkAbove() == rCompare.getEmphasisMarkAbove()
                    && getEmphasisMarkBelow() == rCompare.getEmphasisMarkBelow()
                    && getShadow() == rCompare.getShadow());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextDecoratedPortionPrimitive2D, PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
