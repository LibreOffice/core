/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <simpleformulacalc.hxx>
#include <document.hxx>
#include <tokenarray.hxx>
#include <interpre.hxx>
#include <compiler.hxx>
#include <sfx2/linkmgr.hxx>

#define DISPLAY_LEN 15

ScSimpleFormulaCalculator::ScSimpleFormulaCalculator( ScDocument& rDoc, const ScAddress& rAddr,
        const OUString& rFormula, bool bMatrixFormula, formula::FormulaGrammar::Grammar eGram )
    : mnFormatType(SvNumFormatType::ALL)
    , mbCalculated(false)
    , maAddr(rAddr)
    , mrDoc(rDoc)
    , maGram(eGram)
    , mbMatrixResult(false)
    , mbLimitString(false)
    , mbMatrixFormula(bMatrixFormula)
{
    // compile already here
    ScCompiler aComp(mrDoc, maAddr, eGram, true, bMatrixFormula);
    mpCode = aComp.CompileString(rFormula);
    if(mpCode->GetCodeError() == FormulaError::NONE && mpCode->GetLen())
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

    ScInterpreter aInt(mrDoc.GetFormulaCell( maAddr ), mrDoc, mrDoc.GetNonThreadedContext(), maAddr, *mpCode);
    if (mbMatrixFormula)
        aInt.AssertFormulaMatrix();

    std::unique_ptr<sfx2::LinkManager> pNewLinkMgr( new sfx2::LinkManager(mrDoc.GetDocumentShell()) );
    aInt.SetLinkManager( pNewLinkMgr.get() );

    formula::StackVar aIntType = aInt.Interpret();
    if ( aIntType == formula::svMatrixCell )
    {
        ScCompiler aComp(mrDoc, maAddr, maGram);
        OUStringBuffer aStr;
        aComp.CreateStringFromToken(aStr, aInt.GetResultToken().get());

        mbMatrixResult = true;

        if (mbLimitString)
        {
            size_t n = aStr.getLength();
            for (size_t i = DISPLAY_LEN; i < n; ++i)
            {
                if (aStr[i] == ',' || aStr[i] == ';')
                {
                    aStr.truncate(i);
                    aStr.append("...");
                    break;
                }
            }
        }

        maMatrixFormulaResult = aStr.makeStringAndClear();
    }
    mnFormatType = aInt.GetRetFormatType();
    maResult.SetToken(aInt.GetResultToken().get());
}

bool ScSimpleFormulaCalculator::IsValue()
{
    Calculate();

    if (mbMatrixResult)
        return false;

    return maResult.IsValue();
}

bool ScSimpleFormulaCalculator::IsMatrix()
{
    Calculate();

    return mbMatrixResult;
}

FormulaError ScSimpleFormulaCalculator::GetErrCode()
{
    Calculate();

    FormulaError nErr = mpCode->GetCodeError();
    if (nErr != FormulaError::NONE)
        return nErr;
    return maResult.GetResultError();
}

double ScSimpleFormulaCalculator::GetValue()
{
    Calculate();

    if ((mpCode->GetCodeError() == FormulaError::NONE) &&
            maResult.GetResultError() == FormulaError::NONE)
        return maResult.GetDouble();

    return 0.0;
}

svl::SharedString ScSimpleFormulaCalculator::GetString()
{
    Calculate();

    if (mbMatrixResult)
        return svl::SharedString( maMatrixFormulaResult);   // string not interned

    if ((mpCode->GetCodeError() == FormulaError::NONE) &&
            maResult.GetResultError() == FormulaError::NONE)
        return maResult.GetString();

    return svl::SharedString::getEmptyString();
}

bool ScSimpleFormulaCalculator::HasColRowName() const
{
    return formula::FormulaTokenArrayPlainIterator(*mpCode).GetNextColRowName() != nullptr;
}

ScTokenArray* ScSimpleFormulaCalculator::GetCode()
{
    return mpCode.get();
}

void ScSimpleFormulaCalculator::SetLimitString(bool bLimitString)
{
    mbLimitString = bLimitString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
