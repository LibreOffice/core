/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SIMPLE_FORMULA_CALC_HXX
#define SC_SIMPLE_FORMULA_CALC_HXX

#include <boost/scoped_ptr.hpp>
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
    boost::scoped_ptr<ScTokenArray> mpCode;
    ScAddress maAddr;
    ScDocument* mpDoc;
    ScFormulaResult maResult;

public:
    ScSimpleFormulaCalculator(ScDocument* pDoc, const ScAddress& rAddr,
            const OUString& rFormula, formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_DEFAULT);
    ~ScSimpleFormulaCalculator();

    void Calculate();
    bool IsValue();
    sal_uInt16 GetErrCode();
    double GetValue();
    OUString GetString();
    short GetFormatType() const { return mnFormatType; }
    sal_uLong GetFormatIndex() const { return mnFormatIndex; }

    bool HasColRowName();

    ScTokenArray* GetCode();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
