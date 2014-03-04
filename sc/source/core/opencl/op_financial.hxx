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
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual bool takeString() const { return false; }
    virtual bool takeNumeric() const { return true; }
};

class OpRRI:public RRI
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "RRI"; }
};

class OpNominal: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "NOMINAL_ADD"; }
};

class OpDollarde:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);

        virtual std::string BinFuncName(void) const { return "Dollarde"; }

};

class OpDollarfr:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);

        virtual std::string BinFuncName(void) const { return "Dollarfr"; }

};

class OpDISC: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );

    virtual std::string BinFuncName(void) const { return "DISC"; }
};

class OpINTRATE: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );

    virtual std::string BinFuncName(void) const { return "INTRATE"; }
};

class OpFV: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const {
        return "FV"; }
};

class OpIPMT: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const {
        return "IPMT"; }
};

class OpISPMT: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "ISPMT"; }
};

class OpDuration: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);

    virtual std::string BinFuncName(void) const { return "Duration"; }
};

class OpDuration_ADD: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
                                     std::set<std::string>& );

    virtual std::string BinFuncName(void) const {
        return "Duration_ADD"; }
};
class OpMDuration: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );

    virtual std::string BinFuncName(void) const {return "MDuration"; }
};

class Fvschedule: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
};

class Cumipmt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class IRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
};

class OpIRR: public IRR
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "IRR"; }
};

class XNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
};

class PriceMat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};
class OpSYD: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

     virtual std::string BinFuncName(void) const { return "SYD"; }
};

class MIRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
};

class OpEffective:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "Effect_Add"; }
};

class OpCumipmt: public Cumipmt
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "Cumipmt"; }
};

class OpXNPV: public XNPV
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "XNPV"; }

};

class OpTbilleq: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "fTbilleq"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpCumprinc: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "cumprinc"; }
};

class OpAccrintm: public Normal
{
 public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Accrintm"; }
};
class OpAccrint: public Normal
{
 public:
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Accrint"; }
};

class OpYield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yield"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};


class OpSLN: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "SLN"; }
};

class OpFvschedule: public Fvschedule
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "Fvschedule"; }
};

class OpYieldmat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yieldmat"; }
     virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "PMT"; }
};
class OpNPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "NPV"; }
};

class OpPrice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Price"; }
};

class OpNper: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "NPER"; }
};
class OpOddlprice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Oddlprice"; }
};
class OpOddlyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,
        std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Oddlyield"; }
};
class OpPriceDisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>&,
        std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "PriceDisc"; }
};
class OpPPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "PPMT"; }
};

class OpCoupdaybs:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Coupdaybs"; }

};

class OpCoupdays:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Coupdays";}

};

class OpCoupdaysnc:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Coupdaysnc"; }

};

class OpCouppcd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
    virtual std::string BinFuncName(void) const { return "Couppcd"; }

};

class OpCoupncd:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "Coupncd"; }

};

class OpCoupnum:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "Coupnum"; }

};
class OpDDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

     virtual std::string BinFuncName(void) const { return "DDB"; }
};
class OpDB:public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

     virtual std::string BinFuncName(void) const { return "DB"; }
};
class OpAmordegrc:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "Amordegrc"; }
};
class OpAmorlinc:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream& ss,
        const std::string &sSymName, SubArguments& vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>&);
    virtual std::string BinFuncName(void) const { return "Amorlinc"; }
};

class OpReceived:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Received"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpYielddisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yielddisc"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpTbillprice: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "fTbillprice"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpPriceMat:public PriceMat
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "PriceMat"; }
};

class RATE: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};
class OpIntrate: public RATE {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "rate"; }
};

class OpTbillyield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "fTbillyield"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpMIRR: public MIRR
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "MIRR"; }
};

class OpPV: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
                const std::string &sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "PV"; }
};

class OpVDB: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "VDB"; }
    virtual void BinInlineFun(std::set<std::string>& ,std::set<std::string>& );
};

class OpXirr: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "Xirr"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
