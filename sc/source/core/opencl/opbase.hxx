/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <clew/clew.h>
#include <formula/token.hxx>
#include <formula/types.hxx>
#include <formula/vectortoken.hxx>
#include <memory>
#include <set>
#include <vector>
#include "utils.hxx"

struct ScCalcConfig;

namespace sc::opencl {

// FIXME: The idea that somebody would bother to (now and then? once a year? once a month?) manually
// edit a source file and change the value of some #defined constant and run some ill-defined
// "correctness test" is of course ludicrous. Either things are checked in normal unit tests, in
// every 'make check', or not at all. The below comments are ridiculous.

#define REDUCE_THRESHOLD 201  // set to 4 for correctness testing. priority 1
#define UNROLLING_FACTOR 16  // set to 4 for correctness testing (if no reduce)


class FormulaTreeNode;

/// Exceptions

/// Failed in parsing
class UnhandledToken
{
public:
    UnhandledToken( const char* m, std::string fn, int ln );

    std::string mMessage;
    std::string mFile;
    int mLineNumber;
};

/// Failed in marshaling
class OpenCLError
{
public:
    OpenCLError( std::string  function, cl_int error, std::string file, int line );

    std::string mFunction;
    cl_int mError;
    std::string mFile;
    int mLineNumber;
};

/// Inconsistent state
class Unhandled
{
public:
    Unhandled( std::string fn, int ln );

    std::string mFile;
    int mLineNumber;
};

class InvalidParameterCount
{
public:
    InvalidParameterCount( int parameterCount, std::string file, int ln );

    int mParameterCount;
    std::string mFile;
    int const mLineNumber;
};

// Helper macros to be used in code emitting OpenCL code for Calc functions.
// Require the vSubArguments parameter.
#define CHECK_PARAMETER_COUNT(min, max) \
    do { \
        const int count = vSubArguments.size(); \
        if( count < ( min ) || count > ( max )) \
            throw InvalidParameterCount( count, __FILE__, __LINE__ ); \
    } while( false )
#define CHECK_PARAMETER_COUNT_MIN(min) \
    do { \
        const int count = vSubArguments.size(); \
        if( count < ( min )) \
            throw InvalidParameterCount( count, __FILE__, __LINE__ ); \
    } while( false )

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
class DynamicKernelArgument
{
public:
    /// delete copy constructor
    DynamicKernelArgument( const DynamicKernelArgument& ) = delete;

    /// delete copy-assignment operator
    const DynamicKernelArgument& operator=( const DynamicKernelArgument& ) = delete;

    DynamicKernelArgument( const ScCalcConfig& config, std::string s, FormulaTreeNodeRef  ft );
    virtual ~DynamicKernelArgument() {}

    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const = 0;

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( outputstream& ss ) const = 0;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const = 0;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) = 0;

    virtual size_t GetWindowSize() const = 0;

    /// When Mix, it will be called
    virtual std::string GenDoubleSlidingWindowDeclRef( bool = false ) const;

    /// When Mix, it will be called
    virtual std::string GenStringSlidingWindowDeclRef( bool = false ) const;

    /// Generate use/references to the argument
    virtual void GenDeclRef( outputstream& ss ) const;

    virtual void GenSlidingWindowFunction( outputstream& );
    formula::FormulaToken* GetFormulaToken() const;
    virtual std::string DumpOpName() const;
    virtual void DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const;
    const std::string& GetName() const;
    virtual bool NeedParallelReduction() const;
    /// If there's actually no argument, i.e. it expands to no code.
    virtual bool IsEmpty() const { return false; }

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
    VectorRef( const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft, int index = 0 );
    virtual ~VectorRef() override;

    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override;
    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override;

    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override;

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) override;

    virtual void GenSlidingWindowFunction( outputstream& ) override;
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

/// A vector of strings
class DynamicKernelStringArgument : public VectorRef
{
public:
    DynamicKernelStringArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft, int index = 0 ) :
        VectorRef(config, s, ft, index) { }

    virtual void GenSlidingWindowFunction( outputstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        ss << "__global unsigned int *" << mSymName;
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        DynamicKernelStringArgument::GenDecl(ss);
    }
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) override;
};

/// Abstract class for code generation
class OpBase
{
public:
    // FIXME: What exactly is this? It seems to be a starting value for some calculations
    // (1 for OpMul, MAXFLOAT for OpMin), but it's often used pointlessly and sometimes
    // even incorrectly (default value for when the cell is empty).
    virtual std::string GetBottom() { return "";};
    virtual std::string Gen2( const std::string&/*lhs*/,
        const std::string&/*rhs*/ ) const { return "";}
    static std::string Gen( std::vector<std::string>& /*argVector*/ ) { return "";};
    virtual std::string BinFuncName() const { return "";};
    virtual void BinInlineFun( std::set<std::string>&,
        std::set<std::string>& ) { }
    virtual bool takeString() const = 0;
    virtual bool takeNumeric() const = 0;
    // Whether DoubleRef containing more than one column is handled properly.
    virtual bool canHandleMultiVector() const { return false; }
    //Continue process 'Zero' or Not(like OpMul, not continue process when meet
    // 'Zero'
    virtual bool ZeroReturnZero() { return false;}
    virtual ~OpBase() { }
};

class SlidingFunctionBase : public OpBase
{
public:
    typedef std::vector<DynamicKernelArgumentRef> SubArguments;
    virtual void GenSlidingWindowFunction( outputstream&,
        const std::string&, SubArguments& ) = 0;
protected:
    // generate code for "double <name> = <value>;" from vSubArguments
    static void GenerateArg( const char* name, int num, SubArguments& vSubArguments, outputstream& ss );
    // overload, variable will be named "arg<num>"
    static void GenerateArg( int num, SubArguments& vSubArguments, outputstream& ss );
    // generate code for "double <name> = <value>;" from vSubArguments, if it exists,
    // otherwise set to <def>
    static void GenerateArgWithDefault( const char* name, int num, double def, SubArguments& vSubArguments,
        outputstream& ss );
    void GenerateFunctionDeclaration( const std::string& sSymName,
        SubArguments& vSubArguments, outputstream& ss );
};

class Normal : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( outputstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override;
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

class CheckVariables : public Normal
{
public:
    static void GenTmpVariables( outputstream& ss, const SubArguments& vSubArguments );
    static void CheckSubArgumentIsNan( outputstream& ss,
        SubArguments& vSubArguments, int argumentNum );
    static void CheckAllSubArgumentIsNan( outputstream& ss,
        SubArguments& vSubArguments );
    // only check isnan
    static void CheckSubArgumentIsNan2( outputstream& ss,
        SubArguments& vSubArguments, int argumentNum, const std::string& p );
    static void UnrollDoubleVector( outputstream& ss,
        const outputstream& unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
        int nCurWindowSize );
};

class OpAverage;
class OpCount;

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
/// Generate a sequential loop for reductions
template<class Base>
class DynamicKernelSlidingArgument : public Base
{
public:
    DynamicKernelSlidingArgument(const ScCalcConfig& config, const std::string& s,
                                 const FormulaTreeNodeRef& ft,
                                 std::shared_ptr<SlidingFunctionBase> CodeGen, int index);
    // Should only be called by SumIfs. Yikes!
    virtual bool NeedParallelReduction() const;
    virtual void GenSlidingWindowFunction( outputstream& ) { }

    std::string GenSlidingWindowDeclRef( bool nested = false ) const;
    /// Controls how the elements in the DoubleVectorRef are traversed
    size_t GenReductionLoopHeader( outputstream& ss, bool& needBody );

    size_t GetArrayLength() const { return mpDVR->GetArrayLength(); }

    size_t GetWindowSize() const { return mpDVR->GetRefRowSize(); }

    bool GetStartFixed() const { return bIsStartFixed; }

    bool GetEndFixed() const { return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken* mpDVR;
    // from parent nodes
    std::shared_ptr<SlidingFunctionBase> mpCodeGen;
};

/// Handling a Double Vector that is used as a sliding window input
/// Performs parallel reduction based on given operator
template<class Base>
class ParallelReductionVectorRef : public Base
{
public:
    ParallelReductionVectorRef(const ScCalcConfig& config, const std::string& s,
                               const FormulaTreeNodeRef& ft,
                               std::shared_ptr<SlidingFunctionBase> CodeGen, int index);
    ~ParallelReductionVectorRef();

    /// Emit the definition for the auxiliary reduction kernel
    virtual void GenSlidingWindowFunction( outputstream& ss );
    virtual std::string GenSlidingWindowDeclRef( bool ) const;
    /// Controls how the elements in the DoubleVectorRef are traversed
    size_t GenReductionLoopHeader( outputstream& ss, int nResultSize, bool& needBody );
    virtual size_t Marshal( cl_kernel k, int argno, int w, cl_program mpProgram );
    size_t GetArrayLength() const { return mpDVR->GetArrayLength(); }
    size_t GetWindowSize() const { return mpDVR->GetRefRowSize(); }
    bool GetStartFixed() const { return bIsStartFixed; }
    bool GetEndFixed() const { return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken* mpDVR;
    // from parent nodes
    std::shared_ptr<SlidingFunctionBase> mpCodeGen;
    // controls whether to invoke the reduction kernel during marshaling or not
    cl_mem mpClmem2;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
