/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <matrixoperators.hxx>

namespace sc::op
{
/* Operators for simple tasks */

void Sum::operator()(KahanSum& rAccum, double fVal) const { rAccum += fVal; }

const double Sum::m_aInitVal = 0.0;

void SumSquare::operator()(KahanSum& rAccum, double fVal) const { rAccum += fVal * fVal; }

const double SumSquare::m_aInitVal = 0.0;

void Product::operator()(double& rAccum, double fVal) const { rAccum *= fVal; }

const double Product::m_aInitVal = 1.0;

/* Operators for complex tasks */

Op<KahanSum> VOpSumAndSumSquare[2] = { OpSum, OpSumSquare };

std::vector<Op<KahanSum>> OpSumAndSumSquare(std::begin(VOpSumAndSumSquare),
                                            std::end(VOpSumAndSumSquare));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
