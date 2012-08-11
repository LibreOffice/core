/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_TEMPLATEPROPERTIES_HXX__
#define __SFX2_TEMPLATEPROPERTIES_HXX__

#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>

struct TemplateItemProperties
{
    sal_uInt16 nId;         ///< Index for ThumbnailView
    sal_uInt16 nDocId;      ///< Index based on SfxTemplateDocument
    sal_uInt16 nRegionId;
    rtl::OUString aName;
    rtl::OUString aPath;
    rtl::OUString aType;
    BitmapEx aThumbnail;
};

#endif // __SFX2_TEMPLATEPROPERTIES_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
