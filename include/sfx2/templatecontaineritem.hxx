/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATECONTAINERITEM_HXX
#define INCLUDED_SFX2_TEMPLATECONTAINERITEM_HXX

#include <sfx2/templateproperties.hxx>

class TemplateContainerItem
{
public:

    sal_uInt16 mnId;
    sal_uInt16 mnRegionId;
    OUString maTitle;
    std::vector<TemplateItemProperties> maTemplates;

    TemplateContainerItem (sal_uInt16 nId);

    virtual ~TemplateContainerItem ();
};

#endif // INCLUDED_SFX2_TEMPLATECONTAINERITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
