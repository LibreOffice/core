/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_SIMPLEFORMULACALC_HXX
#define INCLUDED_SC_INC_SIMPLEFORMULACALC_HXX

#include <memory>
#include <formula/grammar.hxx>

#include "address.hxx"
#include "formularesult.hxx"

class ScDocument;
class ScTokenArray;

class ScSimpleFormulaCalculator
{
private:
    short mnFormatType;
    sal_uLong mnFormatIndex;

    bool mbCalculated;
    std::unique_ptr<ScTokenArray> mpCode;
    ScAddress maAddr;
    ScDocument* mpDoc;
    ScFormulaResult maResult;
    formula::FormulaGrammar::Grammar maGram;
    bool bIsMatrix;
    OUString maMatrixFormulaResult;
    bool mbLimitString;

public:
    ScSimpleFormulaCalculator(ScDocument* pDoc, const ScAddress& rAddr,
            const OUString& rFormula, formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_DEFAULT);
    ~ScSimpleFormulaCalculator();

    void Calculate();
    bool IsValue();
    bool IsMatrix();
    sal_uInt16 GetErrCode();
    double GetValue();
    svl::SharedString GetString();
    short GetFormatType() const { return mnFormatType; }

    bool HasColRowName();

    ScTokenArray* GetCode();

    void SetLimitString(bool bLimitString);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
