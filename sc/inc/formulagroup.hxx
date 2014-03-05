/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_FORMULAGROUP_HXX
#define SC_FORMULAGROUP_HXX

#include "address.hxx"
#include "types.hxx"
#include "platforminfo.hxx"
#include <stlalgorithm.hxx>

#include "svl/sharedstringpool.hxx"

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_set.hpp>

class ScDocument;
class ScTokenArray;

namespace sc {

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
    FormulaGroupInterpreter() {}
    virtual ~FormulaGroupInterpreter() {}

 public:
    static FormulaGroupInterpreter *getStatic();
    static void fillOpenCLInfo(std::vector<OpenclPlatformInfo>& rPlatforms);
    static bool switchOpenCLDevice(const OUString& rDeviceId, bool bAutoSelect, bool bForceEvaluation = false);
    static void enableOpenCL(bool bEnable);
    static void getOpenCLDeviceInfo(sal_Int32& rDeviceId, sal_Int32& rPlatformId);

    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat) = 0;
    virtual CompiledFormula* createCompiledFormula(ScDocument& rDoc,
                                                   const ScAddress& rTopPos,
                                                   ScFormulaCellGroupRef& xGroup,
                                                   ScTokenArray& rCode) = 0;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) = 0;
};

/// Inherit from this for alternate formula group calculation approaches.
class SC_DLLPUBLIC FormulaGroupInterpreterSoftware : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterSoftware();
    virtual ~FormulaGroupInterpreterSoftware() {}

    virtual ScMatrixRef inverseMatrix(const ScMatrix& rMat);
    virtual CompiledFormula* createCompiledFormula(ScDocument& rDoc,
                                                   const ScAddress& rTopPos,
                                                   ScFormulaCellGroupRef& xGroup,
                                                   ScTokenArray& rCode) SAL_OVERRIDE;
    virtual bool interpret(ScDocument& rDoc, const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) SAL_OVERRIDE;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
