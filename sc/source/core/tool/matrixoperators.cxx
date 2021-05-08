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
void fOpSum(KahanSum& aSum, double fVal) { aSum += fVal; }

void fOpSumSquare(KahanSum& aSum, double fVal) { aSum += fVal * fVal; }

void fOpProduct(double& fProd, double fVal) { fProd *= fVal; }

Op<KahanSum> OpSum(0.0, fOpSum);

Op<KahanSum> OpSumSquare(0.0, fOpSumSquare);

Op<double> OpProduct(1.0, fOpProduct);

Op<KahanSum> VOpSumAndSumSquare[2] = { OpSum, OpSumSquare };

std::vector<Op<KahanSum>> OpSumAndSumSquare(std::begin(VOpSumAndSumSquare),
                                            std::end(VOpSumAndSumSquare));

void OpSumSquareDiff::m_aOp(KahanSum& aSum, double fVal) const
{
    aSum += (fVal - m_fDiff) * (fVal - m_fDiff);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
