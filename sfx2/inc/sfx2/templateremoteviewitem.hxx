/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEREMOTEVIEWITEM_HXX__
#define __SFX2_TEMPLATEREMOTEVIEWITEM_HXX__

#include <sfx2/thumbnailviewitem.hxx>
#include <sfx2/templateproperties.hxx>

class TemplateRemoteViewItem : public ThumbnailViewItem
{
public:

    TemplateRemoteViewItem (ThumbnailView &rView, Window *pParent);

    virtual ~TemplateRemoteViewItem ();

    void setURL (const rtl::OUString &rURL) { maUrl = rURL; }

    const rtl::OUString& getURL () const { return maUrl; }

    void insertTemplate (const TemplateItemProperties &prop) { maTemplates.push_back(prop); }

    void clearTemplates () { maTemplates.clear(); }

    const std::vector<TemplateItemProperties>& getTemplates () const { return maTemplates; }

private:

    rtl::OUString maUrl;
    std::vector<TemplateItemProperties> maTemplates;
};

#endif // __SFX2_TEMPLATEREMOTEVIEWITEM_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
