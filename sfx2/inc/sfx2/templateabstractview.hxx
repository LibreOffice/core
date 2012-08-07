/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEABSTRACTVIEW_HXX__
#define __SFX2_TEMPLATEABSTRACTVIEW_HXX__

#include <sfx2/thumbnailview.hxx>

class TemplateView;
class SfxDocumentTemplates;

enum FILTER_APPLICATION
{
    FILTER_APP_NONE,
    FILTER_APP_WRITER,
    FILTER_APP_CALC,
    FILTER_APP_IMPRESS,
    FILTER_APP_DRAW
};

// Display template items depending on the generator application
class ViewFilter_Application
{
public:

    ViewFilter_Application (FILTER_APPLICATION App)
        : mApp(App)
    {}

    bool operator () (const ThumbnailViewItem *pItem);

private:

    FILTER_APPLICATION mApp;
};

class ViewFilter_Keyword
{
public:

    ViewFilter_Keyword (const OUString &rKeyword)
        : maKeyword(rKeyword)
    {}

    bool operator () (const ThumbnailViewItem *pItem);

private:

    OUString maKeyword;
};

class SFX2_DLLPUBLIC TemplateAbstractView : public ThumbnailView
{
public:

    TemplateAbstractView (Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);

    TemplateAbstractView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~TemplateAbstractView ();

    // Fill view with template folders thumbnails
    virtual void Populate () = 0;

    virtual void reload () = 0;

    virtual void filterTemplatesByApp (const FILTER_APPLICATION &eApp) = 0;

    virtual void showOverlay (bool bVisible) = 0;

    sal_uInt16 getOverlayRegionId () const;

    // Check if the overlay is visible or not.
    bool isOverlayVisible () const;

    void sortOverlayItems (const boost::function<bool (const ThumbnailViewItem*,
                                                       const ThumbnailViewItem*) > &func);

    virtual void filterTemplatesByKeyword (const OUString &rKeyword);

    void setOverlayItemStateHdl (const Link &aLink) { maOverlayItemStateHdl = aLink; }

    void setOverlayDblClickHdl (const Link &rLink);

    void setOverlayCloseHdl (const Link &rLink);

    static BitmapEx scaleImg (const BitmapEx &rImg, long width, long height);

    static BitmapEx fetchThumbnail (const rtl::OUString &msURL, long width, long height);

protected:

    virtual void Resize();

    virtual void OnSelectionMode (bool bMode);

    DECL_LINK(OverlayItemStateHdl, const ThumbnailViewItem*);

protected:

    TemplateView *mpItemView;
    Link maOverlayItemStateHdl;
};

#endif // __SFX2_TEMPLATEABSTRACTVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
