/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OPINLINFUN_MATH
#define SC_OPENCL_OPINLINFUN_MATH

std::string Math_Intg_Str=
"\ndouble Intg(double n)\n\
{\n\
    if(trunc(n)==n )\n\
        return n;\n\
    else if(n<0)\n\
        return trunc(n)-1;\n\
    else\n\
        return trunc(n)+1;\n\
}\n";

std::string bikDecl = "double bik(double n,double k);\n";
std::string bik =
"double bik(double n,double k)\n"
"{\n"
"    double nVal = n/k;\n"
"    n = n - 1;\n"
"    k = k - 1;\n"
"    while (k > 0)\n"
"    {\n"
"        nVal = nVal * ( n/k );\n"
"        k = k - 1;\n"
"        n = n - 1;\n"
"    }\n"
"    return nVal;\n"
"}\n";

std::string local_cothDecl = "double local_coth(double n);\n";
std::string local_coth =
"double local_coth(double n)\n"
"{\n"
"    double a = exp(n);\n"
"    double b = exp(-n);\n"
"    double nVal = (a + b) / (a - b);\n"
"    return nVal;\n"
"}\n";

std::string local_coshDecl = "double local_cosh(double n);\n";
std::string local_cosh =
"double local_cosh(double n)\n"
"{\n"
"    double nVal = (exp(n) + exp(-n)) / 2;\n"
"    return nVal;\n"
"}\n";


#endif //SC_OPENCL_OPINLINFUN_MATH

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
