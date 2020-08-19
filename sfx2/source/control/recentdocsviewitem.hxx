/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_RECENTDOCSVIEWITEM_HXX
#define INCLUDED_SFX2_RECENTDOCSVIEWITEM_HXX

#include <sfx2/thumbnailviewitem.hxx>

class ThumbnailView;

namespace sfx2
{
    class RecentDocsView;
}

class RecentDocsViewItem final : public ThumbnailViewItem
{
public:
    RecentDocsViewItem(sfx2::RecentDocsView &rView, const OUString &rURL,
        const OUString &rTitle, const BitmapEx& rThumbnail, sal_uInt16 nId, long nThumbnailSize);

    /** Updates own highlight status based on the aPoint position.

        Calls the ancestor's updateHighlight, and then takes care of m_bRemoveIconHighlighted.

        Returns rectangle that needs to be invalidated.
    */
    virtual tools::Rectangle updateHighlight(bool bVisible, const Point& rPoint) override;

    /// Text to be used for the tooltip.
    virtual OUString getHelpText() const override;

    virtual void Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                       const ThumbnailItemAttributes *pAttrs) override;

    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;

    /// Called when the user clicks a document - it will open it.
    void OpenDocument();

private:
    sfx2::RecentDocsView& mrParentView;

    /// Return area where is the icon to remove document from the recent documents.
    tools::Rectangle getRemoveIconArea() const;

    OUString maURL;

    OUString m_sHelpText;

    /// Is the icon that the user can click to remove the document from the recent documents highlighted?
    bool m_bRemoveIconHighlighted;

    BitmapEx m_aRemoveRecentBitmap;

    BitmapEx m_aRemoveRecentBitmapHighlighted;
};

#endif // INCLUDED_SFX2_RECENTDOCSVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
