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

#include "address.hxx"
#include "scdllapi.h"

namespace sc
{
/** Data defining a sparkline - input data and output position */
struct SC_DLLPUBLIC SparklineData
{
    ScAddress maPosition;
    ScRange maData;

    SparklineData(ScAddress const& rPosition, ScRange const& rData)
        : maPosition(rPosition)
        , maData(rData)
    {
    }
};

enum class RangeOrientation
{
    Unknown,
    Row,
    Col
};

/** Determine the sparkline group orientation for the input data the output size */
SC_DLLPUBLIC RangeOrientation calculateOrientation(sal_Int32 nOutputSize,
                                                   ScRange const& rInputRange);

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
