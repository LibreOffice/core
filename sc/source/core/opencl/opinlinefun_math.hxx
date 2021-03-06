/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

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
"    double nVal1 = n;\n"
"    double nVal2 = k;\n"
"    n = n - 1;\n"
"    k = k - 1;\n"
"    while (k > 0)\n"
"    {\n"
"        nVal1 = nVal1 * n;\n"
"        nVal2 = nVal2 * k;\n"
"        k = k - 1;\n"
"        n = n - 1;\n"
"    }\n"
"    return (nVal1 / nVal2);\n"
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
std::string atan2Decl = "double arctan2(double y, double x);\n";
std::string atan2Content =
"double arctan2(double y, double x)\n"
"{\n"
"    if(y==0.0)\n"
"        return 0.0;\n"
"    double a,num,den,tmpPi;\n"
"    int flag;\n"
"    tmpPi = 0;\n"
"    if (fabs(x) >= fabs(y))\n"
"    {\n"
"        num = y;\n"
"        den = x;\n"
"        flag = 1;\n"
"        if (x < 0.0)\n"
"            tmpPi = M_PI;\n"
"    }\n"
"    if(fabs(x) < fabs(y))\n"
"    {\n"
"        num = x;\n"
"        den = y;\n"
"        flag = -1;\n"
"        tmpPi = M_PI_2;\n"
"    }\n"
"    a = atan(num/den);\n"
"    a = flag==1?a:-a;\n"
"    a = a + (y >= 0.0 ? tmpPi : -tmpPi);\n"
"    return a;\n"
"}\n";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
