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

#include <editeng/StripPortionsHelper.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <editeng/escapementitem.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <editeng/smallcaps.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>

// anonymous Outline/EditEngine decompose helpers
namespace
{
rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D>
CheckFieldPrimitive(drawinglayer::primitive2d::BasePrimitive2D* pPrimitive,
                    const DrawPortionInfo& rInfo)
{
    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> xRet = pPrimitive;
    if (rInfo.mpFieldData)
    {
        // Support for FIELD_SEQ_BEGIN, FIELD_SEQ_END. If used, create a TextHierarchyFieldPrimitive2D
        // which holds the field type and, if applicable, the URL
        const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(rInfo.mpFieldData);
        const SvxPageField* pPageField = dynamic_cast<const SvxPageField*>(rInfo.mpFieldData);

        // embed current primitive to a sequence
        drawinglayer::primitive2d::Primitive2DContainer aSequence;

        if (pPrimitive)
        {
            aSequence.resize(1);
            aSequence[0] = drawinglayer::primitive2d::Primitive2DReference(pPrimitive);
        }

        if (pURLField)
        {
            // extended this to hold more of the contents of the original
            // SvxURLField since that stuff is still used in HitTest and e.g. Calc
            std::vector<std::pair<OUString, OUString>> meValues;
            meValues.emplace_back("URL", pURLField->GetURL());
            meValues.emplace_back("Representation", pURLField->GetRepresentation());
            meValues.emplace_back("TargetFrame", pURLField->GetTargetFrame());
            meValues.emplace_back("AltText", pURLField->GetName());
            meValues.emplace_back(
                "SvxURLFormat", OUString::number(static_cast<sal_uInt16>(pURLField->GetFormat())));
            xRet = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                std::move(aSequence), drawinglayer::primitive2d::FIELD_TYPE_URL, &meValues);
        }
        else if (pPageField)
        {
            xRet = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                std::move(aSequence), drawinglayer::primitive2d::FIELD_TYPE_PAGE);
        }
        else
        {
            xRet = new drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D(
                std::move(aSequence), drawinglayer::primitive2d::FIELD_TYPE_COMMON);
        }
    }

    return xRet;
}

rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D>
buildTextPortionPrimitive(const DrawPortionInfo& rInfo, const OUString& rText,
                          const drawinglayer::attribute::FontAttribute& rFontAttribute,
                          const std::vector<double>& rDXArray,
                          const basegfx::B2DHomMatrix& rNewTransform)
{
    ::std::vector<sal_Bool> aKashidaArray;

    if (!rInfo.mpKashidaArray.empty() && rInfo.mnTextLen)
    {
        aKashidaArray.reserve(rInfo.mnTextLen);

        for (sal_Int32 a = 0; a < rInfo.mnTextLen; a++)
        {
            aKashidaArray.push_back(rInfo.mpKashidaArray[a]);
        }
    }

    // create complex text primitive and append
    const Color aFontColor(rInfo.mrFont.GetColor());
    const basegfx::BColor aBFontColor(aFontColor.getBColor());

    const Color aTextFillColor(rInfo.mrFont.GetFillColor());

    // prepare wordLineMode (for underline and strikeout)
    // NOT for bullet texts. It is set (this may be an error by itself), but needs to be suppressed to hinder e.g. '1)'
    // to be split which would not look like the original
    const bool bWordLineMode(rInfo.mrFont.IsWordLineMode() && !rInfo.mbEndOfBullet);

    // prepare new primitive
    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pNewPrimitive;
    const bool bDecoratedIsNeeded(
        LINESTYLE_NONE != rInfo.mrFont.GetOverline()
        || LINESTYLE_NONE != rInfo.mrFont.GetUnderline()
        || STRIKEOUT_NONE != rInfo.mrFont.GetStrikeout()
        || FontEmphasisMark::NONE != (rInfo.mrFont.GetEmphasisMark() & FontEmphasisMark::Style)
        || FontRelief::NONE != rInfo.mrFont.GetRelief() || rInfo.mrFont.IsShadow()
        || bWordLineMode);

    if (bDecoratedIsNeeded)
    {
        // TextDecoratedPortionPrimitive2D needed, prepare some more data
        // get overline and underline color. If it's on automatic (0xffffffff) use FontColor instead
        const Color aUnderlineColor(rInfo.maTextLineColor);
        const basegfx::BColor aBUnderlineColor(
            (aUnderlineColor == COL_AUTO) ? aBFontColor : aUnderlineColor.getBColor());
        const Color aOverlineColor(rInfo.maOverlineColor);
        const basegfx::BColor aBOverlineColor(
            (aOverlineColor == COL_AUTO) ? aBFontColor : aOverlineColor.getBColor());

        // prepare overline and underline data
        const drawinglayer::primitive2d::TextLine eFontOverline(
            drawinglayer::primitive2d::mapFontLineStyleToTextLine(rInfo.mrFont.GetOverline()));
        const drawinglayer::primitive2d::TextLine eFontLineStyle(
            drawinglayer::primitive2d::mapFontLineStyleToTextLine(rInfo.mrFont.GetUnderline()));

        // check UnderlineAbove
        const bool bUnderlineAbove(drawinglayer::primitive2d::TEXT_LINE_NONE != eFontLineStyle
                                   && rInfo.mrFont.IsUnderlineAbove());

        // prepare strikeout data
        const drawinglayer::primitive2d::TextStrikeout eTextStrikeout(
            drawinglayer::primitive2d::mapFontStrikeoutToTextStrikeout(
                rInfo.mrFont.GetStrikeout()));

        // prepare emphasis mark data
        drawinglayer::primitive2d::TextEmphasisMark eTextEmphasisMark(
            drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_NONE);

        switch (rInfo.mrFont.GetEmphasisMark() & FontEmphasisMark::Style)
        {
            case FontEmphasisMark::Dot:
                eTextEmphasisMark = drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_DOT;
                break;
            case FontEmphasisMark::Circle:
                eTextEmphasisMark = drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_CIRCLE;
                break;
            case FontEmphasisMark::Disc:
                eTextEmphasisMark = drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_DISC;
                break;
            case FontEmphasisMark::Accent:
                eTextEmphasisMark = drawinglayer::primitive2d::TEXT_FONT_EMPHASIS_MARK_ACCENT;
                break;
            default:
                break;
        }

        const bool bEmphasisMarkAbove(rInfo.mrFont.GetEmphasisMark() & FontEmphasisMark::PosAbove);
        const bool bEmphasisMarkBelow(rInfo.mrFont.GetEmphasisMark() & FontEmphasisMark::PosBelow);

        // prepare font relief data
        drawinglayer::primitive2d::TextRelief eTextRelief(
            drawinglayer::primitive2d::TEXT_RELIEF_NONE);

        switch (rInfo.mrFont.GetRelief())
        {
            case FontRelief::Embossed:
                eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_EMBOSSED;
                break;
            case FontRelief::Engraved:
                eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_ENGRAVED;
                break;
            default:
                break; // RELIEF_NONE, FontRelief_FORCE_EQUAL_SIZE
        }

        // prepare shadow/outline data
        const bool bShadow(rInfo.mrFont.IsShadow());

        // TextDecoratedPortionPrimitive2D is needed, create one
        pNewPrimitive = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(

            // attributes for TextSimplePortionPrimitive2D
            rNewTransform, rText, rInfo.mnTextStart, rInfo.mnTextLen, std::vector(rDXArray),
            std::move(aKashidaArray), rFontAttribute,
            rInfo.mpLocale ? *rInfo.mpLocale : css::lang::Locale(), aBFontColor, aTextFillColor,

            // attributes for TextDecoratedPortionPrimitive2D
            aBOverlineColor, aBUnderlineColor, eFontOverline, eFontLineStyle, bUnderlineAbove,
            eTextStrikeout, bWordLineMode, eTextEmphasisMark, bEmphasisMarkAbove,
            bEmphasisMarkBelow, eTextRelief, bShadow);
    }
    else
    {
        // TextSimplePortionPrimitive2D is enough
        pNewPrimitive = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
            rNewTransform, rText, rInfo.mnTextStart, rInfo.mnTextLen, std::vector(rDXArray),
            std::move(aKashidaArray), rFontAttribute,
            rInfo.mpLocale ? *rInfo.mpLocale : css::lang::Locale(), aBFontColor, aTextFillColor);
    }

    return pNewPrimitive;
}

class DoCapitalsDrawPortionInfo : public SvxDoCapitals
{
private:
    drawinglayer::primitive2d::Primitive2DContainer& mrTarget;
    const basegfx::B2DHomMatrix& mrNewTransformA;
    const basegfx::B2DHomMatrix& mrNewTransformB;
    const DrawPortionInfo& m_rInfo;
    SvxFont m_aFont;

public:
    DoCapitalsDrawPortionInfo(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                              const basegfx::B2DHomMatrix& rNewTransformA,
                              const basegfx::B2DHomMatrix& rNewTransformB,
                              const DrawPortionInfo& rInfo)
        : SvxDoCapitals(rInfo.maText, rInfo.mnTextStart, rInfo.mnTextLen)
        , mrTarget(rTarget)
        , mrNewTransformA(rNewTransformA)
        , mrNewTransformB(rNewTransformB)
        , m_rInfo(rInfo)
        , m_aFont(rInfo.mrFont)
    {
        assert(!m_rInfo.mpDXArray.empty());

        /* turn all these off as they are handled outside subportions for the whole portion */
        m_aFont.SetTransparent(false);
        m_aFont.SetUnderline(LINESTYLE_NONE);
        m_aFont.SetOverline(LINESTYLE_NONE);
        m_aFont.SetStrikeout(STRIKEOUT_NONE);

        m_aFont.SetCaseMap(SvxCaseMap::NotMapped); /* otherwise this would call itself */
    }
    virtual void Do(const OUString& rSpanTxt, const sal_Int32 nSpanIdx, const sal_Int32 nSpanLen,
                    const bool bUpper) override;
};

void CreateTextPortionPrimitivesFromDrawPortionInfo(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB,
    const DrawPortionInfo& rInfo)
{
    if (rInfo.maText.isEmpty() || !rInfo.mnTextLen)
        return;

    basegfx::B2DVector aFontScaling;
    drawinglayer::attribute::FontAttribute aFontAttribute(
        drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontScaling, rInfo.mrFont,
                                                               rInfo.IsRTL(), false));
    basegfx::B2DHomMatrix aNewTransform;

    // add font scale to new transform
    aNewTransform.scale(aFontScaling.getX(), aFontScaling.getY());

    // look for proportional font scaling, if necessary, scale accordingly
    sal_Int8 nPropr(rInfo.mrFont.GetPropr());
    const double fPropFontFactor(nPropr / 100.0);
    if (100 != nPropr)
        aNewTransform.scale(fPropFontFactor, fPropFontFactor);

    // apply font rotate
    if (rInfo.mrFont.GetOrientation())
    {
        aNewTransform.rotate(-toRadians(rInfo.mrFont.GetOrientation()));
    }

    // look for escapement, if necessary, translate accordingly
    if (rInfo.mrFont.GetEscapement())
    {
        sal_Int16 nEsc(rInfo.mrFont.GetEscapement());

        if (DFLT_ESC_AUTO_SUPER == nEsc)
        {
            nEsc = .8 * (100 - nPropr);
            assert(nEsc == DFLT_ESC_SUPER
                   && "I'm sure this formula needs to be changed, but how to confirm that???");
            nEsc = DFLT_ESC_SUPER;
        }
        else if (DFLT_ESC_AUTO_SUB == nEsc)
        {
            nEsc = .2 * -(100 - nPropr);
            assert(nEsc == -20
                   && "I'm sure this formula needs to be changed, but how to confirm that???");
            nEsc = -20;
        }

        if (nEsc > MAX_ESC_POS)
        {
            nEsc = MAX_ESC_POS;
        }
        else if (nEsc < -MAX_ESC_POS)
        {
            nEsc = -MAX_ESC_POS;
        }

        const double fEscapement(nEsc / -100.0);
        aNewTransform.translate(0.0, fEscapement * aFontScaling.getY());
    }

    // apply transformA
    aNewTransform *= rNewTransformA;

    // apply local offset
    aNewTransform.translate(rInfo.mrStartPos.X(), rInfo.mrStartPos.Y());

    // also apply embedding object's transform
    aNewTransform *= rNewTransformB;

    // prepare DXArray content. To make it independent from font size (and such from
    // the text transformation), scale it to unit coordinates
    ::std::vector<double> aDXArray;

    if (!rInfo.mpDXArray.empty())
    {
        aDXArray.reserve(rInfo.mnTextLen);
        for (sal_Int32 a = 0; a < rInfo.mnTextLen; a++)
        {
            aDXArray.push_back(rInfo.mpDXArray[a]);
        }
    }

    OUString caseMappedText = rInfo.mrFont.CalcCaseMap(rInfo.maText);
    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pNewPrimitive(
        buildTextPortionPrimitive(rInfo, caseMappedText, aFontAttribute, aDXArray, aNewTransform));

    bool bSmallCaps = rInfo.mrFont.IsCapital();
    if (bSmallCaps && rInfo.mpDXArray.empty())
    {
        SAL_WARN("svx", "SmallCaps requested with DXArray, abandoning");
        bSmallCaps = false;
    }
    if (bSmallCaps)
    {
        // rerun with each sub-portion
        DoCapitalsDrawPortionInfo aDoDrawPortionInfo(rTarget, rNewTransformA, rNewTransformB,
                                                     rInfo);
        rInfo.mrFont.DoOnCapitals(aDoDrawPortionInfo);

        // transfer collected primitives from rTarget to a new container
        drawinglayer::primitive2d::Primitive2DContainer aContainer = std::move(rTarget);

        // Take any decoration for the whole formatted portion and keep it to get continuous over/under/strike-through
        if (pNewPrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)
        {
            const drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D* pTCPP
                = static_cast<const drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D*>(
                    pNewPrimitive.get());

            if (pTCPP->getWordLineMode()) // single word mode: 'Individual words' in UI
            {
                // Split to single word primitives using TextBreakupHelper
                drawinglayer::primitive2d::TextBreakupHelper aTextBreakupHelper(*pTCPP);
                drawinglayer::primitive2d::Primitive2DContainer aBroken(
                    aTextBreakupHelper.extractResult(drawinglayer::primitive2d::BreakupUnit::Word));
                for (auto& rPortion : aBroken)
                {
                    assert(rPortion->getPrimitive2DID()
                               == PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D
                           && "TextBreakupHelper generates same output primitive type as input");

                    const drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D* pPortion
                        = static_cast<
                            const drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D*>(
                            rPortion.get());

                    // create and add decoration
                    const drawinglayer::primitive2d::Primitive2DContainer&
                        rDecorationGeometryContent(pPortion->getOrCreateDecorationGeometryContent(
                            pPortion->getTextTransform(), caseMappedText,
                            pPortion->getTextPosition(), pPortion->getTextLength(),
                            pPortion->getDXArray()));

                    aContainer.insert(aContainer.end(), rDecorationGeometryContent.begin(),
                                      rDecorationGeometryContent.end());
                }
            }
            else
            {
                // create and add decoration
                const drawinglayer::primitive2d::Primitive2DContainer& rDecorationGeometryContent(
                    pTCPP->getOrCreateDecorationGeometryContent(pTCPP->getTextTransform(),
                                                                caseMappedText, rInfo.mnTextStart,
                                                                rInfo.mnTextLen, aDXArray));

                aContainer.insert(aContainer.end(), rDecorationGeometryContent.begin(),
                                  rDecorationGeometryContent.end());
            }
        }

        pNewPrimitive = new drawinglayer::primitive2d::GroupPrimitive2D(std::move(aContainer));
    }

    const Color aFontColor(rInfo.mrFont.GetColor());
    if (aFontColor.IsTransparent())
    {
        // Handle semi-transparent text for both the decorated and simple case here.
        pNewPrimitive = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer{ pNewPrimitive },
            (255 - aFontColor.GetAlpha()) / 255.0);
    }

    if (rInfo.mbEndOfBullet)
    {
        // embed in TextHierarchyBulletPrimitive2D
        drawinglayer::primitive2d::Primitive2DContainer aNewSequence{ pNewPrimitive };
        pNewPrimitive = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(
            std::move(aNewSequence));
    }

    if (rInfo.mpFieldData)
    {
        pNewPrimitive = CheckFieldPrimitive(pNewPrimitive.get(), rInfo);
    }

    rTarget.push_back(pNewPrimitive);

    // support for WrongSpellVector. Create WrongSpellPrimitives as needed
    if (!rInfo.mpWrongSpellVector || aDXArray.empty())
        return;

    const sal_Int32 nSize(rInfo.mpWrongSpellVector->size());
    const sal_Int32 nDXCount(aDXArray.size());
    const basegfx::BColor aSpellColor(1.0, 0.0, 0.0); // red, hard coded

    for (sal_Int32 a(0); a < nSize; a++)
    {
        const EEngineData::WrongSpellClass& rCandidate = (*rInfo.mpWrongSpellVector)[a];

        if (rCandidate.nStart >= rInfo.mnTextStart && rCandidate.nEnd >= rInfo.mnTextStart
            && rCandidate.nEnd > rCandidate.nStart)
        {
            const sal_Int32 nStart(rCandidate.nStart - rInfo.mnTextStart);
            const sal_Int32 nEnd(rCandidate.nEnd - rInfo.mnTextStart);
            double fStart(0.0);
            double fEnd(0.0);

            if (nStart > 0 && nStart - 1 < nDXCount)
            {
                fStart = aDXArray[nStart - 1];
            }

            if (nEnd > 0 && nEnd - 1 < nDXCount)
            {
                fEnd = aDXArray[nEnd - 1];
            }

            if (!basegfx::fTools::equal(fStart, fEnd))
            {
                if (rInfo.IsRTL())
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
                const double fFontScaleX(aFontScaling.getX() * fPropFontFactor);

                if (!basegfx::fTools::equal(fFontScaleX, 1.0)
                    && !basegfx::fTools::equalZero(fFontScaleX))
                {
                    fStart /= fFontScaleX;
                    fEnd /= fFontScaleX;
                }

                rTarget.push_back(new drawinglayer::primitive2d::WrongSpellPrimitive2D(
                    aNewTransform, fStart, fEnd, aSpellColor));
            }
        }
    }
}

void DoCapitalsDrawPortionInfo::Do(const OUString& rSpanTxt, const sal_Int32 nSpanIdx,
                                   const sal_Int32 nSpanLen, const bool bUpper)
{
    sal_uInt8 nProp(0);
    if (!bUpper)
    {
        nProp = m_aFont.GetPropr();
        m_aFont.SetProprRel(SMALL_CAPS_PERCENTAGE);
    }

    sal_Int32 nStartOffset = nSpanIdx - nIdx;
    double nStartX = nStartOffset ? m_rInfo.mpDXArray[nStartOffset - 1] : 0;

    Point aStartPos(m_rInfo.mrStartPos.X() + nStartX, m_rInfo.mrStartPos.Y());

    KernArray aDXArray;
    aDXArray.resize(nSpanLen);
    for (sal_Int32 i = 0; i < nSpanLen; ++i)
        aDXArray[i] = m_rInfo.mpDXArray[nStartOffset + i] - nStartX;

    auto aKashidaArray
        = !m_rInfo.mpKashidaArray.empty()
              ? std::span<const sal_Bool>(m_rInfo.mpKashidaArray.data() + nStartOffset, nSpanLen)
              : std::span<const sal_Bool>();

    DrawPortionInfo aInfo(
        aStartPos, rSpanTxt, nSpanIdx, nSpanLen, aDXArray, aKashidaArray, m_aFont, m_rInfo.mnPara,
        m_rInfo.mnBiDiLevel, nullptr, /* no spelling in subportion, handled outside */
        nullptr, /* no field in subportion, handled outside */
        false, false, false, m_rInfo.mpLocale, m_rInfo.maOverlineColor, m_rInfo.maTextLineColor);

    CreateTextPortionPrimitivesFromDrawPortionInfo(mrTarget, mrNewTransformA, mrNewTransformB,
                                                   aInfo);

    if (!bUpper)
        m_aFont.SetPropr(nProp);
}

void CreateDrawBulletPrimitivesFromDrawBulletInfo(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DHomMatrix& rNewTransformA, const basegfx::B2DHomMatrix& rNewTransformB,
    const DrawBulletInfo& rInfo)
{
    basegfx::B2DHomMatrix aNewTransform;

    // add size to new transform
    aNewTransform.scale(rInfo.maBulletSize.getWidth(), rInfo.maBulletSize.getHeight());

    // apply transformA
    aNewTransform *= rNewTransformA;

    // apply local offset
    aNewTransform.translate(rInfo.maBulletPosition.X(), rInfo.maBulletPosition.Y());

    // also apply embedding object's transform
    aNewTransform *= rNewTransformB;

    // prepare empty GraphicAttr
    const GraphicAttr aGraphicAttr;

    // create GraphicPrimitive2D
    const drawinglayer::primitive2d::Primitive2DReference aNewReference(
        new drawinglayer::primitive2d::GraphicPrimitive2D(
            aNewTransform, rInfo.maBulletGraphicObject, aGraphicAttr));

    // embed in TextHierarchyBulletPrimitive2D
    drawinglayer::primitive2d::Primitive2DContainer aNewSequence{ aNewReference };
    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pNewPrimitive
        = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(std::move(aNewSequence));

    // add to output
    rTarget.push_back(pNewPrimitive);
}
} // end of anonymous namespace

void TextHierarchyBreakup::flushTextPortionPrimitivesToLinePrimitives()
{
    // only create a line primitive when we had content; there is no need for
    // empty line primitives (contrary to paragraphs, see below).
    if (!maTextPortionPrimitives.empty())
    {
        maLinePrimitives.push_back(new drawinglayer::primitive2d::TextHierarchyLinePrimitive2D(
            std::move(maTextPortionPrimitives)));
    }
}

sal_Int16 TextHierarchyBreakup::getOutlineLevelFromParagraph(sal_Int32 /*nPara*/) const
{
    return -1;
}

sal_Int32 TextHierarchyBreakup::getParagraphCount() const { return 0; }

void TextHierarchyBreakup::flushLinePrimitivesToParagraphPrimitives(sal_Int32 nPara)
{
    // ALWAYS create a paragraph primitive, even when no content was added. This is done to
    // have the correct paragraph count even with empty paragraphs. Those paragraphs will
    // have an empty sub-PrimitiveSequence.
    maParagraphPrimitives.push_back(
        new drawinglayer::primitive2d::TextHierarchyParagraphPrimitive2D(
            std::move(maLinePrimitives), getOutlineLevelFromParagraph(nPara)));
}

void TextHierarchyBreakup::processDrawPortionInfo(const DrawPortionInfo& rDrawPortionInfo)
{
    CreateTextPortionPrimitivesFromDrawPortionInfo(maTextPortionPrimitives, maNewTransformA,
                                                   maNewTransformB, rDrawPortionInfo);

    if (rDrawPortionInfo.mbEndOfLine || rDrawPortionInfo.mbEndOfParagraph)
    {
        flushTextPortionPrimitivesToLinePrimitives();
    }

    if (rDrawPortionInfo.mbEndOfParagraph)
    {
        flushLinePrimitivesToParagraphPrimitives(rDrawPortionInfo.mnPara);
    }
}

void TextHierarchyBreakup::processDrawBulletInfo(const DrawBulletInfo& rDrawBulletInfo)
{
    CreateDrawBulletPrimitivesFromDrawBulletInfo(maTextPortionPrimitives, maNewTransformA,
                                                 maNewTransformB, rDrawBulletInfo);
    basegfx::B2DHomMatrix aNewTransform;

    // add size to new transform
    aNewTransform.scale(rDrawBulletInfo.maBulletSize.getWidth(),
                        rDrawBulletInfo.maBulletSize.getHeight());

    // apply transformA
    aNewTransform *= maNewTransformA;

    // apply local offset
    aNewTransform.translate(rDrawBulletInfo.maBulletPosition.X(),
                            rDrawBulletInfo.maBulletPosition.Y());

    // also apply embedding object's transform
    aNewTransform *= maNewTransformB;

    // prepare empty GraphicAttr
    const GraphicAttr aGraphicAttr;

    // create GraphicPrimitive2D
    const drawinglayer::primitive2d::Primitive2DReference aNewReference(
        new drawinglayer::primitive2d::GraphicPrimitive2D(
            aNewTransform, rDrawBulletInfo.maBulletGraphicObject, aGraphicAttr));

    // embed in TextHierarchyBulletPrimitive2D
    drawinglayer::primitive2d::Primitive2DContainer aNewSequence{ aNewReference };
    rtl::Reference<drawinglayer::primitive2d::BasePrimitive2D> pNewPrimitive
        = new drawinglayer::primitive2d::TextHierarchyBulletPrimitive2D(std::move(aNewSequence));

    // add to output
    maTextPortionPrimitives.push_back(pNewPrimitive);
}

TextHierarchyBreakup::TextHierarchyBreakup()
    : maTextPortionPrimitives()
    , maLinePrimitives()
    , maParagraphPrimitives()
    , maNewTransformA()
    , maNewTransformB()
{
}

TextHierarchyBreakup::TextHierarchyBreakup(const basegfx::B2DHomMatrix& rNewTransformA,
                                           const basegfx::B2DHomMatrix& rNewTransformB)
    : maTextPortionPrimitives()
    , maLinePrimitives()
    , maParagraphPrimitives()
    , maNewTransformA(rNewTransformA)
    , maNewTransformB(rNewTransformB)
{
}

const drawinglayer::primitive2d::Primitive2DContainer&
TextHierarchyBreakup::getTextPortionPrimitives()
{
    if (!maTextPortionPrimitives.empty())
    {
        // collect non-closed lines
        flushTextPortionPrimitivesToLinePrimitives();
    }

    if (!maLinePrimitives.empty())
    {
        // collect non-closed paragraphs
        flushLinePrimitivesToParagraphPrimitives(getParagraphCount() - 1);
    }

    return maParagraphPrimitives;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
