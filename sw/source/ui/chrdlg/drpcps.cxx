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

#include <hintids.hxx>

#include "cmdid.h"
#include <docsh.hxx>
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"

#include <vcl/metric.hxx>
#include <vcl/settings.hxx>

#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/svxfont.hxx>
#include <vcl/print.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/scripttypeitem.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>

#include "charatr.hxx"
#include "viewopt.hxx"
#include "drpcps.hxx"
#include "paratr.hxx"
#include "uitool.hxx"
#include "charfmt.hxx"

#include "chrdlg.hrc"

using namespace css;
using namespace css::uno;
using namespace css::lang;

const sal_uInt16 SwDropCapsPage::aPageRg[] = {
    RES_PARATR_DROP, RES_PARATR_DROP,
    0
};

class SwDropCapsPict : public Control
{
    VclPtr<SwDropCapsPage> mpPage;
    OUString        maText;
    OUString        maScriptText;
    Color           maBackColor;
    Color           maTextLineColor;
    sal_uInt8       mnLines;
    long            mnTotLineH;
    long            mnLineH;
    long            mnTextH;
    sal_uInt16      mnDistance;
    VclPtr<Printer> mpPrinter;
    bool            mbDelPrinter;
    /// The _ScriptInfo structure holds information on where we change from one
    /// script to another.
    struct _ScriptInfo
    {
        sal_uLong  textWidth;   ///< Physical width of this segment.
        sal_uInt16 scriptType;  ///< Script type (e.g. Latin, Asian, Complex)
        sal_Int32 changePos;   ///< Character position where the script changes.
        _ScriptInfo(sal_uLong txtWidth, sal_uInt16 scrptType, sal_Int32 position)
            : textWidth(txtWidth), scriptType(scrptType), changePos(position) {}
    };
    std::vector<_ScriptInfo> maScriptChanges;
    SvxFont         maFont;
    SvxFont         maCJKFont;
    SvxFont         maCTLFont;
    Size            maTextSize;
    Reference< css::i18n::XBreakIterator >   xBreak;

    virtual void    Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle &rRect) override;
    void            CheckScript();
    Size            CalcTextSize();
    inline void     InitPrinter();
    void            _InitPrinter();
    static void     GetFontSettings( const SwDropCapsPage& _rPage, vcl::Font& _rFont, sal_uInt16 _nWhich );
    void            GetFirstScriptSegment(sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType);
    bool            GetNextScriptSegment(size_t &nIdx, sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType);

public:

    SwDropCapsPict(vcl::Window *pParent, WinBits nBits)
        : Control(pParent, nBits)
        , mpPage(nullptr)
        , mnLines(0)
        , mnTotLineH(0)
        , mnLineH(0)
        , mnTextH(0)
        , mnDistance(0)
        , mpPrinter(nullptr)
        , mbDelPrinter(false)
    {}

    void SetDropCapsPage(SwDropCapsPage* pPage) { mpPage = pPage; }

    virtual ~SwDropCapsPict();
    virtual void dispose() override;

    void UpdatePaintSettings();       // also invalidates control!

    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    void SetText( const OUString& rT ) override;
    void SetLines( sal_uInt8 nL );
    void SetDistance( sal_uInt16 nD );
    void SetValues( const OUString& rText, sal_uInt8 nLines, sal_uInt16 nDistance );

    void DrawPrev(vcl::RenderContext& rRenderContext, const Point& rPt);
};

VCL_BUILDER_FACTORY_ARGS(SwDropCapsPict, WB_BORDER)

void SwDropCapsPict::SetText( const OUString& rT )
{
    maText = rT;
    UpdatePaintSettings();
}

Size SwDropCapsPict::GetOptimalSize() const
{
    return getParagraphPreviewOptimalSize(this);
}

void SwDropCapsPict::Resize()
{
    Control::Resize();
    UpdatePaintSettings();
}

void SwDropCapsPict::SetLines( sal_uInt8 nL )
{
    mnLines = nL;
    UpdatePaintSettings();
}

void SwDropCapsPict::SetDistance( sal_uInt16 nD )
{
    mnDistance = nD;
    UpdatePaintSettings();
}

void SwDropCapsPict::SetValues( const OUString& rText, sal_uInt8 nLines, sal_uInt16 nDistance )
{
    maText = rText;
    mnLines = nLines;
    mnDistance = nDistance;

    UpdatePaintSettings();
}

void SwDropCapsPict::InitPrinter()
{
    if( !mpPrinter )
        _InitPrinter();
}

// Create Default-String from character-count (A, AB, ABC, ...)
OUString GetDefaultString(sal_Int32 nChars)
{
    OUString aStr;
    for (sal_Int32 i = 0; i < nChars; i++)
        aStr += OUString((char) (i + 65));
    return aStr;
}

static void calcFontHeightAnyAscent( vcl::RenderContext* _pWin, vcl::Font& _rFont, long& _nHeight, long& _nAscent )
{
    if ( !_nHeight )
    {
        _pWin->SetFont( _rFont );
        FontMetric aMetric( _pWin->GetFontMetric() );
        _nHeight = aMetric.GetLineHeight();
        _nAscent = aMetric.GetAscent();
    }
}

SwDropCapsPict::~SwDropCapsPict()
{
    disposeOnce();
}

void SwDropCapsPict::dispose()
{
     if( mbDelPrinter )
         mpPrinter.disposeAndClear();
     mpPage.clear();
     Control::dispose();
}

/// Get the details of the first script change.
/// @param[out] start      The character position of the start of the segment.
/// @param[out] end        The character position of the end of the segment.
/// @param[out] scriptType The script type (Latin, Asian, Complex etc.)
void SwDropCapsPict::GetFirstScriptSegment(sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType)
{
    start = 0;
    if( maScriptChanges.empty() )
    {
        end = maText.getLength();
        scriptType = css::i18n::ScriptType::LATIN;
    }
    else
    {
        end = maScriptChanges[ 0 ].changePos;
        scriptType = maScriptChanges[ 0 ].scriptType;
    }
}

/// Get the details of the first script change.
/// @param[in,out] nIdx       Index of the current script change.
/// @param[out]    start      The character position of the start of the segment.
/// @param[in,out] end        The character position of the end of the segment.
/// @param[out]    scriptType The script type (Latin, Asian, Complex etc.)
/// @returns True if there was a next segment, false if not.
bool SwDropCapsPict::GetNextScriptSegment(size_t &nIdx, sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType)
{
    if (maScriptChanges.empty() || nIdx >= maScriptChanges.size() - 1 || end >= maText.getLength())
        return false;
    start = maScriptChanges[nIdx++].changePos;
    end = maScriptChanges[ nIdx ].changePos;
    scriptType = maScriptChanges[ nIdx ].scriptType;
    return true;
}

#define LINES  10
#define BORDER  2

void SwDropCapsPict::GetFontSettings( const SwDropCapsPage& _rPage, vcl::Font& _rFont, sal_uInt16 _nWhich )
{
    SfxItemSet aSet( _rPage.rSh.GetAttrPool(), _nWhich, _nWhich);
    _rPage.rSh.GetCurAttr(aSet);
    SvxFontItem aFormatFont(static_cast<const SvxFontItem &>( aSet.Get(_nWhich)));

    _rFont.SetFamily (aFormatFont.GetFamily());
    _rFont.SetName   (aFormatFont.GetFamilyName());
    _rFont.SetPitch  (aFormatFont.GetPitch());
    _rFont.SetCharSet(aFormatFont.GetCharSet());
}

void SwDropCapsPict::UpdatePaintSettings()
{
    maBackColor = GetSettings().GetStyleSettings().GetWindowColor();
    maTextLineColor = Color( COL_LIGHTGRAY );

    // gray lines
    mnTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES;
    mnLineH = mnTotLineH - 2;

    vcl::Font aFont;
    if (mpPage)
    {
        if (!mpPage->m_pTemplateBox->GetSelectEntryPos())
        {
            // query the Font at paragraph's beginning
            mpPage->rSh.Push();
            mpPage->rSh.SttCrsrMove();
            mpPage->rSh.ClearMark();
            SwWhichPara pSwuifnParaCurr = GetfnParaCurr();
            SwPosPara pSwuifnParaStart = GetfnParaStart();
            mpPage->rSh.MovePara(pSwuifnParaCurr,pSwuifnParaStart);
            // normal
            GetFontSettings( *mpPage, aFont, RES_CHRATR_FONT );

            // CJK
            GetFontSettings( *mpPage, maCJKFont, RES_CHRATR_CJK_FONT );

            // CTL
            GetFontSettings( *mpPage, maCTLFont, RES_CHRATR_CTL_FONT );

            mpPage->rSh.EndCrsrMove();
            mpPage->rSh.Pop(false);
        }
        else
        {
            // query Font at character template
            SwCharFormat *pFormat = mpPage->rSh.GetCharStyle(
                                    mpPage->m_pTemplateBox->GetSelectEntry(),
                                    SwWrtShell::GETSTYLE_CREATEANY );
            OSL_ENSURE(pFormat, "character style doesn't exist!");
            const SvxFontItem &rFormatFont = pFormat->GetFont();

            aFont.SetFamily (rFormatFont.GetFamily());
            aFont.SetName   (rFormatFont.GetFamilyName());
            aFont.SetPitch  (rFormatFont.GetPitch());
            aFont.SetCharSet(rFormatFont.GetCharSet());
        }
    }

    mnTextH = mnLines * mnTotLineH;
    aFont.SetSize(Size(0, mnTextH));
    maCJKFont.SetSize(Size(0, mnTextH));
    maCTLFont.SetSize(Size(0, mnTextH));

    aFont.SetTransparent(true);
    maCJKFont.SetTransparent(true);
    maCTLFont.SetTransparent(true);

    aFont.SetColor( SwViewOption::GetFontColor() );
    maCJKFont.SetColor( SwViewOption::GetFontColor() );
    maCTLFont.SetColor( SwViewOption::GetFontColor() );

    aFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCJKFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCTLFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());

    maCJKFont.SetSize(Size(0, maCJKFont.GetSize().Height()));
    maCTLFont.SetSize(Size(0, maCTLFont.GetSize().Height()));

    SetFont(aFont);
    aFont.SetSize(Size(0, aFont.GetSize().Height()));
    SetFont(aFont);
    maFont = aFont;

    CheckScript();

    maTextSize = CalcTextSize();

    Invalidate();
}

void SwDropCapsPict::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    if (!IsVisible())
        return;

    rRenderContext.SetMapMode(MapMode(MAP_PIXEL));
    rRenderContext.SetLineColor();

    rRenderContext.SetFillColor(maBackColor);

    Size aOutputSizePixel(GetOutputSizePixel());

    rRenderContext.DrawRect(Rectangle(Point(0, 0), aOutputSizePixel));
    rRenderContext.SetClipRegion(vcl::Region(Rectangle(Point(BORDER, BORDER),
                                                       Size(aOutputSizePixel.Width () - 2 * BORDER,
                                                            aOutputSizePixel.Height() - 2 * BORDER))));

    OSL_ENSURE(mnLineH > 0, "We cannot make it that small");
    long nY0 = (aOutputSizePixel.Height() - (LINES * mnTotLineH)) / 2;

    rRenderContext.SetFillColor(maTextLineColor);

    for (int i = 0; i < LINES; ++i)
    {
        rRenderContext.DrawRect(Rectangle(Point(BORDER, nY0 + i * mnTotLineH),
                                Size(aOutputSizePixel.Width() - 2 * BORDER, mnLineH)));
    }

    // Text background with gap (240 twips ~ 1 line height)
    const long nDistW = (((static_cast<long>(mnDistance) * 100) / 240) * mnTotLineH) / 100;
    rRenderContext.SetFillColor(maBackColor);
    if (mpPage && mpPage->m_pDropCapsBox->IsChecked())
    {
        const Size aTextSize(maTextSize.Width() + nDistW, maTextSize.Height());
        rRenderContext.DrawRect(Rectangle(Point(BORDER, nY0), aTextSize));

        // draw Text
        DrawPrev(rRenderContext, Point(BORDER, nY0));
    }
    rRenderContext.SetClipRegion();
}

void SwDropCapsPict::DrawPrev(vcl::RenderContext& rRenderContext, const Point& rPt)
{
    Point aPt(rPt);
    InitPrinter();

    vcl::Font aOldFont = mpPrinter->GetFont();
    sal_uInt16 nScript;
    size_t nIdx = 0;
    sal_Int32 nStart;
    sal_Int32 nEnd;

    GetFirstScriptSegment(nStart, nEnd, nScript);

    do
    {
        SvxFont& rFnt = (nScript == css::i18n::ScriptType::ASIAN)
                            ? maCJKFont
                            : ((nScript == css::i18n::ScriptType::COMPLEX)
                                    ? maCTLFont
                                    : maFont);
        mpPrinter->SetFont(rFnt);

        rFnt.DrawPrev(&rRenderContext, mpPrinter, aPt, maText, nStart, nEnd - nStart);

        if (!maScriptChanges.empty())
            aPt.X() += maScriptChanges[nIdx].textWidth;

        if (!GetNextScriptSegment(nIdx, nStart, nEnd, nScript))
            break;
    }
    while(true);

    mpPrinter->SetFont(aOldFont);
}

void SwDropCapsPict::CheckScript()
{
    if( maScriptText == maText )
        return;

    maScriptText = maText;
    maScriptChanges.clear();
    if( !xBreak.is() )
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        xBreak = css::i18n::BreakIterator::create(xContext);
    }
    sal_Int16 nScript = xBreak->getScriptType( maText, 0 );
    sal_Int32 nChg = 0;
    if( css::i18n::ScriptType::WEAK == nScript )
    {
        nChg = xBreak->endOfScript( maText, nChg, nScript );
        if( nChg < maText.getLength() )
            nScript = xBreak->getScriptType( maText, nChg );
        else
            nScript = css::i18n::ScriptType::LATIN;
    }

    for(;;)
    {
        nChg = xBreak->endOfScript( maText, nChg, nScript );
        maScriptChanges.push_back( _ScriptInfo(0, nScript, nChg) );
        if( nChg >= maText.getLength() || nChg < 0 )
            break;
        nScript = xBreak->getScriptType( maText, nChg );
    }
}

Size SwDropCapsPict::CalcTextSize()
{
    InitPrinter();

    sal_uInt16 nScript;
    size_t nIdx = 0;
    sal_Int32 nStart;
    sal_Int32 nEnd;
    GetFirstScriptSegment(nStart, nEnd, nScript);
    long nTextWidth = 0;
    long nCJKHeight = 0;
    long nCTLHeight = 0;
    long nHeight = 0;
    long nAscent = 0;
    long nCJKAscent = 0;
    long nCTLAscent = 0;
    do
    {
        SvxFont& rFnt = (nScript == css::i18n::ScriptType::ASIAN)
                            ? maCJKFont
                            : ((nScript == css::i18n::ScriptType::COMPLEX)
                                    ? maCTLFont
                                    : maFont);

        sal_uLong nWidth = rFnt.GetTextSize(mpPrinter, maText, nStart, nEnd-nStart ).Width();

        if (nIdx < maScriptChanges.size())
            maScriptChanges[nIdx].textWidth = nWidth;
        nTextWidth += nWidth;
        switch(nScript)
        {
            case css::i18n::ScriptType::ASIAN:
                calcFontHeightAnyAscent(this, maCJKFont, nCJKHeight, nCJKAscent);
                break;
            case css::i18n::ScriptType::COMPLEX:
                calcFontHeightAnyAscent(this, maCTLFont, nCTLHeight, nCTLAscent);
                break;
            default:
                calcFontHeightAnyAscent(this, maFont, nHeight, nAscent);
        }

        if (!GetNextScriptSegment(nIdx, nStart, nEnd, nScript))
            break;
    }
    while(true);

    nHeight -= nAscent;
    nCJKHeight -= nCJKAscent;
    nCTLHeight -= nCTLAscent;
    if (nHeight < nCJKHeight)
        nHeight = nCJKHeight;
    if (nAscent < nCJKAscent)
        nAscent = nCJKAscent;
    if (nHeight < nCTLHeight)
        nHeight = nCTLHeight;
    if (nAscent < nCTLAscent)
        nAscent = nCTLAscent;
    nHeight += nAscent;

    Size aTextSize(nTextWidth, nHeight);
    return aTextSize;
}

void SwDropCapsPict::_InitPrinter()
{
    SfxViewShell* pSh = SfxViewShell::Current();

    if (pSh)
        mpPrinter = pSh->GetPrinter();

    if (!mpPrinter)
    {
        mpPrinter = VclPtr<Printer>::Create();
        mbDelPrinter = true;
    }
}

SwDropCapsDlg::SwDropCapsDlg(vcl::Window *pParent, const SfxItemSet &rSet )
    : SfxSingleTabDialog(pParent, rSet)
{
    VclPtr<SwDropCapsPage> pNewPage( static_cast<SwDropCapsPage*>( SwDropCapsPage::Create(get_content_area(), &rSet).get() ) );
    pNewPage->SetFormat(false);
    SetTabPage(pNewPage);
}

SwDropCapsPage::SwDropCapsPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "DropCapPage","modules/swriter/ui/dropcapspage.ui", &rSet)
    , bModified(false)
    , bFormat(true)
    , rSh(::GetActiveView()->GetWrtShell())
{
    get(m_pDropCapsBox,"checkCB_SWITCH");
    get(m_pWholeWordCB,"checkCB_WORD");
    get(m_pDropCapsField,"spinFLD_DROPCAPS");
    get(m_pLinesField,"spinFLD_LINES");
    get(m_pDistanceField,"spinFLD_DISTANCE");
    get(m_pSwitchText,"labelFT_DROPCAPS");
    get(m_pLinesText,"labelTXT_LINES");
    get(m_pDistanceText,"labelTXT_DISTANCE");
    get(m_pTemplateText,"labelTXT_TEMPLATE");
    get(m_pTextText,"labelTXT_TEXT");
    get(m_pTextEdit,"entryEDT_TEXT");
    get(m_pTemplateBox,"comboBOX_TEMPLATE");
    get(m_pPict,"drawingareaWN_EXAMPLE");

    m_pPict->SetDropCapsPage(this);

    SetExchangeSupport();

    const sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    bHtmlMode = (nHtmlMode & HTMLMODE_ON) != 0;

    // In the template dialog the text is not influenceable
    m_pTextText->Enable( !bFormat );
    m_pTextEdit->Enable( !bFormat );

    // Metrics
    SetMetric( *m_pDistanceField, GetDfltMetric(bHtmlMode) );

    m_pPict->SetBorderStyle( WindowBorderStyle::MONO );

    // Install handler
    Link<Edit&,void> aLk = LINK(this, SwDropCapsPage, ModifyHdl);
    m_pDropCapsField->SetModifyHdl( aLk );
    m_pLinesField->SetModifyHdl( aLk );
    m_pDistanceField->SetModifyHdl( aLk );
    m_pTextEdit->SetModifyHdl( aLk );
    m_pDropCapsBox->SetClickHdl (LINK(this, SwDropCapsPage, ClickHdl ));
    m_pTemplateBox->SetSelectHdl(LINK(this, SwDropCapsPage, SelectHdl));
    m_pWholeWordCB->SetClickHdl (LINK(this, SwDropCapsPage, WholeWordHdl ));

    setPreviewsToSamePlace(pParent, this);
}

SwDropCapsPage::~SwDropCapsPage()
{
    disposeOnce();
}

void SwDropCapsPage::dispose()
{
    m_pDropCapsBox.clear();
    m_pWholeWordCB.clear();
    m_pSwitchText.clear();
    m_pDropCapsField.clear();
    m_pLinesText.clear();
    m_pLinesField.clear();
    m_pDistanceText.clear();
    m_pDistanceField.clear();
    m_pTextText.clear();
    m_pTextEdit.clear();
    m_pTemplateText.clear();
    m_pTemplateBox.clear();
    m_pPict.clear();
    SfxTabPage::dispose();
}

SfxTabPage::sfxpg SwDropCapsPage::DeactivatePage(SfxItemSet * _pSet)
{
    if (_pSet)
        FillSet(*_pSet);

    return LEAVE_PAGE;
}

VclPtr<SfxTabPage> SwDropCapsPage::Create(vcl::Window *pParent,
                                          const SfxItemSet *rSet)
{
    return VclPtr<SwDropCapsPage>::Create(pParent, *rSet);
}

bool  SwDropCapsPage::FillItemSet(SfxItemSet *rSet)
{
    if (bModified)
        FillSet(*rSet);
    return bModified;
}

void  SwDropCapsPage::Reset(const SfxItemSet *rSet)
{
    // Characters, lines, gap and text
    SwFormatDrop aFormatDrop(static_cast<const SwFormatDrop &>( rSet->Get(RES_PARATR_DROP)));
    if (aFormatDrop.GetLines() > 1)
    {
        m_pDropCapsField->SetValue(aFormatDrop.GetChars());
        m_pLinesField->SetValue(aFormatDrop.GetLines());
        m_pDistanceField->SetValue(m_pDistanceField->Normalize(aFormatDrop.GetDistance()), FUNIT_TWIP);
        m_pWholeWordCB->Check(aFormatDrop.GetWholeWord());
    }
    else
    {
        m_pDropCapsField->SetValue(1);
        m_pLinesField->SetValue(3);
        m_pDistanceField->SetValue(0);
    }

    ::FillCharStyleListBox(*m_pTemplateBox, rSh.GetView().GetDocShell(), true);

    m_pTemplateBox->InsertEntry(SW_RESSTR(SW_STR_NONE), 0);

    // Reset format
    m_pTemplateBox->SelectEntryPos(0);
    if (aFormatDrop.GetCharFormat())
        m_pTemplateBox->SelectEntry(aFormatDrop.GetCharFormat()->GetName());

    // Enable controls
    m_pDropCapsBox->Check(aFormatDrop.GetLines() > 1);
    const sal_Int32 nVal = static_cast<sal_Int32>(m_pDropCapsField->GetValue());
    if (bFormat)
        m_pTextEdit->SetText(GetDefaultString(nVal));
    else
    {
        m_pTextEdit->SetText(rSh.GetDropText(nVal));
        m_pTextEdit->Enable();
        m_pTextText->Enable();
    }

    // Preview
    m_pPict->SetValues(m_pTextEdit->GetText(),
                       sal_uInt8(m_pLinesField->GetValue()),
                       sal_uInt16(m_pDistanceField->Denormalize(m_pDistanceField->GetValue(FUNIT_TWIP))));

    ClickHdl(m_pDropCapsBox);
    bModified = false;
}

IMPL_LINK_NOARG_TYPED(SwDropCapsPage, ClickHdl, Button*, void)
{
    bool bChecked = m_pDropCapsBox->IsChecked();

    m_pWholeWordCB->Enable( bChecked && !bHtmlMode );

    m_pSwitchText->Enable( bChecked && !m_pWholeWordCB->IsChecked() );
    m_pDropCapsField->Enable( bChecked && !m_pWholeWordCB->IsChecked() );
    m_pLinesText->Enable( bChecked );
    m_pLinesField->Enable( bChecked );
    m_pDistanceText->Enable( bChecked );
    m_pDistanceField->Enable( bChecked );
    m_pTemplateText->Enable( bChecked );
    m_pTemplateBox->Enable( bChecked );
    m_pTextEdit->Enable( bChecked && !bFormat );
    m_pTextText->Enable( bChecked && !bFormat );

    if ( bChecked )
    {
        ModifyHdl(*m_pDropCapsField);
        m_pDropCapsField->GrabFocus();
    }
    else
        m_pPict->SetText("");

    bModified = true;
}

IMPL_LINK_NOARG_TYPED(SwDropCapsPage, WholeWordHdl, Button*, void)
{
    m_pDropCapsField->Enable( !m_pWholeWordCB->IsChecked() );
    m_pSwitchText->Enable(!m_pWholeWordCB->IsChecked());

    ModifyHdl(*m_pDropCapsField);

    bModified = true;
}

IMPL_LINK_TYPED( SwDropCapsPage, ModifyHdl, Edit&, rEdit, void )
{
    OUString sPreview;

    // set text if applicable
    if (&rEdit == m_pDropCapsField)
    {
        const sal_Int32 nVal = !m_pWholeWordCB->IsChecked()
            ? static_cast<sal_Int32>(m_pDropCapsField->GetValue())
            : 0;
        bool bSetText = false;

        if (bFormat || rSh.GetDropText(1).isEmpty())
            sPreview = GetDefaultString(nVal);
        else
        {
            bSetText = true;
            sPreview = rSh.GetDropText(nVal);
        }

        OUString sEdit(m_pTextEdit->GetText());

        if (!sEdit.isEmpty() && !sPreview.startsWith(sEdit))
        {
            sPreview = sEdit.copy(0, std::min(sEdit.getLength(), sPreview.getLength()));
            bSetText = false;
        }

        if (bSetText)
            m_pTextEdit->SetText(sPreview);
    }
    else if (&rEdit == m_pTextEdit)   // set quantity if applicable
    {
        const sal_Int32 nTmp = m_pTextEdit->GetText().getLength();
        m_pDropCapsField->SetValue(std::max<sal_Int32>(1, nTmp));
        sPreview = m_pTextEdit->GetText();
    }

    // adjust image
    if (&rEdit == m_pDropCapsField || &rEdit == m_pTextEdit)
        m_pPict->SetText (sPreview);
    else if (&rEdit == m_pLinesField)
        m_pPict->SetLines((sal_uInt8)m_pLinesField->GetValue());
    else
        m_pPict->SetDistance((sal_uInt16)m_pDistanceField->Denormalize(m_pDistanceField->GetValue(FUNIT_TWIP)));

    bModified = true;
}

IMPL_LINK_NOARG_TYPED(SwDropCapsPage, SelectHdl, ListBox&, void)
{
    m_pPict->UpdatePaintSettings();
    bModified = true;
}

void SwDropCapsPage::FillSet( SfxItemSet &rSet )
{
    if(bModified)
    {
        SwFormatDrop aFormat;

        bool bOn = m_pDropCapsBox->IsChecked();
        if(bOn)
        {
            // quantity, lines, gap
            aFormat.GetChars()     = (sal_uInt8) m_pDropCapsField->GetValue();
            aFormat.GetLines()     = (sal_uInt8) m_pLinesField->GetValue();
            aFormat.GetDistance()  = (sal_uInt16) m_pDistanceField->Denormalize(m_pDistanceField->GetValue(FUNIT_TWIP));
            aFormat.GetWholeWord() = m_pWholeWordCB->IsChecked();

            // template
            if (m_pTemplateBox->GetSelectEntryPos())
                aFormat.SetCharFormat(rSh.GetCharStyle(m_pTemplateBox->GetSelectEntry()));
        }
        else
        {
            aFormat.GetChars()    = 1;
            aFormat.GetLines()    = 1;
            aFormat.GetDistance() = 0;
        }

        // set attributes
        const SfxPoolItem* pOldItem;
        if (nullptr == (pOldItem = GetOldItem(rSet, FN_FORMAT_DROPCAPS)) || aFormat != *pOldItem)
            rSet.Put(aFormat);

        // hard text formatting
        // Bug 24974: in designer/template catalog this doesn't make sense!!
        if (!bFormat && m_pDropCapsBox->IsChecked())
        {
            OUString sText(m_pTextEdit->GetText());

            if (!m_pWholeWordCB->IsChecked())
            {
                sText = sText.copy(0, std::min<sal_Int32>(sText.getLength(), m_pDropCapsField->GetValue()));
            }

            SfxStringItem aStr(FN_PARAM_1, sText);
            rSet.Put(aStr);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
