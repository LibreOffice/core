/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEFOLDERVIEW_HXX
#define TEMPLATEFOLDERVIEW_HXX

#include <sfx2/thumbnailview.hxx>

class SfxOrganizeMgr;

class TemplateFolderView : public ThumbnailView
{
public:

    TemplateFolderView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~TemplateFolderView ();

    // Fill view with template folders thumbnails
    void Populate ();

    // Check if the overlay is visible or not.
    bool isOverlayVisible () const;

    void showOverlay (bool bVisible);

private:

    virtual void OnItemDblClicked (ThumbnailViewItem *pRegionItem);

private:

    SfxOrganizeMgr *mpMgr;
    ThumbnailView *mpItemView;
};

#endif // TEMPLATEFOLDERVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
