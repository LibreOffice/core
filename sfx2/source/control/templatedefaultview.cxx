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
#include <vcl/event.hxx>
#include <sfx2/app.hxx>

#include <sfx2/strings.hrc>

#define MNI_OPEN               1
#define MNI_EDIT               2

#include <officecfg/Office/Common.hxx>

VCL_BUILDER_FACTORY(TemplateDefaultView)

static constexpr int gnItemPadding(5); //TODO:: Change padding to 10. It looks really crowded and occupied.
static constexpr long gnTextHeight = 30;

TemplateDefaultView::TemplateDefaultView( Window* pParent)
    : TemplateLocalView(pParent, WB_TABSTOP)
{
    tools::Rectangle aScreen = Application::GetScreenPosSizePixel(Application::GetDisplayBuiltInScreen());
    mnItemMaxSize = std::min(aScreen.GetWidth(),aScreen.GetHeight()) > 800 ? 256 : 192;
    ThumbnailView::setItemDimensions( mnItemMaxSize, mnItemMaxSize, gnTextHeight, gnItemPadding );
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
    set_width_request(gnTextHeight + mnItemMaxSize + 2*gnItemPadding);
}

void TemplateDefaultView::showAllTemplates()
{
    mnCurRegionId = 0;

    insertItems(maAllTemplates, false);
    maOpenRegionHdl.Call(nullptr);
}

void TemplateDefaultView::KeyInput( const KeyEvent& rKEvt )
{
    ThumbnailView::KeyInput(rKEvt);
}

void TemplateDefaultView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem);
        if(pViewItem)
            maOpenTemplateHdl.Call(pViewItem);
        return;
    }

    TemplateLocalView::MouseButtonDown(rMEvt);
}

void TemplateDefaultView::createContextMenu()
{
    ScopedVclPtrInstance<PopupMenu> pItemMenu;
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN));
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE));
    deselectItems();
    maSelectedItem->setSelection(true);
    pItemMenu->SetSelectHdl(LINK(this, TemplateLocalView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, tools::Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK(TemplateDefaultView, ContextMenuSelectHdl, Menu*, pMenu, void)
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
