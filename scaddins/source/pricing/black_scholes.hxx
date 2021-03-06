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


#pragma once

// options prices and greeks in the Black-Scholes model
// also known as TV (theoretical value)

namespace sca {
namespace pricing {

namespace bs {

namespace types {
enum Greeks {
    Value    = 0,
    Delta    = 1,        // d/dS
    Gamma    = 2,        // d^2/dS^2
    Theta    = 3,        // d/dt
    Vega     = 4,        // d/dsigma
    Volga    = 5,        // d^2/dsigma^2
    Vanna    = 6,        // d^2/dsigma dS
    Rho_d    = 7,        // d/dr_d
    Rho_f    = 8         // d/dr_f
};

enum PutCall {
    Call     = 1,
    Put      = -1
};

enum KOType {
    Regular = 0,
    Reverse = 1
};

enum BarrierKIO {
    KnockIn = -1,
    KnockOut = 1
};

// barrier observed continuously or just at maturity (truncated payoff)
enum BarrierActive {
    Continuous = 0,
    Maturity   = 1
};

enum ForDom {
    Domestic = 0,
    Foreign  = 1
};

} // namespace types


// binary option cash (domestic)
//   call - pays 1 if S_T is above strike K
//   put  - pays 1 if S_T is below strike K
double bincash(double S, double vol, double rd, double rf,
               double tau, double K,
               types::PutCall pc, types::Greeks greeks);

// binary option asset (foreign)
//   call - pays S_T if S_T is above strike K
//   put  - pays S_T if S_T is below strike K
double binasset(double S, double vol, double rd, double rf,
                double tau, double K,
                types::PutCall pc, types::Greeks greeks);

// vanilla put/call option
//   call pays (S_T-K)^+
//   put  pays (K-S_T)^+
// this is the same as: +/- (binasset - K*bincash)
double putcall(double S, double vol, double rd, double rf,
               double tau, double K,
               types::PutCall putcall, types::Greeks greeks);


// truncated put/call option, single barrier
// need to specify whether it's down-and-out or up-and-out
// regular (keeps monotonicity): down-and-out for call, up-and-out for put
// reverse (destroys monoton):   up-and-out for call, down-and-out for put
//   call pays (S_T-K)^+
//   put  pays (K-S_T)^+
double putcalltrunc(double S, double vol, double rd, double rf,
                    double tau, double K, double B,
                    types::PutCall pc, types::KOType kotype,
                    types::Greeks greeks);


// wrapper function for put/call option which combines
// double/single/no truncation barrier
// B1<=0 - assume no lower barrier
// B2<=0 - assume no upper barrier
double putcalltrunc(double S, double vol, double rd, double rf,
                    double tau, double K, double B1, double B2,
                    types::PutCall pc, types::Greeks greek);

// barrier
// touch/no-touch options (cash/asset or nothing payoff profile)
double touch(double S, double vol, double rd, double rf,
             double tau, double B1, double B2, types::ForDom fd,
             types::BarrierKIO kio, types::BarrierActive bcont,
             types::Greeks greek);

// barrier
// barrier option  (put/call payoff profile)
double barrier(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               double rebate,
               types::PutCall pc, types::BarrierKIO kio,
               types::BarrierActive bcont,
               types::Greeks greek);


// probability of hitting a barrier
double prob_hit(double S, double vol, double mu,
                double tau, double B1, double B2);


// probability of being in-the-money, ie payoff is greater zero,
// assuming payoff(S_T) > 0 iff S_T in [B1, B2]
double prob_in_money(double S, double vol, double mu,
                     double tau, double B1, double B2);
double prob_in_money(double S, double vol, double mu,
                     double tau, double K, double B1, double B2,
                     types::PutCall pc);


} // namespace bs

} // namespace pricing
} // namespace sca


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
