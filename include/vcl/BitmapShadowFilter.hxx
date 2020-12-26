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

#include <vcl/BitmapFilter.hxx>

/** If the alpha is beyond a certain threshold, make it fully transparent
 */
class VCL_DLLPUBLIC BitmapShadowFilter final : public BitmapFilter
{
public:
    BitmapShadowFilter(Color aShadowColor)
        : maShadowColor(aShadowColor)
    {
    }

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const override;

private:
    Color maShadowColor;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
