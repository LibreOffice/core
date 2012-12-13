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

#include <sfx2/templateproperties.hxx>
#include <sfx2/thumbnailview.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

class Edit;
class TemplateViewItem;

class TemplateView : public ThumbnailView
{
public:

    TemplateView (Window *pParent);

    virtual ~TemplateView ();

    void setId (const sal_uInt16 nId) { mnId = nId; }

    sal_uInt16 getId () const { return mnId; }

    void setName (const OUString &rName);

    const OUString& getName () const { return maName; }

    void InsertItems (const std::vector<TemplateItemProperties> &rTemplates);

    void setDblClickHdl (const Link &rLink) { maDblClickHdl = rLink; }

    void setCloseHdl (const Link &rLink) { maAllButton.SetClickHdl(rLink); }

protected:

    virtual void Resize ();

    virtual void OnItemDblClicked (ThumbnailViewItem *pItem);

private:

    Control    maButtons;
    PushButton maAllButton;
    FixedText  maFTName;
    sal_uInt16 mnId;
    OUString maName;
    Link maDblClickHdl;
};

#endif // TEMPLATEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

