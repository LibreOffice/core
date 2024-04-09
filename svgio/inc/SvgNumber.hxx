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

#include <basegfx/range/b2drange.hxx>
#include <vector>

namespace svgio::svgreader
{

enum class NumberType
{
    xcoordinate,
    ycoordinate,
    length
};

class InfoProvider
{
public:
    virtual ~InfoProvider() {}
    virtual basegfx::B2DRange getCurrentViewPort() const = 0;
    /// return font size of node, either set here or inherited from parents
    virtual double getCurrentFontSize() const = 0;
    /// return xheight of node inherited from parents
    virtual double getCurrentXHeightInherited() const = 0;
};

enum class SvgUnit
{
    em = 0,    // relative to current font size
    ex,        // relative to current x-height

    px,        // 'user unit'
    pt,        // points, 1/72 in
    pc,        // 1/6 in
    cm,
    mm,
    in,

    percent,   // relative to range
    none       // for stroke-miterlimit, which has no unit
};

class SvgNumber
{
private:
    double      mfNumber;
    SvgUnit     meUnit;

    bool        mbSet : 1;

public:
    SvgNumber()
    :   mfNumber(0.0),
        meUnit(SvgUnit::px),
        mbSet(false)
    {
    }

    SvgNumber(double fNum, SvgUnit aSvgUnit = SvgUnit::px, bool bSet = true)
    :   mfNumber(fNum),
        meUnit(aSvgUnit),
        mbSet(bSet)
    {
    }

    double getNumber() const
    {
        return mfNumber;
    }

    SvgUnit getUnit() const
    {
        return meUnit;
    }

    bool isSet() const
    {
        return mbSet;
    }

    bool isPositive() const
    {
        return mfNumber >= 0.0;
    }

    // Only usable in cases, when the unit is not SvgUnit::percent, otherwise use method solve
    double solveNonPercentage(const InfoProvider& rInfoProvider) const;

    double solve(const InfoProvider& rInfoProvider, NumberType aNumberType = NumberType::length) const;
};

typedef std::vector<SvgNumber> SvgNumberVector;

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
