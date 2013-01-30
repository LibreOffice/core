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

//template thumbnail item defines
#define TEMPLATE_ITEM_MAX_WIDTH 160
#define TEMPLATE_ITEM_MAX_HEIGHT 160
#define TEMPLATE_ITEM_PADDING 5
#define TEMPLATE_ITEM_MAX_TEXT_LENGTH 20
#define TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT 88

//template thumbnail image defines
#define TEMPLATE_THUMBNAIL_MAX_HEIGHT TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT - 2*TEMPLATE_ITEM_PADDING
#define TEMPLATE_THUMBNAIL_MAX_WIDTH TEMPLATE_ITEM_MAX_WIDTH - 2*TEMPLATE_ITEM_PADDING

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

    virtual ~ViewFilter_Application () {}

    virtual bool operator () (const ThumbnailViewItem *pItem);

    bool isValid (const rtl::OUString& rPath) const;

protected:

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
    virtual void Populate () { };

    virtual void reload () { };

    virtual void filterTemplatesByApp (const FILTER_APPLICATION &eApp);

    void showOverlay (bool bVisible);

    void setItemDimensions (long ItemWidth, long ThumbnailHeight, long DisplayHeight, int itemPadding);

    sal_uInt16 getOverlayRegionId () const;

    const OUString& getOverlayName () const;

    // Check if the overlay is visible or not.
    bool isOverlayVisible () const;

    void deselectOverlayItems ();

    void sortOverlayItems (const boost::function<bool (const ThumbnailViewItem*,
                                                       const ThumbnailViewItem*) > &func);

    virtual void filterTemplatesByKeyword (const OUString &rKeyword);

    void setOverlayItemStateHdl (const Link &aLink) { maOverlayItemStateHdl = aLink; }

    void setOpenHdl (const Link &rLink);

    void setOverlayCloseHdl (const Link &rLink);

    static BitmapEx scaleImg (const BitmapEx &rImg, long width, long height);

    static BitmapEx fetchThumbnail (const OUString &msURL, long width, long height);

    virtual void Resize();

protected:

    virtual void Paint( const Rectangle& rRect );

    virtual void DrawItem (ThumbnailViewItem *pItem);

    DECL_LINK(OverlayItemStateHdl, const ThumbnailViewItem*);

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem);

protected:

    TemplateView *mpItemView;
    Link maOverlayItemStateHdl;
    Link maOpenHdl;

    bool mbFilteredResults;     // Flag keep track if overlay has been filtered so folders can get filtered too afterwards
    FILTER_APPLICATION meFilterOption;
};

#endif // __SFX2_TEMPLATEABSTRACTVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
