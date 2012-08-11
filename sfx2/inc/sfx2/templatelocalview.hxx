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

#include <set>

#include <sfx2/templateabstractview.hxx>
#include <sfx2/templateproperties.hxx>

class SfxDocumentTemplates;
class TemplateLocalViewItem;

class SFX2_DLLPUBLIC TemplateLocalView : public TemplateAbstractView
{
public:

    TemplateLocalView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~TemplateLocalView ();

    // Fill view with template folders thumbnails
    virtual void Populate ();

    virtual void reload ();

    std::vector<rtl::OUString> getFolderNames ();

    virtual void showOverlay (bool bVisible);

    virtual void filterTemplatesByApp (const FILTER_APPLICATION &eApp);

    std::vector<TemplateItemProperties>
        getFilteredItems (const boost::function<bool (const TemplateItemProperties&) > &rFunc) const;

    sal_uInt16 createRegion (const rtl::OUString &rName);

    bool removeRegion (const sal_uInt16 nItemId);

    bool removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId);

    bool moveTemplate (const ThumbnailViewItem* pItem, const sal_uInt16 nSrcItem,
                       const sal_uInt16 nTargetItem, bool bCopy);

    bool moveTemplates (std::set<const ThumbnailViewItem*> &rItems, const sal_uInt16 nTargetItem, bool bCopy);

    void copyFrom (const sal_uInt16 nRegionItemId, const BitmapEx &rThumbnail, const OUString &rPath);

    void copyFrom (TemplateLocalViewItem *pItem, const rtl::OUString &rPath);

    bool exportTo (const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName);

private:

    virtual void OnItemDblClicked (ThumbnailViewItem *pRegionItem);

    DECL_LINK(ChangeNameHdl, TemplateView*);

private:

    bool mbFilteredResults;     // Flag keep track if overlay has been filtered so folders can get filtered too afterwards
    FILTER_APPLICATION meFilterOption;
    SfxDocumentTemplates *mpDocTemplates;
};

#endif // TEMPLATEFOLDERVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
