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
/* Simple operators */

void Sum::operator()(KahanSum& rAccum, double fVal) const { rAccum += fVal; }

const double Sum::InitVal = 0.0;

void SumSquare::operator()(KahanSum& rAccum, double fVal) const { rAccum += fVal * fVal; }

const double SumSquare::InitVal = 0.0;

void Product::operator()(double& rAccum, double fVal) const { rAccum *= fVal; }

const double Product::InitVal = 1.0;

/* Op operators */

void fkOpSum(KahanSum& rAccum, double fVal) { rAccum += fVal; }

kOp kOpSum(0.0, fkOpSum);

void fkOpSumSquare(KahanSum& rAccum, double fVal) { rAccum += fVal * fVal; }

kOp kOpSumSquare(0.0, fkOpSumSquare);

kOp vkOpSumAndSumSquare[] = { kOpSum, kOpSumSquare };

std::vector<kOp> kOpSumAndSumSquare
    = std::vector<kOp>(std::begin(vkOpSumAndSumSquare), std::end(vkOpSumAndSumSquare));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
