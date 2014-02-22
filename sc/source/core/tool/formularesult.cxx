/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "formularesult.hxx"
#include "scmatrix.hxx"

namespace sc {

FormulaResultValue::FormulaResultValue() : meType(Invalid), mfValue(0.0), mnError(0) {}
FormulaResultValue::FormulaResultValue( double fValue ) : meType(Value), mfValue(fValue), mnError(0) {}
FormulaResultValue::FormulaResultValue( const svl::SharedString& rStr ) : meType(String), mfValue(0.0), maString(rStr), mnError(0) {}
FormulaResultValue::FormulaResultValue( sal_uInt16 nErr ) : meType(Error), mfValue(0.0), mnError(nErr) {}

}

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
        
        const ScMatrixFormulaCellToken* pMatFormula =
            r.GetMatrixFormulaCellToken();
        if (pMatFormula)
            SetToken( new ScMatrixFormulaCellToken( *pMatFormula));
        else
            SetToken( r.mpToken);
    }
    else
        SetDouble( r.mfValue);
    
    
    mnError = r.mnError;
}

void ScFormulaResult::SetToken( const formula::FormulaToken* p )
{
    ResetToDefaults();
    IncrementTokenRef( p);
    
    
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
            
            
            
            pMatFormula->Assign( *p);
            p->DecRef();
        }
        else
        {
            
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
        
        sv = static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftType();
    return sv;
}

bool ScFormulaResult::IsEmptyDisplayedAsString() const
{
    if (mbEmpty)
        return mbEmptyDisplayedAsString;
    if (GetType() == formula::svMatrixCell)
    {
        
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

inline bool isString( formula::StackVar sv )
{
    switch (sv)
    {
        case formula::svString:
        case formula::svHybridCell:
        case formula::svHybridValueCell:
            return true;
        default:
            break;
    }

    return false;
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
        svl::SharedString aStr = GetString();
        if (!aStr.isEmpty() && aStr.getString().indexOf('\n') != -1)
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

bool ScFormulaResult::GetErrorOrString( sal_uInt16& rErr, svl::SharedString& rStr ) const
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

    if (!mbToken)
        return false;

    if (!isString(sv))
        return false;

    rStr = GetString();
    return true;
}

sc::FormulaResultValue ScFormulaResult::GetResult() const
{
    if (mnError)
        return sc::FormulaResultValue(mnError);

    formula::StackVar sv = GetCellResultType();
    sal_uInt16 nErr = 0;
    if (sv == formula::svError)
    {
        if (GetType() == formula::svMatrixCell)
        {
            
            nErr = static_cast<const ScMatrixCellResultToken*>(mpToken)->
                GetUpperLeftToken()->GetError();
        }
        else if (mpToken)
        {
            nErr = mpToken->GetError();
        }
    }

    if (nErr)
        return sc::FormulaResultValue(nErr);

    if (isValue(sv))
        return sc::FormulaResultValue(GetDouble());

    if (!mbToken)
        
        return sc::FormulaResultValue();

    if (isString(sv))
        return sc::FormulaResultValue(GetString());

    
    return sc::FormulaResultValue();
}

sal_uInt16 ScFormulaResult::GetResultError() const
{
    if (mnError)
        return mnError;
    formula::StackVar sv = GetCellResultType();
    if (sv == formula::svError)
    {
        if (GetType() == formula::svMatrixCell)
            
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
        
        return static_cast<const ScMatrixCellResultToken*>(mpToken)->GetUpperLeftToken();
    return GetToken();
}

double ScFormulaResult::GetDouble() const
{
    if (mbToken)
    {
        
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
                    ;   
            }
        }
        return 0.0;
    }
    if (mbEmpty)
        return 0.0;
    return mfValue;
}

svl::SharedString ScFormulaResult::GetString() const
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
                ;   
        }
    }
    return svl::SharedString::getEmptyString();
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
            svl::SharedString aString = GetString();
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
    
    double f = GetDouble();
    svl::SharedString aStr = GetString();
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
