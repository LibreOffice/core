/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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



//

//



//



//



//









//



//

//





//
//



//



//







//




namespace sca {
namespace pricing {

namespace bs {




inline double sqr(double x) {
    return x*x;
}

inline double dnorm(double x) {
    
    return 0.39894228040143268*exp(-0.5*x*x);
}

inline double pnorm(double x) {
    
    return 0.5 * ::rtl::math::erfc(-x * 0.7071067811865475);
}






double bincash(double S, double vol, double rd, double rf,
               double tau, double K,
               types::PutCall pc, types::Greeks greeks) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double   val=0.0;

    if(tau<=0.0) {
        
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
        
        if(pc==types::Put) {
            
            val=0.0;
        } else {
            
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






double binasset(double S, double vol, double rd, double rf,
                double tau, double K,
                types::PutCall pc, types::Greeks greeks) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double   val=0.0;
    if(tau<=0.0) {
        
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
        
        if(pc==types::Put) {
            
            val = 0.0;
        } else {
            
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
        
        assert(false);
    }
    return val;
}





double binary(double S, double vol, double rd, double rf,
              double tau, double B1, double B2,
              types::ForDom fd, types::Greeks greek) {
    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    double val=0.0;

    if(B1<=0.0 && B2<=0.0) {
        
        val = binary(S,vol,rd,rf,tau,0.0,types::Call,fd,greek);
    } else if(B1<=0.0 && B2>0.0) {
        
        val = binary(S,vol,rd,rf,tau,B2,types::Put,fd,greek);
    } else if(B1>0.0 && B2<=0.0) {
        
        val = binary(S,vol,rd,rf,tau,B1,types::Call,fd,greek);
    } else if(B1>0.0 && B2>0.0) {
        
        if(B2<=B1) {
            val = 0.0;
        } else {
            val = binary(S,vol,rd,rf,tau,B2,types::Put,fd,greek)
                  - binary(S,vol,rd,rf,tau,B1,types::Put,fd,greek);
        }
    } else {
        
        assert(false);
    }

    return val;
}







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
        
        val = pm * ( binasset(S,vol,rd,rf,tau,K,putcall,greeks)
                     - K*bincash(S,vol,rd,rf,tau,K,putcall,greeks) );
    } else {
        
        
        
        
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
            
            val = pm * ( binasset(S,vol,rd,rf,tau,K,putcall,greeks)
                         - K*bincash(S,vol,rd,rf,tau,K,putcall,greeks) );
        }
    }
    return val;
}







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
            
            val = putcall(S,vol,rd,rf,tau,K,pc,greeks);
        } else {
            
            val = pm * ( binasset(S,vol,rd,rf,tau,B,pc,greeks)
                         - K*bincash(S,vol,rd,rf,tau,B,pc,greeks) );
        }
        break;
    case types::Reverse:
        if( (pc==types::Call && B<=K) || (pc==types::Put && B>=K) ) {
            
            val = 0.0;
        } else {
            
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





double putcalltrunc(double S, double vol, double rd, double rf,
                    double tau, double K, double B1, double B2,
                    types::PutCall pc, types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);
    assert(K>=0.0);

    double val=0.0;

    if(B1<=0.0 && B2<=0.0) {
        
        val = putcall(S,vol,rd,rf,tau,K,pc,greek);
    } else if(B1<=0.0 && B2>0.0) {
        
        if(pc==types::Call) {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B2,pc,types::Reverse,greek);
        } else {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B2,pc,types::Regular,greek);
        }
    } else if(B1>0.0 && B2<=0.0) {
        
        if(pc==types::Call) {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B1,pc,types::Regular,greek);
        } else {
            val = putcalltrunc(S,vol,rd,rf,tau,K,B1,pc,types::Reverse,greek);
        }
    } else if(B1>0.0 && B2>0.0) {
        
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
        
        assert(false);
    }
    return val;
}

namespace internal {






double vanilla(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               types::PutCall pc, types::ForDom fd,
               types::Greeks greek) {
    double val = 0.0;
    if(K<0.0) {
        
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
        
        
        val = binary(S,vol,rd,rf,tau,B1,B2,fd,greek);
    } else {
        val = putcalltrunc(S,vol,rd,rf,tau,K,B1,B2,pc,greek);
    }
    return val;
}

} 






namespace internal {









double barrier_term(double S, double vol, double rd, double rf,
                    double tau, double K, double B1, double B2, double sc,
                    types::PutCall pc, types::ForDom fd,
                    types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    
    double   val = 0.0;
    double   B   = (B1>0.0) ? B1 : B2;
    double   a   = 2.0*(rd-rf)/(vol*vol)-1.0;    
    double   b   = 4.0*(rd-rf)/(vol*vol*vol);    
    double   c   = 12.0*(rd-rf)/(vol*vol*vol*vol); 
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


double barrier_double_term( double S, double vol, double rd, double rf,
                            double tau, double K, double B1, double B2,
                            double fac, double sc, int i,
                            types::PutCall pc, types::ForDom fd, types::Greeks greek) {

    double val = 0.0;
    double   b   = 4.0*i*(rd-rf)/(vol*vol*vol);    
    double   c   = 12.0*i*(rd-rf)/(vol*vol*vol*vol); 
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




double barrier_ko(double S, double vol, double rd, double rf,
                  double tau, double K, double B1, double B2,
                  types::PutCall pc, types::ForDom fd,
                  types::Greeks greek) {

    assert(tau>=0.0);
    assert(S>0.0);
    assert(vol>0.0);

    double val = 0.0;

    if(B1<=0.0 && B2<=0.0) {
        
        val = vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if(B1>0.0 && B2<=0.0) {
        
        if(S<=B1) {
            val = 0.0;     
        } else {
            val = barrier_term(S,vol,rd,rf,tau,K,B1,B2,1.0,pc,fd,greek);
        }
    } else if(B1<=0.0 && B2>0.0) {
        
        if(S>=B2) {
            val = 0.0;     
        } else {
            val = barrier_term(S,vol,rd,rf,tau,K,B1,B2,1.0,pc,fd,greek);
        }
    } else if(B1>0.0 && B2>0.0) {
        
        if(S<=B1 || S>=B2) {
            val = 0.0;     
        } else {
            
            
            
            double a = 2.0*(rd-rf)/(vol*vol)-1.0;    
            double BB2=sqr(B2/B1);
            double BBa=pow(B2/B1,a);
            double BB2inv=1.0/BB2;
            double BBainv=1.0/BBa;
            double fac=1.0;
            double facinv=1.0;
            double sc=1.0;
            double scinv=1.0;

            
            val=barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,fac,sc,0,pc,fd,greek);
            
            for(int i=1; i<10; i++) {
                fac*=BBa;
                facinv*=BBainv;
                sc*=BB2;
                scinv*=BB2inv;
                double add =
                    barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,fac,sc,i,pc,fd,greek) +
                    barrier_double_term(S,vol,rd,rf,tau,K,B1,B2,facinv,scinv,-i,pc,fd,greek);
                val += add;
                
                if(fabs(add) <= 1e-12*fabs(val)) {
                    break;
                }
            }
            
        }
        
    } else {
        
        assert(false);
    }

    return val;
}


double barrier_ki(double S, double vol, double rd, double rf,
                  double tau, double K, double B1, double B2,
                  types::PutCall pc, types::ForDom fd,
                  types::Greeks greek) {
    return vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
           -barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
}


double barrier(double S, double vol, double rd, double rf,
               double tau, double K, double B1, double B2,
               types::PutCall pc, types::ForDom fd,
               types::BarrierKIO kio, types::BarrierActive bcont,
               types::Greeks greek) {

    double val = 0.0;
    if( kio==types::KnockOut && bcont==types::Maturity ) {
        
        val = vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockOut && bcont==types::Continuous ) {
        
        val = barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Maturity ) {
        
        val = vanilla(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek)
              - vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Continuous ) {
        
        val = barrier_ki(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else {
        
        assert(false);
    }
    return val;
}

} 





double touch(double S, double vol, double rd, double rf,
             double tau, double B1, double B2, types::ForDom fd,
             types::BarrierKIO kio, types::BarrierActive bcont,
             types::Greeks greek) {

    double K=-1.0;                      
    types::PutCall pc = types::Call;    
    double val = 0.0;
    if( kio==types::KnockOut && bcont==types::Maturity ) {
        
        val = internal::vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockOut && bcont==types::Continuous ) {
        
        val = internal::barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Maturity ) {
        
        val = internal::vanilla(S,vol,rd,rf,tau,K,-1.0,-1.0,pc,fd,greek)
              - internal::vanilla_trunc(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else if ( kio==types::KnockIn && bcont==types::Continuous ) {
        
        val = internal::vanilla(S,vol,rd,rf,tau,K,-1.0,-1.0,pc,fd,greek)
              - internal::barrier_ko(S,vol,rd,rf,tau,K,B1,B2,pc,fd,greek);
    } else {
        
        assert(false);
    }
    return val;
}


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
        
        types::BarrierKIO kio2 = (kio==types::KnockIn) ? types::KnockOut
                                 : types::KnockIn;
        val += rebate*touch(S,vol,rd,rf,tau,B1,B2,fd,kio2,bcont,greek);
    }
    return val;
}







double prob_hit(double S, double vol, double mu,
                double tau, double B1, double B2) {
    double rd=0.0;
    double rf=-mu;
    return 1.0 - touch(S,vol,rd,rf,tau,B1,B2,types::Domestic,types::KnockOut,
                       types::Continuous, types::Value);
}





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

    
    if(K<0.0) {
        return prob_in_money(S,vol,mu,tau,B1,B2);
    }

    double val = 0.0;
    double BM1, BM2;     
    
    if( (B1>B2 && B1>0.0 && B2>0.0) ||
            (K>=B2 && B2>0.0 && pc==types::Call) ||
            (K<=B1 && pc==types::Put) ) {
        val = 0.0;
        
    } else if(pc==types::Call) {
        BM1=std::max(B1, K);
        BM2=B2;
        val = prob_in_money(S,vol,mu,tau,BM1,BM2);
    } else if (pc==types::Put) {
        BM1=B1;
        BM2= (B2>0.0) ? std::min(B2,K) : K;
        val = prob_in_money(S,vol,mu,tau,BM1,BM2);
    } else {
        
        assert(false);
    }
    return val;
}



} 

} 
} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
