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


#include <cmath>
#include <file/FNumericFunctions.hxx>
#include <rtl/math.hxx>

using namespace connectivity;
using namespace connectivity::file;

const double fPi = 3.14159265358979323846;

ORowSetValue OOp_Abs::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    if ( nVal < 0 )
        nVal *= -1.0;
    return fabs(nVal);
}

ORowSetValue OOp_Sign::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    sal_Int32 nRet = 0;
    double nVal(lhs);
    if ( nVal < 0 )
        nRet = -1;
    else if ( nVal > 0 )
        nRet = 1;

    return nRet;
}

ORowSetValue OOp_Mod::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return ORowSetValue();

    return fmod(static_cast<double>(lhs),static_cast<double>(rhs));
}

ORowSetValue OOp_Floor::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return floor(static_cast<double>(lhs));
}

ORowSetValue OOp_Ceiling::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    return ceil(nVal);
}

ORowSetValue OOp_Round::operate(const std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() || lhs.size() > 2 )
        return ORowSetValue();

    size_t nSize = lhs.size();
    double nVal = lhs[nSize-1];

    sal_Int32 nDec = 0;
    if ( nSize == 2 && !lhs[0].isNull() )
        nDec = lhs[0];
    return ::rtl::math::round(nVal,nDec);
}

ORowSetValue OOp_Exp::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    return exp(nVal);
}

ORowSetValue OOp_Ln::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || static_cast<double>(lhs) < 0.0 )
        return lhs;

    double nVal(lhs);
    nVal = log(nVal);
    if ( std::isnan(nVal) )
        return ORowSetValue();
    return nVal;
}

ORowSetValue OOp_Log::operate(const std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() || lhs.size() > 2 )
        return ORowSetValue();
    size_t nSize = lhs.size();
    double nVal = log( static_cast<double>(lhs[nSize-1]) );


    if ( nSize == 2 && !lhs[0].isNull() )
        nVal /= log(static_cast<double>(lhs[0]));

    if ( std::isnan(nVal) )
        return ORowSetValue();
    return nVal;
}

ORowSetValue OOp_Log10::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || static_cast<double>(lhs) < 0.0 )
        return lhs;

    double nVal = log(static_cast<double>(lhs));
    if ( std::isnan(nVal) )
        return ORowSetValue();
    nVal /= log(10.0);
    return nVal;
}

ORowSetValue OOp_Pow::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return pow(static_cast<double>(lhs),static_cast<double>(rhs));
}

ORowSetValue OOp_Sqrt::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = sqrt(static_cast<double>(lhs));
    if ( std::isnan(nVal) )
        return ORowSetValue();
    return nVal;
}

ORowSetValue OOp_Pi::operate(const std::vector<ORowSetValue>& /*lhs*/) const
{
    return fPi;
}

ORowSetValue OOp_Cos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return cos(static_cast<double>(lhs));
}

ORowSetValue OOp_Sin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return sin(static_cast<double>(lhs));
}

ORowSetValue OOp_Tan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return tan(static_cast<double>(lhs));
}

ORowSetValue OOp_ACos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return acos(static_cast<double>(lhs));
}

ORowSetValue OOp_ASin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return asin(static_cast<double>(lhs));
}

ORowSetValue OOp_ATan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return atan(static_cast<double>(lhs));
}

ORowSetValue OOp_ATan2::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return atan2(static_cast<double>(lhs),static_cast<double>(rhs));
}

ORowSetValue OOp_Degrees::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs;
    return nLhs*180*(1.0/fPi);
}

ORowSetValue OOp_Radians::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs;
    return nLhs*fPi*(1.0/180.0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
