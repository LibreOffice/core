/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATEPROPERTIES_HXX
#define INCLUDED_SFX2_TEMPLATEPROPERTIES_HXX

#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>

struct TemplateItemProperties
{
    sal_uInt16 nId; ///< Index for ThumbnailView
    sal_uInt16 nDocId; ///< Index based on SfxTemplateDocument
    sal_uInt16 nRegionId;
    OUString aName;
    OUString aPath;
    OUString aRegionName;
    BitmapEx aThumbnail;
};

#endif // INCLUDED_SFX2_TEMPLATEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
