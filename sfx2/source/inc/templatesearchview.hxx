/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEW_HXX

#include <sfx2/thumbnailview.hxx>
#include <sfx2/listview.hxx>

class TemplateViewItem;
class PopupMenu;
class Menu;

class TemplateSearchView final : public ThumbnailView, public ListView
{
public:

    TemplateSearchView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                       std::unique_ptr<weld::Menu> xMenu,
                       std::unique_ptr<weld::TreeView> xTreeView);

    void setOpenTemplateHdl (const Link<ThumbnailViewItem*, void> &rLink);

    void ContextMenuSelectHdl(std::string_view rIdent);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void createContextMenu(const bool bIsDefault, const bool bIsBuiltIn);

    void AppendItem(sal_uInt16 nAssocItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                    const OUString &rTitle, const OUString &rSubtitle,
                    const OUString &rPath, const BitmapEx &rImage );

    void setTemplateViewMode ( TemplateViewMode eMode );

    void Show() override;

    void Hide() override;

    void Clear() override;

    bool IsVisible();

    void syncCursor();

    void updateSelection();

    void connect_focus_rect(const Link<weld::Widget&, tools::Rectangle>& rLink) { GetDrawingArea()->connect_focus_rect(rLink);}

    void MakeItemVisible( sal_uInt16 nId ) { ThumbnailView::MakeItemVisible(nId);}

    DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);

    DECL_LINK(ListViewChangedHdl, weld::TreeView&, void);

    DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

    void RemoveDefaultTemplateIcon(std::u16string_view rPath);

private:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual bool Command(const CommandEvent& rPos) override;

    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    TemplateViewItem *maSelectedItem;

    Point maPosition;

    Link<ThumbnailViewItem*, void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*, void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
    Link<ThumbnailViewItem*,void> maDeleteTemplateHdl;
    Link<ThumbnailViewItem*,void> maDefaultTemplateHdl;
    TemplateViewMode mViewMode;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */