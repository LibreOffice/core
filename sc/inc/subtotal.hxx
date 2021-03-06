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

#pragma once

#include "global.hxx"

class SubTotal
{
public:
    static bool SafePlus( double& fVal1, double fVal2);
    static bool SafeMult( double& fVal1, double fVal2);
    static bool SafeDiv( double& fVal1, double fVal2);
};

class ScFunctionData;

/** Implements the Welford Online one-pass algorithm.
    See https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_Online_algorithm
    and Donald E. Knuth, TAoCP vol.2, 3rd edn., p. 232
 */
class WelfordRunner
{
public:
    WelfordRunner() : mfMean(0.0), mfM2(0.0), mnCount(0) {}
    void        update( double fVal );
    sal_uInt64  getCount() const                { return mnCount; }
    double      getVarianceSample() const       { return mnCount > 1 ? mfM2 / (mnCount-1) : 0.0; }
    double      getVariancePopulation() const   { return mnCount > 0 ? mfM2 / mnCount : 0.0; }

    // The private variables can be abused by ScFunctionData as general
    // sum/min/max/ave/count/... variables to reduce memory footprint for that
    // ScFunctionData may be a mass object during consolidation.
    // ScFunctionData::update() and getResult() take care that purposes are not
    // mixed.
    friend class ScFunctionData;
private:
    double      mfMean;
    double      mfM2;
    sal_uInt64  mnCount;
};

/** To calculate a single subtotal function. */
class ScFunctionData
{
public:
    ScFunctionData() : meFunc(SUBTOTAL_FUNC_NONE), mbError(false) {}
    ScFunctionData( ScSubTotalFunc eFn ) : meFunc(eFn), mbError(false) {}

    void            update( double fNewVal );
    /// Check getError() after (!) obtaining the result.
    double          getResult();
    bool            getError() const    { return mbError; }
    ScSubTotalFunc  getFunc() const     { return meFunc; }
    void            setError()          { mbError = true; }

private:
    WelfordRunner   maWelford;
    ScSubTotalFunc  meFunc;
    bool            mbError;

    double&         getValueRef()   { return maWelford.mfMean; }
    sal_uInt64&     getCountRef()   { return maWelford.mnCount; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
