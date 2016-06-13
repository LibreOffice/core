/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEW_HXX

#include <sfx2/thumbnailview.hxx>

class TemplateOnlineViewItem;
class PopupMenu;

class TemplateOnlineView : public ThumbnailView
{
public:

    TemplateOnlineView ( vcl::Window* pParent, WinBits nWinStyle = WB_TABSTOP | WB_VSCROLL);

    virtual ~TemplateOnlineView ();

    // Fill view with template thumbnails
    void Populate ();

//    void setQueryOutputString (const OUString &sStr) { msJSONQueryOutput = sStr; }

    void setOpenTemplateHdl (const Link<ThumbnailViewItem*, void> &rLink);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void createContextMenu();

    void AppendItem(const OUString &rTitle, const OUString &rHelpText,
                    const OUString &rPath, const OUString &rUrl,
                    const OUString &rName, const OUString &rDesc,
                    const BitmapEx &rImage );

    DECL_LINK_TYPED(ContextMenuSelectHdl, Menu*, bool);

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void Command( const CommandEvent& rCEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

protected:
    char* msJSONQueryOutput;

    TemplateOnlineViewItem *maSelectedItem;

    Point maPosition;

    Link<ThumbnailViewItem*, void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*, void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
