/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (C) 2012 Tino Kluge <tino.kluge@hrz.tu-chemnitz.de>
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <rtl/math.hxx>
#include "black_scholes.hxx"

// options prices and greeks in the Black-Scholes model
// also known as TV (theoretical value)
//
// the code is structured as follows:
//
// (1) basic assets
//   - cash-or-nothing option:  bincash()
//   - asset-or-nothing option: binasset()
//
// (2) derived basic assets, can all be priced based on (1)
//   - vanilla put/call:  putcall() = +/- ( binasset() - K*bincash() )
//   - truncated put/call (barriers active at maturity only)
//
// (3) write a wrapper function to include all vanilla pricers
//   - this is so we don't duplicate code when pricing barriers
//     as this is derived from vanillas
//
// (4) single barrier options (knock-out), priced based on truncated vanillas
//   - it follows from the reflection principle that the price W(S) of a
//     single barrier option is given by
//        W(S) = V(S) - (B/S)^a V(B^2/S), a = 2(rd-rf)/vol^2 - 1
//     where V(S) is the price of the corresponding truncated vanilla
//     option
//   - to reduce code duplication and in anticipation of double barrier
//     options we write the following function
//        barrier_term(S,c) = V(c*S) - (B/S)^a V(c*B^2/S)
//
//  (5) double barrier options (knock-out)
//   - value is an infinite sum over option prices of the corresponding
//     truncated vanillas (truncated at both barriers):
//
//   W(S)=sum (B2/B1)^(i*a) (V(S(B2/B1)^(2i)) - (B1/S)^a V(B1^2/S (B2/B1)^(2i))
//
//  (6) write routines for put/call barriers and touch options which
//     mainly call the general double barrier pricer
//     the main routines are touch() and barrier()
//     both can price in/out barriers, double/single barriers as well as
//     vanillas
//
//
// the framework allows any barriers to be priced as long as we define
// the value/greek functions for the corresponding truncated vanilla
// and wrap them into internal::vanilla() and internal::vanilla_trunc()
//
// disadvantage of that approach is that due to the rules of
// differentiations the formulas for greeks become long and possible
// simplifications in the formulas won't be made
//
// other code inefficiency due to multiplication with pm (+/- 1)
//   cvtsi2sd: int-->double, 6/3 cycles
//   mulsd: double-double multiplication, 5/1 cycles
//   with -O3, however, it compiles 2 versions with pm=1, and pm=-1
//   which are efficient
//   note this is tiny anyway as compared to exp/log (100 cycles),
//   pow (200 cycles), erf (70 cycles)
//
// this code is not tested for numerical instability, ie overruns,
// underruns, accuracy, etc


namespace sca {
namespace pricing {

namespace bs {


// helper functions
// ----------------
inline double sqr(double x) {
    return x*x;
}
// normal density (see also ScInterpreter::phi)
inline double dnorm(double x) {
    //return (1.0/sqrt(2.0*M_PI))*exp(-0.5*x*x); // windows may not have M_PI
    return 0.39894228040143268*exp(-0.5*x*x);
}
// cumulative normal distribution (see also ScInterpreter::integralPhi)
inline double pnorm(double x) {
    //return 0.5*(erf(sqrt(0.5)*x)+1.0);       // windows may not have erf
    return 0.5 * ::rtl::math::erfc(-x * 0.7071067811865475);
}



// binary option cash (domestic)
//   call - pays 1 if S_T is above strike K
//   put  - pays 1 if S_T is below strike K
double bincash(double S, double vol, double rd, double rf,
               double tau, double K,
               types::PutCall pc, types::Greeks greeks) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double   val=0.0;

    if(tau<=0.0) {
        // special case tau=0 (expiry)
        switch(greeks) {
        case types::Value:
            if( (pc==types::Call && S>=K) || (pc==types::Put && S<=K) ) {
                val = 1.0;
            } else {
                val = 0.0;
            }
            break;
        default:
            val = 0.0;
        }
    } else if(K==0.0) {
        // special case with zero strike
        if(pc==types::Put) {
            // up-and-out (put) with K=0
            val=0.0;
        } else {
            // down-and-out (call) with K=0 (zero coupon bond)
            switch(greeks) {
            case types::Value:
                val = 1.0;
                break;
            case types::Theta:
                val = rd;
                break;
            case types::Rho_d:
                val = -tau;
                break;
            default:
                val = 0.0;
            }
        }
    } else {
        // standard case with K>0, tau>0
        double   d1 = ( log(S/K)+(rd-rf+0.5*vol*vol)*tau ) / (vol*sqrt(tau));
        double   d2 = d1 - vol*sqrt(tau);
        int      pm    = (pc==types::Call) ? 1 : -1;

        switch(greeks) {
        case types::Value:
            val = pnorm(pm*d2);
            break;
        case types::Delta:
            val = pm*dnorm(d2)/(S*vol*sqrt(tau));
            break;
        case types::Gamma:
            val = -pm*dnorm(d2)*d1/(sqr(S*vol)*tau);
            break;
        case types::Theta:
            val = rd*pnorm(pm*d2)
                  + pm*dnorm(d2)*(log(S/K)/(vol*sqrt(tau))-0.5*d2)/tau;
            break;
        case types::Vega:
            val = -pm*dnorm(d2)*d1/vol;
            break;
        case types::Volga:
            val = pm*dnorm(d2)/(vol*vol)*(-d1*d1*d2+d1+d2);
            break;
        case types::Vanna:
            val = pm*dnorm(d2)/(S*vol*vol*sqrt(tau))*(d1*d2-1.0);
            break;
        case types::Rho_d:
            val = -tau*pnorm(pm*d2) + pm*dnorm(d2)*sqrt(tau)/vol;
            break;
        case types::Rho_f:
            val = -pm*dnorm(d2)*sqrt(tau)/vol;
            break;
        default:
            printf("bincash: greek %d not implemented\n", greeks );
            abort();
        }
    }
    return exp(-rd*tau)*val;
}



// binary option asset (foreign)
//   call - pays S_T if S_T is above strike K
//   put  - pays S_T if S_T is below strike K
double binasset(double S, double vol, double rd, double rf,
                double tau, double K,
                types::PutCall pc, types::Greeks greeks) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double   val=0.0;
    if(tau<=0.0) {
        // special case tau=0 (expiry)
        switch(greeks) {
        case types::Value:
            if( (pc==types::Call && S>=K) || (pc==types::Put && S<=K) ) {
                val = S;
            } else {
                val = 0.0;
            }
            break;
        case types::Delta:
            if( (pc==types::Call && S>=K) || (pc==types::Put && S<=K) ) {
                val = 1.0;
            } else {
                val = 0.0;
            }
            break;
        default:
            val = 0.0;
        }
    } else if(K==0.0) {
        // special case with zero strike (forward with zero strike)
        if(pc==types::Put) {
            // up-and-out (put) with K=0
            val = 0.0;
        } else {
            // down-and-out (call) with K=0 (type of forward)
            switch(greeks) {
            case types::Value:
                val = S;
                break;
            case types::Delta:
                val = 1.0;
                break;
            case types::Theta:
                val = rf*S;
                break;
            case types::Rho_f:
                val = -tau*S;
                break;
            default:
                val = 0.0;
            }
        }
    } else {
        // normal case
        double   d1 = ( log(S/K)+(rd-rf+0.5*vol*vol)*tau ) / (vol*sqrt(tau));
        double   d2 = d1 - vol*sqrt(tau);
        int      pm    = (pc==types::Call) ? 1 : -1;

        switch(greeks) {
        case types::Value:
            val = S*pnorm(pm*d1);
            break;
        case types::Delta:
            val = pnorm(pm*d1) + pm*dnorm(d1)/(vol*sqrt(tau));
            break;
        case types::Gamma:
            val = -pm*dnorm(d1)*d2/(S*sqr(vol)*tau);
            break;
        case types::Theta:
            val = rf*S*pnorm(pm*d1)
                  + pm*S*dnorm(d1)*(log(S/K)/(vol*sqrt(tau))-0.5*d1)/tau;
            break;
        case types::Vega:
            val = -pm*S*dnorm(d1)*d2/vol;
            break;
        case types::Volga:
            val = pm*S*dnorm(d1)/(vol*vol)*(-d1*d2*d2+d1+d2);
            break;
        case types::Vanna:
            val = pm*dnorm(d1)/(vol*vol*sqrt(tau))*(d2*d2-1.0);
            break;
        case types::Rho_d:
            val = pm*S*dnorm(d1)*sqrt(tau)/vol;
            break;
        case types::Rho_f:
            val = -tau*S*pnorm(pm*d1) - pm*S*dnorm(d1)*sqrt(tau)/vol;
            break;
        default:
            printf("binasset: greek %d not implemented\n", greeks );
            abort();
        }
    }
    return exp(-rf*tau)*val;
}

// just for convenience we can combine bincash and binasset into
// one function binary
// using bincash()  if fd==types::Domestic
// using binasset() if fd==types::Foreign
double binary(double S, double vol, double rd, double rf,
              double tau, double K,
              types::PutCall pc, types::ForDom fd,
              types::Greeks greek) {
    double val=0.0;
    switch(fd) {
    case types::Domestic:
        val = bincash(S,vol,rd,rf,tau,K,pc,greek);
        break;
    case types::Foreign:
        val = binasset(S,vol,rd,rf,tau,K,pc,greek);
        break;
    default:
        // never get here
        assert(false);
    }
    return val;
}

// further wrapper to combine single/double barrier binary options
// into one function
// B1<=0 - it is assumed lower barrier not set
// B2<=0 - it is assumed upper barrier not set
double binary(double S, double vol, double rd, double rf,
              double tau, double B1, double B2,
              types::ForDom fd, types::Greeks greek) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    double val=0.0;

    if(B1<=0.0 && B2<=0.0) {
        // no barriers set, payoff 1.0 (domestic) or S_T (foreign)
        val = binary(S,vol,rd,rf,tau,0.0,types::Call,fd,greek);
    } else if(B1<=0.0 && B2>0.0) {
        // upper barrier (put)
        val = binary(S,vol,rd,rf,tau,B2,types::Put,fd,greek);
    } else if(B1>0.0 && B2<=0.0) {
        // lower barrier (call)
        val = binary(S,vol,rd,rf,tau,B1,types::Call,fd,greek);
    } else if(B1>0.0 && B2>0.0) {
        // double barrier
        if(B2<=B1) {
            val = 0.0;
        } else {
            val = binary(S,vol,rd,rf,tau,B2,types::Put,fd,greek)
                  - binary(S,vol,rd,rf,tau,B1,types::Put,fd,greek);
        }
    } else {
        // never get here
        assert(false);
    }

    return val;
}



// vanilla put/call option
//   call pays (S_T-K)^+
//   put  pays (K-S_T)^+
// this is the same as: +/- (binasset - K*bincash)
double putcall(double S, double vol, double rd, double rf,
               double tau, double K,
               types::PutCall putcall, types::Greeks greeks) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double   val = 0.0;
    int      pm  = (putcall==types::Call) ? 1 : -1;

    if(K==0 || tau==0.0) {
        // special cases, simply refer to binasset() and bincash()
        val = pm * ( binasset(S,vol,rd,rf,tau,K,putcall,greeks)
                     - K*bincash(S,vol,rd,rf,tau,K,putcall,greeks) );
    } else {
        // general case
        // we could just use pm*(binasset-K*bincash), however
        // since the formula for delta and gamma simplify we write them
        // down here
        double   d1 = ( log(S/K)+(rd-rf+0.5*vol*vol)*tau ) / (vol*sqrt(tau));
        double   d2 = d1 - vol*sqrt(tau);

        switch(greeks) {
        case types::Value:
            val = pm * ( exp(-rf*tau)*S*pnorm(pm*d1)-exp(-rd*tau)*K*pnorm(pm*d2) );
            break;
        case types::Delta:
            val = pm*exp(-rf*tau)*pnorm(pm*d1);
            break;
        case types::Gamma:
            val = exp(-rf*tau)*dnorm(d1)/(S*vol*sqrt(tau));
            break;
        default:
            // too lazy for the other greeks, so simply refer to binasset/bincash
            val = pm * ( binasset(S,vol,rd,rf,tau,K,putcall,greeks)
                         - K*bincash(S,vol,rd,rf,tau,K,putcall,greeks) );
        }
    }
    return val;
}

// truncated put/call option, single barrier
// need to specify whether it's down-and-out or up-and-out
// regular (keeps monotonicity): down-and-out for call, up-and-out for put
// reverse (destroys monoton):   up-and-out for call, down-and-out for put
//   call pays (S_T-K)^+
//   put  pays (K-S_T)^+
double putcalltrunc(double S, double vol, double rd, double rf,
                    double tau, double K, double B,
                    types::PutCall pc, types::KOType kotype,
                    types::Greeks greeks) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);
    assert(B>=0.0);

    int      pm  = (pc==types::Call) ? 1 : -1;
    double   val = 0.0;

    switch(kotype) {
    case types::Regular:
        if( (pc==types::Call && B<=K) || (pc==types::Put && B>=K) ) {
            // option degenerates to standard plain vanilla call/put
            val = putcall(S,vol,rd,rf,tau,K,pc,greeks);
        } else {
            // normal case with truncation
            val = pm * ( binasset(S,vol,rd,rf,tau,B,pc,greeks)
                         - K*bincash(S,vol,rd,rf,tau,B,pc,greeks) );
        }
        break;
    case types::Reverse:
        if( (pc==types::Call && B<=K) || (pc==types::Put && B>=K) ) {
            // option degenerates to zero payoff
            val = 0.0;
        } else {
            // normal case with truncation
            val = binasset(S,vol,rd,rf,tau,K,types::Call,greeks)
                  - binasset(S,vol,rd,rf,tau,B,types::Call,greeks)
                  - K * ( bincash(S,vol,rd,rf,tau,K,types::Call,greeks)
                          - bincash(S,vol,rd,rf,tau,B,types::Call,greeks) );
        }
        break;
    default:
        assert(false);
    }
    return val;
}

// wrapper function for put/call option which combines
// double/single/no truncation barrier
// B1<=0 - assume no lower barrier
// B2<=0 - assume no upper barrier
double putcalltrunc(double S, double vol, double rd, double rf,
                    double tau, double K, double B1, double B2,
                    types::PutCall pc, types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double val=0.0;

    if(B1<=0.0 && B2<=0.0) {
        // no barriers set, plain vanilla
        val = putcall(S,vol,rd,rf,tau,K,pc,greek);
    } else if(B1<=0.0 && B2>0.0) {
        // upper barrier: reverse barrier for call, regular barrier for put
        if(pc==types::Call) {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B2,pc,types::Reverse,greek);
        } else {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B2,pc,types::Regular,greek);
        }
    } else if(B1>0.0 && B2<=0.0) {
        // lower barrier: regular barrier for call, reverse barrier for put
        if(pc==types::Call) {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B1,pc,types::Regular,greek);
        } else {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B1,pc,types::Reverse,greek);
        }
    } else if(B1>0.0 && B2>0.0) {
        // double barrier
        if(B2<=B1) {
            val = 0.0;
        } else {
            int   pm  = (pc==types::Call) ? 1 : -1;
            val = pm * (
                      putcalltrunc(S,vol,rd,rf,tau,K,B1,pc,types::Regular,greek)
                      - putcalltrunc(S,vol,rd,rf,tau,K,B2,pc,types::Regular,greek)
                  );
        }
    } else {
        // never get here
        assert(false);
    }
    return val;
}

namespace internal {

// wrapper function for all non-path dependent options
// this is only an internal function, used to avoid code duplication when
// going to path-dependent barrier options,
// K<0  - assume binary option
// K>=0 - assume put/call option
double vanilla(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               types::PutCall pc, types::ForDom fd,
               types::Greeks greek) {
    double val = 0.0;
    if(K<0.0) {
        // binary option if K<0
        val = binary(S,vol,rd,rf,tau,B1,B2,fd,greek);
    } else {
        val = putcall(S,vol,rd,rf,tau,K,pc,greek);
    }
    return val;
}
double vanilla_trunc(double S, double vol, double rd, double rf,
                     double tau, double K, double B1, double B2,
                     types::PutCall pc, types::ForDom fd,
                     types::Greeks greek) {
    double val = 0.0;
    if(K<0.0) {
        // binary option if K<0
        // truncated is actually the same as the vanilla binary
        val = binary(S,vol,rd,rf,tau,B1,B2,fd,greek);
    } else {
        val = putcalltrunc(S,vol,rd,rf,tau,K,B1,B2,pc,greek);
    }
    return val;
}

} // namespace internal


// ---------------------------------------------------------------------
// path dependent options
// ---------------------------------------------------------------------

namespace internal {

// helper term for any type of options with continuously monitored barriers,
// internal, should not be called from outside
// calculates value and greeks based on
//   V(S) = V1(sc*S) - (B/S)^a V1(sc*B^2/S)
//   (a=2 mu/vol^2, mu drift in logspace, ie. mu=(rd-rf-1/2vol^2))
// with sc=1 and V1() being the price of the respective truncated
// vanilla option, V() would be the price of the respective barrier
// option if only one barrier is present
double barrier_term(double S, double vol, double rd, double rf,
                    double tau, double K, double B1, double B2, double sc,
                    types::PutCall pc, types::ForDom fd,
                    types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    // V(S) = V1(sc*S) - (B/S)^a V1(sc*B^2/S)
    double   val = 0.0;
    double   B   = (B1>0.0) ? B1 : B2;
    double   a   = 2.0*(rd-rf)/(vol*vol)-1.0;    // helper variable
    double   b   = 4.0*(rd-rf)/(vol*vol*vol);    // helper variable -da/dvol
    double   c   = 12.0*(rd-rf)/(vol*vol*vol*vol); // helper -db/dvol
    switch(greek) {
    case types::Value:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a)*
              vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
        break;
    case types::Delta:
        val = sc*vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              + pow(B/S,a) * (
                  a/S*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + sqr(B/S)*sc*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              );
        break;
    case types::Gamma:
        val = sc*sc*vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  a*(a+1.0)/(S*S)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + (2.0*a+2.0)*B*B/(S*S*S)*sc*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Delta)
                  + sqr(sqr(B/S))*sc*sc*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Gamma)
              );
        break;
    case types::Theta:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a)*
              vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
        break;
    case types::Vega:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  - b*log(B/S)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + 1.0*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              );
        break;
    case types::Volga:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  log(B/S)*(b*b*log(B/S)+c)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  - 2.0*b*log(B/S)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Vega)
                  + 1.0*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Volga)
              );
        break;
    case types::Vanna:
        val = sc*vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  b/S*(log(B/S)*a+1.0)*
                  vanilla_trunc(B*B/S*sc,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + b*log(B/S)*sqr(B/S)*sc*
                  vanilla_trunc(B*B/S*sc,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Delta)
                  - a/S*
                  vanilla_trunc(B*B/S*sc,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Vega)
                  - sqr(B/S)*sc*
                  vanilla_trunc(B*B/S*sc,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Vanna)
              );
        break;
    case types::Rho_d:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  2.0*log(B/S)/(vol*vol)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + 1.0*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              );
        break;
    case types::Rho_f:
        val = vanilla_trunc(sc*S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - pow(B/S,a) * (
                  - 2.0*log(B/S)/(vol*vol)*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,types::Value)
                  + 1.0*
                  vanilla_trunc(sc*B*B/S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              );
        break;
    default:
        printf("barrier_term: greek %d not implemented\n", greek );
        abort();
    }
    return val;
}

// one term of the infinite sum for the valuation of double barriers
double barrier_double_term( double S, double vol, double rd, double rf,
                            double tau, double K, double B1, double B2,
                            double fac, double sc, int i,
                            types::PutCall pc, types::ForDom fd, types::Greeks greek) {

    double val = 0.0;
    double   b   = 4.0*i*(rd-rf)/(vol*vol*vol);    // helper variable -da/dvol
    double   c   = 12.0*i*(rd-rf)/(vol*vol*vol*vol); // helper -db/dvol
    switch(greek) {
    case types::Value:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek);
        break;
    case types::Delta:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek);
        break;
    case types::Gamma:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek);
        break;
    case types::Theta:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek);
        break;
    case types::Vega:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek)
              - b*log(B2/B1)*fac *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Value);
        break;
    case types::Volga:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek)
              - 2.0*b*log(B2/B1)*fac *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Vega)
              + log(B2/B1)*fac*(c+b*b*log(B2/B1)) *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Value);
        break;
    case types::Vanna:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek)
              - b*log(B2/B1)*fac *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Delta);
        break;
    case types::Rho_d:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek)
              + 2.0*i/(vol*vol)*log(B2/B1)*fac *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Value);
        break;
    case types::Rho_f:
        val = fac*barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,greek)
              - 2.0*i/(vol*vol)*log(B2/B1)*fac *
              barrier_term(S,vol,rd,rf,tau,K,B1,B2,sc,pc,fd,types::Value);
        break;
    default:
        printf("barrier_double_term: greek %d not implemented\n", greek );
        abort();
    }
    return val;
}

// general pricer for any type of options with continuously monitored barriers
// allows two, one or zero barriers, only knock-out style
// payoff profiles allowed based on vanilla_trunc()
double barrier_ko(double S, double vol, double rd, double rf,
                  double tau, double K, double B1, double B2,
                  types::PutCall pc, types::ForDom fd,
                  types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    double val = 0.0;

    if(B1<=0.0 && B2<=0.0) {
        // no barriers --> vanilla case
        val = vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if(B1>0.0 && B2<=0.0) {
        // lower barrier
        if(S<=B1) {
            val = 0.0;     // knocked out
        } else {
            val = barrier_term(S,vol,rd,rf,tau,K,B1,B2,1.0,pc,fd,greek);
        }
    } else if(B1<=0.0 && B2>0.0) {
        // upper barrier
        if(S>=B2) {
            val = 0.0;     // knocked out
        } else {
            val = barrier_term(S,vol,rd,rf,tau,K,B1,B2,1.0,pc,fd,greek);
        }
    } else if(B1>0.0 && B2>0.0) {
        // double barrier
        if(S<=B1 || S>=B2) {
            val = 0.0;     // knocked out (always true if wrong input B1>B2)
        } else {
            // more complex calculation as we have to evaluate an infinite
            // sum
            // to reduce very costly pow() calls we define some variables
            double a = 2.0*(rd-rf)/(vol*vol)-1.0;    // 2 (mu-1/2vol^2)/sigma^2
            double BB2=sqr(B2/B1);
            double BBa=pow(B2/B1,a);
            double BB2inv=1.0/BB2;
            double BBainv=1.0/BBa;
            double fac=1.0;
            double facinv=1.0;
            double sc=1.0;
            double scinv=1.0;

            // initial term i=0
            val=barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,fac,sc,0,pc,fd,greek);
            // infinite loop, 10 should be plenty, normal would be 2
            for(int i=1; i<10; i++) {
                fac*=BBa;
                facinv*=BBainv;
                sc*=BB2;
                scinv*=BB2inv;
                double add =
                    barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,fac,sc,i,pc,fd,greek) +
                    barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,facinv,scinv,-i,pc,fd,greek);
                val += add;
                //printf("%i: val=%e (add=%e)\n",i,val,add);
                if(fabs(add) <= 1e-12*fabs(val)) {
                    break;
                }
            }
            // not knocked-out double barrier end
        }
        // double barrier end
    } else {
        // no such barrier combination exists
        assert(false);
    }

    return val;
}

// knock-in style barrier
double barrier_ki(double S, double vol, double rd, double rf,
                  double tau, double K, double B1, double B2,
                  types::PutCall pc, types::ForDom fd,
                  types::Greeks greek) {
    return vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
           -barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
}

// general barrier
double barrier(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               types::PutCall pc, types::ForDom fd,
               types::BarrierKIO kio, types::BarrierActive bcont,
               types::Greeks greek) {

    double val = 0.0;
    if( kio==types::KnockOut && bcont==types::Maturity ) {
        // truncated vanilla option
        val = vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockOut && bcont==types::Continuous ) {
        // standard knock-out barrier
        val = barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Maturity ) {
        // inverse truncated vanilla
        val = vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Continuous ) {
        // standard knock-in barrier
        val = barrier_ki(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else {
        // never get here
        assert(false);
    }
    return val;
}

} // namespace internal




// touch/no-touch options (cash/asset or nothing payoff profile)
double touch(double S, double vol, double rd, double rf,
             double tau, double B1, double B2, types::ForDom fd,
             types::BarrierKIO kio, types::BarrierActive bcont,
             types::Greeks greek) {

    double K=-1.0;                      // dummy
    types::PutCall pc = types::Call;    // dummy
    double val = 0.0;
    if( kio==types::KnockOut && bcont==types::Maturity ) {
        // truncated vanilla option
        val = internal::vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockOut && bcont==types::Continuous ) {
        // standard knock-out barrier
        val = internal::barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Maturity ) {
        // inverse truncated vanilla
        val = internal::vanilla(S,vol,rd,rf,tau,K,-1.0,-1.0,pc,fd,greek)
              - internal::vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Continuous ) {
        // standard knock-in barrier
        val = internal::vanilla(S,vol,rd,rf,tau,K,-1.0,-1.0,pc,fd,greek)
              - internal::barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else {
        // never get here
        assert(false);
    }
    return val;
}

// barrier option  (put/call payoff profile)
double barrier(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               double rebate,
               types::PutCall pc, types::BarrierKIO kio,
               types::BarrierActive bcont,
               types::Greeks greek) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);
    types::ForDom fd = types::Domestic;
    double val=internal::barrier(S,vol,rd,rf,tau,K,B1,B2,pc,fd,kio,bcont,greek);
    if(rebate!=0.0) {
        // opposite of barrier knock-in/out type
        types::BarrierKIO kio2 = (kio==types::KnockIn) ? types::KnockOut
                                 : types::KnockIn;
        val += rebate*touch(S,vol,rd,rf,tau,B1,B2,fd,kio2,bcont,greek);
    }
    return val;
}



// probability of hitting a barrier
// this is almost the same as the price of a touch option (domestic)
// as it pays one if a barrier is hit; we only have to offset the
// discounting and we get the probability
double prob_hit(double S, double vol, double mu,
                double tau, double B1, double B2) {
    double rd=0.0;
    double rf=-mu;
    return 1.0 - touch(S,vol,rd,rf,tau,B1,B2,types::Domestic,types::KnockOut,
                       types::Continuous, types::Value);
}

// probability of being in-the-money, ie payoff is greater zero,
// assuming payoff(S_T) > 0 iff S_T in [B1, B2]
// this the same as the price of a cash or nothing option
// with no discounting
double prob_in_money(double S, double vol, double mu,
                     double tau, double B1, double B2) {
    assert(S>0.0);
    assert(vol>0.0);
    assert(tau>=0.0);
    double val = 0.0;
    if( B1<B2 || B1<=0.0 || B2<=0.0 ) {
        val = binary(S,vol,0.0,-mu,tau,B1,B2,types::Domestic,types::Value);
    }
    return val;
}
double prob_in_money(double S, double vol, double mu,
                     double tau, double K, double B1, double B2,
                     types::PutCall pc) {
    assert(S>0.0);
    assert(vol>0.0);
    assert(tau>=0.0);

    // if K<0 we assume a binary option is given
    if(K<0.0) {
        return prob_in_money(S,vol,mu,tau,B1,B2);
    }

    double val = 0.0;
    double BM1, BM2;     // range of in the money [BM1, BM2]
    // non-sense parameters with no positive payoff
    if( (B1>B2 && B1>0.0 && B2>0.0) ||
            (K>=B2 && B2>0.0 && pc==types::Call) ||
            (K<=B1 && pc==types::Put) ) {
        val = 0.0;
        // need to figure out between what barriers payoff is greater 0
    } else if(pc==types::Call) {
        BM1=std::max(B1, K);
        BM2=B2;
        val = prob_in_money(S,vol,mu,tau,BM1,BM2);
    } else if (pc==types::Put) {
        BM1=B1;
        BM2= (B2>0.0) ? std::min(B2,K) : K;
        val = prob_in_money(S,vol,mu,tau,BM1,BM2);
    } else {
        // don't get here
        assert(false);
    }
    return val;
}



} // namespace bs

} // namespace pricing
} // namespace sca


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
