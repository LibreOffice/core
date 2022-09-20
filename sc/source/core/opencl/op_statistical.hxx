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

class OpStandard: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Standard"; }
};
class OpExponDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ExponDist"; }
};
class OpZTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "ZTest"; }
};
class OpWeibull: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Weibull"; }
};
class OpFdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Fdist"; }
};
class OpTDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpTInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpTTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TTest"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpFisher: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Fisher"; }
};

class OpFisherInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "FisherInv"; }
};

class OpGamma: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Gamma"; }
};

class OpNegbinomdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNegbinomdist"; }
};

class OpGammaLn: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "GammaLn"; }
};

class OpGauss: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
       virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Gauss"; }
};

class OpGeoMean: public CheckVariables
{
public:
    OpGeoMean(): CheckVariables() {}
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "GeoMean"; }
};

class OpHarMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "HarMean"; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpNormdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNormdist"; }
};
class OpMedian:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpMedian"; }
};
class OpNormsdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNormsdist"; }
};
class OpNorminv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNorminv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpNormsinv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNormsinv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpPhi:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPhi"; }
};

class OpPermut:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPermut"; }
};
class OpPermutationA:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPermutationA";}
};

class OpConfidence: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(outputstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "Confidence"; }
};
class OpLogInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "LogInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpCritBinom: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(outputstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "CritBinom"; }
};
class OpLogNormDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "LogNormdist"; }
};
class OpGammaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "GammaDist"; }
};
class OpHypGeomDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpHypGeomDist"; }
};
class OpChiDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpChiDist"; }
};
class OpBinomdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpBinomdist"; }
};
class OpChiSqDist: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ChiSqDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpChiSqInv: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ChiSqInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};
class OpChiInv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpChiInv"; }
};
class OpPoisson:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpPoisson"; }
};

class OpGammaInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "GammaInv"; }
};
class OpFInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "FInv"; }
};
class OpFTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "FTest"; }
};
class OpDevSq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "DevSq"; }
    virtual bool canHandleMultiVector() const override { return true; }
};
class OpB: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "B"; }
};
class OpBetaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "BetaDist"; }
};
class OpBetainv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpBetainv"; }
};

class OpAveDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "AveDev"; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpCovar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Covar"; }
};

class OpForecast: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Forecast"; }
};

class OpInterceptSlopeBase: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override = 0;
protected:
    void GenerateCode( outputstream& ss, const std::string &sSymName,
        SubArguments &vSubArguments, const char* finalComputeCode );
};

class OpIntercept: public OpInterceptSlopeBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Intercept"; }
};

class OpSlope: public OpInterceptSlopeBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Slope"; }
};

class OpPearsonCovarBase: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override = 0;
protected:
    void GenerateCode( outputstream& ss, const std::string &sSymName,
        SubArguments &vSubArguments, double minimalCountValue, const char* finalComputeCode );
};

class OpPearson: public OpPearsonCovarBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPearson"; }
};

class OpCorrel: public OpPearson // they are identical
{
public:
    virtual std::string BinFuncName() const override { return "Correl"; }
};

class OpSTEYX: public OpPearsonCovarBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "STEYX"; }
};

class OpRsq: public OpPearsonCovarBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpRsq"; }
};

class OpVarStDevBase : public Normal
{
public:
    virtual void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual bool canHandleMultiVector() const override { return true; }
protected:
    // Generates function setup and checks, then generates a loop that will calculate
    // fMean and fCount from all arguments (ranges) and then a second loop that will
    // calculate vSum (pown(fsub_approx(arg,fMean),2)) from all arguments.
    void GenerateCode( outputstream& ss, const std::string& sSymName, SubArguments &vSubArguments );
};

class OpVar: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Var"; }
};

class OpVarP: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "VarP"; }
};

class OpStDev: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "StDev"; }
};

class OpStDevP: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "StDevP"; }
};

class OpSkew: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Skew"; }
};

class OpSkewp: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Skewp"; }
};

class OpKurt: public OpVarStDevBase
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Kurt"; }
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

class OpCountA: public OpCount
{
public:
    explicit OpCountA(int nResultSize) : OpCount(nResultSize) {}
    virtual std::string BinFuncName() const override { return "OpCountA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpMaxA: public OpMax
{
public:
    explicit OpMaxA(int nResultSize) : OpMax(nResultSize) {}
    virtual std::string BinFuncName() const override { return "OpMaxA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpMinA : public OpMin
{
public:
    explicit OpMinA(int nResultSize) : OpMin(nResultSize) {}
    virtual std::string BinFuncName() const override { return "OpMinA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpVarA: public OpVar
{
public:
    virtual std::string BinFuncName() const override { return "OpVarA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpVarPA: public OpVarP
{
public:
    virtual std::string BinFuncName() const override { return "OpVarPA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpStDevPA: public OpStDevP
{
public:
    virtual std::string BinFuncName() const override { return "OpStDevPA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpAverageA: public OpAverage
{
public:
    explicit OpAverageA(int nResultSize) : OpAverage(nResultSize) {}
    virtual std::string BinFuncName() const override { return "OpAverageA"; }
    virtual bool forceStringsToZero() const override { return true; }
};
class OpStDevA: public OpStDev
{
public:
    virtual std::string BinFuncName() const override { return "OpStDevA"; }
    virtual bool forceStringsToZero() const override { return true; }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
