/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OPBASE_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OPBASE_HXX

#include <sal/log.hxx>

#include <clew/clew.h>

#include <formula/token.hxx>
#include <formula/vectortoken.hxx>
#include <boost/noncopyable.hpp>
#include <memory>
#include <set>

#include "calcconfig.hxx"

namespace sc { namespace opencl {

class FormulaTreeNode;

/// Exceptions

/// Failed in parsing
class UnhandledToken
{
public:
    UnhandledToken( formula::FormulaToken* t, const char* m, const std::string& fn = "", int ln = 0 );

    formula::FormulaToken* mToken;
    std::string mMessage;
    std::string mFile;
    int mLineNumber;
};

/// Failed in marshaling
class OpenCLError
{
public:
    OpenCLError( const std::string& function, cl_int error, const std::string& file, int line );

    std::string mFunction;
    cl_int mError;
    std::string mFile;
    int mLineNumber;
};

/// Inconsistent state
class Unhandled
{
public:
    Unhandled( const std::string& fn = "", int ln = 0 );

    std::string mFile;
    int mLineNumber;
};

typedef std::shared_ptr<FormulaTreeNode> FormulaTreeNodeRef;

class FormulaTreeNode
{
public:
    explicit FormulaTreeNode( const formula::FormulaToken* ft ) : mpCurrentFormula(ft)
    {
        Children.reserve(8);
    }
    std::vector<FormulaTreeNodeRef> Children;
    formula::FormulaToken* GetFormulaToken() const
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
    DynamicKernelArgument( const ScCalcConfig& config, const std::string& s, FormulaTreeNodeRef ft );
    virtual ~DynamicKernelArgument() {}

    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const = 0;

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const = 0;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const = 0;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) = 0;

    virtual size_t GetWindowSize() const = 0;

    /// When Mix, it will be called
    virtual std::string GenDoubleSlidingWindowDeclRef( bool = false ) const;

    /// When Mix, it will be called
    virtual std::string GenStringSlidingWindowDeclRef( bool = false ) const;

    virtual bool IsMixedArgument() const;

    /// Generate use/references to the argument
    virtual void GenDeclRef( std::stringstream& ss ) const;
    virtual void GenNumDeclRef( std::stringstream& ss ) const;

    virtual void GenStringDeclRef( std::stringstream& ss ) const;

    virtual void GenSlidingWindowFunction( std::stringstream& );
    formula::FormulaToken* GetFormulaToken() const;
    virtual std::string DumpOpName() const;
    virtual void DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const;
    const std::string& GetName() const;
    virtual bool NeedParallelReduction() const;

protected:
    const ScCalcConfig& mCalcConfig;
    std::string mSymName;
    FormulaTreeNodeRef mFormulaTree;
};

typedef std::shared_ptr<DynamicKernelArgument> DynamicKernelArgumentRef;

/// Holds an input (read-only) argument reference to a SingleVectorRef.
/// or a DoubleVectorRef for non-sliding-window argument of complex functions
/// like SumOfProduct
/// In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class VectorRef : public DynamicKernelArgument
{
public:
    VectorRef( const ScCalcConfig& config, const std::string& s, FormulaTreeNodeRef ft, int index = 0 );
    virtual ~VectorRef();

    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override;
    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) override;

    virtual void GenSlidingWindowFunction( std::stringstream& ) override;
    virtual size_t GetWindowSize() const override;
    virtual std::string DumpOpName() const override;
    virtual void DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const override;
    const std::string& GetName() const;
    cl_mem GetCLBuffer() const;
    virtual bool NeedParallelReduction() const override;

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
    virtual std::string GetBottom() { return "";};
    virtual std::string Gen2( const std::string&/*lhs*/,
        const std::string&/*rhs*/ ) const { return "";}
    static std::string Gen( ArgVector& /*argVector*/ ) { return "";};
    virtual std::string BinFuncName() const { return "";};
    virtual void BinInlineFun( std::set<std::string>&,
        std::set<std::string>& ) { }
    virtual bool takeString() const = 0;
    virtual bool takeNumeric() const = 0;
    //Continue process 'Zero' or Not(like OpMul, not continue process when meet
    // 'Zero'
    virtual bool ZeroReturnZero() { return false;}
    virtual ~OpBase() { }
};

class SlidingFunctionBase : public OpBase
{
public:
    typedef std::vector<DynamicKernelArgumentRef> SubArguments;
    virtual void GenSlidingWindowFunction( std::stringstream&,
        const std::string&, SubArguments& ) = 0;
    virtual ~SlidingFunctionBase() { }
};

class Normal : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( std::stringstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override;
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

class CheckVariables : public Normal
{
public:
    static void GenTmpVariables( std::stringstream& ss, SubArguments& vSubArguments );
    static void CheckSubArgumentIsNan( std::stringstream& ss,
        SubArguments& vSubArguments, int argumentNum );
    static void CheckAllSubArgumentIsNan( std::stringstream& ss,
        SubArguments& vSubArguments );
    // only check isNan
    static void CheckSubArgumentIsNan2( std::stringstream& ss,
        SubArguments& vSubArguments, int argumentNum, std::string p );
    static void UnrollDoubleVector( std::stringstream& ss,
        std::stringstream& unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
        int nCurWindowSize );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
