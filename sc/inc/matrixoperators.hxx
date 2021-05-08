/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include <functional>
#include "math.hxx"

namespace sc::op {

template<typename tRes>
struct Op
{
    tRes m_aInitVal;
    std::function<void(tRes&, double)> m_aOp;

    inline Op(tRes aInitVal, std::function<void(tRes&, double)> aOp):
        m_aInitVal(aInitVal), m_aOp(aOp)
    {}

    inline Op(const Op<tRes>& aOp):
        m_aInitVal(aOp.m_aInitVal), m_aOp(aOp.m_aOp)
    {}
};

void fOpSum(KahanSum& aSum, double fVal);

void fOpSumSquare(KahanSum& aSum, double fVal);

void fOpProduct(double& fProd, double fVal);

void fOpSumSquareDiff(KahanSum& aSum, double fVal);

extern Op<KahanSum> OpSum;

extern Op<KahanSum> OpSumSquare;

extern Op<double> OpProduct;

extern Op<KahanSum> VOpSumAndSumSquare [2];

extern std::vector<Op<KahanSum>> OpSumAndSumSquare;

struct OpSumSquareDiff
{
    KahanSum m_aInitVal;
    double m_fDiff;

    void m_aOp(KahanSum& aSum, double fVal) const;

    inline OpSumSquareDiff(double fDiff):
        m_aInitVal(0), m_fDiff(fDiff)
    {}

    inline OpSumSquareDiff(const OpSumSquareDiff& aOp):
        m_aInitVal(aOp.m_aInitVal), m_fDiff(aOp.m_fDiff)
    {}
};


}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
