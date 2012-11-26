/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <editeng/outliner.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <editeng/escpitem.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <editeng/flditem.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// helpers

namespace
{
    drawinglayer::primitive2d::Primitive2DSequence impConvertVectorToPrimitive2DSequence(
        const std::vector< drawinglayer::primitive2d::BasePrimitive2D* >& rPrimitiveVector)
    {
        const sal_Int32 nCount(rPrimitiveVector.size());
        drawinglayer::primitive2d::Primitive2DSequence aRetval(nCount);

        for(sal_Int32 a(0); a < nCount; a++)
        {
            aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(rPrimitiveVector[a]);
        }

        return aRetval;
    }

    class impTextBreakupHandler
    {
    private:
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maTextPortionPrimitives;
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maLinePrimitives;
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* >  maParagraphPrimitives;

        Outliner&                                                   mrOutliner;

        // the visible area for contour text decomposition
        const basegfx::B2DVector*                                   mpContourScale;

        // #SJ# ClipRange for BlockText decomposition; only text portions completely
        // inside are to be accepted, so this is different from geometric clipping
        // (which would allow e.g. upper parts of portions to remain). Only used for
        // BlockText (see there)
        const basegfx::B2DRange*                                    mpClipRange;

        DECL_LINK(decomposeTextPrimitive, DrawPortionInfo* );
        DECL_LINK(decomposeBulletPrimitive, DrawBulletInfo* );

        bool impIsUnderlineAbove(const Font& rFont) const;
        void impCreateTextPortionPrimitive(const DrawPortionInfo& rInfo);
        drawinglayer::primitive2d::BasePrimitive2D* impCheckFieldPrimitive(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive, const DrawPortionInfo& rInfo) const;
        void impFlushTextPortionPrimitivesToLinePrimitives();
        void impFlushLinePrimitivesToParagraphPrimitives();
        void impHandleDrawPortionInfo(const DrawPortionInfo& rInfo);
        void impHandleDrawBulletInfo(const DrawBulletInfo& rInfo);

    public:
        impTextBreakupHandler(Outliner& rOutliner)
        :   maTextPortionPrimitives(),
            maLinePrimitives(),
            maParagraphPrimitives(),
            mrOutliner(rOutliner),
            mpContourScale(0),
            mpClipRange(0)
        {
        }

        void decompose(const basegfx::B2DVector* pContourScale, const basegfx::B2DRange* pClipRange)
        {
            mpContourScale = pContourScale;
            mpClipRange = pClipRange;

            const Link aOldDrawPortionHdl(mrOutliner.GetDrawPortionHdl());
            const Link aOldDrawBulletHdl(mrOutliner.GetDrawBulletHdl());

            mrOutliner.SetDrawPortionHdl(LINK(this, impTextBreakupHandler, decomposeTextPrimitive));
            mrOutliner.SetDrawBulletHdl(LINK(this, impTextBreakupHandler, decomposeBulletPrimitive));

            mrOutliner.StripPortions();

            mrOutliner.SetDrawPortionHdl(aOldDrawPortionHdl);
            mrOutliner.SetDrawBulletHdl(aOldDrawBulletHdl);
        }

        drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequence();
    };

    bool impTextBreakupHandler::impIsUnderlineAbove(const Font& rFont) const
    {
        if(!rFont.IsVertical())
        {
            return false;
        }

        if((LANGUAGE_JAPANESE == rFont.GetLanguage()) || (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()))
        {
            // the underline is right for Japanese only
            return true;
        }

        return false;
    }

    void impTextBreakupHandler::impCreateTextPortionPrimitive(const DrawPortionInfo& rInfo)
    {
        if(rInfo.mrText.Len() && rInfo.mnTextLen)
        {
            basegfx::B2DVector aFontScaling;
            basegfx::B2DHomMatrix aNewTransform;
            drawinglayer::attribute::FontAttribute aFontAttribute(
                drawinglayer::primitive2d::getFontAttributeFromVclFont(
                    aFontScaling,
                    rInfo.mrFont,
                    rInfo.IsRTL(),
                    false));

            // add font scale to new transform
            aNewTransform.scale(aFontScaling);

            // look for proportional font scaling, evtl scale accordingly
            if(100 != rInfo.mrFont.GetPropr())
            {
                const double fFactor(rInfo.mrFont.GetPropr() / 100.0);
                aNewTransform.scale(fFactor, fFactor);
            }

            // apply font rotate
            if(rInfo.mrFont.GetOrientation())
            {
                aNewTransform.rotate(-rInfo.mrFont.GetOrientation() * F_PI1800);
            }

            // look for escapement, evtl translate accordingly
            if(rInfo.mrFont.GetEscapement())
            {
                sal_Int16 nEsc(rInfo.mrFont.GetEscapement());

                if(DFLT_ESC_AUTO_SUPER == nEsc)
                {
                    nEsc = 33;
                }
                else if(DFLT_ESC_AUTO_SUB == nEsc)
                {
                    nEsc = -20;
                }

                if(nEsc > 100)
                {
                    nEsc = 100;
                }
                else if(nEsc < -100)
                {
                    nEsc = -100;
                }

                const double fEscapement(nEsc / -100.0);
                aNewTransform.translate(0.0, fEscapement * aFontScaling.getY());
            }

            // apply local offset
            aNewTransform.translate(rInfo.mrStartPos.X(), rInfo.mrStartPos.Y());

            // prepare DXArray content. To make it independent from font size (and such from
            // the text transformation), scale it to unit coordinates
            ::std::vector< double > aDXArray;
            static bool bDisableTextArray(false);

            if(!bDisableTextArray && rInfo.mpDXArray && rInfo.mnTextLen)
            {
                aDXArray.reserve(rInfo.mnTextLen);

                for(xub_StrLen a(0); a < rInfo.mnTextLen; a++)
                {
                    aDXArray.push_back((double)rInfo.mpDXArray[a]);
                }
            }

            // create complex text primitive and append
            const Color aFontColor(rInfo.mrFont.GetColor());
            const basegfx::BColor aBFontColor(aFontColor.getBColor());

            // prepare wordLineMode (for underline and strikeout)
            // NOT for bullet texts. It is set (this may be an error by itself), but needs to be suppressed to hinder e.g. '1)'
            // to be splitted which would not look like the original
            const bool bWordLineMode(rInfo.mrFont.IsWordLineMode() && !rInfo.mbEndOfBullet);

            // prepare new primitive
            drawinglayer::primitive2d::BasePrimitive2D* pNewPrimitive = 0;
            const bool bDecoratedIsNeeded(
                   UNDERLINE_NONE != rInfo.mrFont.GetOverline()
                || UNDERLINE_NONE != rInfo.mrFont.GetUnderline()
                || STRIKEOUT_NONE != rInfo.mrFont.GetStrikeout()
                || EMPHASISMARK_NONE != (rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                || RELIEF_NONE != rInfo.mrFont.GetRelief()
                || rInfo.mrFont.IsShadow()
                || bWordLineMode);

            if(bDecoratedIsNeeded)
            {
                // TextDecoratedPortionPrimitive2D needed, prepare some more data
                // get overline and underline color. If it's on automatic (0xffffffff) use FontColor instead
                const Color aUnderlineColor(rInfo.maTextLineColor);
                const basegfx::BColor aBUnderlineColor((0xffffffff == aUnderlineColor.GetColor()) ? aBFontColor : aUnderlineColor.getBColor());
                const Color aOverlineColor(rInfo.maOverlineColor);
                const basegfx::BColor aBOverlineColor((0xffffffff == aOverlineColor.GetColor()) ? aBFontColor : aOverlineColor.getBColor());

                // prepare overline and underline data
                const drawinglayer::primitive2d::TextLine eFontOverline(
                    drawinglayer::primitive2d::mapFontUnderlineToTextLine(rInfo.mrFont.GetOverline()));
                const drawinglayer::primitive2d::TextLine eFontUnderline(
                    drawinglayer::primitive2d::mapFontUnderlineToTextLine(rInfo.mrFont.GetUnderline()));

                // check UndelineAbove
                const bool bUnderlineAbove(
                    drawinglayer::primitive2d::TEXT_LINE_NONE != eFontUnderline && impIsUnderlineAbove(rInfo.mrFont));

                // prepare strikeout data
                const drawinglayer::primitive2d::TextStrikeout eTextStrikeout(
                    drawinglayer::primitive2d::mapFontStrikeoutToTextStrikeout(rInfo.mrFont.GetStrikeout()));

                // prepare emphasis mark data
                drawinglayer::primitive2d::TextEmphasisMark eTextEmphasisMark(drawinglayer::primitive2d::TEXT_EMPHASISMARK_NONE);

                switch(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                {
                    case EMPHASISMARK_DOT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DOT; break;
                    case EMPHASISMARK_CIRCLE : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_CIRCLE; break;
                    case EMPHASISMARK_DISC : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DISC; break;
                    case EMPHASISMARK_ACCENT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_ACCENT; break;
                }

                const bool bEmphasisMarkAbove(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_POS_ABOVE);
                const bool bEmphasisMarkBelow(rInfo.mrFont.GetEmphasisMark() & EMPHASISMARK_POS_BELOW);

                // prepare font relief data
                drawinglayer::primitive2d::TextRelief eTextRelief(drawinglayer::primitive2d::TEXT_RELIEF_NONE);

                switch(rInfo.mrFont.GetRelief())
                {
                    case RELIEF_EMBOSSED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_EMBOSSED; break;
                    case RELIEF_ENGRAVED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_ENGRAVED; break;
                    default : break; // RELIEF_NONE, FontRelief_FORCE_EQUAL_SIZE
                }

                // prepare shadow/outline data
                const bool bShadow(rInfo.mrFont.IsShadow());

                // TextDecoratedPortionPrimitive2D is needed, create one
                pNewPrimitive = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(

                    // attributes for TextSimplePortionPrimitive2D
                    aNewTransform,
                    rInfo.mrText,
                    rInfo.mnTextStart,
                    rInfo.mnTextLen,
                    aDXArray,
                    aFontAttribute,
                    rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale(),
                    aBFontColor,

                    // attributes for TextDecoratedPortionPrimitive2D
                    aBOverlineColor,
                    aBUnderlineColor,
                    eFontOverline,
                    eFontUnderline,
                    bUnderlineAbove,
                    eTextStrikeout,
                    bWordLineMode,
                    eTextEmphasisMark,
                    bEmphasisMarkAbove,
                    bEmphasisMarkBelow,
                    eTextRelief,
                    bShadow);
            }
            else
            {
                // TextSimplePortionPrimitive2D is enough
                pNewPrimitive = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                    aNewTransform,
                    rInfo.mrText,
                    rInfo.mnTextStart,
                    rInfo.mnTextLen,
                    aDXArray,
                    aFontAttribute,
                    rInfo.mpLocale ? *rInfo.mpLocale : ::com::sun::star::lang::Locale(),
                    aBFontColor);
            }

            if(rInfo.mbEndOfBullet)
            {
                // embed in TextHierarchyBulletPrimitive2D
                const drawinglayer::primitive2d::Primitive2DReference aNewReference(pNewPrimitive);
                const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(&aNewReference, 1);
                pNewPrimitive = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(aNewSequence);
            }

            if(rInfo.mpFieldData)
            {
                pNewPrimitive = impCheckFieldPrimitive(pNewPrimitive, rInfo);
            }

            maTextPortionPrimitives.push_back(pNewPrimitive);

            // support for WrongSpellVector. Create WrongSpellPrimitives as needed
            if(rInfo.mpWrongSpellVector && !aDXArray.empty())
            {
                const sal_uInt32 nSize(rInfo.mpWrongSpellVector->size());
                const sal_uInt32 nDXCount(aDXArray.size());
                const basegfx::BColor aSpellColor(1.0, 0.0, 0.0); // red, hard coded

                for(sal_uInt32 a(0); a < nSize; a++)
                {
                    const EEngineData::WrongSpellClass& rCandidate = (*rInfo.mpWrongSpellVector)[a];

                    if(rCandidate.nStart >= rInfo.mnTextStart && rCandidate.nEnd >= rInfo.mnTextStart && rCandidate.nEnd > rCandidate.nStart)
                    {
                        const sal_uInt32 nStart(rCandidate.nStart - rInfo.mnTextStart);
                        const sal_uInt32 nEnd(rCandidate.nEnd - rInfo.mnTextStart);
                        double fStart(0.0);
                        double fEnd(0.0);

                        if(nStart > 0 && nStart - 1 < nDXCount)
                        {
                            fStart = aDXArray[nStart - 1];
                        }

                        if(nEnd > 0 && nEnd - 1 < nDXCount)
                        {
                            fEnd = aDXArray[nEnd - 1];
                        }

                        if(!basegfx::fTools::equal(fStart, fEnd))
                        {
                            if(rInfo.IsRTL())
                            {
                                // #i98523#
                                // When the portion is RTL, mirror the redlining using the
                                // full portion width
                                const double fTextWidth(aDXArray[aDXArray.size() - 1]);

                                fStart = fTextWidth - fStart;
                                fEnd = fTextWidth - fEnd;
                            }

                            // need to take FontScaling out of values; it's already part of
                            // aNewTransform and would be double applied
                            const double fFontScaleX(aFontScaling.getX());

                            if(!basegfx::fTools::equal(fFontScaleX, 1.0)
                                && !basegfx::fTools::equalZero(fFontScaleX))
                            {
                                fStart /= fFontScaleX;
                                fEnd /= fFontScaleX;
                            }

                            maTextPortionPrimitives.push_back(new drawinglayer::primitive2d::WrongSpellPrimitive2D(
                                aNewTransform,
                                fStart,
                                fEnd,
                                aSpellColor));
                        }
                    }
                }
            }
        }
    }

    drawinglayer::primitive2d::BasePrimitive2D* impTextBreakupHandler::impCheckFieldPrimitive(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive, const DrawPortionInfo& rInfo) const
    {
        if(rInfo.mpFieldData)
        {
            // Support for FIELD_SEQ_BEGIN, FIELD_SEQ_END. If used, create a TextHierarchyFieldPrimitive2D
            // which holds the field type and evtl. the URL
            const SvxURLField* pURLField = dynamic_cast< const SvxURLField* >(rInfo.mpFieldData);
            const SvxPageField* pPageField = dynamic_cast< const SvxPageField* >(rInfo.mpFieldData);

            // embed current primitive to a sequence
            drawinglayer::primitive2d::Primitive2DSequence aSequence;

            if(pPrimitive)
            {
                aSequence.realloc(1);
                aSequence[0] = drawinglayer::primitive2d::Primitive2DReference(pPrimitive);
            }

            if(pURLField)
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                    aSequence,
                    drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D::FIELD_TYPE_URL,
                    pURLField->GetURL(),
                    pURLField->GetTargetFrame());
            }
            else if(pPageField)
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                    aSequence,
                    drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D::FIELD_TYPE_PAGE);
            }
            else
            {
                pPrimitive = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                    aSequence,
                    drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D::FIELD_TYPE_COMMON);
            }
        }

        return pPrimitive;
    }

    void impTextBreakupHandler::impFlushTextPortionPrimitivesToLinePrimitives()
    {
        // only create a line primitive when we had content; there is no need for
        // empty line primitives (contrary to paragraphs, see below).
        if(maTextPortionPrimitives.size())
        {
            drawinglayer::primitive2d::Primitive2DSequence aLineSequence(impConvertVectorToPrimitive2DSequence(maTextPortionPrimitives));
            maTextPortionPrimitives.clear();
            maLinePrimitives.push_back(new drawinglayer::primitive2d::TextHierarchyLinePrimitive2D(aLineSequence));
        }
    }

    void impTextBreakupHandler::impFlushLinePrimitivesToParagraphPrimitives()
    {
        // ALWAYS create a paragraph primitive, even when no content was added. This is done to
        // have the correct paragraph count even with empty paragraphs. Those paragraphs will
        // have an empty sub-PrimitiveSequence.
        drawinglayer::primitive2d::Primitive2DSequence aParagraphSequence(impConvertVectorToPrimitive2DSequence(maLinePrimitives));
        maLinePrimitives.clear();
        maParagraphPrimitives.push_back(new drawinglayer::primitive2d::TextHierarchyParagraphPrimitive2D(aParagraphSequence));
    }

    void impTextBreakupHandler::impHandleDrawPortionInfo(const DrawPortionInfo& rInfo)
    {
        impCreateTextPortionPrimitive(rInfo);

        if(rInfo.mbEndOfLine || rInfo.mbEndOfParagraph)
        {
            impFlushTextPortionPrimitivesToLinePrimitives();
        }

        if(rInfo.mbEndOfParagraph)
        {
            impFlushLinePrimitivesToParagraphPrimitives();
        }
    }

    void impTextBreakupHandler::impHandleDrawBulletInfo(const DrawBulletInfo& rInfo)
    {
        basegfx::B2DHomMatrix aNewTransform;

        // add size to new transform
        aNewTransform.scale(rInfo.maBulletSize.getWidth(), rInfo.maBulletSize.getHeight());

        // apply local offset
        aNewTransform.translate(rInfo.maBulletPosition.X(), rInfo.maBulletPosition.Y());

        // prepare empty GraphicAttr
        const GraphicAttr aGraphicAttr;

        // create GraphicPrimitive2D
        const drawinglayer::primitive2d::Primitive2DReference aNewReference(new drawinglayer::primitive2d::GraphicPrimitive2D(
            aNewTransform,
            rInfo.maBulletGraphicObject,
            aGraphicAttr));

        // embed in TextHierarchyBulletPrimitive2D
        const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(&aNewReference, 1);
        drawinglayer::primitive2d::BasePrimitive2D* pNewPrimitive = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(aNewSequence);

        // add to output
        maTextPortionPrimitives.push_back(pNewPrimitive);
    }

    IMPL_LINK(impTextBreakupHandler, decomposeTextPrimitive, DrawPortionInfo*, pInfo)
    {
        if(pInfo)
        {
            // for contour text, ignore (clip away) all portions which are below
            // the visible area given by mpContourScale
            if(mpContourScale)
            {
                if((double)pInfo->mrStartPos.Y() >= mpContourScale->getY())
                {
                    return 0;
                }
            }
            // #SJ# Is clipping wanted? This is text clipping; only accept a portion
            // if it's completely in the range
            if(mpClipRange && !mpClipRange->isEmpty())
            {
                // Test start position first; this allows to not get the text range at
                // all if text is far outside
                const basegfx::B2DPoint aStartPosition(pInfo->mrStartPos.X(), pInfo->mrStartPos.Y());

                if(!mpClipRange->isInside(aStartPosition))
                {
                    return 0;
                }

                // Start position is inside. Get TextBoundRect and TopLeft next
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                aTextLayouterDevice.setFont(pInfo->mrFont);

                const basegfx::B2DRange aTextBoundRect(
                    aTextLayouterDevice.getTextBoundRect(
                        pInfo->mrText, pInfo->mnTextStart, pInfo->mnTextLen));
                const basegfx::B2DPoint aTopLeft(aTextBoundRect.getMinimum() + aStartPosition);

                if(!mpClipRange->isInside(aTopLeft))
                {
                    return 0;
                }

                // TopLeft is inside. Get BottomRight and check
                const basegfx::B2DPoint aBottomRight(aTextBoundRect.getMaximum() + aStartPosition);

                if(!mpClipRange->isInside(aBottomRight))
                {
                    return 0;
                }

                // all inside, clip was successful
            }

            // action
            impHandleDrawPortionInfo(*pInfo);
        }

        return 0;
    }

    IMPL_LINK(impTextBreakupHandler, decomposeBulletPrimitive, DrawBulletInfo*, pInfo)
    {
        if(pInfo)
        {
            impHandleDrawBulletInfo(*pInfo);
        }

        return 0;
    }

    drawinglayer::primitive2d::Primitive2DSequence impTextBreakupHandler::getPrimitive2DSequence()
    {
        if(maTextPortionPrimitives.size())
        {
            // collect non-closed lines
            impFlushTextPortionPrimitivesToLinePrimitives();
        }

        if(maLinePrimitives.size())
        {
            // collect non-closed paragraphs
            impFlushLinePrimitivesToParagraphPrimitives();
        }

        return impConvertVectorToPrimitive2DSequence(maParagraphPrimitives);
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

void Outliner::getPrimitive2DSequence(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const basegfx::B2DVector* pContourScale,
    const basegfx::B2DRange* pClipRange)
{
    // incarnate helper
    impTextBreakupHandler aConverter(*this);

    // use it
    aConverter.decompose(pContourScale, pClipRange);

    // append results
    rTarget = aConverter.getPrimitive2DSequence();
}

//////////////////////////////////////////////////////////////////////////////
// eof
