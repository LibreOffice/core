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

#include <vcl/bitmapex.hxx>
class Animation;

class VCL_DLLPUBLIC BitmapFilter
{
public:
    BitmapFilter();
    virtual ~BitmapFilter();

    virtual BitmapEx execute(BitmapEx const& rBitmapEx) const = 0;

    static bool Filter(BitmapEx& rBmpEx, BitmapFilter const& rFilter);
    static bool Filter(Animation& rBmpEx, BitmapFilter const& rFilter);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
