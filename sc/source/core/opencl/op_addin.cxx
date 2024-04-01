/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_addin.hxx"

#include <sstream>

namespace sc::opencl {

void OpBesselj::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "N", 1, vSubArguments, ss );
    ss << "    double f_2_DIV_PI = 2.0 / M_PI;\n";
    ss << "    if( N < 0.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    if (x == 0.0)\n";
    ss << "        return (N == 0.0) ? 1.0 : 0.0;\n";
    ss << "    double fSign = ((int)N % 2 == 1 && x < 0.0) ? -1.0 : 1.0;\n";
    ss << "    double fX = fabs(x);\n";
    ss << "    double fMaxIteration = 9000000.0;\n";
    ss << "    double fEstimateIteration = fX * 1.5 + N;\n";
    ss << "    bool bAsymptoticPossible = pow(fX,0.4) > N;\n";
    ss << "    if (fEstimateIteration > fMaxIteration)\n";
    ss << "    {\n";
    ss << "        if (bAsymptoticPossible)\n";
    ss << "            return fSign * sqrt(f_2_DIV_PI/fX)";
    ss << "* cos(fX-N*M_PI_2-M_PI_4);\n";
    ss << "        else\n";
    ss << "            return CreateDoubleError(NoConvergence);\n";
    ss << "    }\n";
    ss << "    double epsilon = 1.0e-15;\n";
    ss << "    bool bHasfound = false;\n";
    ss << "    double k= 0.0;\n";
    ss << "    double  u ;\n";
    ss << "    double m_bar;\n";
    ss << "    double g_bar;\n";
    ss << "    double g_bar_delta_u;\n";
    ss << "    double g = 0.0;\n";
    ss << "    double delta_u = 0.0;\n";
    ss << "    double f_bar = -1.0;\n";
    ss << "    if (N==0)\n";
    ss << "    {\n";
    ss << "        u = 1.0;\n";
    ss << "        g_bar_delta_u = 0.0;\n";
    ss << "        g_bar = - 2.0/fX; \n";
    ss << "        delta_u = g_bar_delta_u / g_bar;\n";
    ss << "        u = u + delta_u ;\n";
    ss << "        g = -1.0 / g_bar; \n";
    ss << "        f_bar = f_bar * g;\n";
    ss << "        k = 2.0;\n";
    ss << "    }\n";
    ss << "    if (N!=0)\n";
    ss << "    {\n";
    ss << "        u=0.0;\n";
    ss << "        for (k =1.0; k<= N-1; k = k + 1.0)\n";
    ss << "        {\n";
    ss << "            m_bar=2.0 * fmod(k-1.0, 2.0) * f_bar;\n";
    ss << "            g_bar_delta_u = - g * delta_u - m_bar * u;\n";
    ss << "            g_bar = m_bar - 2.0*k/fX + g;\n";
    ss << "            delta_u = g_bar_delta_u / g_bar;\n";
    ss << "            u = u + delta_u;\n";
    ss << "            g = -1.0/g_bar;\n";
    ss << "            f_bar=f_bar * g;\n";
    ss << "        }\n";
    ss << "        m_bar=2.0 * fmod(k-1.0, 2.0) * f_bar;\n";
    ss << "        g_bar_delta_u = f_bar - g * delta_u - m_bar * u;\n";
    ss << "        g_bar = m_bar - 2.0*k/fX + g;\n";
    ss << "        delta_u = g_bar_delta_u / g_bar;\n";
    ss << "        u = u + delta_u;\n";
    ss << "        g = -1.0/g_bar;\n";
    ss << "        f_bar = f_bar * g;\n";
    ss << "        k = k + 1.0;\n";
    ss << "    }\n";
    ss << "    do\n";
    ss << "    {\n";
    ss << "        m_bar = 2.0 * fmod(k-1.0, 2.0) * f_bar;\n";
    ss << "        g_bar_delta_u = - g * delta_u - m_bar * u;\n";
    ss << "        g_bar = m_bar - 2.0*k/fX + g;\n";
    ss << "        delta_u = g_bar_delta_u / g_bar;\n";
    ss << "        u = u + delta_u;\n";
    ss << "        g = -pow(g_bar,-1.0);\n";
    ss << "        f_bar = f_bar * g;\n";
    ss << "        bHasfound = (fabs(delta_u)<=fabs(u)*epsilon);\n";
    ss << "        k = k + 1.0;\n";
    ss << "    }\n";
    ss << "    while (!bHasfound && k <= fMaxIteration);\n";
    ss << "    if (bHasfound)\n";
    ss << "        return u * fSign;\n";
    ss << "    else\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "}";
}
void OpGestep::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp=0;\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss <<"\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    ss << "    tmp =tmp0 >= tmp1 ? 1 : 0;\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
