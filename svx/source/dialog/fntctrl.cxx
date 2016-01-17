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
#include <vcl/builderfactory.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>

#include <vector>
#include <deque>
#include <svtools/colorcfg.hxx>
#include <svtools/sampletext.hxx>

#include <svx/fntctrl.hxx>
#include <svx/dialogs.hrc>
#include <svx/sdasitm.hxx>

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
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/langitem.hxx>

//TODO: remove this and calculate off the actual size of text, not
//an arbitrary number of characters
#define TEXT_WIDTH 80
#define ISITEMSET rSet.GetItemState(nWhich)>=SfxItemState::DEFAULT

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::com::sun::star::i18n::XBreakIterator;
using ::com::sun::star::i18n::BreakIterator;


// small helper functions to set fonts

namespace
{
void scaleFontWidth(vcl::Font& rFont, vcl::RenderContext& rRenderContext,long& n100PercentFont)
{
    rFont.SetWidth(0);
    n100PercentFont = rRenderContext.GetFontMetric(rFont).GetWidth();
}

void initFont(vcl::Font& rFont)
{
    rFont.SetTransparent(true);
    rFont.SetAlign(ALIGN_BASELINE);
}

void setFontSize(vcl::Font& rFont)
{
    Size aSize(rFont.GetSize());
    aSize.Height() = (aSize.Height() * 3) / 5;
    aSize.Width() = (aSize.Width() * 3) / 5;
    rFont.SetSize(aSize);
}

void calcFontHeightAnyAscent(vcl::RenderContext& rRenderContext, const vcl::Font& rFont, long& nHeight, long& nAscent)
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
    rImplFont.SetAlign(ALIGN_BASELINE);
}

} // end anonymous namespace

class FontPrevWin_Impl
{
    friend class SvxFontPrevWindow;

    SvxFont maFont;
    VclPtr<Printer> mpPrinter;
    bool mbDelPrinter;

    Reference <XBreakIterator> mxBreak;
    std::vector<sal_uIntPtr> maTextWidth;
    std::deque<sal_Int32> maScriptChg;
    std::vector<sal_uInt16> maScriptType;
    SvxFont maCJKFont;
    SvxFont maCTLFont;
    OUString maText;
    OUString maScriptText;
    Color* mpColor;
    Color* mpBackColor;
    long mnAscent;
    sal_Unicode mcStartBracket;
    sal_Unicode mcEndBracket;

    long mn100PercentFontWidth; // initial -1 -> not set yet
    long mn100PercentFontWidthCJK;
    long mn100PercentFontWidthCTL;
    sal_uInt16 mnFontWidthScale;

    bool mbSelection : 1;
    bool mbGetSelection : 1;
    bool mbUseResText : 1;
    bool mbPreviewBackgroundToCharacter : 1;
    bool mbTwoLines : 1;
    bool mbUseFontNameAsText : 1;
    bool mbTextInited : 1;

    bool m_bCJKEnabled;
    bool m_bCTLEnabled;


public:
    inline FontPrevWin_Impl() :
        mpPrinter(nullptr),
        mbDelPrinter(false),
        mpColor(nullptr),
        mpBackColor(nullptr),
        mnAscent(0),
        mcStartBracket(0),
        mcEndBracket(0),
        mnFontWidthScale(100),
        mbSelection(false),
        mbGetSelection(false),
        mbUseResText(false),
        mbPreviewBackgroundToCharacter(false),
        mbTwoLines(false),
        mbUseFontNameAsText(false),
        mbTextInited(false)
    {
        SvtLanguageOptions aLanguageOptions;
        m_bCJKEnabled = aLanguageOptions.IsAnyEnabled();
        m_bCTLEnabled = aLanguageOptions.IsCTLFontEnabled();

        Invalidate100PercentFontWidth();
    }

    inline ~FontPrevWin_Impl()
    {
        delete mpColor;
        delete mpBackColor;
        if (mbDelPrinter)
            mpPrinter.disposeAndClear();
    }

    void CheckScript();
    Size CalcTextSize(vcl::RenderContext& rRenderContext, OutputDevice* pPrinter, const SvxFont& rFont);
    void DrawPrev(vcl::RenderContext& rRenderContext, Printer* pPrinter, Point& rPt, const SvxFont& rFont);

    bool SetFontWidthScale(sal_uInt16 nScaleInPercent);
    inline void Invalidate100PercentFontWidth();
    inline bool Is100PercentFontWidthValid() const;
    void ScaleFontWidth(vcl::RenderContext& rRenderContext);
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
 * evalutates the scripttypes of the actual string.
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

Size FontPrevWin_Impl::CalcTextSize(vcl::RenderContext& rRenderContext, OutputDevice* _pPrinter, const SvxFont& rInFont)
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
    long nTxtWidth = 0;
    long nCJKHeight = 0;
    long nCTLHeight = 0;
    long nHeight = 0;
    mnAscent = 0;
    long nCJKAscent = 0;
    long nCTLAscent = 0;

    do
    {
        const SvxFont& rFont = (nScript == css::i18n::ScriptType::ASIAN) ?
                                    maCJKFont :
                                    ((nScript == css::i18n::ScriptType::COMPLEX) ?
                                        maCTLFont :
                                        rInFont);
        sal_uIntPtr nWidth = rFont.GetTextSize(_pPrinter, maText, nStart, nEnd - nStart).Width();
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

        rPt.X() += maTextWidth[nIdx++];
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

void FontPrevWin_Impl::ScaleFontWidth(vcl::RenderContext& rOutDev)
{
    if (!Is100PercentFontWidthValid())
    {
        scaleFontWidth(maFont, rOutDev, mn100PercentFontWidth);
        scaleFontWidth(maCJKFont, rOutDev, mn100PercentFontWidthCJK);
        scaleFontWidth(maCTLFont, rOutDev, mn100PercentFontWidthCTL);
    }

    maFont.SetWidth(mn100PercentFontWidth * mnFontWidthScale / 100);
    maCJKFont.SetWidth(mn100PercentFontWidthCJK * mnFontWidthScale / 100);
    maCTLFont.SetWidth(mn100PercentFontWidthCTL * mnFontWidthScale / 100);
}

void SvxFontPrevWindow::ResetSettings(bool bForeground, bool bBackground)
{
    mbResetForeground = bForeground;
    mbResetBackground = bBackground;
    Invalidate();
}

void SvxFontPrevWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if (mbResetForeground)
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor(aColorConfig.GetColorValue(svtools::FONTCOLOR).nColor);

        if (IsControlForeground())
            aTextColor = GetControlForeground();
        rRenderContext.SetTextColor(aTextColor);
        mbResetForeground = false;
    }

    if (mbResetBackground)
    {
        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(rStyleSettings.GetWindowColor());
        mbResetBackground = false;
    }
}

void SvxFontPrevWindow::Init()
{
    pImpl.reset(new FontPrevWin_Impl);
    SfxViewShell* pSh = SfxViewShell::Current();

    if (pSh)
        pImpl->mpPrinter = pSh->GetPrinter();

    if (!pImpl->mpPrinter)
    {
        pImpl->mpPrinter = VclPtr<Printer>::Create();
        pImpl->mbDelPrinter = true;
    }
    SetMapMode(MapMode(MAP_TWIP));
    initFont(pImpl->maFont);
    initFont(pImpl->maCJKFont);
    initFont(pImpl->maCTLFont);

    ResetSettings(true, true);

    SetBorderStyle(WindowBorderStyle::MONO);
}

SvxFontPrevWindow::SvxFontPrevWindow(vcl::Window* pParent, const ResId& rId)
    : Window(pParent, rId)
    , mbResetForeground(true)
    , mbResetBackground(true)
{
    Init();
}

SvxFontPrevWindow::SvxFontPrevWindow(vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , mbResetForeground(true)
    , mbResetBackground(true)
{
    Init();
}

VCL_BUILDER_DECL_FACTORY(SvxFontPrevWindow)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SvxFontPrevWindow>::Create(pParent, nWinStyle);
}

SvxFontPrevWindow::~SvxFontPrevWindow()
{
    disposeOnce();
}

void SvxFontPrevWindow::dispose()
{
    pImpl.reset();
    vcl::Window::dispose();
}

SvxFont& SvxFontPrevWindow::GetCTLFont()
{
    return pImpl->maCTLFont;
}

SvxFont& SvxFontPrevWindow::GetCJKFont()
{
    return pImpl->maCJKFont;
}

void SvxFontPrevWindow::StateChanged( StateChangedType nType )
{
    if (nType == StateChangedType::ControlForeground)
        ResetSettings(true, false);
    else if (nType == StateChangedType::ControlBackground)
        ResetSettings(false, true);

    Window::StateChanged(nType);
    Invalidate();
}

void SvxFontPrevWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
        ResetSettings(true, true);
    else
        Window::DataChanged( rDCEvt );
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
    delete pImpl->mpColor;
    pImpl->mpColor = new Color(rColor);
    Invalidate();
}

void SvxFontPrevWindow::ResetColor()
{
    delete pImpl->mpColor;
    pImpl->mpColor = nullptr;
    Invalidate();
}

void SvxFontPrevWindow::SetBackColor(const Color &rColor)
{
    delete pImpl->mpBackColor;
    pImpl->mpBackColor = new Color(rColor);
    Invalidate();
}

void SvxFontPrevWindow::UseResourceText(bool bUse)
{
    pImpl->mbUseResText = bUse;
}

void SvxFontPrevWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    ApplySettings(rRenderContext);

    Printer* pPrinter = pImpl->mpPrinter;
    const SvxFont& rFont = pImpl->maFont;
    const SvxFont& rCJKFont = pImpl->maCJKFont;
    const SvxFont& rCTLFont = pImpl->maCTLFont;

    if (!IsEnabled())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size aLogSize(rRenderContext.GetOutputSize());

        Rectangle aRect(Point(0, 0), aLogSize);
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetWindowColor());
        rRenderContext.DrawRect(aRect);
    }
    else
    {
        if (pImpl->mbUseResText)
            pImpl->maText = GetText();
        else if (!pImpl->mbSelection && !pImpl->mbTextInited)
        {
            using namespace css::i18n::ScriptType;

            SfxViewShell* pSh = SfxViewShell::Current();

            if (pSh && !pImpl->mbGetSelection && !pImpl->mbUseFontNameAsText)
            {
                pImpl->maText = pSh->GetSelectionText();
                pImpl->mbGetSelection = true;
                pImpl->mbSelection = !pImpl->maText.isEmpty();
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
                pImpl->maText = GetText();

            if (pImpl->maText.isEmpty())
            {   // fdo#58427: still no text? let's try that one...
                pImpl->maText = makeRepresentativeTextForFont(LATIN, rFont);
            }

            // remove line feeds and carriage returns from string
            bool bNotEmpty = false;
            for (sal_Int32 i = 0; i < pImpl->maText.getLength(); ++i)
            {
                if (0xa == pImpl->maText[i] || 0xd == pImpl->maText[i])
                     pImpl->maText = pImpl->maText.replaceAt(i, 1, " ");
                else
                    bNotEmpty = true;
            }
            if (!bNotEmpty)
                pImpl->maText = GetText();

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

        long nX = aLogSize.Width()  / 2 - aTxtSize.Width() / 2;
        long nY = aLogSize.Height() / 2 - aTxtSize.Height() / 2;

        if (nY + pImpl->mnAscent > aLogSize.Height())
            nY = aLogSize.Height() - pImpl->mnAscent;

        if (pImpl->mpBackColor)
        {
            Rectangle aRect(Point(0, 0), aLogSize);
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(*pImpl->mpBackColor);
            rRenderContext.DrawRect(aRect);
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }
        if (pImpl->mpColor)
        {
            Rectangle aRect(Point(nX, nY), aTxtSize);
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            rRenderContext.SetFillColor(*pImpl->mpColor);
            rRenderContext.DrawRect(aRect);
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }

        long nStdAscent = pImpl->mnAscent;
        nY += nStdAscent;

        if (IsTwoLines())
        {
            SvxFont aSmallFont(rFont);
            Size aOldSize = pImpl->maCJKFont.GetSize();
            setFontSize(aSmallFont);
            setFontSize(pImpl->maCJKFont);

            long nStartBracketWidth = 0;
            long nEndBracketWidth = 0;
            long nTextWidth = 0;
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
            long nResultWidth = nStartBracketWidth;
            nResultWidth += nEndBracketWidth;
            nResultWidth += nTextWidth;

            long _nX = (aLogSize.Width() - nResultWidth) / 2;
            rRenderContext.DrawLine(Point(0,  nY), Point(_nX, nY));
            rRenderContext.DrawLine(Point(_nX + nResultWidth, nY), Point(aLogSize.Width(), nY));

            long nSmallAscent = pImpl->mnAscent;
            long nOffset = (nStdAscent - nSmallAscent) / 2;

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
            pImpl->maCJKFont.SetSize(aOldSize);
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
    Color   aFontColor( GetTextColor() );

    if (COL_AUTO == pImpl->maFont.GetColor().GetColor())
        pImpl->maFont.SetColor(aFontColor);

    if (COL_AUTO == pImpl->maCJKFont.GetColor().GetColor())
        pImpl->maCJKFont.SetColor(aFontColor);

    if (COL_AUTO == pImpl->maCTLFont.GetColor().GetColor())
        pImpl->maCTLFont.SetColor(aFontColor);
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
        rFont.SetName(rFontItem.GetFamilyName());
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
        rFont.SetItalic( ( FontItalic ) rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    if( GetWhich( rSet, nWeight, nWhich ) )
    {
        const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>( rSet.Get( nWhich ) );
        rFont.SetWeight( ( FontWeight ) rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

void SvxFontPrevWindow::SetFontSize( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    long nH;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        nH = LogicToLogic(static_cast<const SvxFontHeightItem&>(rSet.Get(nWhich)).GetHeight(),
                            (MapUnit) rSet.GetPool()->GetMetric(nWhich),
                            MAP_TWIP);
    }
    else
        nH = 240;// as default 12pt

    rFont.SetSize(Size(0, nH));
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

static void SetPrevFontEscapement(SvxFont& rFont, sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc)
{
    rFont.SetPropr(nProp);
    rFont.SetProprRel(nEscProp);
    rFont.SetEscapement(nEsc);
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
        OUString aString = rItem.GetValue();
        if( !aString.isEmpty() )
            SetPreviewText( aString );
        else
            SetFontNameAsPreviewText();
    }

    // Underline
    FontUnderline eUnderline;
    if( GetWhich( rSet, SID_ATTR_CHAR_UNDERLINE, nWhich ) )
    {
        const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>( rSet.Get( nWhich ) );
        eUnderline = ( FontUnderline ) rItem.GetValue();
        SetTextLineColor( rItem.GetColor() );
    }
    else
        eUnderline = UNDERLINE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    // Overline
    FontUnderline eOverline;
    if( GetWhich( rSet, SID_ATTR_CHAR_OVERLINE, nWhich ) )
    {
        const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>( rSet.Get( nWhich ) );
        eOverline = ( FontUnderline ) rItem.GetValue();
        SetOverlineColor( rItem.GetColor() );
    }
    else
        eOverline = UNDERLINE_NONE;

    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );

    //  Strikeout
    FontStrikeout eStrikeout;
    if( GetWhich( rSet, SID_ATTR_CHAR_STRIKEOUT, nWhich ) )
    {
        const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>( rSet.Get( nWhich ) );
        eStrikeout = ( FontStrikeout ) rItem.GetValue();
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
        FontRelief eFontRelief = ( FontRelief ) rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    if( GetWhich( rSet, SID_ATTR_CHAR_CASEMAP, nWhich ) )
    {
        const SvxCaseMapItem& rItem = static_cast<const SvxCaseMapItem&>( rSet.Get( nWhich ) );
        SvxCaseMap eCaseMap = ( SvxCaseMap ) rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaseMap == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : eCaseMap );
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
         bTransparent = rColor.GetTransparency() > 0;
         rFont.SetFillColor( rColor );
         rCJKFont.SetFillColor( rColor );
         rCTLFont.SetFillColor( rColor );
    }
    else
        bTransparent = TRUE;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    Color aBackCol( COL_TRANSPARENT );
    if( !bPreviewBackgroundToCharacter )
    {
        if( GetWhich( rSet, SID_ATTR_BRUSH, nWhich ) )
        {
            const SvxBrushItem& rBrush = static_cast<const  SvxBrushItem&>( rSet.Get( nWhich ) );
            if( GPOS_NONE == rBrush.GetGraphicPos() )
                aBackCol = rBrush.GetColor();
        }
    }
    SetBackColor( aBackCol );

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
        short nKern = ( short )
                        LogicToLogic( rItem.GetValue(), ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ), MAP_TWIP );
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
        nEscProp = rItem.GetProp();

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

void SvxFontPrevWindow::Init(const SfxItemSet& rSet)
{
    SvxFont& rFont = GetFont();
    SvxFont& rCJKFont = GetCJKFont();
    SvxFont& rCTLFont = GetCTLFont();

    initFont(rFont);
    initFont(rCJKFont);
    initFont(rCTLFont);
    ResetSettings(true, true);

    sal_uInt16 nWhich;
    nWhich = rSet.GetPool()->GetWhich( SID_CHAR_DLG_PREVIEW_STRING );
    if (ISITEMSET)
    {
        const SfxStringItem& rItem = static_cast<const SfxStringItem&>( rSet.Get( nWhich ) );
        OUString aString = rItem.GetValue();
        if( !aString.isEmpty() )
            SetPreviewText( aString );
        else
            SetFontNameAsPreviewText();
    }

    // Underline
    FontUnderline eUnderline;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if( ISITEMSET )
    {
        const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>( rSet.Get( nWhich ) );
        eUnderline = ( FontUnderline ) rItem.GetValue();
        SetTextLineColor( rItem.GetColor() );
    }
    else
        eUnderline = UNDERLINE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    // Overline
    FontUnderline eOverline;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_OVERLINE );
    if( ISITEMSET )
    {
        const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>( rSet.Get( nWhich ) );
        eOverline = ( FontUnderline ) rItem.GetValue();
        SetOverlineColor( rItem.GetColor() );
    }
    else
        eOverline = UNDERLINE_NONE;

    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );

    //  Strikeout
    FontStrikeout eStrikeout;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if( ISITEMSET )
    {
        const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>( rSet.Get( nWhich ) );
        eStrikeout = ( FontStrikeout ) rItem.GetValue();
    }
    else
        eStrikeout = STRIKEOUT_NONE;

    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    // WordLineMode
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if( ISITEMSET )
    {
        const SvxWordLineModeItem& rItem = static_cast<const SvxWordLineModeItem&>( rSet.Get( nWhich ) );
        rFont.SetWordLineMode( rItem.GetValue() );
        rCJKFont.SetWordLineMode( rItem.GetValue() );
        rCTLFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if( ISITEMSET )
    {
        const SvxEmphasisMarkItem& rItem = static_cast<const SvxEmphasisMarkItem&>( rSet.Get( nWhich ) );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    // Relief
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_RELIEF );
    if( ISITEMSET )
    {
        const SvxCharReliefItem& rItem = static_cast<const SvxCharReliefItem&>( rSet.Get( nWhich ) );
        FontRelief eFontRelief = ( FontRelief ) rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_CASEMAP );
    if( ISITEMSET )
    {
        const SvxCaseMapItem& rItem = static_cast<const SvxCaseMapItem&>( rSet.Get( nWhich ) );
        SvxCaseMap eCaseMap = ( SvxCaseMap ) rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaseMap == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : eCaseMap );
    }

    // Outline
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_CONTOUR );
    if( ISITEMSET )
    {
        const SvxContourItem& rItem = static_cast<const SvxContourItem&>( rSet.Get( nWhich ) );
        bool bOutline = rItem.GetValue();
        rFont.SetOutline( bOutline );
        rCJKFont.SetOutline( bOutline );
        rCTLFont.SetOutline( bOutline );
    }

    // Shadow
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_SHADOWED );
    if( ISITEMSET )
    {
        const SvxShadowedItem& rItem = static_cast<const SvxShadowedItem&>( rSet.Get( nWhich ) );
        bool bShadow = rItem.GetValue();
        rFont.SetShadow( bShadow );
        rCJKFont.SetShadow( bShadow );
        rCTLFont.SetShadow( bShadow );
    }

    // Background
    bool bTransparent;
    nWhich = rSet.GetPool()->GetWhich(pImpl->mbPreviewBackgroundToCharacter ? SID_ATTR_BRUSH : SID_ATTR_BRUSH_CHAR);
    if (ISITEMSET)
    {
         const SvxBrushItem& rBrush = static_cast<const SvxBrushItem&>( rSet.Get( nWhich ) );
         const Color& rColor = rBrush.GetColor();
         bTransparent = rColor.GetTransparency() > 0;
         rFont.SetFillColor(rColor);
         rCJKFont.SetFillColor(rColor);
         rCTLFont.SetFillColor(rColor);
    }
    else
        bTransparent = true;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    Color aBackCol( COL_TRANSPARENT );
    if (!pImpl->mbPreviewBackgroundToCharacter)
    {
        nWhich = rSet.GetPool()->GetWhich( SID_ATTR_BRUSH );
        if (ISITEMSET)
        {
            const SvxBrushItem& rBrush = static_cast<const SvxBrushItem&>(rSet.Get(nWhich));
            if (GPOS_NONE == rBrush.GetGraphicPos())
                aBackCol = rBrush.GetColor();
        }
    }
    SetBackColor(aBackCol);

    // Font
    SetFont(rSet, SID_ATTR_CHAR_FONT, rFont);
    SetFont(rSet, SID_ATTR_CHAR_CJK_FONT, rCJKFont);
    SetFont(rSet, SID_ATTR_CHAR_CTL_FONT, rCTLFont);

    // Style
    SetFontStyle(rSet, SID_ATTR_CHAR_POSTURE, SID_ATTR_CHAR_WEIGHT, rFont);
    SetFontStyle(rSet, SID_ATTR_CHAR_CJK_POSTURE, SID_ATTR_CHAR_CJK_WEIGHT, rCJKFont);
    SetFontStyle(rSet, SID_ATTR_CHAR_CTL_POSTURE, SID_ATTR_CHAR_CTL_WEIGHT, rCTLFont);

    // Size
    SetFontSize(rSet, SID_ATTR_CHAR_FONTHEIGHT, rFont);
    SetFontSize(rSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, rCJKFont);
    SetFontSize(rSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, rCTLFont);

    // Language
    SetFontLang( rSet, SID_ATTR_CHAR_LANGUAGE, rFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CJK_LANGUAGE, rCJKFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CTL_LANGUAGE, rCTLFont );

    // Color
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_COLOR );
    if( ISITEMSET )
    {
        const SvxColorItem& rItem = static_cast<const SvxColorItem&>( rSet.Get( nWhich ) );
        Color aCol( rItem.GetValue() );
        rFont.SetColor( aCol );
        rCJKFont.SetColor( aCol );
        rCTLFont.SetColor( aCol );

        AutoCorrectFontColor(); // handle color COL_AUTO
    }

    // Kerning
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_KERNING );
    if( ISITEMSET )
    {
        const SvxKerningItem& rItem = static_cast<const SvxKerningItem&>( rSet.Get( nWhich ) );
        short nKern = ( short )
                        LogicToLogic( rItem.GetValue(), ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ), MAP_TWIP );
        rFont.SetFixKerning( nKern );
        rCJKFont.SetFixKerning( nKern );
        rCTLFont.SetFixKerning( nKern );
    }

    // Escapement
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const sal_uInt8 nProp = 100;
    short nEsc;
    sal_uInt8 nEscProp;
    if( ISITEMSET )
    {
        const SvxEscapementItem& rItem = static_cast<const SvxEscapementItem&>( rSet.Get( nWhich ) );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

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

    SetFontEscapement( nProp, nEscProp, nEsc );

    // Font width scale
    SetFontWidthScale( rSet );

    Invalidate();
}

void SvxFontPrevWindow::SetFont( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich = rSet.GetPool()->GetWhich( nSlot );
    if( ISITEMSET )
    {
        const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>( rSet.Get(nWhich) );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        rFont.SetStyleName( rFontItem.GetStyleName() );
    }
}

void SvxFontPrevWindow::SetFontStyle( const SfxItemSet& rSet, sal_uInt16 nPosture, sal_uInt16 nWeight, SvxFont& rFont )
{
    sal_uInt16 nWhich = rSet.GetPool()->GetWhich( nPosture );
    if( ISITEMSET )
    {
        const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>( rSet.Get( nWhich ) );
        rFont.SetItalic( ( FontItalic ) rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    nWhich = rSet.GetPool()->GetWhich( nWeight );
    if( ISITEMSET )
    {
        const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>( rSet.Get( nWhich ) );
        rFont.SetWeight( ( FontWeight ) rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

void SvxFontPrevWindow::SetFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16  nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if( ISITEMSET )
    {
        const SvxCharScaleWidthItem&    rItem = static_cast<const SvxCharScaleWidthItem&>( rSet.Get( nWhich ) );

        SetFontWidthScale( rItem.GetValue() );
    }
}

namespace
{
    void setFontEscapement(SvxFont& rFont, sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc)
    {
        rFont.SetPropr(nProp);
        rFont.SetProprRel(nEscProp);
        rFont.SetEscapement(nEsc);
    }
}

void SvxFontPrevWindow::SetFontEscapement(sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc)
{
    setFontEscapement(GetFont(), nProp, nEscProp, nEsc);
    setFontEscapement(GetCJKFont(), nProp, nEscProp, nEsc);
    setFontEscapement(GetCTLFont(), nProp, nEscProp, nEsc);
    Invalidate();
}

Size SvxFontPrevWindow::GetOptimalSize() const
{
    return getPreviewStripSize(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
