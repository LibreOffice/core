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
#include <formula/vectortoken.hxx>
#include <sstream>
#include "opinlinefun_statistical.cxx"

using namespace formula;

namespace sc { namespace opencl {
void OpVar::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "        {\n";
                ss << "            arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg=" << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(errDivisionByZero);\n";
    ss << "    else\n";
    ss << "        return vSum * pow(fCount - 1.0,-1.0);\n";
    ss << "}\n";
}
void OpZTest::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(phiDecl);
    funs.insert(phi);
    decls.insert(taylorDecl);
    funs.insert(taylor);
    decls.insert(gaussDecl);
    funs.insert(gauss);
}
void OpZTest::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fSumSqr = 0.0;\n";
    ss << "    double mue = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    ss << "    double sigma = 0.0;\n";
    ss << "    double mu = 0.0;\n";
    if(vSubArguments.size() == 1 || vSubArguments.size() == 0)
    {
        ss << "    return DBL_MAX;\n";
        ss << "}\n";
        return ;
    }
    else if(vSubArguments.size() == 2)
    {
        FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
        FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
        assert(pCur);
        assert(pCur1);
        if(pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++)\n";
                ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                ss << "    {\n";
            }
            else
            {
                ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                ss << "    {\n";
            }

            ss << "        arg = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(arg))\n";
            ss << "            continue;\n";
            ss << "        fSum += arg;\n";
            ss << "        fSumSqr += arg * arg;\n";
            ss << "        fCount += 1.0;\n";
            ss << "    }\n";
            ss << "    if(fCount <= 1.0)\n";
            ss << "        return DBL_MAX;\n";
            ss << "    mue = fSum *pow(fCount,-1.0);\n";
            ss << "    sigma = (fSumSqr-fSum*fSum*";
            ss << "pow(fCount,-1.0))*pow(fCount-1.0,-1.0);\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }
        if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
        {
            if(pCur1->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast<const formula::SingleVectorRefToken* >(pCur1);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        mu = " ;
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(mu))\n";
                ss << "            mu = 0.0;\n";
                ss << "    }\n";

            }
            else if(pCur1->GetType() == formula::svDouble)
            {
                ss << "    mu = " << pCur1->GetDouble() << ";\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    mu = " ;
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        }
        ss << "    return 0.5 - gauss((mue-mu)/sqrt(sigma/fCount));\n";
        ss << "}\n";
        return ;
    }
    else
    {
        FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
        FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
        FormulaToken *pCur2 = vSubArguments[2]->GetFormulaToken();
        assert(pCur);
        assert(pCur1);
        assert(pCur2);
        if(pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++)\n";
                ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                ss << "    {\n";
            }
            else
            {
                ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                ss << "    {\n";
            }
            ss << "        arg = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(arg))\n";
            ss << "            continue;\n";
            ss << "        fSum += arg;\n";
            ss << "        fSumSqr += arg * arg;\n";
            ss << "        fCount += 1.0;\n";
            ss << "    }\n";
            ss << "    if(fCount <= 1.0)\n";
            ss << "        return DBL_MAX;\n";
            ss << "    mue = fSum * pow(fCount,-1.0);\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }
        if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
        {
            if(pCur1->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR1 =
                    static_cast<const formula::SingleVectorRefToken* >(pCur1);
                ss << "    if (gid0 < " << pSVR1->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        mu = " ;
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(mu))\n";
                ss << "            mu = 0.0;\n";
                ss << "    }\n";
            }
            else if(pCur1->GetType() == formula::svDouble)
            {
                ss << "    mu = " << pCur1->GetDouble() << ";\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    mu=" ;
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        }
        if(ocPush == vSubArguments[2]->GetFormulaToken()->GetOpCode())
        {
            if(pCur2->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR2 =
                    static_cast<const formula::SingleVectorRefToken* >(pCur2);
                ss << "    if (gid0 < " << pSVR2->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        sigma = " ;
                ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(sigma))\n";
                ss << "            sigma = 0.0;\n";
                ss << "    }\n";
            }
            else if(pCur2->GetType() == formula::svDouble)
            {
                ss << "    sigma = " << pCur2->GetDouble() << ";\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    sigma = " ;
            ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
        }
        ss << "    return 0.5 - gauss((mue-mu)*sqrt(fCount)/sigma);\n";
        ss << "}\n";
    }
}

void OpTTest::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
    decls.insert(GetBetaDecl);
    funs.insert(GetBeta);
    decls.insert(GetLogBetaDecl);
    funs.insert(GetLogBeta);
    decls.insert(GetBetaDistPDFDecl);
    funs.insert(GetBetaDistPDF);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    funs.insert(lcl_GetBetaHelperContFrac);
    decls.insert(GetBetaDistDecl);
    funs.insert(GetBetaDist);
    decls.insert(GetTDistDecl);
    funs.insert(GetTDist);
}

void OpTTest::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum1 = 0.0;\n";
    ss << "    double fSum2 = 0.0;\n";
    ss << "    double fSumSqr1 = 0.0;\n";
    ss << "    double fSumSqr2 = 0.0;\n";
    ss << "    double fCount1 = 0.0;\n";
    ss << "    double fCount2 = 0.0;\n";
    ss << "    double arg1 = 0.0;\n";
    ss << "    double arg2 = 0.0;\n";
    ss << "    double mode = 0.0;\n";
    ss << "    double type = 0.0;\n";
    ss << "    double fT = 0.0;\n";
    ss << "    double fF = 0.0;\n";
    if(vSubArguments.size() != 4)
    {
        ss << "    return DBL_MAX;\n";
        ss << "}\n";
        return ;
    }
    if(vSubArguments.size() == 4)
    {
        FormulaToken *pCur  = vSubArguments[0]->GetFormulaToken();
        FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
        FormulaToken *pCur2 = vSubArguments[2]->GetFormulaToken();
        FormulaToken *pCur3 = vSubArguments[3]->GetFormulaToken();
        assert(pCur);
        assert(pCur1);
        assert(pCur2);
        assert(pCur3);
        if(ocPush == vSubArguments[2]->GetFormulaToken()->GetOpCode())
        {
            if(pCur2->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur2);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        mode = " ;
                ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(mode))\n";
                ss << "            mode = 0.0;\n";
                ss << "        else\n";
                ss << "            mode = floor(mode);\n";
                ss << "    }\n";
            }
            else if(pCur2->GetType() == formula::svDouble)
            {
                ss << "    mode = floor(convert_double(";
                ss << pCur2->GetDouble() << "));\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    mode = floor(" ;
            ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ");\n";
        }
        ss << "    if(!(mode == 1.0 || mode == 2.0))\n";
        ss << "        return DBL_MAX;\n";
        if(ocPush==vSubArguments[3]->GetFormulaToken()->GetOpCode())
        {
            if(pCur3->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur3);
                assert(pSVR);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        if (isNan(";
                ss << vSubArguments[3]->GenSlidingWindowDeclRef() << "))\n";
                ss << "            type=0.0;\n";
                ss << "        else\n";
                ss << "            type=floor(";
                ss << vSubArguments[3]->GenSlidingWindowDeclRef() << ");\n";
                ss << "    }\n";
            }
            else if(pCur3->GetType() == formula::svDouble)
            {
                ss << "    type = floor(convert_double(" << pCur3->GetDouble() <<
                "));\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    type=floor(";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef() << ");\n";
        }
        ss << "    if(!(type == 1.0||type == 2.0||type == 3.0))\n";
        ss << "        return DBL_MAX;\n";

        if(pCur->GetType() == formula::svDoubleVectorRef &&
               pCur1->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            const formula::DoubleVectorRefToken* pDVR1 =
                static_cast<const formula::DoubleVectorRefToken *>(pCur1);

            size_t nCurWindowSize  = pDVR->GetRefRowSize();
            size_t nCurWindowSize1 = pDVR1->GetRefRowSize();

            if(nCurWindowSize == nCurWindowSize1)
            {
                ss << "    if(type == 1.0)\n";
                ss << "    {\n";
                ss << "        for (int i = ";
                if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed()) &&
                         (!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "        {\n";
                }
                else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed()) &&
                             (pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+"<< nCurWindowSize << "; i++)\n";
                    ss << "        {\n";
                }
                else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed()) &&
                             (!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "        {\n";
                }
                else if ((pDVR->IsStartFixed() && pDVR->IsEndFixed()) &&
                             (pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
                {
                    ss << "0; i < " << nCurWindowSize << "; i++)\n";
                    ss << "        {\n";
                }
                else
                {
                    ss << "0; i < " << nCurWindowSize << "; i++)\n";
                    ss << "        {\n";
                    ss << "            break;\n";
                    ss << "        }";
                    ss << "        return DBL_MAX;\n";
                    ss << "    }\n";
                    ss << "}\n";
                    return ;
                }

                ss << "            arg1 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef(true) << ";\n";
                ss << "            arg2 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef(true) << ";\n";
                ss << "            if (isNan(arg1)||isNan(arg2))\n";
                ss << "                continue;\n";
                ss << "            fSum1 += arg1;\n";
                ss << "            fSum2 += arg2;\n";
                ss << "            fSumSqr1 += (arg1 - arg2)*(arg1 - arg2);\n";
                ss << "            fCount1 += 1;\n";
                ss << "        }\n";
                ss << "        if(fCount1 < 1.0)\n";
                ss << "            return DBL_MAX;\n";
                ss << "        fT = sqrt(fCount1-1.0) * fabs(fSum1 - fSum2)\n";
                ss << "            /sqrt(fCount1 * fSumSqr1 - (fSum1-fSum2)\n";
                ss << "             *(fSum1-fSum2));\n";
                ss << "        fF = fCount1 - 1.0;\n";
            }
            else
            {
                ss << "    return DBL_MAX;\n";
                ss << "}\n";
                return ;
            }
        }
        else
        {
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }
        ss << "    }\n";
        ss << "    if(type == 2.0 || type == 3.0)\n";
        ss << "    {\n";

        if(pCur->GetType() == formula::svDoubleVectorRef &&
               pCur1->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            const formula::DoubleVectorRefToken* pDVR1 =
                static_cast<const formula::DoubleVectorRefToken *>(pCur1);

            size_t nCurWindowSize  = pDVR->GetRefRowSize();
            size_t nCurWindowSize1 = pDVR1->GetRefRowSize();
            ss << "        for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++)\n";
                ss << "        {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++)\n";
                ss << "        {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                ss << "        {\n";
            }
            else
            {
                ss << "0; i < " << nCurWindowSize << "; i++)\n";
                ss << "        {\n";
            }

            ss << "            arg1 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef(true) << ";\n";
            ss << "            if (isNan(arg1))\n";
            ss << "                continue;\n";
            ss << "            fSum1 += arg1;\n";
            ss << "            fSumSqr1 += arg1 * arg1;\n";
            ss << "            fCount1 += 1;\n";
            ss << "        }\n";

            ss << "        for (int i = ";
            if (!pDVR1->IsStartFixed() && pDVR1->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR1->GetArrayLength();
                ss << " && i < " << nCurWindowSize1  << "; i++)\n";
                ss << "        {\n";
            }
            else if (pDVR1->IsStartFixed() && !pDVR1->IsEndFixed())
            {
                ss << "0; i < " << pDVR1->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize1 << "; i++)\n";
                ss << "        {\n";
            }
            else if (!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR1->GetArrayLength();
                ss << " &&  i < " << nCurWindowSize1 << "; i++)\n";
                ss << "        {\n";
            }
            else
            {
                ss << "0; i < " << nCurWindowSize1 << "; i++)\n";
                ss << "        {\n";
            }
            ss << "            arg2 = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef(true) << ";\n";
            ss << "            if (isNan(arg2))\n";
            ss << "                continue;\n";
            ss << "            fSum2 += arg2;\n";
            ss << "            fSumSqr2 += arg2 * arg2;\n";
            ss << "            fCount2 += 1;\n";
            ss << "        }\n";
        }
        else
        {
            ss << "        return DBL_MAX;\n";
            ss << "    }\n";
            ss << "}\n";
            return ;
        }
        ss << "        if (fCount1 < 2.0 || fCount2 < 2.0)\n";
        ss << "            return DBL_MAX;\n";
        ss << "    }\n";
        ss << "    if(type == 3.0)\n";
        ss << "    {\n";
        ss << "        double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)\n";
        ss << "            /(fCount1-1.0)/fCount1;\n";
        ss << "        double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)\n";
        ss << "            /(fCount2-1.0)/fCount2;\n";
        ss << "        if (fS1 + fS2 == 0.0)\n";
        ss << "            return DBL_MAX;\n";
        ss << "        fT = fabs(fSum1/fCount1 - fSum2/fCount2)\n";
        ss << "             /sqrt(fS1+fS2);\n";
        ss << "        double c = fS1/(fS1+fS2);\n";
        ss << "        fF = 1.0/(c*c/(fCount1-1.0)+(1.0-c)*(1.0-c)\n";
        ss << "             /(fCount2-1.0));\n";
        ss << "    }\n";
        ss << "    if(type == 2.0)\n";
        ss << "    {\n";
        ss << "        double fS1 = (fSumSqr1 - fSum1*fSum1/fCount1)\n";
        ss << "             /(fCount1 - 1.0);\n";
        ss << "        double fS2 = (fSumSqr2 - fSum2*fSum2/fCount2)\n";
        ss << "             /(fCount2 - 1.0);\n";
        ss << "        fT = fabs( fSum1/fCount1 - fSum2/fCount2 )\n";
        ss << "            /sqrt( (fCount1-1.0)*fS1 + (fCount2-1.0)*fS2 )\n";
        ss << "            *sqrt( fCount1*fCount2*(fCount1+fCount2-2)\n";
        ss << "            /(fCount1+fCount2) );\n";
        ss << "        fF = fCount1 + fCount2 - 2;\n";
        ss << "    }\n";

        ss << "    double tdist=GetTDist(fT, fF);\n";
        ss << "    if (mode==1)\n";
        ss << "        return tdist;\n";
        ss << "    else\n";
        ss << "        return 2.0*tdist;\n";
        ss << "}\n";
    }
}
void OpVarP::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount == 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return vSum * pow(fCount,-1.0);\n";
    ss << "}\n";
}

void OpTDist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
    decls.insert(GetBetaDecl);
    funs.insert(GetBeta);
    decls.insert(GetLogBetaDecl);
    funs.insert(GetLogBeta);
    decls.insert(GetBetaDistPDFDecl);
    funs.insert(GetBetaDistPDF);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    funs.insert(lcl_GetBetaHelperContFrac);
    decls.insert(GetBetaDistDecl);
    funs.insert(GetBetaDist);
    decls.insert(GetTDistDecl);
    funs.insert(GetTDist);
}
void OpTDist::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x = 0.0;\n";
    ss << "    double fDF = 0.0;\n";
    ss << "    double fFlag = 0.0;\n";
    if(vSubArguments.size() != 3)
    {
        ss << "    return DBL_MAX;\n}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR0 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    if(gid0 < ";
            ss << tmpCurDVR0->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        x = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(x))\n";
            ss << "            x = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    x = " << tmpCur0->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur1->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR1 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur1);
            ss << "    if(gid0 < ";
            ss << tmpCurDVR1->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        fDF = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(fDF))\n";
            ss << "            fDF = 0.0;\n";
            ss << "        else\n";
            ss << "            fDF = floor(fDF);\n";
            ss << "    }\n";
        }
        else if(tmpCur1->GetType() == formula::svDouble)
        {
            ss << "    fDF = floor(convert_double(";
            ss << tmpCur1->GetDouble() << "));\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    fDF = floor(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
    }

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    assert(tmpCur2);
    if(ocPush == vSubArguments[2]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur2->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR2 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur2);
            ss << "    if(gid0 < ";
            ss << tmpCurDVR2->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        fFlag = ";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(fFlag))\n";
            ss << "            fFlag = 0.0;\n";
            ss << "        else\n";
            ss << "            fFlag = floor(fFlag);\n";
            ss << "    }\n";

        }
        else if(tmpCur2->GetType() == formula::svDouble)
        {
            ss << "    fFlag = floor(convert_double(";
            ss << tmpCur2->GetDouble() << "));\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    fFlag = floor(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ");\n";
    }
    ss << "    if(fDF < 1.0 || x < 0.0 || (fFlag != 1.0 && fFlag != 2.0))\n";
    ss << "        return DBL_MAX;\n";
    ss << "    double R = GetTDist(x, fDF);\n";
    ss << "    if (fFlag == 1.0)\n";
    ss << "        return R;\n";
    ss << "    else\n";
    ss << "        return 2.0 * R;\n";
    ss << "}\n";
}
void OpExponDist::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0,tmp0=0,tmp1=0,tmp2=0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double rx,rlambda,rkum;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "   rx = tmp0;\n";
    ss << "   rlambda = tmp1;\n";
    ss << "   rkum = tmp2;\n";
    ss <<"    if(rlambda <= 0.0)\n";
    ss <<"    {\n";
    ss <<"        tmp = -DBL_MAX;\n";
    ss <<"    }\n";
    ss <<"    else if(rkum == 0)\n";
    ss <<"    {\n";
    ss <<"        if(rx >= 0)\n";
    ss <<"            tmp = rlambda*exp(-rlambda*rx);\n";
    ss <<"        else\n";
    ss <<"            tmp = 0.0;\n";
    ss <<"    }\n";
    ss <<"    else\n";
    ss <<"    {\n";
    ss <<"        if(rx > 0)\n";
    ss <<"            tmp = 1.0 - exp(-rlambda*rx);\n";
    ss <<"        else\n";
    ss <<"            tmp = 0.0;\n";
    ss <<"    }\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpFdist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetFDistDecl);decls.insert(GetBetaDistDecl);
    decls.insert(GetBetaDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    decls.insert(GetBetaDistPDFDecl);
    decls.insert(GetLogBetaDecl);decls.insert(lcl_getLanczosSumDecl);
    decls.insert(fMachEpsDecl);
    funs.insert(GetFDist);funs.insert(GetBetaDist);
    funs.insert(GetBeta);
    funs.insert(lcl_GetBetaHelperContFrac);funs.insert(GetBetaDistPDF);
    funs.insert(GetLogBeta);
    funs.insert(lcl_getLanczosSum);
}
void OpFdist::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0,tmp0=0,tmp1=0,tmp2=0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double rF1,rF2,rX;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << " rX  = tmp0;\n";
    ss << " rF1 = floor(tmp1);\n";
    ss << " rF2 = floor(tmp2);\n";
    ss <<"    if (rX < 0.0 || rF1 < 1.0 || rF2 < 1.0 || rF1 >= 1.0E10 ||";
    ss <<"rF2 >= 1.0E10)\n";
    ss <<"    {\n";
    ss <<"        tmp = -DBL_MAX;\n";
    ss <<"    }\n";
    ss <<"    tmp = GetFDist(rX, rF1, rF2);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpStandard::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x = 0.0;\n";
    ss << "    double mu = 0.0;\n";
    ss << "    double sigma = 0.0;\n";
    if(vSubArguments.size() != 3)
    {
        ss << "    return DBL_MAX;\n}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR0 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    if (gid0 < " << tmpCurSVR0->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        x = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(x))\n";
            ss << "            x = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    x = " << tmpCur0->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur1->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR1 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur1);
            ss << "    if (gid0 < " << tmpCurSVR1->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        mu = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(mu))\n";
            ss << "            mu = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur1->GetType() == formula::svDouble)
        {
            ss << "    mu = " << tmpCur1->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    mu = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    }

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    assert(tmpCur2);
    if(ocPush == vSubArguments[2]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur2->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR2 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur2);
            ss << "    if (gid0 < " << tmpCurSVR2->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        sigma = ";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(sigma))\n";
            ss << "            sigma = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur2->GetType() == formula::svDouble)
        {
            ss << "    sigma = " << tmpCur2->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }
    }
    else
    {
        ss << "    sigma = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
    }

    ss << "    if(sigma <= 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return (x - mu)*pow(sigma,-1.0);\n";
    ss << "}";
}

void OpWeibull::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x = 0.0;\n";
    ss << "    double alpha = 0.0;\n";
    ss << "    double beta = 0.0;\n";
    ss << "    double kum = 0.0;\n";
    if(vSubArguments.size() != 4)
    {
        ss << "    return DBL_MAX;\n}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR0 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    if (gid0 < " << tmpCurSVR0->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        x = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(x))\n";
            ss << "            x = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    x = " << tmpCur0->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur1->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR1 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur1);
            ss << "    if (gid0 < " << tmpCurSVR1->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        alpha = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(alpha))\n";
            ss << "            alpha = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur1->GetType() == formula::svDouble)
        {
            ss << "    alpha = " << tmpCur1->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    alpha = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    }

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    assert(tmpCur2);
    if(ocPush == vSubArguments[2]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur2->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR2 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur2);
            ss << "    if (gid0 < " << tmpCurSVR2->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        beta = ";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(beta))\n";
            ss << "            beta = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur2->GetType() == formula::svDouble)
        {
            ss << "    beta = " << tmpCur2->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    beta = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << ";\n";
    }

    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    assert(tmpCur3);
    if(ocPush == vSubArguments[3]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur3->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurSVR3 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur3);
            ss << "    if (gid0 < " << tmpCurSVR3->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        kum = ";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if (isNan(kum))\n";
            ss << "            kum = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur3->GetType() == formula::svDouble)
        {
            ss << "    kum = " << tmpCur3->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    kum = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef() << ";\n";
    }

    ss << "    if(alpha <= 0.0 || beta <=0.0 || kum < 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else if(kum == 0.0)\n";
    ss << "    {\n";
    ss << "        return alpha*pow(pow(beta,alpha),-1.0)*pow(x,alpha-1.0)";
    ss << "*exp(-pow(x*pow(beta,-1.0),alpha));\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "        return 1.0-exp(-pow(x*pow(beta,-1.0),alpha));\n";
    ss << "}\n";
}

void OpSkew::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";
        }

        if(i == 0)
        {
            ss << "    if(fCount <= 2.0)\n";
            ss << "        return DBL_MAX;\n";
            ss << "    else\n";
            ss << "        fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    double fStdDev = sqrt(vSum * pow(fCount - 1.0,-1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return DBL_MAX;\n";
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "        xcube = xcube + dx * dx * dx;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "            xcube = xcube + dx * dx * dx;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "    xcube = xcube + dx * dx * dx;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
            ss << "    xcube = xcube + dx * dx * dx;\n";
        }
    }
    ss << "    return ((xcube * fCount) * pow(fCount - 1.0,-1.0))";
    ss << " * pow(fCount - 2.0,-1.0);\n";
    ss << "}\n";
}

void OpSkewp::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";
        }

        if(i == 0)
        {
            ss << "    if(fCount <= 2.0)\n";
            ss << "        return DBL_MAX;\n";
            ss << "    else\n";
            ss << "        fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    double fStdDev = sqrt(vSum * pow(fCount,-1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return DBL_MAX;\n";
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "        xcube = xcube + dx * dx * dx;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "            xcube = xcube + dx * dx * dx;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
                ss << "    xcube = xcube + dx * dx * dx;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    dx = (arg - fMean) * pow(fStdDev,-1.0);\n";
            ss << "    xcube = xcube + dx * dx * dx;\n";
        }
    }
    ss << "    return xcube * pow(fCount,-1.0);\n";
    ss << "}\n";
}

void OpTInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
    decls.insert(GetBetaDecl);
    funs.insert(GetBeta);
    decls.insert(GetLogBetaDecl);
    funs.insert(GetLogBeta);
    decls.insert(GetBetaDistPDFDecl);
    funs.insert(GetBetaDistPDF);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    funs.insert(lcl_GetBetaHelperContFrac);
    decls.insert(GetBetaDistDecl);
    funs.insert(GetBetaDist);
    decls.insert(GetTDistDecl);
    funs.insert(GetTDist);
    decls.insert(GetValueDecl);
    funs.insert(GetValue);
    decls.insert(lcl_HasChangeOfSignDecl);
    funs.insert(lcl_HasChangeOfSign);
    decls.insert(lcl_IterateInverseDecl);
    funs.insert(lcl_IterateInverse);
}

void OpTInv::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x = 0.0;\n";
    ss << "    double fDF = 0.0;\n";
    if(vSubArguments.size() != 2)
    {
        ss << "    return DBL_MAX;\n}\n";
        return ;
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR0 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    if(gid0 < ";
            ss << tmpCurDVR0->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        x = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(x))\n";
            ss << "            x = 0.0;\n";
            ss << "    }\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    x = " << tmpCur0->GetDouble() << ";\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    x = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur1);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur1->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR1 =
                static_cast<const formula::SingleVectorRefToken *>(tmpCur1);
            ss << "    if(gid0 < ";
            ss << tmpCurDVR1->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        fDF = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(fDF))\n";
            ss << "            fDF = 0.0;\n";
            ss << "        else\n";
            ss << "            fDF = floor(fDF);\n";
            ss << "    }\n";
        }
        else if(tmpCur1->GetType() == formula::svDouble)
        {
            ss << "    fDF = floor(convert_double(";
            ss << tmpCur1->GetDouble() << "));\n";
        }
        else
        {
            ss << "    return DBL_MAX;\n}\n";
            return ;
        }
    }
    else
    {
        ss << "    fDF = floor(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
    }
    ss << "    if (x > 1.0||fDF < 1.0 || fDF > 1.0E10 || x <= 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    bool bConvError;\n";
    ss << "    double fVal = lcl_IterateInverse(\n";
    ss << "        fDF*0.5, fDF, &bConvError,x,fDF );\n";
    ss << "    if (bConvError)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    return fVal;\n";
    ss << "}\n";
}

void OpStDev::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum * pow(fCount - 1.0,-1.0));\n";
    ss << "}\n";
}

void OpStDevP::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        fSum += arg;\n";
                ss << "        fCount += 1.0;\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            fSum += arg;\n";
                ss << "            fCount += 1.0;\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount += 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount += 1.0;\n";

        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (isNan(arg))\n";
                ss << "            continue;\n";
                ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "    }\n";
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        if (!isNan(arg))\n";
                ss << "        {\n";
                ss << "            vSum += (arg - fMean) * (arg - fMean);\n";
                ss << "        }\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount == 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum * pow(fCount,-1.0));\n";
    ss << "}\n";
}

void OpSlope::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fMeanX = 0.0;\n";
    ss << "    double fMeanY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fSumSqrDeltaX = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double argX = 0.0;\n";
    ss << "    double argY = 0.0;\n";
    if(vSubArguments.size() != 2)
    {
        ss << "    return NAN;\n";
        ss << "}\n";
        return ;
    }
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
    FormulaToken *pCur1 = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    assert(pCur1);
    if (pCur->GetType() == formula::svDoubleVectorRef&&
        pCur1->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken *>(pCur);
        const formula::DoubleVectorRefToken* pDVR1 =
            static_cast<const formula::DoubleVectorRefToken *>(pCur1);

        size_t nCurWindowSize = pDVR->GetRefRowSize();
        size_t nCurWindowSize1 = pDVR1->GetRefRowSize();
        size_t arrayLength = pDVR->GetArrayLength()<
               pDVR1->GetArrayLength() ? pDVR->GetArrayLength():
                    pDVR1->GetArrayLength();
        if(nCurWindowSize != nCurWindowSize1)
        {
            ss << "    return NAN;\n";
            ss << "}\n";
            return ;
        }
        ss << "    for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength ;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "    {\n";
        }
        else
        {
            ss << "0; i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        break;\n";
            ss << "    }";
            ss << "    return NAN;\n";
            ss << "}\n";
            return ;
        }

        ss << "        argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        if (isNan(argX) || isNan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 1.0)\n";
        ss << "        return CreateDoubleError(errNoValue);\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX * pow(fCount,-1.0);\n";
        ss << "        fMeanY = fSumY * pow(fCount,-1.0);\n";

        ss << "        for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "        {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength ;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "        {\n";
        }

        ss << "            argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (isNan(argX) || isNan(argY))\n";
        ss << "                 continue;\n";
        ss << "            fSumDeltaXDeltaY += (argX-fMeanX)*(argY-fMeanY);\n";
        ss << "            fSumSqrDeltaX += (argX-fMeanX) * (argX-fMeanX);\n";
        ss << "        }\n";
        ss << "        if(fSumSqrDeltaX == 0.0)\n";
        ss << "            return CreateDoubleError(errDivisionByZero);\n";
        ss << "        else\n";
        ss << "        {\n";
        ss << "            return fSumDeltaXDeltaY*pow(fSumSqrDeltaX,-1.0);\n";
        ss << "        }\n";
        ss << "    }\n";
        ss << "}\n";
    }
    else
    {
        ss << "    return NAN;\n";
        ss << "}\n";
    }
}
void OpSTEYX::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fMeanX = 0.0;\n";
    ss << "    double fMeanY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fSumSqrDeltaX = 0.0;\n";
    ss << "    double fSumSqrDeltaY = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double argX = 0.0;\n";
    ss << "    double argY = 0.0;\n";
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
    FormulaToken *pCur1 = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    assert(pCur1);
    if (pCur->GetType() == formula::svDoubleVectorRef&&
        pCur1->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken *>(pCur);
        const formula::DoubleVectorRefToken* pDVR1 =
            static_cast<const formula::DoubleVectorRefToken *>(pCur1);
        size_t nCurWindowSize = pDVR->GetRefRowSize();
        size_t nCurWindowSize1 = pDVR1->GetRefRowSize();
        size_t arrayLength = pDVR->GetArrayLength()<
               pDVR1->GetArrayLength() ? pDVR->GetArrayLength():
                    pDVR1->GetArrayLength();
        if(nCurWindowSize != nCurWindowSize1)
        {
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }
        ss << "    for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "    {\n";
        }
        else
        {
            ss << "0; i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        break;\n";
            ss << "    }";
            ss << "    return DBL_MAX;\n";
            ss << "}\n";
            return ;
        }

        ss << "        argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        if (isNan(argX) || isNan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 3.0)\n";
        ss << "        return DBL_MAX;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX * pow(fCount,-1.0);\n";
        ss << "        fMeanY = fSumY * pow(fCount,-1.0);\n";

        ss << "        for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "        {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength ;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())&&
            (!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "        {\n";
        }

        ss << "            argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (isNan(argX)||isNan(argY))\n";
        ss << "                continue;\n";
        ss << "            fSumDeltaXDeltaY +=(argX-fMeanX)*(argY-fMeanY);\n";
        ss << "            fSumSqrDeltaX += (argX-fMeanX)*(argX-fMeanX);\n";
        ss << "            fSumSqrDeltaY += (argY-fMeanY)*(argY-fMeanY);\n";
        ss << "        }\n";
        ss << "        if(fSumSqrDeltaX == 0.0)\n";
        ss << "            return DBL_MAX;\n";
        ss << "        else\n";
        ss << "        {\n";
        ss << "            return sqrt((fSumSqrDeltaY - fSumDeltaXDeltaY * \n";
        ss << "                   fSumDeltaXDeltaY*pow(fSumSqrDeltaX,-1.0))\n";
        ss << "                   *pow(fCount - 2.0,-1.0));\n";
        ss << "        }\n";
        ss << "    }\n";
        ss << "}\n";
    }
    else
    {
        ss << "    return DBL_MAX;\n";
        ss << "}\n";
    }
}
void OpFisher::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0;\n";
    if(vSubArguments.size() != 1)
    {
        ss << "    return DBL_MAX;\n";
        return ;
    }
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    if (pCur->GetType() == formula::svDoubleVectorRef)
    {
        ss << "    return DBL_MAX;\n";
        return ;
    }
    else if (pCur->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
        ss << "    arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss<< "    if(isNan(arg0)||(gid0>=";
        ss<<pSVR->GetArrayLength();
        ss<<"))\n";
        ss<<"        arg0 = 0;\n";
    }
    else if (pCur->GetType() == formula::svDouble)
    {
        ss << "    arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss << "    if(isNan(arg0))\n";
        ss << "        return DBL_MAX;\n";
    }
    ss << "    if (fabs(arg0) >= 1.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    double tmp=0.5*log((1+arg0)*pow((1-arg0),-1));\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpFisherInv::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR = static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
    ss << "double tmp=tanh(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}

void OpGamma::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpCorrelfor GPU calculating.
        throw Unhandled();
    const formula::DoubleVectorRefToken* pCurDVRX =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVRY =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pCurDVRX->GetRefRowSize() != pCurDVRY->GetRefRowSize() )
         throw Unhandled();

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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

    size_t nCurWindowSizeX = pCurDVRY->GetRefRowSize();

        ss << "for (int i = ";
        if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        }
        else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
            ss << "++cnt;\n\t\t";
            ss << "vXSum += arg0;\n\t\t";
            ss << "vYSum += arg1;\n\t";
            ss << "}\n\t";
        } else {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
            ss << "arg0 = 0.0;\n\t\t\t";
            ss << "arg1 = 0.0;\n\t\t\t";
            ss << "--cnt;\n\t\t";
            ss << "}\n\t\t";
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
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
            ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
            ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        } else {
            ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
            ss << "arg0 = " << vSubArguments[0]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "arg1 = " << vSubArguments[1]
                ->GenSlidingWindowDeclRef(true) << ";\n\t\t\t";
            ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
            ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
            ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
            ss << "arg0 = vXMean;\n\t\t\t\t";
            ss << "arg1 = vYMean;\n\t\t\t";
            ss << "}\n\t\t\t";
            ss << "vXSum += ((arg0 - vXMean)*(arg0 - vXMean));\n\t\t\t";
            ss << "vYSum += ((arg1 - vYMean)*(arg1 - vYMean));\n\t\t\t";
            ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
            ss << "}\n\t\t";
        }

        ss << "if(vXSum == 0.0 || vYSum == 0.0) {\n\t\t\t";
        ss << "return NAN;\n\t\t";
        ss << "}\n\t\t";
        ss << "else {\n\t\t\t";
        ss << "return vSum/pow(vXSum*vYSum, 0.5);\n\t\t";
        ss << "}\n\t";
        ss << "}\n";
        ss << "}";
}

void OpNegbinomdist::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n\t";
    ss << "double f,s,p,tmp0,tmp1,tmp2;\n";
    ss << " int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << " p = tmp2;\n";
    ss << " s = tmp1;\n";
    ss << " f = tmp0;\n";
    ss << " double q = 1.0 - p;\n\t";
    ss << " double fFactor = pow(p,s);\n\t";
    ss << " for(int i=0; i<f; i++)\n\t";
    ss << " {\n\t";
    ss << "  fFactor *= ((double)i+s)*pow(((double)i+1.0),-1.0)/pow(q,-1);\n";
    ss << " }\n\t";
    ss << " double temp=fFactor;\n\t";
    ss << " return temp;\n";
    ss << "}\n";
}

void OpPearson::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpPearson for GPU calculating.
        throw Unhandled();
    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVRY =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pDVR->GetRefRowSize() != pCurDVRY->GetRefRowSize() )
         throw Unhandled();

    size_t nCurWindowSize = pDVR->GetRefRowSize();

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fInx;\n";
    ss << "    double fIny;\n";
    ss << "for (int i = ";
    if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
        ss << "gid0; i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize  << "; i++){\n";
    } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
        ss << "0; i < " << pDVR->GetArrayLength();
        ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
    } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
        ss << "0; i + gid0 < " << pDVR->GetArrayLength();
        ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
    }
    else {
        ss << "0; i < "<< nCurWindowSize << "; i++){\n";
    }
    ss << "          fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << ";\n";
    ss << "          fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef(true);
    ss << "  ;\n";
    ss << " if(isNan(fInx)||isNan(fIny)){fInx=0.0;fIny=0.0;fCount = fCount-1;}\n";
    ss << "       fSumX += fInx;\n";
    ss << "       fSumY += fIny;\n";
    ss << "       fCount = fCount + 1;\n";
    ss << "     }\n";
    ss << "       double fMeanX = fSumX / fCount;\n";
    ss << "       double fMeanY = fSumY / fCount;\n";
    ss << "       fSumX = 0.0;\n";
    ss << "       fSumY = 0.0;\n";
    ss << "for (int i = ";
    if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
        ss << "gid0; i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize  << "; i++){\n";
    } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
        ss << "0; i < " << pDVR->GetArrayLength();
        ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
    } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
         ss << "0; i + gid0 < " << pDVR->GetArrayLength();
         ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
    }
    else {
         ss << "0; i < "<< nCurWindowSize << "; i++){\n";
    }
    ss << "           fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << " ;\n";
    ss << "           fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef(true);
    ss << " ;\n";
    ss << " if(isNan(fInx)||isNan(fIny)){fInx=0.0;fIny=0.0;}\n";
    ss << "           fSumDeltaXDeltaY += (fInx - fMeanX) * (fIny - fMeanY);\n";
    ss << "           fSumX += pow(fInx - fMeanX,2);\n";
    ss << "           fSumY += pow(fIny - fMeanY,2);\n";
    ss << "       }\n";
    ss << "      double tmp = ( fSumDeltaXDeltaY / ";
    ss << "sqrt( fSumX * fSumY));\n\t";
    ss << "      if (isnan(tmp))\n";
    ss << "          return CreateDoubleError(errNoValue);\n";
    ss << "      return tmp;\n";
    ss << "}\n";
}

void OpGammaLn::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
    ss << "double tmp=lgamma(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}\n";
}
void OpGauss::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(taylorDecl);decls.insert(phiDecl);
    decls.insert(gaussDecl);
    funs.insert(taylor);funs.insert(phi);
    funs.insert(gauss);
}

void OpGauss::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0;\n";
    if(vSubArguments.size() != 1)
    {
        ss << "    return DBL_MAX;\n";
        return ;
    }
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    if (pCur->GetType() == formula::svDoubleVectorRef)
    {
        ss << "    return DBL_MAX;\n";
        return ;
    }
    else if (pCur->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
        ss << "    arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss<< "    if(isNan(arg0)||(gid0>=";
        ss<<pSVR->GetArrayLength();
        ss<<"))\n";
        ss<<"        arg0 = 0;\n";
    }
    else if (pCur->GetType() == formula::svDouble)
    {
        ss << "    arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss << "    if(isNan(arg0))\n";
        ss << "        return DBL_MAX;\n";
    }
    ss << "    double tmp=gauss(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpGeoMean::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "__kernel void ";
    ss << "GeoMean_reduction(  ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ", __global double *result)\n";
    ss << "{\n";
    ss << "    double tmp =0;\n";
    ss << "    int count = 0;\n";
    ss << "    int i ;\n";
    GenTmpVariables(ss,vSubArguments);
    ss << "    double current_sum = 0.0;\n";
    ss << "    int windowSize;\n";
    ss << "    int arrayLength;\n";
    ss << "    int current_count = 0;\n";
    ss << "    int writePos = get_group_id(1);\n";
    ss << "    int lidx = get_local_id(0);\n";
    ss << "    __local double shm_buf[256];\n";
    ss << "    __local int count_buf[256];\n";
    ss << "    int loop;\n";
    ss << "    int offset;\n";
    ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";

    for(size_t i=0;i<vSubArguments.size();i++)
    {
        assert(vSubArguments[i]->GetFormulaToken());

        if(vSubArguments[i]->GetFormulaToken()->GetType() ==
        formula::svDoubleVectorRef)
        {
            FormulaToken *tmpCur = vSubArguments[i]->GetFormulaToken();
                const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
                     formula::DoubleVectorRefToken *>(tmpCur);
                size_t nCurWindowSize = pCurDVR->GetArrayLength() <
                pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
                pCurDVR->GetRefRowSize() ;

            if (pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
                ss << "    offset = 0;\n";
            else if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                ss << "    offset = get_group_id(1);\n";
            else
                throw Unhandled();
            ss << "    windowSize = ";
            ss << nCurWindowSize;
            ss << ";\n";
            ss << "    arrayLength = ";
            ss << pCurDVR->GetArrayLength();
            ss << ";\n";
            ss << "    loop = arrayLength/512 + 1;\n";
            ss << "    for (int l=0; l<loop; l++){\n";
            ss << "        tmp = 0.0;\n";
            ss << "        count = 0;\n";
            ss << "        int loopOffset = l*512;\n";
            ss << "        int p1 = loopOffset + lidx + offset, p2 = p1 + 256;\n";
            ss << "        if (p2 < min(offset + windowSize, arrayLength)) {\n";
            ss << "            tmp0 = 0.0;\n";
            std::string p1 = "p1";
            std::string p2 = "p2";

            ss << "        tmp0 =";
            vSubArguments[i]->GenDeclRef(ss);
            ss << "["<<p1.c_str()<<"];\n";
            ss << "        if(!isNan(tmp0))\n";
            ss << "       {\n";
            ss << "           tmp += log(tmp0);\n";
            ss << "           count++;\n";
            ss << "       }\n";

            ss << "        tmp0 =";
            vSubArguments[i]->GenDeclRef(ss);
            ss << "["<<p2.c_str()<<"];\n";
            ss << "        if(!isNan(tmp0))\n";
            ss << "       {\n";
            ss << "           tmp += log(tmp0);\n";
            ss << "           count++;\n";
            ss << "       }\n";

            ss << "        }\n";
            ss << "        else if (p1 < min(arrayLength, offset + windowSize)) {\n";

            ss << "        tmp0 =";
            vSubArguments[i]->GenDeclRef(ss);
            ss << "["<<p1.c_str()<<"];\n";
            ss << "        if(!isNan(tmp0))\n";
            ss << "        {\n";
            ss << "            tmp += log(tmp0);\n";
            ss << "            count++;\n";
            ss << "        }\n";

            ss << "        }\n";
            ss << "        shm_buf[lidx] = tmp;\n";
            ss << "        count_buf[lidx] = count;\n";
            ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";

            ss << "        for (int i = 128; i >0; i/=2) {\n";
            ss << "            if (lidx < i)\n";
            ss << "            {\n";
            ss << "                shm_buf[lidx] += shm_buf[lidx + i];\n";
            ss << "                count_buf[lidx] += count_buf[lidx + i];\n";
            ss << "            }\n";
            ss << "            barrier(CLK_LOCAL_MEM_FENCE);\n";
            ss << "        }\n";
            ss << "        if (lidx == 0)\n";
            ss << "        {\n";
            ss << "            current_sum += shm_buf[0];\n";
            ss << "            current_count += count_buf[0];\n";
            ss << "        }\n";
             //  ss << "if(writePos == 14 && lidx ==0)\n";
            //ss <<"printf(\"\\n********************sum is  is %f, count is%d\",current_sum,current_count);\n";
            ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
            ss << "    }\n";
        }else
        {
            ss << "    if (lidx == 0)\n";
            ss << "    {\n";
            ss << "        tmp0 =";
            if(vSubArguments[i]->GetFormulaToken()->GetType() ==
     formula::svSingleVectorRef)
            {
                vSubArguments[i]->GenDeclRef(ss);
                ss << "[writePos];\n";
            }
            else
            {
                vSubArguments[i]->GenDeclRef(ss);
                ss <<";\n";
                //ss <<"printf(\"\\n********************tmp0 is %f\",tmp0);\n";
            }
            ss << "        if(!isNan(tmp0))\n";
            ss << "       {\n";
            ss << "           current_sum += log(tmp0);\n";
            ss << "           current_count++;\n";
            ss << "       }\n";
            ss << "    }\n";
        }
    }

    ss << "    if (lidx == 0)\n";
    ss << "        result[writePos] = exp(current_sum/current_count);\n";
    ss << "}\n";

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n    {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    tmp =";
    vSubArguments[0]->GenDeclRef(ss);
    ss << "[gid0];\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpHarMean::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss <<"{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double nVal=0.0;\n";
    ss << "    double tmp = 0;\n";
    ss << "    int length;\n";
    ss << "    int totallength=0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    length="<<nCurWindowSize;
            ss << ";\n";
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            length--;\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        nVal += (1.0 *pow(";
            ss << " arg"<<i<<",-1));\n";
            ss << "    }\n";
            ss << "    totallength +=length;\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss << "        nVal += (1.0 * pow( tmp,-1));\n";
            ss << "        totallength +=1;\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss << "    nVal += (1.0 *pow( tmp,-1));\n";
           ss << "    totallength +=1;\n";
        }
        else
        {
            ss << "    return DBL_MIN;\n";
        }
    }
    ss << "    tmp = totallength*pow(nVal,-1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpConfidence::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpConfidence::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
    ss << "    double tmp0,tmp1,tmp2;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    alpha = tmp0;\n";
    ss << "    sigma = tmp1;\n";
    ss << "    size = tmp2;\n";
    ss << "    double rn = floor(size);\n";
    ss << "    if(sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0";
    ss << "|| rn < 1.0)\n";
    ss << "        tmp = -DBL_MAX;\n";
    ss << "    else\n";
    ss << "        tmp = gaussinv(1.0 - alpha * pow(2.0,-1.0)) * sigma ";
    ss << "* pow(sqrt( rn ),-1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCritBinom::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(MinDecl);
    funs.insert("");
}

void OpCritBinom::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
    ss << "    double tmp0 = 0.0,tmp1 = 0.0,tmp2 = 0.0;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    n = tmp0;\n";
    ss << "    p = tmp1;\n";
    ss << "    alpha = tmp2;\n";
    ss << "    double rn = floor(n);\n";
    ss << "    if (rn < 0.0 || alpha <= 0.0 || alpha >= 1.0 || p < 0.0";
    ss << " || p > 1.0)\n";
    ss << "        tmp = -DBL_MIN;\n";
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
    ss << " fFactor *= (rn - i) * pow((double)(i + 1),-1.0) *";
    ss << " rq * pow(p, -1.0);\n";
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
    ss << " fFactor *= (rn - i) * pow((double)(i + 1), -1.0) *";
    ss << " p * pow(rq, -1.0);\n";
    ss << "                fSum += fFactor;\n";
    ss << "            }\n";
    ss << "            tmp = (i);\n";
    ss << "        }\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpRsq::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpRsq for GPU calculating.
        throw Unhandled();
    const formula::DoubleVectorRefToken* pCurDVR1 =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVR2 =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pCurDVR1->GetRefRowSize() != pCurDVR2->GetRefRowSize() )
         throw Unhandled();

    size_t nCurWindowSize = pCurDVR1->GetRefRowSize();

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fInx;\n";
    ss << "    double fIny;\n";
    ss << "    double tmp0,tmp1;\n";

    ss <<"\n";

    ss << "   for(int i=0; i<"<<nCurWindowSize<<"; i++)\n";
    ss << "   {\n";
    ss << "     if(isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << "))\n";
    ss << "         fInx = 0;\n";
    ss << "     else\n";
    ss << "        fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "      if(isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef(true);
    ss << "))\n";
    ss << "          fIny = 0;\n";
    ss << "      else\n";
    ss << "        fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << " ;\n";
    ss << "      fSumX += fInx;\n";
    ss << "      fSumY += fIny;\n";
    ss << "      fCount = fCount + 1;\n";
    ss << "    }\n";
    ss << "    double fMeanX = fSumX / fCount;\n";
    ss << "    double fMeanY = fSumY / fCount;\n";
    ss << "    fSumX = 0.0;\n";
    ss << "    fSumY = 0.0;\n";
    ss << "    for(int i=0; i<"<<nCurWindowSize<<"; i++)\n";
    ss << "    {\n";
    ss << "     if(isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << "))\n";
    ss << "         fInx = 0;\n";
    ss << "     else\n";
    ss << "        fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "      if(isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "          fIny = 0;\n";
    ss << "      else\n";
    ss << "        fIny = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << " ;\n";
    ss << "        fSumDeltaXDeltaY += (fInx - fMeanX) * (fIny - fMeanY);\n";
    ss << "        fSumX    += pow(fInx - fMeanX,2);\n";
    ss << "        fSumY    += pow(fIny - fMeanY,2);\n";
    ss << "    }\n";
    ss << "    double tmp = pow( fSumDeltaXDeltaY,2) / (fSumX * fSumY);\n";
    ss << "    return tmp ;\n";
    ss << "}\n";
}

void OpChiInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(fBigInvDecl);
    funs.insert("");
    decls.insert(fHalfMachEpsDecl);
    funs.insert("");
    decls.insert(lcl_IterateInverseChiInvDecl);
    funs.insert(lcl_IterateInverseChiInv);
    decls.insert(GetChiDistDecl);
    funs.insert(GetChiDist);
    decls.insert(lcl_HasChangeOfSignDecl);
    funs.insert(lcl_HasChangeOfSign);
    decls.insert(GetUpRegIGammaDecl);
    funs.insert(GetUpRegIGamma);
    decls.insert(GetGammaContFractionDecl);
    funs.insert(GetGammaContFraction);
    decls.insert(GetGammaSeriesDecl);
    funs.insert(GetGammaSeries);
}
void OpChiInv::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double tmp0,tmp1,tmp;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
                } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
                } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
                }
                else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
                }
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
            }
            else if (pCur->GetType() == formula::svDouble)
            {
                ss << "{\n";
            }
            else
            {
            }
            if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
            {
                ss << "if (isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "    tmp"<<i<<"= 0;\n";
                ss << "else\n";
                ss <<"tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n}\n";
            }
            else
            {
               ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
               ss << ";\n";
            }
            }
    ss << "    tmp1 = floor(tmp1);";
    ss << "    if (tmp1 < 1.0 || tmp0 <= 0.0 || tmp0 > 1.0 )\n";
    ss << "    {\n";
    ss << "        return DBL_MIN;\n";
    ss << "    }\n";
    ss << "    bool bConvError;\n";
    ss << "    double fVal = lcl_IterateInverseChiInv";
    ss << "(tmp0, tmp1, tmp1*0.5, tmp1, &bConvError);\n";
    ss << "    if(bConvError)\n";
    ss << "        return DBL_MIN;\n";
    ss << "    return fVal;\n";
    ss << "}\n";
}
void OpNormdist::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x,mue,sigma,c;\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0,tmp1,tmp2,tmp3;\n";
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "x = tmp0;\n";
    ss << "mue = tmp1;\n";
    ss << "sigma = tmp2;\n";
    ss << "c = tmp3;\n";
    ss << "double mid,tmp;\n";
    ss << "mid = (x - mue)/sigma;\n";
    ss << "if(c)\n";
    ss << "    tmp = 0.5 *erfc(-mid * 0.7071067811865475);\n";
    ss << "else \n";
    ss <<"     tmp=(0.39894228040143268*exp(-pow(mid,2)/2.0))/sigma;\n";
    ss << "return tmp;\n";
    ss << "}\n";
}
void OpNormsdist::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x = 0,tmp0 = 0;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    x = tmp0;\n";
    ss << "    double tmp = 0.5 * erfc((-1)*x * 0.7071067811865475);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpPermut::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
       if (i)
         ss << ",";
       vSubArguments[i]->GenSlidingWindowDecl(ss);
     }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double inA;\n";
    ss <<"    double inB;\n";
    ss <<"    double tmp0,tmp1;\n";
    ss <<"    double tmp = 1 ;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";

        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "      inA = tmp0;\n";
    ss << "      inB = tmp1;\n";
    ss << "      for( int i =0; i<inB; i++)\n";
    ss << "      {\n";
    ss << "        tmp *= inA ;\n";
    ss << "        inA = inA - 1.0;\n";
    ss << "      }\n";
    ss << "      return tmp;\n";
    ss << "}\n";
}
void OpPermutationA::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
       if (i)
         ss << ",";
       vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double inA;\n";
    ss <<"    double inB;\n";
    ss <<"    double tmp = 1.0;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss << "int buffer_fIna_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_fInb_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss << "    if((gid0)>=buffer_fIna_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "    inA = 0;\nelse \n";
    ss << "        inA = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "if((gid0)>=buffer_fInb_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "inB = 0;\nelse \n";
    ss << "    inB = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << " for(int i=0; i<inB; i++)\n";
    ss << " {\n";
    ss << "     tmp *= inA;\n";
    ss << " }\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpPhi::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x,tmp0;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    x = tmp0;\n";
    ss << "    double tmp = 0.39894228040143268 * exp((-1)*pow(x,2) / 2.0);\n";
    ss << "     return tmp;\n";
    ss << "}\n";
}

void OpNorminv::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss <<"    double q,t,z;\n";
    ss <<"    double x,mue,sigma;\n";
    ss <<"    double tmp0,tmp1,tmp2;\n";
    ss <<"    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss <<"    x = tmp0;\n";
    ss <<"    mue = tmp1;\n";
    ss <<"    sigma = tmp2;\n";
    ss <<"    q = x -0.5;\n";
    ss <<"    if(fabs(q)<=.425)\n";
    ss <<"   {\n";
    ss <<"        t=0.180625-pow(q,2);\n";
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
    ss << "z = q < 0.0 ? (-1)*z : z;\n";
    ss<<"}\n";
    ss<<"double tmp =  z*sigma + mue;\n";
    ss<<"return tmp;\n";
    ss<<"}\n";
}
void OpNormsinv:: GenSlidingWindowFunction
    (std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double q,t,z,x,tmp0;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss <<"    x = tmp0;\n";
    ss <<"    q = x -0.5;\n";
    ss <<"    if(fabs(q)<=.425)\n";
    ss <<"    {\n";
    ss <<"        t=0.180625-pow(q,2);\n";
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
    ss << "z = q < 0.0 ? (-1)*z : z;\n";
    ss <<"}\n";
    ss <<"if (isnan(z))\n";
    ss <<"    return CreateDoubleError(errNoValue);\n";
    ss <<"return z;\n";
    ss <<"}\n";
}
void OpMedian::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
            ss << "gid0; endFlag = gid0+"<< nCurWindowSize <<";\n";
        }
        else
        {
            ss<<"startFlag=gid0;endFlag=gid0;\n";
        }
    }
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::DoubleVectorRefToken *>(tmpCur0);
    ss << "int buffer_fIna_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<<"if((i+gid0)>=buffer_fIna_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"    dataIna = 0;\n";
    ss << "    int nSize =endFlag- startFlag ;\n";
    ss << "    if (nSize & 1)\n";
    ss << "    {\n";
    ss << "        tmp = "<<vSubArguments[0]->GetName();
    ss << "        [startFlag+nSize/2];\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        tmp =("<<vSubArguments[0]->GetName();
    ss << "        [startFlag+nSize/2]+";
    ss <<          vSubArguments[0]->GetName();
    ss << "        [startFlag+nSize/2-1])/2;\n";
    ss << "    }\n";
    ss <<"     return tmp;\n";
    ss << "}\n";
}
void OpKurt:: GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss <<"{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double length;\n";
    ss << "    double totallength=0;\n";
    ss << "    double tmp = 0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    length="<<nCurWindowSize;
            ss << ";\n";
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            length-=1.0;\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        fSum +=  arg"<<i<<";\n";
            ss << "    }\n";
            ss << "    totallength +=length;\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss << "        fSum += tmp;\n";
            ss << "        totallength +=1;\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss << "    fSum += tmp;\n";
           ss << "    totallength +=1;\n";
        }
        else
        {
            ss << "    return DBL_MIN;\n";
        }
    }
    ss << "    double fMean = fSum * pow(totallength,-1);\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        vSum +=  (arg"<<i<<"-fMean)*(arg"<<i<<"-fMean);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss << "        vSum += (tmp-fMean)*(tmp-fMean);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss << "    vSum += (tmp-fMean)*(tmp-fMean);\n";
        }
    }
    ss << "    double fStdDev = sqrt(vSum / (totallength - 1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xpower4 = 0.0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss<< "        dx = (arg"<<i<<" -fMean) / fStdDev;\n";
            ss<< "        xpower4 = xpower4 + (dx * dx * dx * dx);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss<< "        dx = (tmp -fMean) / fStdDev;\n";
            ss<< "        xpower4 = xpower4 + (dx * dx * dx * dx);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss<< "        dx = (tmp -fMean) / fStdDev;\n";
           ss<< "        xpower4 = xpower4 + (dx * dx * dx * dx);\n";
        }
    }
    ss<< "    double k_d = (totallength - 2.0) * (totallength - 3.0);\n";
    ss<< "    double k_l = totallength * (totallength + 1.0) /";
    ss<< "((totallength - 1.0) * k_d);\n";
    ss<< "    double k_t = 3.0 * (totallength - 1.0) * ";
    ss<< "(totallength - 1.0) / k_d;\n";
    ss<< "    tmp = xpower4 * k_l - k_t;\n";
    ss<< "    return tmp;\n";
    ss << "}";
}

void OpIntercept::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fMeanX = 0.0;\n";
    ss << "    double fMeanY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fSumSqrDeltaX = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double argX = 0.0;\n";
    ss << "    double argY = 0.0;\n";
    if(vSubArguments.size() != 2)
    {
        ss << "    return NAN;\n";
        ss << "}\n";
        return ;
    }
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
    FormulaToken *pCur1 = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    assert(pCur1);
    if (pCur->GetType() == formula::svDoubleVectorRef&&
        pCur1->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken *>(pCur);
        const formula::DoubleVectorRefToken* pDVR1 =
            static_cast<const formula::DoubleVectorRefToken *>(pCur1);

        size_t nCurWindowSize = pDVR->GetRefRowSize();
        size_t nCurWindowSize1 = pDVR1->GetRefRowSize();
        size_t arrayLength = pDVR->GetArrayLength()<
               pDVR1->GetArrayLength() ? pDVR->GetArrayLength():
                    pDVR1->GetArrayLength();
        if(nCurWindowSize != nCurWindowSize1)
        {
            ss << "    return NAN;\n";
            ss << "}\n";
            return ;
        }
        ss << "    for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength ;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
        }
        else if ((pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "    {\n";
        }
        else
        {
            ss << "0; i < " << nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        break;\n";
            ss << "    }";
            ss << "    return NAN;\n";
            ss << "}\n";
            return ;
        }

        ss << "        argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        if (isNan(argX) || isNan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 1.0)\n";
        ss << "        return NAN;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX * pow(fCount,-1.0);\n";
        ss << "        fMeanY = fSumY * pow(fCount,-1.0);\n";

        ss << "        for (int i = ";
        if ((!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && pDVR1->IsEndFixed()))
        {
            ss << "gid0; i < " << arrayLength;
            ss << " && i < " << nCurWindowSize  << "; i++)\n";
            ss << "        {\n";
        }
        else if ((pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i < " << arrayLength ;
            ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else if ((!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            &&(!pDVR1->IsStartFixed() && !pDVR1->IsEndFixed()))
        {
            ss << "0; i + gid0 < " << arrayLength;
            ss << " &&  i < " << nCurWindowSize << "; i++)\n";
            ss << "        {\n";
        }
        else
        {
            ss << "0; i < " << arrayLength << "; i++)\n";
            ss << "        {\n";
        }

        ss << "            argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (isNan(argX) || isNan(argY))\n";
        ss << "                 continue;\n";
        ss << "            fSumDeltaXDeltaY += (argX-fMeanX)*(argY-fMeanY);\n";
        ss << "            fSumSqrDeltaX += (argX-fMeanX) * (argX-fMeanX);\n";
        ss << "        }\n";
        ss << "        if(fSumSqrDeltaX == 0.0)\n";
        ss << "            return NAN;\n";
        ss << "        else\n";
        ss << "        {\n";
        ss << "            return fMeanY -";
        ss << " (fSumDeltaXDeltaY*pow(fSumSqrDeltaX,-1.0))*fMeanX;\n";
        ss << "        }\n";
        ss << "    }\n";
        ss << "}\n";
    }
    else
    {
        ss << "    return NAN;\n";
        ss << "}\n";
    }
}
void OpLogInv:: GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2,arg3;\n";
    size_t i = vSubArguments.size();
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"=";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss<< "    double q,t,z;\n";
    ss<< "    q = arg0 -0.5;\n";
    ss<< "    if(fabs(q)<=.425)\n";
    ss<< "    {\n";
    ss<< "        t=0.180625-pow(q, 2);\n";
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
    ss<<"        t = q > 0 ? 1 - arg0 : arg0;\n";
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
    ss << "        z = q < 0.0 ? (-1)*z : z;\n";
    ss << "    }\n";
    ss << "    tmp = exp(arg1+arg2*z);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpForecast::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur0 = vSubArguments[0]->GetFormulaToken();
    assert(pCur0);
    const formula::SingleVectorRefToken*pCurDVR0= static_cast<const
          formula::SingleVectorRefToken *>(pCur0);
    FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
    assert(pCur1);
    const formula::DoubleVectorRefToken* pCurDVR1 =
        static_cast<const formula::DoubleVectorRefToken *>(pCur1);
    size_t nCurWindowSize = pCurDVR1->GetRefRowSize();
    FormulaToken *pCur2 = vSubArguments[2]->GetFormulaToken();
    assert(pCur2);
    const formula::DoubleVectorRefToken* pCurDVR2 =
        static_cast<const formula::DoubleVectorRefToken *>(pCur2);
    size_t nCurWindowSize1 = pCurDVR2->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
    if(pCur0->GetType()== formula::svDouble ||
        pCur0->GetType() == formula::svSingleVectorRef)
    {
        ss << "    double arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    else
        ss << "return HUGE_VAL";
    if(pCur1->GetType() != formula::svDoubleVectorRef ||
        pCur2->GetType() != formula::svDoubleVectorRef)
        ss << "return HUGE_VAL";
    else
    {
        ss<< "    if(isNan(arg0)||(gid0>=";
        ss<<pCurDVR0->GetArrayLength();
        ss<<"))\n";
        ss<<"        arg0 = 0;\n";
        ss << "    int length="<<nCurWindowSize;
        ss << ";\n";
        ss << "    int length1= "<<nCurWindowSize1;
        ss << ";\n";
        ss << "    if(length!=length1)\n";
        ss << "        return 0;\n";
        ss << "    double tmp = 0;\n";
        ss << "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
        ss << "    {\n";
        ss << "        double arg1 = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef(true);
        ss << ";\n";
        ss << "        double arg2 = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef(true);
        ss << ";\n";
        ss << "        if(isNan(arg1)||((gid0+i)>=";
        ss << pCurDVR1->GetArrayLength();
        ss << "))\n";
        ss << "        {\n";
        ss << "            length--;\n";
        ss << "            continue;\n";
        ss << "        }\n";
        ss << "        if(isNan(arg2)||((gid0+i)>=";
        ss << pCurDVR2->GetArrayLength();
        ss << "))\n";
        ss << "        {\n";
        ss << "            length--;\n";
        ss << "            continue;\n";
        ss << "        }\n";
        ss << "        fSumY+=arg1;\n";
        ss << "        fSumX+=arg2;\n";
        ss << "    }\n";
        ss << "    double fMeanX = fSumX / length;\n";
        ss << "    double fMeanY = fSumY / length;\n";
        ss << "    for (int i = 0; i <" << nCurWindowSize << "; i++)\n";
        ss << "    {\n";
        ss << "        double arg1 = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef(true);
        ss << ";\n";
        ss << "        double arg2 = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef(true);
        ss << ";\n";
        ss << "        if(isNan(arg1)||((gid0+i)>=";
        ss <<pCurDVR1->GetArrayLength();
        ss <<"))\n";
        ss <<"        {\n";
        ss <<"            continue;\n";
        ss <<"        }\n";
        ss << "        if(isNan(arg2)||((gid0+i)>=";
        ss <<pCurDVR2->GetArrayLength();
        ss <<"))\n";
        ss <<"        {\n";
        ss <<"            continue;\n";
        ss <<"        }\n";
        ss <<"        fSumDeltaXDeltaY+=(arg2 - fMeanX) * (arg1 - fMeanY);\n";
        ss <<"        fSumSqrDeltaX+=pow(arg2 - fMeanX, 2);\n";
        ss <<"    }\n";
        ss <<"    tmp =fMeanY + fSumDeltaXDeltaY / fSumSqrDeltaX *";
        ss <<" (arg0 - fMeanX);\n";
        ss <<"    return tmp;\n";
        ss << "}";
    }
}
void OpLogNormDist::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0,arg1,arg2,arg3;\n";
    size_t i = vSubArguments.size();
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double tmp;\n";
    ss << "    if(isNan(arg0)||(gid0>=";
    ss << tmpCurDVR0->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
    ss << "    if(isNan(arg1)||(gid0>=";
    ss << tmpCurDVR1->GetArrayLength();
    ss << "))\n";
    ss << "        arg1 = 0;\n";
    ss << "    if(isNan(arg2)||(gid0>=";
    ss << tmpCurDVR2->GetArrayLength();
    ss << "))\n";
    ss << "        arg2 = 0;\n";
    ss << "    if(isNan(arg3)||(gid0>=";
    ss << tmpCurDVR3->GetArrayLength();
    ss << "))\n";
    ss << "        arg3 = 0;\n";
    ss << "    double temp = (log(arg0)-arg1)/arg2;\n";
    ss << "    if(arg3)\n";
    ss << "    {\n";
    ss << "        if(arg0<=0)\n";
    ss << "            tmp = 0.0;\n";
    ss << "        else\n";
    ss << "            tmp = 0.5 * erfc(-temp * 0.7071067811865475);\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "        tmp = (0.39894228040143268 * exp((-1)*pow(temp, 2)";
    ss << " / 2.0))/(arg2*arg0);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpGammaDist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fBigInvDecl);decls.insert(fLogDblMaxDecl);
    decls.insert(fHalfMachEpsDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(GetGammaSeriesDecl);decls.insert(GetGammaContFractionDecl);
    decls.insert(GetLowRegIGammaDecl);decls.insert(GetGammaDistDecl);
    decls.insert(GetGammaDistPDFDecl);
    funs.insert(GetGammaSeries);funs.insert(GetGammaContFraction);
    funs.insert(GetLowRegIGamma);funs.insert(GetGammaDist);
    funs.insert(GetGammaDistPDF);
}

void OpGammaDist::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2,arg3;\n";
    size_t i = vSubArguments.size();

    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    if (arg3)\n";
    ss << "        tmp=GetGammaDist( arg0, arg1, arg2);\n";
    ss << "    else\n";
    ss << "        tmp=GetGammaDistPDF( arg0, arg1, arg2);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}
void OpChiDist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fBigInvDecl);
    funs.insert("");
    decls.insert(fHalfMachEpsDecl);
    funs.insert("");
    decls.insert(GetUpRegIGammaDecl);
    funs.insert(GetUpRegIGamma);
    decls.insert(GetGammaSeriesDecl);
    funs.insert(GetGammaSeries);
    decls.insert(GetGammaContFractionDecl);
    funs.insert(GetGammaContFraction);
    decls.insert(GetChiDistDecl);
    funs.insert(GetChiDist);
}
void OpChiDist::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double fx,fDF,tmp=0,tmp0=0,tmp1=0;\n";
    ss << "    int gid0=get_global_id(0);\n";

    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
            }
            else
            {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
    }
    ss << "    fx = tmp0;\n";
    ss << "    fDF = floor(tmp1);\n";
    ss << "    if(fDF < 1.0)\n";
    ss << "    {\n";
    ss << "        return DBL_MIN;\n";
    ss << "    }\n";
    ss << "    tmp = GetChiDist( fx, fDF);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}
void OpBinomdist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(MinDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(GetBinomDistPMFDecl);
    funs.insert(GetBinomDistPMF);
    decls.insert(GetBetaDistDecl);
    funs.insert(GetBetaDist);
    decls.insert(lcl_GetBinomDistRangeDecl);
    funs.insert(lcl_GetBinomDistRange);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    funs.insert(lcl_GetBetaHelperContFrac);
    decls.insert(GetBetaDistPDFDecl);
    funs.insert(GetBetaDistPDF);
    decls.insert(GetLogBetaDecl);
    funs.insert(GetLogBeta);
    decls.insert(GetBetaDecl);
    funs.insert(GetBeta);
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
}
void OpBinomdist::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double tmp0,tmp1,tmp2,tmp3;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();

    ss <<"\n    ";
    //while (i-- > 1)
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
            }
            else
            {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    tmp0 = floor(tmp0);\n";
    ss << "    tmp1 = floor(tmp1);\n";
    ss << "    double rq = (0.5 - tmp2) + 0.5;\n";
    ss << "    if (tmp1 < 0.0 || tmp0 < 0.0 || tmp0 > tmp1 ||";
    ss << "tmp2 < 0.0 || tmp2 > 1.0)\n";
    ss << "    {\n";
    ss << "        return DBL_MIN;\n";
    ss << "    }\n";
    ss << "    if(tmp2 == 0.0)\n";
    ss << "        return ( (tmp0 == 0.0 || tmp3) ? 1.0 : 0.0 );\n";
    ss << "    if(tmp2 == 1.0)\n";
    ss << "        return ( (tmp0 == tmp1) ? 1.0 : 0.0);\n";
    ss << "    if(!tmp3)\n";
    ss << "        return ( GetBinomDistPMF(tmp0, tmp1, tmp2));\n";
    ss << "    else \n";
    ss << "    {\n";
    ss << "        if(tmp0 == tmp1)\n";
    ss << "            return 1.0;\n";
    ss << "        else\n";
    ss << "        {\n";
    ss << "            double fFactor = pow(rq,tmp1);\n";
    ss << "            if(tmp0 == 0.0)\n";
    ss << "            return (fFactor);\n";
    ss << "            else if(fFactor <= Min)\n";
    ss << "            {\n";
    ss << "                fFactor = pow(tmp2,tmp1);\n";
    ss << "                if(fFactor <= Min)\n";
    ss << "                    return GetBetaDist";
    ss << "(rq, tmp1 - tmp0, tmp0 + 1.0);\n";
    ss << "                else\n";
    ss << "                {\n";
    ss << "                    if(fFactor > fMachEps)\n";
    ss << "                    {\n";
    ss << "                        double fSum = 1.0 - fFactor;\n";
    ss << "                        unsigned int max = ";
    ss << "(unsigned int)((tmp1 - tmp0)-1);\n";
    ss << "                        for (uint i = 0; i < max && fFactor > 0.0;";
    ss << " i++)\n";
    ss << "                        {\n";
    ss << "                           fFactor *= (tmp1 - i)*pow((i + 1),-1.0)*";
    ss << "rq*pow(tmp2,-1.0);\n";
    ss << "                            fSum -= fFactor;\n";
    ss << "                        }\n";
    ss << "                         return ( (fSum < 0.0) ? 0.0 : fSum );\n";
    ss << "                    }\n";
    ss << "                    else \n";
    ss << "                        return (lcl_GetBinomDistRange";
    ss << "(tmp1, tmp1 -  tmp0, tmp1, fFactor, rq, tmp2));\n";
    ss << "                }\n";
    ss << "            }\n";
    ss << "           else\n";
    ss << "           {\n";
    ss << "               double rtmp = ( lcl_GetBinomDistRange";
    ss << "(tmp1, 0.0, tmp0, fFactor, tmp2, rq));\n";
    ss << "               return rtmp;\n";
    ss << "           }\n";
    ss << "       }\n";
    ss << "   }\n";
    ss << "}\n";
}

void OpChiSqDist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMaxGammaArgumentDecl);decls.insert(GetChiSqDistCDFDecl);
    decls.insert(GetChiSqDistPDFDecl);decls.insert(GetLowRegIGammaDecl);
    decls.insert(GetGammaContFractionDecl);decls.insert(GetGammaSeriesDecl);
    decls.insert(fHalfMachEpsDecl);decls.insert(F_PIDecl);
    decls.insert(fBigInvDecl);

    funs.insert(GetGammaContFraction);funs.insert(GetChiSqDistCDF);
    funs.insert(GetChiSqDistPDF);funs.insert(GetLowRegIGamma);
    funs.insert(GetGammaSeries);
}

void OpChiSqDist::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    if(vSubArguments.size()<2)
    {
        ss << "    result = -DBL_MAX;\n";
        ss << "    return result;\n";
    }else
    {
        GenTmpVariables(ss,vSubArguments);
        CheckAllSubArgumentIsNan(ss,vSubArguments);
        if(vSubArguments.size() == 2)
        {
            ss << "    int tmp2  = 1;\n";
        }
    }
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
        ss << "    tmp1 = floor(tmp1);\n";
        ss << "    if(tmp1 < 1.0)\n";
        ss << "        result = -DBL_MAX;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        if(tmp2)\n";
        ss << "            result =GetChiSqDistCDF(tmp0,tmp1);\n";
        ss << "        else\n";
        ss << "            result =GetChiSqDistPDF(tmp0,tmp1);\n";
        ss << "    }\n";
        ss << "    return result;\n";
        ss << "}";
    }

 void OpChiSqInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMaxGammaArgumentDecl);decls.insert(GetChiSqDistCDFDecl);
    decls.insert(GetLowRegIGammaDecl);decls.insert(lcl_IterateInverseChiSQInvDecl);
    decls.insert(GetGammaContFractionDecl);decls.insert(GetGammaSeriesDecl);
    decls.insert(fHalfMachEpsDecl);decls.insert(F_PIDecl);
    decls.insert(fBigInvDecl);decls.insert(lcl_HasChangeOfSignDecl);
    decls.insert(fMachEpsDecl);

    funs.insert(GetGammaContFraction);funs.insert(GetChiSqDistCDF);
    funs.insert(GetLowRegIGamma);funs.insert(lcl_HasChangeOfSign);
    funs.insert(GetGammaSeries);funs.insert(lcl_IterateInverseChiSQInv);
}

void OpChiSqInv::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    if(vSubArguments.size()!=2)
    {
        ss << "    result = -DBL_MAX;\n";
        ss << "    return result;\n";
    }
    else
    {
        GenTmpVariables(ss,vSubArguments);
        CheckAllSubArgumentIsNan(ss,vSubArguments);
        size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
        ss << "    tmp1 = floor(tmp1);\n";
        ss << "    bool bConvError;\n";
        ss << "    if(tmp1 < 1.0 || tmp0 < 0 || tmp0>=1.0)\n";
        ss << "        result = -DBL_MAX;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        result =lcl_IterateInverseChiSQInv( tmp0, tmp1,";
        ss << "tmp1*0.5, tmp1, &bConvError );\n";
        ss << "    }\n";
        ss << "    if(bConvError)\n";
        ss << "        result = -DBL_MAX;\n";
        ss << "    return result;\n";
        ss << "}";
    }

}
void OpGammaInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fBigInvDecl);decls.insert(fHalfMachEpsDecl);
    decls.insert(GetGammaSeriesDecl);decls.insert(GetGammaContFractionDecl);
    decls.insert(GetGammaInvValueDecl);
    funs.insert(GetGammaSeries);funs.insert(GetGammaContFraction);
    funs.insert(GetGammaInvValue);
}

void OpGammaInv::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2;\n";
    size_t i = vSubArguments.size();

    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    if (arg0 == 0.0)\n"
    "    {\n"
    "        tmp=0.0;\n"
    "        return tmp;\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        bool bConvError;\n"
    "        double fStart = arg1 * arg2;\n"
    "        double fAx=fStart*0.5;\n"
    "        double fBx=fStart;\n"
    "        bConvError = false;\n"
    "        double fYEps = 1.0E-307;\n"
    "        double fXEps = 2.22045e-016;\n"
    "        double fAy = arg0-GetGammaInvValue(arg1,arg2,fAx);\n"
    "        double fBy = arg0-GetGammaInvValue(arg1,arg2,fBx);\n"
    "        double fTemp;\n"
    "        unsigned short nCount;\n"
    "        for (nCount = 0; nCount < 1000 && !((fAy < 0.0 && fBy > 0.0)"
    " || (fAy > 0.0 && fBy < 0.0)); nCount++)\n"
    "        {\n"
    "            if (fabs(fAy) <= fabs(fBy))\n"
    "            {\n"
    "                fTemp = fAx;\n"
    "                fAx += 2.0 * (fAx - fBx);\n"
    "                if (fAx < 0.0)\n"
    "                    fAx = 0.0;\n"
    "                fBx = fTemp;\n"
    "                fBy = fAy;\n"
    "                fAy = arg0-GetGammaInvValue(arg1,arg2,fAx);\n"
    "            }\n"
    "            else\n"
    "            {\n"
    "                fTemp = fBx;\n"
    "                fBx += 2.0 * (fBx - fAx);\n"
    "                fAx = fTemp;\n"
    "                fAy = fBy;\n"
    "                fBy = arg0-GetGammaInvValue(arg1,arg2,fBx);\n"
    "            }\n"
    "        }\n"
    "        if (fAy == 0.0)\n"
    "        {\n"
    "            tmp = fAx;\n"
    "            return tmp;\n"
    "        }\n"
    "        if (fBy == 0.0)\n"
    "        {\n"
    "            tmp = fBx;\n"
    "            return tmp;\n"
    "        }\n"
    "        if (!((fAy < 0.0 && fBy > 0.0) || (fAy > 0.0 && fBy < 0.0)))\n"
    "        {\n"
    "            bConvError = true;\n"
    "            tmp = 0.0;\n"
    "            return tmp;\n"
    "        }\n"
    "        double fPx = fAx;\n"
    "        double fPy = fAy;\n"
    "        double fQx = fBx;\n"
    "        double fQy = fBy;\n"
    "        double fRx = fAx;\n"
    "        double fRy = fAy;\n"
    "        double fSx = 0.5 * (fAx + fBx);\n"
    "        bool bHasToInterpolate = true;\n"
    "        nCount = 0;\n"
    "        while ( nCount < 500 && fabs(fRy) > fYEps &&"
    "(fBx-fAx) > fmax( fabs(fAx), fabs(fBx)) * fXEps )\n"
    "        {\n"
    "            if (bHasToInterpolate)\n"
    "            {\n"
    "                if (fPy!=fQy && fQy!=fRy && fRy!=fPy)\n"
    "                {\n"
    "                    fSx = fPx * fRy * fQy *pow( (fRy-fPy),-1) *pow"
    "( (fQy-fPy),-1)"
    "+ fRx * fQy * fPy *pow( (fQy-fRy),-1) *pow( (fPy-fRy),-1)"
    "+ fQx * fPy * fRy *pow( (fPy-fQy),-1) *pow( (fRy-fQy),-1);\n"
    "                    bHasToInterpolate = (fAx < fSx) && (fSx < fBx);\n"
    "                }\n"
    "                else\n"
    "                    bHasToInterpolate = false;\n"
    "            }\n"
    "            if(!bHasToInterpolate)\n"
    "            {\n"
    "                fSx = 0.5 * (fAx + fBx);\n"
    "                fPx = fAx; fPy = fAy;\n"
    "                fQx = fBx; fQy = fBy;\n"
    "                bHasToInterpolate = true;\n"
    "            }\n"
    "            fPx = fQx; fQx = fRx; fRx = fSx;\n"
    "            fPy = fQy; fQy = fRy;\n"
    "            fRy =  arg0-GetGammaInvValue(arg1,arg2,fSx);\n"
    "            if ((fAy < 0.0 && fRy > 0.0) || (fAy > 0.0 && fRy < 0.0))\n"
    "            {\n"
    "                fBx = fRx;\n"
    "                fBy = fRy;\n"
    "            }\n"
    "            else\n"
    "            {\n"
    "                fAx = fRx;\n"
    "                fAy = fRy;\n"
    "            }\n"
    "            bHasToInterpolate = bHasToInterpolate && (fabs(fRy)"
    " * 2.0 <= fabs(fQy));\n"
    "            ++nCount;\n"
    "        }\n"
    "        tmp = fRx;\n"
    "        return tmp;\n"
    "    }\n"
    "}\n";
}
void OpFInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(lcl_getLanczosSumDecl);decls.insert(GetBetaDecl);
    decls.insert(GetLogBetaDecl);decls.insert(GetBetaDistPDFDecl);
    decls.insert(lcl_GetBetaHelperContFracDecl);decls.insert(GetFInvValueDecl);
    funs.insert(lcl_getLanczosSum);funs.insert(GetBeta);
    funs.insert(GetLogBeta);funs.insert(GetBetaDistPDF);
    funs.insert(lcl_GetBetaHelperContFrac);funs.insert(GetFInvValue);
}

void OpFInv::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2;\n";
    size_t i = vSubArguments.size();

    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"="<<vSubArguments[i]->
                                              GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"="<<vSubArguments[i]->
                                             GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double fF2=floor(arg2);\n"
    "    double fF1=floor(arg1);\n"
    "    bool bConvError;\n"
    "    double fAx=fF1*0.5;\n"
    "    double fBx=fF1;\n"
    "    bConvError = false;\n"
    "    const double fYEps = 1.0E-307;\n"
    "    const double fXEps = 2.22045e-016;\n"
    "    double fAy = arg0-GetFInvValue(fF1,fF2,fAx);\n"
    "    double fBy = arg0-GetFInvValue(fF1,fF2,fBx);\n"
    "    double fTemp;\n"
    "    unsigned short nCount;\n"
    "    for (nCount = 0; nCount < 1000 && !((fAy < 0.0 && fBy > 0.0)"
    " || (fAy > 0.0 && fBy < 0.0)); nCount++)\n"
    "    {\n"
    "        if (fabs(fAy) <= fabs(fBy))\n"
    "        {\n"
    "            fTemp = fAx;\n"
    "            fAx += 2.0 * (fAx - fBx);\n"
    "            if (fAx < 0.0)\n"
    "                fAx = 0.0;\n"
    "            fBx = fTemp;\n"
    "            fBy = fAy;\n"
    "            fAy = arg0-GetFInvValue(fF1,fF2,fAx);\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            fTemp = fBx;\n"
    "            fBx += 2.0 * (fBx - fAx);\n"
    "            fAx = fTemp;\n"
    "            fAy = fBy;\n"
    "            fBy = arg0-GetFInvValue(fF1,fF2,fBx);\n"
    "        }\n"
    "    }\n"
    "    if (fAy == 0.0)\n"
    "    {\n"
    "        tmp = fAx;\n"
    "        return tmp;\n"
    "    }\n"
    "    if (fBy == 0.0)\n"
    "    {\n"
    "        tmp = fBx;\n"
    "        return tmp;\n"
    "    }\n"
    "    if (!((fAy < 0.0 && fBy > 0.0) || (fAy > 0.0 && fBy < 0.0)))\n"
    "    {\n"
    "        bConvError = true;\n"
    "        tmp = 0.0;\n"
    "        return tmp;\n"
    "    }\n"
    "    double fPx = fAx;\n"
    "    double fPy = fAy;\n"
    "    double fQx = fBx;\n"
    "    double fQy = fBy;\n"
    "    double fRx = fAx;\n"
    "    double fRy = fAy;\n"
    "    double fSx = 0.5 * (fAx + fBx);\n"
    "    bool bHasToInterpolate = true;\n"
    "    nCount = 0;\n"
    "    while ( nCount < 500 && fabs(fRy) > fYEps &&"
    "(fBx-fAx) > fmax( fabs(fAx), fabs(fBx)) * fXEps )\n"
    "    {\n"
    "        if (bHasToInterpolate)\n"
    "        {\n"
    "            if (fPy!=fQy && fQy!=fRy && fRy!=fPy)\n"
    "            {\n"
    "                fSx = fPx * fRy * fQy *pow( (fRy-fPy),-1)"
    " *pow( (fQy-fPy),-1)+fRx * fQy * fPy*pow( (fQy-fRy),-1) *"
    "pow( (fPy-fRy),-1)+ fQx * fPy * fRy *pow( (fPy-fQy),-1)"
    " *pow((fRy-fQy),-1);\n"
    "                bHasToInterpolate = (fAx < fSx) && (fSx < fBx);\n"
    "            }\n"
    "            else\n"
    "                bHasToInterpolate = false;\n"
    "        }\n"
    "        if(!bHasToInterpolate)\n"
    "        {\n"
    "            fSx = 0.5 * (fAx + fBx);\n"
    "            fPx = fAx; fPy = fAy;\n"
    "            fQx = fBx; fQy = fBy;\n"
    "            bHasToInterpolate = true;\n"
    "        }\n"
    "        fPx = fQx; fQx = fRx; fRx = fSx;\n"
    "        fPy = fQy; fQy = fRy;\n"
    "        fRy = arg0-GetFInvValue(fF1,fF2,fSx);\n"
    "        if ((fAy < 0.0 && fRy > 0.0) || (fAy > 0.0 && fRy < 0.0))\n"
    "        {\n"
    "            fBx = fRx; fBy = fRy;\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            fAx = fRx; fAy = fRy;\n"
    "        }\n"
    "        bHasToInterpolate = bHasToInterpolate && (fabs(fRy)"
    " * 2.0 <= fabs(fQy));\n"
    "        ++nCount;\n"
    "    }\n"
    "    tmp = fRx;\n"
    "    return tmp;"
    "}";
}
void OpFTest::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(lcl_getLanczosSumDecl);decls.insert(GetBetaDecl);
    decls.insert(GetLogBetaDecl);decls.insert(GetBetaDistPDFDecl);
    decls.insert(lcl_GetBetaHelperContFracDecl);decls.insert(GetBetaDistDecl);
    decls.insert(GetFDistDecl);
    funs.insert(lcl_getLanczosSum);funs.insert(GetBeta);
    funs.insert(GetLogBeta);funs.insert(GetBetaDistPDF);
    funs.insert(lcl_GetBetaHelperContFrac);funs.insert(GetBetaDist);
    funs.insert(GetFDist);
}
void OpFTest::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    FormulaToken *pCur1 = vSubArguments[1]->GetFormulaToken();
    assert(pCur1);
    const formula::DoubleVectorRefToken* pCurDVR1 =
        static_cast<const formula::DoubleVectorRefToken *>(pCur1);
    size_t nCurWindowSize1 = pCurDVR1->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum1 = 0.0;\n";
    ss << "    double fSumSqr1 = 0.0;\n";
    ss << "    double fSum2 = 0.0;\n";
    ss << "    double fSumSqr2 = 0.0;\n";
    ss << "    int length0="<<nCurWindowSize;
    ss << ";\n";
    ss << "    int length1= "<<nCurWindowSize1;
    ss << ";\n";
    ss << "    double tmp = 0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCurSub = vSubArguments[i]->GetFormulaToken();
        assert(pCurSub);
        if (pCurSub->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCurSub);
            ss << "    for (int i = ";
            ss << "0; i < "<< pDVR->GetRefRowSize() << "; i++){\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            length"<<i<<"--;\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        fSum"<<i+1<<" += arg"<<i<<";\n";
            ss << "        fSumSqr"<<i+1<<" += arg"<<i;
            ss << " * arg"<<i<<";\n";
            ss << "    }\n";
        }
        else if (pCurSub->GetType() == formula::svSingleVectorRef)
        {
            ss << "return HUGE_VAL";
        }
        else if (pCurSub->GetType() == formula::svDouble)
        {
            ss << "return HUGE_VAL";
        }
    }
    ss << "    double fS1 = (fSumSqr1-fSum1*fSum1/length0)/(length0-1.0);\n"
        "    double fS2 = (fSumSqr2-fSum2*fSum2/length1)/(length1-1.0);\n"
        "    double fF, fF1, fF2;\n"
        "    if (fS1 > fS2)\n"
        "    {\n"
        "        fF = fS1/fS2;\n"
        "        fF1 = length0-1.0;\n"
        "        fF2 = length1-1.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        fF = fS2/fS1;\n"
        "        fF1 = length1-1.0;\n"
        "        fF2 = length0-1.0;\n"
        "    }\n"
        "    tmp = 2.0*GetFDist(fF, fF1, fF2);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpB::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    //decls.insert(fBigInvDecl);decls.insert(fLogDblMaxDecl);
    decls.insert(GetBinomDistPMFDecl);decls.insert(MinDecl);
    decls.insert(fMachEpsDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(GetBetaDistDecl);decls.insert(GetBetaDistPDFDecl);
    decls.insert(lcl_GetBetaHelperContFracDecl);decls.insert(GetLogBetaDecl);
    decls.insert(lcl_getLanczosSumDecl); decls.insert(GetBetaDecl);
    funs.insert(GetBinomDistPMF);funs.insert(lcl_GetBinomDistRange);
    funs.insert(GetBetaDist);funs.insert(GetBetaDistPDF);
    funs.insert(lcl_GetBetaHelperContFrac);funs.insert(GetLogBeta);
    funs.insert(lcl_getLanczosSum);funs.insert(GetBeta);
}

void OpB::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double min = 2.22507e-308;\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2,arg3;\n";
    size_t i = vSubArguments.size();
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double rxs = floor(arg2);\n"
    "    double rxe = floor(arg3);\n"
    "    double rn = floor(arg0);\n"
    "    double rq = (0.5 - arg1) + 0.5;\n"
    "    bool bIsValidX = (0.0 <= rxs && rxs <= rxe && rxe <= rn);\n"
    "    if (bIsValidX && 0.0 < arg1 && arg1 < 1.0)\n"
    "    {\n"
    "        if (rxs == rxe)\n"
    "            tmp = GetBinomDistPMF(rxs, rn, arg1);\n"
    "        else\n"
    "        {\n"
    "            double fFactor = pow(rq, rn);\n"
    "            if (fFactor > min)\n"
    "                tmp ="
    " lcl_GetBinomDistRange(rn, rxs, rxe, fFactor, arg1, rq);\n"
    "            else\n"
    "            {\n"
    "                fFactor = pow(arg1, rn);\n"
    "                if (fFactor > min)\n"
    "                {\n"
    "                    tmp ="
    "lcl_GetBinomDistRange(rn, rn - rxe, rn - rxs, fFactor, rq, arg1);\n"
    "            }\n"
    "                else\n"
    "                    tmp ="
    "GetBetaDist(rq, rn - rxe, rxe + 1.0)"
    "- GetBetaDist(rq, rn - rxs + 1, rxs);\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        if (bIsValidX)\n"
    "        {\n"
    "            if (arg1 == 0.0)\n"
    "                tmp = (rxs == 0.0 ? 1.0 : 0.0);\n"
    "            else if (arg1 == 1.0)\n"
    "                tmp = (rxe == rn ? 1.0 : 0.0);\n"
    "            else\n"
    "            {\n"
    "                tmp = DBL_MIN;\n"
    "            }\n"
    "        }\n"
    "        else\n"
    "        {\n"
    "            tmp = DBL_MIN;\n"
    "        }\n"
    "    }\n"
    "    return tmp;"
    "}\n";
}
void OpBetaDist::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);decls.insert(fMaxGammaArgumentDecl);
    decls.insert(GetBetaDistDecl);decls.insert(GetBetaDistPDFDecl);
    decls.insert(lcl_GetBetaHelperContFracDecl);decls.insert(GetLogBetaDecl);
    decls.insert(GetBetaDecl);decls.insert(lcl_getLanczosSumDecl);
    funs.insert(GetBetaDist);funs.insert(GetBetaDistPDF);
    funs.insert(lcl_GetBetaHelperContFrac);funs.insert(GetLogBeta);
    funs.insert(GetBeta);funs.insert(lcl_getLanczosSum);
}
void OpPoisson::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fHalfMachEpsDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(fBigInvDecl);
    funs.insert("");
    decls.insert(GetLogGammaDecl);
    funs.insert(GetLogGamma);
    decls.insert(lcl_GetLogGammaHelperDecl);
    funs.insert(lcl_GetLogGammaHelper);
    decls.insert(lcl_GetGammaHelperDecl);
    funs.insert(lcl_GetGammaHelper);
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
    decls.insert(GetUpRegIGammaDecl);
    funs.insert(GetUpRegIGamma);
    decls.insert(GetGammaContFractionDecl);
    funs.insert(GetGammaContFraction);
    decls.insert(GetGammaSeriesDecl);
    funs.insert(GetGammaSeries);
}
void OpPoisson::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double x,lambda,tmp,tmp0,tmp1,tmp2;\n";
    ss << "    int bCumulative;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    x = floor(tmp0);\n";
    ss << "    lambda = tmp1;\n";
    ss << "    bCumulative = tmp2;\n ";
    ss << "    if (!bCumulative)\n";
    ss << "    {\n";
    ss << "        if(lambda == 0.0)\n";
    ss << "        {\n";
    ss << "            return 0;\n";
    ss << "        }\n";
    ss << "        else\n";
    ss << "        {\n";
    ss << "            if (lambda >712)\n";
    ss << "            {\n";
    ss << "            tmp = (exp(x*log(lambda)-lambda-GetLogGamma(x+1.0)));\n";
    ss << "            return tmp;\n";
    ss << "            }\n";
    ss << "            else\n";
    ss << "            {\n";
    ss << "                double fPoissonVar = 1.0;\n";
    ss << "                for ( int f = 0; f < x; ++f )\n";
    ss << "          fPoissonVar *= lambda * pow(( (double)f + 1.0 ),-1);\n";
    ss << "                tmp = ( fPoissonVar * exp( -lambda ) );\n";
    ss << "                return tmp;\n";
    ss << "            }\n";
    ss << "        }\n";
    ss << "     } \n";
    ss << "     else\n";
    ss << "     {\n";
    ss << "         if (lambda == 0.0)\n";
    ss << "         {\n";
    ss << "             return 1;\n";
    ss << "         }\n";
    ss << "         else\n";
    ss << "         {\n";
    ss << "             if (lambda > 712 )\n";
    ss << "             {\n";
    ss << "                 tmp = (GetUpRegIGamma(x+1.0,lambda));\n";
    ss << "                 return tmp;\n";
    ss << "             }\n";
    ss << "             else\n";
    ss << "             {\n";
    ss << "                 if (x >= 936.0)\n";
    ss << "                 {\n";
    ss << "                     return 1;\n";
    ss << "                 }\n";
    ss << "                 else\n";
    ss << "                 {\n";
    ss << "                     double fSummand = exp(-lambda);\n";
    ss << "                     double fSum = fSummand;\n";
    ss << "                     int nEnd = (int) (x + 0.5);\n";
    ss << "                     for (int i = 1; i <= nEnd; i++)\n";
    ss << "                     {\n";
    ss << "                fSummand = (fSummand*lambda)*pow((double)i,-1);\n";
    ss << "                         fSum += fSummand;\n";
    ss << "                     }\n";
    ss << "                     tmp = fSum;\n";
    ss << "                     return tmp;\n";
    ss << "                 }\n";
    ss << "             }\n";
    ss << "         }\n";
    ss << "     }\n";
    ss << "}\n";
}
void OpCovar::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n";
        ss << "    int gid0 = get_global_id(0);\n";
        ss << "    double vSum = 0.0;\n";
        ss << "    double vSum0 = 0.0;\n";
        ss << "    double vSum1 = 0.0;\n";
        ss << "    double vMean0 = 0.0;\n";
        ss << "    double vMean1 = 0.0;\n";
        ss << "    double arg0 = 0.0;\n";
        ss << "    double arg1 = 0.0;\n";
        FormulaToken* pCurX = vSubArguments[0]->GetFormulaToken();
        FormulaToken* pCurY = vSubArguments[1]->GetFormulaToken();
        if ((pCurX->GetType() == formula::svDoubleVectorRef)&&
            (pCurY->GetType() == formula::svDoubleVectorRef)){
        ss << "    int cnt = 0;\n";
        const formula::DoubleVectorRefToken* pCurDVRX =
            static_cast<const formula::DoubleVectorRefToken* >(pCurX);
        const formula::DoubleVectorRefToken* pCurDVRY =
            static_cast<const formula::DoubleVectorRefToken* >(pCurY);
        size_t nCurWindowSizeX = pCurDVRX->GetRefRowSize();
        size_t nCurWindowSizeY = pCurDVRY->GetRefRowSize();
        if(nCurWindowSizeX == nCurWindowSizeY)
        {
            ss << "    for( ";
            if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "int i = gid0; i < " << nCurWindowSizeX;
                ss << " && i < " << pCurDVRX->GetArrayLength() << " && i < ";
                ss << pCurDVRY->GetArrayLength() << "; i++){\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isNan("<< vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")) {\n";
                ss << "            arg0 = 0.0;\n";
                ss << "            arg1 = 0.0;\n";
                ss << "            --cnt;\n";
                ss << "        }\n";
                ss << "else{\n";
                ss << "        arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
                ss << "}\n";
                ss << "        ++cnt;\n";
                ss << "        vSum0 += arg0;\n";
                ss << "        vSum1 += arg1;\n";
                ss << "    }\n";
            }
            else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
                ss << "int i = 0; i < gid0 + " << nCurWindowSizeX << " && ";
                ss << " i < " << pCurDVRX->GetArrayLength() << " && ";
                ss << " i < " << pCurDVRY->GetArrayLength() << "; i++) {\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")) {\n";
                ss << "            arg0 = 0.0;\n";
                ss << "            arg1 = 0.0;\n";
                ss << "            --cnt;\n";
                ss << "        }\n";
                ss << "else{\n";
                ss << "        arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";}\n";
                ss << "        ++cnt;\n";
                ss << "        vSum0 += arg0;\n";
                ss << "        vSum1 += arg1;\n";
                ss << "    }\n";
            }
            else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "int i = 0; i < " << nCurWindowSizeX << " && i < ";
                ss << pCurDVRX->GetArrayLength() << " && i < ";
                ss << pCurDVRY->GetArrayLength() << "; i++) {\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")) {\n";
                ss << "            arg0 = 0.0;\n";
                ss << "            arg1 = 0.0;\n";
                ss << "            --cnt;\n";
                ss << "        }\n";
                ss << "else{\n";
                ss << "        arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";}\n";
                ss << "        ++cnt;\n";
                ss << "        vSum0 += arg0;\n";
                ss << "        vSum1 += arg1;\n";
                ss << "    }\n";
            }
            else {
                ss << "int i = 0; i < " << nCurWindowSizeX << " && ";
                ss << " i + gid0 < " << pCurDVRX->GetArrayLength();
                ss << " && i + gid0 < " << pCurDVRX->GetArrayLength();
                ss << "; i++) {\n";
                ss << "if ((isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ")) || ";
                ss << "(isNan("<< vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")))  {\n";
                ss << "            arg0 = 0.0;\n";
                ss << "            arg1 = 0.0;\n";
                ss << "            --cnt;\n";
                ss << "        }\n";
                ss << "        else {\n";
                ss << "            arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "            arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        }\n";
                ss << "        ++cnt;\n";
                ss << "        vSum0 += arg0;\n";
                ss << "        vSum1 += arg1;\n";
                ss << "    }\n";
            }
            ss << "    if(cnt < 1) {\n";
            ss << "        return CreateDoubleError(errNoValue);\n";
            ss << "    }\n";
            ss << "    else {\n";
            ss << "        vMean0 = vSum0 / cnt;\n";
            ss << "        vMean1 = vSum1 / cnt;\n";
            ss << "    for(";
            if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "int i = gid0; i < " << nCurWindowSizeX;
                ss << " && i < " << pCurDVRX->GetArrayLength() << " && i < ";
                ss << pCurDVRY->GetArrayLength() << "; i++){\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            arg0 = vMean0;\n";
                ss << "            arg1 = vMean1;\n";
                ss << "        }\n";
                ss << "        else{\n";
                ss << "            arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "            arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";}\n";
                ss << "        vSum += (arg0 - vMean0) * (arg1 - vMean1);\n";
                ss << "    }\n";
            }
            else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
                ss << "int i = 0; i < gid0 + " << nCurWindowSizeX << " && ";
                ss << " i < " << pCurDVRX->GetArrayLength() << " && ";
                ss << " i < " << pCurDVRY->GetArrayLength() << "; i++) {\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") || ";
                ss << "isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")) {\n";
                ss << "            arg0 = vMean0;\n";
                ss << "            arg1 = vMean1;\n";
                ss << "        }\n";
                ss << "else{\n";
                ss << "        arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";}\n";
                ss << "        vSum += (arg0 - vMean0) * (arg1 - vMean1);\n";
                ss << "    }\n";
            }
            else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "int i = 0; i < " << nCurWindowSizeX << " && i < ";
                ss << pCurDVRX->GetArrayLength() << " && i < ";
                ss << pCurDVRY->GetArrayLength() << "; i++) {\n";
                ss << "        if(isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") || ";
                ss << "isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")) {\n";
                ss << "            arg0 = vMean0;\n";
                ss << "            arg1 = vMean1;\n";
                ss << "        }\n";
                ss << "else{\n";
                ss << "        arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";}\n";
                ss << "        vSum += (arg0 - vMean0) * (arg1 - vMean1);\n";
                ss << "    }\n";
            }
            else {
                ss << "int i = 0; i < " << nCurWindowSizeX << " && ";
                ss << " i + gid0 < " << pCurDVRX->GetArrayLength();
                ss << " && i + gid0 < " << pCurDVRX->GetArrayLength();
                ss << "; i++) {\n";
                ss << "if((isNan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ")) || ";
                ss << "(isNan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
                ss << ")))  {\n";
                ss << "            arg0 = vMean0;\n";
                ss << "            arg1 = vMean1;\n";
                ss << "        }\n";
                ss << "        else{\n";
                ss << "            arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
                ss << "            arg1 = ";
                ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
                ss << "        }\n";
                ss << "        vSum += (arg0 - vMean0) * (arg1 - vMean1);\n";
                ss << "    }\n";
            }
            ss << "    return vSum / cnt;\n";
            ss << "    }\n";
            ss << "}";
        }
        }
        else {
        ss << "    int cnt0 = 0,cnt1 = 0;\n";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
            if (pCur->GetType() == formula::svSingleVectorRef){
                const formula::SingleVectorRefToken* pTVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if(isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ") || gid0 >= " << pTVR->GetArrayLength() << ")\n";
                ss << "        arg" << i << " = 0;\n    else\n";
                ss << "        arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "    cnt" << i << "++;\n";
                ss << "    vSum" << i << " += arg" << i << ";\n";
            }
            else if (pCur->GetType() == formula::svDouble){
                ss << "    if(isNan ( ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << "))\n";
                ss << "        arg" << i << " = 0;\n    else\n";
                ss << "        arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "    cnt" << i << "++;\n";
                ss << "    vSum" << i << " += arg" << i << ";\n";
            }
            else {
                ss << "    arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "    cnt" << i << "++;\n";
                ss << "    vSum" << i << " += arg" << i << ";\n";
            }
        }
        ss << "        vMean0 = vSum0 / cnt0;\n";
        ss << "        vMean1 = vSum0 / cnt1;\n";
        for(size_t i = 0; i < vSubArguments.size(); i++ ) {
            FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
            if (pCur->GetType() == formula::svSingleVectorRef) {
                const formula::SingleVectorRefToken* pTVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                ss << "    if(isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ") || gid0 >= " << pTVR->GetArrayLength() << ")\n";
                ss << "        arg" << i << " = vMean" << i << ";\n";
                ss << "    else\n";
                ss << "        arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            }
            else if (pCur->GetType() == formula::svDouble) {
                ss << "    arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "    if(isNan(arg" << i << "))\n";
                ss << "        arg" << i << " = vMean" << i << ";\n";
            }
            else {
                ss << "    arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            }
        }
        ss << "        vSum += (arg0 - vMean0) * ( arg1 - vMean1 );\n";
        ss << "    return vSum / cnt0;\n";
        ss << "}";
        }
}
void OpBetaDist::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    ss << "    double arg0,arg1,arg2,arg3,arg4,arg5;\n";
    size_t i = vSubArguments.size();

    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }

        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double fScale = arg4 - arg3;\n"
    "    if (fScale <= 0.0 || arg1 <= 0.0 || arg2 <= 0.0)\n"
    "    {\n"
    "        tmp = DBL_MIN;\n"
    "        return tmp;\n"
    "    }\n"
    "    if (arg5)\n"
    "    {\n"
    "        if (arg0< arg3)\n"
    "        {\n"
    "            tmp = 0.0;\n"
    "            return tmp;\n"
    "        }\n"
    "        if (arg0 > arg4)\n"
    "        {\n"
    "            tmp = 1.0;\n"
    "            return tmp;\n"
    "        }\n"
    "        arg0 = (arg0-arg3)*pow(fScale,-1);\n"
    "        tmp =  GetBetaDist(arg0, arg1, arg2);\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        if (arg0 < arg3 || arg0 > arg4 )\n"
    "        {\n"
    "            tmp = 0.0;\n"
    "            return tmp;\n"
    "        }\n"
    "        arg0 = (arg0 - arg3)*pow(fScale,-1);\n"
    "        tmp = GetBetaDistPDF(arg0, arg1, arg2)*pow(fScale,-1);\n"
    "    }\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}
void OpBetainv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(fMachEpsDecl);
    funs.insert("");
    decls.insert(fMaxGammaArgumentDecl);
    funs.insert("");
    decls.insert(lcl_IterateInverseBetaInvDecl);
    funs.insert(lcl_IterateInverseBetaInv);
    decls.insert(GetBetaDistDecl);
    funs.insert(GetBetaDist);
    decls.insert(lcl_HasChangeOfSignDecl);
    funs.insert(lcl_HasChangeOfSign);
    decls.insert(lcl_HasChangeOfSignDecl);
    funs.insert(lcl_HasChangeOfSign);
    decls.insert(lcl_HasChangeOfSignDecl);
    funs.insert(lcl_HasChangeOfSign);
    decls.insert(lcl_GetBetaHelperContFracDecl);
    funs.insert(lcl_GetBetaHelperContFrac);
    decls.insert(GetBetaDistPDFDecl);
    funs.insert(GetBetaDistPDF);
    decls.insert(GetLogBetaDecl);
    funs.insert(GetLogBeta);
    decls.insert(GetBetaDecl);
    funs.insert(GetBeta);
    decls.insert(lcl_getLanczosSumDecl);
    funs.insert(lcl_getLanczosSum);
}
void OpBetainv::GenSlidingWindowFunction(
    std::stringstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    double tmp0,tmp1,tmp2,tmp3,tmp4;\n";
    ss << "    int gid0=get_global_id(0);\n";
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    //while (i-- > 1)
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {

        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        tmp"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        tmp"<<i<<"=\n";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n}\n";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss << "    if (tmp0 < 0.0 || tmp0 >= 1.0 ||";
    ss << "tmp3 == tmp4 || tmp1 <= 0.0 || tmp2 <= 0.0)\n";
    ss << "    {\n";
    ss << "        return DBL_MIN;\n";
    ss << "    }\n";
    ss << "    if (tmp0 == 0.0)\n";
    ss << "        return 0.0;\n";
    ss << "    else\n";
    ss << "    {";
    ss << "        bool bConvError;";
    ss << "        double fVal = lcl_IterateInverseBetaInv";
    ss << "(tmp0, tmp1, tmp2, 0.0, 1.0, &bConvError);\n";
    ss << "        if(bConvError)\n";
    ss << "            return DBL_MIN;\n";
    ss << "        else\n";
    ss << "            return (tmp3 + fVal*(tmp4 - tmp3));\n";
    ss << "    }";
    ss << "}\n";
}
void OpDevSq::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
            }
        ss << ") {\n";
        ss << "    int gid0 = get_global_id(0);\n";
        ss << "    double vSum = 0.0;\n";
        ss << "    double vMean = 0.0;\n";
        ss << "    int cnt = 0;\n";
        for(size_t i = 0; i < vSubArguments.size(); i++ )
        {
        ss << "    double arg" << i << " = 0.0;\n";
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
        const formula::DoubleVectorRefToken* pCurDVR =
            static_cast<const formula::DoubleVectorRefToken* >(pCur);
        size_t nCurWindowSize = pCurDVR->GetRefRowSize();
        ss << "    for(int i = ";
        if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << i << ") || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << i << " = 0.0;\n";
            ss << "            --cnt;\n";
            ss << "        }\n";
            ss << "        ++cnt;\n";
            ss << "        vSum += arg" << i << ";\n";
            ss << "    }\n";
        } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << i << ") || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << i << " = 0.0;\n";
            ss << "            --cnt;\n";
            ss << "        }\n";
            ss << "        ++cnt;\n";
            ss << "        vSum += arg" << i << ";\n";
            ss << "    }\n";
        } else if (pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << i << ") || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << i << " = 0.0;\n";
            ss << "            --cnt;\n";
            ss << "        }\n";
            ss << "        ++cnt;\n";
            ss << "        vSum += arg" << i << ";\n";
            ss << "    }\n";
        } else {
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << i << ") || (i + gid0 >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << i << " = 0.0;\n";
            ss << "            --cnt;\n";
            ss << "        }\n";
            ss << "        ++cnt;\n";
            ss << "        vSum += arg" << i << ";\n";
            ss << "    }\n";
        }
}
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pTVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ") || gid0 >= " << pTVR->GetArrayLength() << ")\n";
            ss << "        arg" << i << " = 0;\n    else\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    cnt++;\n";
            ss << "    vSum += arg" << i << ";\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if(isNan ( ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        arg" << i << " = 0;\n    else\n";
            ss << "        arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    cnt++;\n";
            ss << "    vSum += arg" << i << ";\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    cnt++;\n";
            ss << "    vSum += arg" << i << ";\n";
        }
            }
        ss << "    vMean = vSum / cnt;\n";
        ss << "    vSum = 0.0;\n";
        for(size_t k = 0; k < vSubArguments.size(); k++ )
        {
        FormulaToken* pCur = vSubArguments[k]->GetFormulaToken();
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
        const formula::DoubleVectorRefToken* pCurDVR =
            static_cast<const formula::DoubleVectorRefToken* >(pCur);
        size_t nCurWindowSize = pCurDVR->GetRefRowSize();
        ss << "    for(int i = ";
        if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan( arg" << k << " ) || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << k << " = vXMean;\n";
            ss << "        }\n";
            ss << "        vSum += pow( arg" << k << " - vMean, 2 );\n";
            ss << "    }\n";
        } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
            ss << "0; i < gid0 + " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan( arg" << k << ") || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << k << " = vMean;\n";
            ss << "        }\n";
            ss << "        vSum += pow( arg" << k << " - vMean, 2 );\n";
            ss << "    }\n";
        } else if (pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << k << ") || (i >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << k << " = vMean;\n";
            ss << "        }\n";
            ss << "        vSum += pow( arg" << k << " - vMean, 2 );\n";
            ss << "    }\n";
        } else {
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            ss << "        arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "        if(isNan(arg" << k << ") || (i + gid0 >= ";
            ss << pCurDVR->GetArrayLength() << ")) {\n";
            ss << "            arg" << k << " = vMean;\n";
            ss << "        }\n";
            ss << "        vSum += pow( arg" << k << " - vMean, 2 );\n";
            ss << "    }\n";
        }
            }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pTVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(isNan(";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef();
            ss << ") || gid0 >= " << pTVR->GetArrayLength() << ")\n";
            ss << "        arg" << k << " = vMean;\n    else\n";
            ss << "        arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef()<<";\n";
            ss << "    vSum += pow( arg" << k << " - vMean, 2 );\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    if(isNan(arg" << k << "))\n";
            ss << "        arg" << k << " = vMean;\n";
            ss << "    vSum += pow( arg" << k << " - vMean, 2 );\n";
        }
        else
        {
            ss << "    arg" << k << " = ";
            ss << vSubArguments[k]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += pow( arg" << k << " - vMean, 2 );\n";
        }
            }
        ss << "    return vSum;\n";
        ss << "}";
}
void OpHypGeomDist::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0,arg1,arg2,arg3;\n";
    size_t i = vSubArguments.size();

    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n ";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
            ss << "    {\n";
            ss << "        if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    }\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"= 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg"<<i<<"= 0;\n";
            ss << "    else\n";
            ss << "        arg"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double N1=floor(arg3);\n"
    "    double M1=floor(arg2);\n"
    "    double n1=floor(arg1);\n"
    "    double x1=floor(arg0);\n"
    "    double num[9];\n"
    "    double PI = 3.1415926535897932384626433832795;\n"
    "    double tmp;\n"
    "    if( (x1 < 0.0) || (n1 < x1) || (M1 < x1) || (N1 < n1) ||"
    "(N1 < M1) || (x1 < n1 - N1 + M1) )\n"
    "    {\n"
    "        tmp = DBL_MIN;\n"
    "        return tmp;\n"
    "    }\n"
    "    num[0]=M1;\n"
    "    num[1]=x1;\n"
    "    num[2]=M1-x1;\n"
    "    num[3]=N1-M1;\n"
    "    num[4]=n1-x1;\n"
    "    num[5]=N1-M1-n1+x1;\n"
    "    num[6]=N1;\n"
    "    num[7]=n1;\n"
    "    num[8]=N1-n1;\n"
    "    for(int i=0;i<9;i++)\n"
    "    {\n"
    "        if(num[i]<171)\n"
    "        {\n"
    "            if(num[i]==0)\n"
    "                num[i]=0;\n"
    "            else\n"
    "                num[i]=log(tgamma(num[i])*num[i]);\n"
    "        }\n"
    "        else\n"
    "            num[i]=0.5*log(2.0*PI)+(num[i]+0.5)*log(num[i])-num[i]+"
    "(1.0*pow(12.0*num[i],-1)-1.0*pow(360*pow(num[i],3),-1));\n"
    "    }\n";
    ss << "    tmp=pow(M_E,(num[0]+num[3]+num[7]+num[8]";
    ss << "-num[1]-num[2]-num[4]-num[5]-num[6]));\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

enum MixDoubleString
{
    svDoubleVectorRefDoubleString,
    svDoubleVectorRefDouble,
    svDoubleVectorRefString,
    svDoubleVectorRefNULL,
    svSingleVectorRefDoubleString,
    svSingleVectorRefDouble,
    svSingleVectorRefString,
    svSingleVectorRefNULL,
    svDoubleDouble
};

void OpMinA::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixed = 0;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 1.79769e+308;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            if(pDVR->GetArrays()[0].mpNumericArray
                && pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefDoubleString;
            else if(pDVR->GetArrays()[0].mpNumericArray)
                isMixed = svDoubleVectorRefDouble;
            else if(pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefString;
            else
                isMixed = svDoubleVectorRefNULL;
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " && i < "<< nCurWindowSize << "; i++){\n";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);

            if(pSVR->GetArray().mpNumericArray
                && pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefDoubleString;
            else if(pSVR->GetArray().mpNumericArray)
                isMixed = svSingleVectorRefDouble;
            else if(pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefString;
            else
                isMixed = svSingleVectorRefNULL;
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
            isMixed = svDoubleDouble;
        }
        else
        {
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 > ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " ? ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " : tmp0;\n";
                ss << "        else if(isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ") && ";
                ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            tmp0 = tmp0 > 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleVectorRefDouble
                || isMixed == svSingleVectorRefDouble)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 > ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " : tmp0;";
                ss <<"\n    }\n";
            }
            else if(isMixed == svDoubleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " == 0)\n            continue;\n";
                ss << "        tmp0 = tmp0 > 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svSingleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            tmp0 = tmp0 > 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleDouble)
            {
                ss << "        tmp0 = tmp0 > ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " : tmp0;\n    }\n";
            }
            else
            {
                ss << "    }\n";
            }
        }
        else
        {
            ss << "        tmp0 = tmp0 > ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << " : tmp0;";
            ss <<"\n    }\n";
        }
    }
    ss << "    return tmp0 == 1.79769e+308 ? 0.0 : tmp0;\n";
    ss << "}\n";
}
void OpCountA::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double nCount = 0.0;\n";
    size_t i = vSubArguments.size();

    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            if(pDVR->GetArrays()[0].mpNumericArray
                && pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefDoubleString;
            else if(pDVR->GetArrays()[0].mpNumericArray)
                isMixed = svDoubleVectorRefDouble;
            else if(pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefString;
            else
                isMixed = svDoubleVectorRefNULL;
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " && i < "<< nCurWindowSize << "; i++){\n";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);

            if(pSVR->GetArray().mpNumericArray
                && pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefDoubleString;
            else if(pSVR->GetArray().mpNumericArray)
                isMixed = svSingleVectorRefDouble;
            else if(pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefString;
            else
                isMixed = svSingleVectorRefNULL;
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
            isMixed = svDoubleDouble;
        }
        else
        {
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
                ss << "        else if(isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ") && ";
                ss<< vSubArguments[i]->GenStringSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleVectorRefDouble
                || isMixed == svSingleVectorRefDouble)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            nCount+=1.0;\n";
                ss <<"}\n    }\n";
            }
            else if(isMixed == svDoubleVectorRefString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            nCount+=1.0;\n";
                ss <<"\n    }\n";
            }
            else if(isMixed == svSingleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleDouble)
            {
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    }\n";
            }
        }
        else
        {
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
        }
    }
    ss << "    return nCount;\n";
    ss << "}\n";
}
void OpMaxA::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 2.22507e-308;\n";
    size_t i = vSubArguments.size();

    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            if(pDVR->GetArrays()[0].mpNumericArray
                && pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefDoubleString;
            else if(pDVR->GetArrays()[0].mpNumericArray)
                isMixed = svDoubleVectorRefDouble;
            else if(pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefString;
            else
                isMixed = svDoubleVectorRefNULL;
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " && i < "<< nCurWindowSize << "; i++){\n";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);

            if(pSVR->GetArray().mpNumericArray
                && pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefDoubleString;
            else if(pSVR->GetArray().mpNumericArray)
                isMixed = svSingleVectorRefDouble;
            else if(pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefString;
            else
                isMixed = svSingleVectorRefNULL;
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
            isMixed = svDoubleDouble;
        }
        else
        {
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 < ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " ? ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " : tmp0;\n";
                ss << "        else if(isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ") && ";
                ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            tmp0 = tmp0 < 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleVectorRefDouble
                || isMixed == svSingleVectorRefDouble)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 < ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " : tmp0;";
                ss <<"\n    }\n";
            }
            else if(isMixed == svDoubleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " == 0)\n            continue;\n";
                ss << "        tmp0 = tmp0 < 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svSingleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            tmp0 = tmp0 < 0.0 ? 0.0 : tmp0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleDouble)
            {
                ss << "        tmp0 = tmp0 < ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " : tmp0;\n    }\n";
            }
            else
            {
                ss << "    }\n";
            }
        }
        else
        {
            ss << "        tmp0 = tmp0 < ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << " ? " << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << " : tmp0;";
            ss <<"\n    }\n";
        }
    }
    ss << "    return tmp0 == 2.22507e-308 ? 0.0 : tmp0;\n";
    ss << "}\n";
}
void OpAverageA::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 0.0;\n";
    ss << "    double nCount = 0.0;\n";
    size_t i = vSubArguments.size();
    ss <<"\n";
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            if(pDVR->GetArrays()[0].mpNumericArray
                && pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefDoubleString;
            else if(pDVR->GetArrays()[0].mpNumericArray)
                isMixed = svDoubleVectorRefDouble;
            else if(pDVR->GetArrays()[0].mpStringArray)
                isMixed = svDoubleVectorRefString;
            else
                isMixed = svDoubleVectorRefNULL;
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " && i < "<< nCurWindowSize << "; i++){\n";
            }
            else {
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);

            if(pSVR->GetArray().mpNumericArray
                && pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefDoubleString;
            else if(pSVR->GetArray().mpNumericArray)
                isMixed = svSingleVectorRefDouble;
            else if(pSVR->GetArray().mpStringArray)
                isMixed = svSingleVectorRefString;
            else
                isMixed = svSingleVectorRefNULL;
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
            isMixed = svDoubleDouble;
        }
        else
        {
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            tmp0 +=";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ";\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
                ss << "        else if(isNan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ") && ";
                ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleVectorRefDouble
                || isMixed == svSingleVectorRefDouble)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            tmp0 +=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "            nCount+=1.0;\n";
                ss <<"}\n    }\n";
            }
            else if(isMixed == svDoubleVectorRefString)
            {
                ss << "        if (!isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            nCount+=1.0;\n";
                ss <<"\n    }\n";
            }
            else if(isMixed == svSingleVectorRefString)
            {
                ss << "        if(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << " != 0)\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else if(isMixed == svDoubleDouble)
            {
                ss << "            tmp0 +=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
            }
            else
            {
                ss << "    }\n";
            }
        }
        else
        {
                ss << "            tmp0 +=";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ";\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
        }
    }
    ss << "    return tmp0*pow(nCount,-1);\n";
    ss << "}\n";
}
void OpVarA::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                assert(pDVR);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "            continue;\n";
                    ss << "        }\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }

            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                assert(pSVR);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount += 1.0;\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {

                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg =0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount = fCount + 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount = fCount + 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                   ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                   ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg = 0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return vSum * pow(fCount - 1.0,-1.0);\n";
    ss << "}\n";
}

void OpVarPA::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "            continue;\n";
                    ss << "        }\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }

            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount += 1.0;\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {

                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg =0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount = fCount + 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount = fCount + 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                   ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                   ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg = 0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount == 0.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return vSum * pow(fCount,-1.0);\n";
    ss << "}\n";
}
void OpStDevA::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "            continue;\n";
                    ss << "        }\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }

            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount += 1.0;\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {

                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg =0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount = fCount + 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount = fCount + 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                   ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                   ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg = 0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum * pow(fCount - 1.0,-1.0));\n";
    ss << "}\n";
}

void OpStDevPA::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << "){\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }

                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "            continue;\n";
                    ss << "        }\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        fSum += arg;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }

            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount += 1.0;\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {

                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg =0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    fSum += arg;\n";
                ss << "    fCount = fCount + 1.0;\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    fSum += arg;\n";
            ss << "    fCount = fCount + 1.0;\n";
        }
        if (i == 0)
        {
            ss << "    fMean = fSum * pow(fCount,-1.0);\n";
        }
    }
    i = vSubArguments.size();
    while (i--)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(pCur);
                if(pDVR->GetArrays()[0].mpNumericArray
                    && pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefDoubleString;
                else if(pDVR->GetArrays()[0].mpNumericArray)
                    isMixedDV = svDoubleVectorRefDouble;
                else if(pDVR->GetArrays()[0].mpStringArray)
                    isMixedDV = svDoubleVectorRefString;
                else
                    isMixedDV = svDoubleVectorRefNULL;

                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                    ss << "    {\n";
                }
                if(isMixedDV == svDoubleVectorRefDoubleString)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "            arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefDouble)
                {
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (isNan(arg))\n";
                    ss << "            continue;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";

                }
                else if(isMixedDV == svDoubleVectorRefString)
                {
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        arg = 0.0;\n";
                    ss << "        vSum += (arg - fMean) * (arg - fMean);\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "        continue;\n";
                    ss << "    }\n";
                }
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken* >(pCur);
                if(pSVR->GetArray().mpNumericArray
                    && pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefDoubleString;
                else if(pSVR->GetArray().mpNumericArray)
                    isMixedSV = svSingleVectorRefDouble;
                else if(pSVR->GetArray().mpStringArray)
                    isMixedSV = svSingleVectorRefString;
                else
                    isMixedSV = svSingleVectorRefNULL;

                if(isMixedSV == svSingleVectorRefDoubleString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                    ss << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isNan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefDouble)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                   ss << "    {\n";
                    ss << "        arg = ";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                    ss << "        if (!isNan(arg))\n";
                    ss << "        {\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                   ss << "    }\n";
                }
                else if(isMixedSV == svSingleVectorRefString)
                {
                    ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                    ss << "    {\n";
                    ss << "        if (";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << " != 0)\n";
                    ss << "        {\n";
                    ss << "            arg = 0.0;\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        }\n";
                    ss << "    }\n";
                }
                else
                {
                    ss << "    arg = 0.0;\n";
                }
            }
            else
            {
                ss << "    arg = " << pCur->GetDouble() << ";\n";
                ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
            }
        }
        else
        {
            ss << "    arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    vSum += (arg - fMean) * (arg - fMean);\n";
        }
    }
    ss << "    if (fCount == 1.0)\n";
    ss << "        return DBL_MAX;\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum * pow(fCount,-1.0));\n";
    ss << "}\n";
}

void OpAveDev:: GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss <<"{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double sum=0.0;\n";
    ss << "    double length;\n";
    ss << "    double totallength=0;\n";
    ss << "    double tmp = 0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    length="<<nCurWindowSize;
            ss << ";\n";
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            length-=1.0;\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        sum +=  arg"<<i<<";\n";
            ss << "    }\n";
            ss << "    totallength +=length;\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss << "        sum += tmp;\n";
            ss << "        totallength +=1;\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss << "    sum += tmp;\n";
           ss << "    totallength +=1;\n";
        }
    }
    ss << "    double mean = sum * pow(totallength,-1);\n";
    ss << "    sum = 0.0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            ss << "0; i < "<< nCurWindowSize << "; i++)\n";
            ss << "    {\n";
            ss << "        double arg"<<i<<" = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if(isNan(arg"<<i<<")||((gid0+i)>=";
            ss << pDVR->GetArrayLength();
            ss << "))\n";
            ss << "        {\n";
            ss << "            continue;\n";
            ss << "        }\n";
            ss << "        sum +=  fabs(arg"<<i<<"-mean);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    tmp = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(!isNan(tmp))\n";
            ss << "    {\n";
            ss << "        sum += fabs(tmp-mean);\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
           ss << "    tmp = ";
           ss << vSubArguments[i]->GenSlidingWindowDeclRef();
           ss << ";\n";
           ss << "    sum += fabs(tmp-mean);\n";
        }
    }
    ss << "    tmp=sum*pow(totallength,-1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
