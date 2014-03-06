/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OPBASE_HXX
#define SC_OPENCL_OPBASE_HXX

#include <sal/log.hxx>

#include "clcc/clew.h"

#include "formula/token.hxx"
#include "formula/vectortoken.hxx"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <set>
#define ISNAN

namespace sc { namespace opencl {

class FormulaTreeNode;

/// Exceptions

/// Failed in parsing
class UnhandledToken
{
public:
    UnhandledToken(formula::FormulaToken *t,
            const char *const m, std::string fn="", int ln=0):
            mToken(t), mMessage(m), mFile(fn), mLineNumber(ln) {}
    formula::FormulaToken *mToken;
    std::string mMessage;
    std::string mFile;
    int mLineNumber;
};

/// Failed in marshaling
class OpenCLError
{
private:
    const char *strerror(cl_int i)
    {
#define CASE(val) case val: return #val
        switch (i)
        {
        CASE(CL_SUCCESS);
        CASE(CL_DEVICE_NOT_FOUND);
        CASE(CL_DEVICE_NOT_AVAILABLE);
        CASE(CL_COMPILER_NOT_AVAILABLE);
        CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
        CASE(CL_OUT_OF_RESOURCES);
        CASE(CL_OUT_OF_HOST_MEMORY);
        CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
        CASE(CL_MEM_COPY_OVERLAP);
        CASE(CL_IMAGE_FORMAT_MISMATCH);
        CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
        CASE(CL_BUILD_PROGRAM_FAILURE);
        CASE(CL_MAP_FAILURE);
        CASE(CL_INVALID_VALUE);
        CASE(CL_INVALID_DEVICE_TYPE);
        CASE(CL_INVALID_PLATFORM);
        CASE(CL_INVALID_DEVICE);
        CASE(CL_INVALID_CONTEXT);
        CASE(CL_INVALID_QUEUE_PROPERTIES);
        CASE(CL_INVALID_COMMAND_QUEUE);
        CASE(CL_INVALID_HOST_PTR);
        CASE(CL_INVALID_MEM_OBJECT);
        CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        CASE(CL_INVALID_IMAGE_SIZE);
        CASE(CL_INVALID_SAMPLER);
        CASE(CL_INVALID_BINARY);
        CASE(CL_INVALID_BUILD_OPTIONS);
        CASE(CL_INVALID_PROGRAM);
        CASE(CL_INVALID_PROGRAM_EXECUTABLE);
        CASE(CL_INVALID_KERNEL_NAME);
        CASE(CL_INVALID_KERNEL_DEFINITION);
        CASE(CL_INVALID_KERNEL);
        CASE(CL_INVALID_ARG_INDEX);
        CASE(CL_INVALID_ARG_VALUE);
        CASE(CL_INVALID_ARG_SIZE);
        CASE(CL_INVALID_KERNEL_ARGS);
        CASE(CL_INVALID_WORK_DIMENSION);
        CASE(CL_INVALID_WORK_GROUP_SIZE);
        CASE(CL_INVALID_WORK_ITEM_SIZE);
        CASE(CL_INVALID_GLOBAL_OFFSET);
        CASE(CL_INVALID_EVENT_WAIT_LIST);
        CASE(CL_INVALID_EVENT);
        CASE(CL_INVALID_OPERATION);
        CASE(CL_INVALID_GL_OBJECT);
        CASE(CL_INVALID_BUFFER_SIZE);
        CASE(CL_INVALID_MIP_LEVEL);
        CASE(CL_INVALID_GLOBAL_WORK_SIZE);
        default:
            return "Unknown OpenCL error code";
        }
#undef CASE
    }

public:
    OpenCLError(cl_int err, std::string fn, int ln): mError(err),
    mFile(fn), mLineNumber(ln)
    {
        SAL_INFO("sc.opencl", "OpenCLError:" << mError << ": " << strerror(mError));
    }
    cl_int mError;
    std::string mFile;
    int mLineNumber;
};

/// Inconsistent state
class Unhandled
{
public:
    Unhandled(std::string fn="", int ln=0):
            mFile(fn), mLineNumber(ln) {}
    std::string mFile;
    int mLineNumber;
};

typedef boost::shared_ptr<FormulaTreeNode> FormulaTreeNodeRef;

class FormulaTreeNode
{
public:
    FormulaTreeNode(const formula::FormulaToken* ft): mpCurrentFormula(ft)
    {
        Children.reserve(8);
    }
    std::vector<FormulaTreeNodeRef> Children;
    formula::FormulaToken *GetFormulaToken(void) const
    {
        return const_cast<formula::FormulaToken*>(mpCurrentFormula.get());
    }
private:
    formula::FormulaConstTokenRef mpCurrentFormula;
};

/// (Partially) abstract base class for an operand
class DynamicKernelArgument : boost::noncopyable
{
public:
    DynamicKernelArgument(const std::string &s, FormulaTreeNodeRef ft);

    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const = 0;

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const = 0;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef(bool=false) const = 0;

    /// When Mix, it will be called
    virtual std::string GenDoubleSlidingWindowDeclRef(bool=false) const
    { return std::string(""); }

    /// When Mix, it will be called
    virtual std::string GenStringSlidingWindowDeclRef(bool=false) const
    { return std::string(""); }

    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel, int, int, cl_program) = 0;

    virtual ~DynamicKernelArgument() {}

    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    const std::string &GetSymName(void) const { return mSymName; }
    formula::FormulaToken *GetFormulaToken(void) const;
    virtual size_t GetWindowSize(void) const = 0;
    virtual std::string DumpOpName(void) const { return std::string(""); }
    virtual void DumpInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) const {}
    const std::string& GetName(void) const { return mSymName; }
    virtual bool NeedParallelReduction(void) const { return false; }

protected:
    std::string mSymName;
    FormulaTreeNodeRef mFormulaTree;
};

/// Holds an input (read-only) argument reference to a SingleVectorRef.
/// or a DoubleVectorRef for non-sliding-window argument of complex functions
/// like SumOfProduct
/// In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class VectorRef : public DynamicKernelArgument
{
public:
    VectorRef(const std::string &s, FormulaTreeNodeRef ft, int index = 0);

    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const;
    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef(bool=false) const;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel, int, int, cl_program);

    virtual ~VectorRef();

    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    const std::string &GetSymName(void) const { return mSymName; }
    virtual size_t GetWindowSize(void) const;
    virtual std::string DumpOpName(void) const { return std::string(""); }
    virtual void DumpInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) const {}
    const std::string& GetName(void) const { return mSymName; }
    virtual cl_mem GetCLBuffer(void) const { return mpClmem; }
    virtual bool NeedParallelReduction(void) const { return false; }

protected:
    // Used by marshaling
    cl_mem mpClmem;
    // index in multiple double vector refs that have multiple ranges
    const int mnIndex;
};
/// Abstract class for code generation

class OpBase
{
public:
    typedef std::vector<std::string> ArgVector;
    typedef std::vector<std::string>::iterator ArgVectorIter;
    virtual std::string GetBottom(void) {return "";};
    virtual std::string Gen2(const std::string &/*lhs*/,
        const std::string &/*rhs*/) const {return "";}
    virtual std::string Gen(ArgVector& /*argVector*/){return "";};
    virtual std::string BinFuncName(void)const {return "";};
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) {}
    virtual bool takeString() const = 0;
    virtual bool takeNumeric() const = 0;
    virtual ~OpBase() {}
};

class SlidingFunctionBase : public OpBase
{
public:
    typedef boost::shared_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArguments;
    virtual void GenSlidingWindowFunction(std::stringstream &,
        const std::string&, SubArguments &) = 0;
    virtual ~SlidingFunctionBase() {};
};

class Normal: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual bool takeString() const { return false; }
    virtual bool takeNumeric() const { return true; }
};

class CheckVariables:public Normal
{
public:
    void GenTmpVariables(std::stringstream &ss, SubArguments &vSubArguments);
    void CheckSubArgumentIsNan(std::stringstream &ss,
            SubArguments &vSubArguments, int argumentNum);
    void CheckAllSubArgumentIsNan(std::stringstream &ss,
            SubArguments &vSubArguments);
    // only check isNan
    void CheckSubArgumentIsNan2(std::stringstream &ss,
            SubArguments &vSubArguments, int argumentNum, std::string p);
    void UnrollDoubleVector(std::stringstream &ss,
    std::stringstream &unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
    int nCurWindowSize);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
