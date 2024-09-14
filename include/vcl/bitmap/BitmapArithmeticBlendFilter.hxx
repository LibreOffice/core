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

#include <vcl/bitmap/BitmapFilter.hxx>

class VCL_DLLPUBLIC BitmapArithmeticBlendFilter final : public BitmapFilter
{
private:
    BitmapEx maBitmapEx2;
    double mnK1;
    double mnK2;
    double mnK3;
    double mnK4;

public:
    BitmapArithmeticBlendFilter(BitmapEx const& rBmpEx2, double nK1, double nK2, double nK3,
                                double nK4);
    ~BitmapArithmeticBlendFilter();

    BitmapEx execute(BitmapEx const& rBitmapEx) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
