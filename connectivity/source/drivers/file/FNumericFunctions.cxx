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
#include <basegfx/numeric/ftools.hxx>
#include <file/FNumericFunctions.hxx>
#include <rtl/math.hxx>

using namespace connectivity;
using namespace connectivity::file;

ORowSetValue OOp_Abs::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = lhs.getDouble();
    if ( nVal < 0 )
        nVal *= -1.0;
    return fabs(nVal);
}

ORowSetValue OOp_Sign::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    sal_Int32 nRet = 0;
    double nVal = lhs.getDouble();
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

    return fmod(lhs.getDouble(), rhs.getDouble());
}

ORowSetValue OOp_Floor::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return floor(lhs.getDouble());
}

ORowSetValue OOp_Ceiling::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = lhs.getDouble();
    return ceil(nVal);
}

ORowSetValue OOp_Round::operate(const std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() || lhs.size() > 2 )
        return ORowSetValue();

    size_t nSize = lhs.size();
    double nVal = lhs[nSize-1].getDouble();

    sal_Int32 nDec = 0;
    if ( nSize == 2 && !lhs[0].isNull() )
        nDec = lhs[0].getDouble();
    return ::rtl::math::round(nVal,nDec);
}

ORowSetValue OOp_Exp::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = lhs.getDouble();
    return exp(nVal);
}

ORowSetValue OOp_Ln::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || lhs.getDouble() < 0.0 )
        return lhs;

    double nVal = lhs.getDouble();
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
    double nVal = log( lhs[nSize-1].getDouble() );


    if ( nSize == 2 && !lhs[0].isNull() )
        nVal /= log(lhs[0].getDouble());

    if ( std::isnan(nVal) )
        return ORowSetValue();
    return nVal;
}

ORowSetValue OOp_Log10::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || lhs.getDouble() < 0.0 )
        return lhs;

    double nVal = log(lhs.getDouble());
    if ( std::isnan(nVal) )
        return ORowSetValue();
    nVal /= log(10.0);
    return nVal;
}

ORowSetValue OOp_Pow::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return pow(lhs.getDouble(), rhs.getDouble());
}

ORowSetValue OOp_Sqrt::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = sqrt(lhs.getDouble());
    if ( std::isnan(nVal) )
        return ORowSetValue();
    return nVal;
}

ORowSetValue OOp_Pi::operate(const std::vector<ORowSetValue>& /*lhs*/) const
{
    return M_PI;
}

ORowSetValue OOp_Cos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return cos(lhs.getDouble());
}

ORowSetValue OOp_Sin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return sin(lhs.getDouble());
}

ORowSetValue OOp_Tan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return tan(lhs.getDouble());
}

ORowSetValue OOp_ACos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return acos(lhs.getDouble());
}

ORowSetValue OOp_ASin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return asin(lhs.getDouble());
}

ORowSetValue OOp_ATan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return atan(lhs.getDouble());
}

ORowSetValue OOp_ATan2::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return atan2(lhs.getDouble(), rhs.getDouble());
}

ORowSetValue OOp_Degrees::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs.getDouble();
    return basegfx::rad2deg(nLhs);
}

ORowSetValue OOp_Radians::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs.getDouble();
    return basegfx::deg2rad(nLhs);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
