/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>

TemplateContainerItem::TemplateContainerItem (sal_uInt16 nId)
    : mnId(nId)
    , mnRegionId(0)
{
}

TemplateContainerItem::~TemplateContainerItem ()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


