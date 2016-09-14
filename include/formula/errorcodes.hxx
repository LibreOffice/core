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
#include <sal/mathconf.h>
#include <sal/types.h>

enum class FormulaError
{
    NONE                 = 0,

    // these 3 are only used by the starcalc filter in sc/source/filter/starcalc/*
    UnknownFormat        = 1,
    UnknownID            = 2,
    OutOfMemory          = 3,

    IllegalChar          = 501,
    IllegalArgument      = 502,
    IllegalFPOperation   = 503, // #NUM!
    IllegalParameter     = 504,
    IllegalJump          = 505,
    Separator            = 506,
    Pair                 = 507,
    PairExpected         = 508,
    OperatorExpected     = 509,
    VariableExpected     = 510,
    ParameterExpected    = 511,
    CodeOverflow         = 512,
    StringOverflow       = 513,
    StackOverflow        = 514,
    UnknownState         = 515,
    UnknownVariable      = 516,
    UnknownOpCode        = 517,
    UnknownStackVariable = 518,
    NoValue              = 519, // #VALUE!
    UnknownToken         = 520,
    NoCode               = 521, // #NULL!
    CircularReference    = 522,
    NoConvergence        = 523,
    NoRef                = 524, // #REF!
    NoName               = 525, // #NAME?
    DoubleRef            = 526,
// Not displayed, temporary for TrackFormulas,
// Cell depends on another cell that has FormulaError::CircularReference
    TrackFromCircRef     = 528,
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// set as mnStringNoValueError no error is generated but 0 returned.
    CellNoValue          = 529,
// Interpreter: needed AddIn not found
    NoAddin              = 530,
// Interpreter: needed Macro not found
    NoMacro              = 531,
// Interpreter: Division by zero
    DivisionByZero       = 532, // #DIV/0!
// Compiler: a non-simple (str,err,val) value was put in an array
    NestedArray          = 533,
// ScInterpreter internal:  no numeric value but numeric queried. If this is
// temporarily (!) set as mnStringNoValueError, the error is generated and can
// be used to distinguish that condition from all other (inherited) errors. Do
// not use for anything else! Never push or inherit the error otherwise!
    NotNumericString     = 534,
// ScInterpreter internal:  jump matrix already has a result at this position,
// do not overwrite in case of empty code path.
    JumpMatHasResult     = 535,
// ScInterpreter internal:  (matrix) element is not a numeric value, i.e.
// string or empty, to be distinguished from the general FormulaError::NoValue NAN and not
// to be used as result.
    ElementNaN           = 536,
// ScInterpreter/ScFormulaCell internal:  keep dirty, retry interpreting next
// round.
    RetryCircular        = 537,
// If matrix could not be allocated.
    MatrixSize           = 538,

// Interpreter: NA() not available condition, not a real error
    NotAvailable         = 0x7fff
};

namespace ScErrorCodes
{

/** Unconditionally construct a double value of NAN where the lower bits
    represent an interpreter error code. */
inline double CreateDoubleError( FormulaError nErr )
{
    sal_math_Double smVal;
    ::rtl::math::setNan( &smVal.value );
    smVal.nan_parts.fraction_lo = static_cast<sal_uInt16>(nErr);
    return smVal.value;
}

/** Recreate the error code of a coded double error, if any. */
inline FormulaError GetDoubleErrorValue( double fVal )
{
    if ( ::rtl::math::isFinite( fVal ) )
        return FormulaError::NONE;
    if ( ::rtl::math::isInf( fVal ) )
        return FormulaError::IllegalFPOperation;       // normal INF
    sal_uInt32 nErr = reinterpret_cast< sal_math_Double * >( &fVal)->nan_parts.fraction_lo;
    if ( nErr & 0xffff0000 )
        return FormulaError::NoValue;                  // just a normal NAN
    if (!nErr)
        // Another NAN, e.g. -nan(0x8000000000000) from calculating with -inf
        return FormulaError::IllegalFPOperation;
    // Any other error known to us as error code.
    return (FormulaError)(nErr & 0x0000ffff);
}

} // namespace ScErrorCodes

#endif // INCLUDED_FORMULA_ERRORCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
