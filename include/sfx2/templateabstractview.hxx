/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATEABSTRACTVIEW_HXX
#define INCLUDED_SFX2_TEMPLATEABSTRACTVIEW_HXX

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
class TemplateViewItem;
class TemplateContainerItem;

enum class FILTER_APPLICATION
{
    NONE,
    WRITER,
    CALC,
    IMPRESS,
    DRAW
};

// Display template items depending on the generator application
class ViewFilter_Application
{
public:

    ViewFilter_Application (FILTER_APPLICATION App)
        : mApp(App)
    {}

    virtual ~ViewFilter_Application () {}

    bool operator () (const ThumbnailViewItem *pItem);

    static bool isFilteredExtension(FILTER_APPLICATION filter, const OUString &rExt);
    bool isValid (const OUString& rPath) const;

protected:

    FILTER_APPLICATION mApp;
};

class SFX2_DLLPUBLIC TemplateAbstractView : public ThumbnailView
{
public:

    TemplateAbstractView(vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren);
    TemplateAbstractView(vcl::Window* pParent);

    virtual ~TemplateAbstractView();
    virtual void dispose() override;

    void insertItem (const TemplateItemProperties &rTemplate);

    // Fill view with new item list
    void insertItems (const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected = true, bool bShowCategoryInTooltip = false);

    // Fill view with template folders thumbnails
    virtual void Populate () { }

    virtual void reload () { }

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void RequestHelp( const HelpEvent& rHEvt ) override;

    virtual void Command( const CommandEvent& rCEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    virtual void showAllTemplates () = 0;

    virtual void showRegion (TemplateContainerItem *pItem) = 0;

    virtual sal_uInt16 createRegion (const OUString &rName) = 0;

    sal_uInt16 getCurRegionId () const { return mnCurRegionId;}

    const OUString& getCurRegionName () const { return maCurRegionName;}

    // Check if the root region is visible or not.
    bool isNonRootRegionVisible () const { return mnCurRegionId > 0;}

    void setOpenRegionHdl(const Link<void*,void> &rLink);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void updateThumbnailDimensions(long itemMaxSize);

    long getThumbnailWidth() const  { return mnThumbnailWidth;}
    long getThumbnailHeight() const {return mnThumbnailHeight;}

    void RemoveDefaultTemplateIcon( const OUString& rPath);

    static BitmapEx scaleImg (const BitmapEx &rImg, long width, long height);

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

    static BitmapEx fetchThumbnail (const OUString &msURL, long width, long height);

protected:

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

public:
    static bool IsDefaultTemplate(const OUString& rPath);

protected:

    sal_uInt16 mnCurRegionId;
    OUString maCurRegionName;

    TemplateViewItem *maSelectedItem;

    long mnThumbnailWidth;
    long mnThumbnailHeight;

    Point maPosition;

    Link<void*,void>              maOpenRegionHdl;
    Link<ThumbnailViewItem*,void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
    Link<ThumbnailViewItem*,void> maDeleteTemplateHdl;
    Link<ThumbnailViewItem*,void> maDefaultTemplateHdl;
};

#endif // INCLUDED_SFX2_TEMPLATEABSTRACTVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
