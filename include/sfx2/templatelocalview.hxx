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

#include <vcl/menu.hxx>
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
class PopupMenu;

namespace com {
    namespace sun { namespace star { namespace frame {
        class XModel;
    }   }   }
}

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

    static bool isFilteredExtension(FILTER_APPLICATION filter, const OUString &rExt);
    bool isValid (const OUString& rPath) const;

private:

    FILTER_APPLICATION const mApp;
};


class SFX2_DLLPUBLIC TemplateLocalView : public ThumbnailView
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*,const ThumbnailViewItem*);

public:

    TemplateLocalView ( vcl::Window* pParent, WinBits nWinStyle = WB_TABSTOP | WB_BORDER );

    virtual ~TemplateLocalView () override;
    virtual void dispose() override;

    // Fill view with new item list
    void insertItems (const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected = true, bool bShowCategoryInTooltip = false);

    // Fill view with template folders thumbnails
    void Populate ();

    virtual void reload ();

    virtual void showAllTemplates ();

    void showRegion (TemplateContainerItem const *pItem);

    void showRegion (const OUString &rName);

    void createContextMenu(const bool bIsDefault );

    DECL_LINK(ContextMenuSelectHdl, Menu*, bool);

    TemplateContainerItem* getRegion(OUString const & sStr);

    sal_uInt16 getRegionId (size_t pos) const;

    sal_uInt16 getRegionId (OUString const & sRegionName) const;

    OUString getRegionName(const sal_uInt16 nRegionId) const;

    OUString getRegionItemName(const sal_uInt16 nItemId) const;

    std::vector<OUString> getFolderNames ();

    std::vector<TemplateItemProperties>
        getFilteredItems (const std::function<bool (const TemplateItemProperties&) > &rFunc) const;

    sal_uInt16 createRegion (const OUString &rName);

    bool renameRegion(const OUString &rTitle, const OUString &rNewTitle);

    bool removeRegion (const sal_uInt16 nItemId);

    bool removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId);

    bool moveTemplate (const ThumbnailViewItem* pItem, const sal_uInt16 nSrcItem,
                       const sal_uInt16 nTargetItem);

    void moveTemplates (const std::set<const ThumbnailViewItem*,selection_cmp_fn> &rItems, const sal_uInt16 nTargetItem);

    bool copyFrom(TemplateContainerItem *pItem, const OUString &rPath);

    bool exportTo (const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName);

    virtual bool renameItem(ThumbnailViewItem* pItem, const OUString& sNewTitle) override;

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void RequestHelp( const HelpEvent& rHEvt ) override;

    virtual void Command( const CommandEvent& rCEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    sal_uInt16 getCurRegionId () const { return mnCurRegionId;}

    void setOpenRegionHdl(const Link<void*,void> &rLink);

    void setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink);

    void updateThumbnailDimensions(long itemMaxSize);

    void RemoveDefaultTemplateIcon( const OUString& rPath);

    static BitmapEx scaleImg (const BitmapEx &rImg, long width, long height);

    static BitmapEx getDefaultThumbnail( const OUString& rPath );

    static BitmapEx fetchThumbnail (const OUString &msURL, long width, long height);

    static bool IsDefaultTemplate(const OUString& rPath);

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

protected:
    sal_uInt16 mnCurRegionId;

    TemplateViewItem *maSelectedItem;

    long mnThumbnailWidth;
    long mnThumbnailHeight;

    Point maPosition; //store the point of click event

    Link<void*,void>              maOpenRegionHdl;
    Link<ThumbnailViewItem*,void> maCreateContextMenuHdl;
    Link<ThumbnailViewItem*,void> maOpenTemplateHdl;
    Link<ThumbnailViewItem*,void> maEditTemplateHdl;
    Link<ThumbnailViewItem*,void> maDeleteTemplateHdl;
    Link<ThumbnailViewItem*,void> maDefaultTemplateHdl;

    std::unique_ptr<SfxDocumentTemplates> mpDocTemplates;
    std::vector<std::unique_ptr<TemplateContainerItem> > maRegions;
    std::vector<TemplateItemProperties > maAllTemplates;
};

#endif // INCLUDED_SFX2_TEMPLATELOCALVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
