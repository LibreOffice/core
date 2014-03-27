/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OP_MATH_HXX
#define SC_OPENCL_OP_MATH_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class OpCos: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Cos"; }
};
class OpSec: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Sec"; }
};
class OpSecH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SecH"; }
};
class OpMROUND: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "MROUND"; }
};


class OpCsc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Csc"; }
};
class OpSumIfs: public CheckVariables
{
public:
    OpSumIfs(void): CheckVariables(), mNeedReductionKernel(false) {}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SumIfs"; }
    bool NeedReductionKernel(void) const { return mNeedReductionKernel; }
protected:
    bool mNeedReductionKernel;
};
class OpCosh: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Cosh"; }
};
class OpSinh: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Sinh"; }
};
class OpSin: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Sin"; }
};
class OpAbs:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScAbs"; }
};
class OpArcCos:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScACos"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpArcCosHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "1.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScACosH"; }
};
class OpTan: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Tan"; }
};
class OpTanH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "TanH"; }
};
class OpSqrt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Sqrt"; }
};
class OpArcCot:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScACot"; }
};
class OpArcCotHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "2.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScACotH"; }
};
class OpArcSin:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScASin"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpArcSinHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScASinH"; }
};
class OpTrunc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Trunc"; }
};
class OpArcTan2:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScATan2"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpArcTan:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScATan"; }
};
class OpArcTanH:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScATanH"; }
};
class OpBitAnd:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
        virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScBitAnd"; }
};
class OpBitOr:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScBitOr"; }
};
class OpBitXor:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScBitXor"; }
};
class OpBitLshift:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScBitLshift"; }
};
class OpBitRshift:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScBitRshift"; }
};
class OpLn: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Ln"; }
};
class OpRound: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Round"; }
};
class OpRoundUp: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "RoundUp"; }
};
class OpRoundDown: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "RoundDown"; }
};
class OpInt: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Int"; }
};
class OpRadians: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Radians"; }
};
class OpIsEven: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "IsEven"; }
};
class OpIsOdd: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "IsOdd"; }
};
class OpCot: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Cot"; }
};
class OpSumSQ: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SumSQ"; }
};
class OpCoth: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
            std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coth"; }
};
class OpPower: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Power"; }
};
class OpOdd: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Odd"; }
};
class OpFloor: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Floor"; }
};
class OpCscH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "CscH"; }
};
class OpCeil:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScCeil"; }
};
class OpExp: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Exp"; }
};
class OpLog10: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Log10"; }
};
class OpConvert: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Convert"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }

};
class OpEven: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Even"; }
};
class OpAverageIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "AverageIfs"; }
};
class OpCountIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "CountIfs"; }
};
class OpMod: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Mod"; }
};
class OpProduct: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Product"; }
};
class OpSqrtPi: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SqrtPi"; }
};

class OpCombina: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Combina"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};
class OpLog: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Log"; }
};
class OpKombin: public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ScKombin"; }
};
class OpAverageIf: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "AverageIf"; }
};
class OpDeg: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Degrees"; }
};
class OpCountIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Countif"; }
};
class OpFact: public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0.0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Fact"; }
};
class OpSeriesSum: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SeriesSum"; }
};
class OpSumIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SumIf"; }
};
class OpQuotient: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Quotient"; }
};
class OpNegSub: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "NegSub"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
