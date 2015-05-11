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

VCL_BUILDER_FACTORY(TemplateDefaultView)

TemplateDefaultView::TemplateDefaultView( Window* pParent)
    : TemplateLocalView(pParent)
    , mnTextHeight(30)
    , mnItemPadding(5)
    , mnLastMouseDownItem( THUMBNAILVIEW_ITEM_NOTFOUND )
{
    Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;
    ThumbnailView::setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );
}

void TemplateDefaultView::reload()
{
    TemplateLocalView::reload();
    // Set preferred width
    set_width_request(mnTextHeight + mnItemMaxSize + 2*mnItemPadding);

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
