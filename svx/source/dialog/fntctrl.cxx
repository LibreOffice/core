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

#include <sfx2/dialoghelper.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/printer.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>

#include <vector>
#include <deque>
#include <optional>
#include <svtools/colorcfg.hxx>
#include <svtools/sampletext.hxx>

#include <svx/fntctrl.hxx>
#include <svx/svxids.hrc>

// Item set includes
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>

#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/langitem.hxx>

//TODO: remove this and calculate off the actual size of text, not
//an arbitrary number of characters
#define TEXT_WIDTH 80

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::com::sun::star::i18n::XBreakIterator;
using ::com::sun::star::i18n::BreakIterator;


// small helper functions to set fonts

namespace
{
void scaleFontWidth(vcl::Font& rFont, vcl::RenderContext const & rRenderContext,tools::Long& n100PercentFont)
{
    rFont.SetAverageFontWidth(0);
    n100PercentFont = rRenderContext.GetFontMetric(rFont).GetAverageFontWidth();
}

void initFont(vcl::Font& rFont)
{
    rFont.SetTransparent(true);
    rFont.SetAlignment(ALIGN_BASELINE);
}

void setFontSize(vcl::Font& rFont)
{
    Size aSize(rFont.GetFontSize());
    aSize.setHeight( (aSize.Height() * 3) / 5 );
    aSize.setWidth( (aSize.Width() * 3) / 5 );
    rFont.SetFontSize(aSize);
}

void calcFontHeightAnyAscent(vcl::RenderContext& rRenderContext, const vcl::Font& rFont, tools::Long& nHeight, tools::Long& nAscent)
{
    if (!nHeight)
    {
        rRenderContext.SetFont(rFont);
        FontMetric aMetric(rRenderContext.GetFontMetric());
        nHeight = aMetric.GetLineHeight();
        nAscent = aMetric.GetAscent();
    }
}

void setFont(const SvxFont& rNewFont, SvxFont& rImplFont)
{
    rImplFont = rNewFont;
    rImplFont.SetTransparent(true);
    rImplFont.SetAlignment(ALIGN_BASELINE);
}

/*
 * removes line feeds and carriage returns from string
 * returns if param is empty
 */
bool CleanAndCheckEmpty(OUString& rText)
{
    bool bEmpty = true;
    for (sal_Int32 i = 0; i < rText.getLength(); ++i)
    {
        if (0xa == rText[i] || 0xd == rText[i])
            rText = rText.replaceAt(i, 1, " ");
        else
            bEmpty = false;
    }
    return bEmpty;
}
} // end anonymous namespace

class FontPrevWin_Impl
{
    friend class SvxFontPrevWindow;

    SvxFont maFont;
    VclPtr<Printer> mpPrinter;
    bool mbDelPrinter;

    Reference <XBreakIterator> mxBreak;
    std::vector<tools::Long> maTextWidth;
    std::deque<sal_Int32> maScriptChg;
    std::vector<sal_uInt16> maScriptType;
    SvxFont maCJKFont;
    SvxFont maCTLFont;
    OUString maText;
    OUString maScriptText;
    std::optional<Color> mxColor;
    std::optional<Color> mxBackColor;
    std::optional<Color> mxTextLineColor;
    std::optional<Color> mxOverlineColor;
    tools::Long mnAscent;
    sal_Unicode mcStartBracket;
    sal_Unicode mcEndBracket;

    tools::Long mn100PercentFontWidth; // initial -1 -> not set yet
    tools::Long mn100PercentFontWidthCJK;
    tools::Long mn100PercentFontWidthCTL;
    sal_uInt16 mnFontWidthScale;

    bool mbSelection : 1;
    bool mbGetSelection : 1;
    bool mbTwoLines : 1;
    bool mbUseFontNameAsText : 1;
    bool mbTextInited : 1;

    bool m_bCJKEnabled;
    bool m_bCTLEnabled;


public:
    FontPrevWin_Impl() :
        mpPrinter(nullptr),
        mbDelPrinter(false),
        mnAscent(0),
        mcStartBracket(0),
        mcEndBracket(0),
        mnFontWidthScale(100),
        mbSelection(false),
        mbGetSelection(false),
        mbTwoLines(false),
        mbUseFontNameAsText(false),
        mbTextInited(false)
    {
        SvtLanguageOptions aLanguageOptions;
        m_bCJKEnabled = aLanguageOptions.IsAnyEnabled();
        m_bCTLEnabled = aLanguageOptions.IsCTLFontEnabled();
        mxBackColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
        Invalidate100PercentFontWidth();
    }

    ~FontPrevWin_Impl()
    {
        if (mbDelPrinter)
            mpPrinter.disposeAndClear();
    }

    void CheckScript();
    Size CalcTextSize(vcl::RenderContext& rRenderContext, OutputDevice const * pPrinter, const SvxFont& rFont);
    void DrawPrev(vcl::RenderContext& rRenderContext, Printer* pPrinter, Point& rPt, const SvxFont& rFont);

    bool SetFontWidthScale(sal_uInt16 nScaleInPercent);
    inline void Invalidate100PercentFontWidth();
    inline bool Is100PercentFontWidthValid() const;
    void ScaleFontWidth(vcl::RenderContext const & rRenderContext);
                            // scales rNonCJKFont and aCJKFont depending on nFontWidthScale and
                            // sets the 100%-Font-Widths
};

inline void FontPrevWin_Impl::Invalidate100PercentFontWidth()
{
    mn100PercentFontWidth = mn100PercentFontWidthCJK = mn100PercentFontWidthCTL = -1;
}

inline bool FontPrevWin_Impl::Is100PercentFontWidthValid() const
{
    DBG_ASSERT( ( mn100PercentFontWidth == -1 && mn100PercentFontWidthCJK == -1 ) ||
                ( mn100PercentFontWidth != -1 && mn100PercentFontWidthCJK != -1 ) ||
                ( mn100PercentFontWidth == -1 && mn100PercentFontWidthCTL == -1 ) ||
                ( mn100PercentFontWidth != -1 && mn100PercentFontWidthCTL != -1 ),
                "*FontPrevWin_Impl::Is100PercentFontWidthValid(): 100PercentFontWidth's not synchronous" );
    return mn100PercentFontWidth != -1;
}

/*
 * evaluates the scripttypes of the actual string.
 * Afterwards the positions of script change are notified in aScriptChg,
 * the scripttypes in aScriptType.
 * The aTextWidth array will be filled with zero.
 */
void FontPrevWin_Impl::CheckScript()
{
    assert(!maText.isEmpty()); // must have a preview text here!
    if (maText == maScriptText)
    {
        return; // already initialized
    }

    maScriptText = maText;

    maScriptChg.clear();
    maScriptType.clear();
    maTextWidth.clear();

    if (!mxBreak.is())
    {
        Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        mxBreak = BreakIterator::create(xContext);
    }

    sal_uInt16 nScript = 0;
    sal_Int32 nChg = 0;

    while (nChg < maText.getLength())
    {
        nScript = mxBreak->getScriptType(maText, nChg);
        nChg = mxBreak->endOfScript(maText, nChg, nScript);
        if (nChg < maText.getLength() && nChg > 0 &&
            (css::i18n::ScriptType::WEAK ==
             mxBreak->getScriptType(maText, nChg - 1)))
        {
            int8_t nType = u_charType(maText[nChg]);
            if (nType == U_NON_SPACING_MARK || nType == U_ENCLOSING_MARK ||
                nType == U_COMBINING_SPACING_MARK)
            {
                maScriptChg.push_back(nChg - 1);
            }
            else
            {
                maScriptChg.push_back(nChg);
            }
        }
        else
        {
            maScriptChg.push_back(nChg);
        }
        maScriptType.push_back(nScript);
        maTextWidth.push_back(0);
    }
}

/*
 * Size FontPrevWin_Impl::CalcTextSize(..)
 * fills the aTextWidth array with the text width of every part
 * of the actual string without a script change inside.
 * For Latin parts the given rFont will be used,
 * for Asian parts the aCJKFont.
 * The returned size contains the whole string.
 * The member nAscent is calculated to the maximal ascent of all used fonts.
 */

Size FontPrevWin_Impl::CalcTextSize(vcl::RenderContext& rRenderContext, OutputDevice const * _pPrinter, const SvxFont& rInFont)
{
    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd;
    size_t nCnt = maScriptChg.size();

    if (nCnt)
    {
        nEnd = maScriptChg[nIdx];
        nScript = maScriptType[nIdx];
    }
    else
    {
        nEnd = maText.getLength();
        nScript = css::i18n::ScriptType::LATIN;
    }
    tools::Long nTxtWidth = 0;
    tools::Long nCJKHeight = 0;
    tools::Long nCTLHeight = 0;
    tools::Long nHeight = 0;
    mnAscent = 0;
    tools::Long nCJKAscent = 0;
    tools::Long nCTLAscent = 0;

    do
    {
        const SvxFont& rFont = (nScript == css::i18n::ScriptType::ASIAN) ?
                                    maCJKFont :
                                    ((nScript == css::i18n::ScriptType::COMPLEX) ?
                                        maCTLFont :
                                        rInFont);
        tools::Long nWidth = rFont.GetTextSize(_pPrinter, maText, nStart, nEnd - nStart).Width();
        if (nIdx >= maTextWidth.size())
            break;

        maTextWidth[nIdx++] = nWidth;
        nTxtWidth += nWidth;

        switch (nScript)
        {
            case css::i18n::ScriptType::ASIAN:
                calcFontHeightAnyAscent(rRenderContext, maCJKFont, nCJKHeight, nCJKAscent);
                break;
            case css::i18n::ScriptType::COMPLEX:
                calcFontHeightAnyAscent(rRenderContext, maCTLFont, nCTLHeight, nCTLAscent);
                break;
            default:
                calcFontHeightAnyAscent(rRenderContext, rFont, nHeight, mnAscent);
        }

        if (nEnd < maText.getLength() && nIdx < nCnt)
        {
            nStart = nEnd;
            nEnd = maScriptChg[nIdx];
            nScript = maScriptType[nIdx];
        }
        else
            break;
    }
    while(true);

    nHeight -= mnAscent;
    nCJKHeight -= nCJKAscent;
    nCTLHeight -= nCTLAscent;

    if (nHeight < nCJKHeight)
        nHeight = nCJKHeight;

    if (mnAscent < nCJKAscent)
        mnAscent = nCJKAscent;

    if (nHeight < nCTLHeight)
        nHeight = nCTLHeight;

    if (mnAscent < nCTLAscent)
        mnAscent = nCTLAscent;

    nHeight += mnAscent;

    Size aTxtSize(nTxtWidth, nHeight);
    return aTxtSize;
}

/*
 * void FontPrevWin_Impl::DrawPrev(..)
 * calls SvxFont::DrawPrev(..) for every part of the string without a script
 * change inside, for Asian parts the aCJKFont will be used, otherwise the
 * given rFont.
 */

void FontPrevWin_Impl::DrawPrev(vcl::RenderContext& rRenderContext, Printer* _pPrinter, Point &rPt, const SvxFont& rInFont)
{
    vcl::Font aOldFont = _pPrinter->GetFont();
    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd;
    size_t nCnt = maScriptChg.size();
    if (nCnt)
    {
        nEnd = maScriptChg[nIdx];
        nScript = maScriptType[nIdx];
    }
    else
    {
        nEnd = maText.getLength();
        nScript = css::i18n::ScriptType::LATIN;
    }
    do
    {
        const SvxFont& rFont = (nScript == css::i18n::ScriptType::ASIAN)
                                    ? maCJKFont
                                    : ((nScript == css::i18n::ScriptType::COMPLEX)
                                        ? maCTLFont
                                        : rInFont);
        _pPrinter->SetFont(rFont);

        rFont.DrawPrev(&rRenderContext, _pPrinter, rPt, maText, nStart, nEnd - nStart);

        rPt.AdjustX(maTextWidth[nIdx++] );
        if (nEnd < maText.getLength() && nIdx < nCnt)
        {
            nStart = nEnd;
            nEnd = maScriptChg[nIdx];
            nScript = maScriptType[nIdx];
        }
        else
            break;
    }
    while(true);
    _pPrinter->SetFont(aOldFont);
}


bool FontPrevWin_Impl::SetFontWidthScale(sal_uInt16 nScale)
{
    if (mnFontWidthScale != nScale)
    {
        mnFontWidthScale = nScale;
        return true;
    }

    return false;
}

void FontPrevWin_Impl::ScaleFontWidth(vcl::RenderContext const & rOutDev)
{
    if (!Is100PercentFontWidthValid())
    {
        scaleFontWidth(maFont, rOutDev, mn100PercentFontWidth);
        scaleFontWidth(maCJKFont, rOutDev, mn100PercentFontWidthCJK);
        scaleFontWidth(maCTLFont, rOutDev, mn100PercentFontWidthCTL);
    }

    maFont.SetAverageFontWidth(mn100PercentFontWidth * mnFontWidthScale / 100);
    maCJKFont.SetAverageFontWidth(mn100PercentFontWidthCJK * mnFontWidthScale / 100);
    maCTLFont.SetAverageFontWidth(mn100PercentFontWidthCTL * mnFontWidthScale / 100);
}

static bool GetWhich (const SfxItemSet& rSet, sal_uInt16 nSlot, sal_uInt16& rWhich)
{
    rWhich = rSet.GetPool()->GetWhich(nSlot);
    return rSet.GetItemState(rWhich) >= SfxItemState::DEFAULT;
}

static void SetPrevFont(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(rSet.Get(nWhich));
        rFont.SetFamily(rFontItem.GetFamily());
        rFont.SetFamilyName(rFontItem.GetFamilyName());
        rFont.SetPitch(rFontItem.GetPitch());
        rFont.SetCharSet(rFontItem.GetCharSet());
        rFont.SetStyleName(rFontItem.GetStyleName());
    }
}

static void SetPrevFontStyle( const SfxItemSet& rSet, sal_uInt16 nPosture, sal_uInt16 nWeight, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    if( GetWhich( rSet, nPosture, nWhich ) )
    {
        const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>( rSet.Get( nWhich ) );
        rFont.SetItalic( rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    if( GetWhich( rSet, nWeight, nWhich ) )
    {
        const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>( rSet.Get( nWhich ) );
        rFont.SetWeight( rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

static void SetPrevFontEscapement(SvxFont& rFont, sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc)
{
    rFont.SetPropr(nProp);
    rFont.SetProprRel(nEscProp);
    rFont.SetEscapement(nEsc);
}

void SvxFontPrevWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetTextColor( svtools::ColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
    rRenderContext.SetBackground( svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );
}

void SvxFontPrevWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aPrefSize(getPreviewStripSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aPrefSize.Width(), aPrefSize.Height());

    pImpl.reset(new FontPrevWin_Impl);
    SfxViewShell* pSh = SfxViewShell::Current();

    if (pSh)
        pImpl->mpPrinter = pSh->GetPrinter();

    if (!pImpl->mpPrinter)
    {
        pImpl->mpPrinter = VclPtr<Printer>::Create();
        pImpl->mbDelPrinter = true;
    }
    initFont(pImpl->maFont);
    initFont(pImpl->maCJKFont);
    initFont(pImpl->maCTLFont);

    Invalidate();
}

SvxFontPrevWindow::SvxFontPrevWindow()
{
}

SvxFontPrevWindow::~SvxFontPrevWindow()
{
}

SvxFont& SvxFontPrevWindow::GetCTLFont()
{
    return pImpl->maCTLFont;
}

SvxFont& SvxFontPrevWindow::GetCJKFont()
{
    return pImpl->maCJKFont;
}

SvxFont& SvxFontPrevWindow::GetFont()
{
    pImpl->Invalidate100PercentFontWidth();     // because the user might change the size
    return pImpl->maFont;
}

const SvxFont& SvxFontPrevWindow::GetFont() const
{
    return pImpl->maFont;
}

void SvxFontPrevWindow::SetPreviewText( const OUString& rString )
{
    pImpl->maText = rString;
    pImpl->mbTextInited = true;
}

void SvxFontPrevWindow::SetFontNameAsPreviewText()
{
    pImpl->mbUseFontNameAsText = true;
}

void SvxFontPrevWindow::SetFont( const SvxFont& rNormalOutFont, const SvxFont& rCJKOutFont, const SvxFont& rCTLFont )
{
    setFont(rNormalOutFont, pImpl->maFont);
    setFont(rCJKOutFont, pImpl->maCJKFont);
    setFont(rCTLFont, pImpl->maCTLFont);

    pImpl->Invalidate100PercentFontWidth();
    Invalidate();
}

void SvxFontPrevWindow::SetColor(const Color &rColor)
{
    pImpl->mxColor = rColor;
    Invalidate();
}

void SvxFontPrevWindow::ResetColor()
{
    pImpl->mxColor.reset();
    Invalidate();
}

void SvxFontPrevWindow::SetTextLineColor(const Color &rColor)
{
    pImpl->mxTextLineColor = rColor;
    Invalidate();
}

void SvxFontPrevWindow::SetOverlineColor(const Color &rColor)
{
    pImpl->mxOverlineColor = rColor;
    Invalidate();
}

void SvxFontPrevWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::ALL);
    rRenderContext.SetMapMode(MapMode(MapUnit::MapTwip));

    ApplySettings(rRenderContext);

    Printer* pPrinter = pImpl->mpPrinter;
    const SvxFont& rFont = pImpl->maFont;
    const SvxFont& rCJKFont = pImpl->maCJKFont;
    const SvxFont& rCTLFont = pImpl->maCTLFont;

    if (!IsEnabled())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size aLogSize(rRenderContext.GetOutputSize());

        tools::Rectangle aRect(Point(0, 0), aLogSize);
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetWindowColor());
        rRenderContext.DrawRect(aRect);
    }
    else
    {
        if (!pImpl->mbSelection && !pImpl->mbTextInited)
        {
            using namespace css::i18n::ScriptType;

            SfxViewShell* pSh = SfxViewShell::Current();

            if (pSh && !pImpl->mbGetSelection && !pImpl->mbUseFontNameAsText)
            {
                pImpl->maText = pSh->GetSelectionText();
                pImpl->mbGetSelection = true;
                pImpl->mbSelection = !CleanAndCheckEmpty(pImpl->maText);
            }

            if (!pImpl->mbSelection || pImpl->mbUseFontNameAsText)
            {
                //If we're showing multiple sample texts, then they're all
                //sample texts. If only showing Latin, continue to use
                //the fontname as the preview
                if ((pImpl->m_bCJKEnabled) || (pImpl->m_bCTLEnabled))
                    pImpl->maText = makeRepresentativeTextForFont(LATIN, rFont);
                else
                    pImpl->maText = rFont.GetFamilyName();

                if (pImpl->m_bCJKEnabled)
                {
                    if (!pImpl->maText.isEmpty())
                        pImpl->maText += "   ";
                    pImpl->maText += makeRepresentativeTextForFont(ASIAN, rCJKFont);

                }
                if (pImpl->m_bCTLEnabled)
                {
                    if (!pImpl->maText.isEmpty())
                        pImpl->maText += "   ";
                    pImpl->maText += makeRepresentativeTextForFont(COMPLEX, rCTLFont);
                }
            }

            if (pImpl->maText.isEmpty())
            {   // fdo#58427: still no text? let's try that one...
                pImpl->maText = makeRepresentativeTextForFont(LATIN, rFont);
            }

            bool bEmpty = CleanAndCheckEmpty(pImpl->maText);
            if (bEmpty)
                pImpl->maText = OUString();

            if (pImpl->maText.getLength() > (TEXT_WIDTH - 1))
            {
                const sal_Int32 nSpaceIdx = pImpl->maText.indexOf(" ", TEXT_WIDTH);
                if (nSpaceIdx != -1)
                    pImpl->maText = pImpl->maText.copy(0, nSpaceIdx);
                else
                    pImpl->maText = pImpl->maText.copy(0, (TEXT_WIDTH - 1));
            }
        }

        // calculate text width scaling
        pImpl->ScaleFontWidth(rRenderContext);

        pImpl->CheckScript();
        Size aTxtSize = pImpl->CalcTextSize(rRenderContext, pPrinter, rFont);

        const Size aLogSize(rRenderContext.GetOutputSize());

        tools::Long nX = aLogSize.Width()  / 2 - aTxtSize.Width() / 2;
        tools::Long nY = aLogSize.Height() / 2 - aTxtSize.Height() / 2;

        if (nY + pImpl->mnAscent > aLogSize.Height())
            nY = aLogSize.Height() - pImpl->mnAscent;

        if (pImpl->mxBackColor)
        {
            tools::Rectangle aRect(Point(0, 0), aLogSize);
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(*pImpl->mxBackColor);
            rRenderContext.DrawRect(aRect);
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }
        if (pImpl->mxColor)
        {
            tools::Rectangle aRect(Point(nX, nY), aTxtSize);
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(*pImpl->mxColor);
            rRenderContext.DrawRect(aRect);
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }

        if (pImpl->mxTextLineColor)
        {
            rRenderContext.SetTextLineColor(*pImpl->mxTextLineColor);
        }

        if (pImpl->mxOverlineColor)
        {
            rRenderContext.SetOverlineColor(*pImpl->mxOverlineColor);
        }

        tools::Long nStdAscent = pImpl->mnAscent;
        nY += nStdAscent;

        if (IsTwoLines())
        {
            SvxFont aSmallFont(rFont);
            Size aOldSize = pImpl->maCJKFont.GetFontSize();
            setFontSize(aSmallFont);
            setFontSize(pImpl->maCJKFont);

            tools::Long nStartBracketWidth = 0;
            tools::Long nEndBracketWidth = 0;
            tools::Long nTextWidth = 0;
            if (pImpl->mcStartBracket)
            {
                OUString sBracket(pImpl->mcStartBracket);
                nStartBracketWidth = rFont.GetTextSize(pPrinter, sBracket).Width();
            }
            if (pImpl->mcEndBracket)
            {
                OUString sBracket(pImpl->mcEndBracket);
                nEndBracketWidth = rFont.GetTextSize(pPrinter, sBracket).Width();
            }
            nTextWidth = pImpl->CalcTextSize(rRenderContext, pPrinter, aSmallFont).Width();
            tools::Long nResultWidth = nStartBracketWidth;
            nResultWidth += nEndBracketWidth;
            nResultWidth += nTextWidth;

            tools::Long _nX = (aLogSize.Width() - nResultWidth) / 2;
            rRenderContext.DrawLine(Point(0,  nY), Point(_nX, nY));
            rRenderContext.DrawLine(Point(_nX + nResultWidth, nY), Point(aLogSize.Width(), nY));

            tools::Long nSmallAscent = pImpl->mnAscent;
            tools::Long nOffset = (nStdAscent - nSmallAscent) / 2;

            if (pImpl->mcStartBracket)
            {
                OUString sBracket(pImpl->mcStartBracket);
                rFont.DrawPrev(&rRenderContext, pPrinter, Point(_nX, nY - nOffset - 4), sBracket);
                _nX += nStartBracketWidth;
            }

            Point aTmpPoint1(_nX, nY - nSmallAscent - 2);
            Point aTmpPoint2(_nX, nY);
            pImpl->DrawPrev(rRenderContext, pPrinter, aTmpPoint1, aSmallFont);
            pImpl->DrawPrev(rRenderContext, pPrinter, aTmpPoint2, aSmallFont);

            _nX += nTextWidth;
            if (pImpl->mcEndBracket)
            {
                Point aTmpPoint( _nX + 1, nY - nOffset - 4);
                OUString sBracket(pImpl->mcEndBracket);
                rFont.DrawPrev(&rRenderContext, pPrinter, aTmpPoint, sBracket);
            }
            pImpl->maCJKFont.SetFontSize(aOldSize);
        }
        else
        {

            Color aLineCol = rRenderContext.GetLineColor();

            rRenderContext.SetLineColor(rFont.GetColor());
            rRenderContext.DrawLine(Point(0,  nY), Point(nX, nY));
            rRenderContext.DrawLine(Point(nX + aTxtSize.Width(), nY), Point(aLogSize.Width(), nY));
            rRenderContext.SetLineColor(aLineCol);

            Point aTmpPoint(nX, nY);
            pImpl->DrawPrev(rRenderContext, pPrinter, aTmpPoint, rFont);
        }
    }
    rRenderContext.Pop();
}

bool SvxFontPrevWindow::IsTwoLines() const
{
    return pImpl->mbTwoLines;
}

void SvxFontPrevWindow::SetTwoLines(bool bSet)
{
    pImpl->mbTwoLines = bSet;
}

void SvxFontPrevWindow::SetBrackets(sal_Unicode cStart, sal_Unicode cEnd)
{
    pImpl->mcStartBracket = cStart;
    pImpl->mcEndBracket = cEnd;
}

void SvxFontPrevWindow::SetFontWidthScale( sal_uInt16 n )
{
    if (pImpl->SetFontWidthScale(n))
        Invalidate();
}

void SvxFontPrevWindow::AutoCorrectFontColor()
{
    Color aColor(COL_AUTO);
    if ( pImpl->mxBackColor ) aColor = *pImpl->mxBackColor;
    const bool bIsDark(aColor.IsDark());

    aColor = pImpl->maFont.GetColor();
    if (aColor == COL_AUTO)
        pImpl->maFont.SetColor( bIsDark ? COL_WHITE : COL_BLACK );
    aColor = pImpl->maCJKFont.GetColor();
    if (aColor == COL_AUTO)
        pImpl->maCJKFont.SetColor( bIsDark ? COL_WHITE : COL_BLACK );
    aColor = pImpl->maCTLFont.GetColor();
    if (aColor == COL_AUTO)
        pImpl->maCTLFont.SetColor( bIsDark ? COL_WHITE : COL_BLACK );
}

void SvxFontPrevWindow::SetFontSize( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    tools::Long nH;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        nH = OutputDevice::LogicToLogic(static_cast<const SvxFontHeightItem&>(rSet.Get(nWhich)).GetHeight(),
                          rSet.GetPool()->GetMetric(nWhich),
                          MapUnit::MapTwip);
    }
    else
        nH = 240;// as default 12pt

    rFont.SetFontSize(Size(0, nH));
}

void SvxFontPrevWindow::SetFontLang(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    LanguageType nLang;
    if( GetWhich( rSet, nSlot, nWhich ) )
        nLang = static_cast<const SvxLanguageItem&>(rSet.Get(nWhich)).GetLanguage();
    else
        nLang = LANGUAGE_NONE;
    rFont.SetLanguage(nLang);
}

void SvxFontPrevWindow::SetFromItemSet(const SfxItemSet &rSet, bool bPreviewBackgroundToCharacter)
{
    sal_uInt16 nWhich;
    SvxFont& rFont = GetFont();
    SvxFont& rCJKFont = GetCJKFont();
    SvxFont& rCTLFont = GetCTLFont();

    // Preview string
    if( GetWhich( rSet, SID_CHAR_DLG_PREVIEW_STRING, nWhich ) )
    {
        const SfxStringItem& rItem = static_cast<const SfxStringItem&>( rSet.Get( nWhich ) );
        const OUString& aString = rItem.GetValue();
        if( !aString.isEmpty() )
            SetPreviewText( aString );
        else
            SetFontNameAsPreviewText();
    }

    // Underline
    FontLineStyle eUnderline;
    if( GetWhich( rSet, SID_ATTR_CHAR_UNDERLINE, nWhich ) )
    {
        const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>( rSet.Get( nWhich ) );
        eUnderline = rItem.GetValue();
    }
    else
        eUnderline = LINESTYLE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    // Overline
    FontLineStyle eOverline;
    if( GetWhich( rSet, SID_ATTR_CHAR_OVERLINE, nWhich ) )
    {
        const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>( rSet.Get( nWhich ) );
        eOverline = rItem.GetValue();
    }
    else
        eOverline = LINESTYLE_NONE;

    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );

    //  Strikeout
    FontStrikeout eStrikeout;
    if( GetWhich( rSet, SID_ATTR_CHAR_STRIKEOUT, nWhich ) )
    {
        const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>( rSet.Get( nWhich ) );
        eStrikeout = rItem.GetValue();
    }
    else
        eStrikeout = STRIKEOUT_NONE;

    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    // WordLineMode
    if( GetWhich( rSet, SID_ATTR_CHAR_WORDLINEMODE, nWhich ) )
    {
        const SvxWordLineModeItem& rItem = static_cast<const SvxWordLineModeItem&>( rSet.Get( nWhich ) );
        rFont.SetWordLineMode( rItem.GetValue() );
        rCJKFont.SetWordLineMode( rItem.GetValue() );
        rCTLFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    if( GetWhich( rSet, SID_ATTR_CHAR_EMPHASISMARK, nWhich ) )
    {
        const SvxEmphasisMarkItem& rItem = static_cast<const SvxEmphasisMarkItem&>( rSet.Get( nWhich ) );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    // Relief
    if( GetWhich( rSet, SID_ATTR_CHAR_RELIEF, nWhich ) )
    {
        const SvxCharReliefItem& rItem = static_cast<const SvxCharReliefItem&>( rSet.Get( nWhich ) );
        FontRelief eFontRelief = rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    if( GetWhich( rSet, SID_ATTR_CHAR_CASEMAP, nWhich ) )
    {
        const SvxCaseMapItem& rItem = static_cast<const SvxCaseMapItem&>( rSet.Get( nWhich ) );
        SvxCaseMap eCaseMap = rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaseMap == SvxCaseMap::SmallCaps ? SvxCaseMap::NotMapped : eCaseMap );
    }

    // Outline
    if( GetWhich( rSet, SID_ATTR_CHAR_CONTOUR, nWhich ) )
    {
        const SvxContourItem& rItem = static_cast<const  SvxContourItem&>( rSet.Get( nWhich ) );
        bool bOutline = rItem.GetValue();
        rFont.SetOutline( bOutline );
        rCJKFont.SetOutline( bOutline );
        rCTLFont.SetOutline( bOutline );
    }

    // Shadow
    if( GetWhich( rSet, SID_ATTR_CHAR_SHADOWED, nWhich ) )
    {
        const SvxShadowedItem& rItem = static_cast<const  SvxShadowedItem&>( rSet.Get( nWhich ) );
        bool bShadow = rItem.GetValue();
        rFont.SetShadow( bShadow );
        rCJKFont.SetShadow( bShadow );
        rCTLFont.SetShadow( bShadow );
    }

    // Background
    bool bTransparent;
    if( GetWhich( rSet, bPreviewBackgroundToCharacter ? SID_ATTR_BRUSH : SID_ATTR_BRUSH_CHAR, nWhich ) )
    {
         const SvxBrushItem& rBrush = static_cast<const SvxBrushItem&>( rSet.Get( nWhich ) );
         const Color& rColor = rBrush.GetColor();
         bTransparent = rColor.IsTransparent();
         rFont.SetFillColor( rColor );
         rCJKFont.SetFillColor( rColor );
         rCTLFont.SetFillColor( rColor );
    }
    else
        bTransparent = true;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    if( !bPreviewBackgroundToCharacter )
    {
        if( GetWhich( rSet, SID_ATTR_BRUSH, nWhich ) )
        {
            const SvxBrushItem& rBrush = static_cast<const  SvxBrushItem&>( rSet.Get( nWhich ) );
            if( GPOS_NONE == rBrush.GetGraphicPos() )
                pImpl->mxBackColor = rBrush.GetColor();
        }
    }

    // Font
    SetPrevFont( rSet, SID_ATTR_CHAR_FONT, rFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CJK_FONT, rCJKFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CTL_FONT, rCTLFont );

    // Style
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_POSTURE, SID_ATTR_CHAR_WEIGHT, rFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CJK_POSTURE, SID_ATTR_CHAR_CJK_WEIGHT, rCJKFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CTL_POSTURE, SID_ATTR_CHAR_CTL_WEIGHT, rCTLFont );

    // Size
    SetFontSize( rSet, SID_ATTR_CHAR_FONTHEIGHT, rFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, rCJKFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, rCTLFont );

    // Language
    SetFontLang( rSet, SID_ATTR_CHAR_LANGUAGE, rFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CJK_LANGUAGE, rCJKFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CTL_LANGUAGE, rCTLFont );

    // Color
    if( GetWhich( rSet, SID_ATTR_CHAR_COLOR, nWhich ) )
    {
        const SvxColorItem& rItem = static_cast<const SvxColorItem&>( rSet.Get( nWhich ) );
        Color aCol( rItem.GetValue() );
        rFont.SetColor( aCol );

        rCJKFont.SetColor( aCol );
        rCTLFont.SetColor( aCol );

        AutoCorrectFontColor(); // handle color COL_AUTO
    }

    // Kerning
    if( GetWhich( rSet, SID_ATTR_CHAR_KERNING, nWhich ) )
    {
        const SvxKerningItem& rItem = static_cast<const SvxKerningItem&>( rSet.Get( nWhich ) );
        short nKern = static_cast<short>(OutputDevice::LogicToLogic(rItem.GetValue(), rSet.GetPool()->GetMetric(nWhich), MapUnit::MapTwip));
        rFont.SetFixKerning( nKern );
        rCJKFont.SetFixKerning( nKern );
        rCTLFont.SetFixKerning( nKern );
    }

    // Escapement
    const sal_uInt8 nProp = 100;
    short nEsc;
    sal_uInt8 nEscProp;
    if( GetWhich( rSet, SID_ATTR_CHAR_ESCAPEMENT, nWhich ) )
    {
        const SvxEscapementItem& rItem = static_cast<const SvxEscapementItem&>( rSet.Get( nWhich ) );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProportionalHeight();

        if( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = DFLT_ESC_SUPER;
        else if( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = DFLT_ESC_SUB;
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }
    SetPrevFontEscapement( rFont, nProp, nEscProp, nEsc );
    SetPrevFontEscapement( rCJKFont, nProp, nEscProp, nEsc );
    SetPrevFontEscapement( rCTLFont, nProp, nEscProp, nEsc );

    // Font width scale
    if( GetWhich( rSet, SID_ATTR_CHAR_SCALEWIDTH, nWhich ) )
    {
        const SvxCharScaleWidthItem&rItem = static_cast<const SvxCharScaleWidthItem&>( rSet.Get( nWhich ) );
        SetFontWidthScale( rItem.GetValue() );
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
