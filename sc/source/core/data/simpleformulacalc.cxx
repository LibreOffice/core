/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "simpleformulacalc.hxx"
#include "document.hxx"
#include "tokenarray.hxx"
#include "interpre.hxx"
#include "compiler.hxx"

ScSimpleFormulaCalculator::ScSimpleFormulaCalculator( ScDocument* pDoc, const ScAddress& rAddr,
        const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
    : mnFormatType(0)
    , mnFormatIndex(0)
    , mbCalculated(false)
    , maAddr(rAddr)
    , mpDoc(pDoc)
    , maFormula(rFormula)
    , maGram(eGram)
    , bIsMatrix(false)
{
    // compile already here
    ScCompiler aComp(mpDoc, maAddr);
    aComp.SetGrammar(eGram);
    mpCode.reset(aComp.CompileString(rFormula));
    if(!mpCode->GetCodeError() && mpCode->GetLen())
        aComp.CompileTokenArray();
}

ScSimpleFormulaCalculator::~ScSimpleFormulaCalculator()
{
}

void ScSimpleFormulaCalculator::Calculate()
{
    if(mbCalculated)
        return;

    mbCalculated = true;
    ScInterpreter aInt(NULL, mpDoc, maAddr, *mpCode.get());
    aInt.AssertFormulaMatrix();

    formula::StackVar aIntType = aInt.Interpret();
    if ( aIntType == formula::svMatrixCell )
    {
        OUStringBuffer aStr;
        ScCompiler aComp(mpDoc, maAddr);
        aComp.SetGrammar(maGram);
        mpCode.reset(aComp.CompileString(maFormula));
        if(!mpCode->GetCodeError() && mpCode->GetLen())
            aComp.CompileTokenArray();
        aComp.CreateStringFromToken( aStr, aInt.GetResultToken().get(), true );
        bIsMatrix = true;
        maMatrixFormulaResult = aStr.toString();
    }
    mnFormatType = aInt.GetRetFormatType();
    mnFormatIndex = aInt.GetRetFormatIndex();
    maResult.SetToken(aInt.GetResultToken().get());
}

bool ScSimpleFormulaCalculator::IsValue()
{
    Calculate();

    if (bIsMatrix)
        return false;
    return maResult.IsValue();
}

sal_uInt16 ScSimpleFormulaCalculator::GetErrCode()
{
    Calculate();

    sal_uInt16 nErr = mpCode->GetCodeError();
    if (nErr)
        return nErr;
    return maResult.GetResultError();
}

double ScSimpleFormulaCalculator::GetValue()
{
    Calculate();

    if ((!mpCode->GetCodeError() || mpCode->GetCodeError() == errDoubleRef) &&
            !maResult.GetResultError())
        return maResult.GetDouble();

    return 0.0;
}

svl::SharedString ScSimpleFormulaCalculator::GetString()
{
    Calculate();

    if (bIsMatrix)
        return maMatrixFormulaResult;

    if ((!mpCode->GetCodeError() || mpCode->GetCodeError() == errDoubleRef) &&
            !maResult.GetResultError())
        return maResult.GetString();

    return svl::SharedString::getEmptyString();
}

bool ScSimpleFormulaCalculator::HasColRowName()
{
    mpCode->Reset();
    return mpCode->GetNextColRowName() != NULL;
}

ScTokenArray* ScSimpleFormulaCalculator::GetCode()
{
    return mpCode.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
