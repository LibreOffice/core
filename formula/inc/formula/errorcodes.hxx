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

const USHORT errIllegalChar          = 501;
const USHORT errIllegalArgument      = 502;
const USHORT errIllegalFPOperation   = 503; // #NUM!
const USHORT errIllegalParameter     = 504;
const USHORT errIllegalJump          = 505;
const USHORT errSeparator            = 506;
const USHORT errPair                 = 507;
const USHORT errPairExpected         = 508;
const USHORT errOperatorExpected     = 509;
const USHORT errVariableExpected     = 510;
const USHORT errParameterExpected    = 511;
const USHORT errCodeOverflow         = 512;
const USHORT errStringOverflow       = 513;
const USHORT errStackOverflow        = 514;
const USHORT errUnknownState         = 515;
const USHORT errUnknownVariable      = 516;
const USHORT errUnknownOpCode        = 517;
const USHORT errUnknownStackVariable = 518;
const USHORT errNoValue              = 519; // #VALUE!
const USHORT errUnknownToken         = 520;
const USHORT errNoCode               = 521; // #NULL!
const USHORT errCircularReference    = 522;
const USHORT errNoConvergence        = 523;
const USHORT errNoRef                = 524; // #REF!
const USHORT errNoName               = 525; // #NAME?
const USHORT errDoubleRef            = 526;
const USHORT errInterpOverflow       = 527;
// Not displayed, temporary for TrackFormulas,
// Cell depends on another cell that has errCircularReference
const USHORT errTrackFromCircRef     = 528;
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// set as mnStringNoValueError no error is generated but 0 returned.
const USHORT errCellNoValue          = 529;
// Interpreter: needed AddIn not found
const USHORT errNoAddin              = 530;
// Interpreter: needed Macro not found
const USHORT errNoMacro              = 531;
// Interpreter: Division by zero
const USHORT errDivisionByZero       = 532; // #DIV/0!
// Compiler: a non-simple (str,err,val) value was put in an array
const USHORT errNestedArray          = 533;
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// temporarily (!) set as mnStringNoValueError, the error is generated and can
// be used to distinguish that condition from all other (inherited) errors. Do
// not use for anything else! Never push or inherit the error otherwise!
const USHORT errNotNumericString     = 534;

// Interpreter: NA() not available condition, not a real error
const USHORT NOTAVAILABLE            = 0x7fff;


/** Unconditionally construct a double value of NAN where the lower bits
    represent an interpreter error code. */
inline double CreateDoubleError( USHORT nErr )
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
inline USHORT GetDoubleErrorValue( double fVal )
{
    if ( ::rtl::math::isFinite( fVal ) )
        return 0;
    if ( ::rtl::math::isInf( fVal ) )
        return errIllegalFPOperation;       // normal INF
    UINT32 nErr = reinterpret_cast< sal_math_Double * >(
            &fVal)->nan_parts.fraction_lo;
    if ( nErr & 0xffff0000 )
        return errNoValue;                  // just a normal NAN
    return (USHORT)(nErr & 0x0000ffff);     // any other error
}

} // namespace ScErrorCodes

// yes, exceptionally we put a "using namespace" in a header file..
using namespace ScErrorCodes;

#endif // SC_ERRORCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
