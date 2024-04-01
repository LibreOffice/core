/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_statistical.hxx"

#include <sstream>
#include "op_statistical_helpers.hxx"

#include "op_math_helpers.hxx"

namespace sc::opencl {

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
    GenerateRangeArg( 0, vSubArguments, ss, SkipEmpty,
        "        fSum += arg;\n"
        "        fSumSqr += arg * arg;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if(fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    mue = fSum / fCount;\n";
    GenerateArg( "mu", 1, vSubArguments, ss );
    if(vSubArguments.size() == 3)
    {
        GenerateArg( "sigma", 2, vSubArguments, ss );
        ss << "    if(sigma <= 0.0)\n";
        ss << "        return CreateDoubleError(IllegalArgument);\n";
        ss << "    return 0.5 - gauss((mue-mu)*sqrt(fCount)/sigma);\n";
    }
    else
    {
        ss << "    double sigma = (fSumSqr-fSum*fSum/fCount)/(fCount-1.0);\n";
        ss << "    if(sigma == 0.0)\n";
        ss << "        return CreateDoubleError(DivisionByZero);\n";
        ss << "    return 0.5 - gauss((mue-mu)/sqrt(sigma/fCount));\n";
    }
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
    ss << "    double fT = 0.0;\n";
    ss << "    double fF = 0.0;\n";
    GenerateArg( "mode", 2, vSubArguments, ss );
    GenerateArg( "type", 3, vSubArguments, ss );
    ss << "    mode = floor(mode);\n";
    ss << "    type = floor(type);\n";
    ss << "    if(mode != 1.0 && mode != 2.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    if(type != 1.0 && type != 2.0 && type != 3.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";

    ss << "    if(type == 1.0)\n";
    ss << "    {\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "            fSum1 += arg1;\n"
        "            fSum2 += arg2;\n"
        "            fSumSqr1 += (arg1 - arg2)*(arg1 - arg2);\n"
        "            fCount1 += 1;\n"
        );
    ss << "        if(fCount1 < 1.0)\n";
    ss << "            return CreateDoubleError(NoValue);\n";
    ss << "        double divider = sqrt(fCount1 * fSumSqr1 - (fSum1-fSum2)*(fSum1-fSum2));\n";
    ss << "        if(divider == 0)\n";
    ss << "            return CreateDoubleError(DivisionByZero);\n";
    ss << "        fT = sqrt(fCount1-1.0) * fabs(fSum1 - fSum2) / divider;\n";
    ss << "        fF = fCount1 - 1.0;\n";
    ss << "    }\n";
    ss << "    if(type == 2.0 || type == 3.0)\n";
    ss << "    {\n";
    GenerateRangeArg( 0, vSubArguments, ss, SkipEmpty,
        "        fSum1 += arg;\n"
        "        fSumSqr1 += arg * arg;\n"
        "        fCount1 += 1;\n"
        );
    GenerateRangeArg( 1, vSubArguments, ss, SkipEmpty,
        "        fSum2 += arg;\n"
        "        fSumSqr2 += arg * arg;\n"
        "        fCount2 += 1;\n"
        );
    ss << "        if (fCount1 < 2.0 || fCount2 < 2.0)\n";
    ss << "            return CreateDoubleError(NoValue);\n";
    ss << "    }\n";
    ss << "    if(type == 3.0)\n";
    ss << "    {\n";
    ss << "        double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)\n";
    ss << "            /(fCount1-1.0)/fCount1;\n";
    ss << "        double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)\n";
    ss << "            /(fCount2-1.0)/fCount2;\n";
    ss << "        if (fS1 + fS2 == 0.0)\n";
    ss << "            return CreateDoubleError(NoValue);\n";
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
    GenerateArg( "mu", 1, vSubArguments, ss );
    GenerateArg( "sigma", 2, vSubArguments, ss );
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
    ss << "    if(alpha <= 0.0 || beta <=0.0 || x < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    if (kum == 0.0)\n";
    ss << "        return alpha/pow(beta,alpha)*pow(x,alpha-1.0)*\n";
    ss << "                       exp(-pow(x/beta,alpha));\n";
    ss << "    else\n";
    ss << "        return 1.0 - exp(-pow(x/beta,alpha));\n";
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

void OpNegbinomdist::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "f", 0, vSubArguments, ss );
    GenerateArg( "s", 1, vSubArguments, ss );
    GenerateArg( "p", 2, vSubArguments, ss );
    ss << "    f = floor( f );\n";
    ss << "    s = floor( s );\n";
    ss << "    if ((f + s) <= 1.0 || p < 0.0 || p > 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double q = 1.0 - p;\n";
    ss << "    double fFactor = pow(p,s);\n";
    ss << "    for(int i=0; i<f; i++)\n";
    ss << "        fFactor *= (i+s)/(i+1.0)*q;\n";
    ss << "    return fFactor;\n";
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
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        if( arg < 0 )\n"
        "            return CreateDoubleError(IllegalArgument);\n"
        "        if( arg == 0 )\n"
        "            return 0;\n"
        "        nVal += log(arg);\n"
        "        ++totallength;\n"
        );
    ss << "    return exp(nVal/totallength);\n";
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
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        if( arg <= 0 )\n"
        "            return CreateDoubleError(IllegalArgument);\n"
        "        nVal += (1.0 / arg);\n"
        "        ++totallength;\n"
        );
    ss << "    return totallength/nVal;\n";
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
    GenerateArg( "sigma", 1, vSubArguments, ss );
    GenerateArg( "size", 2, vSubArguments, ss );
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
    ss << "    if (rn < 0.0 || alpha < 0.0 || alpha > 1.0 || p < 0.0";
    ss << " || p > 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else if ( alpha == 0 )\n";
    ss << "        return 0;\n";
    ss << "    else if ( alpha == 1 )\n";
    ss << "        return p == 0 ? 0 : rn;\n";
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
    ss << "    inA = floor( inA );\n";
    ss << "    inB = floor( inB );\n";
    ss << "    if (inA < 0.0 || inB < 0.0 || inB > inA)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    for( int i = 0; i<inB; i++)\n";
    ss << "    {\n";
    ss << "        tmp *= inA ;\n";
    ss << "        inA = inA - 1.0;\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
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
    ss << "    inA = floor( inA );\n";
    ss << "    inB = floor( inB );\n";
    ss << "    if (inA < 0.0 || inB < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return pow(inA, inB);\n";
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

void OpNorminv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpNorminv::GenSlidingWindowFunction(
    outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "mue", 1, vSubArguments, ss );
    GenerateArg( "sigma", 2, vSubArguments, ss );
    ss << "    if (sigma <= 0.0 || x < 0.0 || x > 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else if (x == 0.0 || x == 1.0)\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    return gaussinv(x)*sigma + mue;\n";
    ss << "}\n";
}

void OpNormsinv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpNormsinv:: GenSlidingWindowFunction
    (outputstream &ss,const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    ss << "    if (x < 0.0 || x > 1.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else if (x == 0.0 || x == 1.0)\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    return gaussinv(x);\n";
    ss << "}\n";
}

void OpLogInv::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(gaussinvDecl);
    funs.insert(gaussinv);
}

void OpLogInv:: GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp;\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArgWithDefault( "mue", 1, 0, vSubArguments, ss );
    GenerateArgWithDefault( "sigma", 2, 1, vSubArguments, ss );
    ss << "    if ( sigma <= 0.0 || x <= 0.0 || x >= 1.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return exp(mue+sigma*gaussinv(x));\n";
    ss << "}\n";
}

void OpLogNormDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArgWithDefault( "mue", 1, 0, vSubArguments, ss );
    GenerateArgWithDefault( "sigma", 2, 1, vSubArguments, ss );
    GenerateArgWithDefault( "fCumulative", 3, 1, vSubArguments, ss );
    ss << "    if (sigma <= 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double tmp;\n";
    ss << "    double temp = (log(x)-mue)/sigma;\n";
    ss << "    if(fCumulative != 0)\n";
    ss << "    {\n";
    ss << "        if(x<=0)\n";
    ss << "            tmp = 0.0;\n";
    ss << "        else\n";
    ss << "            tmp = 0.5 * erfc(-temp * 0.7071067811865475);\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "        if(x<=0)\n";
    ss << "            return CreateDoubleError(IllegalArgument);\n";
    ss << "        else\n";
    ss << "            tmp = (0.39894228040143268 * exp((-1)*pow(temp, 2)";
    ss << " / 2.0))/(sigma*x);\n";
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
    ss << "    if(arg1 <= 0 || arg2 <= 0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    ss << "    double fx,fDF,tmp=0;\n";
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
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    ss << "    double fF2=floor(arg2);\n"
    "    double fF1=floor(arg1);\n"
    "    if( arg0 <= 0 || arg1 < 1 || arg2 < 1 || arg1 >= 1.0e10 || arg2 >= 1.0e10 || arg0 > 1 )\n"
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
    ss << "    double fLength1 = 0.0;\n";
    ss << "    double fLength2 = 0.0;\n";
    ss << "    double tmp = 0;\n";
    GenerateRangeArg( 0, vSubArguments, ss, SkipEmpty,
        "        fSum1 += arg;\n"
        "        fSumSqr1 += arg * arg;\n"
        "        fLength1 += 1;\n"
        );
    GenerateRangeArg( 1, vSubArguments, ss, SkipEmpty,
        "        fSum2 += arg;\n"
        "        fSumSqr2 += arg * arg;\n"
        "        fLength2 += 1;\n"
        );
    ss << "    if(fLength1 < 2 || fLength2 < 2)\n"
        "        return CreateDoubleError(NoValue);\n"
        "    double fS1 = (fSumSqr1-fSum1*fSum1/fLength1)/(fLength1-1.0);\n"
        "    double fS2 = (fSumSqr2-fSum2*fSum2/fLength2)/(fLength2-1.0);\n"
        "    if(fS1 == 0 || fS2 == 0)\n"
        "        return CreateDoubleError(NoValue);\n"
        "    double fF, fF1, fF2;\n"
        "    if (fS1 > fS2)\n"
        "    {\n"
        "        fF = fS1/fS2;\n"
        "        fF1 = fLength1-1.0;\n"
        "        fF2 = fLength2-1.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        fF = fS2/fS1;\n"
        "        fF1 = fLength2-1.0;\n"
        "        fF2 = fLength1-1.0;\n"
        "    }\n"
        "    double fFcdf = GetFDist(fF, fF1, fF2);\n"
        "    return 2.0*min(fFcdf, 1 - fFcdf);\n";
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
    ss << "    if (lambda <= 0.0 || x < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    ss << "    if (tmp0 < 0.0 || tmp0 > 1.0 ||";
    ss << "tmp3 >= tmp4 || tmp1 <= 0.0 || tmp2 <= 0.0)\n";
    ss << "    {\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    }\n";
    ss << "    bool bConvError;\n";
    ss << "    double fVal = lcl_IterateInverseBetaInv";
    ss << "(tmp0, tmp1, tmp2, 0.0, 1.0, &bConvError);\n";
    ss << "    if(bConvError)\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "    return (tmp3 + fVal*(tmp4 - tmp3));\n";
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
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        vSum += arg;\n"
        "        ++cnt;\n"
        );
    ss << "    vMean = vSum / cnt;\n";
    ss << "    vSum = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        vSum += ( arg - vMean ) * ( arg - vMean );\n"
        );
    ss << "    return vSum;\n";
    ss << "}\n";
}

void OpHypGeomDist::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "x", 0, vSubArguments, ss );
    GenerateArg( "n", 1, vSubArguments, ss );
    GenerateArg( "M", 2, vSubArguments, ss );
    GenerateArg( "N", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fCumulative", 4, 0, vSubArguments, ss );
    ss <<
    "    x = floor(x);\n"
    "    n = floor(n);\n"
    "    M = floor(M);\n"
    "    N = floor(N);\n"
    "    double num[9];\n"
    "    double tmp = 0;\n"
    "    if( (x < 0.0) || (n < x) || (N < n) ||"
    "(N < M) || (M < 0.0) )\n"
    "    {\n"
    "        return CreateDoubleError(IllegalArgument);\n"
    "    }\n"
    "    for(int i = (fCumulative ? 0 : x); i <= x; ++i )\n"
    "    {\n"
    "        if( (M < i) || (i < n - N + M) )\n"
    "            continue;\n"
    "        num[0]=M;\n"
    "        num[1]=i;\n"
    "        num[2]=M-i;\n"
    "        num[3]=N-M;\n"
    "        num[4]=n-i;\n"
    "        num[5]=N-M-n+i;\n"
    "        num[6]=N;\n"
    "        num[7]=n;\n"
    "        num[8]=N-n;\n"
    "        for(int i=0;i<9;i++)\n"
    "        {\n"
    "            if(num[i]<171)\n"
    "            {\n"
    "                if(num[i]==0)\n"
    "                    num[i]=0;\n"
    "                else\n"
    "                    num[i]=log(tgamma(num[i])*num[i]);\n"
    "            }\n"
    "            else\n"
    "                num[i]=0.5*log(2.0*M_PI)+(num[i]+0.5)*log(num[i])-num[i]+\n"
    "                    (1.0/(12.0*num[i])-1.0/(360*pow(num[i],3)));\n"
    "        }\n"
    "        tmp+=pow(M_E,(num[0]+num[3]+num[7]+num[8]-num[1]-num[2]-num[4]-num[5]-num[6]));\n"
    "    }\n"
    "    return tmp;\n";
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
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        sum += arg;\n"
        "        ++totallength;\n"
        );
    ss << "    double mean = sum / totallength;\n";
    ss << "    sum = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        sum += fabs(arg-mean);\n"
        );
    ss << "    return sum/totallength;\n";
    ss << "}";
}

// Block of functions OpCovar-OpRsq that are rather similar.

void OpCovar::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSumX = 0.0;\n";
    ss << "    double fSumY = 0.0;\n";
    ss << "    double fMeanX = 0.0;\n";
    ss << "    double fMeanY = 0.0;\n";
    ss << "    double fSumDeltaXDeltaY = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "        fSumX += arg1;\n"
        "        fSumY += arg2;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if( fCount < 1 )\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    fMeanX = fSumX / fCount;\n";
    ss << "    fMeanY = fSumY / fCount;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "        fSumDeltaXDeltaY +=(arg1-fMeanX)*(arg2-fMeanY);\n"
        );
    ss << "    return fSumDeltaXDeltaY / fCount;\n";
    ss << "}\n";
}

void OpForecast::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 2 );
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
    GenerateArg( "arg0", 0, vSubArguments, ss );
    GenerateRangeArgPair( 1, 2, vSubArguments, ss, SkipEmpty,
        // note that arg1 -> Y, arg2 -> X
        "        fSumX += arg2;\n"
        "        fSumY += arg1;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if( fCount < 1 )\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    fMeanX = fSumX / fCount;\n";
    ss << "    fMeanY = fSumY / fCount;\n";
    GenerateRangeArgPair( 1, 2, vSubArguments, ss, SkipEmpty,
        "        fSumDeltaXDeltaY +=(arg2-fMeanX)*(arg1-fMeanY);\n"
        "        fSumSqrDeltaX += (arg2-fMeanX)*(arg2-fMeanX);\n"
        );
    ss << "    if(fSumSqrDeltaX == 0.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    return fMeanY + fSumDeltaXDeltaY / fSumSqrDeltaX * (arg0 - fMeanX);\n";
    ss << "}\n";
}

void OpInterceptSlopeBase::GenerateCode( outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments, const char* finalComputeCode )
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
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
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        // note that arg1 -> Y, arg2 -> X
        "        fSumX += arg2;\n"
        "        fSumY += arg1;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if( fCount < 1 )\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    fMeanX = fSumX / fCount;\n";
    ss << "    fMeanY = fSumY / fCount;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "        fSumDeltaXDeltaY +=(arg2-fMeanX)*(arg1-fMeanY);\n"
        "        fSumSqrDeltaX += (arg2-fMeanX)*(arg2-fMeanX);\n"
        );
    ss << finalComputeCode;
    ss << "}\n";
}

void OpIntercept::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments,
        "    if(fSumSqrDeltaX == 0.0)\n"
        "        return CreateDoubleError(DivisionByZero);\n"
        "    return fMeanY - (fSumDeltaXDeltaY/fSumSqrDeltaX)*fMeanX;\n"
        );
}

void OpSlope::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments,
        "    if(fSumSqrDeltaX == 0.0)\n"
        "        return CreateDoubleError(DivisionByZero);\n"
        "    return fSumDeltaXDeltaY / fSumSqrDeltaX;\n"
        );
}

void OpPearsonCovarBase::GenerateCode( outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments, double minimalCountValue, const char* finalComputeCode )
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
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
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        // note that arg1 -> Y, arg2 -> X
        "        fSumX += arg2;\n"
        "        fSumY += arg1;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if( fCount < " << minimalCountValue <<" )\n";
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    fMeanX = fSumX / fCount;\n";
    ss << "    fMeanY = fSumY / fCount;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "        fSumDeltaXDeltaY +=(arg2-fMeanX)*(arg1-fMeanY);\n"
        "        fSumSqrDeltaX += (arg2-fMeanX)*(arg2-fMeanX);\n"
        "        fSumSqrDeltaY += (arg1-fMeanY)*(arg1-fMeanY);\n"
        );
    ss << finalComputeCode;
    ss << "}\n";
}

void OpPearson::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments, 1,
        "    if (fSumSqrDeltaX == 0 || fSumSqrDeltaY == 0)\n"
        "        return CreateDoubleError(DivisionByZero);\n"
        "    return ( fSumDeltaXDeltaY / sqrt( fSumSqrDeltaX * fSumSqrDeltaY));\n"
        );
}

void OpSTEYX::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments, 3,
        "    if(fSumSqrDeltaX == 0.0)\n"
        "        return CreateDoubleError(DivisionByZero);\n"
        "    return sqrt((fSumSqrDeltaY - fSumDeltaXDeltaY * \n"
        "                   fSumDeltaXDeltaY / fSumSqrDeltaX)\n"
        "                   /(fCount - 2.0));\n"
        );
}

void OpRsq::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    // This is basically pow(OpPearson,2)
    GenerateCode( ss, sSymName, vSubArguments, 1,
        "    if (fSumSqrDeltaX == 0 || fSumSqrDeltaY == 0)\n"
        "        return CreateDoubleError(DivisionByZero);\n"
        "    return ( fSumDeltaXDeltaY * fSumDeltaXDeltaY / (fSumSqrDeltaX * fSumSqrDeltaY));\n"
        );
}

void OpVarStDevBase::BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs)
{
    decls.insert(is_representable_integerDecl);
    funs.insert(is_representable_integer);
    decls.insert(approx_equalDecl);
    funs.insert(approx_equal);
    decls.insert(fsub_approxDecl);
    funs.insert(fsub_approx);
}

void OpVarStDevBase::GenerateCode(outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n"; // this must be matched by whoever calls this function
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fSum = 0.0;\n";
    ss << "    double fCount = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        fSum += arg;\n"
        "        fCount += 1.0;\n"
        );
    ss << "    if (fCount == 0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    double fMean = fSum / fCount;\n";
    ss << "    double vSum = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        vSum += pown( fsub_approx(arg, fMean), 2 );\n"
        );
}

void OpVar::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return vSum / (fCount - 1.0);\n";
    ss << "}\n";
}

void OpVarP::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if (fCount == 0.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return vSum / fCount;\n";
    ss << "}\n";
}

void OpStDev::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if (fCount <= 1.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum / (fCount - 1.0));\n";
    ss << "}\n";
}

void OpStDevP::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if (fCount <= 0.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    else\n";
    ss << "        return sqrt(vSum / fCount);\n";
    ss << "}\n";
}

void OpSkew::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if(fCount <= 2.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    double fStdDev = sqrt(vSum / (fCount - 1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        dx = fsub_approx(arg, fMean) / fStdDev;\n"
        "        xcube = xcube + dx * dx * dx;\n"
        );
    ss << "    return ((xcube * fCount) / (fCount - 1.0)) / (fCount - 2.0);\n";
    ss << "}\n";
}

void OpSkewp::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if(fCount <= 2.0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    double fStdDev = sqrt(vSum / fCount);\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xcube = 0.0;\n";
    ss << "    if(fStdDev == 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        dx = fsub_approx(arg, fMean) / fStdDev;\n"
        "        xcube = xcube + dx * dx * dx;\n"
        );
    ss << "    return xcube / fCount;\n";
    ss << "}\n";
}

void OpKurt:: GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateCode( ss, sSymName, vSubArguments );
    ss << "    if( fCount < 4 )\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
    ss << "    double fStdDev = sqrt(vSum / (fCount - 1.0));\n";
    ss << "    double dx = 0.0;\n";
    ss << "    double xpower4 = 0.0;\n";
    GenerateRangeArgs( vSubArguments, ss, SkipEmpty,
        "        dx = (arg -fMean) / fStdDev;\n"
        "        xpower4 = xpower4 + (dx * dx * dx * dx);\n"
        );
    ss<< "    double k_d = (fCount - 2.0) * (fCount - 3.0);\n";
    ss<< "    double k_l = fCount * (fCount + 1.0) / ((fCount - 1.0) * k_d);\n";
    ss<< "    double k_t = 3.0 * (fCount - 1.0) * (fCount - 1.0) / k_d;\n";
    ss<< "    return xpower4 * k_l - k_t;\n";
    ss << "}";
}

void OpMin::BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs)
{
    decls.insert(fmin_countDecl);
    funs.insert(fmin_count);
}

void OpMax::BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs)
{
    decls.insert(fmax_countDecl);
    funs.insert(fmax_count);
}

void OpAverage::BinInlineFun(std::set<std::string>& decls,std::set<std::string>& funs)
{
    decls.insert(fsum_countDecl);
    funs.insert(fsum_count);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
