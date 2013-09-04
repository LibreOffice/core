/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "types.hxx"
#include "scmatrix.hxx"

namespace sc {

RangeMatrix::RangeMatrix() : mpMat(NULL), mnCol1(-1), mnRow1(-1), mnCol2(-1), mnRow2(-1) {}

bool RangeMatrix::isRangeValid() const
{
    return mnCol1 >= 0 && mnRow1 >= 0 && mnCol2 >= 0 && mnRow2 >= 0 && mnCol1 <= mnCol2 && mnRow1 <= mnRow2;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
