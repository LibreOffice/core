/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include <cmath>
#include "file/FNumericFunctions.hxx"
#include <rtl/math.hxx>

using namespace connectivity;
using namespace connectivity::file;
//------------------------------------------------------------------
ORowSetValue OOp_Abs::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    if ( nVal < 0 )
        nVal *= -1.0;
    return fabs(nVal);
}
//------------------------------------------------------------------
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
//------------------------------------------------------------------
ORowSetValue OOp_Mod::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return ORowSetValue();

    return fmod((double)lhs,(double)rhs);
}
//------------------------------------------------------------------
ORowSetValue OOp_Floor::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return floor((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Ceiling::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    return ceil(nVal);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Round::operate(const ::std::vector<ORowSetValue>& lhs) const
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
// -----------------------------------------------------------------------------
ORowSetValue OOp_Exp::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal(lhs);
    return exp(nVal);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Ln::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || static_cast<double>(lhs) < 0.0 )
        return lhs;

    double nVal(lhs);
    nVal = log(nVal);
    if ( rtl::math::isNan(nVal) )
        return ORowSetValue();
    return nVal;
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Log::operate(const ::std::vector<ORowSetValue>& lhs) const
{
    if ( lhs.empty() || lhs.size() > 2 )
        return ORowSetValue();
    size_t nSize = lhs.size();
    double nVal = log( (double)lhs[nSize-1] );


    if ( nSize == 2 && !lhs[0].isNull() )
        nVal /= log((double)lhs[0]);

    if ( rtl::math::isNan(nVal) )
        return ORowSetValue();
    return nVal;
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Log10::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() || static_cast<double>(lhs) < 0.0 )
        return lhs;

    double nVal = log((double)lhs);
    if ( rtl::math::isNan(nVal) )
        return ORowSetValue();
    nVal /= log(10.0);
    return nVal;
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Pow::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return pow((double)lhs,(double)rhs);
}
//------------------------------------------------------------------
ORowSetValue OOp_Sqrt::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nVal = sqrt((double)lhs);
    if ( rtl::math::isNan(nVal) )
        return ORowSetValue();
    return nVal;
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Pi::operate(const ::std::vector<ORowSetValue>& /*lhs*/) const
{
    return 3.141592653589793116;
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Cos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return cos((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Sin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return sin((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Tan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return tan((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_ACos::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return acos((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_ASin::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return asin((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_ATan::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    return atan((double)lhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_ATan2::operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const
{
    if ( lhs.isNull() || rhs.isNull() )
        return lhs;

    return atan2((double)lhs,(double)rhs);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Degrees::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs;
    return nLhs*180*(1.0/3.141592653589793116);
}
// -----------------------------------------------------------------------------
ORowSetValue OOp_Radians::operate(const ORowSetValue& lhs) const
{
    if ( lhs.isNull() )
        return lhs;

    double nLhs = lhs;
    return nLhs*3.141592653589793116*(1.0/180.0);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
