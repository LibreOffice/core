/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svdotext.hxx>
#include <svx/svdoutl.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <algorithm>
#include <svx/xtextit.hxx>
#include <svx/xftshtit.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <editeng/unolingu.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// primitive decomposition helpers

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <editeng/editstat.hxx>
#include <svx/unoapi.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/attribute/sdrformtextoutlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

//////////////////////////////////////////////////////////////////////////////
// PathTextPortion helper

namespace
{
    class impPathTextPortion
    {
        basegfx::B2DVector                          maOffset;
        OUString                                    maText;
        xub_StrLen                                  mnTextStart;
        xub_StrLen                                  mnTextLength;
        sal_Int32                                   mnParagraph;
        xub_StrLen                                  mnIndex;
        SvxFont                                     maFont;
        ::std::vector< double >                     maDblDXArray;   // double DXArray, font size independent -> unit coordinate system
        ::com::sun::star::lang::Locale              maLocale;

        // bitfield
        unsigned                                    mbRTL : 1;

    public:
        impPathTextPortion(DrawPortionInfo& rInfo)
        :   maOffset(rInfo.mrStartPos.X(), rInfo.mrStartPos.Y()),
            maText(rInfo.mrText),
            mnTextStart(rInfo.mnTextStart),
            mnTextLength(rInfo.mnTextLen),
            mnParagraph(rInfo.mnPara),
            mnIndex(rInfo.mnIndex),
            maFont(rInfo.mrFont),
            maDblDXArray(),
            maLocale(rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale()),
            mbRTL(rInfo.mrFont.IsVertical() ? false : rInfo.IsRTL())
        {
            if(mnTextLength && rInfo.mpDXArray)
            {
                maDblDXArray.reserve(mnTextLength);

                for(xub_StrLen a(0); a < mnTextLength; a++)
                {
                    maDblDXArray.push_back((double)rInfo.mpDXArray[a]);
                }
            }
        }

        // for ::std::sort
        bool operator<(const impPathTextPortion& rComp) const
        {
            if(mnParagraph < rComp.mnParagraph)
            {
                return true;
            }

            if(maOffset.getX() < rComp.maOffset.getX())
            {
                return true;
            }

            return (maOffset.getY() < rComp.maOffset.getY());
        }

        const basegfx::B2DVector& getOffset() const { return maOffset; }
        const OUString& getText() const { return maText; }
        xub_StrLen getTextStart() const { return mnTextStart; }
        xub_StrLen getTextLength() const { return mnTextLength; }
        sal_Int32 getParagraph() const { return mnParagraph; }
        xub_StrLen getIndex() const { return mnIndex; }
        const SvxFont& getFont() const { return maFont; }
        bool isRTL() const { return mbRTL; }
        const ::std::vector< double >& getDoubleDXArray() const { return maDblDXArray; }
        const ::com::sun::star::lang::Locale& getLocale() const { return maLocale; }

        xub_StrLen getPortionIndex(xub_StrLen nIndex, xub_StrLen nLength) const
        {
            if(mbRTL)
            {
                return (mnTextStart + (mnTextLength - (nIndex + nLength)));
            }
            else
            {
                return (mnTextStart + nIndex);
            }
        }

        double getDisplayLength(xub_StrLen nIndex, xub_StrLen nLength) const
        {
            drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
            double fRetval(0.0);

            if(maFont.IsVertical())
            {
                fRetval = aTextLayouter.getTextHeight() * (double)nLength;
            }
            else
            {
                fRetval = aTextLayouter.getTextWidth(maText, getPortionIndex(nIndex, nLength), nLength);
            }

            return fRetval;
        }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// TextBreakup helper

namespace
{
    class impTextBreakupHandler
    {
        SdrOutliner&                                mrOutliner;
        ::std::vector< impPathTextPortion >         maPathTextPortions;

        DECL_LINK(decompositionPathTextPrimitive, DrawPortionInfo* );

    public:
        impTextBreakupHandler(SdrOutliner& rOutliner)
        :   mrOutliner(rOutliner)
        {
        }

        const ::std::vector< impPathTextPortion >& decompositionPathTextPrimitive()
        {
            // strip portions to maPathTextPortions
            mrOutliner.SetDrawPortionHdl(LINK(this, impTextBreakupHandler, decompositionPathTextPrimitive));
            mrOutliner.StripPortions();

            if(!maPathTextPortions.empty())
            {
                // sort portions by paragraph, x and y
                ::std::sort(maPathTextPortions.begin(), maPathTextPortions.end());
            }

            return maPathTextPortions;
        }
    };

    IMPL_LINK(impTextBreakupHandler, decompositionPathTextPrimitive, DrawPortionInfo*, pInfo)
    {
        maPathTextPortions.push_back(impPathTextPortion(*pInfo));
        return 0;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// TextBreakup one poly and one paragraph helper

namespace
{
    class impPolygonParagraphHandler
    {
        const drawinglayer::attribute::SdrFormTextAttribute         maSdrFormTextAttribute; // FormText parameters
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& mrDecomposition;        // destination primitive list
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& mrShadowDecomposition;  // destination primitive list for shadow
        Reference < com::sun::star::i18n::XBreakIterator >          mxBreak;                // break iterator

        double getParagraphTextLength(const ::std::vector< const impPathTextPortion* >& rTextPortions)
        {
            drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
            double fRetval(0.0);

            for(sal_uInt32 a(0L); a < rTextPortions.size(); a++)
            {
                const impPathTextPortion* pCandidate = rTextPortions[a];

                if(pCandidate && pCandidate->getTextLength())
                {
                    aTextLayouter.setFont(pCandidate->getFont());
                    fRetval += pCandidate->getDisplayLength(0L, pCandidate->getTextLength());
                }
            }

            return fRetval;
        }

        xub_StrLen getNextGlyphLen(const impPathTextPortion* pCandidate, xub_StrLen nPosition, const ::com::sun::star::lang::Locale& rFontLocale)
        {
            xub_StrLen nNextGlyphLen(1);

            if(mxBreak.is())
            {
                sal_Int32 nDone(0L);
                nNextGlyphLen = (xub_StrLen)mxBreak->nextCharacters(pCandidate->getText(), nPosition,
                    rFontLocale, CharacterIteratorMode::SKIPCELL, 1, nDone) - nPosition;
            }

            return nNextGlyphLen;
        }

    public:
        impPolygonParagraphHandler(
            const drawinglayer::attribute::SdrFormTextAttribute& rSdrFormTextAttribute,
            std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rDecomposition,
            std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rShadowDecomposition)
        :   maSdrFormTextAttribute(rSdrFormTextAttribute),
            mrDecomposition(rDecomposition),
            mrShadowDecomposition(rShadowDecomposition)
        {
            // prepare BreakIterator
            Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            mxBreak = com::sun::star::i18n::BreakIterator::create(xContext);
        }

        void HandlePair(const basegfx::B2DPolygon rPolygonCandidate, const ::std::vector< const impPathTextPortion* >& rTextPortions)
        {
            // prepare polygon geometry, take into account as many parameters as possible
            basegfx::B2DPolygon aPolygonCandidate(rPolygonCandidate);
            const double fPolyLength(basegfx::tools::getLength(aPolygonCandidate));
            double fPolyEnd(fPolyLength);
            double fPolyStart(0.0);
            double fAutosizeScaleFactor(1.0);
            bool bAutosizeScale(false);

            if(maSdrFormTextAttribute.getFormTextMirror())
            {
                aPolygonCandidate.flip();
            }

            if(maSdrFormTextAttribute.getFormTextStart()
                && (XFT_LEFT == maSdrFormTextAttribute.getFormTextAdjust()
                    || XFT_RIGHT == maSdrFormTextAttribute.getFormTextAdjust()))
            {
                if(XFT_LEFT == maSdrFormTextAttribute.getFormTextAdjust())
                {
                    fPolyStart += maSdrFormTextAttribute.getFormTextStart();

                    if(fPolyStart > fPolyEnd)
                    {
                        fPolyStart = fPolyEnd;
                    }
                }
                else
                {
                    fPolyEnd -= maSdrFormTextAttribute.getFormTextStart();

                    if(fPolyEnd < fPolyStart)
                    {
                        fPolyEnd = fPolyStart;
                    }
                }
            }

            if(XFT_LEFT != maSdrFormTextAttribute.getFormTextAdjust())
            {
                // calculate total text length of this paragraph, some layout needs to be done
                const double fParagraphTextLength(getParagraphTextLength(rTextPortions));

                // check if text is too long for paragraph. If yes, handle as if left aligned (default),
                // but still take care of XFT_AUTOSIZE in that case
                const bool bTextTooLong(fParagraphTextLength > (fPolyEnd - fPolyStart));

                if(XFT_RIGHT == maSdrFormTextAttribute.getFormTextAdjust())
                {
                    if(!bTextTooLong)
                    {
                        // if right aligned, add difference to polygon start
                        fPolyStart += ((fPolyEnd - fPolyStart) - fParagraphTextLength);
                    }
                }
                else if(XFT_CENTER == maSdrFormTextAttribute.getFormTextAdjust())
                {
                    if(!bTextTooLong)
                    {
                        // if centered, add half of difference to polygon start
                        fPolyStart += ((fPolyEnd - fPolyStart) - fParagraphTextLength) / 2.0;
                    }
                }
                else if(XFT_AUTOSIZE == maSdrFormTextAttribute.getFormTextAdjust())
                {
                    // if scale, prepare scale factor between curve length and text length
                    if(0.0 != fParagraphTextLength)
                    {
                        fAutosizeScaleFactor = (fPolyEnd - fPolyStart) / fParagraphTextLength;
                        bAutosizeScale = true;
                    }
                }
            }

            // handle text portions for this paragraph
            for(sal_uInt32 a(0L); a < rTextPortions.size() && fPolyStart < fPolyEnd; a++)
            {
                const impPathTextPortion* pCandidate = rTextPortions[a];
                basegfx::B2DVector aFontScaling;

                if(pCandidate && pCandidate->getTextLength())
                {
                    const drawinglayer::attribute::FontAttribute aCandidateFontAttribute(
                        drawinglayer::primitive2d::getFontAttributeFromVclFont(
                            aFontScaling,
                            pCandidate->getFont(),
                            pCandidate->isRTL(),
                            false));

                    drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
                    aTextLayouter.setFont(pCandidate->getFont());
                    xub_StrLen nUsedTextLength(0);

                    while(nUsedTextLength < pCandidate->getTextLength() && fPolyStart < fPolyEnd)
                    {
                        xub_StrLen nNextGlyphLen(getNextGlyphLen(pCandidate, pCandidate->getTextStart() + nUsedTextLength, pCandidate->getLocale()));

                        // prepare portion length. Takes RTL sections into account.
                        double fPortionLength(pCandidate->getDisplayLength(nUsedTextLength, nNextGlyphLen));

                        if(bAutosizeScale)
                        {
                            // when autosize scaling, expand portion length
                            fPortionLength *= fAutosizeScaleFactor;
                        }

                        // create transformation
                        basegfx::B2DHomMatrix aNewTransformA, aNewTransformB, aNewShadowTransform;
                        basegfx::B2DPoint aStartPos(basegfx::tools::getPositionAbsolute(aPolygonCandidate, fPolyStart, fPolyLength));
                        basegfx::B2DPoint aEndPos(aStartPos);

                        // add font scaling
                        aNewTransformA.scale(aFontScaling.getX(), aFontScaling.getY());

                        // prepare scaling of text primitive
                        if(bAutosizeScale)
                        {
                            // when autosize scaling, expand text primitive scaling to it
                            aNewTransformA.scale(fAutosizeScaleFactor, fAutosizeScaleFactor);
                        }

                        // eventually create shadow primitives from aDecomposition and add to rDecomposition
                        const bool bShadow(XFTSHADOW_NONE != maSdrFormTextAttribute.getFormTextShadow());

                        if(bShadow)
                        {
                            if(XFTSHADOW_NORMAL == maSdrFormTextAttribute.getFormTextShadow())
                            {
                                aNewShadowTransform.translate(
                                    maSdrFormTextAttribute.getFormTextShdwXVal(),
                                    -maSdrFormTextAttribute.getFormTextShdwYVal());
                            }
                            else // XFTSHADOW_SLANT
                            {
                                double fScaleValue(maSdrFormTextAttribute.getFormTextShdwYVal() / 100.0);
                                double fShearValue(-maSdrFormTextAttribute.getFormTextShdwXVal() * F_PI1800);

                                aNewShadowTransform.scale(1.0, fScaleValue);
                                aNewShadowTransform.shearX(sin(fShearValue));
                                aNewShadowTransform.scale(1.0, cos(fShearValue));
                            }
                        }

                        switch(maSdrFormTextAttribute.getFormTextStyle())
                        {
                            case XFT_ROTATE :
                            {
                                aEndPos = basegfx::tools::getPositionAbsolute(aPolygonCandidate, fPolyStart + fPortionLength, fPolyLength);
                                const basegfx::B2DVector aDirection(aEndPos - aStartPos);
                                aNewTransformB.rotate(atan2(aDirection.getY(), aDirection.getX()));
                                aNewTransformB.translate(aStartPos.getX(), aStartPos.getY());

                                break;
                            }
                            case XFT_UPRIGHT :
                            {
                                aNewTransformB.translate(aStartPos.getX() - (fPortionLength / 2.0), aStartPos.getY());

                                break;
                            }
                            case XFT_SLANTX :
                            {
                                aEndPos = basegfx::tools::getPositionAbsolute(aPolygonCandidate, fPolyStart + fPortionLength, fPolyLength);
                                const basegfx::B2DVector aDirection(aEndPos - aStartPos);
                                const double fShearValue(atan2(aDirection.getY(), aDirection.getX()));
                                const double fSin(sin(fShearValue));
                                const double fCos(cos(fShearValue));

                                   aNewTransformB.shearX(-fSin);

                                // Scale may lead to objects without height since fCos == 0.0 is possible.
                                // Renderers need to handle that, it's not a forbidden value and does not
                                // need to be avoided
                                aNewTransformB.scale(1.0, fCos);
                                aNewTransformB.translate(aStartPos.getX() - (fPortionLength / 2.0), aStartPos.getY());

                                break;
                            }
                            case XFT_SLANTY :
                            {
                                aEndPos = basegfx::tools::getPositionAbsolute(aPolygonCandidate, fPolyStart + fPortionLength, fPolyLength);
                                const basegfx::B2DVector aDirection(aEndPos - aStartPos);
                                const double fShearValue(atan2(aDirection.getY(), aDirection.getX()));
                                const double fCos(cos(fShearValue));
                                const double fTan(tan(fShearValue));

                                // shear to 'stand' on the curve
                                aNewTransformB.shearY(fTan);

                                // scale in X to make as tight as needed. As with XFT_SLANT_X, this may
                                // lead to primitives without width which the renderers will handle
                                   aNewTransformA.scale(fCos, 1.0);

                                aNewTransformB.translate(aStartPos.getX(), aStartPos.getY());

                                break;
                            }
                            default : break; // XFT_NONE
                        }

                        // distance from path?
                        if(maSdrFormTextAttribute.getFormTextDistance())
                        {
                            if(aEndPos.equal(aStartPos))
                            {
                                aEndPos = basegfx::tools::getPositionAbsolute(aPolygonCandidate, fPolyStart + fPortionLength, fPolyLength);
                            }

                            // use back vector (aStartPos - aEndPos) here to get mirrored perpendicular as in old stuff
                            const basegfx::B2DVector aPerpendicular(
                                basegfx::getNormalizedPerpendicular(aStartPos - aEndPos) *
                                maSdrFormTextAttribute.getFormTextDistance());
                            aNewTransformB.translate(aPerpendicular.getX(), aPerpendicular.getY());
                        }

                        if(!pCandidate->getText().isEmpty() && nNextGlyphLen)
                        {
                            const xub_StrLen nPortionIndex(pCandidate->getPortionIndex(nUsedTextLength, nNextGlyphLen));
                            ::std::vector< double > aNewDXArray;

                            if(nNextGlyphLen > 1 && pCandidate->getDoubleDXArray().size())
                            {
                                // copy DXArray for portion
                                aNewDXArray.insert(
                                    aNewDXArray.begin(),
                                    pCandidate->getDoubleDXArray().begin() + nPortionIndex,
                                    pCandidate->getDoubleDXArray().begin() + (nPortionIndex + nNextGlyphLen));

                                if(nPortionIndex > 0)
                                {
                                    // adapt to portion start
                                    double fDXOffset= *(pCandidate->getDoubleDXArray().begin() + (nPortionIndex - 1));
                                    ::std::transform(
                                        aNewDXArray.begin(), aNewDXArray.end(),
                                        aNewDXArray.begin(), ::std::bind2nd(::std::minus<double>(), fDXOffset));
                                }

                                if(bAutosizeScale)
                                {
                                    // when autosize scaling, adapt to DXArray, too
                                    ::std::transform(
                                        aNewDXArray.begin(), aNewDXArray.end(),
                                        aNewDXArray.begin(), ::std::bind2nd(::std::multiplies<double>(), fAutosizeScaleFactor));
                                }
                            }

                            if(bShadow)
                            {
                                // shadow primitive creation
                                const Color aShadowColor(maSdrFormTextAttribute.getFormTextShdwColor());
                                const basegfx::BColor aRGBShadowColor(aShadowColor.getBColor());

                                drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pNew =
                                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                        aNewTransformB * aNewShadowTransform * aNewTransformA,
                                        pCandidate->getText(),
                                        nPortionIndex,
                                        nNextGlyphLen,
                                        aNewDXArray,
                                        aCandidateFontAttribute,
                                        pCandidate->getLocale(),
                                        aRGBShadowColor);

                                mrShadowDecomposition.push_back(pNew);
                            }

                            {
                                // primitive creation
                                const Color aColor(pCandidate->getFont().GetColor());
                                const basegfx::BColor aRGBColor(aColor.getBColor());

                                drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pNew =
                                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                        aNewTransformB * aNewTransformA,
                                        pCandidate->getText(),
                                        nPortionIndex,
                                        nNextGlyphLen,
                                        aNewDXArray,
                                        aCandidateFontAttribute,
                                        pCandidate->getLocale(),
                                        aRGBColor);

                                mrDecomposition.push_back(pNew);
                            }
                        }

                        // consume from portion // no += here, xub_StrLen is sal_uInt16 and the compiler will generate a warning here
                        nUsedTextLength = nUsedTextLength + nNextGlyphLen;

                        // consume from polygon
                        fPolyStart += fPortionLength;
                    }
                }
            }
        }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// primitive decomposition helpers

namespace
{
    void impAddPolygonStrokePrimitives(
        const basegfx::B2DPolyPolygonVector& rB2DPolyPolyVector,
        const basegfx::B2DHomMatrix& rTransform,
        const drawinglayer::attribute::LineAttribute& rLineAttribute,
        const drawinglayer::attribute::StrokeAttribute& rStrokeAttribute,
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rTarget)
    {
        for(basegfx::B2DPolyPolygonVector::const_iterator aPolygon(rB2DPolyPolyVector.begin()); aPolygon != rB2DPolyPolyVector.end(); ++aPolygon)
        {
            // prepare PolyPolygons
            basegfx::B2DPolyPolygon aB2DPolyPolygon = *aPolygon;
            aB2DPolyPolygon.transform(rTransform);

            for(sal_uInt32 a(0L); a < aB2DPolyPolygon.count(); a++)
            {
                // create one primitive per polygon
                drawinglayer::primitive2d::PolygonStrokePrimitive2D* pNew =
                    new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                        aB2DPolyPolygon.getB2DPolygon(a), rLineAttribute, rStrokeAttribute);
                rTarget.push_back(pNew);
            }
        }
    }

    drawinglayer::primitive2d::Primitive2DSequence impAddPathTextOutlines(
        const std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rSource,
        const drawinglayer::attribute::SdrFormTextOutlineAttribute& rOutlineAttribute)
    {
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aNewPrimitives;

        for(sal_uInt32 a(0L); a < rSource.size(); a++)
        {
            const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* pTextCandidate = dynamic_cast< const drawinglayer::primitive2d::TextSimplePortionPrimitive2D* >(rSource[a]);

            if(pTextCandidate)
            {
                basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
                basegfx::B2DHomMatrix aPolygonTransform;

                // get text outlines and their object transformation
                pTextCandidate->getTextOutlinesAndTransformation(aB2DPolyPolyVector, aPolygonTransform);

                if(!aB2DPolyPolyVector.empty())
                {
                    // create stroke primitives
                    std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aStrokePrimitives;
                    impAddPolygonStrokePrimitives(
                        aB2DPolyPolyVector,
                        aPolygonTransform,
                        rOutlineAttribute.getLineAttribute(),
                        rOutlineAttribute.getStrokeAttribute(),
                        aStrokePrimitives);
                    const sal_uInt32 nStrokeCount(aStrokePrimitives.size());

                    if(nStrokeCount)
                    {
                        if(rOutlineAttribute.getTransparence())
                        {
                            // create UnifiedTransparencePrimitive2D
                            drawinglayer::primitive2d::Primitive2DSequence aStrokePrimitiveSequence(nStrokeCount);

                            for(sal_uInt32 b(0L); b < nStrokeCount; b++)
                            {
                                aStrokePrimitiveSequence[b] = drawinglayer::primitive2d::Primitive2DReference(aStrokePrimitives[b]);
                            }

                            drawinglayer::primitive2d::UnifiedTransparencePrimitive2D* pNew2 =
                                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                    aStrokePrimitiveSequence,
                                    (double)rOutlineAttribute.getTransparence() / 100.0);
                            aNewPrimitives.push_back(pNew2);
                        }
                        else
                        {
                            // add polygons to rDecomposition as polygonStrokePrimitives
                            aNewPrimitives.insert(aNewPrimitives.end(), aStrokePrimitives.begin(), aStrokePrimitives.end());
                        }
                    }
                }
            }
        }

        const sal_uInt32 nNewCount(aNewPrimitives.size());

        if(nNewCount)
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval(nNewCount);

            for(sal_uInt32 a(0L); a < nNewCount; a++)
            {
                aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(aNewPrimitives[a]);
            }

            return aRetval;
        }
        else
        {
            return drawinglayer::primitive2d::Primitive2DSequence();
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// primitive decomposition

void SdrTextObj::impDecomposePathTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrPathTextPrimitive2D& rSdrPathTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    drawinglayer::primitive2d::Primitive2DSequence aRetvalA;
    drawinglayer::primitive2d::Primitive2DSequence aRetvalB;

    // prepare outliner
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    rOutliner.SetUpdateMode(true);
    rOutliner.Clear();
    rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
    rOutliner.SetText(rSdrPathTextPrimitive.getOutlinerParaObject());

    // set visualizing page at Outliner; needed e.g. for PageNumberField decomposition
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    // now break up to text portions
    impTextBreakupHandler aConverter(rOutliner);
    const ::std::vector< impPathTextPortion > rPathTextPortions = aConverter.decompositionPathTextPrimitive();

    if(!rPathTextPortions.empty())
    {
        // get FormText and polygon values
        const drawinglayer::attribute::SdrFormTextAttribute& rFormTextAttribute = rSdrPathTextPrimitive.getSdrFormTextAttribute();
        const basegfx::B2DPolyPolygon& rPathPolyPolygon(rSdrPathTextPrimitive.getPathPolyPolygon());

        // get loop count
        sal_uInt32 nLoopCount(rPathPolyPolygon.count());

        if(static_cast<sal_uInt32>(rOutliner.GetParagraphCount()) < nLoopCount)
        {
            nLoopCount = rOutliner.GetParagraphCount();
        }

        if(nLoopCount)
        {
            // prepare common decomposition stuff
            std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aRegularDecomposition;
            std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aShadowDecomposition;
            impPolygonParagraphHandler aPolygonParagraphHandler(
                rFormTextAttribute,
                aRegularDecomposition,
                aShadowDecomposition);
            sal_uInt32 a;

            for(a = 0L; a < nLoopCount; a++)
            {
                // filter text portions for this paragraph
                ::std::vector< const impPathTextPortion* > aParagraphTextPortions;

                for(sal_uInt32 b(0L); b < rPathTextPortions.size(); b++)
                {
                    const impPathTextPortion& rCandidate = rPathTextPortions[b];

                    if(static_cast<sal_uInt32>(rCandidate.getParagraph()) == a)
                    {
                        aParagraphTextPortions.push_back(&rCandidate);
                    }
                }

                // handle data pair polygon/ParagraphTextPortions
                if(!aParagraphTextPortions.empty())
                {
                    aPolygonParagraphHandler.HandlePair(rPathPolyPolygon.getB2DPolygon(a), aParagraphTextPortions);
                }
            }

            const sal_uInt32 nShadowCount(aShadowDecomposition.size());
            const sal_uInt32 nRegularCount(aRegularDecomposition.size());

            if(nShadowCount)
            {
                // add shadow primitives to decomposition
                aRetvalA.realloc(nShadowCount);

                for(a = 0L; a < nShadowCount; a++)
                {
                    aRetvalA[a] = drawinglayer::primitive2d::Primitive2DReference(aShadowDecomposition[a]);
                }

                // if necessary, add shadow outlines
                if(rFormTextAttribute.getFormTextOutline()
                    && !rFormTextAttribute.getShadowOutline().isDefault())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence aOutlines(
                        impAddPathTextOutlines(
                            aShadowDecomposition,
                            rFormTextAttribute.getShadowOutline()));

                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aRetvalA, aOutlines);
                }
            }

            if(nRegularCount)
            {
                // add normal primitives to decomposition
                aRetvalB.realloc(nRegularCount);

                for(a = 0L; a < nRegularCount; a++)
                {
                    aRetvalB[a] = drawinglayer::primitive2d::Primitive2DReference(aRegularDecomposition[a]);
                }

                // if necessary, add outlines
                if(rFormTextAttribute.getFormTextOutline()
                    && !rFormTextAttribute.getOutline().isDefault())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence aOutlines(
                        impAddPathTextOutlines(
                            aRegularDecomposition,
                            rFormTextAttribute.getOutline()));

                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aRetvalB, aOutlines);
                }
            }
        }
    }

    // clean up outliner
    rOutliner.SetDrawPortionHdl(Link());
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);

    // concatenate all results
    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aRetvalA);
    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aRetvalB);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
