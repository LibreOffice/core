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

#include <sfx2/templateproperties.hxx>
#include <sfx2/thumbnailview.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

//template thumbnail item defines
#define TEMPLATE_ITEM_MAX_WIDTH 160
#define TEMPLATE_ITEM_MAX_HEIGHT 148
#define TEMPLATE_ITEM_PADDING 5
#define TEMPLATE_ITEM_MAX_TEXT_LENGTH 20
#define TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT 96

//template thumbnail height with a subtitle
#define TEMPLATE_ITEM_MAX_HEIGHT_SUB 160

//template thumbnail image defines
#define TEMPLATE_THUMBNAIL_MAX_HEIGHT TEMPLATE_ITEM_THUMBNAIL_MAX_HEIGHT - 2*TEMPLATE_ITEM_PADDING
#define TEMPLATE_THUMBNAIL_MAX_WIDTH TEMPLATE_ITEM_MAX_WIDTH - 2*TEMPLATE_ITEM_PADDING

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

    static bool isFilteredExtension(FILTER_APPLICATION filter, const OUString &rExt);
    bool isValid (const OUString& rPath) const;

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
    TemplateAbstractView ( Window* pParent );

    virtual ~TemplateAbstractView ();

    void insertItem (const TemplateItemProperties &rTemplate);

    // Fill view with new item list
    void insertItems (const std::vector<TemplateItemProperties> &rTemplates);

    // Fill view with template folders thumbnails
    virtual void Populate () { }

    virtual void reload () { }

    virtual void showRootRegion () = 0;

    virtual void showRegion (ThumbnailViewItem *pItem) = 0;

    virtual sal_uInt16 createRegion (const OUString &rName) = 0;

    // Return if we can have regions inside the current region
    virtual bool isNestedRegionAllowed () const = 0;

    // Return if we can import templates to the current region
    virtual bool isImportAllowed () const = 0;

    sal_uInt16 getCurRegionId () const;

    const OUString& getCurRegionName () const;

    // Check if the root region is visible or not.
    bool isNonRootRegionVisible () const;

    void setOpenRegionHdl(const Link &rLink);

    void setOpenTemplateHdl (const Link &rLink);

    static BitmapEx scaleImg (const BitmapEx &rImg, long width, long height);

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

    static BitmapEx fetchThumbnail (const OUString &msURL, long width, long height);

protected:

    DECL_LINK(ShowRootRegionHdl, void*);

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem);

    virtual void Paint( const Rectangle& rRect );

protected:

    sal_uInt16 mnCurRegionId;
    OUString maCurRegionName;

    PushButton maAllButton;
    FixedText  maFTName;

    Link maOpenRegionHdl;
    Link maOpenTemplateHdl;
};

#endif // __SFX2_TEMPLATEABSTRACTVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
