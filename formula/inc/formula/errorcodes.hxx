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

#ifndef SC_ERRORCODES_HXX
#define SC_ERRORCODES_HXX

#include <rtl/math.hxx>
#include <tools/solar.h>

namespace ScErrorCodes
{

const sal_uInt16 errIllegalChar          = 501;
const sal_uInt16 errIllegalArgument      = 502;
const sal_uInt16 errIllegalFPOperation   = 503; // #NUM!
const sal_uInt16 errIllegalParameter     = 504;
const sal_uInt16 errIllegalJump          = 505;
const sal_uInt16 errSeparator            = 506;
const sal_uInt16 errPair                 = 507;
const sal_uInt16 errPairExpected         = 508;
const sal_uInt16 errOperatorExpected     = 509;
const sal_uInt16 errVariableExpected     = 510;
const sal_uInt16 errParameterExpected    = 511;
const sal_uInt16 errCodeOverflow         = 512;
const sal_uInt16 errStringOverflow       = 513;
const sal_uInt16 errStackOverflow        = 514;
const sal_uInt16 errUnknownState         = 515;
const sal_uInt16 errUnknownVariable      = 516;
const sal_uInt16 errUnknownOpCode        = 517;
const sal_uInt16 errUnknownStackVariable = 518;
const sal_uInt16 errNoValue              = 519; // #VALUE!
const sal_uInt16 errUnknownToken         = 520;
const sal_uInt16 errNoCode               = 521; // #NULL!
const sal_uInt16 errCircularReference    = 522;
const sal_uInt16 errNoConvergence        = 523;
const sal_uInt16 errNoRef                = 524; // #REF!
const sal_uInt16 errNoName               = 525; // #NAME?
const sal_uInt16 errDoubleRef            = 526;
const sal_uInt16 errInterpOverflow       = 527;
// Not displayed, temporary for TrackFormulas,
// Cell depends on another cell that has errCircularReference
const sal_uInt16 errTrackFromCircRef     = 528;
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// set as mnStringNoValueError no error is generated but 0 returned.
const sal_uInt16 errCellNoValue          = 529;
// Interpreter: needed AddIn not found
const sal_uInt16 errNoAddin              = 530;
// Interpreter: needed Macro not found
const sal_uInt16 errNoMacro              = 531;
// Interpreter: Division by zero
const sal_uInt16 errDivisionByZero       = 532; // #DIV/0!
// Compiler: a non-simple (str,err,val) value was put in an array
const sal_uInt16 errNestedArray          = 533;
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// temporarily (!) set as mnStringNoValueError, the error is generated and can
// be used to distinguish that condition from all other (inherited) errors. Do
// not use for anything else! Never push or inherit the error otherwise!
const sal_uInt16 errNotNumericString     = 534;

// Interpreter: NA() not available condition, not a real error
const sal_uInt16 NOTAVAILABLE            = 0x7fff;


/** Unconditionally construct a double value of NAN where the lower bits
    represent an interpreter error code. */
inline double CreateDoubleError( sal_uInt16 nErr )
{
    union
    {
        double fVal;
        sal_math_Double smVal;
    };
    ::rtl::math::setNan( &fVal );
    smVal.nan_parts.fraction_lo = nErr;
    return fVal;
}


/** Recreate the error code of a coded double error, if any. */
inline sal_uInt16 GetDoubleErrorValue( double fVal )
{
    if ( ::rtl::math::isFinite( fVal ) )
        return 0;
    if ( ::rtl::math::isInf( fVal ) )
        return errIllegalFPOperation;       // normal INF
    sal_uInt32 nErr = reinterpret_cast< sal_math_Double * >(
            &fVal)->nan_parts.fraction_lo;
    if ( nErr & 0xffff0000 )
        return errNoValue;                  // just a normal NAN
    return (sal_uInt16)(nErr & 0x0000ffff);     // any other error
}

} // namespace ScErrorCodes

// yes, exceptionally we put a "using namespace" in a header file..
using namespace ScErrorCodes;

#endif // SC_ERRORCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
