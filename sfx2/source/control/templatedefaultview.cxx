/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4     -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatedefaultview.hxx>
#include <sfx2/thumbnailview.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/app.hxx>

#include <officecfg/Office/Common.hxx>

VCL_BUILDER_FACTORY(TemplateDefaultView)

TemplateDefaultView::TemplateDefaultView( Window* pParent)
    : TemplateLocalView(pParent)
    , mnTextHeight(30)
    , mnItemPadding(5)
{
    Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;
    ThumbnailView::setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );

    // startcenter specific settings
    maFillColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsBackgroundColor::get());
    maTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsTextColor::get());
    maHighlightColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightColor::get());
    maHighlightTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightTextColor::get());
    mfHighlightTransparence = 0.25;


    maAllButton->SetControlForeground(maTextColor);
    maFTName->SetControlForeground(maTextColor);

    // TODO - convert the TemplateAbstractView to .ui (instead of fixed layout
    // of the button and the fixed text), and do the following:
    // const float fMultiplier = 1.4;
    // vcl::Font aFont(maAllButton->GetSettings().GetStyleSettings().GetPushButtonFont());
    // aFont.SetSize(Size(0, aFont.GetSize().Height() * fMultiplier));
    // maAllButton->SetControlFont(aFont);
    // maFTName->SetControlFont(aFont);
}

void TemplateDefaultView::reload()
{
    TemplateLocalView::reload();
    // Set preferred width
    set_width_request(mnTextHeight + mnItemMaxSize + 2*mnItemPadding);

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
