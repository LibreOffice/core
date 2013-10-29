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

#define ISNAN

namespace sc { namespace opencl {
/// Exceptions

/// Failed in parsing
class UnhandledToken
{
public:
    UnhandledToken(formula::FormulaToken *t): mToken(t) {}
    formula::FormulaToken *mToken;
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
    Unhandled() {}
};

class FormulaTreeNode
{
public:
    FormulaTreeNode(formula::FormulaToken *ft): mpCurrentFormula(ft)
    {
        Children.reserve(8);
    }
    std::vector<boost::shared_ptr<FormulaTreeNode> > Children;
    formula::FormulaToken *GetFormulaToken(void) const
    {
        return mpCurrentFormula;
    }
private:
    formula::FormulaToken *const mpCurrentFormula;
};

/// Holds an input (read-only) argument reference to a SingleVectorRef.
/// or a DoubleVectorRef for non-sliding-window argument of complex functions
/// like SumOfProduct
/// In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class DynamicKernelArgument
{
public:
    DynamicKernelArgument(const std::string &s, boost::shared_ptr<FormulaTreeNode> ft);

    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const;

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef(bool=false) const;

    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel, int, int);

    virtual ~DynamicKernelArgument();

    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    const std::string &GetSymName(void) const { return mSymName; }
    formula::FormulaToken *GetFormulaToken(void) const;
    virtual size_t GetWindowSize(void) const;
    virtual std::string DumpOpName(void) const { return std::string(""); }
    const std::string& GetName(void) const { return mSymName; }
protected:
    const std::string mSymName;
    boost::shared_ptr<FormulaTreeNode> mFormulaTree;
    // Used by marshaling
    cl_mem mpClmem;
};

/// Abstract class for code generation

class SlidingFunctionBase
{
public:
    typedef std::unique_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArguments;
    virtual void GenSlidingWindowFunction(std::stringstream &,
        const std::string, SubArguments &) = 0;
    virtual ~SlidingFunctionBase() {};
};

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
    virtual ~OpBase() {}
};

class Normal: public SlidingFunctionBase, public OpBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
