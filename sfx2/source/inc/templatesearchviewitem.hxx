/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATESEARCHVIEWITEM_HXX__
#define __SFX2_TEMPLATESEARCHVIEWITEM_HXX__

#include <sfx2/templateviewitem.hxx>

struct TemplateSearchViewItem : public TemplateViewItem
{
    TemplateSearchViewItem (ThumbnailView &rView, sal_uInt16 nId)
        : TemplateViewItem(rView, nId)
    {}

    virtual ~TemplateSearchViewItem ()
    {}

    sal_uInt16 mnAssocId;    //Associated item id to the TemplateViews
};

#endif // __SFX2_TEMPLATESEARCHVIEWITEM_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
