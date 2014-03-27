/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OP_STATISTICAL_HXX
#define SC_OPENCL_OP_STATISTICAL_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class OpStandard: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Standard"; }
};
class OpExponDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ExponDist"; }
};
class OpVar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Var"; }
};
class OpSTEYX: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "STEYX"; }
};
class OpVarP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "VarP"; }
};
class OpZTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ZTest"; }
};
class OpStDevP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "StDevP"; }
};

class OpStDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "StDev"; }
};
class OpSkewp: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Skewp"; }
};
class OpSlope: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Slope"; }
};
class OpWeibull: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Weibull"; }
};
class OpFdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Fdist"; }
};
class OpTDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "TDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpTInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "TInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpTTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "TTest"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
};
class OpSkew: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Skew"; }
};
class OpFisher: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Fisher"; }
};

class OpFisherInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "FisherInv"; }
};

class OpGamma: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Gamma"; }
};

class OpCorrel: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Correl"; }
};

class OpNegbinomdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpNegbinomdist"; }
};

class OpPearson: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpPearson"; }
};

class OpGammaLn: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "GammaLn"; }
};

class OpGauss: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
       virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Gauss"; }
};

class OpGeoMean: public CheckVariables
{
public:
    OpGeoMean(void): CheckVariables(), mNeedReductionKernel(true) {}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "GeoMean"; }
    bool NeedReductionKernel(void) const { return mNeedReductionKernel; }
protected:
    bool mNeedReductionKernel;
};

class OpHarMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "HarMean"; }
};

class OpRsq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpRsq"; }
};
class OpNormdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpNormdist"; }
};
class OpMedian:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpMedian"; }
};
class OpNormsdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpNormsdist"; }
};
class OpNorminv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
     virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpNorminv"; }
};
class OpNormsinv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
     virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpNormsinv"; }
};
class OpPhi:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpPhi"; }
};
class OpKurt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Kurt"; }
};
class OpCovar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Covar"; }
};

class OpVariationen:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpVariationen"; }
};
class OpVariationen2:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpVariationen2";}
};

class OpConfidence: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Confidence"; }
};
class OpIntercept: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Intercept"; }
};
class OpLogInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "LogInv"; }
};
class OpCritBinom: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "CritBinom"; }
};
class OpForecast: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Forecast"; }
};
class OpLogNormDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "LogNormdist"; }
};
class OpGammaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "GammaDist"; }
};
class OpHypGeomDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpHypGeomDist"; }
};
class OpChiDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpChiDist"; }
};
class OpBinomdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpBinomdist"; }
};
class OpChiSqDist: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ChiSqDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpChiSqInv: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ChiSqInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};
class OpChiInv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpChiInv"; }
};
class OpPoisson:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpPoisson"; }
};

class OpGammaInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "GammaInv"; }
};
class OpFInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "FInv"; }
};
class OpFTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "FTest"; }
};
class OpDevSq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "DevSq"; }
};
class OpB: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "B"; }
};
class OpBetaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "BetaDist"; }
};
class OpBetainv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpBetainv"; }
};
class OpMinA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpMinA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpCountA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpCountA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpMaxA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpMaxA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpVarA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpVarA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpVarPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpVarPA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpStDevPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpStDevPA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpAverageA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpAverageA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpStDevA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "OpStDevA"; }
    virtual bool takeString() const SAL_OVERRIDE { return true; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};
class OpAveDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "AveDev"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
