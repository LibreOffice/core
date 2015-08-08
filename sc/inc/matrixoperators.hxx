/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
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
