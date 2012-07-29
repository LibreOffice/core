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
class TemplateFolderViewItem;

enum FILTER_APPLICATION
{
    FILTER_APP_NONE,
    FILTER_APP_WRITER,
    FILTER_APP_CALC,
    FILTER_APP_IMPRESS,
    FILTER_APP_DRAW
};

class SFX2_DLLPUBLIC TemplateFolderView : public TemplateAbstractView
{
public:

    TemplateFolderView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~TemplateFolderView ();

    // Fill view with template folders thumbnails
    virtual void Populate ();

    std::vector<rtl::OUString> getFolderNames ();

    virtual void showOverlay (bool bVisible);

    void filterTemplatesByApp (const FILTER_APPLICATION &eApp);

    std::vector<TemplateItemProperties>
        getFilteredItems (const boost::function<bool (const TemplateItemProperties&) > &rFunc) const;

    void setTemplateStateHdl (const Link &aLink) { maTemplateStateHdl = aLink; }

    sal_uInt16 createRegion (const rtl::OUString &rName);

    bool removeRegion (const sal_uInt16 nItemId);

    bool removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId);

    bool moveTemplate (const ThumbnailViewItem* pItem, const sal_uInt16 nSrcItem,
                       const sal_uInt16 nTargetItem, bool bCopy);

    bool moveTemplates (std::set<const ThumbnailViewItem*> &rItems, const sal_uInt16 nTargetItem, bool bCopy);

    void copyFrom (TemplateFolderViewItem *pItem, const rtl::OUString &rPath);

private:

    virtual void Resize ();

    virtual void OnItemDblClicked (ThumbnailViewItem *pRegionItem);

    DECL_LINK(TVTemplateStateHdl, const ThumbnailViewItem*);

    DECL_LINK(ChangeNameHdl, TemplateView*);

private:

    bool mbFilteredResults;     // Flag keep track if overlay has been filtered so folders can get filtered too afterwards
    FILTER_APPLICATION meFilterOption;
    SfxDocumentTemplates *mpDocTemplates;
    Link maTemplateStateHdl;
};

#endif // TEMPLATEFOLDERVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
