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


template<typename T>
_Op<T>::_Op(double InitVal, T aOp):
    mInitVal(InitVal), maOp(aOp)
{
}

template<typename T>
double _Op<T>::operator()(double fRes, double fVal)
{
    return maOp(fRes, fVal);
}

Sum::Sum():
    Op(InitVal, [](double fRes, double fVal){return fRes + fVal;})
{
}

const double Sum::InitVal = 0.0;

SumSquare::SumSquare():
    Op(InitVal, [](double fRes, double fVal){return fRes + fVal * fVal;})
{
}

const double SumSquare::InitVal = 0.0;

Product::Product():
    Op(InitVal, [](double fRes, double fVal){return fRes * fVal;})
{
}

const double Product::InitVal = 1.0;

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
