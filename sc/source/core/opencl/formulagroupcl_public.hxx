/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_FORMULAGROUPCL_PUBLIC_HXX
#define SC_OPENCL_FORMULAGROUPCL_PUBLIC_HXX

const char* publicFunc =
 "int isNan(double a) { return isnan(a); }\n"
 "double fsum_count(double a, double b, __private int *p) {\n"
 "    bool t = isNan(a);\n"
 "    (*p) += t?0:1;\n"
 "    return t?b:a+b;\n"
 "}\n"
 "double fsum(double a, double b) { return isNan(a)?b:a+b; }\n"
 "double legalize(double a, double b) { return isNan(a)?b:a;}\n"
 "double fsub(double a, double b) { return a-b; }\n"
 "double fdiv(double a, double b) { return a/b; }\n"
 "double strequal(unsigned a, unsigned b) { return (a==b)?1.0:0; }\n"
#ifdef USE_FMIN_FMAX
 "double mcw_fmin(double a, double b) { return fmin(a, b); }\n"
 "double mcw_fmax(double a, double b) { return fmax(a, b); }\n"
#else
 "double mcw_fmin(double a, double b) { return a>b?b:a; }\n"
 "double mcw_fmax(double a, double b) { return a>b?a:b; }\n"
#endif

 ;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
