/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEWITEM_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEWITEM_HXX

#include <templateviewitem.hxx>

struct TemplateSearchViewItem : public TemplateViewItem
{
    TemplateSearchViewItem(ThumbnailViewBase& rView, sal_uInt16 nId)
        : TemplateViewItem(rView, nId)
        , mnAssocId(0)
    {
    }

    sal_uInt16 mnAssocId; //Associated item id to the TemplateViews
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATESEARCHVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
