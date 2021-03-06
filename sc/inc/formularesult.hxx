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
#include "calcmacros.hxx"
#include <svl/sharedstring.hxx>
#include <formula/token.hxx>
#include <formula/types.hxx>

class ScMatrixFormulaCellToken;

namespace sc {

struct FormulaResultValue
{
    enum Type : sal_uInt8 { Invalid, Value, String, Error };

    double mfValue;
    svl::SharedString maString;
    Type meType;
    FormulaError mnError;

    FormulaResultValue();
    FormulaResultValue( double fValue );
    FormulaResultValue( const svl::SharedString& rStr );
    FormulaResultValue( FormulaError nErr );
};

}

/** Store a variable formula cell result, balancing between runtime performance
    and memory consumption. */
class ScFormulaResult
{
    typedef unsigned char Multiline;
    static const Multiline MULTILINE_UNKNOWN = 0;
    static const Multiline MULTILINE_FALSE   = 1;
    static const Multiline MULTILINE_TRUE    = 2;

    // Clone token if the 16-bit only reference counter is nearing it's
    // capacity during fill or copy&paste, leaving 4k for temporary passing
    // around. (That should be enough for all times (TM) ;-)
    static const sal_uInt16 MAX_TOKENREF_COUNT = 0xf000;
    static void IncrementTokenRef( const formula::FormulaToken* & rp )
    {
        if (rp)
        {
            if (rp->GetRef() >= MAX_TOKENREF_COUNT)
                rp = rp->Clone();
            rp->IncRef();
        }
    }

    union
    {
        double          mfValue;    // double result direct for performance and memory consumption
        const formula::FormulaToken*  mpToken;    // if not, result token obtained from interpreter
    };
    bool                mbToken :1; // whether content of union is a token
    bool                mbEmpty :1; // empty cell result
    bool                mbEmptyDisplayedAsString :1;    // only if mbEmpty
    // If set it implies that the result is a simple double (in mfValue) and no error
    bool                mbValueCached :1;
    Multiline           meMultiline :2; // result is multiline
    FormulaError        mnError;    // error code

    /** Reset mnError, mbEmpty and mbEmptyDisplayedAsString to their defaults
        prior to assigning other types */
    void ResetToDefaults();

    /** If token is of formula::svError set error code and decrement RefCount.
        If token is of formula::svEmptyCell set mbEmpty and mbEmptyAsString and
        decrement RefCount.
        If token is of formula::svDouble set mfValue and decrement RefCount.
        Else assign token to mpToken. NULL is valid => svUnknown.
        Other member variables are set accordingly.
        @precondition: Token MUST had been IncRef'ed prior to this call!
        @precondition: An already existing different mpToken MUST had been
        DecRef'ed prior to this call, p will be assigned to mpToken if not
        resolved.
        ATTENTION! Token may get deleted in this call! */
    void ResolveToken( const formula::FormulaToken * p );

public:
    /** Effectively type svUnknown. */
    ScFormulaResult();

    ScFormulaResult( const ScFormulaResult & r );

    /** Same comments as for SetToken() apply! */
    explicit ScFormulaResult( const formula::FormulaToken* p );

    ~ScFormulaResult();

    /** Well, guess what ... */
    ScFormulaResult& operator=( const ScFormulaResult & r );

    /** Assignment as in operator=() but without return */
    void Assign( const ScFormulaResult & r );

    /** Sets a direct double if token type is formula::svDouble, or mbEmpty if
        formula::svEmptyCell, else token. If p is NULL, that is set as well, effectively
        resulting in GetType()==svUnknown. If the already existing result is
        ScMatrixFormulaCellToken, the upper left is set to token.

        ATTENTION! formula::FormulaToken had to be allocated using 'new' and if of type
        formula::svDouble and no RefCount was set may not be used after this call
        because it was deleted after decrement! */
    void SetToken( const formula::FormulaToken* p );

    /** May be NULL if SetToken() did so, also if type formula::svDouble or formula::svError! */
    formula::FormulaConstTokenRef GetToken() const;

    /** Return upper left token if formula::svMatrixCell, else return GetToken().
        May be NULL if SetToken() did so, also if type formula::svDouble or formula::svError! */
    formula::FormulaConstTokenRef GetCellResultToken() const;

    /** Return type of result, including formula::svError, formula::svEmptyCell, formula::svDouble and
        formula::svMatrixCell. */
    formula::StackVar GetType() const;

    /** If type is formula::svMatrixCell return the type of upper left element, else
        GetType() */
    formula::StackVar GetCellResultType() const;

    /** If type is formula::svEmptyCell (including matrix upper left) and should be
        displayed as empty string */
    bool IsEmptyDisplayedAsString() const;

    /** Test for cell result type formula::svDouble, including upper left if
        formula::svMatrixCell. Also included is formula::svError for legacy, because previously
        an error result was treated like a numeric value at some places in
        ScFormulaCell. Also included is formula::svEmptyCell as a reference to an empty
        cell usually is treated as numeric 0. Use GetCellResultType() for
        details instead. */
    bool IsValue() const;

    bool IsValueNoError() const;

    /** Determines whether or not the result is a string containing more than
        one paragraph */
    bool IsMultiline() const;

    bool GetErrorOrDouble( FormulaError& rErr, double& rVal ) const;
    sc::FormulaResultValue GetResult() const;

    /** Get error code if set or GetCellResultType() is formula::svError or svUnknown,
        else 0. */
    FormulaError GetResultError() const;

    /** Set error code, don't touch token or double. */
    void SetResultError( FormulaError nErr );

    /** Set direct double. Shouldn't be used externally except in
        ScFormulaCell for rounded CalcAsShown or SetErrCode() or
        SetResultDouble(), see there for condition. If
        ScMatrixFormulaCellToken the token isn't replaced but upper
        left result is modified instead, but only if it was of type
        formula::svDouble before or not set at all.
     */
    void SetDouble( double f );

    /** Return value if type formula::svDouble or formula::svHybridCell or formula::svMatrixCell and upper
        left formula::svDouble, else 0.0 */
    double GetDouble() const;

    /** Return string if type formula::svString or formula::svHybridCell or formula::svMatrixCell and
        upper left formula::svString, else empty string. */
    const svl::SharedString & GetString() const;

    /** Return matrix if type formula::svMatrixCell and ScMatrix present, else NULL. */
    ScConstMatrixRef GetMatrix() const;

    /** Return formula string if type formula::svHybridCell, else empty string. */
    const OUString& GetHybridFormula() const;

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString(), or only SetHybridFormula() for
        formula string to be compiled later. */
    void SetHybridDouble( double f );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    void SetHybridString( const svl::SharedString & rStr );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridFormula(),
        SetHybridEmptyDisplayedAsString() must be last. */
    void SetHybridEmptyDisplayedAsString();

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    void SetHybridFormula( const OUString & rFormula );

    void SetMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL );

    /** Get the const ScMatrixFormulaCellToken* if token is of that type, else
        NULL. */
    const ScMatrixFormulaCellToken* GetMatrixFormulaCellToken() const;

    /** Get the ScMatrixFormulaCellToken* if token is of that type, else NULL.
        Shouldn't be used externally except by ScFormulaCell::SetMatColsRows(). */
    ScMatrixFormulaCellToken* GetMatrixFormulaCellTokenNonConst();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
