/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <whatsnew.hxx>
#include <whatsnew.hrc>

#include <dialmgr.hxx>
#include <comphelper/DirectoryHelper.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/gdimtf.hxx>

constexpr tools::Long TEXT_HEIGHT(120);
constexpr tools::Long PROGRESS_DOTSIZE(24);
constexpr tools::Long PROGRESS_DOTSPACING(4);

WhatsNewDialog::WhatsNewDialog(weld::Window* pParent, const bool bWelcome)
    : GenericDialogController(pParent, u"cui/ui/whatsnewdialog.ui"_ustr, u"WhatsNewDialog"_ustr)
    , m_bWelcome(bWelcome)
    , m_aPreview()
    , m_aProgress()
    , m_pPrevBtn(m_xBuilder->weld_button(u"btnPrev"_ustr))
    , m_pNextBtn(m_xBuilder->weld_button(u"btnNext"_ustr))
    , m_pProgress(new weld::CustomWeld(*m_xBuilder, u"imProgress"_ustr, m_aProgress))
    , m_pImage(new weld::CustomWeld(*m_xBuilder, u"imNews"_ustr, m_aPreview))
{
    if (m_bWelcome)
        m_xDialog->set_title(CuiResId(STR_WELCOME));
    m_pPrevBtn->set_sensitive(false);

    m_pPrevBtn->connect_clicked(LINK(this, WhatsNewDialog, OnPrevClick));
    m_pNextBtn->connect_clicked(LINK(this, WhatsNewDialog, OnNextClick));

    m_nNumberOfNews = m_bWelcome ? std::size(WELCOME_STRINGARRAY) : std::size(WHATSNEW_STRINGARRAY);
    m_nCurrentNews = 0;
    m_pNextBtn->set_sensitive(m_nNumberOfNews > 1);
    m_pProgress->set_size_request(m_nNumberOfNews * (PROGRESS_DOTSIZE + PROGRESS_DOTSPACING),
                                  PROGRESS_DOTSIZE + 1);

    LoadImage();
}

WhatsNewDialog::~WhatsNewDialog() {}

void WhatsNewDialog::LoadImage()
{
    if (m_nCurrentNews < m_nNumberOfNews)
    {
        auto[sText, sImage] = m_bWelcome ? WELCOME_STRINGARRAY[m_nCurrentNews]
                                         : WHATSNEW_STRINGARRAY[m_nCurrentNews];
        OUString aURL(u"$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/whatsnew/"_ustr);
        rtl::Bootstrap::expandMacros(aURL);

        const bool bFileExists = comphelper::DirectoryHelper::fileExists(aURL + sImage);
        if (!sImage.isEmpty() && bFileExists)
            m_aPreview.Update(aURL + sImage, CuiResId(sText));

        m_aProgress.Update(m_nCurrentNews, m_nNumberOfNews);
    }
}

IMPL_LINK_NOARG(WhatsNewDialog, OnPrevClick, weld::Button&, void)
{
    m_nCurrentNews--;
    if (m_nCurrentNews == 0)
        m_pPrevBtn->set_sensitive(false);
    m_pNextBtn->set_sensitive(true);
    LoadImage();
}

IMPL_LINK_NOARG(WhatsNewDialog, OnNextClick, weld::Button&, void)
{
    m_nCurrentNews++;
    if (m_nCurrentNews == m_nNumberOfNews - 1)
        m_pNextBtn->set_sensitive(false);
    m_pPrevBtn->set_sensitive(true);
    LoadImage();
}

void WhatsNewImg::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings rSettings(Application::GetSettings().GetStyleSettings());

    //clear
    rRenderContext.SetBackground(Wallpaper(rSettings.GetDialogColor()));
    rRenderContext.Erase();

    //background image
    Graphic aGraphic;
    GraphicFilter::LoadGraphic(m_sImage, OUString(), aGraphic);
    const Size aGraphicSize(aGraphic.GetSizePixel());

    if (aGraphic.IsAnimated())
        aGraphic.StartAnimation(rRenderContext, Point(), aGraphicSize);
    else
        aGraphic.Draw(rRenderContext, Point(), aGraphicSize);

    tools::Rectangle aRect(Point(0, aGraphicSize.Height() - TEXT_HEIGHT),
                           Size(aGraphicSize.Width(), TEXT_HEIGHT));

    //transparent text background
    ScopedVclPtrInstance<VirtualDevice> aVDev;
    GDIMetaFile aMetafile;

    aMetafile.Record(aVDev.get());
    aMetafile.SetPrefSize(aRect.GetSize());
    aVDev->SetOutputSize(aRect.GetSize());
    aVDev->SetBackground(Color(0x10, 0x68, 0x02));
    aVDev->Erase();
    aMetafile.Stop();

    Gradient aVCLGradient;
    aVCLGradient.SetStyle(css::awt::GradientStyle_LINEAR);
    aVCLGradient.SetStartColor(COL_GRAY);
    aVCLGradient.SetEndColor(COL_GRAY);
    aVCLGradient.SetStartIntensity(33);
    aVCLGradient.SetEndIntensity(66);

    rRenderContext.DrawTransparent(aMetafile, aRect.TopLeft(), aRect.GetSize(), aVCLGradient);

    //text
    vcl::Font aFont = rRenderContext.GetFont();
    const Size aFontSize = aFont.GetFontSize();
    aFont.SetFontSize(Size(0, 24));
    aFont.SetColor(COL_WHITE);
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    DrawTextFlags nDrawTextStyle(DrawTextFlags::MultiLine | DrawTextFlags::WordBreak
                                 | DrawTextFlags::EndEllipsis);
    const bool bIsRTL = rRenderContext.GetTextIsRTL(m_sText, 0, m_sText.getLength());
    if (bIsRTL)
        nDrawTextStyle |= DrawTextFlags::Right;

    aRect.shrink(6);
    aRect.setWidth(rRenderContext.GetOutputSizePixel().Width() - 12);
    rRenderContext.DrawText(aRect, m_sText, nDrawTextStyle);

    aFont.SetFontSize(aFontSize);
    aFont.SetWeight(WEIGHT_NORMAL);
    rRenderContext.SetFont(aFont);
}

void WhatsNewProgress::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings rSettings(Application::GetSettings().GetStyleSettings());
    rRenderContext.SetBackground(rSettings.GetDialogColor());
    rRenderContext.Erase();
    rRenderContext.SetAntialiasing(AntialiasingFlags::Enable);

    tools::Rectangle aRect(Point(1, 1), Size(PROGRESS_DOTSIZE, PROGRESS_DOTSIZE));
    for (sal_Int32 i = 0; i < m_nTotal; i++)
    {
        if (i == m_nCurrent)
            rRenderContext.SetFillColor(rSettings.GetAccentColor());
        else
            rRenderContext.SetFillColor(COL_WHITE);
        rRenderContext.DrawEllipse(aRect);

        aRect.AdjustLeft(PROGRESS_DOTSIZE + PROGRESS_DOTSPACING);
        aRect.AdjustRight(PROGRESS_DOTSIZE + PROGRESS_DOTSPACING);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */