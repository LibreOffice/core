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

class SfxDocumentTemplates;
class TemplateView;

// Display template items depending on the generator application
class ViewFilter_Application
{
public:

    enum APP_GENERATOR
    {
        APP_WRITER,
        APP_CALC,
        APP_IMPRESS,
        APP_DRAW
    };

    ViewFilter_Application (SfxDocumentTemplates *pDocTemplates, APP_GENERATOR App);

    bool operator () (const ThumbnailViewItem *pItem);

private:

    APP_GENERATOR mApp;
    SfxDocumentTemplates *mpDocTemplates;
};

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

    virtual void OnSelectionMode (bool bMode);

    virtual void OnItemDblClicked (ThumbnailViewItem *pRegionItem);

private:

    SfxDocumentTemplates *mpDocTemplates;
    TemplateView *mpItemView;
};

#endif // TEMPLATEFOLDERVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
