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

namespace sc::opencl {

class OpCos: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Cos"; }
};
class OpSec: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Sec"; }
};
class OpSecH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "SecH"; }
};
class OpMROUND: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "MROUND"; }
};

class OpCsc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Csc"; }
};
class OpSumIfs final : public CheckVariables
{
public:
    OpSumIfs(): CheckVariables(), mNeedReductionKernel(false) {}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumIfs"; }
    bool NeedReductionKernel() const { return mNeedReductionKernel; }
private:
    bool mNeedReductionKernel;
};
class OpCosh: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Cosh"; }
};
class OpSinh: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Sinh"; }
};
class OpSin: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Sin"; }
};
class OpAbs:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScAbs"; }
};
class OpArcCos:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScACos"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpArcCosHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "1.0"; }
    virtual std::string BinFuncName() const override { return "ScACosH"; }
};
class OpTan: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Tan"; }
};
class OpTanH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "TanH"; }
};
class OpSqrt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Sqrt"; }
};
class OpArcCot:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScACot"; }
};
class OpArcCotHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "2.0"; }
    virtual std::string BinFuncName() const override { return "ScACotH"; }
};
class OpArcSin:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScASin"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpArcSinHyp:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScASinH"; }
};
class OpTrunc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Trunc"; }
};
class OpArcTan2:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScATan2"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpArcTan:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScATan"; }
};
class OpArcTanH:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScATanH"; }
};
class OpBitAnd:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
        virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScBitAnd"; }
};
class OpBitOr:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScBitOr"; }
};
class OpBitXor:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScBitXor"; }
};
class OpBitLshift:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScBitLshift"; }
};
class OpBitRshift:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScBitRshift"; }
};
class OpLn: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Ln"; }
};
class OpRound: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Round"; }
};
class OpRoundUp: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "RoundUp"; }
};
class OpRoundDown: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "RoundDown"; }
};
class OpInt: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Int"; }
};
class OpRadians: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Radians"; }
};
class OpIsEven: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "IsEven"; }
};
class OpIsOdd: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "IsOdd"; }
};
class OpCot: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Cot"; }
};
class OpSumSQ: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "SumSQ"; }
};
class OpCoth: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
            std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Coth"; }
};
class OpPower: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Power"; }
};
class OpOdd: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Odd"; }
};
class OpFloor: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Floor"; }
};
class OpCscH: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "CscH"; }
};
class OpCeil:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScCeil"; }
};
class OpExp: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Exp"; }
};
class OpLog10: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Log10"; }
};
class OpConvert: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Convert"; }
    virtual bool takeString() const override { return true; }

};
class OpEven: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Even"; }
};
class OpAverageIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "AverageIfs"; }
};
class OpCountIfs: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "CountIfs"; }
};
class OpMod: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Mod"; }
};
class OpProduct: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Product"; }
    virtual bool canHandleMultiVector() const override { return true; }
};
class OpSqrtPi: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "SqrtPi"; }
};

class OpCombinA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Combina"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};
class OpLog: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Log"; }
};
class OpCombin: public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "ScCombin"; }
};
class OpAverageIf: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "AverageIf"; }
};
class OpDeg: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Degrees"; }
};
class OpCountIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Countif"; }
};
class OpFact: public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual std::string GetBottom() override { return "0.0"; }
    virtual std::string BinFuncName() const override { return "Fact"; }
};
class OpSeriesSum: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SeriesSum"; }
};
class OpSumIf: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumIf"; }
};
class OpQuotient: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Quotient"; }
};
class OpNegSub: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "NegSub"; }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
