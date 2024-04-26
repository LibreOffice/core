/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATELOCALVIEW_HXX
#define INCLUDED_SFX2_TEMPLATELOCALVIEW_HXX

#include <sfx2/thumbnailview.hxx>
#include <sfx2/templateproperties.hxx>
#include <functional>
#include <memory>
#include <set>

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
class TemplateContainerItem;
class TemplateViewItem;

enum class FILTER_APPLICATION
{
    NONE,
    WRITER,
    CALC,
    IMPRESS,
    DRAW
};

// Display template items depending on the generator application
class ViewFilter_Application final
{
public:

    ViewFilter_Application (FILTER_APPLICATION App)
        : mApp(App)
    {}

    bool operator () (const ThumbnailViewItem *pItem);

    static bool isFilteredExtension(FILTER_APPLICATION filter, std::u16string_view rExt);
    bool isValid (std::u16string_view rPath) const;

private:

    FILTER_APPLICATION mApp;
};

class TemplateLocalView : public ThumbnailView
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*,const ThumbnailViewItem*);

public:

    TemplateLocalView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                         std::unique_ptr<weld::Menu> xMenu);

    virtual ~TemplateLocalView () override;

    // Fill view with new item list
    void insertItems (const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected = true, bool bShowCategoryInTooltip = false);

    // Fill view with template folders thumbnails
    void Populate ();

    void reload ();

    virtual void showAllTemplates ();

    void showRegion (TemplateContainerItem const *pItem);

    void ContextMenuSelectHdl(std::u16string_view  rIdent);

    TemplateContainerItem* getRegion(std::u16string_view sStr);

    sal_uInt16 getRegionId (size_t pos) const;

    sal_uInt16 getRegionId (std::u16string_view sRegionName) const;

    OUString getRegionName(const sal_uInt16 nRegionId) const;

    OUString getRegionItemName(const sal_uInt16 nItemId) const;

    std::vector<OUString> getFolderNames ();

    std::vector<TemplateItemProperties>
        getFilteredItems (const std::function<bool (const TemplateItemProperties&) > &rFunc) const;

    sal_uInt16 createRegion (const OUString &rName);

    bool renameRegion(std::u16string_view rTitle, const OUString &rNewTitle);

    bool removeRegion (const sal_uInt16 nItemId);

    bool removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId);

    void moveTemplates (const std::set<const ThumbnailViewItem*,selection_cmp_fn> &rItems, const sal_uInt16 nTargetItem);

    bool copyFrom(TemplateContainerItem *pItem, const OUString &rPath);

    bool exportTo (const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, std::u16string_view rName);

    virtual bool renameItem(ThumbnailViewItem& rItem, const OUString& sNewTitle) override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual bool Command(const CommandEvent& rPos) override;

    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    sal_uInt16 getCurRegionId () const { return mnCurRegionId;}

    void setCurRegionId (sal_uInt16 nCurRegionId) { mnCurRegionId = nCurRegionId;}

    void setOpenRegionHdl(const Link<void*,void> &rLink);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDeleteTemplateHdl(const Link<void*,void> &rLink);

    void setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setMoveTemplateHdl(const Link<void*,void> &rLink);

    void setExportTemplateHdl(const Link<void*,void> &rLink);

    void updateThumbnailDimensions(tools::Long itemMaxSize);

    void RemoveDefaultTemplateIcon( std::u16string_view rPath);

    bool IsBuiltInRegion(const OUString& rRegionName);

    static BitmapEx scaleImg (const BitmapEx &rImg, tools::Long width, tools::Long height);

    static BitmapEx getDefaultThumbnail( std::u16string_view rPath );

    static BitmapEx fetchThumbnail (const OUString &msURL, tools::Long width, tools::Long height);

    static bool IsDefaultTemplate(const OUString& rPath);

    static bool IsInternalTemplate(const OUString& rPath);

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    sal_uInt16 mnCurRegionId;

    TemplateViewItem *maSelectedItem;

    tools::Long mnThumbnailWidth;
    tools::Long mnThumbnailHeight;

    Point maPosition; //store the point of click event

    Link<void*,void>              maOpenRegionHdl;
    Link<ThumbnailViewItem*,void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
    Link<void*,void> maDeleteTemplateHdl;
    Link<ThumbnailViewItem*,void> maDefaultTemplateHdl;
    Link<void*,void> maMoveTemplateHdl;
    Link<void*,void> maExportTemplateHdl;

    std::unique_ptr<SfxDocumentTemplates> mpDocTemplates;
    std::vector<std::unique_ptr<TemplateContainerItem> > maRegions;
    std::vector<TemplateItemProperties > maAllTemplates;
};


#endif // INCLUDED_SFX2_TEMPLATELOCALVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
