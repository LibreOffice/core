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

#ifndef SC_FORMULARESULT_HXX
#define SC_FORMULARESULT_HXX

#include "token.hxx"


/** Store a variable formula cell result, balancing between runtime performance
    and memory consumption. */
class ScFormulaResult
{
    typedef unsigned char Multiline;
    static const Multiline MULTILINE_UNKNOWN = 0;
    static const Multiline MULTILINE_FALSE   = 1;
    static const Multiline MULTILINE_TRUE    = 2;

    union
    {
        double          mfValue;    // double result direct for performance and memory consumption
        const formula::FormulaToken*  mpToken;    // if not, result token obtained from interpreter
    };
    sal_uInt16              mnError;    // error code
    bool                mbToken :1; // whether content of union is a token
    bool                mbEmpty :1; // empty cell result
    bool                mbEmptyDisplayedAsString :1;    // only if mbEmpty
    Multiline           meMultiline :2; // result is multiline

    /** Reset mnError, mbEmpty and mbEmptyDisplayedAsString to their defaults
        prior to assigning other types */
    inline  void                ResetToDefaults();

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
    inline  void                ResolveToken( const formula::FormulaToken * p );

public:
                                /** Effectively type svUnknown. */
                                ScFormulaResult()
                                    : mpToken(NULL), mnError(0), mbToken(true),
                                    mbEmpty(false), mbEmptyDisplayedAsString(false),
                                    meMultiline(MULTILINE_UNKNOWN) {}

                                ScFormulaResult( const ScFormulaResult & r )
                                    : mnError( r.mnError), mbToken( r.mbToken),
                                    mbEmpty( r.mbEmpty),
                                    mbEmptyDisplayedAsString( r.mbEmptyDisplayedAsString),
                                    meMultiline( r.meMultiline)
                                {
                                    if (mbToken)
                                    {
                                        mpToken = r.mpToken;
                                        if (mpToken)
                                        {
                                            // Since matrix dimension and
                                            // results are assigned to a matrix
                                            // cell formula token we have to
                                            // clone that instead of sharing it.
                                            const ScMatrixFormulaCellToken* pMatFormula =
                                                r.GetMatrixFormulaCellToken();
                                            if (pMatFormula)
                                                mpToken = new ScMatrixFormulaCellToken( *pMatFormula);
                                            mpToken->IncRef();
                                        }
                                    }
                                    else
                                        mfValue = r.mfValue;
                                }

    /** Same comments as for SetToken() apply! */
    explicit                    ScFormulaResult( const formula::FormulaToken* p )
                                    : mnError(0), mbToken(false),
                                    mbEmpty(false), mbEmptyDisplayedAsString(false),
                                    meMultiline(MULTILINE_UNKNOWN)
                                {
                                    SetToken( p);
                                }

                                ~ScFormulaResult()
                                {
                                    if (mbToken && mpToken)
                                        mpToken->DecRef();
                                }

    /** Well, guess what ... */
    inline  ScFormulaResult &   operator=( const ScFormulaResult & r );

    /** Assignment as in operator=() but without return */
    inline  void                Assign( const ScFormulaResult & r );

    /** Sets a direct double if token type is formula::svDouble, or mbEmpty if
        formula::svEmptyCell, else token. If p is NULL, that is set as well, effectively
        resulting in GetType()==svUnknown. If the already existing result is
        ScMatrixFormulaCellToken, the upper left ist set to token.

        ATTENTION! formula::FormulaToken had to be allocated using 'new' and if of type
        formula::svDouble and no RefCount was set may not be used after this call
        because it was deleted after decrement! */
    inline  void                SetToken( const formula::FormulaToken* p );

    /** May be NULL if SetToken() did so, also if type formula::svDouble or formula::svError! */
    inline  formula::FormulaConstTokenRef     GetToken() const;

    /** Return upper left token if formula::svMatrixCell, else return GetToken().
        May be NULL if SetToken() did so, also if type formula::svDouble or formula::svError! */
    inline  formula::FormulaConstTokenRef     GetCellResultToken() const;

    /** Return type of result, including formula::svError, formula::svEmptyCell, formula::svDouble and
        formula::svMatrixCell. */
    inline  formula::StackVar            GetType() const;

    /** If type is formula::svMatrixCell return the type of upper left element, else
        GetType() */
    inline  formula::StackVar            GetCellResultType() const;

    /** If type is formula::svEmptyCell (including matrix upper left) and should be
        displayed as empty string */
    inline  bool                IsEmptyDisplayedAsString() const;

    /** Test for cell result type formula::svDouble, including upper left if
        formula::svMatrixCell. Also included is formula::svError for legacy, because previously
        an error result was treated like a numeric value at some places in
        ScFormulaCell. Also included is formula::svEmptyCell as a reference to an empty
        cell usually is treated as numeric 0. Use GetCellResultType() for
        details instead. */
    inline  bool                IsValue() const;

    /** Determines whether or not the result is a string containing more than
        one paragraph */
    inline  bool                IsMultiline() const;

    /** Get error code if set or GetCellResultType() is formula::svError or svUnknown,
        else 0. */
    inline  sal_uInt16              GetResultError() const;

    /** Set error code, don't touch token or double. */
    inline  void                SetResultError( sal_uInt16 nErr );

    /** Set direct double. Shouldn't be used externally except in
        ScFormulaCell for rounded CalcAsShown or SetErrCode(). If
        ScMatrixFormulaCellToken the token isn't replaced but upper left result
        is modified instead, but only if it was of type formula::svDouble before or not
        set at all. */
    inline  void                SetDouble( double f );

    /** Return value if type formula::svDouble or formula::svHybridCell or formula::svMatrixCell and upper
        left formula::svDouble, else 0.0 */
    inline  double              GetDouble() const;

    /** Return string if type formula::svString or formula::svHybridCell or formula::svMatrixCell and
        upper left formula::svString, else empty string. */
    inline  const String &      GetString() const;

    /** Return matrix if type formula::svMatrixCell and ScMatrix present, else NULL. */
    inline  ScConstMatrixRef    GetMatrix() const;

    /** Return formula string if type formula::svHybridCell, else empty string. */
    inline  const String &      GetHybridFormula() const;

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString(), or only SetHybridString() for
        formula string to be compiled later. */
    inline  void                SetHybridDouble( double f );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    inline  void                SetHybridString( const String & rStr );

    /** Should only be used by import filters, best in the order
        SetHybridDouble(), SetHybridString()/SetHybridFormula(), or only
        SetHybridFormula() for formula string to be compiled later. */
    inline  void                SetHybridFormula( const String & rFormula );

    /** Get the const ScMatrixFormulaCellToken* if token is of that type, else
        NULL. */
    inline const ScMatrixFormulaCellToken* GetMatrixFormulaCellToken() const;

    /** Get the ScMatrixFormulaCellToken* if token is of that type, else NULL.
        Shouldn't be used externally except by ScFormulaCell::SetMatColsRows(). */
    inline ScMatrixFormulaCellToken* GetMatrixFormulaCellTokenNonConst();
};


inline void ScFormulaResult::ResetToDefaults()
{
    mnError = 0;
    mbEmpty = false;
    mbEmptyDisplayedAsString = false;
    meMultiline = MULTILINE_UNKNOWN;
}


inline void ScFormulaResult::ResolveToken( const formula::FormulaToken * p )
{
    ResetToDefaults();
    if (!p)
    {
        mpToken = p;
        mbToken = true;
    }
    else
    {
        switch (p->GetType())
        {
            case formula::svError:
                mnError = p->GetError();
                p->DecRef();
                mbToken = false;
                // set in case mnError is 0 now, which shouldn't happen but ...
                mfValue = 0.0;
                meMultiline = MULTILINE_FALSE;
                break;
            case formula::svEmptyCell:
                mbEmpty = true;
                mbEmptyDisplayedAsString = static_cast<const ScEmptyCellToken*>(p)->IsDisplayedAsString();
                p->DecRef();
                mbToken = false;
                meMultiline = MULTILINE_FALSE;
                break;
            case formula::svDouble:
                mfValue = p->GetDouble();
                p->DecRef();
                mbToken = false;
                meMultiline = MULTILINE_FALSE;
                break;
            default:
                mpToken = p;
                mbToken = true;
        }
    }
}


inline ScFormulaResult & ScFormulaResult::operator=( const ScFormulaResult & r )
{
    Assign( r);
    return *this;
}


inline void ScFormulaResult::Assign( const ScFormulaResult & r )
{
    if (this == &r)
        return;
    if (r.mbEmpty)
    {
        if (mbToken && mpToken)
            mpToken->DecRef();
        mbToken = false;
        mbEmpty = true;
        mbEmptyDisplayedAsString = r.mbEmptyDisplayedAsString;
        meMultiline = r.meMultiline;
    }
    else if (r.mbToken)
    {
        // Matrix formula cell token must be cloned, see copy-ctor.
        const ScMatrixFormulaCellToken* pMatFormula =
            r.GetMatrixFormulaCellToken();
        if (pMatFormula)
            SetToken( new ScMatrixFormulaCellToken( *pMatFormula));
        else
            SetToken( r.mpToken);
    }
    else
        SetDouble( r.mfValue);
    // If there was an error there will be an error, no matter what Set...()
    // methods did.
    mnError = r.mnError;
}


inline void ScFormulaResult::SetToken( const formula::FormulaToken* p )
{
    ResetToDefaults();
    if (p)
        p->IncRef();
    // Handle a result obtained from the interpreter to be assigned to a matrix
    // formula cell's ScMatrixFormulaCellToken.
    ScMatrixFormulaCellToken* pMatFormula = GetMatrixFormulaCellTokenNonConst();
    if (pMatFormula)
    {
        const ScMatrixCellResultToken* pMatResult =
            (p && p->GetType() == formula::svMatrixCell ?
             dynamic_cast<const ScMatrixCellResultToken*>(p) : NULL);
        if (pMatResult)
        {
            const ScMatrixFormulaCellToken* pNewMatFormula =
                dynamic_cast<const ScMatrixFormulaCellToken*>(pMatResult);
            if (pNewMatFormula)
            {
                DBG_ERRORFILE( "ScFormulaResult::SetToken: pNewMatFormula and pMatFormula, overriding matrix formula dimension; intended?");
                pMatFormula->SetMatColsRows( pNewMatFormula->GetMatCols(),
                        pNewMatFormula->GetMatRows());
            }
            pMatFormula->Assign( *pMatResult);
            p->DecRef();
        }
        else if (p)
        {
            // This may be the result of some constant expression like
            // {="string"} that doesn't result in a matrix but still would
            // display the result in all cells of this matrix formula.
            pMatFormula->Assign( *p);
            p->DecRef();
        }
        else
        {
            // NULL result? Well, if you say so ...
            pMatFormula->ResetResult();
        }
    }
    else
    {
        if (mbToken && mpToken)
            mpToken->DecRef();
        ResolveToken( p);
    }
}


inline void ScFormulaResult::SetDouble( double f )
{
    ResetToDefaults();
    // Handle a result obtained from the interpreter to be assigned to a matrix
    // formula cell's ScMatrixFormulaCellToken.
    ScMatrixFormulaCellToken* pMatFormula = GetMatrixFormulaCellTokenNonConst();
    if (pMatFormula)
        pMatFormula->SetUpperLeftDouble( f);
    else
    {
        if (mbToken && mpToken)
            mpToken->DecRef();
        mfValue = f;
        mbToken = false;
        meMultiline = MULTILINE_FALSE;
    }
}


inline formula::StackVar ScFormulaResult::GetType() const
{
    // Order is significant.
    if (mnError)
        return formula::svError;
    if (mbEmpty)
        return formula::svEmptyCell;
    if (!mbToken)
        return formula::svDouble;
    if (mpToken)
        return mpToken->GetType();
    return formula::svUnknown;
}


inline formula::StackVar ScFormulaResult::GetCellResultType() const
{
    formula::StackVar sv = GetType();
    if (sv == formula::svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        sv = static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftType();
    return sv;
}


inline bool ScFormulaResult::IsEmptyDisplayedAsString() const
{
    if (mbEmpty)
        return mbEmptyDisplayedAsString;
    if (GetType() == formula::svMatrixCell)
    {
        // don't need to test for mpToken here, GetType() already did it
        const ScEmptyCellToken* p = dynamic_cast<const ScEmptyCellToken*>(
                static_cast<const ScMatrixCellResultToken*>(
                    mpToken)->GetUpperLeftToken().operator->());
        if (p)
            return p->IsDisplayedAsString();
    }
    return false;
}


inline bool ScFormulaResult::IsValue() const
{
    formula::StackVar sv = GetCellResultType();
    return sv == formula::svDouble || sv == formula::svError || sv == formula::svEmptyCell;
}

inline bool ScFormulaResult::IsMultiline() const
{
    if (meMultiline == MULTILINE_UNKNOWN)
    {
        const String& rStr = GetString();
        if (rStr.Len() && rStr.Search( _LF ) != STRING_NOTFOUND)
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_TRUE;
        else
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_FALSE;
    }
    return meMultiline == MULTILINE_TRUE;
}


inline sal_uInt16 ScFormulaResult::GetResultError() const
{
    if (mnError)
        return mnError;
    formula::StackVar sv = GetCellResultType();
    if (sv == formula::svError)
    {
        if (GetType() == formula::svMatrixCell)
            // don't need to test for mpToken here, GetType() already did it
            return static_cast<const ScMatrixCellResultToken*>(mpToken)->
                GetUpperLeftToken()->GetError();
        if (mpToken)
            return mpToken->GetError();
    }
    return 0;
}


inline void ScFormulaResult::SetResultError( sal_uInt16 nErr )
{
    mnError = nErr;
}


inline formula::FormulaConstTokenRef ScFormulaResult::GetToken() const
{
    if (mbToken)
        return mpToken;
    return NULL;
}


inline formula::FormulaConstTokenRef ScFormulaResult::GetCellResultToken() const
{
    if (GetType() == formula::svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        return static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftToken();
    return GetToken();
}


inline double ScFormulaResult::GetDouble() const
{
    if (mbToken)
    {
        // Should really not be of type formula::svDouble here.
        if (mpToken)
        {
            switch (mpToken->GetType())
            {
                case formula::svHybridCell:
                    return mpToken->GetDouble();
                case formula::svMatrixCell:
                    {
                        const ScMatrixCellResultToken* p =
                            static_cast<const ScMatrixCellResultToken*>(mpToken);
                        if (p->GetUpperLeftType() == formula::svDouble)
                            return p->GetUpperLeftToken()->GetDouble();
                    }
                    break;
                default:
                    ;   // nothing
            }
        }
        return 0.0;
    }
    if (mbEmpty)
        return 0.0;
    return mfValue;
}


inline const String & ScFormulaResult::GetString() const
{
    if (mbToken && mpToken)
    {
        switch (mpToken->GetType())
        {
            case formula::svString:
            case formula::svHybridCell:
                return mpToken->GetString();
            case formula::svMatrixCell:
                {
                    const ScMatrixCellResultToken* p =
                        static_cast<const ScMatrixCellResultToken*>(mpToken);
                    if (p->GetUpperLeftType() == formula::svString)
                        return p->GetUpperLeftToken()->GetString();
                }
                break;
            default:
                ;   // nothing
        }
    }
    return EMPTY_STRING;
}


inline ScConstMatrixRef ScFormulaResult::GetMatrix() const
{
    if (GetType() == formula::svMatrixCell)
        return static_cast<const ScToken*>(mpToken)->GetMatrix();
    return NULL;
}


inline const String & ScFormulaResult::GetHybridFormula() const
{
    if (GetType() == formula::svHybridCell)
    {
        const ScHybridCellToken* p = dynamic_cast<const ScHybridCellToken*>(mpToken);
        if (p)
            return p->GetFormula();
    }
    return EMPTY_STRING;
}


inline void ScFormulaResult::SetHybridDouble( double f )
{
    ResetToDefaults();
    if (mbToken && mpToken)
    {
        String aString( GetString());
        String aFormula( GetHybridFormula());
        mpToken->DecRef();
        mpToken = new ScHybridCellToken( f, aString, aFormula);
        mpToken->IncRef();
    }
    else
    {
        mfValue = f;
        mbToken = false;
        meMultiline = MULTILINE_FALSE;
    }
}


inline void ScFormulaResult::SetHybridString( const String & rStr )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    String aFormula( GetHybridFormula());
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, rStr, aFormula);
    mpToken->IncRef();
    mbToken = true;
}


inline void ScFormulaResult::SetHybridFormula( const String & rFormula )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    String aStr( GetString());
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, aStr, rFormula);
    mpToken->IncRef();
    mbToken = true;
}


inline const ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellToken() const
{
    return (GetType() == formula::svMatrixCell ?
            dynamic_cast<const ScMatrixFormulaCellToken*>(mpToken) : NULL);
}


inline ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellTokenNonConst()
{
    return const_cast<ScMatrixFormulaCellToken*>( GetMatrixFormulaCellToken());
}


#endif // SC_FORMULARESULT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
