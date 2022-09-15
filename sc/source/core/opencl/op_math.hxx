/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "opbase.hxx"
#include "utils.hxx"

namespace sc::opencl {

/// Implements functions in the form of FUNC(x), e.g. COS().
/// The function should take one simple argument (i.e. no ranges).
class OpMathOneArgument : public Normal
{
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    /// This writes out OpenCL code returning the computed value, the argument is "arg0".
    virtual void GenerateCode( outputstream& ss ) const = 0;
};

/// Implements functions in the form of FUNC(x1, x2), e.g. ATAN2().
/// The function should take exactly two simple arguments (i.e. no ranges).
class OpMathTwoArguments : public Normal
{
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    /// This writes out OpenCL code returning the computed value, the arguments are "arg0" and "arg1".
    virtual void GenerateCode( outputstream& ss ) const = 0;
};


class OpCos: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Cos"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSec: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Sec"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSecH: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "SecH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpMROUND: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "MROUND"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpCsc: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Csc"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSumIfs final : public CheckVariables
{
public:
    OpSumIfs(): CheckVariables(), mNeedReductionKernel(false) {}
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumIfs"; }
    bool NeedReductionKernel() const { return mNeedReductionKernel; }
private:
    bool mNeedReductionKernel;
};

class OpCosh: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Cosh"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSinh: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Sinh"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSin: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Sin"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpAbs: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScAbs"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcCos: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScACos"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcCosHyp : public OpMathOneArgument
{
public:
    virtual std::string GetBottom() override { return "1.0"; }
    virtual std::string BinFuncName() const override { return "ScACosH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpTan: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Tan"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpTanH: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "TanH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSqrt: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Sqrt"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcCot : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScACot"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcCotHyp : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScACotH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcSin : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScASin"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcSinHyp : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScASinH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpTrunc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Trunc"; }
};

class OpArcTan2 : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScATan2"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcTan : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScATan"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpArcTanH : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "ScATanH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpBitAnd : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScBitAnd"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpBitOr : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScBitOr"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpBitXor : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScBitXor"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpBitLshift : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScBitLshift"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpBitRshift : public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScBitRshift"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpLn: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Ln"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpRound: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Round"; }
};
class OpRoundUp: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "RoundUp"; }
};
class OpRoundDown: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "RoundDown"; }
};
class OpInt: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Int"; }
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual void BinInlineFun(std::set<std::string>& , std::set<std::string>& ) override;
};

class OpRadians: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Radians"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpIsEven: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "IsEven"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpIsOdd: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "IsOdd"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpCot: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Cot"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSumSQ: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "SumSQ"; }
};

class OpCoth: public OpMathOneArgument
{
public:
    virtual void BinInlineFun(std::set<std::string>& ,
            std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Coth"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpPower: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "Power"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpOdd: public OpMathOneArgument
{
public:
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Odd"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpFloor: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Floor"; }
};

class OpCscH: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "CscH"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpCeil:public Normal{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScCeil"; }
};

class OpExp: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Exp"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpLog10: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Log10"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpEven: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Even"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpAverageIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "AverageIfs"; }
};
class OpCountIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "CountIfs"; }
};
class OpMod: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "Mod"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpProduct: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Product"; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpSqrtPi: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "SqrtPi"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpCombinA: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "Combina"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpLog: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Log"; }
};

class OpCombin: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "ScCombin"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpAverageIf: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "AverageIf"; }
};
class OpDeg: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Degrees"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpCountIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Countif"; }
};
class OpFact : public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "Fact"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpSeriesSum: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SeriesSum"; }
};
class OpSumIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumIf"; }
};

class OpQuotient: public OpMathTwoArguments
{
public:
    virtual std::string BinFuncName() const override { return "Quotient"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpNegSub: public OpMathOneArgument
{
public:
    virtual std::string BinFuncName() const override { return "NegSub"; }
    virtual void GenerateCode( outputstream& ss ) const override;
};

class OpEqual : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "eq"; }
    virtual void BinInlineFun(std::set<std::string>& , std::set<std::string>& ) override;
};

class OpNotEqual : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "neq"; }
    virtual void BinInlineFun(std::set<std::string>& , std::set<std::string>& ) override;
};

class OpLessEqual : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "le"; }
    virtual void BinInlineFun(std::set<std::string>& , std::set<std::string>& ) override;
};

class OpLess : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "lt"; }
};

class OpGreaterEqual : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "ge"; }
    virtual void BinInlineFun(std::set<std::string>& , std::set<std::string>& ) override;
};

class OpGreater : public OpMathTwoArguments
{
public:
    virtual void GenerateCode( outputstream& ss ) const override;
    virtual std::string BinFuncName() const override { return "gt"; }
};

class SumOfProduct : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( outputstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override;
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

class OpSumProduct : public SumOfProduct
{
public:
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName() const override { return "fsop"; }
};

class Reduction : public SlidingFunctionBase
{
    int const mnResultSize;
public:
    explicit Reduction(int nResultSize) : mnResultSize(nResultSize) {}

    typedef DynamicKernelSlidingArgument<VectorRef> NumericRange;
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

/// operator traits
class OpNop : public Reduction
{
public:
    explicit OpNop(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& ) const override
    {
        return lhs;
    }
    virtual std::string BinFuncName() const override { return "nop"; }
};

class OpCount : public Reduction
{
public:
    explicit OpCount(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        outputstream ss;
        ss << "(isnan(" << lhs << ")?" << rhs << ":" << rhs << "+1.0)";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "fcount"; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpSum : public Reduction
{
public:
    explicit OpSum(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        outputstream ss;
        ss << "fsum_approx((" << lhs << "),(" << rhs << "))";
        return ss.str();
    }
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "fsum"; }
    // All arguments are simply summed, so it doesn't matter if SvDoubleVector is split.
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpAverage : public Reduction
{
public:
    explicit OpAverage(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        outputstream ss;
        ss << "fsum_count(" << lhs << "," << rhs << ", &nCount)";
        return ss.str();
    }
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "average"; }
    virtual bool isAverage() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpSub : public Reduction
{
public:
    explicit OpSub(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fsub_approx(" + lhs + "," + rhs + ")";
    }
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "fsub"; }
};

class OpMul : public Reduction
{
public:
    explicit OpMul(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "1"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName() const override { return "fmul"; }
    virtual bool ZeroReturnZero() override { return true; }
};

/// Technically not a reduction, but fits the framework.
class OpDiv : public Reduction
{
public:
    explicit OpDiv(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "1.0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "(" + rhs + "==0 ? CreateDoubleError(DivisionByZero) : (" + lhs + "/" + rhs + ") )";
    }
    virtual std::string BinFuncName() const override { return "fdiv"; }

    virtual bool HandleNaNArgument( outputstream& ss, unsigned argno, SubArguments& vSubArguments ) const override
    {
        if (argno == 1)
        {
            ss <<
                "if (isnan(" << vSubArguments[argno]->GenSlidingWindowDeclRef() << ")) {\n"
                "    return CreateDoubleError(DivisionByZero);\n"
                "}\n";
            return true;
        }
        else if (argno == 0)
        {
            ss <<
                "if (isnan(" << vSubArguments[argno]->GenSlidingWindowDeclRef() << ") &&\n"
                "    !(isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef() << ") || " << vSubArguments[1]->GenSlidingWindowDeclRef() << " == 0)) {\n"
                "    return 0;\n"
                "}\n";
        }
        return false;
    }

};

class OpMin : public Reduction
{
public:
    explicit OpMin(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "NAN"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fmin_count(" + lhs + "," + rhs + ", &nCount)";
    }
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "min"; }
    virtual bool isMinOrMax() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpMax : public Reduction
{
public:
    explicit OpMax(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "NAN"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fmax_count(" + lhs + "," + rhs + ", &nCount)";
    }
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "max"; }
    virtual bool isMinOrMax() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
