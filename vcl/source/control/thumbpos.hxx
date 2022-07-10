/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

inline tools::Long ImplMulDiv(tools::Long nNumber, tools::Long nNumerator, tools::Long nDenominator)
{
    if (!nDenominator)
        return 0;
    double n = (static_cast<double>(nNumber) * static_cast<double>(nNumerator))
               / static_cast<double>(nDenominator);
    return static_cast<tools::Long>(n);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
