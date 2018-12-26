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

class TemplateViewItem;
class PopupMenu;
class Menu;

class TemplateSearchView final : public ThumbnailView
{
public:

    TemplateSearchView ( vcl::Window* pParent);

    void setOpenTemplateHdl (const Link<ThumbnailViewItem*, void> &rLink);

    DECL_LINK(ContextMenuSelectHdl, Menu*, bool);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void createContextMenu(const bool bIsDefault);

    void AppendItem(sal_uInt16 nAssocItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                    const OUString &rTitle, const OUString &rSubtitle,
                    const OUString &rPath, const BitmapEx &rImage );

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

private:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void Command( const CommandEvent& rCEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    TemplateViewItem *maSelectedItem;

    Point maPosition;

    Link<ThumbnailViewItem*, void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*, void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
    Link<ThumbnailViewItem*,void> maDeleteTemplateHdl;
    Link<ThumbnailViewItem*,void> maDefaultTemplateHdl;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
