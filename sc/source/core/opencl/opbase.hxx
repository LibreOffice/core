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
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <set>
#define ISNAN

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
private:
    const char* strerror( cl_int i ) const;

public:
    OpenCLError( cl_int err, const std::string& fn, int ln );

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

typedef boost::shared_ptr<FormulaTreeNode> FormulaTreeNodeRef;

class FormulaTreeNode
{
public:
    FormulaTreeNode( const formula::FormulaToken* ft ) : mpCurrentFormula(ft)
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
    virtual void GenDecl( std::stringstream& ss ) const SAL_OVERRIDE;
    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const SAL_OVERRIDE;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const SAL_OVERRIDE;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) SAL_OVERRIDE;

    virtual void GenSlidingWindowFunction( std::stringstream& ) SAL_OVERRIDE;
    virtual size_t GetWindowSize() const SAL_OVERRIDE;
    virtual std::string DumpOpName() const SAL_OVERRIDE;
    virtual void DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const SAL_OVERRIDE;
    const std::string& GetName() const;
    virtual cl_mem GetCLBuffer() const;
    virtual bool NeedParallelReduction() const SAL_OVERRIDE;

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
    virtual std::string Gen( ArgVector& /*argVector*/ ) { return "";};
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
    typedef boost::shared_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArguments;
    virtual void GenSlidingWindowFunction( std::stringstream&,
        const std::string&, SubArguments& ) = 0;
    virtual ~SlidingFunctionBase() { }
};

class Normal : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( std::stringstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) SAL_OVERRIDE;
    virtual bool takeString() const SAL_OVERRIDE { return false; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};

class CheckVariables : public Normal
{
public:
    void GenTmpVariables( std::stringstream& ss, SubArguments& vSubArguments );
    void CheckSubArgumentIsNan( std::stringstream& ss,
        SubArguments& vSubArguments, int argumentNum );
    void CheckAllSubArgumentIsNan( std::stringstream& ss,
        SubArguments& vSubArguments );
    // only check isNan
    void CheckSubArgumentIsNan2( std::stringstream& ss,
        SubArguments& vSubArguments, int argumentNum, std::string p );
    void UnrollDoubleVector( std::stringstream& ss,
        std::stringstream& unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
        int nCurWindowSize );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
