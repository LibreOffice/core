/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <functional>
#include "matrixoperators.hxx"


namespace sc {

namespace op {

void Sum::operator()(double& rAccum, double fVal)
{
    rAccum += fVal;
}

const double Sum::InitVal = 0.0;

void SumSquare::operator()(double& rAccum, double fVal)
{
    rAccum += fVal * fVal;
}

const double SumSquare::InitVal = 0.0;

void Product::operator()(double& rAccum, double fVal)
{
    rAccum *= fVal;
}

const double Product::InitVal = 1.0;

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
