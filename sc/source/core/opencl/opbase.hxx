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

#include "clcc/clew.h"

#include "formula/token.hxx"

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
public:
    OpenCLError(cl_int err): mError(err) {}
    cl_int mError;
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
    FormulaTreeNode(formula::FormulaToken *ft): mpCurrentFormula(ft)
    {
        Children.reserve(8);
    }
    std::vector<FormulaTreeNodeRef> Children;
    formula::FormulaToken *GetFormulaToken(void) const
    {
        return mpCurrentFormula;
    }
private:
    formula::FormulaToken *const mpCurrentFormula;
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
        const std::string, SubArguments &) = 0;
    virtual ~SlidingFunctionBase() {};
};

class Normal: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
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
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
