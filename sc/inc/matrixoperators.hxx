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
    const tRes m_aInitVal;
    const std::function<void(tRes&, double)> m_aOp;
    Op(tRes aInitVal, std::function<void(tRes&, double)> aOp):
        m_aInitVal(aInitVal), m_aOp(aOp)
    {
    }
    void operator()(tRes& rAccum, double fVal) const
    {
        m_aOp(rAccum, fVal);
    }
};

static void fOpSum(KahanSum& aSum, double fVal) {aSum += fVal;}

static void fOpSumSquare(KahanSum& aSum, double fVal) {aSum += fVal*fVal;}

static void fOpProduct(double& fProd, double fVal){fProd *= fVal;}

static Op<KahanSum> OpSum( 0.0, fOpSum );

static Op<KahanSum> OpSumSquare( 0.0, fOpSumSquare );

static Op<double> OpProduct( 0.0, fOpProduct );

extern Op<KahanSum> VOpSumAndSumSquare [2];

extern std::vector<Op<KahanSum>> OpSumAndSumSquare;

struct Sum
{
    static const double m_aInitVal;
    void operator()(KahanSum& rAccum, double fVal) const;
};

struct SumSquare
{
    static const double m_aInitVal;
    void operator()(KahanSum& rAccum, double fVal) const;
};

struct Product
{
    static const double m_aInitVal;
    void operator()(double& rAccum, double fVal) const;
};

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
