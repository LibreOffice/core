/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tokenstringcontext.hxx"
#include "compiler.hxx"
#include "document.hxx"
#include "dbdata.hxx"

using namespace com::sun::star;

namespace sc {

TokenStringContext::TokenStringContext( const ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram ) :
    meGram(eGram),
    mpRefConv(ScCompiler::GetRefConvention(formula::FormulaGrammar::extractRefConvention(eGram)))
{
    ScCompiler aComp(NULL, ScAddress());
    mxOpCodeMap = aComp.GetOpCodeMap(formula::FormulaGrammar::extractFormulaLanguage(eGram));
    if (mxOpCodeMap)
        maErrRef = mxOpCodeMap->getSymbol(ocErrRef);

    if (pDoc)
    {
        // Fetch all sheet names.
        maTabNames = pDoc->GetAllTableNames();
        {
            std::vector<OUString>::iterator it = maTabNames.begin(), itEnd = maTabNames.end();
            for (; it != itEnd; ++it)
                ScCompiler::CheckTabQuotes(*it, formula::FormulaGrammar::extractRefConvention(eGram));
        }

        // Fetch all named range names.
        const ScRangeName* pNames = pDoc->GetRangeName();
        if (pNames)
        {
            ScRangeName::const_iterator it = pNames->begin(), itEnd = pNames->end();
            for (; it != itEnd; ++it)
            {
                const ScRangeData* pData = it->second;
                maGlobalRangeNames.insert(IndexNameMapType::value_type(pData->GetIndex(), pData->GetName()));
            }
        }

        // Fetch all named database ranges names.
        const ScDBCollection* pDBs = pDoc->GetDBCollection();
        if (pDBs)
        {
            const ScDBCollection::NamedDBs& rNamedDBs = pDBs->getNamedDBs();
            ScDBCollection::NamedDBs::const_iterator it = rNamedDBs.begin(), itEnd = rNamedDBs.end();
            for (; it != itEnd; ++it)
            {
                const ScDBData& rData = *it;
                maNamedDBs.insert(IndexNameMapType::value_type(rData.GetIndex(), rData.GetName()));
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
