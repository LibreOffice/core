/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formularesult.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:16:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_FORMULARESULT_HXX
#define SC_FORMULARESULT_HXX

#ifndef SC_TOKEN_HXX
#include "token.hxx"
#endif


/** Store a variable formula cell result, balancing between runtime performance
    and memory consumption. */
class ScFormulaResult
{
    union
    {
        double          mfValue;    // double result direct for performance and memory consumption
        const ScToken*  mpToken;    // if not, result token obtained from interpreter
    };
    USHORT              mnError;    // error code
    bool                mbToken :1; // whether content of union is a token
    bool                mbEmpty :1; // empty cell result
    bool                mbEmptyDisplayedAsString :1;    // only if mbEmpty

    /** Reset mnError, mbEmpty and mbEmptyDisplayedAsString to their defaults
        prior to assigning other types */
    inline  void                ResetToDefaults();

    /** If token is of svError set error code and decrement RefCount.
        If token is of svEmptyCell set mbEmpty and mbEmptyAsString and
        decrement RefCount.
        If token is of svDouble set mfValue and decrement RefCount.
        Else assign token to mpToken. NULL is valid => svUnknown.
        Other member variables are set accordingly.
        @precondition: Token MUST had been IncRef'ed prior to this call!
        @precondition: An already existing different mpToken MUST had been
        DecRef'ed prior to this call, p will be assigned to mpToken if not
        resolved.
        ATTENTION! Token may get deleted in this call! */
    inline  void                ResolveToken( const ScToken * p );

public:
                                /** Effectively type svUnknown. */
                                ScFormulaResult()
                                    : mpToken(NULL), mnError(0), mbToken(true),
                                    mbEmpty(false), mbEmptyDisplayedAsString(false) {}

                                ScFormulaResult( const ScFormulaResult & r )
                                    : mnError( r.mnError), mbToken( r.mbToken),
                                    mbEmpty( r.mbEmpty),
                                    mbEmptyDisplayedAsString( r.mbEmptyDisplayedAsString)
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
    explicit                    ScFormulaResult( const ScToken* p )
                                    : mnError(0), mbToken(false),
                                    mbEmpty(false), mbEmptyDisplayedAsString(false)
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

    /** Sets a direct double if token type is svDouble, or mbEmpty if
        svEmptyCell, else token. If p is NULL, that is set as well, effectively
        resulting in GetType()==svUnknown. If the already existing result is
        ScMatrixFormulaCellToken, the upper left ist set to token.

        ATTENTION! ScToken had to be allocated using 'new' and if of type
        svDouble and no RefCount was set may not be used after this call
        because it was deleted after decrement! */
    inline  void                SetToken( const ScToken* p );

    /** May be NULL if SetToken() did so, also if type svDouble or svError! */
    inline  ScConstTokenRef     GetToken() const;

    /** Return upper left token if svMatrixCell, else return GetToken().
        May be NULL if SetToken() did so, also if type svDouble or svError! */
    inline  ScConstTokenRef     GetCellResultToken() const;

    /** Return type of result, including svError, svEmptyCell, svDouble and
        svMatrixCell. */
    inline  StackVar            GetType() const;

    /** If type is svMatrixCell return the type of upper left element, else
        GetType() */
    inline  StackVar            GetCellResultType() const;

    /** If type is svEmptyCell (including matrix upper left) and should be
        displayed as empty string */
    inline  bool                IsEmptyDisplayedAsString() const;

    /** Test for cell result type svDouble, including upper left if
        svMatrixCell. Also included is svError for legacy, because previously
        an error result was treated like a numeric value at some places in
        ScFormulaCell. Also included is svEmptyCell as a reference to an empty
        cell usually is treated as numeric 0. Use GetCellResultType() for
        details instead. */
    inline  bool                IsValue() const;

    /** Get error code if set or GetCellResultType() is svError or svUnknown,
        else 0. */
    inline  USHORT              GetResultError() const;

    /** Set error code, don't touch token or double. */
    inline  void                SetResultError( USHORT nErr );

    /** Set direct double. Shouldn't be used externally except in
        ScFormulaCell for rounded CalcAsShown or SetErrCode(). If
        ScMatrixFormulaCellToken the token isn't replaced but upper left result
        is modified instead, but only if it was of type svDouble before or not
        set at all. */
    inline  void                SetDouble( double f );

    /** Return value if type svDouble or svHybridCell or svMatrixCell and upper
        left svDouble, else 0.0 */
    inline  double              GetDouble() const;

    /** Return string if type svString or svHybridCell or svMatrixCell and
        upper left svString, else empty string. */
    inline  const String &      GetString() const;

    /** Return matrix if type svMatrixCell and ScMatrix present, else NULL. */
    inline  ScConstMatrixRef    GetMatrix() const;

    /** Return formula string if type svHybridCell, else empty string. */
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
}


inline void ScFormulaResult::ResolveToken( const ScToken * p )
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
            case svError:
                mnError = p->GetError();
                p->DecRef();
                mbToken = false;
                // set in case mnError is 0 now, which shouldn't happen but ...
                mfValue = 0.0;
                break;
            case svEmptyCell:
                mbEmpty = true;
                mbEmptyDisplayedAsString = static_cast<const ScEmptyCellToken*>(p)->IsDisplayedAsString();
                p->DecRef();
                mbToken = false;
                break;
            case svDouble:
                mfValue = p->GetDouble();
                p->DecRef();
                mbToken = false;
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


inline void ScFormulaResult::SetToken( const ScToken* p )
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
            (p && p->GetType() == svMatrixCell ?
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
    }
}


inline StackVar ScFormulaResult::GetType() const
{
    // Order is significant.
    if (mnError)
        return svError;
    if (mbEmpty)
        return svEmptyCell;
    if (!mbToken)
        return svDouble;
    if (mpToken)
        return mpToken->GetType();
    return svUnknown;
}


inline StackVar ScFormulaResult::GetCellResultType() const
{
    StackVar sv = GetType();
    if (sv == svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        sv = static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftType();
    return sv;
}


inline bool ScFormulaResult::IsEmptyDisplayedAsString() const
{
    if (mbEmpty)
        return mbEmptyDisplayedAsString;
    if (GetType() == svMatrixCell)
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
    StackVar sv = GetCellResultType();
    return sv == svDouble || sv == svError || sv == svEmptyCell;
}


inline USHORT ScFormulaResult::GetResultError() const
{
    if (mnError)
        return mnError;
    StackVar sv = GetCellResultType();
    if (sv == svError)
    {
        if (GetType() == svMatrixCell)
            // don't need to test for mpToken here, GetType() already did it
            return static_cast<const ScMatrixCellResultToken*>(mpToken)->
                GetUpperLeftToken()->GetError();
        if (mpToken)
            return mpToken->GetError();
    }
    return 0;
}


inline void ScFormulaResult::SetResultError( USHORT nErr )
{
    mnError = nErr;
}


inline ScConstTokenRef ScFormulaResult::GetToken() const
{
    if (mbToken)
        return mpToken;
    return NULL;
}


inline ScConstTokenRef ScFormulaResult::GetCellResultToken() const
{
    if (GetType() == svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        return static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftToken();
    return GetToken();
}


inline double ScFormulaResult::GetDouble() const
{
    if (mbToken)
    {
        // Should really not be of type svDouble here.
        if (mpToken)
        {
            switch (mpToken->GetType())
            {
                case svHybridCell:
                    return mpToken->GetDouble();
                case svMatrixCell:
                    {
                        const ScMatrixCellResultToken* p =
                            static_cast<const ScMatrixCellResultToken*>(mpToken);
                        if (p->GetUpperLeftType() == svDouble)
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
            case svString:
            case svHybridCell:
                return mpToken->GetString();
            case svMatrixCell:
                {
                    const ScMatrixCellResultToken* p =
                        static_cast<const ScMatrixCellResultToken*>(mpToken);
                    if (p->GetUpperLeftType() == svString)
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
    if (GetType() == svMatrixCell)
        return mpToken->GetMatrix();
    return NULL;
}


inline const String & ScFormulaResult::GetHybridFormula() const
{
    if (GetType() == svHybridCell)
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
    return (GetType() == svMatrixCell ?
            dynamic_cast<const ScMatrixFormulaCellToken*>(mpToken) : NULL);
}


inline ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellTokenNonConst()
{
    return const_cast<ScMatrixFormulaCellToken*>( GetMatrixFormulaCellToken());
}


#endif // SC_FORMULARESULT_HXX
