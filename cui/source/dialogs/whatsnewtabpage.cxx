/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <whatsnewtabpage.hxx>

#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/viewfrm.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/graphicfilter.hxx>

WhatsNewTabPage::WhatsNewTabPage(weld::Container* pPage, weld::DialogController* pController,
                                 const SfxItemSet& rAttr)
    : SfxTabPage(pPage, pController, u"cui/ui/whatsnewtabpage.ui"_ustr, u"WhatsNewTabPage"_ustr,
                 &rAttr)
    , m_aBrand()
    , m_pBrand(new weld::CustomWeld(*m_xBuilder, u"imBrand"_ustr, m_aBrand))
{
    const Size aGraphicSize = m_aBrand.GetGraphicSize();
    m_pBrand->set_size_request(aGraphicSize.getWidth(), aGraphicSize.getHeight());
}

std::unique_ptr<SfxTabPage> WhatsNewTabPage::Create(weld::Container* pPage,
                                                    weld::DialogController* pController,
                                                    const SfxItemSet* rAttr)
{
    return std::make_unique<WhatsNewTabPage>(pPage, pController, *rAttr);
}

AnimatedBrand::AnimatedBrand()
{
    OUString aURL(u"$BRAND_BASE_DIR/" LIBO_SHARE_SHELL_FOLDER "/animatedbrand.gif"_ustr);
    rtl::Bootstrap::expandMacros(aURL);
    GraphicFilter::LoadGraphic(aURL, OUString(), m_pGraphic);
    m_pGraphicSize = m_pGraphic.GetSizePixel();
}

void AnimatedBrand::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    //clear
    rRenderContext.SetBackground(COL_WHITE);
    rRenderContext.Erase();

    const Point aGraphicPosition((rRect.GetWidth() - m_pGraphicSize.Width()),
                                 (rRect.GetHeight() - m_pGraphicSize.Height()) >> 1);
#ifndef MACOSX
    if (m_pGraphic.IsAnimated() && MiscSettings::IsAnimatedOthersAllowed())
        m_pGraphic.StartAnimation(rRenderContext, aGraphicPosition, m_pGraphicSize);
    else
#endif
        m_pGraphic.Draw(rRenderContext, aGraphicPosition, m_pGraphicSize);

    tools::Rectangle aTextRect;
    if (m_pGraphic.isAvailable())
    {
        aTextRect.SetPos(Point(8, 8));
        aTextRect.SetSize(Size(aGraphicPosition.getX(), m_pGraphicSize.Height() - 20));
    }
    else
        aTextRect = rRect;

    //text
    OUString sText(SfxResId(STR_WHATSNEW_FIRST));

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetFontSize(Size(0, 24));
    aFont.SetColor(COL_BLACK);
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    DrawTextFlags nDrawTextStyle(DrawTextFlags::MultiLine | DrawTextFlags::WordBreak
                                 | DrawTextFlags::EndEllipsis);
    const bool bIsRTL = rRenderContext.GetTextIsRTL(sText, 0, sText.getLength());
    if (bIsRTL)
        nDrawTextStyle |= DrawTextFlags::Right;

    aTextRect.shrink(6);
    aTextRect.setWidth(aTextRect.GetWidth() - 12);
    rRenderContext.DrawText(aTextRect, sText, nDrawTextStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */