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
#include <opencl/OpenCLZone.hxx>
#include <sal/log.hxx>
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

constexpr auto REDUCE_THRESHOLD = 201;  // set to 4 for correctness testing. priority 1
constexpr auto UNROLLING_FACTOR = 16;  // set to 4 for correctness testing (if no reduce)


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
#define CHECK_PARAMETER_DOUBLEVECTORREF(arg) \
    do { \
        formula::FormulaToken *token = vSubArguments[arg]->GetFormulaToken(); \
        if (token == nullptr || token->GetType() != formula::svDoubleVectorRef) \
            throw Unhandled(__FILE__, __LINE__); \
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

    /// Will generate value saying whether the value is a string.
    virtual std::string GenIsString( bool = false ) const { return "false"; }

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

    static void ClearStringIds();

protected:
    const ScCalcConfig& mCalcConfig;
    std::string mSymName;
    FormulaTreeNodeRef mFormulaTree;
    static int GetStringId( const rtl_uString* string );
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
    // Makes Marshall convert strings to 0 values.
    bool forceStringsToZero;
    // Used for storing when the data needs to be modified before sending to OpenCL.
    std::vector< double > dataBuffer;
};

// Sets VectorRef::forceStringsToZero.
class VectorRefStringsToZero : public VectorRef
{
public:
    VectorRefStringsToZero( const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft, int index = 0 );
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
        ss << "__global double *" << mSymName;
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        DynamicKernelStringArgument::GenDecl(ss);
    }
    virtual std::string GenIsString( bool = false ) const override;
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) override;
};

/// Arguments that are actually compile-time constants
class DynamicKernelConstantArgument : public DynamicKernelArgument
{
public:
    DynamicKernelConstantArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        if (GetFormulaToken()->GetType() != formula::svDouble)
            throw Unhandled(__FILE__, __LINE__);
        return mSymName;
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    virtual double GetDouble() const
    {
        formula::FormulaToken* Tok = GetFormulaToken();
        if (Tok->GetType() != formula::svDouble)
            throw Unhandled(__FILE__, __LINE__);
        return Tok->GetDouble();
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        OpenCLZone zone;
        double tmp = GetDouble();
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": double: " << preciseFloat( tmp ));
        cl_int err = clSetKernelArg(k, argno, sizeof(double), static_cast<void*>(&tmp));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

// Constant 0 argument when a string is forced to zero.
class DynamicKernelStringToZeroArgument : public DynamicKernelConstantArgument
{
public:
    DynamicKernelStringToZeroArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelConstantArgument(config, s, ft) { }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        return mSymName;
    }
    virtual double GetDouble() const override { return 0; }
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
    // For use with COUNTA() etc, input strings will be converted to 0 in data.
    virtual bool forceStringsToZero() const { return false; }
    virtual ~OpBase() { }
};

class SlidingFunctionBase : public OpBase
{
public:
    typedef std::vector<DynamicKernelArgumentRef> SubArguments;
    virtual void GenSlidingWindowFunction( outputstream&,
        const std::string&, SubArguments& ) = 0;
protected:
    // This enum controls how the generated code will handle empty cells in ranges.
    enum EmptyArgType
    {
        EmptyIsZero, // empty cells become 0.0
        EmptyIsNan,  // empty cells become NAN, use isnan() to check in code
        SkipEmpty    // empty cells will be skipped
    };
    // This enum controls whether the generated code will also include variable
    // <name>_is_string that will be set depending on the value type.
    enum GenerateArgTypeType
    {
        DoNotGenerateArgType,
        GenerateArgType
    };
    void GenerateFunctionDeclaration( const std::string& sSymName,
        SubArguments& vSubArguments, outputstream& ss );
    // Generate code for "double <name> = <value>;" from vSubArguments, svDoubleVectorRef is not supported.
    void GenerateArg( const char* name, int arg, SubArguments& vSubArguments, outputstream& ss,
        EmptyArgType empty = EmptyIsZero, GenerateArgTypeType generateType = DoNotGenerateArgType );
    // overload, variable will be named "arg<arg>"
    void GenerateArg( int arg, SubArguments& vSubArguments, outputstream& ss,
        EmptyArgType empty = EmptyIsZero, GenerateArgTypeType generateType = DoNotGenerateArgType );
    // generate code for "double <name> = <value>;" from vSubArguments, if it exists,
    // otherwise set to <def>
    void GenerateArgWithDefault( const char* name, int arg, double def, SubArguments& vSubArguments,
        outputstream& ss, EmptyArgType empty = EmptyIsZero );
    // Generate code that will handle all arguments firstArg-lastArg (zero-based, inclusive),
    // including range arguments (svDoubleVectorRef) and each value will be processed by 'code',
    // value will be named "arg".
    static void GenerateRangeArgs( int firstArg, int lastArg, SubArguments& vSubArguments,
        outputstream& ss, EmptyArgType empty, const char* code );
    // overload, handle all arguments
    static void GenerateRangeArgs( SubArguments& vSubArguments, outputstream& ss,
        EmptyArgType empty, const char* code );
    // overload, handle the given argument
    static void GenerateRangeArg( int arg, SubArguments& vSubArguments, outputstream& ss,
        EmptyArgType empty, const char* code );
    // Overload.
    // Both arguments must be svDoubleRef of the same size.
    // If 'firstElementDiff' is set, the loop start will be offset by '+ firstElementDiff'.
    void GenerateRangeArg( int arg1, int arg2, SubArguments& vSubArguments,
        outputstream& ss, EmptyArgType empty, const char* code, const char* firstElementDiff = nullptr );
    // Generate code that will handle the given two arguments in one loop where n-th element of arg1 and arg2
    // will be handled at the same time, named 'arg1' and 'arg2'.
    // Both arguments must be svDoubleRef of the same size.
    // If 'firstElementDiff' is set, the loop start will be offset by '+ firstElementDiff'.
    static void GenerateRangeArgPair( int arg1, int arg2, SubArguments& vSubArguments,
        outputstream& ss, EmptyArgType empty, const char* code, const char* firstElementDiff = nullptr );
    // Generate code for "double <name> = range[<element>]" from vSubArguments.
    // The argument must be svDoubleRef.
    static void GenerateRangeArgElement( const char* name, int arg, const char* element,
        SubArguments& vSubArguments, outputstream& ss, EmptyArgType empty );
    static void GenerateDoubleVectorLoopHeader( outputstream& ss,
        const formula::DoubleVectorRefToken* pDVR, const char* firstElementDiff );
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

class Reduction : public SlidingFunctionBase
{
    int const mnResultSize;
public:
    explicit Reduction(int nResultSize) : mnResultSize(nResultSize) {}

    typedef DynamicKernelSlidingArgument<VectorRef> NumericRange;
    typedef DynamicKernelSlidingArgument<VectorRefStringsToZero> NumericRangeStringsToZero;
    typedef DynamicKernelSlidingArgument<DynamicKernelStringArgument> StringRange;
    typedef ParallelReductionVectorRef<VectorRef> ParallelNumericRange;

    virtual bool HandleNaNArgument( outputstream&, unsigned, SubArguments& ) const
    {
        return false;
    }

    virtual void GenSlidingWindowFunction( outputstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override;
    virtual bool isAverage() const { return false; }
    virtual bool isMinOrMax() const { return false; }
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
