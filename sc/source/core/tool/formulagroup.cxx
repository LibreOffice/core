/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <formulagroup.hxx>
#include <formulagroupcl.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <compiler.hxx>
#include <interpre.hxx>
#include <scmatrix.hxx>
#include <globalnames.hxx>

#include <formula/vectortoken.hxx>
#include <officecfg/Office/Common.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <o3tl/make_unique.hxx>
#include <rtl/bootstrap.hxx>

#include <cstdio>
#include <unordered_map>
#include <vector>

#if HAVE_FEATURE_OPENCL
#  include <opencl/openclwrapper.hxx>
#endif

namespace sc {

FormulaGroupEntry::FormulaGroupEntry( ScFormulaCell** pCells, size_t nRow, size_t nLength ) :
    mpCells(pCells), mnRow(nRow), mnLength(nLength), mbShared(true) {}

FormulaGroupEntry::FormulaGroupEntry( ScFormulaCell* pCell, size_t nRow ) :
    mpCell(pCell), mnRow(nRow), mnLength(0), mbShared(false) {}

size_t FormulaGroupContext::ColKey::Hash::operator ()( const FormulaGroupContext::ColKey& rKey ) const
{
    return rKey.mnTab * MAXCOLCOUNT + rKey.mnCol;
}

FormulaGroupContext::ColKey::ColKey( SCTAB nTab, SCCOL nCol ) : mnTab(nTab), mnCol(nCol) {}

bool FormulaGroupContext::ColKey::operator== ( const ColKey& r ) const
{
    return mnTab == r.mnTab && mnCol == r.mnCol;
}

FormulaGroupContext::ColArray::ColArray( NumArrayType* pNumArray, StrArrayType* pStrArray ) :
    mpNumArray(pNumArray), mpStrArray(pStrArray), mnSize(0)
{
    if (mpNumArray)
        mnSize = mpNumArray->size();
    else if (mpStrArray)
        mnSize = mpStrArray->size();
}

FormulaGroupContext::ColArray* FormulaGroupContext::getCachedColArray( SCTAB nTab, SCCOL nCol, size_t nSize )
{
    ColArraysType::iterator itColArray = maColArrays.find(ColKey(nTab, nCol));
    if (itColArray == maColArrays.end())
        // Not cached for this column.
        return nullptr;

    ColArray& rCached = itColArray->second;
    if (nSize > rCached.mnSize)
        // Cached data array is not long enough for the requested range.
        return nullptr;

    return &rCached;
}

FormulaGroupContext::ColArray* FormulaGroupContext::setCachedColArray(
    SCTAB nTab, SCCOL nCol, NumArrayType* pNumArray, StrArrayType* pStrArray )
{
    ColArraysType::iterator it = maColArrays.find(ColKey(nTab, nCol));
    if (it == maColArrays.end())
    {
        std::pair<ColArraysType::iterator,bool> r =
            maColArrays.emplace(ColKey(nTab, nCol), ColArray(pNumArray, pStrArray));

        if (!r.second)
            // Somehow the insertion failed.
            return nullptr;

        return &r.first->second;
    }

    // Prior array exists for this column. Overwrite it.
    ColArray& rArray = it->second;
    rArray = ColArray(pNumArray, pStrArray);
    return &rArray;
}

void FormulaGroupContext::ensureStrArray( ColArray& rColArray, size_t nArrayLen )
{
    if (rColArray.mpStrArray)
        return;

    m_StrArrays.push_back(
        o3tl::make_unique<sc::FormulaGroupContext::StrArrayType>(nArrayLen, nullptr));
    rColArray.mpStrArray = m_StrArrays.back().get();
}

void FormulaGroupContext::ensureNumArray( ColArray& rColArray, size_t nArrayLen )
{
    if (rColArray.mpNumArray)
        return;

    double fNan;
    rtl::math::setNan(&fNan);

    m_NumArrays.push_back(
        o3tl::make_unique<sc::FormulaGroupContext::NumArrayType>(nArrayLen, fNan));
    rColArray.mpNumArray = m_NumArrays.back().get();
}

FormulaGroupContext::FormulaGroupContext()
{
}

FormulaGroupContext::~FormulaGroupContext()
{
}

CompiledFormula::CompiledFormula() {}

CompiledFormula::~CompiledFormula() {}

FormulaGroupInterpreterSoftware::FormulaGroupInterpreterSoftware() : FormulaGroupInterpreter()
{
}

ScMatrixRef FormulaGroupInterpreterSoftware::inverseMatrix(const ScMatrix& /*rMat*/)
{
    return ScMatrixRef();
}

bool FormulaGroupInterpreterSoftware::interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                                                ScFormulaCellGroupRef& xGroup,
                                                ScTokenArray& rCode)
{
    typedef std::unordered_map<const formula::FormulaToken*, formula::FormulaTokenRef> CachedTokensType;

    // Decompose the group into individual cells and calculate them individually.

    // The caller must ensure that the top position is the start position of
    // the group.

    ScAddress aTmpPos = rTopPos;
    std::vector<formula::FormulaConstTokenRef> aResults;
    aResults.reserve(xGroup->mnLength);
    CachedTokensType aCachedTokens;

    double fNan;
    rtl::math::setNan(&fNan);

    for (SCROW i = 0; i < xGroup->mnLength; ++i, aTmpPos.IncRow())
    {
        ScTokenArray aCode2;
        formula::FormulaTokenArrayPlainIterator aIter(rCode);
        for (const formula::FormulaToken* p = aIter.First(); p; p = aIter.Next())
        {
            CachedTokensType::iterator it = aCachedTokens.find(p);
            if (it != aCachedTokens.end())
            {
                // This token is cached. Use the cached one.
                aCode2.AddToken(*it->second);
                continue;
            }

            switch (p->GetType())
            {
                case formula::svSingleVectorRef:
                {
                    const formula::SingleVectorRefToken* p2 = static_cast<const formula::SingleVectorRefToken*>(p);
                    const formula::VectorRefArray& rArray = p2->GetArray();

                    rtl_uString* pStr = nullptr;
                    double fVal = fNan;
                    if (static_cast<size_t>(i) < p2->GetArrayLength())
                    {
                        if (rArray.mpStringArray)
                            // See if the cell is of string type.
                            pStr = rArray.mpStringArray[i];

                        if (!pStr && rArray.mpNumericArray)
                            fVal = rArray.mpNumericArray[i];
                    }

                    if (pStr)
                    {
                        // This is a string cell.
                        svl::SharedStringPool& rPool = rDoc.GetSharedStringPool();
                        aCode2.AddString(rPool.intern(OUString(pStr)));
                    }
                    else if (rtl::math::isNan(fVal))
                        // Value of NaN represents an empty cell.
                        aCode2.AddToken(ScEmptyCellToken(false, false));
                    else
                        // Numeric cell.
                        aCode2.AddDouble(fVal);
                }
                break;
                case formula::svDoubleVectorRef:
                {
                    const formula::DoubleVectorRefToken* p2 = static_cast<const formula::DoubleVectorRefToken*>(p);
                    size_t nRowStart = p2->IsStartFixed() ? 0 : i;
                    size_t nRowEnd = p2->GetRefRowSize() - 1;
                    if (!p2->IsEndFixed())
                        nRowEnd += i;

                    assert(nRowStart <= nRowEnd);
                    ScMatrixRef pMat(new ScVectorRefMatrix(p2, nRowStart, nRowEnd - nRowStart + 1));

                    if (p2->IsStartFixed() && p2->IsEndFixed())
                    {
                        // Cached the converted token for absolute range reference.
                        ScComplexRefData aRef;
                        ScRange aRefRange = rTopPos;
                        aRefRange.aEnd.SetRow(rTopPos.Row() + nRowEnd);
                        aRef.InitRange(aRefRange);
                        formula::FormulaTokenRef xTok(new ScMatrixRangeToken(pMat, aRef));
                        aCachedTokens.emplace(p, xTok);
                        aCode2.AddToken(*xTok);
                    }
                    else
                    {
                        ScMatrixToken aTok(pMat);
                        aCode2.AddToken(aTok);
                    }
                }
                break;
                default:
                    aCode2.AddToken(*p);
            }
        }

        ScFormulaCell* pDest = rDoc.GetFormulaCell(aTmpPos);
        if (!pDest)
            return false;

        ScCompiler aComp(&rDoc, aTmpPos, aCode2);
        aComp.CompileTokenArray();
        ScInterpreter aInterpreter(pDest, &rDoc, rDoc.GetNonThreadedContext(), aTmpPos, aCode2);
        aInterpreter.Interpret();
        aResults.push_back(aInterpreter.GetResultToken());
    } // for loop end (xGroup->mnLength)

    if (!aResults.empty())
        rDoc.SetFormulaResults(rTopPos, &aResults[0], aResults.size());

    return true;
}

FormulaGroupInterpreter *FormulaGroupInterpreter::msInstance = nullptr;

void FormulaGroupInterpreter::MergeCalcConfig(const ScDocument& rDoc)
{
    maCalcConfig = ScInterpreter::GetGlobalConfig();
    maCalcConfig.MergeDocumentSpecific(rDoc.GetCalcConfig());
}

/// load and/or configure the correct formula group interpreter
FormulaGroupInterpreter *FormulaGroupInterpreter::getStatic()
{
    if ( !msInstance )
    {
#if HAVE_FEATURE_OPENCL
        if (ScCalcConfig::isOpenCLEnabled())
        {
            const ScCalcConfig& rConfig = ScInterpreter::GetGlobalConfig();
            switchOpenCLDevice(rConfig.maOpenCLDevice, rConfig.mbOpenCLAutoSelect);
        }
#endif

        if (!msInstance && ScCalcConfig::isSwInterpreterEnabled()) // software interpreter
        {
            SAL_INFO("sc.core.formulagroup", "Create S/W interpreter");
            msInstance = new sc::FormulaGroupInterpreterSoftware();
        }
    }

    return msInstance;
}

#if HAVE_FEATURE_OPENCL
void FormulaGroupInterpreter::fillOpenCLInfo(std::vector<OpenCLPlatformInfo>& rPlatforms)
{
    const std::vector<OpenCLPlatformInfo>& rPlatformsFromWrapper =
        openclwrapper::fillOpenCLInfo();

    rPlatforms.assign(rPlatformsFromWrapper.begin(), rPlatformsFromWrapper.end());
}

bool FormulaGroupInterpreter::switchOpenCLDevice(const OUString& rDeviceId, bool bAutoSelect, bool bForceEvaluation)
{
    bool bOpenCLEnabled = ScCalcConfig::isOpenCLEnabled();
    if (!bOpenCLEnabled || (rDeviceId == OPENCL_SOFTWARE_DEVICE_CONFIG_NAME))
    {
        bool bSwInterpreterEnabled = ScCalcConfig::isSwInterpreterEnabled();
        if (msInstance)
        {
            // if we already have a software interpreter don't delete it
            if (bSwInterpreterEnabled && dynamic_cast<sc::FormulaGroupInterpreterSoftware*>(msInstance))
                return true;

            delete msInstance;
            msInstance = nullptr;
        }

        if (bSwInterpreterEnabled)
        {
            msInstance = new sc::FormulaGroupInterpreterSoftware();
            return true;
        }

        return false;
    }

    OUString aSelectedCLDeviceVersionID;
    bool bSuccess = openclwrapper::switchOpenCLDevice(&rDeviceId, bAutoSelect, bForceEvaluation, aSelectedCLDeviceVersionID);

    if (!bSuccess)
        return false;

    delete msInstance;
    msInstance = new sc::opencl::FormulaGroupInterpreterOpenCL();

    return true;
}

void FormulaGroupInterpreter::getOpenCLDeviceInfo(sal_Int32& rDeviceId, sal_Int32& rPlatformId)
{
    rDeviceId = -1;
    rPlatformId = -1;
    bool bOpenCLEnabled = ScCalcConfig::isOpenCLEnabled();
    if(!bOpenCLEnabled)
        return;

    size_t aDeviceId = static_cast<size_t>(-1);
    size_t aPlatformId = static_cast<size_t>(-1);

    openclwrapper::getOpenCLDeviceInfo(aDeviceId, aPlatformId);
    rDeviceId = aDeviceId;
    rPlatformId = aPlatformId;
}

void FormulaGroupInterpreter::enableOpenCL_UnitTestsOnly()
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseOpenCL::set(true, batch);
    batch->commit();

    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();

    aConfig.mbOpenCLSubsetOnly = false;
    aConfig.mnOpenCLMinimumFormulaGroupSize = 2;

    ScInterpreter::SetGlobalConfig(aConfig);
}

void FormulaGroupInterpreter::disableOpenCL_UnitTestsOnly()
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseOpenCL::set(false, batch);
    batch->commit();
}

#endif

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
