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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
