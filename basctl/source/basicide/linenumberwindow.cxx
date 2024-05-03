/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "baside2.hxx"

#include <vcl/event.hxx>
#include <vcl/textview.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/settings.hxx>

namespace basctl
{
LineNumberWindow::LineNumberWindow(vcl::Window* pParent, ModulWindow* pModulWindow)
    : Window(pParent, WB_BORDER)
    , m_pModulWindow(pModulWindow)
    , m_nCurYOffset(0)
{
    // tdf#153853 The line number window does not need to be affected by RTL
    EnableRTL(false);

    const Wallpaper aBackground(GetSettings().GetStyleSettings().GetWindowColor());
    SetBackground(aBackground);
    GetWindow(GetWindowType::Border)->SetBackground(aBackground);
    m_FontColor = GetSettings().GetStyleSettings().GetWindowTextColor();
    m_HighlightColor = GetSettings().GetStyleSettings().GetFaceColor();
    m_nBaseWidth = GetTextWidth(u"8"_ustr);
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
}

LineNumberWindow::~LineNumberWindow() { disposeOnce(); }

void LineNumberWindow::dispose()
{
    m_pModulWindow.clear();
    Window::dispose();
}

void LineNumberWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (SyncYOffset())
        return;

    ExtTextEngine* txtEngine = m_pModulWindow->GetEditEngine();
    if (!txtEngine)
        return;

    TextView* txtView = m_pModulWindow->GetEditView();
    if (!txtView)
        return;

    int windowHeight = rRenderContext.GetOutputSize().Height();
    int windowWidth = rRenderContext.GetOutputSize().Width();
    int nLineHeight = rRenderContext.GetTextHeight();
    if (!nLineHeight)
    {
        return;
    }

    int startY = txtView->GetStartDocPos().Y();
    const sal_uInt32 nStartLine = startY / nLineHeight + 1;
    sal_uInt32 nEndLine = (startY + windowHeight) / nLineHeight + 1;

    if (txtEngine->GetParagraphCount() + 1 < nEndLine)
        nEndLine = txtEngine->GetParagraphCount() + 1;

    // FIXME: it would be best if we could get notified of a font change
    // rather than doing that re-calculation at each Paint event
    m_nBaseWidth = GetTextWidth(u"8"_ustr);

    // reserve enough for 3 digit minimum, with a bit to spare for comfort
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
    auto nMaxLineNumber = std::max(nEndLine, txtEngine->GetParagraphCount() + 1);
    sal_uInt32 i = (nMaxLineNumber + 1) / 1000;
    while (i)
    {
        i /= 10;
        m_nWidth += m_nBaseWidth;
    }

    vcl::Font aNormalFont = rRenderContext.GetFont();
    vcl::Font aBoldFont(aNormalFont);
    aBoldFont.SetWeight(FontWeight::WEIGHT_BOLD);

    sal_uInt32 nParaEnd = txtView->GetSelection().GetEnd().GetPara() + 1;
    sal_Int64 y = (nStartLine - 1) * static_cast<sal_Int64>(nLineHeight);

    for (sal_uInt32 n = nStartLine; n <= nEndLine; ++n, y += nLineHeight)
    {
        // Font weight for the selected lines is bold
        if (n == nParaEnd)
        {
            tools::Rectangle aRect(Point(0, y - m_nCurYOffset),
                                   Point(windowWidth, y - m_nCurYOffset + nLineHeight));
            rRenderContext.SetFillColor(m_HighlightColor);
            rRenderContext.DrawRect(aRect);
            rRenderContext.SetFont(aBoldFont);
        }
        else
        {
            rRenderContext.SetFont(aNormalFont);
        }

        rRenderContext.SetTextColor(m_FontColor);
        const OUString aLineNumber = OUString::number(n);
        // tdf#153798 - align line numbers to the right
        rRenderContext.DrawText(
            Point(m_nWidth - GetTextWidth(aLineNumber) - m_nBaseWidth / 2, y - m_nCurYOffset),
            aLineNumber);
    }
    // Restore the original font
    rRenderContext.SetFont(aNormalFont);

    // Resize the parent after calculating the new width and height values
    GetParent()->Resize();
}

void LineNumberWindow::DataChanged(DataChangedEvent const& rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DataChangedEventType::SETTINGS
        && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetFieldColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
    }
}

void LineNumberWindow::DoScroll(tools::Long nVertScroll)
{
    m_nCurYOffset -= nVertScroll;
    Window::Scroll(0, nVertScroll);
}

bool LineNumberWindow::SyncYOffset()
{
    TextView* pView = m_pModulWindow->GetEditView();
    if (!pView)
        return false;

    tools::Long nViewYOffset = pView->GetStartDocPos().Y();
    if (m_nCurYOffset == nViewYOffset)
        return false;

    m_nCurYOffset = nViewYOffset;
    Invalidate();
    return true;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
