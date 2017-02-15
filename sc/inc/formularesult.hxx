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

#ifndef INCLUDED_SC_INC_FORMULARESULT_HXX
#define INCLUDED_SC_INC_FORMULARESULT_HXX

#include "token.hxx"
#include "scdllapi.h"

namespace sc {

struct FormulaResultValue
{
    enum Type { Invalid, Value, String, Error };

    Type meType;

    double mfValue;
    svl::SharedString maString;
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
    FormulaError        mnError;    // error code
    bool                mbToken :1; // whether content of union is a token
    bool                mbEmpty :1; // empty cell result
    bool                mbEmptyDisplayedAsString :1;    // only if mbEmpty
    Multiline           meMultiline :2; // result is multiline

    /** Reset mnError, mbEmpty and mbEmptyDisplayedAsString to their defaults
        prior to assigning other types */
    void ResetToDefaults();

    /** If token is of formula::StackVar::Error set error code and decrement RefCount.
        If token is of formula::StackVar::EmptyCell set mbEmpty and mbEmptyAsString and
        decrement RefCount.
        If token is of formula::StackVar::Double set mfValue and decrement RefCount.
        Else assign token to mpToken. NULL is valid => StackVar::Unknown.
        Other member variables are set accordingly.
        @precondition: Token MUST had been IncRef'ed prior to this call!
        @precondition: An already existing different mpToken MUST had been
        DecRef'ed prior to this call, p will be assigned to mpToken if not
        resolved.
        ATTENTION! Token may get deleted in this call! */
    void ResolveToken( const formula::FormulaToken * p );

public:
    /** Effectively type StackVar::Unknown. */
    ScFormulaResult();

    ScFormulaResult( const ScFormulaResult & r );

    /** Same comments as for SetToken() apply! */
    explicit ScFormulaResult( const formula::FormulaToken* p );

    ~ScFormulaResult();

    /** Well, guess what ... */
    ScFormulaResult& operator=( const ScFormulaResult & r );

    /** Assignment as in operator=() but without return */
    void Assign( const ScFormulaResult & r );

    /** Sets a direct double if token type is formula::StackVar::Double, or mbEmpty if
        formula::StackVar::EmptyCell, else token. If p is NULL, that is set as well, effectively
        resulting in GetType()==StackVar::Unknown. If the already existing result is
        ScMatrixFormulaCellToken, the upper left is set to token.

        ATTENTION! formula::FormulaToken had to be allocated using 'new' and if of type
        formula::StackVar::Double and no RefCount was set may not be used after this call
        because it was deleted after decrement! */
    void SetToken( const formula::FormulaToken* p );

    /** May be NULL if SetToken() did so, also if type formula::StackVar::Double or formula::StackVar::Error! */
    formula::FormulaConstTokenRef GetToken() const;

    /** Return upper left token if formula::StackVar::MatrixCell, else return GetToken().
        May be NULL if SetToken() did so, also if type formula::StackVar::Double or formula::StackVar::Error! */
    formula::FormulaConstTokenRef GetCellResultToken() const;

    /** Return type of result, including formula::StackVar::Error, formula::StackVar::EmptyCell, formula::StackVar::Double and
        formula::StackVar::MatrixCell. */
    formula::StackVar GetType() const;

    /** If type is formula::StackVar::MatrixCell return the type of upper left element, else
        GetType() */
    formula::StackVar GetCellResultType() const;

    /** If type is formula::StackVar::EmptyCell (including matrix upper left) and should be
        displayed as empty string */
    bool IsEmptyDisplayedAsString() const;

    /** Test for cell result type formula::StackVar::Double, including upper left if
        formula::StackVar::MatrixCell. Also included is formula::StackVar::Error for legacy, because previously
        an error result was treated like a numeric value at some places in
        ScFormulaCell. Also included is formula::StackVar::EmptyCell as a reference to an empty
        cell usually is treated as numeric 0. Use GetCellResultType() for
        details instead. */
    bool IsValue() const;

    bool IsValueNoError() const;

    /** Determines whether or not the result is a string containing more than
        one paragraph */
    bool IsMultiline() const;

    bool GetErrorOrDouble( FormulaError& rErr, double& rVal ) const;
    sc::FormulaResultValue GetResult() const;

    /** Get error code if set or GetCellResultType() is formula::StackVar::Error or StackVar::Unknown,
        else 0. */
    FormulaError GetResultError() const;

    /** Set error code, don't touch token or double. */
    void SetResultError( FormulaError nErr );

    /** Set direct double. Shouldn't be used externally except in
        ScFormulaCell for rounded CalcAsShown or SetErrCode() or
        SetResultDouble(), see there for condition. If
        ScMatrixFormulaCellToken the token isn't replaced but upper
        left result is modified instead, but only if it was of type
        formula::StackVar::Double before or not set at all.
     */
    SC_DLLPUBLIC void SetDouble( double f );

    /** Return value if type formula::StackVar::Double or formula::StackVar::HybridCell or formula::StackVar::MatrixCell and upper
        left formula::StackVar::Double, else 0.0 */
    double GetDouble() const;

    /** Return string if type formula::StackVar::String or formula::StackVar::HybridCell or formula::StackVar::MatrixCell and
        upper left formula::StackVar::String, else empty string. */
    svl::SharedString GetString() const;

    /** Return matrix if type formula::StackVar::MatrixCell and ScMatrix present, else NULL. */
    ScConstMatrixRef GetMatrix() const;

    /** Return formula string if type formula::StackVar::HybridCell, else empty string. */
    const OUString& GetHybridFormula() const;

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString(), or only SetHybridFormula() for
        formula string to be compiled later. */
    SC_DLLPUBLIC void SetHybridDouble( double f );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    SC_DLLPUBLIC void SetHybridString( const svl::SharedString & rStr );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridFormula(),
        SetHybridEmptyDisplayedAsString() must be last. */
    SC_DLLPUBLIC void SetHybridEmptyDisplayedAsString();

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    SC_DLLPUBLIC void SetHybridFormula( const OUString & rFormula );

    SC_DLLPUBLIC void SetMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL );

    /** Get the const ScMatrixFormulaCellToken* if token is of that type, else
        NULL. */
    const ScMatrixFormulaCellToken* GetMatrixFormulaCellToken() const;

    /** Get the ScMatrixFormulaCellToken* if token is of that type, else NULL.
        Shouldn't be used externally except by ScFormulaCell::SetMatColsRows(). */
    ScMatrixFormulaCellToken* GetMatrixFormulaCellTokenNonConst();
};

#endif // INCLUDED_SC_INC_FORMULARESULT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
