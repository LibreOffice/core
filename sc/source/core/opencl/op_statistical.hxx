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
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Standard"; }
};
class OpExponDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "ExponDist"; }
};
class OpVar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Var"; }
};
class OpSTEYX: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "STEYX"; }
};
class OpVarP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "VarP"; }
};
class OpZTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "ZTest"; }
};
class OpStDevP: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "StDevP"; }
};

class OpStDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "StDev"; }
};
class OpSkewp: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Skewp"; }
};
class OpSlope: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Slope"; }
};
class OpWeibull: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Weibull"; }
};
class OpFdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "Fdist"; }
};
class OpTDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "TDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
};
class OpTInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "TInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
};
class OpTTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "TTest"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
};
class OpSkew: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Skew"; }
};
class OpFisher: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Fisher"; }
};

class OpFisherInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "FisherInv"; }
};

class OpGamma: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Gamma"; }
};

class OpCorrel: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Correl"; }
};

class OpNegbinomdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpNegbinomdist"; }
};

class OpPearson: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpPearson"; }
};

class OpGammaLn: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "GammaLn"; }
};

class OpGauss: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
       virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Gauss"; }
};

class OpGeoMean: public CheckVariables
{
public:
    OpGeoMean(void): CheckVariables(), mNeedReductionKernel(true) {}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "GeoMean"; }
    bool NeedReductionKernel(void) const { return mNeedReductionKernel; }
protected:
    bool mNeedReductionKernel;
};

class OpHarMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "HarMean"; }
};

class OpRsq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpRsq"; }
};
class OpNormdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpNormdist"; }
};
class OpMedian:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpMedian"; }
};
class OpNormsdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpNormsdist"; }
};
class OpNorminv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
     virtual std::string BinFuncName(void) const { return "OpNorminv"; }
};
class OpNormsinv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
     virtual std::string BinFuncName(void) const { return "OpNormsinv"; }
};
class OpPhi:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpPhi"; }
};
class OpKurt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Kurt"; }
};
class OpCovar: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual std::string BinFuncName(void) const { return "Covar"; }
};

class OpVariationen:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVariationen"; }
};
class OpVariationen2:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVariationen2";}
};

class OpConfidence: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const { return "Confidence"; }
};
class OpIntercept: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Intercept"; }
};
class OpLogInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "LogInv"; }
};
class OpCritBinom: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const { return "CritBinom"; }
};
class OpForecast: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Forecast"; }
};
class OpLogNormDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "LogNormdist"; }
};
class OpGammaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs);
    virtual std::string BinFuncName(void) const { return "GammaDist"; }
};
class OpHypGeomDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpHypGeomDist"; }
};
class OpChiDist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpChiDist"; }
};
class OpBinomdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpBinomdist"; }
};
class OpChiSqDist: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "ChiSqDist"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpChiSqInv: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "ChiSqInv"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};
class OpChiInv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpChiInv"; }
};
class OpPoisson:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpPoisson"; }
};

class OpGammaInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
);
    virtual std::string BinFuncName(void) const { return "GammaInv"; }
};
class OpFInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
);
    virtual std::string BinFuncName(void) const { return "FInv"; }
};
class OpFTest: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
);
    virtual std::string BinFuncName(void) const { return "FTest"; }
};
class OpDevSq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "DevSq"; }
};
class OpB: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
);
    virtual std::string BinFuncName(void) const { return "B"; }
};
class OpBetaDist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    void BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs
);
    virtual std::string BinFuncName(void) const { return "BetaDist"; }
};
class OpBetainv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "OpBetainv"; }
};
class OpMinA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpMinA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpCountA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpCountA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpMaxA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpMaxA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpVarA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVarA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpVarPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVarPA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpStDevPA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpStDevPA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpAverageA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpAverageA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpStDevA: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpStDevA"; }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};
class OpAveDev: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "AveDev"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
