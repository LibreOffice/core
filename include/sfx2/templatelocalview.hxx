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

#include <set>
#include <functional>

#include <sfx2/templateabstractview.hxx>

class SfxDocumentTemplates;
class TemplateContainerItem;

namespace com {
    namespace sun { namespace star { namespace frame {
        class XModel;
    }   }   }
}

class SFX2_DLLPUBLIC TemplateLocalView : public TemplateAbstractView
{
    typedef bool (*selection_cmp_fn)(const ThumbnailViewItem*,const ThumbnailViewItem*);

public:

    TemplateLocalView ( vcl::Window* pParent );

    virtual ~TemplateLocalView ();
    virtual void dispose() override;

    // Fill view with template folders thumbnails
    virtual void Populate () override;

    virtual void reload () override;

    virtual void showRootRegion () override;

    virtual void showRegion (ThumbnailViewItem *pItem) override;

    void showRegion (const OUString &rName);

    sal_uInt16 getCurRegionItemId () const;

    sal_uInt16 getRegionId (size_t pos) const;

    OUString getRegionName(const sal_uInt16 nRegionId) const;

    OUString getRegionItemName(const sal_uInt16 nItemId) const;

    std::vector<OUString> getFolderNames ();

    std::vector<TemplateItemProperties>
        getFilteredItems (const std::function<bool (const TemplateItemProperties&) > &rFunc) const;

    virtual sal_uInt16 createRegion (const OUString &rName) override;

    virtual bool isNestedRegionAllowed () const override;

    virtual bool isImportAllowed () const override;

    bool removeRegion (const sal_uInt16 nItemId);

    bool removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId);

    bool moveTemplate (const ThumbnailViewItem* pItem, const sal_uInt16 nSrcItem,
                       const sal_uInt16 nTargetItem);

    bool moveTemplates (const std::set<const ThumbnailViewItem*,selection_cmp_fn> &rItems, const sal_uInt16 nTargetItem);

    bool copyFrom (const sal_uInt16 nRegionItemId, const BitmapEx &rThumbnail, const OUString &rPath);

    // Import a template to the current region
    bool copyFrom (const OUString &rPath);

    bool copyFrom(TemplateContainerItem *pItem, const OUString &rPath);

    bool exportTo (const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName);

    bool saveTemplateAs (sal_uInt16 nItemId,
                         css::uno::Reference<css::frame::XModel> &rModel,
                         const OUString &rName);

    bool saveTemplateAs (TemplateContainerItem *pDstItem,
                         css::uno::Reference<css::frame::XModel> &rModel,
                         const OUString &rName);

    bool isTemplateNameUnique (const sal_uInt16 nRegionItemId, const OUString &rName) const;

    virtual bool renameItem(ThumbnailViewItem* pItem, const OUString& sNewTitle) override;

private:

    SfxDocumentTemplates *mpDocTemplates;
    std::vector<TemplateContainerItem* > maRegions;
};

#endif // INCLUDED_SFX2_TEMPLATELOCALVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
