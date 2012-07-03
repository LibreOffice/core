/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEVIEW_HXX
#define TEMPLATEVIEW_HXX

#include <sfx2/thumbnailview.hxx>
#include <vcl/image.hxx>

class Edit;
class SfxDocumentTemplates;
class TemplateViewItem;

class TemplateView : public ThumbnailView
{
public:

    TemplateView (Window *pParent, SfxDocumentTemplates *pTemplates);

    virtual ~TemplateView ();

    void setRegionId (const sal_uInt16 nRegionId);

    sal_uInt16 getRegionId () const { return mnRegionId; }

    const rtl::OUString& getRegionName () const { return maFolderName; }

    virtual void Paint (const Rectangle &rRect);

    void InsertItem (const TemplateViewItem *pItem);

    void InsertItems (const std::vector<TemplateViewItem*> &rTemplates);

    void setDblClickHdl (const Link &rLink) { maDblClickHdl = rLink; }

    void setChangeNameHdl (const Link &rLink) { maChangeNameHdl = rLink; }

    void setCloseHdl (const Link &rLink) { maCloseHdl = rLink; }

protected:

    virtual void Resize ();

    virtual void MouseButtonDown (const MouseEvent &rMEvt);

    virtual void OnItemDblClicked (ThumbnailViewItem *pItem);

    DECL_LINK (ChangeNameHdl, void*);

private:

    Image maCloseImg;
    sal_uInt16 mnRegionId;
    rtl::OUString maFolderName;
    SfxDocumentTemplates *mpDocTemplates;
    Link maDblClickHdl;
    Link maChangeNameHdl;
    Link maCloseHdl;

    Edit *mpEditName;
};

#endif // TEMPLATEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

