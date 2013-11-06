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
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Standard"; }
};

class OpWeibull: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Weibull"; }
};

class OpFisher: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Fisher"; }
};

class OpFisherInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "FisherInv"; }
};

class OpGamma: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Gamma"; }
};

class OpCorrel: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Correl"; }
};

class OpNegbinomdist: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpNegbinomdist"; }
};

class OpPearson: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpPearson"; }
};

class OpGammaLn: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "GammaLn"; }
};

class OpGauss: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Gauss"; }
};

class OpGeoMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "GeoMean"; }
};

class OpHarMean: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "HarMean"; }
};

class OpRsq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpRsq"; }
};
class OpNormdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpNormdist"; }
};
class OpMedian:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpMedian"; }
};
class OpNormsdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpNormsdist"; }
};
class OpNorminv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
     virtual std::string BinFuncName(void) const { return "OpNorminv"; }
};
class OpNormsinv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
     virtual std::string BinFuncName(void) const { return "OpNormsinv"; }
};
class OpPhi:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpPhi"; }
};
class OpKurt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Kurt"; }
};
class OpVariationen:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVariationen"; }
};
class OpVariationen2:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "OpVariationen2";}
};

class OpConfidence: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const { return "Confidence"; }
};
class OpIntercept: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Intercept"; }
};
class OpLogInv: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "LogInv"; }
};
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
