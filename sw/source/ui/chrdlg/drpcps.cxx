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

#include <cmdid.h>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <strings.hrc>

#include <vcl/metric.hxx>
#include <vcl/settings.hxx>

#include <rtl/ustrbuf.hxx>
#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <svtools/unitconv.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include <charatr.hxx>
#include <viewopt.hxx>
#include <drpcps.hxx>
#include <paratr.hxx>
#include <uitool.hxx>
#include <charfmt.hxx>

using namespace css;
using namespace css::uno;

const WhichRangesContainer SwDropCapsPage::s_aPageRg(svl::Items<RES_PARATR_DROP, RES_PARATR_DROP>);

void SwDropCapsPict::SetText( const OUString& rT )
{
    maText = rT;
    UpdatePaintSettings();
}

void SwDropCapsPict::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aPrefSize(getParagraphPreviewOptimalSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aPrefSize.Width(), aPrefSize.Height());
}

void SwDropCapsPict::Resize()
{
    CustomWidgetController::Resize();
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
        InitPrinter_();
}

// Create Default-String from character-count (A, AB, ABC, ...)
static OUString GetDefaultString(sal_Int32 nChars)
{
    OUStringBuffer aStr(nChars);
    for (sal_Int32 i = 0; i < nChars; i++)
        aStr.append(static_cast<sal_Unicode>(i + 65));
    return aStr.makeStringAndClear();
}

static void calcFontHeightAnyAscent(vcl::RenderContext& rWin, vcl::Font const & _rFont, tools::Long& _nHeight, tools::Long& _nAscent)
{
    if ( !_nHeight )
    {
        rWin.Push(vcl::PushFlags::FONT);
        rWin.SetFont(_rFont);
        FontMetric aMetric(rWin.GetFontMetric());
        _nHeight = aMetric.GetLineHeight();
        _nAscent = aMetric.GetAscent();
        rWin.Pop();
    }
}

SwDropCapsPict::~SwDropCapsPict()
{
     if (mbDelPrinter)
         mpPrinter.disposeAndClear();
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

void SwDropCapsPict::GetFontSettings( vcl::Font& _rFont, sal_uInt16 _nWhich )
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rWrtShell = pView->GetWrtShell();

    SfxItemSet aSet( rWrtShell.GetAttrPool(), _nWhich, _nWhich);
    rWrtShell.GetCurAttr(aSet);
    SvxFontItem aFormatFont(static_cast<const SvxFontItem &>( aSet.Get(_nWhich)));

    _rFont.SetFamily(aFormatFont.GetFamily());
    _rFont.SetFamilyName(aFormatFont.GetFamilyName());
    _rFont.SetPitch(aFormatFont.GetPitch());
    _rFont.SetCharSet(aFormatFont.GetCharSet());
}

void SwDropCapsPict::UpdatePaintSettings()
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rWrtShell = pView->GetWrtShell();

    maBackColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    maTextLineColor = COL_LIGHTGRAY;

    // gray lines
    mnTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES;
    mnLineH = mnTotLineH - 2;

    vcl::Font aFont;
    if (mpPage)
    {
        // tdf#135244: preview generation should not jump document view
        auto aLock(rWrtShell.GetView().GetDocShell()->LockAllViews());

        if (!mpPage->m_xTemplateBox->get_active())
        {
            // query the Font at paragraph's beginning
            rWrtShell.Push();
            rWrtShell.SttCursorMove();
            rWrtShell.ClearMark();
            SwWhichPara pSwuifnParaCurr = GoCurrPara;
            SwMoveFnCollection const & pSwuifnParaStart = fnParaStart;
            rWrtShell.MovePara(pSwuifnParaCurr,pSwuifnParaStart);
            // normal
            GetFontSettings( aFont, RES_CHRATR_FONT );

            // CJK
            GetFontSettings( maCJKFont, RES_CHRATR_CJK_FONT );

            // CTL
            GetFontSettings( maCTLFont, RES_CHRATR_CTL_FONT );

            rWrtShell.EndCursorMove();
            rWrtShell.Pop(SwCursorShell::PopMode::DeleteCurrent);
        }
        else
        {
            // query Font at character template
            SwCharFormat *pFormat = rWrtShell.GetCharStyle(
                                    mpPage->m_xTemplateBox->get_active_text(),
                                    SwWrtShell::GETSTYLE_CREATEANY );
            assert(pFormat && "character style doesn't exist!");
            const SvxFontItem &rFormatFont = pFormat->GetFont();

            aFont.SetFamily(rFormatFont.GetFamily());
            aFont.SetFamilyName(rFormatFont.GetFamilyName());
            aFont.SetPitch(rFormatFont.GetPitch());
            aFont.SetCharSet(rFormatFont.GetCharSet());
        }

        const Color& rFontColor = rWrtShell.GetViewOptions()->GetFontColor();
        aFont.SetColor( rFontColor );
        maCJKFont.SetColor( rFontColor );
        maCTLFont.SetColor( rFontColor );
    }

    mnTextH = mnLines * mnTotLineH;
    aFont.SetFontSize(Size(0, mnTextH));
    maCJKFont.SetFontSize(Size(0, mnTextH));
    maCTLFont.SetFontSize(Size(0, mnTextH));

    aFont.SetTransparent(true);
    maCJKFont.SetTransparent(true);
    maCTLFont.SetTransparent(true);

    aFont.SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());
    maCJKFont.SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());
    maCTLFont.SetFillColor(Application::GetSettings().GetStyleSettings().GetWindowColor());

    maCJKFont.SetFontSize(Size(0, maCJKFont.GetFontSize().Height()));
    maCTLFont.SetFontSize(Size(0, maCTLFont.GetFontSize().Height()));

    aFont.SetFontSize(Size(0, aFont.GetFontSize().Height()));
    maFont = aFont;

    CheckScript();

    maTextSize = CalcTextSize();

    Invalidate();
}

void SwDropCapsPict::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    if (!IsVisible())
        return;

    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));
    rRenderContext.SetLineColor();

    rRenderContext.SetFillColor(maBackColor);

    Size aOutputSizePixel(GetOutputSizePixel());

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aOutputSizePixel));
    rRenderContext.SetClipRegion(vcl::Region(tools::Rectangle(Point(BORDER, BORDER),
                                                       Size(aOutputSizePixel.Width () - 2 * BORDER,
                                                            aOutputSizePixel.Height() - 2 * BORDER))));

    OSL_ENSURE(mnLineH > 0, "We cannot make it that small");
    tools::Long nY0 = (aOutputSizePixel.Height() - (LINES * mnTotLineH)) / 2;

    rRenderContext.SetFillColor(maTextLineColor);

    for (int i = 0; i < LINES; ++i)
    {
        rRenderContext.DrawRect(tools::Rectangle(Point(BORDER, nY0 + i * mnTotLineH),
                                Size(aOutputSizePixel.Width() - 2 * BORDER, mnLineH)));
    }

    // Text background with gap (240 twips ~ 1 line height)
    const tools::Long nDistW = (((static_cast<tools::Long>(mnDistance) * 100) / 240) * mnTotLineH) / 100;
    rRenderContext.SetFillColor(maBackColor);
    if (mpPage && mpPage->m_xDropCapsBox->get_active())
    {
        const Size aTextSize(maTextSize.Width() + nDistW, maTextSize.Height());
        rRenderContext.DrawRect(tools::Rectangle(Point(BORDER, nY0), aTextSize));

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
            aPt.AdjustX(maScriptChanges[nIdx].textWidth );

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
    if( !m_xBreak.is() )
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        m_xBreak = css::i18n::BreakIterator::create(xContext);
    }
    sal_Int16 nScript = m_xBreak->getScriptType( maText, 0 );
    sal_Int32 nChg = 0;
    if( css::i18n::ScriptType::WEAK == nScript )
    {
        nChg = m_xBreak->endOfScript( maText, nChg, nScript );
        if( nChg < maText.getLength() )
            nScript = m_xBreak->getScriptType( maText, nChg );
        else
            nScript = css::i18n::ScriptType::LATIN;
    }

    for(;;)
    {
        nChg = m_xBreak->endOfScript( maText, nChg, nScript );
        maScriptChanges.emplace_back(nScript, nChg );
        if( nChg >= maText.getLength() || nChg < 0 )
            break;
        nScript = m_xBreak->getScriptType( maText, nChg );
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
    tools::Long nTextWidth = 0;
    tools::Long nCJKHeight = 0;
    tools::Long nCTLHeight = 0;
    tools::Long nHeight = 0;
    tools::Long nAscent = 0;
    tools::Long nCJKAscent = 0;
    tools::Long nCTLAscent = 0;
    do
    {
        SvxFont& rFnt = (nScript == css::i18n::ScriptType::ASIAN)
                            ? maCJKFont
                            : ((nScript == css::i18n::ScriptType::COMPLEX)
                                    ? maCTLFont
                                    : maFont);

        sal_uLong nWidth = rFnt.GetTextSize(*mpPrinter, maText, nStart, nEnd-nStart ).Width();

        if (nIdx < maScriptChanges.size())
            maScriptChanges[nIdx].textWidth = nWidth;
        nTextWidth += nWidth;
        switch(nScript)
        {
            case css::i18n::ScriptType::ASIAN:
                calcFontHeightAnyAscent(GetDrawingArea()->get_ref_device(), maCJKFont, nCJKHeight, nCJKAscent);
                break;
            case css::i18n::ScriptType::COMPLEX:
                calcFontHeightAnyAscent(GetDrawingArea()->get_ref_device(), maCTLFont, nCTLHeight, nCTLAscent);
                break;
            default:
                calcFontHeightAnyAscent(GetDrawingArea()->get_ref_device(), maFont, nHeight, nAscent);
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

void SwDropCapsPict::InitPrinter_()
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

SwDropCapsDlg::SwDropCapsDlg(weld::Window *pParent, const SfxItemSet &rSet)
    : SfxSingleTabDialogController(pParent, &rSet)
{
    auto xNewPage(SwDropCapsPage::Create(get_content_area(), this, &rSet));
    static_cast<SwDropCapsPage*>(xNewPage.get())->SetFormat(false);
    SetTabPage(std::move(xNewPage));
}

SwDropCapsPage::SwDropCapsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/dropcapspage.ui"_ustr, u"DropCapPage"_ustr, &rSet)
    , m_bModified(false)
    , m_bFormat(true)
    , m_xDropCapsBox(m_xBuilder->weld_check_button(u"checkCB_SWITCH"_ustr))
    , m_xWholeWordCB(m_xBuilder->weld_check_button(u"checkCB_WORD"_ustr))
    , m_xSwitchText(m_xBuilder->weld_label(u"labelFT_DROPCAPS"_ustr))
    , m_xDropCapsField(m_xBuilder->weld_spin_button(u"spinFLD_DROPCAPS"_ustr))
    , m_xLinesText(m_xBuilder->weld_label(u"labelTXT_LINES"_ustr))
    , m_xLinesField(m_xBuilder->weld_spin_button(u"spinFLD_LINES"_ustr))
    , m_xDistanceText(m_xBuilder->weld_label(u"labelTXT_DISTANCE"_ustr))
    , m_xDistanceField(m_xBuilder->weld_metric_spin_button(u"spinFLD_DISTANCE"_ustr, FieldUnit::CM))
    , m_xTextText(m_xBuilder->weld_label(u"labelTXT_TEXT"_ustr))
    , m_xTextEdit(m_xBuilder->weld_entry(u"entryEDT_TEXT"_ustr))
    , m_xTemplateText(m_xBuilder->weld_label(u"labelTXT_TEMPLATE"_ustr))
    , m_xTemplateBox(m_xBuilder->weld_combo_box(u"comboBOX_TEMPLATE"_ustr))
    , m_xPict(new weld::CustomWeld(*m_xBuilder, u"drawingareaWN_EXAMPLE"_ustr, m_aPict))
{
    m_aPict.SetDropCapsPage(this);

    SetExchangeSupport();

    const sal_uInt16 nHtmlMode = ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    m_bHtmlMode = (nHtmlMode & HTMLMODE_ON) != 0;

    // tdf#92154 limit comboBOX_TEMPLATE length
    const int nMaxWidth(m_xTemplateBox->get_approximate_digit_width() * 50);
    m_xTemplateBox->set_size_request(nMaxWidth , -1);

    // In the template dialog the text is not influenceable
    m_xTextText->set_sensitive(!m_bFormat);
    m_xTextEdit->set_sensitive(!m_bFormat);

    // Metrics
    SetFieldUnit(*m_xDistanceField, GetDfltMetric(m_bHtmlMode));

    // Install handler
    Link<weld::SpinButton&,void> aValueChangedLk = LINK(this, SwDropCapsPage, ValueChangedHdl);
    m_xDropCapsField->connect_value_changed(aValueChangedLk);
    m_xLinesField->connect_value_changed(aValueChangedLk);
    Link<weld::MetricSpinButton&,void> aMetricValueChangedLk = LINK(this, SwDropCapsPage, MetricValueChangedHdl);
    m_xDistanceField->connect_value_changed(aMetricValueChangedLk);
    m_xTextEdit->connect_changed(LINK(this, SwDropCapsPage, ModifyHdl));
    m_xDropCapsBox->connect_toggled(LINK(this, SwDropCapsPage, ClickHdl));
    m_xTemplateBox->connect_changed(LINK(this, SwDropCapsPage, SelectHdl));
    m_xWholeWordCB->connect_toggled(LINK(this, SwDropCapsPage, WholeWordHdl));
}

SwDropCapsPage::~SwDropCapsPage()
{
}

DeactivateRC SwDropCapsPage::DeactivatePage(SfxItemSet * _pSet)
{
    if (_pSet)
        FillSet(*_pSet);

    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SwDropCapsPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet)
{
    return std::make_unique<SwDropCapsPage>(pPage, pController, *rSet);
}

bool  SwDropCapsPage::FillItemSet(SfxItemSet *rSet)
{
    if (m_bModified)
        FillSet(*rSet);
    return m_bModified;
}

void  SwDropCapsPage::Reset(const SfxItemSet *rSet)
{
    // Characters, lines, gap and text
    SwFormatDrop aFormatDrop( rSet->Get(RES_PARATR_DROP) );
    if (aFormatDrop.GetLines() > 1)
    {
        m_xDropCapsField->set_value(aFormatDrop.GetChars());
        m_xLinesField->set_value(aFormatDrop.GetLines());
        m_xDistanceField->set_value(m_xDistanceField->normalize(aFormatDrop.GetDistance()), FieldUnit::TWIP);
        m_xWholeWordCB->set_active(aFormatDrop.GetWholeWord());
    }
    else
    {
        m_xDropCapsField->set_value(1);
        m_xLinesField->set_value(3);
        m_xDistanceField->set_value(0, FieldUnit::TWIP);
    }

    SwView* pView = GetActiveView();
    if (pView)
        ::FillCharStyleListBox(*m_xTemplateBox, pView->GetWrtShell().GetView().GetDocShell(), true);

    m_xTemplateBox->insert_text(0, SwResId(SW_STR_NONE));

    // Reset format
    int nSelect = 0;
    if (aFormatDrop.GetCharFormat())
    {
        int nPos = m_xTemplateBox->find_text(aFormatDrop.GetCharFormat()->GetName());
        if (nPos != -1)
            nSelect = nPos;
    }
    m_xTemplateBox->set_active(nSelect);

    // Enable controls
    m_xDropCapsBox->set_active(aFormatDrop.GetLines() > 1);
    const sal_Int32 nVal = m_xDropCapsField->get_value();
    if (m_bFormat)
        m_xTextEdit->set_text(GetDefaultString(nVal));
    else
    {
        if (pView)
            m_xTextEdit->set_text(pView->GetWrtShell().GetDropText(nVal));
        m_xTextEdit->set_sensitive(true);
        m_xTextText->set_sensitive(true);
    }

    // Preview
    m_aPict.SetValues(m_xTextEdit->get_text(),
                      sal_uInt8(m_xLinesField->get_value()),
                      sal_uInt16(m_xDistanceField->denormalize(m_xDistanceField->get_value(FieldUnit::TWIP))));

    ClickHdl(*m_xDropCapsBox);
    m_bModified = false;
}

IMPL_LINK_NOARG(SwDropCapsPage, ClickHdl, weld::Toggleable&, void)
{
    bool bChecked = m_xDropCapsBox->get_active();

    m_xWholeWordCB->set_sensitive(bChecked && !m_bHtmlMode);

    m_xSwitchText->set_sensitive(bChecked && !m_xWholeWordCB->get_active());
    m_xDropCapsField->set_sensitive(bChecked && !m_xWholeWordCB->get_active());
    m_xLinesText->set_sensitive( bChecked );
    m_xLinesField->set_sensitive( bChecked );
    m_xDistanceText->set_sensitive( bChecked );
    m_xDistanceField->set_sensitive( bChecked );
    m_xTemplateText->set_sensitive( bChecked );
    m_xTemplateBox->set_sensitive( bChecked );
    m_xTextEdit->set_sensitive( bChecked && !m_bFormat );
    m_xTextText->set_sensitive( bChecked && !m_bFormat );

    if ( bChecked )
    {
        ValueChangedHdl(*m_xDropCapsField);
        m_xDropCapsField->grab_focus();
    }
    else
        m_aPict.SetText(u""_ustr);

    m_bModified = true;
}

IMPL_LINK_NOARG(SwDropCapsPage, WholeWordHdl, weld::Toggleable&, void)
{
    m_xDropCapsField->set_sensitive(!m_xWholeWordCB->get_active());
    m_xSwitchText->set_sensitive(!m_xWholeWordCB->get_active());

    ValueChangedHdl(*m_xDropCapsField);

    m_bModified = true;
}

void SwDropCapsPage::ModifyEntry(const weld::Entry& rEdit)
{
    OUString sPreview;

    // set text if applicable
    if (&rEdit == m_xDropCapsField.get())
    {
        const sal_Int32 nVal = !m_xWholeWordCB->get_active()
            ? static_cast<sal_Int32>(m_xDropCapsField->get_value())
            : 0;
        bool bSetText = false;

        if (SwView* pView = GetActiveView())
        {
            if (m_bFormat || pView->GetWrtShell().GetDropText(1).isEmpty())
                sPreview = GetDefaultString(nVal);
            else
            {
                bSetText = true;
                sPreview = pView->GetWrtShell().GetDropText(nVal);
            }
        }

        OUString sEdit(m_xTextEdit->get_text());

        if (!sEdit.isEmpty() && !sPreview.startsWith(sEdit))
        {
            sPreview = sEdit.copy(0, std::min(sEdit.getLength(), sPreview.getLength()));
            bSetText = false;
        }

        if (bSetText)
            m_xTextEdit->set_text(sPreview);
    }
    else if (&rEdit == m_xTextEdit.get())   // set quantity if applicable
    {
        const sal_Int32 nTmp = m_xTextEdit->get_text().getLength();
        m_xDropCapsField->set_value(std::max<sal_Int32>(1, nTmp));
        sPreview = m_xTextEdit->get_text();
    }

    // adjust image
    if (&rEdit == m_xDropCapsField.get() || &rEdit == m_xTextEdit.get())
        m_aPict.SetText(sPreview);
    else if (&rEdit == m_xLinesField.get())
        m_aPict.SetLines(static_cast<sal_uInt8>(m_xLinesField->get_value()));
    else
        m_aPict.SetDistance(o3tl::narrowing<sal_uInt16>(m_xDistanceField->denormalize(m_xDistanceField->get_value(FieldUnit::TWIP))));

    m_bModified = true;
}

IMPL_LINK(SwDropCapsPage, ModifyHdl, weld::Entry&, rEdit, void)
{
    ModifyEntry(rEdit);
}

IMPL_LINK(SwDropCapsPage, ValueChangedHdl, weld::SpinButton&, rEdit, void)
{
    ModifyEntry(rEdit);
}

IMPL_LINK(SwDropCapsPage, MetricValueChangedHdl, weld::MetricSpinButton&, rEdit, void)
{
    ModifyEntry(rEdit.get_widget());
}

IMPL_LINK_NOARG(SwDropCapsPage, SelectHdl, weld::ComboBox&, void)
{
    m_aPict.UpdatePaintSettings();
    m_bModified = true;
}

void SwDropCapsPage::FillSet( SfxItemSet &rSet )
{
    if(!m_bModified)
        return;

    SwFormatDrop aFormat;

    bool bOn = m_xDropCapsBox->get_active();
    if (bOn)
    {
        // quantity, lines, gap
        aFormat.GetChars()     = static_cast<sal_uInt8>(m_xDropCapsField->get_value());
        aFormat.GetLines()     = static_cast<sal_uInt8>(m_xLinesField->get_value());
        aFormat.GetDistance()  = o3tl::narrowing<sal_uInt16>(m_xDistanceField->denormalize(m_xDistanceField->get_value(FieldUnit::TWIP)));
        aFormat.GetWholeWord() = m_xWholeWordCB->get_active();

        // template
        if (SwView* pView = GetActiveView())
            if (m_xTemplateBox->get_active())
                aFormat.SetCharFormat(pView->GetWrtShell().GetCharStyle(m_xTemplateBox->get_active_text()));
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
    if (!m_bFormat && m_xDropCapsBox->get_active())
    {
        OUString sText(m_xTextEdit->get_text());

        if (!m_xWholeWordCB->get_active())
        {
            sText = sText.copy(0, std::min<sal_Int32>(sText.getLength(), m_xDropCapsField->get_value()));
        }

        SfxStringItem aStr(FN_PARAM_1, sText);
        rSet.Put(aStr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
