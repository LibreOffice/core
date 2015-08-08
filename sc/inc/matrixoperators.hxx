/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_MATRIXOPERATORS_HXX
#define INCLUDED_SC_INC_MATRIXOPERATORS_HXX


namespace sc {

namespace op {


template<typename T>
struct _Op
{
    const double mInitVal;
    const T maOp;
    _Op(double InitVal, T aOp);
    virtual ~_Op() = default;
    virtual double operator() (double fRes, double fVal);
};

typedef _Op<std::function<double(double, double)>> Op;

struct Sum : public Op
{
    static const double InitVal;
    Sum();
};

struct SumSquare : public Op
{
    static const double InitVal;
    SumSquare();
};

struct Product : public Op
{
    static const double InitVal;
    Product();
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
