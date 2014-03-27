/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OP_FINANCIAL_HXX
#define SC_OPENCL_OP_FINANCIAL_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class RRI: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual bool takeString() const SAL_OVERRIDE { return false; }
    virtual bool takeNumeric() const SAL_OVERRIDE { return true; }
};

class OpRRI:public RRI
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "RRI"; }
};

class OpNominal: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "NOMINAL_ADD"; }
};

class OpDollarde:public Normal
{
public:
        virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

        virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Dollarde"; }

};

class OpDollarfr:public Normal
{
public:
        virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

        virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Dollarfr"; }

};

class OpDISC: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "DISC"; }
};

class OpINTRATE: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "INTRATE"; }
};

class OpFV: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE {
        return "FV"; }
};

class OpIPMT: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE {
        return "IPMT"; }
};

class OpISPMT: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "ISPMT"; }
};

class OpDuration: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Duration"; }
};

class OpDuration_ADD: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
                                     std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE {
        return "Duration_ADD"; }
};
class OpMDuration: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE {return "MDuration"; }
};

class Fvschedule: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
};

class Cumipmt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class IRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
};

class OpIRR: public IRR
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "IRR"; }
};

class XNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
};

class PriceMat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};
class OpSYD: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

     virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SYD"; }
};

class MIRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
};

class OpEffective:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Effect_Add"; }
};

class OpCumipmt: public Cumipmt
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Cumipmt"; }
};

class OpXNPV: public XNPV
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "XNPV"; }

};

class OpTbilleq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "fTbilleq"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpCumprinc: public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "cumprinc"; }
};

class OpAccrintm: public Normal
{
 public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Accrintm"; }
};
class OpAccrint: public Normal
{
 public:
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Accrint"; }
};

class OpYield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Yield"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};


class OpSLN: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "SLN"; }
};

class OpFvschedule: public Fvschedule
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Fvschedule"; }
};

class OpYieldmat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Yieldmat"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "PMT"; }
};
class OpNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "NPV"; }
};

class OpPrice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Price"; }
};

class OpNper: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "NPER"; }
};
class OpOddlprice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Oddlprice"; }
};
class OpOddlyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Oddlyield"; }
};
class OpPriceDisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "PriceDisc"; }
};
class OpPPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "PPMT"; }
};

class OpCoupdaybs:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coupdaybs"; }

};

class OpCoupdays:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coupdays";}

};

class OpCoupdaysnc:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coupdaysnc"; }

};

class OpCouppcd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Couppcd"; }

};

class OpCoupncd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coupncd"; }

};

class OpCoupnum:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Coupnum"; }

};
class OpDDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

     virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "DDB"; }
};
class OpDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

     virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "DB"; }
};
class OpAmordegrc:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Amordegrc"; }
};
class OpAmorlinc:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Amorlinc"; }
};

class OpReceived:public Normal
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Received"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpYielddisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Yielddisc"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpTbillprice: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "fTbillprice"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpPriceMat:public PriceMat
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "PriceMat"; }
};

class RATE: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};
class OpIntrate: public RATE {
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "rate"; }
};

class OpTbillyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "fTbillyield"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpMIRR: public MIRR
{
public:
    virtual std::string GetBottom(void) SAL_OVERRIDE { return "0"; }
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "MIRR"; }
};

class OpPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
                const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "PV"; }
};

class OpVDB: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "VDB"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& ) SAL_OVERRIDE;
};

class OpXirr: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;

    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Xirr"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
