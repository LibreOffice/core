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
#include <comphelper/threadpool.hxx>
#include <tools/cpuid.hxx>

#include <formula/vectortoken.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

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

void FormulaGroupContext::discardCachedColArray( SCTAB nTab, SCCOL nCol )
{
    ColArraysType::iterator itColArray = maColArrays.find(ColKey(nTab, nCol));
    if (itColArray != maColArrays.end())
        maColArrays.erase(itColArray);
}

void FormulaGroupContext::ensureStrArray( ColArray& rColArray, size_t nArrayLen )
{
    if (rColArray.mpStrArray)
        return;

    m_StrArrays.push_back(
        std::make_unique<sc::FormulaGroupContext::StrArrayType>(nArrayLen, nullptr));
    rColArray.mpStrArray = m_StrArrays.back().get();
}

void FormulaGroupContext::ensureNumArray( ColArray& rColArray, size_t nArrayLen )
{
    if (rColArray.mpNumArray)
        return;

    double fNan;
    rtl::math::setNan(&fNan);

    m_NumArrays.push_back(
        std::make_unique<sc::FormulaGroupContext::NumArrayType>(nArrayLen, fNan));
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
            if( !switchOpenCLDevice(rConfig.maOpenCLDevice, rConfig.mbOpenCLAutoSelect))
            {
                if( ScCalcConfig::getForceCalculationType() == ForceCalculationOpenCL )
                {
                    SAL_WARN( "opencl", "OpenCL forced but failed to initialize" );
                    abort();
                }
            }
        }
#endif
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
        delete msInstance;
        msInstance = nullptr;
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
