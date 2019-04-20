/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>

#include <algorithm>

#include <vcl/BitmapFilter.hxx>

class BitmapEx;

class VCL_DLLPUBLIC BitmapMosaicFilter : public BitmapFilter
{
public:
    BitmapMosaicFilter(sal_uLong nTileWidth, sal_uLong nTileHeight)
        : mnTileWidth(std::max(nTileWidth, sal_uLong(1)))
        , mnTileHeight(std::max(nTileHeight, sal_uLong(1)))
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    sal_uLong const mnTileWidth;
    sal_uLong const mnTileHeight;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
