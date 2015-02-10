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

#include <formula/opcode.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <svl/sharedstringpool.hxx>

#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_set.hpp>

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

struct FormulaGroupContext : boost::noncopyable
{
    typedef AlignedAllocator<double,256> DoubleAllocType;
    typedef std::vector<double, DoubleAllocType> NumArrayType;
    typedef std::vector<rtl_uString*> StrArrayType;
    typedef boost::ptr_vector<NumArrayType> NumArrayStoreType;
    typedef boost::ptr_vector<StrArrayType> StrArrayStoreType;

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
        bool operator!= ( const ColKey& r ) const;
    };

    struct ColArray
    {
        NumArrayType* mpNumArray;
        StrArrayType* mpStrArray;
        size_t mnSize;

        ColArray( NumArrayType* pNumArray, StrArrayType* pStrArray );
    };

    typedef boost::unordered_map<ColKey, ColArray, ColKey::Hash> ColArraysType;

    NumArrayStoreType maNumArrays; /// manage life cycle of numeric arrays.
    StrArrayStoreType maStrArrays; /// manage life cycle of string arrays.

    ColArraysType maColArrays; /// keep track of longest array for each column.

    ColArray* getCachedColArray( SCTAB nTab, SCCOL nCol, size_t nSize );

    ColArray* setCachedColArray(
        SCTAB nTab, SCCOL nCol, NumArrayType* pNumArray, StrArrayType* pStrArray );

    void ensureStrArray( ColArray& rColArray, size_t nArrayLen );
    void ensureNumArray( ColArray& rColArray, size_t nArrayLen );

    FormulaGroupContext();
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
    static void enableOpenCL(bool bEnable, bool bEnableCompletely = false, const std::set<OpCodeEnum>& rSubsetToEnable = std::set<OpCodeEnum>());
    static void getOpenCLDeviceInfo(sal_Int32& rDeviceId, sal_Int32& rPlatformId);
#endif
    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat) = 0;
    virtual CompiledFormula* createCompiledFormula(ScDocument& rDoc,
                                                   const ScAddress& rTopPos,
                                                   ScFormulaCellGroup& rGroup,
                                                   ScTokenArray& rCode) = 0;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) = 0;
};

/// Inherit from this for alternate formula group calculation approaches.
class SC_DLLPUBLIC FormulaGroupInterpreterSoftware : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterSoftware();
    virtual ~FormulaGroupInterpreterSoftware() {}

    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat) SAL_OVERRIDE;
    virtual CompiledFormula* createCompiledFormula(ScDocument& rDoc,
                                                   const ScAddress& rTopPos,
                                                   ScFormulaCellGroup& rGroup,
                                                   ScTokenArray& rCode) SAL_OVERRIDE;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) SAL_OVERRIDE;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
