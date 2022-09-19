/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_statistical.hxx"

#include <formula/vectortoken.hxx>
#include <sstream>
#include "op_statistical_helpers.hxx"

using namespace formula;

namespace sc::opencl {
void OpVar::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return vSum / (fCount - 1.0);\n";
    ss << "}\n";
}

void OpVarP::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    ss << "    double arg = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    if (fCount == 0.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return vSum / fCount;\n";
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
void OpZTest::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fSumSqr = 0.0;\n";
    ss << "    double mue = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArg( 0, vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fSumSqr += arg * arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    if(fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    mue = fSum / fCount;\n";
    GenerateArg( "mu", 1, vSubArguments, ss );
    if(vSubArguments.size() == 3)
        GenerateArg( "sigma", 2, vSubArguments, ss );
    else
        ss << "    double sigma = (fSumSqr-fSum*fSum/fCount)/(fCount-1.0);\n";
    ss << "    return 0.5 - gauss((mue-mu)/sqrt(sigma/fCount));\n";
    ss << "}\n";
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

void OpTTest::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
                ss << "        if (isnan(mode))\n";
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
                ss << "        if (isnan(";
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
                ss << "            if (isnan(arg1)||isnan(arg2))\n";
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
            ss << "            if (isnan(arg1))\n";
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
            ss << "            if (isnan(arg2))\n";
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
void OpTDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "fDF", 1, vSubArguments, ss );
    GenerateArg( "fFlag", 2, vSubArguments, ss );
    ss << "    fDF = floor( fDF );\n";
    ss << "    fFlag = floor( fFlag );\n";
    ss << "    if(fDF < 1.0 || x < 0.0 || (fFlag != 1.0 && fFlag != 2.0))\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double R = GetTDist(x, fDF);\n";
    ss << "    if (fFlag == 1.0)\n";
    ss << "        return R;\n";
    ss << "    else\n";
    ss << "        return 2.0 * R;\n";
    ss << "}\n";
}

void OpExponDist::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "rx", 0, vSubArguments, ss );
    GenerateArg( "rlambda", 1, vSubArguments, ss );
    GenerateArg( "rkum", 2, vSubArguments, ss );
    ss <<"    if(rlambda <= 0.0)\n";
    ss <<"        return CreateDoubleError(IllegalArgument);\n";
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
void OpFdist::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "rX", 0, vSubArguments, ss );
    GenerateArg( "rF1", 1, vSubArguments, ss );
    GenerateArg( "rF2", 2, vSubArguments, ss );
    ss <<"    rF1 = floor(rF1);\n";
    ss <<"    rF2 = floor(rF2);\n";
    ss <<"    if (rX < 0.0 || rF1 < 1.0 || rF2 < 1.0 || rF1 >= 1.0E10 ||";
    ss <<"rF2 >= 1.0E10)\n";
    ss <<"        return CreateDoubleError(IllegalArgument);\n";
    ss <<"    tmp = GetFDist(rX, rF1, rF2);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpStandard::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "mu", 0, vSubArguments, ss );
    GenerateArg( "sigma", 0, vSubArguments, ss );
    ss << "    if(sigma < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else if(sigma == 0.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return (x - mu)/sigma;\n";
    ss << "}";
}

void OpWeibull::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "alpha", 1, vSubArguments, ss );
    GenerateArg( "beta", 2, vSubArguments, ss );
    GenerateArg( "kum", 3, vSubArguments, ss );
    ss << "    if(alpha <= 0.0 || beta <=0.0 || kum < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else if(kum == 0.0)\n";
    ss << "    {\n";
    ss << "        return alpha*pow(pow(beta,alpha),-1.0)*pow(x,alpha-1.0)";
    ss << "*exp(-pow(x*pow(beta,-1.0),alpha));\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "        return 1.0-exp(-pow(x*pow(beta,-1.0),alpha));\n";
    ss << "}\n";
}

void OpSkew::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    if(fCount <= 2.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    double fStdDev = sqrt(vSum / (fCount - 1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            dx = (arg - fMean) / fStdDev;\n"
        "            xcube = xcube + dx * dx * dx;\n"
        "        }\n"
        );
    ss << "    return ((xcube * fCount) / (fCount - 1.0))";
    ss << " / (fCount - 2.0);\n";
    ss << "}\n";
}

void OpSkewp::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    if(fCount <= 2.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    double fStdDev = sqrt(vSum / fCount);\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            dx = (arg - fMean) / fStdDev;\n"
        "            xcube = xcube + dx * dx * dx;\n"
        "        }\n"
        );
    ss << "    return xcube / fCount;\n";
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

void OpTInv::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "fDF", 1, vSubArguments, ss );
    ss << "    fDF = floor(fDF);\n";
    ss << "    if (x > 1.0||fDF < 1.0 || fDF > 1.0E10 || x <= 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    bool bConvError;\n";
    ss << "    double fVal = lcl_IterateInverse(\n";
    ss << "        fDF*0.5, fDF, &bConvError,x,fDF );\n";
    ss << "    if (bConvError)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return fVal;\n";
    ss << "}\n";
}

void OpStDev::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum / (fCount - 1.0));\n";
    ss << "}\n";
}

void OpStDevP::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double fMean = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            fCount += 1.0;\n"
        "        }\n"
        );
    ss << "    fMean = fSum / fCount;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if (!isnan(arg))\n"
        "            vSum += (arg - fMean) * (arg - fMean);\n"
        );
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum / fCount);\n";
    ss << "}\n";
}

void OpSlope::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(2,2);
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
            throw Unhandled(__FILE__, __LINE__);
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
            throw Unhandled(__FILE__, __LINE__);
        }

        ss << "        argX = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        argY = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "        if (isnan(argX) || isnan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 1.0)\n";
        ss << "        return CreateDoubleError(NoValue);\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX / fCount;\n";
        ss << "        fMeanY = fSumY / fCount;\n";

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
        ss << "            if (isnan(argX) || isnan(argY))\n";
        ss << "                 continue;\n";
        ss << "            fSumDeltaXDeltaY += (argX-fMeanX)*(argY-fMeanY);\n";
        ss << "            fSumSqrDeltaX += (argX-fMeanX) * (argX-fMeanX);\n";
        ss << "        }\n";
        ss << "        if(fSumSqrDeltaX == 0.0)\n";
        ss << "            return CreateDoubleError(DivisionByZero);\n";
        ss << "        else\n";
        ss << "        {\n";
        ss << "            return fSumDeltaXDeltaY / fSumSqrDeltaX;\n";
        ss << "        }\n";
        ss << "    }\n";
        ss << "}\n";
    }
    else
    {
        throw Unhandled(__FILE__, __LINE__);
    }
}
void OpSTEYX::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
        ss << "        if (isnan(argX) || isnan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 3.0)\n";
        ss << "        return DBL_MAX;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX / fCount;\n";
        ss << "        fMeanY = fSumY / fCount;\n";

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
        ss << "            if (isnan(argX)||isnan(argY))\n";
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
        ss << "                   fSumDeltaXDeltaY / fSumSqrDeltaX)\n";
        ss << "                   /(fCount - 2.0));\n";
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
void OpFisher::GenSlidingWindowFunction( outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "    if (fabs(arg0) >= 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double tmp=0.5*log((1+arg0)/(1-arg0));\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpFisherInv::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "    double tmp=tanh(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpGamma::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"     int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "    double tmp=tgamma(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpCorrel::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpCorrelfor GPU calculating.
        throw Unhandled(__FILE__, __LINE__);
    const formula::DoubleVectorRefToken* pCurDVRX =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVRY =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pCurDVRX->GetRefRowSize() != pCurDVRY->GetRefRowSize() )
         throw Unhandled(__FILE__, __LINE__);

    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i + gid0 >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i >= ";
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
            ss << "if(isnan(arg0) || isnan(arg1) || (i + gid0 >= ";
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
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << " int gid0=get_global_id(0);\n";
    GenerateArg( "f", 0, vSubArguments, ss );
    GenerateArg( "s", 0, vSubArguments, ss );
    GenerateArg( "p", 0, vSubArguments, ss );
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
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpPearson for GPU calculating.
        throw Unhandled(__FILE__, __LINE__);
    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVRY =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pDVR->GetRefRowSize() != pCurDVRY->GetRefRowSize() )
         throw Unhandled(__FILE__, __LINE__);

    size_t nCurWindowSize = pDVR->GetRefRowSize();

    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
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
    ss << " if(!isnan(fInx)&&!isnan(fIny)){\n";
    ss << "       fSumX += fInx;\n";
    ss << "       fSumY += fIny;\n";
    ss << "       fCount = fCount + 1;\n";
    ss << "      }\n";
    ss << "     }\n";
    ss << " if(fCount < 1)\n";
    ss << "   return CreateDoubleError(NoValue);\n";
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
    ss << " if(!isnan(fInx)&&!isnan(fIny)){\n";
    ss << "           fSumDeltaXDeltaY += (fInx - fMeanX) * (fIny - fMeanY);\n";
    ss << "           fSumX += (fInx - fMeanX) * (fInx - fMeanX);\n";
    ss << "           fSumY += (fIny - fMeanY) * (fIny - fMeanY);\n";
    ss << "         }\n";
    ss << "       }\n";
    ss << "      if (fSumX == 0 || fSumY == 0)\n";
    ss << "          return CreateDoubleError(DivisionByZero);\n";
    ss << "      double tmp = ( fSumDeltaXDeltaY / sqrt( fSumX * fSumY));\n";
    ss << "      return tmp;\n";
    ss << "}\n";
}

void OpGammaLn::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    GenerateArg( 0, vSubArguments, ss );
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
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "    double tmp=gauss(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpGeoMean::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
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

    for(const DynamicKernelArgumentRef & rArg : vSubArguments)
    {
        assert(rArg->GetFormulaToken());

        if(rArg->GetFormulaToken()->GetType() ==
        formula::svDoubleVectorRef)
        {
            FormulaToken *tmpCur = rArg->GetFormulaToken();
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
                throw Unhandled(__FILE__, __LINE__);
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
            rArg->GenDeclRef(ss);
            ss << "["<<p1.c_str()<<"];\n";
            ss << "        if(!isnan(tmp0))\n";
            ss << "       {\n";
            ss << "           tmp += log(tmp0);\n";
            ss << "           count++;\n";
            ss << "       }\n";

            ss << "        tmp0 =";
            rArg->GenDeclRef(ss);
            ss << "["<<p2.c_str()<<"];\n";
            ss << "        if(!isnan(tmp0))\n";
            ss << "       {\n";
            ss << "           tmp += log(tmp0);\n";
            ss << "           count++;\n";
            ss << "       }\n";

            ss << "        }\n";
            ss << "        else if (p1 < min(arrayLength, offset + windowSize)) {\n";

            ss << "        tmp0 =";
            rArg->GenDeclRef(ss);
            ss << "["<<p1.c_str()<<"];\n";
            ss << "        if(!isnan(tmp0))\n";
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
            //ss <<"printf(\"\\n********************sum is %f, count is%d\",current_sum,current_count);\n";
            ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
            ss << "    }\n";
        }else
        {
            ss << "    if (lidx == 0)\n";
            ss << "    {\n";
            ss << "        tmp0 =";
            if(rArg->GetFormulaToken()->GetType() == formula::svSingleVectorRef)
            {
                rArg->GenDeclRef(ss);
                ss << "[writePos];\n";
            }
            else
            {
                rArg->GenDeclRef(ss);
                ss <<";\n";
                //ss <<"printf(\"\\n********************tmp0 is %f\",tmp0);\n";
            }
            ss << "        if(!isnan(tmp0))\n";
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

    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    tmp =";
    vSubArguments[0]->GenDeclRef(ss);
    ss << "[gid0];\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpHarMean::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double nVal=0.0;\n";
    ss << "    double tmp = 0;\n";
    ss << "    int length;\n";
    ss << "    int totallength=0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "        {\n"
        "            nVal += (1.0 / arg);\n"
        "            ++totallength;\n"
        "        }\n"
        );
    ss << "    tmp = totallength/nVal;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpConfidence::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpConfidence::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "alpha", 0, vSubArguments, ss );
    GenerateArg( "sigma", 0, vSubArguments, ss );
    GenerateArg( "size", 0, vSubArguments, ss );
    ss << "    double rn = floor(size);\n";
    ss << "    if(sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0";
    ss << "|| rn < 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else\n";
    ss << "        tmp = gaussinv(1.0 - alpha / 2.0) * sigma / sqrt( rn );\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCritBinom::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(MinDecl);
    funs.insert("");
}

void OpCritBinom::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "n", 0, vSubArguments, ss );
    GenerateArg( "p", 1, vSubArguments, ss );
    GenerateArg( "alpha", 2, vSubArguments, ss );
    ss << "    double rn = floor(n);\n";
    ss << "    if (rn < 0.0 || alpha <= 0.0 || alpha >= 1.0 || p < 0.0";
    ss << " || p > 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        double rq = (0.5 - p) + 0.5;\n";
    ss << "        double fFactor = pow(rq, rn);\n";
    ss << "        if (fFactor <= Min)\n";
    ss << "        {\n";
    ss << "            fFactor = pow(p, rn);\n";
    ss << "            if (fFactor <= Min)\n";
    ss << "                return CreateDoubleError(NoValue);\n";
    ss << "            else\n";
    ss << "            {\n";
    ss << "                double fSum = 1.0 - fFactor;\n";
    ss << "                uint max =(uint)(rn), i;\n";
    ss << "                for (i = 0; i < max && fSum >= alpha; i++)\n";
    ss << "                {\n";
    ss << " fFactor *= (rn - i) / (double)(i + 1) * rq / p;\n";
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
    ss << " fFactor *= (rn - i) / (double)(i + 1) *";
    ss << " p / rq;\n";
    ss << "                fSum += fFactor;\n";
    ss << "            }\n";
    ss << "            tmp = (i);\n";
    ss << "        }\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpRsq::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    if( vSubArguments.size() !=2 ||vSubArguments[0]->GetFormulaToken()
        ->GetType() != formula::svDoubleVectorRef||vSubArguments[1]
        ->GetFormulaToken()->GetType() != formula::svDoubleVectorRef )
        ///only support DoubleVector in OpRsq for GPU calculating.
        throw Unhandled(__FILE__, __LINE__);
    const formula::DoubleVectorRefToken* pCurDVR1 =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[0]->GetFormulaToken());
    const formula::DoubleVectorRefToken* pCurDVR2 =
        static_cast<const formula::DoubleVectorRefToken *>(
        vSubArguments[1]->GetFormulaToken());
    if(  pCurDVR1->GetRefRowSize() != pCurDVR2->GetRefRowSize() )
         throw Unhandled(__FILE__, __LINE__);

    size_t nCurWindowSize = pCurDVR1->GetRefRowSize();

    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
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
    ss << "     if(isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << "))\n";
    ss << "         fInx = 0;\n";
    ss << "     else\n";
    ss << "        fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "      if(isnan(";
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
    ss << "     if(isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef(true);
    ss << "))\n";
    ss << "         fInx = 0;\n";
    ss << "     else\n";
    ss << "        fInx = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "      if(isnan(";
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp;\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss <<"\n    ";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    ss << "    tmp1 = floor(tmp1);";
    ss << "    if (tmp1 < 1.0 || tmp0 <= 0.0 || tmp0 > 1.0 )\n";
    ss << "    {\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    }\n";
    ss << "    bool bConvError;\n";
    ss << "    double fVal = lcl_IterateInverseChiInv";
    ss << "(tmp0, tmp1, tmp1*0.5, tmp1, &bConvError);\n";
    ss << "    if(bConvError)\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "    return fVal;\n";
    ss << "}\n";
}
void OpNormdist::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(3,4);
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "mue", 1, vSubArguments, ss );
    GenerateArg( "sigma", 2, vSubArguments, ss );
    GenerateArg( "c", 3, vSubArguments, ss );
    ss << "if(sigma <= 0)\n";
    ss << "    return CreateDoubleError(IllegalArgument);\n";
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    ss << "    double tmp = 0.5 * erfc((-1)*x * 0.7071067811865475);\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpPermut::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double tmp = 1 ;\n";
    GenerateArg( "inA", 0, vSubArguments, ss );
    GenerateArg( "inB", 1, vSubArguments, ss );
    ss << "      for( int i =0; i<inB; i++)\n";
    ss << "      {\n";
    ss << "        tmp *= inA ;\n";
    ss << "        inA = inA - 1.0;\n";
    ss << "      }\n";
    ss << "      return tmp;\n";
    ss << "}\n";
}
void OpPermutationA::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double tmp = 1.0;\n";
    GenerateArg( "inA", 0, vSubArguments, ss );
    GenerateArg( "inB", 1, vSubArguments, ss );
    ss << " for(int i=0; i<inB; i++)\n";
    ss << " {\n";
    ss << "     tmp *= inA;\n";
    ss << " }\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpPhi::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    ss << "    double tmp = 0.39894228040143268 * exp((-1)*pow(x,2) / 2.0);\n";
    ss << "     return tmp;\n";
    ss << "}\n";
}

void OpNorminv::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    double q,t,z;\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "mue", 1, vSubArguments, ss );
    GenerateArg( "sigma", 2, vSubArguments, ss );
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
    (outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double q,t,z;\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
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
    ss <<"    return CreateDoubleError(NoValue);\n";
    ss <<"return z;\n";
    ss <<"}\n";
}
void OpMedian::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    ss << "    int i;\n";
    ss << "    unsigned int startFlag = 0;\n";
    ss << "    unsigned int endFlag = 0;\n";
    ss << "    double dataIna;\n";
    for (const DynamicKernelArgumentRef & rArg : vSubArguments)
    {
        FormulaToken *pCur = rArg->GetFormulaToken();
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
    ss<<"if((i+gid0)>=buffer_fIna_len || isnan(";
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
void OpKurt:: GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double totallength=0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "        {\n"
        "            fSum += arg;\n"
        "            totallength +=1;\n"
        "        }\n"
        );
    ss << "    double fMean = fSum / totallength;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "            vSum += (arg-fMean)*(arg-fMean);\n"
        );
    ss << "    double fStdDev = sqrt(vSum / (totallength - 1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xpower4 = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "        {\n"
        "            dx = (arg -fMean) / fStdDev;\n"
        "            xpower4 = xpower4 + (dx * dx * dx * dx);\n"
        "        }\n"
        );
    ss<< "    double k_d = (totallength - 2.0) * (totallength - 3.0);\n";
    ss<< "    double k_l = totallength * (totallength + 1.0) /";
    ss<< "((totallength - 1.0) * k_d);\n";
    ss<< "    double k_t = 3.0 * (totallength - 1.0) * ";
    ss<< "(totallength - 1.0) / k_d;\n";
    ss<< "    return xpower4 * k_l - k_t;\n";
    ss << "}";
}

void OpIntercept::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
        ss << "        if (isnan(argX) || isnan(argY))\n";
        ss << "            continue;\n";
        ss << "        fSumX += argX;\n";
        ss << "        fSumY += argY;\n";
        ss << "        fCount += 1.0;\n";
        ss << "    }\n";

        ss << "    if (fCount < 1.0)\n";
        ss << "        return NAN;\n";
        ss << "    else\n";
        ss << "    {\n";
        ss << "        fMeanX = fSumX / fCount;\n";
        ss << "        fMeanY = fSumY / fCount;\n";

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
        ss << "            if (isnan(argX) || isnan(argY))\n";
        ss << "                 continue;\n";
        ss << "            fSumDeltaXDeltaY += (argX-fMeanX)*(argY-fMeanY);\n";
        ss << "            fSumSqrDeltaX += (argX-fMeanX) * (argX-fMeanX);\n";
        ss << "        }\n";
        ss << "        if(fSumSqrDeltaX == 0.0)\n";
        ss << "            return NAN;\n";
        ss << "        else\n";
        ss << "        {\n";
        ss << "            return fMeanY -";
        ss << " (fSumDeltaXDeltaY/fSumSqrDeltaX)*fMeanX;\n";
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
void OpLogInv:: GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg1", 1, 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg2", 2, 1, vSubArguments, ss );
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

void OpForecast::GenSlidingWindowFunction(outputstream &ss,
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
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
        ss<< "    if(isnan(arg0)||(gid0>=";
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
        ss << "        if(isnan(arg1)||((gid0+i)>=";
        ss << pCurDVR1->GetArrayLength();
        ss << "))\n";
        ss << "        {\n";
        ss << "            length--;\n";
        ss << "            continue;\n";
        ss << "        }\n";
        ss << "        if(isnan(arg2)||((gid0+i)>=";
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
        ss << "        if(isnan(arg1)||((gid0+i)>=";
        ss <<pCurDVR1->GetArrayLength();
        ss <<"))\n";
        ss <<"        {\n";
        ss <<"            continue;\n";
        ss <<"        }\n";
        ss << "        if(isnan(arg2)||((gid0+i)>=";
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
void OpLogNormDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg1", 1, 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg2", 2, 1, vSubArguments, ss );
    GenerateArgWithDefault( "arg3", 3, 1, vSubArguments, ss );
    ss << "    double tmp;\n";
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

void OpGammaDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArgWithDefault( "arg3", 3, 1, vSubArguments, ss );
    ss << "    double tmp;\n";
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double fx,fDF,tmp=0,tmp0=0,tmp1=0;\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    ss << "    fx = tmp0;\n";
    ss << "    fDF = floor(tmp1);\n";
    ss << "    if(fDF < 1.0)\n";
    ss << "    {\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    ss << "    tmp0 = floor(tmp0);\n";
    ss << "    tmp1 = floor(tmp1);\n";
    ss << "    double rq = (0.5 - tmp2) + 0.5;\n";
    ss << "    if (tmp1 < 0.0 || tmp0 < 0.0 || tmp0 > tmp1 ||";
    ss << "tmp2 < 0.0 || tmp2 > 1.0)\n";
    ss << "    {\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    ss << "                           fFactor *= (tmp1 - i)/(i + 1)*rq/tmp2;\n";
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
    decls.insert(fHalfMachEpsDecl);
    decls.insert(fBigInvDecl);

    funs.insert(GetGammaContFraction);funs.insert(GetChiSqDistCDF);
    funs.insert(GetChiSqDistPDF);funs.insert(GetLowRegIGamma);
    funs.insert(GetGammaSeries);
}

void OpChiSqDist::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArgWithDefault( "tmp2", 2, 1, vSubArguments, ss );
    ss << "    tmp1 = floor(tmp1);\n";
    ss << "    if(tmp1 < 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    decls.insert(fHalfMachEpsDecl);
    decls.insert(fBigInvDecl);decls.insert(lcl_HasChangeOfSignDecl);
    decls.insert(fMachEpsDecl);

    funs.insert(GetGammaContFraction);funs.insert(GetChiSqDistCDF);
    funs.insert(GetLowRegIGamma);funs.insert(lcl_HasChangeOfSign);
    funs.insert(GetGammaSeries);funs.insert(lcl_IterateInverseChiSQInv);
}

void OpChiSqInv::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    ss << "    tmp1 = floor(tmp1);\n";
    ss << "    bool bConvError;\n";
    ss << "    if(tmp1 < 1.0 || tmp0 < 0 || tmp0>=1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        result =lcl_IterateInverseChiSQInv( tmp0, tmp1,";
    ss << "tmp1*0.5, tmp1, &bConvError );\n";
    ss << "    }\n";
    ss << "    if(bConvError)\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "    return result;\n";
    ss << "}";
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

void OpGammaInv::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    ss << "    if( arg0 < 0 || arg0 >= 1 || arg1 <= 0 || arg2 <= 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    "                    fSx = fPx * fRy * fQy / (fRy-fPy) / (fQy-fPy)"
    "+ fRx * fQy * fPy / (fQy-fRy) / (fPy-fRy)"
    "+ fQx * fPy * fRy / (fPy-fQy) / (fRy-fQy);\n"
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

void OpFInv::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    ss << "    double fF2=floor(arg2);\n"
    "    double fF1=floor(arg1);\n"
    "    if( arg0 <= 0 || arg1 < 1 || arg2 < 1 || arg1 >= 1.0e10 || arg2 >= 1.0e10 || arg > 1 )\n"
    "        return CreateDoubleError(IllegalArgument);\n"
    "    double fAx=fF1*0.5;\n"
    "    double fBx=fF1;\n"
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
    "        return CreateDoubleError(NoConvergence);\n"
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
    "                fSx = fPx * fRy * fQy / (fRy-fPy)"
    " / (fQy-fPy)+fRx * fQy * fPy / (fQy-fRy)"
    " / (fPy-fRy)+ fQx * fPy * fRy / (fPy-fQy)"
    " / (fRy-fQy);\n"
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
void OpFTest::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum1 = 0.0;\n";
    ss << "    double fSumSqr1 = 0.0;\n";
    ss << "    double fSum2 = 0.0;\n";
    ss << "    double fSumSqr2 = 0.0;\n";
    ss << "    double tmp = 0;\n";
    GenerateRangeArg( 0, vSubArguments, ss,
        "        if( !isnan(arg))\n"
        "        {\n"
        "            fSum1 += arg;\n"
        "            fSumSqr1 += arg * arg;\n"
        "        }\n"
        );
    GenerateRangeArg( 1, vSubArguments, ss,
        "        if( !isnan(arg))\n"
        "        {\n"
        "            fSum2 += arg;\n"
        "            fSumSqr2 += arg * arg;\n"
        "        }\n"
        );
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

void OpB::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double min = 2.22507e-308;\n";
    ss << "    double tmp;\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp;\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "lambda", 1, vSubArguments, ss );
    GenerateArgWithDefault( "bCumulative", 2, 1, vSubArguments, ss );
    ss << "    x = floor(x);\n";
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
    ss << "          fPoissonVar *= lambda / ( (double)f + 1.0 );\n";
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
    ss << "                fSummand = (fSummand*lambda)/((double)i);\n";
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
void OpCovar::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
        CHECK_PARAMETER_COUNT(2,2);
        GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
        ss << "{\n";
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
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isnan("<< vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << " && i + gid0 < " << pCurDVRY->GetArrayLength();
                ss << "; i++) {\n";
                ss << "if ((isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ")) || ";
                ss << "(isnan("<< vSubArguments[1]->GenSlidingWindowDeclRef();
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
            ss << "        return CreateDoubleError(NoValue);\n";
            ss << "    }\n";
            ss << "    else {\n";
            ss << "        vMean0 = vSum0 / cnt;\n";
            ss << "        vMean1 = vSum1 / cnt;\n";
            ss << "    for(";
            if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "int i = gid0; i < " << nCurWindowSizeX;
                ss << " && i < " << pCurDVRX->GetArrayLength() << " && i < ";
                ss << pCurDVRY->GetArrayLength() << "; i++){\n";
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") ||";
                ss << " isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") || ";
                ss << "isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << "        if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") || ";
                ss << "isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
                ss << "if((isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ")) || ";
                ss << "(isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef();
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
        else
        {
            throw Unhandled(__FILE__, __LINE__);
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
                ss << "    if(isnan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ") || gid0 >= " << pTVR->GetArrayLength() << ")\n";
                ss << "        arg" << i << " = 0;\n    else\n";
                ss << "        arg" << i << " = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                ss << "    cnt" << i << "++;\n";
                ss << "    vSum" << i << " += arg" << i << ";\n";
            }
            else if (pCur->GetType() == formula::svDouble){
                ss << "    if(isnan ( ";
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
                ss << "    if(isnan(";
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
                ss << "    if(isnan(arg" << i << "))\n";
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
void OpBetaDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArgWithDefault( "arg3", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg4", 4, 1, vSubArguments, ss );
    GenerateArgWithDefault( "arg5", 5, 1, vSubArguments, ss );
    ss << "    double fScale = arg4 - arg3;\n"
    "    if (fScale <= 0.0 || arg1 <= 0.0 || arg2 <= 0.0)\n"
    "        return CreateDoubleError(IllegalArgument);\n"
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
    "        arg0 = (arg0-arg3)/fScale;\n"
    "        tmp =  GetBetaDist(arg0, arg1, arg2);\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        if (arg0 < arg3 || arg0 > arg4 )\n"
    "        {\n"
    "            tmp = 0.0;\n"
    "            return tmp;\n"
    "        }\n"
    "        arg0 = (arg0 - arg3)/fScale;\n"
    "        tmp = GetBetaDistPDF(arg0, arg1, arg2)/fScale;\n"
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
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArgWithDefault( "tmp3", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "tmp4", 4, 1, vSubArguments, ss );
    ss << "    if (tmp0 < 0.0 || tmp0 >= 1.0 ||";
    ss << "tmp3 == tmp4 || tmp1 <= 0.0 || tmp2 <= 0.0)\n";
    ss << "    {\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    }\n";
    ss << "    if (tmp0 == 0.0)\n";
    ss << "        return 0.0;\n";
    ss << "    else\n";
    ss << "    {";
    ss << "        bool bConvError;";
    ss << "        double fVal = lcl_IterateInverseBetaInv";
    ss << "(tmp0, tmp1, tmp2, 0.0, 1.0, &bConvError);\n";
    ss << "        if(bConvError)\n";
    ss << "            return CreateDoubleError(NoConvergence);\n";
    ss << "        else\n";
    ss << "            return (tmp3 + fVal*(tmp4 - tmp3));\n";
    ss << "    }";
    ss << "}\n";
}
void OpDevSq::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double vSum = 0.0;\n";
    ss << "    double vMean = 0.0;\n";
    ss << "    int cnt = 0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan (arg))\n"
        "        {\n"
        "            vSum += arg;\n"
        "            ++cnt;\n"
        "        }\n"
        );
    ss << "    vMean = vSum / cnt;\n";
    ss << "    vSum = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan (arg))\n"
        "            vSum += ( arg - vMean ) * ( arg - vMean );\n"
        );
    ss << "    return vSum;\n";
    ss << "}\n";
}

void OpHypGeomDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    ss << "    double N1=floor(arg3);\n"
    "    double M1=floor(arg2);\n"
    "    double n1=floor(arg1);\n"
    "    double x1=floor(arg0);\n"
    "    double num[9];\n"
    "    double tmp;\n"
    "    if( (x1 < 0.0) || (n1 < x1) || (M1 < x1) || (N1 < n1) ||"
    "(N1 < M1) || (x1 < n1 - N1 + M1) )\n"
    "    {\n"
    "        return CreateDoubleError(IllegalArgument);\n"
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
    "            num[i]=0.5*log(2.0*M_PI)+(num[i]+0.5)*log(num[i])-num[i]+"
    "(1.0/(12.0*num[i])-1.0/(360*pow(num[i],3)));\n"
    "    }\n";
    ss << "    tmp=pow(M_E,(num[0]+num[3]+num[7]+num[8]";
    ss << "-num[1]-num[2]-num[4]-num[5]-num[6]));\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpAveDev:: GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double sum=0.0;\n";
    ss << "    double totallength=0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "        {\n"
        "            sum += arg;\n"
        "            ++totallength;\n"
        "        }\n"
        );
    ss << "    double mean = sum / totallength;\n";
    ss << "    sum = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss,
        "        if(!isnan(arg))\n"
        "            sum += fabs(arg-mean);\n"
        );
    ss << "    return sum/totallength;\n";
    ss << "}";
}

namespace {

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

}

void OpMinA::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixed = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 1.79769e+308;\n";
    ss <<"\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isnan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 > ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " ? ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " : tmp0;\n";
                ss << "        else if(isnan(";
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
                ss << "        if (!isnan(";
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
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double nCount = 0.0;\n";

    ss <<"\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isnan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
                ss << "        else if(isnan(";
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
                ss << "        if (!isnan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            nCount+=1.0;\n";
                ss <<"}\n    }\n";
            }
            else if(isMixed == svDoubleVectorRefString)
            {
                ss << "        if (!isnan(";
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
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 2.22507e-308;\n";

    ss <<"\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isnan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << "))\n";
                ss << "            tmp0 = tmp0 < ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " ? ";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << " : tmp0;\n";
                ss << "        else if(isnan(";
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
                ss << "        if (!isnan(";
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
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    int isMixed = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp0 = 0.0;\n";
    ss << "    double nCount = 0.0;\n";
    ss <<"\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
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

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(isMixed == svDoubleVectorRefDoubleString
                || isMixed == svSingleVectorRefDoubleString)
            {
                ss << "        if (!isnan(";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ")){\n";
                ss << "            tmp0 +=";
                ss << vSubArguments[i]->GenDoubleSlidingWindowDeclRef();
                ss << ";\n";
                ss << "            nCount+=1.0;\n";
                ss << "    }\n";
                ss << "        else if(isnan(";
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
                ss << "        if (!isnan(";
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
                ss << "        if (!isnan(";
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
    ss << "    return tmp0/nCount;\n";
    ss << "}\n";
}
void OpVarA::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
            ss << "    fMean = fSum / fCount;\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
    ss << "        return vSum / (fCount - 1.0);\n";
    ss << "}\n";
}

void OpVarPA::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
            ss << "    fMean = fSum / fCount;\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
    ss << "        return vSum / fCount;\n";
    ss << "}\n";
}
void OpStDevA::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
            ss << "    fMean = fSum / fCount;\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
    ss << "        return sqrt(vSum / (fCount - 1.0));\n";
    ss << "}\n";
}

void OpStDevPA::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    int isMixedDV = 0;
    int isMixedSV = 0;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "        {\n";
                    ss << "            fSum += arg;\n";
                    ss << "            fCount = fCount + 1.0;\n";
                    ss << "        }\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
            ss << "    fMean = fSum / fCount;\n";
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
                    ss << "        if (isnan(arg) && ";
                    ss << vSubArguments[i]->GenStringSlidingWindowDeclRef();
                    ss << " == 0)\n";
                    ss << "            continue;\n";
                    ss << "        if(isnan(arg) && ";
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
                    ss << "        if (isnan(arg))\n";
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
                    ss << "        if (!isnan(arg))\n";
                    ss << "            vSum += (arg - fMean)*(arg - fMean);\n";
                    ss << "        if (isnan(arg) && ";
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
                    ss << "        if (!isnan(arg))\n";
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
    ss << "        return sqrt(vSum / fCount);\n";
    ss << "}\n";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
