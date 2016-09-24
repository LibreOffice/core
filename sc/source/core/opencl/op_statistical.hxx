/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OP_STATISTICAL_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OP_STATISTICAL_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class OpStandard: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Standard"; }
};
class OpExponDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ExponDist"; }
};
class OpVar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Var"; }
};
class OpSTEYX: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "STEYX"; }
};
class OpVarP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "VarP"; }
};
class OpZTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "ZTest"; }
};
class OpStDevP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "StDevP"; }
};

class OpStDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "StDev"; }
};
class OpSkewp: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Skewp"; }
};
class OpSlope: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Slope"; }
};
class OpWeibull: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Weibull"; }
};
class OpFdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Fdist"; }
};
class OpTDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpTInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpTTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "TTest"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
};
class OpSkew: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Skew"; }
};
class OpFisher: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Fisher"; }
};

class OpFisherInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "FisherInv"; }
};

class OpGamma: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Gamma"; }
};

class OpCorrel: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Correl"; }
};

class OpNegbinomdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNegbinomdist"; }
};

class OpPearson: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPearson"; }
};

class OpGammaLn: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "GammaLn"; }
};

class OpGauss: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
       virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Gauss"; }
};

class OpGeoMean: public CheckVariables
{
public:
    OpGeoMean(): CheckVariables() {}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "GeoMean"; }
};

class OpHarMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "HarMean"; }
};

class OpRsq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpRsq"; }
};
class OpNormdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNormdist"; }
};
class OpMedian:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpMedian"; }
};
class OpNormsdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpNormsdist"; }
};
class OpNorminv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
     virtual std::string BinFuncName() const override { return "OpNorminv"; }
};
class OpNormsinv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
     virtual std::string BinFuncName() const override { return "OpNormsinv"; }
};
class OpPhi:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPhi"; }
};
class OpKurt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Kurt"; }
};
class OpCovar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Covar"; }
};

class OpPermut:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPermut"; }
};
class OpPermutationA:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpPermutationA";}
};

class OpConfidence: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "Confidence"; }
};
class OpIntercept: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Intercept"; }
};
class OpLogInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "LogInv"; }
};
class OpCritBinom: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "CritBinom"; }
};
class OpForecast: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Forecast"; }
};
class OpLogNormDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "LogNormdist"; }
};
class OpGammaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) override;
    virtual std::string BinFuncName() const override { return "GammaDist"; }
};
class OpHypGeomDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpHypGeomDist"; }
};
class OpChiDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpChiDist"; }
};
class OpBinomdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpBinomdist"; }
};
class OpChiSqDist: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ChiSqDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpChiSqInv: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "ChiSqInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};
class OpChiInv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpChiInv"; }
};
class OpPoisson:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpPoisson"; }
};

class OpGammaInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "GammaInv"; }
};
class OpFInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "FInv"; }
};
class OpFTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "FTest"; }
};
class OpDevSq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "DevSq"; }
};
class OpB: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "B"; }
};
class OpBetaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) override;
    virtual std::string BinFuncName() const override { return "BetaDist"; }
};
class OpBetainv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "OpBetainv"; }
};
class OpMinA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpMinA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpCountA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpCountA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpMaxA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpMaxA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpVarA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpVarA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpVarPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpVarPA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpStDevPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpStDevPA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpAverageA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpAverageA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpStDevA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "OpStDevA"; }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};
class OpAveDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "AveDev"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
