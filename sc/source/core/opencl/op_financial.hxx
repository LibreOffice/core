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

class RRI: public SlidingFunctionBase, public OpBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
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
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "NOMINAL_ADD"; }
};

class OpDollarde:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);

        virtual std::string BinFuncName(void) const { return "Dollarde"; }

};

class OpDollarfr:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }

        virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);

        virtual std::string BinFuncName(void) const { return "Dollarfr"; }

};

class OpDISC: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "DISC"; }
};

class OpINTRATE: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "INTRATE"; }
};

class Fvschedule: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
};

class Cumipmt: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
};

class IRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
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
            const std::string sSymName, SubArguments &vSubArguments);
};

class PriceMat: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
};
class OpSYD: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);

     virtual std::string BinFuncName(void) const { return "SYD"; }
};

class MIRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
};

class OpEffective:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);

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
            const std::string sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "fTbilleq"; }
};

class OpCumprinc: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "cumprinc"; }
};

class OpAccrintm: public Normal
{
 public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Accrintm"; }
};

class OpYield: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yield"; }
};


class OpSLN: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
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
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yieldmat"; }
};

class OpPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "PMT"; }
};

class OpPPMT: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "PPMT"; }
};

class OpCoupdaybs:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Coupdaybs"; }

};

class OpCoupdays:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Coupdays";}

};

class OpCoupdaysnc:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Coupdaysnc"; }

};

class OpReceived:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Received"; }
};

class OpYielddisc: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Yielddisc"; }
};

class OpTbillprice: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "fTbillprice"; }
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
            const std::string sSymName, SubArguments &vSubArguments);
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
            const std::string sSymName, SubArguments &vSubArguments);

    virtual std::string BinFuncName(void) const { return "fTbillyield"; }
};

class OpMIRR: public MIRR
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "MIRR"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
