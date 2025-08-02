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

#include <tools/degree.hxx>
#include <vcl/bitmap/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapEmbossGreyFilter final : public BitmapFilter
{
public:
    BitmapEmbossGreyFilter(Degree100 nAzimuthAngle, Degree100 nElevationAngle)
        : mnAzimuthAngle(nAzimuthAngle)
        , mnElevationAngle(nElevationAngle)
    {
    }

    virtual Bitmap execute(Bitmap const& rBitmap) const override;

private:
    Degree100 mnAzimuthAngle;
    Degree100 mnElevationAngle;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
