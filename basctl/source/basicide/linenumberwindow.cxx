/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 August Sodora <augsod@gmail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "baside2.hxx"
#include "linenumberwindow.hxx"

#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>

namespace basctl
{

LineNumberWindow::LineNumberWindow (Window* pParent, ModulWindow* pModulWindow) :
    Window(pParent, WB_BORDER),
    m_pModulWindow(pModulWindow),
    m_nCurYOffset(0)
{
    SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetFieldColor()));
    m_nBaseWidth = GetTextWidth('8');
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
}

LineNumberWindow::~LineNumberWindow()
{ }

void LineNumberWindow::Paint( const Rectangle& )
{
    if(SyncYOffset())
        return;

    ExtTextEngine* txtEngine = m_pModulWindow->GetEditEngine();
    if(!txtEngine)
        return;

    TextView* txtView = m_pModulWindow->GetEditView();
    if(!txtView)
        return;

    GetParent()->Resize();

    int windowHeight = GetOutputSize().Height();
    int nLineHeight = GetTextHeight();

    int startY = txtView->GetStartDocPos().Y();
    int nStartLine = startY / nLineHeight + 1;
    int nEndLine = (startY + windowHeight) / nLineHeight + 1;

    if(txtEngine->GetParagraphCount() + 1 < (unsigned int)nEndLine)
        nEndLine = txtEngine->GetParagraphCount() + 1;

    // FIXME: it would be best if we could get notified of a font change
    // rather than doing that re-calculation at each Paint event
    m_nBaseWidth = GetTextWidth(OUString('8'));

    // reserve enough for 3 sigit minimum, with a bit to spare for confort
    m_nWidth = m_nBaseWidth * 3 + m_nBaseWidth / 2;
    int i = (nEndLine + 1) / 1000;
    while(i)
    {
        i /= 10;
        m_nWidth += m_nBaseWidth;
    }

    sal_Int64 y = (nStartLine - 1) * nLineHeight;
    for(sal_Int32 n = nStartLine; n <= nEndLine; ++n, y += nLineHeight)
        DrawText(Point(0, y - m_nCurYOffset), OUString::valueOf(n));
}

void LineNumberWindow::DataChanged(DataChangedEvent const & rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
    {
        Color aColor(GetSettings().GetStyleSettings().GetFieldColor());
        if (aColor != rDCEvt.GetOldSettings()->GetStyleSettings().GetFieldColor())
        {
            SetBackground(Wallpaper(aColor));
            Invalidate();
        }
    }
}

void LineNumberWindow::DoScroll(long nHorzScroll, long nVertScroll)
{
    m_nCurYOffset -= nVertScroll;
    Window::Scroll(nHorzScroll, nVertScroll);
}

long& LineNumberWindow::GetCurYOffset()
{
    return m_nCurYOffset;
}

bool LineNumberWindow::SyncYOffset()
{
    TextView* pView = m_pModulWindow->GetEditView();
    if (!pView)
        return false;

    long nViewYOffset = pView->GetStartDocPos().Y();
    if (m_nCurYOffset == nViewYOffset)
        return false;

    m_nCurYOffset = nViewYOffset;
    Invalidate();
    return true;
}

int LineNumberWindow::GetWidth()
{
    return m_nWidth;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
