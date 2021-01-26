/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formularesult.hxx>
#include <scmatrix.hxx>
#include <token.hxx>

#include <sal/log.hxx>

namespace sc {

FormulaResultValue::FormulaResultValue() : mfValue(0.0), meType(Invalid), mnError(FormulaError::NONE) {}
FormulaResultValue::FormulaResultValue( double fValue ) : mfValue(fValue), meType(Value), mnError(FormulaError::NONE) {}
FormulaResultValue::FormulaResultValue( const svl::SharedString& rStr ) : mfValue(0.0), maString(rStr), meType(String), mnError(FormulaError::NONE) {}
FormulaResultValue::FormulaResultValue( FormulaError nErr ) : mfValue(0.0), meType(Error), mnError(nErr) {}

}

ScFormulaResult::ScFormulaResult() :
    mpToken(nullptr),
    mbToken(true),
    mbEmpty(false),
    mbEmptyDisplayedAsString(false),
    mbValueCached(false),
    meMultiline(MULTILINE_UNKNOWN),
    mnError(FormulaError::NONE) {}

ScFormulaResult::ScFormulaResult( const ScFormulaResult & r ) :
    mbToken( r.mbToken),
    mbEmpty( r.mbEmpty),
    mbEmptyDisplayedAsString( r.mbEmptyDisplayedAsString),
    mbValueCached( r.mbValueCached),
    meMultiline( r.meMultiline),
    mnError( r.mnError)
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
    mbToken(false),
    mbEmpty(false),
    mbEmptyDisplayedAsString(false),
    mbValueCached(false),
    meMultiline(MULTILINE_UNKNOWN),
    mnError(FormulaError::NONE)
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
    mnError = FormulaError::NONE;
    mbEmpty = false;
    mbEmptyDisplayedAsString = false;
    meMultiline = MULTILINE_UNKNOWN;
    mbValueCached = false;
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
                // Take advantage of fast double result return for empty result token.
                // by setting mfValue to 0 and turning on mbValueCached flag.
                mfValue = 0.0;
                mbValueCached = true;
                break;
            case formula::svDouble:
                mfValue = p->GetDouble();
                p->DecRef();
                mbToken = false;
                meMultiline = MULTILINE_FALSE;
                mbValueCached = true;
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

    // It is important to reset the value-cache flag to that of the source
    // unconditionally.
    mbValueCached = r.mbValueCached;

    if (r.mbEmpty)
    {
        if (mbToken && mpToken)
            mpToken->DecRef();
        mbToken = false;
        mbEmpty = true;
        mbEmptyDisplayedAsString = r.mbEmptyDisplayedAsString;
        meMultiline = r.meMultiline;
        // here r.mfValue will be 0.0 which is ensured in ResolveToken().
        mfValue = 0.0;
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
    SetResultError(r.mnError);
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
             dynamic_cast<const ScMatrixCellResultToken*>(p) : nullptr);
        if (pMatResult)
        {
            const ScMatrixFormulaCellToken* pNewMatFormula =
                dynamic_cast<const ScMatrixFormulaCellToken*>(pMatResult);
            if (pNewMatFormula && (pMatFormula->GetMatCols() <= 0 || pMatFormula->GetMatRows() <= 0))
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
        mbValueCached = true;
    }
}

formula::StackVar ScFormulaResult::GetType() const
{
    // Order is significant.
    if (mnError != FormulaError::NONE)
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
    switch (GetType())
    {
        case formula::svMatrixCell:
            {
                // don't need to test for mpToken here, GetType() already did it
                const ScEmptyCellToken* p = dynamic_cast<const ScEmptyCellToken*>(
                        static_cast<const ScMatrixCellResultToken*>(
                            mpToken)->GetUpperLeftToken().get());
                if (p)
                    return p->IsDisplayedAsString();
            }
        break;
        case formula::svHybridCell:
            {
                const ScHybridCellToken* p = static_cast<const ScHybridCellToken*>(mpToken);
                return p->IsEmptyDisplayedAsString();
            }
        break;
        default:
        break;
    }
    return false;
}

namespace {

bool isValue( formula::StackVar sv )
{
    return sv == formula::svDouble || sv == formula::svError
        || sv == formula::svEmptyCell
        // The initial uninitialized result value is double 0.0, even if the type
        // is unknown, so the interpreter asking for it gets that double
        // instead of having to convert a string which may result in #VALUE!
        // (otherwise the unknown would be neither error nor double nor string)
        || sv == formula::svUnknown;
}

bool isString( formula::StackVar sv )
{
    switch (sv)
    {
        case formula::svString:
        case formula::svHybridCell:
            return true;
        default:
            break;
    }

    return false;
}

}

bool ScFormulaResult::IsValue() const
{
    if (IsEmptyDisplayedAsString())
        return true;

    return isValue(GetCellResultType());
}

bool ScFormulaResult::IsValueNoError() const
{
    switch (GetCellResultType())
    {
        case formula::svDouble:
        case formula::svEmptyCell:
            return true;
        default:
            return false;
    }
}

bool ScFormulaResult::IsMultiline() const
{
    if (meMultiline == MULTILINE_UNKNOWN)
    {
        svl::SharedString aStr = GetString();
        if (!aStr.isEmpty() && aStr.getString().indexOf('\n') != -1)
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_TRUE;
        else
            const_cast<ScFormulaResult*>(this)->meMultiline = MULTILINE_FALSE;
    }
    return meMultiline == MULTILINE_TRUE;
}

bool ScFormulaResult::GetErrorOrDouble( FormulaError& rErr, double& rVal ) const
{
    if (mbValueCached)
    {
        rVal = mfValue;
        return true;
    }

    if (mnError != FormulaError::NONE)
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

    if (rErr != FormulaError::NONE)
        return true;

    if (!isValue(sv))
        return false;

    rVal = GetDouble();
    return true;
}

sc::FormulaResultValue ScFormulaResult::GetResult() const
{
    if (mbValueCached)
        return sc::FormulaResultValue(mfValue);

    if (mnError != FormulaError::NONE)
        return sc::FormulaResultValue(mnError);

    formula::StackVar sv = GetCellResultType();
    FormulaError nErr = FormulaError::NONE;
    if (sv == formula::svError)
    {
        if (GetType() == formula::svMatrixCell)
        {
            // don't need to test for mpToken here, GetType() already did it
            nErr = static_cast<const ScMatrixCellResultToken*>(mpToken)->
                GetUpperLeftToken()->GetError();
        }
        else if (mpToken)
        {
            nErr = mpToken->GetError();
        }
    }

    if (nErr != FormulaError::NONE)
        return sc::FormulaResultValue(nErr);

    if (isValue(sv))
        return sc::FormulaResultValue(GetDouble());

    if (!mbToken)
        // String result type needs token.
        return sc::FormulaResultValue();

    if (isString(sv))
        return sc::FormulaResultValue(GetString());

    // Invalid
    return sc::FormulaResultValue();
}

FormulaError ScFormulaResult::GetResultError() const
{
    if (mnError != FormulaError::NONE)
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
    return FormulaError::NONE;
}

void ScFormulaResult::SetResultError( FormulaError nErr )
{
    mnError = nErr;
    if (mnError != FormulaError::NONE)
        mbValueCached = false;
}

formula::FormulaConstTokenRef ScFormulaResult::GetToken() const
{
    if (mbToken)
        return mpToken;
    return nullptr;
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
    if (mbValueCached)
        return mfValue;

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
        // Note that we reach here also for the default ctor and
        // formula::svUnknown from GetType().
        return 0.0;
    }
    if (mbEmpty)
        return 0.0;
    return mfValue;
}

const svl::SharedString & ScFormulaResult::GetString() const
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
    return svl::SharedString::getEmptyString();
}

ScConstMatrixRef ScFormulaResult::GetMatrix() const
{
    if (GetType() == formula::svMatrixCell)
        return mpToken->GetMatrix();
    return nullptr;
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
            svl::SharedString aString = GetString();
            OUString aFormula( GetHybridFormula());
            mpToken->DecRef();
            mpToken = new ScHybridCellToken( f, aString, aFormula, false);
            mpToken->IncRef();
        }
    }
    else
    {
        mfValue = f;
        mbToken = false;
        meMultiline = MULTILINE_FALSE;
        mbValueCached = true;
    }
}

void ScFormulaResult::SetHybridString( const svl::SharedString& rStr )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    OUString aFormula( GetHybridFormula());
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, rStr, aFormula, false);
    mpToken->IncRef();
    mbToken = true;
}

void ScFormulaResult::SetHybridEmptyDisplayedAsString()
{
    // Obtain values before changing anything.
    double f = GetDouble();
    OUString aFormula( GetHybridFormula());
    svl::SharedString aStr = GetString();
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    // XXX NOTE: we can't use mbEmpty and mbEmptyDisplayedAsString here because
    // GetType() intentionally returns svEmptyCell if mbEmpty==true. So stick
    // it into the ScHybridCellToken.
    mpToken = new ScHybridCellToken( f, aStr, aFormula, true);
    mpToken->IncRef();
    mbToken = true;
}

void ScFormulaResult::SetHybridFormula( const OUString & rFormula )
{
    // Obtain values before changing anything.
    double f = GetDouble();
    svl::SharedString aStr = GetString();
    ResetToDefaults();
    if (mbToken && mpToken)
        mpToken->DecRef();
    mpToken = new ScHybridCellToken( f, aStr, rFormula, false);
    mpToken->IncRef();
    mbToken = true;
}

void ScFormulaResult::SetMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL )
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
            dynamic_cast<const ScMatrixFormulaCellToken*>(mpToken) : nullptr);
}

ScMatrixFormulaCellToken* ScFormulaResult::GetMatrixFormulaCellTokenNonConst()
{
    return const_cast<ScMatrixFormulaCellToken*>( GetMatrixFormulaCellToken());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
