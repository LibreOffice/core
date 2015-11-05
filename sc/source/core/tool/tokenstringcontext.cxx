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
#include "externalrefmgr.hxx"

using namespace com::sun::star;

namespace sc {

namespace {

void insertAllNames( TokenStringContext::IndexNameMapType& rMap, const ScRangeName& rNames )
{
    for (auto const& it : rNames)
    {
        const ScRangeData *const pData = it.second.get();
        rMap.insert(
            TokenStringContext::IndexNameMapType::value_type(pData->GetIndex(), pData->GetName()));
    }
}

}

TokenStringContext::TokenStringContext( const ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram ) :
    meGram(eGram),
    mpRefConv(ScCompiler::GetRefConvention(formula::FormulaGrammar::extractRefConvention(eGram)))
{
    ScCompiler aComp(NULL, ScAddress());
    mxOpCodeMap = aComp.GetOpCodeMap(formula::FormulaGrammar::extractFormulaLanguage(eGram));
    if (mxOpCodeMap)
        maErrRef = mxOpCodeMap->getSymbol(ocErrRef);

    if (!pDoc)
        return;

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
        // global names
        insertAllNames(maGlobalRangeNames, *pNames);

    {
        ScRangeName::TabNameCopyMap aTabRangeNames;
        pDoc->GetAllTabRangeNames(aTabRangeNames);
        ScRangeName::TabNameCopyMap::const_iterator it = aTabRangeNames.begin(), itEnd = aTabRangeNames.end();
        for (; it != itEnd; ++it)
        {
            const ScRangeName* pSheetNames = it->second;
            if (!pSheetNames)
                continue;

            SCTAB nTab = it->first;
            IndexNameMapType aNames;
            insertAllNames(aNames, *pSheetNames);
            maSheetRangeNames.insert(TabIndexMapType::value_type(nTab, aNames));
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
            const ScDBData& rData = **it;
            maNamedDBs.insert(IndexNameMapType::value_type(rData.GetIndex(), rData.GetName()));
        }
    }

    // Fetch all relevant bits for external references.
    if (pDoc->HasExternalRefManager())
    {
        const ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        maExternalFileNames = pRefMgr->getAllCachedExternalFileNames();
        for (size_t i = 0, n = maExternalFileNames.size(); i < n; ++i)
        {
            sal_uInt16 nFileId = static_cast<sal_uInt16>(i);
            std::vector<OUString> aTabNames;
            pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
            if (!aTabNames.empty())
                maExternalCachedTabNames.insert(
                    IndexNamesMapType::value_type(nFileId, aTabNames));
        }
    }
}

CompileFormulaContext::CompileFormulaContext( ScDocument* pDoc ) :
    mpDoc(pDoc), meGram(pDoc->GetGrammar())
{
    updateTabNames();
}

CompileFormulaContext::CompileFormulaContext( ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram ) :
    mpDoc(pDoc), meGram(eGram)
{
    updateTabNames();
}

void CompileFormulaContext::updateTabNames()
{
    // Fetch all sheet names.
    maTabNames = mpDoc->GetAllTableNames();
    {
        std::vector<OUString>::iterator it = maTabNames.begin(), itEnd = maTabNames.end();
        for (; it != itEnd; ++it)
            ScCompiler::CheckTabQuotes(*it, formula::FormulaGrammar::extractRefConvention(meGram));
    }
}

void CompileFormulaContext::setGrammar( formula::FormulaGrammar::Grammar eGram )
{
    bool bUpdate = (meGram != eGram);
    meGram = eGram;
    if (bUpdate)
        updateTabNames();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
