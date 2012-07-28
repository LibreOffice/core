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
    TemplateSearchViewItem (ThumbnailView &rView, Window *pParent)
        : TemplateViewItem(rView,pParent)
    {}

    virtual ~TemplateSearchViewItem ()
    {}

    sal_uInt16 mnIdx;       // Template associated Index
    sal_uInt16 mnRegionId;  // Template associated Region id
};

#endif // __SFX2_TEMPLATESEARCHVIEWITEM_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
