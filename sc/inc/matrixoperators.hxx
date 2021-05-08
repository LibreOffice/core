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


template< typename tRes>
struct Op
{
    Op(tRes aInitVal) : m_aInitVal(aInitVal) {};
    const tRes m_aInitVal;
    virtual void operator()(tRes& rAccum, double fVal) const;
    virtual~Op(){};
};

class OpSum : public Op<KahanSum>
{
    OpSum() : Op<KahanSum>(0.0) {};
    void operator()(KahanSum& rAccum, double fVal) const override {rAccum += fVal;}
};

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
