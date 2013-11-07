/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_INC_DYNAMICKERNEL_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_DYNAMICKERNEL_HXX

#include <config_features.h>

#include "formulagroup.hxx"

#if !HAVE_FEATURE_OPENCL

namespace sc { namespace opencl {

class DynamikcKernel : public CompiledFormula
{
}

} // namespace opencl

} // namespace sc

#else

#include "clew.h"

#include "document.hxx"
#include "opbase.hxx"

namespace sc { namespace opencl {

class DynamicKernelArgument;
class SlidingFunctionBase;

/// Holds the symbol table for a given dynamic kernel
class SymbolTable {
public:
    typedef std::map<const formula::FormulaToken *,
        boost::shared_ptr<DynamicKernelArgument> > ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    typedef std::list< boost::shared_ptr<DynamicKernelArgument> > ArgumentList;
    SymbolTable(void):mCurId(0) {}
    template <class T>
    const DynamicKernelArgument *DeclRefArg(FormulaTreeNodeRef, SlidingFunctionBase* pCodeGen);
    /// Used to generate sliding window helpers
    void DumpSlidingWindowFunctions(std::stringstream &ss);
    /// Memory mapping from host to device and pass buffers to the given kernel as
    /// arguments
    void Marshal(cl_kernel, int, cl_program);
private:
    unsigned int mCurId;
    ArgumentMap mSymbols;
    ArgumentList mParams;
};

class DynamicKernel : public CompiledFormula
{
public:
    DynamicKernel(FormulaTreeNodeRef r):mpRoot(r),
        mpProgram(NULL), mpKernel(NULL), mpResClmem(NULL) {}
    static DynamicKernel *create(ScDocument& rDoc,
                                 const ScAddress& rTopPos,
                                 ScTokenArray& rCode);
    /// OpenCL code generation
    void CodeGen();
    /// Produce kernel hash
    std::string GetMD5(void);
    /// Create program, build, and create kerenl
    /// TODO cache results based on kernel body hash
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void CreateKernel(void);
    /// Prepare buffers, marshal them to GPU, and launch the kernel
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void Launch(size_t nr);
    ~DynamicKernel();
    cl_mem GetResultBuffer(void) const { return mpResClmem; }
private:
    void TraverseAST(FormulaTreeNodeRef);
    FormulaTreeNodeRef mpRoot;
    SymbolTable mSyms;
    std::string mKernelSignature, mKernelHash;
    std::string mFullProgramSrc;
    cl_program mpProgram;
    cl_kernel mpKernel;
    cl_mem mpResClmem; // Results
    std::set<std::string> inlineDecl;
    std::set<std::string> inlineFun;
};

}

}

#endif // HAVE_FEATURE_OPENCL

#endif // INCLUDED_SC_SOURCE_CORE_INC_DYNAMICKERNEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
