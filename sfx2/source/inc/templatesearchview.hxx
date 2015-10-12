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

class TemplateSearchView : public ThumbnailView
{
public:

    TemplateSearchView ( vcl::Window* pParent, WinBits nWinStyle = WB_TABSTOP | WB_VSCROLL);

    void setOpenTemplateHdl (const Link<ThumbnailViewItem*, void> &rLink);

    void AppendItem(sal_uInt16 nAssocItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                    const OUString &rTitle, const OUString &rSubtitle,
                    const OUString &rPath, const BitmapEx &rImage );

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

protected:
    Link<ThumbnailViewItem*, void> maOpenTemplateHdl;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
