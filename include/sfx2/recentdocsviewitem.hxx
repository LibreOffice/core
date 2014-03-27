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

#include <sfx2/thumbnailview.hxx>

class RecentDocsViewItem : public ThumbnailViewItem
{
public:
    RecentDocsViewItem(ThumbnailView &rView, const OUString &rURL,
        const OUString &rTitle, const BitmapEx& rThumbnail, sal_uInt16 nId);
    virtual void setEditTitle (bool edit, bool bChangeFocus = true) SAL_OVERRIDE;

    /// Text to be used for the tooltip.
    virtual OUString getHelpText() const SAL_OVERRIDE;

    OUString maURL;

private:
    OUString m_sHelpText;
};

#endif // INCLUDED_SFX2_RECENTDOCSVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
