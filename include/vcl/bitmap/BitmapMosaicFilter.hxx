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

#include <vcl/bitmap/BitmapFilter.hxx>

#include <algorithm>

class VCL_DLLPUBLIC BitmapMosaicFilter final : public BitmapFilter
{
public:
    BitmapMosaicFilter(sal_Int32 nTileWidth, sal_Int32 nTileHeight)
        : mnTileWidth(std::max(nTileWidth, sal_Int32(1)))
        , mnTileHeight(std::max(nTileHeight, sal_Int32(1)))
    {
    }

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    sal_Int32 mnTileWidth;
    sal_Int32 mnTileHeight;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
