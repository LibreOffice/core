/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formularesult.hxx"
#include "scmatrix.hxx"

ScFormulaResult::ScFormulaResult() :
    mpToken(NULL), mnError(0), mbToken(true),
    mbEmpty(false), mbEmptyDisplayedAsString(false),
    meMultiline(MULTILINE_UNKNOWN) {}

ScFormulaResult::ScFormulaResult( const ScFormulaResult & r ) :
    mnError( r.mnError), mbToken( r.mbToken),
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
            {
                mpToken = new ScMatrixFormulaCellToken( *pMatFormula);
                mpToken->IncRef();
            }
            else
                IncrementTokenRef( mpToken);
        }
    }
    else
        mfValue = r.mfValue;
}

ScFormulaResult::ScFormulaResult( const formula::FormulaToken* p ) :
    mnError(0), mbToken(false), mbEmpty(false), mbEmptyDisplayedAsString(false),
    meMultiline(MULTILINE_UNKNOWN)
{
    SetToken( p);
}

ScFormulaResult::~ScFormulaResult()
{
    if (mbToken && mpToken)
        mpToken->DecRef();
}


void ScFormulaResult::ResetToDefaults()
{
    mnError = 0;
    mbEmpty = false;
    mbEmptyDisplayedAsString = false;
    meMultiline = MULTILINE_UNKNOWN;
}

void ScFormulaResult::ResolveToken( const formula::FormulaToken * p )
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

ScFormulaResult & ScFormulaResult::operator=( const ScFormulaResult & r )
{
    Assign( r);
    return *this;
}

void ScFormulaResult::Assign( const ScFormulaResult & r )
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

void ScFormulaResult::SetToken( const formula::FormulaToken* p )
{
    ResetToDefaults();
    IncrementTokenRef( p);
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
                SAL_WARN( "sc", "ScFormulaResult::SetToken: pNewMatFormula and pMatFormula, overriding matrix formula dimension; intended?");
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

void ScFormulaResult::SetDouble( double f )
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

formula::StackVar ScFormulaResult::GetType() const
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

formula::StackVar ScFormulaResult::GetCellResultType() const
{
    formula::StackVar sv = GetType();
    if (sv == formula::svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        sv = static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftType();
    return sv;
}

bool ScFormulaResult::IsEmptyDisplayedAsString() const
{
    if (mbEmpty)
        return mbEmptyDisplayedAsString;
    if (GetType() == formula::svMatrixCell)
    {
        // don't need to test for mpToken here, GetType() already did it
        const ScEmptyCellToken* p = dynamic_cast<const ScEmptyCellToken*>(
                static_cast<const ScMatrixCellResultToken*>(
                    mpToken)->GetUpperLeftToken().get());
        if (p)
            return p->IsDisplayedAsString();
    }
    return false;
}

namespace {

inline bool isValue( formula::StackVar sv )
{
    return sv == formula::svDouble || sv == formula::svError
        || sv == formula::svEmptyCell || sv == formula::svHybridValueCell;
}

}

bool ScFormulaResult::IsValue() const
{
    return isValue(GetCellResultType());
}

bool ScFormulaResult::IsValueNoError() const
{
    switch (GetCellResultType())
    {
        case formula::svDouble:
        case formula::svEmptyCell:
        case formula::svHybridValueCell:
            return true;
        default:
            return false;
    }
}

bool ScFormulaResult::IsMultiline() const
{
    if (meMultiline == MULTILINE_UNKNOWN)
    {
        const OUString& rStr = GetString();
        if (!rStr.isEmpty() && rStr.indexOf( '\n' ) != -1)
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_TRUE;
        else
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_FALSE;
    }
    return meMultiline == MULTILINE_TRUE;
}

bool ScFormulaResult::GetErrorOrDouble( sal_uInt16& rErr, double& rVal ) const
{
    if (mnError)
    {
        rErr = mnError;
        return true;
    }

    formula::StackVar sv = GetCellResultType();
    if (sv == formula::svError)
    {
        if (GetType() == formula::svMatrixCell)
        {
            // don't need to test for mpToken here, GetType() already did it
            rErr = static_cast<const ScMatrixCellResultToken*>(mpToken)->
                GetUpperLeftToken()->GetError();
        }
        else if (mpToken)
        {
            rErr = mpToken->GetError();
        }
    }

    if (rErr)
        return true;

    if (!isValue(sv))
        return false;

    rVal = GetDouble();
    return true;
}

sal_uInt16 ScFormulaResult::GetResultError() const
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

void ScFormulaResult::SetResultError( sal_uInt16 nErr )
{
    mnError = nErr;
}

formula::FormulaConstTokenRef ScFormulaResult::GetToken() const
{
    if (mbToken)
        return mpToken;
    return NULL;
}

formula::FormulaConstTokenRef ScFormulaResult::GetCellResultToken() const
{
    if (GetType() == formula::svMatrixCell)
        // don't need to test for mpToken here, GetType() already did it
        return static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftToken();
    return GetToken();
}

double ScFormulaResult::GetDouble() const
{
    if (mbToken)
    {
        // Should really not be of type formula::svDouble here.
        if (mpToken)
        {
            switch (mpToken->GetType())
            {
                case formula::svHybridCell:
                case formula::svHybridValueCell:
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

const OUString & ScFormulaResult::GetString() const
{
    if (mbToken && mpToken)
    {
        switch (mpToken->GetType())
        {
            case formula::svString:
            case formula::svHybridCell:
            case formula::svHybridValueCell:
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
    return EMPTY_OUSTRING;
}

ScConstMatrixRef ScFormulaResult::GetMatrix() const
{
    if (GetType() == formula::svMatrixCell)
        return static_cast<const ScToken*>(mpToken)->GetMatrix();
    return NULL;
}

const OUString& ScFormulaResult::GetHybridFormula() const
{
    if (GetType() == formula::svHybridCell)
    {
        const ScHybridCellToken* p = dynamic_cast<const ScHybridCellToken*>(mpToken);
        if (p)
            return p->GetFormula();
    }
    return EMPTY_OUSTRING;
}

void ScFormulaResult::SetHybridDouble( double f )
{
    ResetToDefaults();
    if (mbToken && mpToken)
    {
        if(GetType() == formula::svMatrixCell)
            SetDouble(f);
        else
        {
            OUString aString( GetString());
            OUString aFormula( GetHybridFormula());
            mpToken->DecRef();
            mpToken = new ScHybridCellToken( f, aString, aFormula);
            mpToken->IncRef();
        }
    }
    else
    {
        mfValue = f;
        mbToken = false;
        meMultiline = MULTILINE_FALSE;
    }
}

void ScFormulaResult::SetHybridString( const OUString & rStr )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    OUString aFormula( GetHybridFormula());
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, rStr, aFormula);
    mpToken->IncRef();
    mbToken = true;
}

void ScFormulaResult::SetHybridFormula( const OUString & rFormula )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    OUString aStr( GetString());
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, aStr, rFormula);
    mpToken->IncRef();
    mbToken = true;
}

void ScFormulaResult::SetMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL )
{
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScMatrixFormulaCellToken(nCols, nRows, pMat, pUL);
    mpToken->IncRef();
    mbToken = true;
}

const ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellToken() const
{
    return (GetType() == formula::svMatrixCell ?
            dynamic_cast<const ScMatrixFormulaCellToken*>(mpToken) : NULL);
}

ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellTokenNonConst()
{
    return const_cast<ScMatrixFormulaCellToken*>( GetMatrixFormulaCellToken());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
