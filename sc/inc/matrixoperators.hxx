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
#include <vector>
#include "kahan.hxx"

namespace sc::op {


template<typename T, typename tRes>
struct Op_
{
    const double mInitVal;
    const T maOp;
    Op_(double InitVal, T aOp):
        mInitVal(InitVal), maOp(std::move(aOp))
    {
    }
    void operator()(tRes& rAccum, double fVal) const
    {
        maOp(rAccum, fVal);
    }
};

using Op = Op_<std::function<void(double&, double)>, double>;
using kOp = Op_<std::function<void(KahanSum&, double)>, KahanSum>;

void fkOpSum(KahanSum& rAccum, double fVal);
void fkOpSumSquare(KahanSum& rAccum, double fVal);

extern kOp kOpSum;
extern kOp kOpSumSquare;
extern std::vector<kOp> kOpSumAndSumSquare;

struct Sum
{
    static const double InitVal;
    void operator()(KahanSum& rAccum, double fVal) const;
};

struct SumSquare
{
    static const double InitVal;
    void operator()(KahanSum& rAccum, double fVal) const;
};

struct Product
{
    static const double InitVal;
    void operator()(double& rAccum, double fVal) const;
};

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
