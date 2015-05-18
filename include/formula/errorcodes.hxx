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

#ifndef INCLUDED_FORMULA_ERRORCODES_HXX
#define INCLUDED_FORMULA_ERRORCODES_HXX

#include <rtl/math.hxx>

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
// ScInterpreter internal:  jump matrix already has a result at this position,
// do not overwrite in case of empty code path.
const sal_uInt16 errJumpMatHasResult     = 535;
// ScInterpreter internal:  (matrix) element is not a numeric value, i.e.
// string or empty, to be distinguished from the general errNoValue NAN and not
// to be used as result.
const sal_uInt16 errElementNaN           = 536;

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

}

// yes, exceptionally we put a "using namespace" in a header file..
using namespace ScErrorCodes;

#endif // INCLUDED_FORMULA_ERRORCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
