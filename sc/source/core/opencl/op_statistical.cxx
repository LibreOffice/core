/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_statistical.hxx"

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"
#include <sstream>
#include "opinlinefun_statistical.cxx"

using namespace formula;

namespace sc { namespace opencl {

void OpStandard::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x,mu,sigma;\n";
    if(vSubArguments.size() != 3)
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR0 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
#ifdef ISNAN
        ss << "    int buffer_x_len = ";
        ss << tmpCurDVR0->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_x_len || isNan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        x = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur0->GetType() == formula::svDouble)
    {
        ss << "    x=" <<tmpCur0->GetDouble() << ";\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR1 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur1);
#ifdef ISNAN
        ss << "    int buffer_mu_len = ";
        ss << tmpCurDVR1->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_mu_len || isNan(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        mu = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        mu = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur1->GetType() == formula::svDouble)
    {
        ss << "    mu=" <<tmpCur1->GetDouble() << ";\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    assert(tmpCur2);
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR2 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur2);
#ifdef ISNAN
        ss << "    int buffer_sigma_len = ";
        ss << tmpCurDVR2->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_sigma_len || isNan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        sigma = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        sigma = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur2->GetType() == formula::svDouble)
    {
        ss << "    sigma=" <<tmpCur2->GetDouble() << ";\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    ss << "    if(" << "sigma" << "<=0)\n";
    ss << "        tmp=DBL_MAX;\n";
    ss << "    else\n";
    ss << "        tmp=(" << "x" << "-" << "mu" << ")/" << "sigma" << ";\n";
    ss << "    return tmp;\n";
    ss << "}";
}


void OpWeibull::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x,alpha,beta,kum;\n";
    if(vSubArguments.size() != 4)
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR0 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
#ifdef ISNAN
        ss << "    int buffer_x_len = ";
        ss << tmpCurDVR0->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_x_len || isNan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        x = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur0->GetType() == formula::svDouble)
    {
        ss << "    x=" <<tmpCur0->GetDouble() << ";\n";
    }
    else
    {
        ss << "return DBL_MAX;\n" << "}\n";
        return ;
    }

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR1 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur1);
#ifdef ISNAN
        ss << "    int buffer_alpha_len = ";
        ss << tmpCurDVR1->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_alpha_len || isNan(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        alpha = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        alpha = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur1->GetType() == formula::svDouble)
    {
        ss << "    alpha=" <<tmpCur1->GetDouble() << ";\n";
    }
    else
    {
        ss << "return DBL_MAX;\n" << "}\n";
        return ;
    }

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    assert(tmpCur2);
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR2 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur2);
#ifdef ISNAN
        ss << "    int buffer_beta_len = ";
        ss << tmpCurDVR2->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_beta_len || isNan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        beta = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        beta = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur2->GetType() == formula::svDouble)
    {
        ss << "    beta=" <<tmpCur2->GetDouble() << ";\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    assert(tmpCur3);
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR3 =
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur3);
#ifdef ISNAN
        ss << "    int buffer_kum_len = ";
        ss << tmpCurDVR3->GetArrayLength() << ";\n";
        ss << "    if(gid0>=buffer_kum_len || isNan(";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        kum = 0.0;\n";
        ss << "    else\n";
#endif
        ss << "        kum = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef() << ";\n";
    }
    else if(tmpCur3->GetType() == formula::svDouble)
    {
        ss << "    kum=" <<tmpCur3->GetDouble() << ";\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n" << "}\n";
        return ;
    }
    ss << "    if(alpha<=0.0||beta<=0.0||kum<0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else if(kum==0.0)\n";
    ss << "    {\n";
    ss << "        tmp=alpha/pow(beta,alpha)*pow(x,alpha-1.0)";
    ss << "*exp(-pow(x/beta,alpha));\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "        tmp=1.0-exp(-pow(x/beta,alpha));\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpFisher::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
#endif
    ss << "double tmp=atanh(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpFisherInv::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
#endif
    ss << "double tmp=tanh(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpGamma::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "double tmp=tgamma(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpCorrel::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss << "double vSum = 0.0;\n\t";
    ss << "double vXSum = 0.0;\n\t";
    ss << "double vYSum = 0.0;\n\t";
    ss << "double vXMean = 0.0;\n\t";
    ss << "double vYMean = 0.0;\n\t";

    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double arg0 = 0.0;\n\t";
    ss << "double arg1 = 0.0;\n\t";
    ss << "int cnt = 0;\n\t";

    FormulaToken *pCurX = vSubArguments[0]->GetFormulaToken();
    FormulaToken *pCurY = vSubArguments[1]->GetFormulaToken();
    const formula::DoubleVectorRefToken* pCurDVRX =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCurX);
    const formula::DoubleVectorRefToken* pCurDVRY =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCurY);
    size_t nCurWindowSizeX = pCurDVRX->GetRefRowSize();
    size_t nCurWindowSizeY = pCurDVRY->GetRefRowSize();
    if(nCurWindowSizeX == nCurWindowSizeY)
    {
        ss << "for (int i = ";
        if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
#endif
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
#endif
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        }
        else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
#endif
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        } else {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
#endif
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        }

        ss << "if(cnt < 1) {\n\t\t";
        ss << "return DBL_MIN;\n\t";
        ss << "}\n\t";
        ss << "else {\n\t\t";
        ss << "vXMean = vXSum/cnt;\n\t\t";
        ss << "vYMean = vYSum/cnt;\n\t\t";
        ss << "vXSum = 0.0;\n\t\t";
        ss << "vYSum = 0.0;\n\t\t";

        ss << "for (int i = ";
        if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
#endif
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
#endif
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
#endif
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
            ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
#endif
            ss << "vXSum += ((arg0 - vXMean)*(arg0 - vXMean));\n\t\t\t";
            ss << "vYSum += ((arg1 - vYMean)*(arg1 - vYMean));\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        }

        ss << "if(vXSum == 0.0 || vYSum == 0.0) {\n\t\t\t";
        ss << "return DBL_MIN;\n\t\t";
        ss << "}\n\t\t";
        ss << "else {\n\t\t\t";
        ss << "return vSum/pow(vXSum*vYSum, 0.5);\n\t\t";
        ss << "}\n\t";
        ss << "}\n";
        ss << "}";
    }
}

void OpNegbinomdist::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n\t";
    ss << "double f,s,p;\n\t";
    ss << " int gid0=get_global_id(0);\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss<< "int buffer_f_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_s_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_p_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss<<"if((gid0)>=buffer_p_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"p = 0;\n\telse \n\t\t";
#endif
    ss << "  p = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
#ifdef ISNAN
    ss<<"if((gid0)>=buffer_s_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"s = 0;\n\telse \n\t\t";
#endif
    ss << "  s = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
#ifdef ISNAN
    ss<<"if((gid0)>=buffer_f_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"f = 0;\n\telse \n\t\t";
#endif
    ss << "  f = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
    ss << " double q = 1.0 - p;\n\t";
    ss << " double fFactor = pow(p,s);\n\t";
    ss << " for(int i=0; i<f; i++)\n\t";
    ss << " {\n\t";
    ss << "   fFactor *= ((double)i+s)/((double)i+1.0)*q;\n\t";
    ss << " }\n\t";
    ss << " double tmp=fFactor;\n\t";
    ss << " return tmp;\n";
    ss << "}\n";
}

void OpPearson::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken* pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t CurWindowSize = pCurDVR->GetRefRowSize();

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n\t";
    ss << "int gid0=get_global_id(0);\n\t";
    ss << "double fCount = 0.0;\n\t";
    ss << "double fSumX = 0.0;\n\t";
    ss << "double fSumY = 0.0;\n\t";
    ss << "double fSumDeltaXDeltaY = 0.0;\n\t";
    ss << "double fSumSqrDeltaX    = 0.0;\n\t";
    ss << "double fSumSqrDeltaY    = 0.0;\n\t";
    ss << "double fInx;\n\t";
    ss << "double fIny;\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur1);

    ss<< "int buffer_fInx_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_fIny_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
#endif
    ss << "for(int i=0; i<"<<CurWindowSize<<"; i++)\n\t";
    ss << "{\n\t";
#ifdef ISNAN
    ss<<"if((gid0+i)>=buffer_fInx_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fInx = 0;\n\telse \n\t";
#endif
    ss << "  fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "  ;\n\t";
#ifdef ISNAN
    ss<<"if((gid0+i)>=buffer_fIny_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fIny = 0;\n\telse \n\t";
#endif
    ss << "  fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "  ;\n\t";
    ss << "  double fValX = fInx;\n\t";
    ss << "  double fValY = fIny;\n\t";
    ss << "  fSumX += fValX;\n\t";
    ss << "  fSumY += fValY;\n\t";
    ss << "  fCount = fCount + 1;\n\t";
    ss << "}\n\t";
    ss << "const double fMeanX = fSumX / fCount;\n\t";
    ss << "const double fMeanY = fSumY / fCount;\n\t";
    ss << "for(int i=0; i<"<<CurWindowSize<<"; i++)\n\t";
    ss << "{\n\t";
    ss << "  fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
    ss << "  fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
    ss << "  const double fValX = fInx;\n\t";
    ss << "  const double fValY = fIny;\n\t";
    ss << "  fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);\n\t";
    ss << "  fSumSqrDeltaX    += (fValX - fMeanX) * (fValX - fMeanX);\n\t";
    ss << "  fSumSqrDeltaY    += (fValY - fMeanY) * (fValY - fMeanY);\n\t";
    ss << "}\n\t";
    ss << "double tmp = ( fSumDeltaXDeltaY / ";
    ss << "sqrt( fSumSqrDeltaX * fSumSqrDeltaY));\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpGammaLn::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
#endif
    ss << "double tmp=lgamma(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpGauss::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
                formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
#endif
    ss << "double tmp=0.5 *erfc(-arg0 * 0.7071067811865475)-0.5;\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpGeoMean::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double nVal=0.0;\n\t";
    ss << "int length="<<nCurWindowSize;
    ss << ";\n\tdouble tmp = 0;\n\t";
    ss << "for (int i = 0; i <" << nCurWindowSize << "; i++)\n\t";
    ss << "{\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||((gid0+i)>=";
    ss<<pCurDVR->GetArrayLength();
    ss<<"))\n\t{";
    ss<<"length--;\n\t";
    ss<<"continue;\n\t}\n\t";
#endif
    ss << "nVal += log(arg0);\n\t";
    ss <<"}\n\t";
    ss<<"tmp = exp(nVal/length);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpHarMean::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double nVal=0.0;\n\t";
    ss << "int length="<<nCurWindowSize;
    ss << ";\n\tdouble tmp = 0;\n\t";
    ss << "for (int i = 0; i <" << nCurWindowSize << "; i++)\n\t";
    ss << "{\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||((gid0+i)>=";
    ss<<pCurDVR->GetArrayLength();
    ss<<"))\n\t{";
    ss<<"length--;\n\t";
    ss<<"continue;\n\t}\n\t";
#endif
    ss << "nVal += (1.0/arg0);\n\t}\n\t";
    ss<<"tmp = length/nVal;\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpConfidence::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpConfidence::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double alpha = " << GetBottom() <<";\n";
    ss << "    double sigma = " << GetBottom() <<";\n";
    ss << "    double size = " << GetBottom() <<";\n";
#ifdef  ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur2);
    ss << "    int buffer_alpha_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_sigma_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_size_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n";
#endif
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_alpha_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        alpha = 0;\n    else\n";
#endif
    ss << "        alpha = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_sigma_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        sigma = 0;\n    else\n";
#endif
    ss << "        sigma = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_size_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        size = 0;\n    else\n";
#endif
    ss << "        size = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double rn = floor(size);\n";
    ss << "    if(sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0";
    ss << "|| rn < 1.0)\n";
    ss << "        tmp = -DBL_MAX;\n";
    ss << "    else\n";
    ss << "        tmp = gaussinv(1.0 - alpha / 2.0) * sigma / sqrt( rn );\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCritBinom::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(MinDecl);
}

void OpCritBinom::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double n = " << GetBottom() <<";\n";
    ss << "    double p = " << GetBottom() <<";\n";
    ss << "    double alpha = " << GetBottom() <<";\n";
#ifdef  ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken* >(tmpCur2);
    ss << "    int buffer_n_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_p_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_alpha_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n";
#endif
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_n_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        n = 0;\n    else\n";
#endif
    ss << "        n = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_p_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        p = 0;\n    else\n";
#endif
    ss << "        p = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef  ISNAN
    ss << "    if((gid0)>=buffer_alpha_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        alpha = 0;\n    else\n";
#endif
    ss << "        alpha = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double rn = floor(n);\n";
    ss << "    if (rn < 0.0 || alpha <= 0.0 || alpha >= 1.0 || p < 0.0";
    ss << " || p > 1.0)\n";
    ss << "        tmp = -DBL_MAX;\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        double rq = (0.5 - p) + 0.5;\n";
    ss << "        double fFactor = pow(rq, rn);\n";
    ss << "        if (fFactor <= Min)\n";
    ss << "        {\n";
    ss << "            fFactor = pow(p, rn);\n";
    ss << "            if (fFactor <= Min)\n";
    ss << "                tmp = -DBL_MAX;\n";
    ss << "            else\n";
    ss << "            {\n";
    ss << "                double fSum = 1.0 - fFactor;\n";
    ss << "                uint max =(uint)(rn), i;\n";
    ss << "                for (i = 0; i < max && fSum >= alpha; i++)\n";
    ss << "                {\n";
    ss << "                    fFactor *= (rn - i) / (i + 1) * rq / p;\n";
    ss << "                    fSum -= fFactor;\n";
    ss << "                }\n";
    ss << "                tmp = (rn - i);\n";
    ss << "            }\n";
    ss << "        }\n";
    ss << "        else\n";
    ss << "        {\n";
    ss << "            double fSum = fFactor;\n";
    ss << "            uint max = (uint)(rn), i;\n";
    ss << "            for (i = 0; i < max && fSum < alpha; i++)\n";
    ss << "            {\n";
    ss << "                fFactor *= (rn - i) / (i + 1) * p / rq;\n";
    ss << "                fSum += fFactor;\n";
    ss << "            }\n";
    ss << "            tmp = (i);\n";
    ss << "        }\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpRsq::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken* pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n\t";
    ss << "int gid0=get_global_id(0);\n\t";
    ss << "double fCount = 0.0;\n\t";
    ss << "double fSumX = 0.0;\n\t";
    ss << "double fSumY = 0.0;\n\t";
    ss << "double fSumDeltaXDeltaY = 0.0;\n\t";
    ss << "double fSumSqrDeltaX    = 0.0;\n\t";
    ss << "double fSumSqrDeltaY    = 0.0;\n\t";
    ss << "double fInx;\n\t";
    ss << "double fIny;\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur1);
    ss<< "int buffer_fInx_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_fIny_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
#endif
    ss << "for(int i=0; i<"<<nCurWindowSize<<"; i++)\n\t";
    ss << "{\n\t";
#ifdef ISNAN
    ss<<"if((gid0+i)>=buffer_fInx_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fInx = 0;\n\telse \n\t";
#endif
    ss << "  fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "  ;\n\t";
#ifdef ISNAN
    ss<<"if((gid0+i)>=buffer_fIny_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fIny = 0;\n\telse \n\t";
#endif
    ss << "  fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "  ;\n\t";
    ss << "  double fValX = fInx;\n\t";
    ss << "  double fValY = fIny;\n\t";
    ss << "  fSumX += fValX;\n\t";
    ss << "  fSumY += fValY;\n\t";
    ss << "  fCount = fCount + 1;\n\t";
    ss << "}\n\t";
    ss << "const double fMeanX = fSumX / fCount;\n\t";
    ss << "const double fMeanY = fSumY / fCount;\n\t";
    ss << "for(int i=0; i<"<<nCurWindowSize<<"; i++)\n\t";
    ss << "{\n\t";
    ss << "  fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
    ss << "  fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << " ;\n\t";
    ss << "  const double fValX = fInx;\n\t";
    ss << "  const double fValY = fIny;\n\t";
    ss << "  fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);\n\t";
    ss << "  fSumSqrDeltaX    += (fValX - fMeanX) * (fValX - fMeanX);\n\t";
    ss << "  fSumSqrDeltaY    += (fValY - fMeanY) * (fValY - fMeanY);\n\t";
    ss << "}\n\t";
    ss << "double tmp = ( fSumDeltaXDeltaY / sqrt( fSumSqrDeltaX * fSumSqrDeltaY));\n\t";
    ss << "return (tmp * tmp);\n";
    ss << "}\n";
}
void OpNormdist::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x,mue,sigma,c;\n";
    ss << "    int gid0=get_global_id(0);\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    ss << "    int buffer_x_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_mue_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_sigma_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_c_len = ";
    ss << tmpCurDVR3->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"if((gid0)>=buffer_c_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        c = 0;\nelse \n";
#endif
    ss << "    c = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"if((gid0)>=buffer_sigma_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        sigma = 0;\nelse \n";
#endif
    ss <<"        sigma = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_mue_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        mue = 0;\nelse \n";
#endif
    ss <<"        mue = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss<<"    if((gid0)>=buffer_x_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"    x = 0;\nelse \n";
#endif
    ss <<"   x = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "double mid,tmp;\n";
    ss << "mid = (x - mue)/sigma;\n";
    ss << "if(c)\n";
    ss << "    tmp = 0.5 *erfc(-mid * 0.7071067811865475);\n";
    ss << "else \n";
    ss <<"     tmp=(0.39894228040143268*exp(-(mid * mid)/2.0))/sigma;\n";
    ss << "return tmp;\n";
    ss << "}\n";
}
void OpNormsdist::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x;\n";
    ss << "    int gid0=get_global_id(0);\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss << "int buffer_x_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_x_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        x = 0;\nelse \n";
#endif
    ss << "       x = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double tmp = 0.5 *erfc(-x * 0.7071067811865475);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}
void OpVariationen::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
       if (i)
         ss << ",";
       vSubArguments[i]->GenSlidingWindowDecl(ss);
     }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double inA;\n";
    ss <<"    double inB;\n";
    ss <<"    double tmp;\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss << "    int buffer_fIna_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_fInb_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss << "    if((gid0)>=buffer_fIna_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        inA = 0;\nelse \n";
#endif
    ss << "        inA = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "if((gid0)>=buffer_fInb_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        inB = 0;\nelse \n";
#endif
    ss << "        inB = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    if(inB == 0.0)\n";
    ss << "       tmp = 1.0;\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "      double nVal = 1;\n";
    ss << "      for( int i =0; i<inB; i++)\n";
    ss << "      {\n";
    ss << "        nVal *= inA ;\n";
    ss << "        inA = inA - 1;\n";
    ss << "    }\n";
    ss << "    tmp = nVal;\n";
    ss << "   }\n";
    ss << "   return tmp;\n";
    ss << "}\n";
}
void OpVariationen2::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
       if (i)
         ss << ",";
       vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double inA;\n";
    ss <<"    double inB;\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss << "int buffer_fIna_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_fInb_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss << "    if((gid0)>=buffer_fIna_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "    inA = 0;\nelse \n";
#endif
    ss << "        inA = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "if((gid0)>=buffer_fInb_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "inB = 0;\nelse \n";
#endif
    ss << "    inB = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double tmp = pow(inA,inB);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}
void OpPhi::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x;\n";
    ss << "    int gid0=get_global_id(0);\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_x_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_x_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        x = 0;\n";
    ss <<"    else \n";
#endif
    ss << "       x = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double tmp = 0.39894228040143268 * exp(-(x * x) / 2.0);\n";
    ss << "     return tmp;\n";
    ss << "}\n";
}
void OpNorminv::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss <<"    double q,t,z;\n";
    ss <<"    double x,mue,sigma;\n";
    ss <<"    int gid0=get_global_id(0);\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss << "    int buffer_x_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_mue_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_sigma_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_x_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        x = 0;\n";
    ss <<"    else \n";
#endif
    ss <<"         x ="<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_mue_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        mue = 0;\n";
    ss <<"    else \n";
#endif
    ss <<"        mue ="<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_sigma_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"    sigma = 0;\n";
    ss <<"else \n";
#endif
    ss <<"     sigma ="<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss <<"    q = x -0.5;\n";
    ss <<"    if(fabs(q)<=.425)\n";
    ss <<"   {\n";
    ss <<"        t=0.180625-q*q;\n";
    ss <<"        z=\n"
    "q*\n"
    "(\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "t*2509.0809287301226727";
    ss <<"+33430.575583588128105\n"
                                ")\n"
                                "*t+67265.770927008700853\n"
                            ")\n"
                            "*t+45921.953931549871457\n"
                        ")\n"
                        "*t+13731.693765509461125\n"
                    ")\n"
                    "*t+1971.5909503065514427\n"
                ")\n"
                "*t+133.14166789178437745\n"
            ")\n"
            "*t+3.387132872796366608\n"
        ")\n"
        "/\n"
        "(\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "t*5226.495278852854561";
    ss <<"+28729.085735721942674\n"
                                ")\n"
                                "*t+39307.89580009271061\n"
                            ")\n"
                            "*t+21213.794301586595867\n"
                        ")\n"
                        "*t+5394.1960214247511077\n"
                    ")\n"
                    "*t+687.1870074920579083\n"
                ")\n"
                "*t+42.313330701600911252\n"
            ")\n"
            "*t+1.0\n"
    ");\n";
    ss <<"}\nelse\n{\n";
    ss <<" if(q>0)\nt=1-x;\n";
    ss <<"else\nt=x;\n";
    ss <<"t=sqrt(-log(t));\n";
    ss <<"if(t<=5.0)\n{\n";
    ss <<"t+=-1.6;\n";
    ss <<"z=\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                               "(\n"
                                   "(\n"
                                        "t*7.7454501427834140764e-4";
    ss <<"+0.0227238449892691845833\n"
                                    ")\n"
                                    "*t+0.24178072517745061177\n"
                                ")\n"
                                "*t+1.27045825245236838258\n"
                            ")\n"
                            "*t+3.64784832476320460504\n"
                        ")\n"
                        "*t+5.7694972214606914055\n"
                    ")\n"
                    "*t+4.6303378461565452959\n"
                ")\n"
                "*t+1.42343711074968357734\n"
            ")\n"
            "/\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*1.05075007164441684324e-9";
    ss <<"+5.475938084995344946e-4\n"
                                    ")\n"
                                    "*t+0.0151986665636164571966\n"
                                ")\n"
                                "*t+0.14810397642748007459\n"
                            ")\n"
                            "*t+0.68976733498510000455\n"
                        ")\n"
                        "*t+1.6763848301838038494\n"
                    ")\n"
                    "*t+2.05319162663775882187\n"
                ")\n"
                "*t+1.0\n"
            ");\n}\n";
    ss <<"else\n{\n";
    ss <<"t+=-5.0;\n";
    ss <<"z=\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*2.01033439929228813265e-7";
    ss<<"+2.71155556874348757815e-5\n"
                                    ")\n"
                                    "*t+0.0012426609473880784386\n"
                                ")\n"
                                "*t+0.026532189526576123093\n"
                            ")\n"
                            "*t+0.29656057182850489123\n"
                        ")\n"
                        "*t+1.7848265399172913358\n"
                    ")\n"
                    "*t+5.4637849111641143699\n"
                ")\n"
                "*t+6.6579046435011037772\n"
            ")\n"
            "/\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*2.04426310338993978564e-15"
          "+1.4215117583164458887e-7\n"
                                    ")\n"
                                    "*t+1.8463183175100546818e-5\n"
                                ")\n"
                                "*t+7.868691311456132591e-4\n"
                            ")\n"
                            "*t+0.0148753612908506148525\n"
                        ")\n"
                        "*t+0.13692988092273580531\n"
                    ")\n"
                    "*t+0.59983220655588793769\n"
                ")\n"
                "*t+1.0\n"
            ");\n";
    ss<<"}\n";
    ss<<"if(q<0.0) z=-z;\n";
    ss<<"}\n";
    ss<<"double tmp =  z*sigma + mue;\n";
    ss<<"return tmp;\n";
    ss<<"}\n";
}
void OpNormsinv:: GenSlidingWindowFunction
    (std::stringstream &ss,const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double q,t,z,x;\n";
    ss << "    int gid0=get_global_id(0);\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_x_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if((gid0)>=buffer_x_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"       x = 0;\n";
    ss <<"   else \n";
#endif
    ss <<"        x ="<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss <<"    q = x -0.5;\n";
    ss <<"    if(fabs(q)<=.425)\n";
    ss <<"    {\n";
    ss <<"        t=0.180625-q*q;\n";
    ss <<"        z=\n"
         "q*\n"
         "(\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "t*2509.0809287301226727";
    ss <<"+33430.575583588128105\n"
                                ")\n"
                                "*t+67265.770927008700853\n"
                            ")\n"
                            "*t+45921.953931549871457\n"
                        ")\n"
                        "*t+13731.693765509461125\n"
                    ")\n"
                    "*t+1971.5909503065514427\n"
                ")\n"
                "*t+133.14166789178437745\n"
            ")\n"
            "*t+3.387132872796366608\n"
        ")\n"
        "/\n"
        "(\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "t*5226.495278852854561";
    ss <<"+28729.085735721942674\n"
                                ")\n"
                                "*t+39307.89580009271061\n"
                            ")\n"
                            "*t+21213.794301586595867\n"
                        ")\n"
                        "*t+5394.1960214247511077\n"
                    ")\n"
                    "*t+687.1870074920579083\n"
                ")\n"
                "*t+42.313330701600911252\n"
            ")\n"
            "*t+1.0\n"
         ");\n";
    ss <<"}\nelse\n{\n";
    ss <<" if(q>0)\nt=1-x;\n";
    ss <<"else\nt=x;\n";
    ss <<"t=sqrt(-log(t));\n";
    ss <<"if(t<=5.0)\n{\n";
    ss <<"t+=-1.6;\n";
    ss <<"z=\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                               "(\n"
                                   "(\n"
                                        "t*7.7454501427834140764e-4";
    ss <<"+0.0227238449892691845833\n"
                                    ")\n"
                                    "*t+0.24178072517745061177\n"
                                ")\n"
                                "*t+1.27045825245236838258\n"
                            ")\n"
                            "*t+3.64784832476320460504\n"
                        ")\n"
                        "*t+5.7694972214606914055\n"
                    ")\n"
                    "*t+4.6303378461565452959\n"
                ")\n"
                "*t+1.42343711074968357734\n"
            ")\n"
            "/\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*1.05075007164441684324e-9";
    ss <<"+5.475938084995344946e-4\n"
                                    ")\n"
                                    "*t+0.0151986665636164571966\n"
                                ")\n"
                                "*t+0.14810397642748007459\n"
                            ")\n"
                            "*t+0.68976733498510000455\n"
                        ")\n"
                        "*t+1.6763848301838038494\n"
                    ")\n"
                    "*t+2.05319162663775882187\n"
                ")\n"
                "*t+1.0\n"
            ");\n}\n";
    ss <<"else\n{\n";
    ss <<"t+=-5.0;\n";
    ss <<"z=\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*2.01033439929228813265e-7";
    ss <<"+2.71155556874348757815e-5\n"
                                    ")\n"
                                    "*t+0.0012426609473880784386\n"
                                ")\n"
                                "*t+0.026532189526576123093\n"
                            ")\n"
                            "*t+0.29656057182850489123\n"
                        ")\n"
                        "*t+1.7848265399172913358\n"
                    ")\n"
                    "*t+5.4637849111641143699\n"
                ")\n"
                "*t+6.6579046435011037772\n"
            ")\n"
            "/\n"
            "(\n"
                "(\n"
                    "(\n"
                        "(\n"
                            "(\n"
                                "(\n"
                                    "(\n"
                                        "t*2.04426310338993978564e-15"
          "+1.4215117583164458887e-7\n"
                                    ")\n"
                                    "*t+1.8463183175100546818e-5\n"
                                ")\n"
                                "*t+7.868691311456132591e-4\n"
                            ")\n"
                            "*t+0.0148753612908506148525\n"
                        ")\n"
                        "*t+0.13692988092273580531\n"
                    ")\n"
                    "*t+0.59983220655588793769\n"
                ")\n"
                "*t+1.0\n"
            ");\n";
    ss <<"}\n";
    ss <<"if(q<0.0) z=-z;\n";
    ss <<"}\n";
    ss <<"double tmp = z;\n";
    ss <<"return tmp;\n";
    ss <<"}\n";
}
void OpMedian::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    ss << "    int i;\n";
    ss << "    unsigned int startFlag = 0;\n";
    ss << "    unsigned int endFlag = 0;\n";
    ss << "    double dataIna;\n";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (const formula::DoubleVectorRefToken* pCurDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur))
        {
            size_t nCurWindowSize = pCurDVR->GetRefRowSize();
            ss << "startFlag = ";
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
            {
                ss << "gid0; endFlag = "<< nCurWindowSize <<"-gid0;\n";
            }
            else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
            {
                ss << "gid0; endFlag = gid0+"<< nCurWindowSize <<";\n";
            }
            else
            {
                ss << "gid0; endFlag = gid0+"<< nCurWindowSize <<";\n";
            }
        }
        else
        {
            ss<<"startFlag=gid0;endFlag=gid0;\n";
        }
    }
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur0);
    ss << "int buffer_fIna_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss<<"if((i+gid0)>=buffer_fIna_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"    dataIna = 0;\n";
#endif
    ss << "    int nSize =endFlag- startFlag ;\n";
    ss << "    if (nSize & 1)\n";
    ss << "    {\n";
    ss << "        tmp = "<<vSubArguments[0]->GetNameAsString();
    ss << "        [startFlag+nSize/2];\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        tmp =("<<vSubArguments[0]->GetNameAsString();
    ss << "        [startFlag+nSize/2]+";
    ss <<          vSubArguments[0]->GetNameAsString();
    ss << "        [startFlag+nSize/2-1])/2;\n";
    ss << "    }\n";
    ss <<"     return tmp;\n";
    ss << "}\n";
}
void OpKurt:: GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
        FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
        assert(pCur);
        const formula::DoubleVectorRefToken* pCurDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
        size_t nCurWindowSize = pCurDVR->GetRefRowSize();
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"( ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n";
        ss << "    int gid0 = get_global_id(0);\n";
        ss << "    double fSum = 0.0;\n";
        ss << "    double vSum = 0.0;\n";
        ss << "    int length="<<nCurWindowSize<<";\n";
        ss << "    double tmp = 0;\n";
        ss << "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
        ss << "    {\n";
        ss << "        double arg0 = ";
        ss<< vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
#ifdef ISNAN
        ss<< "        if(isNan(arg0)||((gid0+i)>=";
        ss<<pCurDVR->GetArrayLength();
        ss<< "))\n";
        ss<< "        {\n";
        ss<< "            length--;\n";
        ss<< "            continue;\n";
        ss<< "        }\n";
#endif
        ss<< "        fSum += arg0;\n";
        ss<< "    }\n";
        ss<< "    double fMean = fSum / length;\n";
        ss<< "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
        ss<< "    {\n";
        ss<< "        double arg0 = ";
        ss<< vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<< ";\n";
#ifdef ISNAN
        ss<< "        if(isNan(arg0)||((gid0+i)>=";
        ss<< pCurDVR->GetArrayLength();
        ss<< "))\n";
        ss<< "        {\n";
        ss<< "            continue;\n";
        ss<< "        }\n";
#endif
        ss<< "        vSum += (arg0 - fMean) * (arg0 - fMean);\n";
        ss<< "    }\n";
        ss<< "    double fStdDev = sqrt(vSum / (length - 1.0));\n";
        ss<< "    double dx = 0.0;\n";
        ss<< "    double xpower4 = 0.0;\n";
        ss<< "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
        ss<< "    {\n";
        ss<< "        double arg0 = ";
        ss<< vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<< ";\n";
#ifdef ISNAN
        ss<< "        if(isNan(arg0)||((gid0+i)>=";
        ss<< pCurDVR->GetArrayLength();
        ss<< "))\n";
        ss<< "        {\n";
        ss<< "            continue;\n";
        ss<< "        }\n";
#endif
        ss<< "        dx = (arg0 -fMean) / fStdDev;\n";
        ss<< "        xpower4 = xpower4 + (dx * dx * dx * dx);\n";
        ss<< "    }\n";
        ss<< "    double k_d = (length - 2.0) * (length - 3.0);\n";
        ss<< "    double k_l = length * (length + 1.0) /";
        ss<< "((length - 1.0) * k_d);\n";
        ss<< "    double k_t = 3.0 * (length - 1.0) * (length - 1.0) / k_d;\n";
        ss<< "    tmp = xpower4 * k_l - k_t;\n";
        ss<< "    return tmp;\n";
        ss<< "}";
}
void OpIntercept::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
    assert(pCur1);
    const formula::DoubleVectorRefToken* pCurDVR1 =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur1);
    size_t nCurWindowSize1 = pCurDVR1->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fSumSqrDeltaX = 0.0;\n";
    ss << "    int length="<<nCurWindowSize;
    ss << ";\n";
    ss << "    int length1= "<<nCurWindowSize1;
    ss << ";\n";
    ss << "    if(length!=length1)\n";
    ss << "        return 0;\n";
    ss << "    double tmp = 0;\n";
    ss << "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
    ss << "    {\n";
    ss << "        double arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "        double arg1 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "        if(isNan(arg0)||((gid0+i)>=";
    ss << pCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        {\n";
    ss << "            length--;\n";
    ss << "            continue;\n";
    ss << "        }\n";
#endif
#ifdef ISNAN
    ss << "        if(isNan(arg1)||((gid0+i)>=";
    ss << pCurDVR1->GetArrayLength();
    ss << "))\n";
    ss << "        {\n";
    ss << "            length--;\n";
    ss << "            continue;\n";
    ss << "        }\n";
#endif
    ss << "        fSumY+=arg0;\n";
    ss << "        fSumX+=arg1;\n";
    ss << "    }\n";
    ss <<"    double fMeanX = fSumX / length;\n";
    ss <<"    double fMeanY = fSumY / length;\n";
    ss << "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
    ss << "    {\n";
    ss << "        double arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "        double arg1 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "        if(isNan(arg0)||((gid0+i)>=";
    ss << pCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        {\n";
    ss << "            continue;\n";
    ss << "        }\n";
#endif
#ifdef ISNAN
    ss << "        if(isNan(arg1)||((gid0+i)>=";
    ss <<pCurDVR1->GetArrayLength();
    ss <<"))\n";
    ss << "        {";
    ss << "            continue;\n";
    ss << "        }\n";
#endif
    ss << "        fSumDeltaXDeltaY+=(arg1 - fMeanX) * (arg0 - fMeanY);";
    ss << ";\n";
    ss << "        fSumSqrDeltaX+=(arg1 - fMeanX) * (arg1 - fMeanX);\n";
    ss << "    }\n";
    ss << "    tmp = fMeanY - fSumDeltaXDeltaY / fSumSqrDeltaX";
    ss << "* fMeanX;\n";
    ss << "    return tmp;\n";
        ss << "}";
}
void OpLogInv:: GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
          formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
          formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
          formula::SingleVectorRefToken *>(tmpCur2);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double arg1 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double arg2 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double tmp;\n";
#ifdef ISNAN
    ss<< "    if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR0->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
#endif
#ifdef ISNAN
    ss<< "    if(isNan(arg1)||(gid0>=";
    ss<< tmpCurDVR1->GetArrayLength();
    ss<< "))\n";
    ss<< "        arg1 = 0;\n";
#endif
#ifdef ISNAN
    ss<< "    if(isNan(arg2)||(gid0>=";
    ss<< tmpCurDVR2->GetArrayLength();
    ss<< "))\n";
    ss<< "        arg2 = 0;\n";
#endif
    ss<< "    double q,t,z;\n";
    ss<< "    q = arg0 -0.5;\n";
    ss<< "    if(fabs(q)<=.425)\n";
    ss<< "    {\n";
    ss<< "        t=0.180625-q*q;\n";
    ss<< "        z=\n"
    "        q*\n"
    "        (\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    t*2509.0809287301226727";
    ss<<"+33430.575583588128105\n"
    "                                )\n"
    "                                *t+67265.770927008700853\n"
    "                            )\n"
    "                            *t+45921.953931549871457\n"
    "                        )\n"
    "                        *t+13731.693765509461125\n"
    "                    )\n"
    "                    *t+1971.5909503065514427\n"
    "                )\n"
    "                *t+133.14166789178437745\n"
    "            )\n"
    "            *t+3.387132872796366608\n"
    "        )\n"
    "        /\n"
    "        (\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    t*5226.495278852854561";
    ss<<"+28729.085735721942674\n"
    "                                )\n"
    "                                *t+39307.89580009271061\n"
    "                            )\n"
    "                            *t+21213.794301586595867\n"
    "                        )\n"
    "                        *t+5394.1960214247511077\n"
    "                    )\n"
    "                    *t+687.1870074920579083\n"
    "                )\n"
    "                *t+42.313330701600911252\n"
    "            )\n"
    "            *t+1.0\n"
    "        );\n";
    ss<<"    }\n";
    ss<<"    else\n";
    ss<<"    {\n";
    ss<<"        if(q>0)\n";
    ss<<"            t=1-arg0;\n";
    ss<<"        else\n";
    ss<<"            t=arg0;\n";
    ss<<"        t=sqrt(-log(t));\n";
    ss<<"        if(t<=5.0)\n";
    ss<<"        {\n";
    ss<<"            t+=-1.6;\n";
    ss<<"            z=\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    (\n"
    "                                        t*7.7454501427834140764e-4";
    ss<<"+0.0227238449892691845833\n"
    "                                    )\n"
    "                                    *t+0.24178072517745061177\n"
    "                                )\n"
    "                                *t+1.27045825245236838258\n"
    "                            )\n"
    "                            *t+3.64784832476320460504\n"
    "                        )\n"
    "                        *t+5.7694972214606914055\n"
    "                    )\n"
    "                    *t+4.6303378461565452959\n"
    "                )\n"
    "                *t+1.42343711074968357734\n"
    "            )\n"
    "            /\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    (\n"
    "                                        t*1.05075007164441684324e-9";
    ss<<"+5.475938084995344946e-4\n"
    "                                    )\n"
    "                                    *t+0.0151986665636164571966\n"
    "                                )\n"
    "                                *t+0.14810397642748007459\n"
    "                            )\n"
    "                            *t+0.68976733498510000455\n"
    "                        )\n"
    "                        *t+1.6763848301838038494\n"
    "                    )\n"
    "                    *t+2.05319162663775882187\n"
    "                )\n"
    "                *t+1.0\n"
    "            );\n";
    ss<<"        }\n";
    ss<<"        else\n";
    ss<<"        {\n";
    ss<<"            t+=-5.0;\n";
    ss<<"            z=\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    (\n"
    "                                        t*2.01033439929228813265e-7";
    ss<<"+2.71155556874348757815e-5\n"
    "                                    )\n"
    "                                    *t+0.0012426609473880784386\n"
    "                                )\n"
    "                                *t+0.026532189526576123093\n"
    "                            )\n"
    "                            *t+0.29656057182850489123\n"
    "                        )\n"
    "                        *t+1.7848265399172913358\n"
    "                    )\n"
    "                    *t+5.4637849111641143699\n"
    "                )\n"
    "                *t+6.6579046435011037772\n"
    "            )\n"
    "            /\n"
    "            (\n"
    "                (\n"
    "                    (\n"
    "                        (\n"
    "                            (\n"
    "                                (\n"
    "                                    (\n"
    "                                        t*2.04426310338993978564e-15"
    " +1.4215117583164458887e-7\n"
    "                                    )\n"
    "                                    *t+1.8463183175100546818e-5\n"
    "                                )\n"
    "                                *t+7.868691311456132591e-4\n"
    "                            )\n"
    "                            *t+0.0148753612908506148525\n"
    "                        )\n"
    "                        *t+0.13692988092273580531\n"
    "                    )\n"
    "                    *t+0.59983220655588793769\n"
    "                )\n"
    "                *t+1.0\n"
    "            );\n";
    ss << "        }\n";
    ss << "        if(q<0.0)\n";
    ss << "            z=-z;\n";
    ss << "    }\n";
    ss << "    tmp = exp(arg1+arg2*z);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
