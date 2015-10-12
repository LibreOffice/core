/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OP_FINANCIAL_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OP_FINANCIAL_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class RRI: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

class OpRRI:public RRI
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "RRI"; }
};

class OpNominal: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "NOMINAL_ADD"; }
};

class OpDollarde:public Normal
{
public:
        virtual std::string GetBottom() override { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;

        virtual std::string BinFuncName() const override { return "Dollarde"; }

};

class OpDollarfr:public Normal
{
public:
        virtual std::string GetBottom() override { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;

        virtual std::string BinFuncName() const override { return "Dollarfr"; }

};

class OpDISC: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "DISC"; }
};

class OpINTRATE: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override { return "INTRATE"; }
};

class OpFV: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override {
        return "FV"; }
};

class OpIPMT: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override {
        return "IPMT"; }
};

class OpISPMT: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "ISPMT"; }
};

class OpDuration: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Duration"; }
};

class OpDuration_ADD: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
                                     std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override {
        return "Duration_ADD"; }
};
class OpMDuration: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;

    virtual std::string BinFuncName() const override {return "MDuration"; }
};

class Fvschedule: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
};

class Cumipmt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class IRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
};

class OpIRR: public IRR
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "IRR"; }
};

class XNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
};

class PriceMat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};
class OpSYD: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

     virtual std::string BinFuncName() const override { return "SYD"; }
};

class MIRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
};

class OpEffective:public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Effect_Add"; }
};

class OpCumipmt: public Cumipmt
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "Cumipmt"; }
};

class OpXNPV: public XNPV
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "XNPV"; }

};

class OpTbilleq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "fTbilleq"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpCumprinc: public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "cumprinc"; }
};

class OpAccrintm: public Normal
{
 public:
    virtual std::string GetBottom() override { return "0"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Accrintm"; }
};
class OpAccrint: public Normal
{
 public:
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Accrint"; }
};

class OpYield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Yield"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpSLN: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SLN"; }
};

class OpFvschedule: public Fvschedule
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "Fvschedule"; }
};

class OpYieldmat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Yieldmat"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "PMT"; }
};
class OpNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "NPV"; }
};

class OpPrice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Price"; }
};

class OpNper: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "NPER"; }
};
class OpOddlprice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Oddlprice"; }
};
class OpOddlyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Oddlyield"; }
};
class OpPriceDisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "PriceDisc"; }
};
class OpPPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "PPMT"; }
};

class OpCoupdaybs:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Coupdaybs"; }

};

class OpCoupdays:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Coupdays";}

};

class OpCoupdaysnc:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Coupdaysnc"; }

};

class OpCouppcd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
    virtual std::string BinFuncName() const override { return "Couppcd"; }

};

class OpCoupncd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Coupncd"; }

};

class OpCoupnum:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Coupnum"; }

};
class OpDDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

     virtual std::string BinFuncName() const override { return "DDB"; }
};
class OpDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

     virtual std::string BinFuncName() const override { return "DB"; }
};
class OpAmordegrc:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Amordegrc"; }
};
class OpAmorlinc:public Normal
{
public:
    virtual std::string GetBottom() override { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) override;
    virtual std::string BinFuncName() const override { return "Amorlinc"; }
};

class OpReceived:public Normal
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Received"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpYielddisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Yielddisc"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpTbillprice: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "fTbillprice"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpPriceMat:public PriceMat
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "PriceMat"; }
};

class RATE: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};
class OpIntrate: public RATE {
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "rate"; }
};

class OpTbillyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "fTbillyield"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpMIRR: public MIRR
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string BinFuncName() const override { return "MIRR"; }
};

class OpPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
                const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "PV"; }
};

class OpVDB: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "VDB"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) override;
};

class OpXirr: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;

    virtual std::string BinFuncName() const override { return "Xirr"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
