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
#include <sfx2/templateviewitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/app.hxx>

#include "../doc/doc.hrc"

#define MNI_OPEN               1
#define MNI_EDIT               2

#include <officecfg/Office/Common.hxx>

VCL_BUILDER_FACTORY(TemplateDefaultView)

TemplateDefaultView::TemplateDefaultView( Window* pParent)
    : TemplateLocalView(pParent)
    , mnTextHeight(30)
    , mnItemPadding(5)//TODO:: Change padding to 10. It looks really crowded and occupied.
{
    Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;
    ThumbnailView::setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );
    updateThumbnailDimensions(mnItemMaxSize);

    // startcenter specific settings
    maFillColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsBackgroundColor::get());
    maTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsTextColor::get());
    maHighlightColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightColor::get());
    maHighlightTextColor = Color(officecfg::Office::Common::Help::StartCenter::StartCenterThumbnailsHighlightTextColor::get());
    mfHighlightTransparence = 0.25;
}

void TemplateDefaultView::reload()
{
    TemplateLocalView::reload();
    // Set preferred width
    set_width_request(mnTextHeight + mnItemMaxSize + 2*mnItemPadding);
}

void TemplateDefaultView::showAllTemplates()
{
    mnCurRegionId = 0;
    maCurRegionName.clear();

    insertItems(maAllTemplates, false);
    maOpenRegionHdl.Call(nullptr);
}

void TemplateDefaultView::KeyInput( const KeyEvent& rKEvt )
{
    ThumbnailView::KeyInput(rKEvt);
}

void TemplateDefaultView::createContextMenu()
{
    std::unique_ptr<PopupMenu> pItemMenu(new PopupMenu);
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN).toString());
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE).toString());
    deselectItems();
    maSelectedItem->setSelection(true);
    pItemMenu->SetSelectHdl(LINK(this, TemplateLocalView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK_TYPED(TemplateDefaultView, ContextMenuSelectHdl, Menu*, pMenu, void)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_OPEN:
        maOpenTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_EDIT:
        maEditTemplateHdl.Call(maSelectedItem);
        break;
    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
