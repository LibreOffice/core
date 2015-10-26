/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include "formulagroup.hxx"
#include "formulagroupcl.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "scmatrix.hxx"
#include "globalnames.hxx"

#include <formula/vectortoken.hxx>
#include <officecfg/Office/Common.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <rtl/bootstrap.hxx>

#include <cstdio>
#include <unordered_map>
#include <vector>

#if HAVE_FEATURE_OPENCL
#include <opencl/openclwrapper.hxx>
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

bool FormulaGroupContext::ColKey::operator!= ( const ColKey& r ) const
{
    return !operator==(r);
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
        return NULL;

    ColArray& rCached = itColArray->second;
    if (nSize > rCached.mnSize)
        // Cached data array is not long enough for the requested range.
        return NULL;

    return &rCached;
}

FormulaGroupContext::ColArray* FormulaGroupContext::setCachedColArray(
    SCTAB nTab, SCCOL nCol, NumArrayType* pNumArray, StrArrayType* pStrArray )
{
    ColArraysType::iterator it = maColArrays.find(ColKey(nTab, nCol));
    if (it == maColArrays.end())
    {
        std::pair<ColArraysType::iterator,bool> r =
            maColArrays.insert(
                ColArraysType::value_type(ColKey(nTab, nCol), ColArray(pNumArray, pStrArray)));

        if (!r.second)
            // Somehow the insertion failed.
            return NULL;

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

    maStrArrays.push_back(
        new sc::FormulaGroupContext::StrArrayType(nArrayLen, NULL));
    rColArray.mpStrArray = &maStrArrays.back();
}

void FormulaGroupContext::ensureNumArray( ColArray& rColArray, size_t nArrayLen )
{
    if (rColArray.mpNumArray)
        return;

    double fNan;
    rtl::math::setNan(&fNan);

    maNumArrays.push_back(
        new sc::FormulaGroupContext::NumArrayType(nArrayLen, fNan));
    rColArray.mpNumArray = &maNumArrays.back();
}

FormulaGroupContext::FormulaGroupContext()
{
}

FormulaGroupContext::~FormulaGroupContext()
{
}

namespace {

/**
 * Input double array consists of segments of NaN's and normal values.
 * Insert only the normal values into the matrix while skipping the NaN's.
 */
void fillMatrix( ScMatrix& rMat, size_t nCol, const double* pNums, size_t nLen )
{
    const double* pNum = pNums;
    const double* pNumEnd = pNum + nLen;
    const double* pNumHead = NULL;
    for (; pNum != pNumEnd; ++pNum)
    {
        if (!rtl::math::isNan(*pNum))
        {
            if (!pNumHead)
                // Store the first non-NaN position.
                pNumHead = pNum;

            continue;
        }

        if (pNumHead)
        {
            // Flush this non-NaN segment to the matrix.
            rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
            pNumHead = NULL;
        }
    }

    if (pNumHead)
    {
        // Flush last non-NaN segment to the matrix.
        rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
    }
}

void flushStrSegment(
    ScMatrix& rMat, size_t nCol, rtl_uString** pHead, rtl_uString** pCur, rtl_uString** pTop )
{
    size_t nOffset = pHead - pTop;
    std::vector<svl::SharedString> aStrs;
    aStrs.reserve(pCur - pHead);
    for (; pHead != pCur; ++pHead)
        aStrs.push_back(svl::SharedString(*pHead, *pHead));

    rMat.PutString(&aStrs[0], aStrs.size(), nCol, nOffset);
}

void fillMatrix( ScMatrix& rMat, size_t nCol, rtl_uString** pStrs, size_t nLen )
{
    rtl_uString** p = pStrs;
    rtl_uString** pEnd = p + nLen;
    rtl_uString** pHead = NULL;
    for (; p != pEnd; ++p)
    {
        if (*p)
        {
            if (!pHead)
                // Store the first non-empty string position.
                pHead = p;

            continue;
        }

        if (pHead)
        {
            // Flush this non-empty segment to the matrix.
            flushStrSegment(rMat, nCol, pHead, p, pStrs);
            pHead = NULL;
        }
    }

    if (pHead)
    {
        // Flush last non-empty segment to the matrix.
        flushStrSegment(rMat, nCol, pHead, p, pStrs);
    }
}

void fillMatrix( ScMatrix& rMat, size_t nCol, const double* pNums, rtl_uString** pStrs, size_t nLen )
{
    if (!pStrs)
    {
        fillMatrix(rMat, nCol, pNums, nLen);
        return;
    }

    const double* pNum = pNums;
    const double* pNumHead = NULL;
    rtl_uString** pStr = pStrs;
    rtl_uString** pStrEnd = pStr + nLen;
    rtl_uString** pStrHead = NULL;

    for (; pStr != pStrEnd; ++pStr, ++pNum)
    {
        if (*pStr)
        {
            // String cell exists.

            if (pNumHead)
            {
                // Flush this numeric segment to the matrix.
                rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
                pNumHead = NULL;
            }

            if (!pStrHead)
                // Store the first non-empty string position.
                pStrHead = pStr;

            continue;
        }

        // No string cell. Check the numeric cell value.

        if (pStrHead)
        {
            // Flush this non-empty string segment to the matrix.
            flushStrSegment(rMat, nCol, pStrHead, pStr, pStrs);
            pStrHead = NULL;
        }

        if (!rtl::math::isNan(*pNum))
        {
            // Numeric cell exists.
            if (!pNumHead)
                // Store the first non-NaN position.
                pNumHead = pNum;

            continue;
        }

        // Empty cell. No action required.
    }

    if (pStrHead)
    {
        // Flush the last non-empty segment to the matrix.
        flushStrSegment(rMat, nCol, pStrHead, pStr, pStrs);
    }
    else if (pNumHead)
    {
        // Flush the last numeric segment to the matrix.
        rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
    }
}

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

CompiledFormula* FormulaGroupInterpreterSoftware::createCompiledFormula(
    ScFormulaCellGroup& /*rGroup*/, ScTokenArray& /*rCode*/ )
{
    return NULL;
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
    std::vector<formula::FormulaTokenRef> aResults;
    aResults.reserve(xGroup->mnLength);
    CachedTokensType aCachedTokens;

    double fNan;
    rtl::math::setNan(&fNan);

    for (SCROW i = 0; i < xGroup->mnLength; ++i, aTmpPos.IncRow())
    {
        ScTokenArray aCode2;
        for (const formula::FormulaToken* p = rCode.First(); p; p = rCode.Next())
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

                    rtl_uString* pStr = NULL;
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
                    const std::vector<formula::VectorRefArray>& rArrays = p2->GetArrays();
                    size_t nColSize = rArrays.size();
                    size_t nRowStart = p2->IsStartFixed() ? 0 : i;
                    size_t nRowEnd = p2->GetRefRowSize() - 1;
                    if (!p2->IsEndFixed())
                        nRowEnd += i;
                    size_t nRowSize = nRowEnd - nRowStart + 1;
                    ScMatrixRef pMat(new ScMatrix(nColSize, nRowSize));

                    size_t nDataRowEnd = p2->GetArrayLength() - 1;
                    if (nRowStart > nDataRowEnd)
                        // Referenced rows are all empty.
                        nRowSize = 0;
                    else if (nRowEnd > nDataRowEnd)
                        // Data array is shorter than the row size of the reference. Truncate it to the data.
                        nRowSize -= nRowEnd - nDataRowEnd;

                    for (size_t nCol = 0; nCol < nColSize; ++nCol)
                    {
                        const formula::VectorRefArray& rArray = rArrays[nCol];
                        if (rArray.mpStringArray)
                        {
                            if (rArray.mpNumericArray)
                            {
                                // Mixture of string and numeric values.
                                const double* pNums = rArray.mpNumericArray;
                                pNums += nRowStart;
                                rtl_uString** pStrs = rArray.mpStringArray;
                                pStrs += nRowStart;
                                fillMatrix(*pMat, nCol, pNums, pStrs, nRowSize);
                            }
                            else
                            {
                                // String cells only.
                                rtl_uString** pStrs = rArray.mpStringArray;
                                pStrs += nRowStart;
                                fillMatrix(*pMat, nCol, pStrs, nRowSize);
                            }
                        }
                        else if (rArray.mpNumericArray)
                        {
                            // Numeric cells only.
                            const double* pNums = rArray.mpNumericArray;
                            pNums += nRowStart;
                            fillMatrix(*pMat, nCol, pNums, nRowSize);
                        }
                    }

                    if (p2->IsStartFixed() && p2->IsEndFixed())
                    {
                        // Cached the converted token for absolute range reference.
                        ScComplexRefData aRef;
                        ScRange aRefRange = rTopPos;
                        aRefRange.aEnd.SetRow(rTopPos.Row() + nRowEnd);
                        aRef.InitRange(aRefRange);
                        formula::FormulaTokenRef xTok(new ScMatrixRangeToken(pMat, aRef));
                        aCachedTokens.insert(CachedTokensType::value_type(p, xTok));
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
        ScInterpreter aInterpreter(pDest, &rDoc, aTmpPos, aCode2);
        aInterpreter.Interpret();
        aResults.push_back(aInterpreter.GetResultToken());
    } // for loop end (xGroup->mnLength)

    if (!aResults.empty())
        rDoc.SetFormulaResults(rTopPos, &aResults[0], aResults.size());

    return true;
}

FormulaGroupInterpreter *FormulaGroupInterpreter::msInstance = NULL;

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
        const ScCalcConfig& rConfig = ScInterpreter::GetGlobalConfig();
        if (ScCalcConfig::isOpenCLEnabled())
            switchOpenCLDevice(rConfig.maOpenCLDevice, rConfig.mbOpenCLAutoSelect);
#endif
        static bool bAllowSoftwareInterpreter = (getenv("SC_ALLOW_BROKEN_SOFTWARE_INTERPRETER") != NULL);

        if ( !msInstance && bAllowSoftwareInterpreter ) // software fallback
        {
            SAL_INFO("sc.formulagroup", "Create S/W interpreter");
            msInstance = new sc::FormulaGroupInterpreterSoftware();
        }
    }

    return msInstance;
}

#if HAVE_FEATURE_OPENCL
void FormulaGroupInterpreter::fillOpenCLInfo(std::vector<OpenCLPlatformInfo>& rPlatforms)
{
    const std::vector<OpenCLPlatformInfo>& rPlatformsFromWrapper =
        ::opencl::fillOpenCLInfo();

    rPlatforms.assign(rPlatformsFromWrapper.begin(), rPlatformsFromWrapper.end());
}

bool FormulaGroupInterpreter::switchOpenCLDevice(const OUString& rDeviceId, bool bAutoSelect, bool bForceEvaluation)
{
    bool bOpenCLEnabled = ScCalcConfig::isOpenCLEnabled();
    static bool bAllowSoftwareInterpreter = (getenv("SC_ALLOW_BROKEN_SOFTWARE_INTERPRETER") != NULL);
    if (!bOpenCLEnabled || (bAllowSoftwareInterpreter && rDeviceId == OPENCL_SOFTWARE_DEVICE_CONFIG_NAME))
    {
        if(msInstance)
        {
            // if we already have a software interpreter don't delete it
            if(dynamic_cast<sc::FormulaGroupInterpreterSoftware*>(msInstance))
                return true;

            delete msInstance;
        }

        msInstance = new sc::FormulaGroupInterpreterSoftware();
        return true;
    }
    bool bSuccess = ::opencl::switchOpenCLDevice(&rDeviceId, bAutoSelect, bForceEvaluation);
    if(!bSuccess)
        return false;

    delete msInstance;
    msInstance = NULL;

    if (ScCalcConfig::isOpenCLEnabled())
    {
        msInstance = new sc::opencl::FormulaGroupInterpreterOpenCL();
        return msInstance != NULL;
    }

    return false;
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

    ::opencl::getOpenCLDeviceInfo(aDeviceId, aPlatformId);
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

#endif

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
