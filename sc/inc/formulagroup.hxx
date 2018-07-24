/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_FORMULAGROUP_HXX
#define INCLUDED_SC_INC_FORMULAGROUP_HXX

#include <config_features.h>

#include "address.hxx"
#include "calcconfig.hxx"
#include "types.hxx"
#include "stlalgorithm.hxx"

#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif

#include <memory>
#include <unordered_map>
#include <vector>

class ScDocument;
class ScTokenArray;
class ScFormulaCell;

namespace sc {

struct FormulaGroupEntry
{
    union
    {
        ScFormulaCell* mpCell;   // non-shared formula cell
        ScFormulaCell** mpCells; // pointer to the top formula cell in a shared group.
    };

    size_t mnRow;
    size_t mnLength;
    bool mbShared;

    FormulaGroupEntry( ScFormulaCell** pCells, size_t nRow, size_t nLength );

    FormulaGroupEntry( ScFormulaCell* pCell, size_t nRow );
};

// Despite the name, this is actually a cache of cell values, used by OpenCL
// code ... I think. And obviously it's not really a struct either.
struct FormulaGroupContext
{
    typedef AlignedAllocator<double,256> DoubleAllocType;
    typedef std::vector<double, DoubleAllocType> NumArrayType;
    typedef std::vector<rtl_uString*> StrArrayType;
    typedef std::vector<std::unique_ptr<NumArrayType>> NumArrayStoreType;
    typedef std::vector<std::unique_ptr<StrArrayType>> StrArrayStoreType;

    struct ColKey
    {
        SCTAB mnTab;
        SCCOL mnCol;

        struct Hash
        {
            size_t operator() ( const ColKey& rKey ) const;
        };

        ColKey( SCTAB nTab, SCCOL nCol );

        bool operator== ( const ColKey& r ) const;
    };

    struct ColArray
    {
        NumArrayType* mpNumArray;
        StrArrayType* mpStrArray;
        size_t mnSize;

        ColArray( NumArrayType* pNumArray, StrArrayType* pStrArray );
    };

    typedef std::unordered_map<ColKey, ColArray, ColKey::Hash> ColArraysType;

    NumArrayStoreType m_NumArrays; /// manage life cycle of numeric arrays.
    StrArrayStoreType m_StrArrays; /// manage life cycle of string arrays.

    ColArraysType maColArrays; /// keep track of longest array for each column.

    ColArray* getCachedColArray( SCTAB nTab, SCCOL nCol, size_t nSize );

    ColArray* setCachedColArray(
        SCTAB nTab, SCCOL nCol, NumArrayType* pNumArray, StrArrayType* pStrArray );

    void discardCachedColArray(SCTAB nTab, SCCOL nCol);

    void ensureStrArray( ColArray& rColArray, size_t nArrayLen );
    void ensureNumArray( ColArray& rColArray, size_t nArrayLen );

    FormulaGroupContext();
    FormulaGroupContext(const FormulaGroupContext&) = delete;
    const FormulaGroupContext& operator=(const FormulaGroupContext&) = delete;
    ~FormulaGroupContext();
};

/**
 * Abstract base class for a "compiled" formula
 */
class SC_DLLPUBLIC CompiledFormula
{
public:
    CompiledFormula();
    virtual ~CompiledFormula();
};

/**
 * Abstract base class for vectorised formula group interpreters,
 * plus a global instance factory.
 */
class SC_DLLPUBLIC FormulaGroupInterpreter
{
    static FormulaGroupInterpreter *msInstance;

protected:
    ScCalcConfig maCalcConfig;

    FormulaGroupInterpreter() {}
    virtual ~FormulaGroupInterpreter() {}

    /// Merge global and document specific settings.
    void MergeCalcConfig(const ScDocument& rDoc);

public:
    static FormulaGroupInterpreter *getStatic();
#if HAVE_FEATURE_OPENCL
    static void fillOpenCLInfo(std::vector<OpenCLPlatformInfo>& rPlatforms);
    static bool switchOpenCLDevice(const OUString& rDeviceId, bool bAutoSelect, bool bForceEvaluation = false);
    // This is intended to be called from opencl-test.cxx only
    static void enableOpenCL_UnitTestsOnly();
    static void disableOpenCL_UnitTestsOnly();
    static void getOpenCLDeviceInfo(sal_Int32& rDeviceId, sal_Int32& rPlatformId);
#endif
    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat) = 0;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) = 0;
};

/// Inherit from this for alternate formula group calculation approaches.
class SC_DLLPUBLIC FormulaGroupInterpreterSoftware : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterSoftware();

    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat) override;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
