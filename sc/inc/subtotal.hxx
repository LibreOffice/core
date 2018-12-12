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

#ifndef INCLUDED_SC_INC_SUBTOTAL_HXX
#define INCLUDED_SC_INC_SUBTOTAL_HXX

#include "global.hxx"

class SubTotal
{
public:
    static bool SafePlus( double& fVal1, double fVal2);
    static bool SafeMult( double& fVal1, double fVal2);
    static bool SafeDiv( double& fVal1, double fVal2);
};

/** Implements the Welford Online one-pass algorithm.
    See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_Online_algorithm
    and Donald E. Knuth, TAoCP vol.2, 3rd edn., p. 232
 */
class WelfordRunner
{
public:
    WelfordRunner() : fMean(0.0), fM2(0.0), nCount(0) {}
    void        update( double fVal );
    sal_uInt64  getCount() const                { return nCount; }
    double      getMean() const                 { return fMean; }
    double      getVarianceSample() const       { return nCount > 1 ? fM2 / (nCount-1) : 0.0; }
    double      getVariancePopulation() const   { return nCount > 0 ? fM2 / nCount : 0.0; }

private:
    double      fMean;
    double      fM2;
    sal_Int64   nCount;
};

struct ScFunctionData                   // to calculate single functions
{
    WelfordRunner   maWelford;
    ScSubTotalFunc const  eFunc;
    double          nVal;
    sal_uInt64      nCount;
    bool            bError;

    ScFunctionData( ScSubTotalFunc eFn ) :
        eFunc(eFn), nVal(0.0), nCount(0), bError(false) {}
    void update( double fNewVal );
    /// Check bError after (!) obtaining the result.
    double getResult();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
