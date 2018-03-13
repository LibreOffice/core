/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_COMMONBITMAPALGORITHMSFILTER_HXX
#define INCLUDED_INCLUDE_VCL_COMMONBITMAPALGORITHMSFILTER_HXX

#include <tools/helpers.hxx>

#define GAMMA(_def_cVal, _def_InvGamma)                                                            \
    (static_cast<sal_uInt8>(MinMax(FRound(pow(_def_cVal / 255.0, _def_InvGamma) * 255.0), 0, 255)))

constexpr sal_uLong RGB15(sal_uInt8 cR, sal_uInt8 cG, sal_uInt8 cB)
{
    return ((static_cast<sal_uLong>(cR) << 10) | (static_cast<sal_uLong>(cG) << 5)
            | static_cast<sal_uLong>(cB));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
