/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sfx2/templatelocalview.hxx>
#include <sfx2/listview.hxx>

class TemplateDlgLocalView final : public TemplateLocalView, public ListView
{
public:
    TemplateDlgLocalView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                         std::unique_ptr<weld::Menu> xMenu,
                         std::unique_ptr<weld::TreeView> xTreeView);

    void setTemplateViewMode(TemplateViewMode eMode);

    virtual void showAllTemplates() override;

    void showRegion(TemplateContainerItem const* pItem);

    void showRegion(std::u16string_view rName);

    void reload();

    virtual bool KeyInput(const KeyEvent& rKEvt) override;

    void createContextMenu(const bool bIsDefault, const bool bIsBuiltIn);

    virtual void Show() override;

    virtual void Hide() override;

    bool IsVisible();

    void connect_focus_rect(const Link<weld::Widget&, tools::Rectangle>& rLink)
    {
        GetDrawingArea()->connect_focus_rect(rLink);
    }

    void MakeItemVisible(sal_uInt16 nId) { ThumbnailView::MakeItemVisible(nId); }

private:
    void ContextMenuSelectHdl(std::string_view rIdent);

    void insertFilteredItems();

    void syncCursor();

    void updateSelection();

    DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);

    DECL_LINK(ListViewChangedHdl, weld::TreeView&, void);

    DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);

    DECL_LINK(KeyPressHdl, const KeyEvent&, bool);

    TemplateViewMode mViewMode;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
