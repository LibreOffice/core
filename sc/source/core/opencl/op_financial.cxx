/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_financial.hxx"

#include <sstream>

using namespace formula;

namespace sc::opencl {
// Definitions of inline functions
#include "op_financial_helpers.hxx"
#include "op_math_helpers.hxx"

void OpRRI::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "nper", 0, vSubArguments, ss );
    GenerateArg( "pv", 1, vSubArguments, ss );
    GenerateArg( "fv", 2, vSubArguments, ss );
    ss << "    if ( nper <= 0.0 || pv == 0 )\n";
    ss << "         return CreateDoubleError(IllegalArgument);\n";
    ss << "    tmp = pow(fv/pv,1.0/nper)-1;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpNominal::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    ss << "double tmp = 0;\n\t";
    ss<<"if(tmp1==0)\n\t";
    ss<<"\treturn 0;\n\t";
    ss<<"tmp= 1.0 / tmp1;\n\t";
    ss<<"tmp=( pow( tmp0+ 1.0, tmp ) - 1.0 ) *";
    ss<<"tmp1;\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpDollarde::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    GenerateArg( "dollar", 0, vSubArguments, ss );
    GenerateArg( "fFrac", 1, vSubArguments, ss );
    ss <<"fFrac = (int)fFrac;\n\t";
    ss << "tmp = modf( dollar , &fInt );\n\t";
    ss << "tmp /= fFrac;\n\t";
    ss << "tmp *= pow( 10.0 , ceil( log10(fFrac ) ) );\n\t";
    ss << "tmp += fInt;\t";
    ss << "\n\treturn tmp;\n";
    ss << "}";
}

void OpDollarfr::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    GenerateArg( "dollar", 0, vSubArguments, ss );
    GenerateArg( "fFrac", 1, vSubArguments, ss );
    ss <<"fFrac = (int)fFrac;\n\t";
    ss << "tmp = modf( dollar , &fInt );\n\t";
    ss << "tmp *= fFrac;\n\t";
    ss << "tmp *= pow( 10.0 , -ceil( log10(fFrac ) ) );\n\t";
    ss << "tmp += fInt;\t";
    ss << "\n\treturn tmp;\n";
    ss << "}";
}

void OpDISC::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetYearFrac);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpDISC::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    GenerateArg( 4, vSubArguments, ss );
    ss << "    int nNullDate = 693594;\n";
    ss << "    tmp = 1.0 - arg2 / arg3;\n";
    ss << "    tmp /=";
    ss << " GetYearFrac(nNullDate, (int)arg0, (int)arg1, (int)arg4);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpINTRATE::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearDiffDecl);decls.insert(GetDiffDateDecl);
    decls.insert(DaysToDateDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysInMonthDecl);
    decls.insert(IsLeapYearDecl);
    funs.insert(GetYearDiff);funs.insert(GetDiffDate);
    funs.insert(DaysToDate);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysInMonth);
    funs.insert(IsLeapYear);
}

void OpINTRATE::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    GenerateArg( 4, vSubArguments, ss );
    ss << "    int nNullDate = GetNullDate();\n";
    ss << "    tmp = ((arg3 / arg2) - 1) / GetYearDiff(nNullDate, (int)arg0,";
    ss << "             (int)arg1,(int)arg4);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpFV::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetFVDecl);
    funs.insert(GetFV);
}

void OpFV::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    GenerateArg( 4, vSubArguments, ss );
    ss << "    tmp = GetFV(arg0, arg1, arg2, arg3, arg4);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpIPMT::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetFVDecl);
    funs.insert(GetFV);
    decls.insert(GetPMTDecl);
    funs.insert(GetPMT);
    decls.insert(GetIpmtDecl);
    funs.insert(GetIpmt);
}

void OpIPMT::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fPer", 1, vSubArguments, ss );
    GenerateArg( "fNper", 2, vSubArguments, ss );
    GenerateArg( "fPv", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fFv", 4, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayInAdvance", 5, 0, vSubArguments, ss );
    ss << "    if (fPer < 1.0 || fPer > fNper)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        double fPmt;\n";
    ss << "        return GetIpmt(fRate, fPer, fNper, fPv, fFv, fPayInAdvance != 0, &fPmt);\n";
    ss << "     }\n";
    ss << "}\n";
}
void OpISPMT::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    ss << "    tmp = arg3 * arg0 * ( arg1 - arg2) / arg2;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpPDuration::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    ss << "    if ( arg0 <= 0.0 || arg1 <= 0.0 || arg2 <= 0.0 )\n";
    ss << "         return CreateDoubleError(IllegalArgument);\n";
    ss << "    tmp = log(arg2 / arg1) / log1p(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpDuration_ADD::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDurationDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(GetYearFracDecl);decls.insert(DaysToDateDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetDuration);funs.insert(lcl_Getcoupnum);
    funs.insert(GetYearFrac);funs.insert(DaysToDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpDuration_ADD::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    GenerateArg( 4, vSubArguments, ss );
    GenerateArg( 5, vSubArguments, ss );
    ss << "    int nNullDate = GetNullDate();\n";
    ss << "    tmp = GetDuration( nNullDate, (int)arg0, (int)arg1, arg2,";
    ss << " arg3, (int)arg4, (int)arg5);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpMDuration::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDurationDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(addMonthsDecl);decls.insert(checklessthanDecl);
    decls.insert(setDayDecl);decls.insert(ScaDateDecl);
    decls.insert(GetYearFracDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetDuration);funs.insert(lcl_Getcoupnum);
    funs.insert(addMonths);funs.insert(checklessthan);
    funs.insert(setDay);funs.insert(ScaDate);
    funs.insert(GetYearFrac);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpMDuration::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    GenerateArg( 3, vSubArguments, ss );
    GenerateArg( 4, vSubArguments, ss );
    GenerateArg( 5, vSubArguments, ss );
    ss << "    int nNullDate = 693594;\n";
    ss << "    tmp = GetDuration( nNullDate, (int)arg0, (int)arg1, arg2,";
    ss << " arg3, (int)arg4, (int)arg5);\n";
    ss << "    tmp = tmp * pow(1.0 + arg3 * pow((int)arg4, -1.0), -1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void Fvschedule::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = 1.0;\n\t";
    ss << "int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "\t";
    GenerateRangeArg( 1, vSubArguments, ss, SkipEmpty,
        "        tmp *= arg + 1;\n"
        );
    ss << "\t";
    ss << "return (double)tmp * arg0";
    ss << ";\n}";
}
void Cumipmt::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetPMTDecl); decls.insert(GetFV);
    funs.insert(GetPMT);funs.insert(GetFV);
}
void Cumipmt::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &
vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fNumPeriouds", 1, vSubArguments, ss );
    GenerateArg( "fVal", 2, vSubArguments, ss );
    GenerateArg( "fStartPer", 3, vSubArguments, ss );
    GenerateArg( "fEndPer", 4, vSubArguments, ss );
    GenerateArg( "fPayType", 5, vSubArguments, ss );
    ss << "    int nNumPeriods = (int)fNumPeriods;\n";
    ss << "    int nStartPer = (int)fStartPer;\n";
    ss << "    int nEndPer = (int)fEndPer;\n";
    ss << "    int nPayType = (int)fPayType;\n";
    ss <<"    double fPmt;\n";
    ss <<"    fPmt = GetPMT( fRate, nNumPeriods, fVal, 0.0, nPayType != 0 );\n";
    ss <<"    double tmp = 0.0;\n";
    ss <<"    if( nStartPer == 1 )\n";
    ss <<"    {\n";
    ss <<"        if( nPayType <= 0 )\n";
    ss <<"            tmp = -fVal;\n";
    ss <<"        nStartPer++;\n";
    ss <<"    }\n";
    ss <<"    for( ; nStartPer<= nEndPer ; nStartPer++ )\n";
    ss <<"    {\n";
    ss <<"        if( nPayType > 0 )\n";
    ss <<"            tmp += GetFV( fRate,  nStartPer - 2 , ";
    ss <<"fPmt, fVal, 1 ) - fPmt;\n";
    ss <<"        else\n";
    ss <<"            tmp += GetFV( fRate,  nStartPer - 1 , ";
    ss <<"fPmt, fVal, 0 );\n";
    ss <<"    }\n";
    ss <<"    tmp *= fRate;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpIRR::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    #define  Epsilon   1.0E-7\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArgWithDefault( "fEstimated", 1, 0.1, vSubArguments, ss );
    ss << "    double fEps = 1.0;\n";
    ss << "    double xNew = 0.0, fNumerator = 0.0, fDenominator = 0.0;\n";
    ss << "    double nCount = 0.0;\n";
    ss << "    unsigned short nItCount = 0;\n";
    ss << "    double x = fEstimated;\n";
    ss << "    while (fEps > Epsilon && nItCount < 20)\n";
    ss << "    {\n";
    ss << "        nCount = 0.0; fNumerator = 0.0;  fDenominator = 0.0;\n";
    GenerateRangeArg( 0, vSubArguments, ss, SkipEmpty,
        "            fNumerator += arg / pow(1.0 + x, nCount);\n"
        "            fDenominator+=-1*nCount*arg/pow(1.0+x,nCount+1.0);\n"
        "            nCount += 1;\n"
        );
    ss << "        xNew = x - fNumerator / fDenominator;\n";
    ss << "        fEps = fabs(xNew - x);\n";
    ss << "        x = xNew;\n";
    ss << "        nItCount++;\n";
    ss << "    }\n";
    ss << "    if (fEstimated == 0.0 && fabs(x) < Epsilon)\n";
    ss << "        x = 0.0;\n";
    ss << "    if (fEps < Epsilon)\n";
    ss << "        return x;\n";
    ss << "    else\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "}\n";
}

void XNPV::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double result = 0.0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "rate", 0, vSubArguments, ss );
    GenerateRangeArgElement( "dateNull", 2, "0", vSubArguments, ss, EmptyIsZero );
    GenerateRangeArgPair( 1, 2, vSubArguments, ss, SkipEmpty,
        "        result += arg1/(pow((rate+1),(arg2-dateNull)/365));\n"
        );
    ss << "    return result;\n";
    ss << "}";
}

void PriceMat::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);

    funs.insert(GetYearFrac);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}
void PriceMat::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double result=0;\n\t";
    ss<< "int nNullDate = GetNullDate( );\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fIssue", 2, vSubArguments, ss );
    GenerateArg( "rate", 3, vSubArguments, ss );
    GenerateArg( "yield", 4, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 5, 0, vSubArguments, ss );
    ss << "\t";
    ss <<"int settle = fSettle;\n\t";
    ss <<"int mat = fMat;\n\t";
    ss <<"int issue = fIssue;\n\t";
    ss <<"int nBase = fBase;\n\t";
    ss<< "double fIssMat = GetYearFrac( nNullDate, issue, mat, nBase);\n";
    ss<<"double fIssSet = GetYearFrac( nNullDate, issue, settle,nBase);\n";
    ss<<"double fSetMat = GetYearFrac( nNullDate, settle, mat, nBase);\n";
    ss<<"result = 1.0 + fIssMat * rate;\n\t";
    ss<<"result /= 1.0 + fSetMat * yield;\n\t";
    ss<<"result -= fIssSet * rate;\n\t";
    ss<<"result*= 100.0;\n\t";
    ss<<"return result;\n\t";
    ss<<"}\n";
}

void OpSYD::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "cost", 0, vSubArguments, ss );
    GenerateArg( "salvage", 1, vSubArguments, ss );
    GenerateArg( "life", 2, vSubArguments, ss );
    GenerateArg( "period", 3, vSubArguments, ss );
    ss << "    double result=0;\n";
    ss <<"    double tmpvalue = ((life*(life+1))/2.0);\n";
    ss <<"    result = ((cost-salvage)*(life-period+1)/tmpvalue);\n";
    ss <<"    return result;\n";
    ss <<"}\n";
}

void OpMIRR::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    GenerateArg( 1, vSubArguments, ss );
    GenerateArg( 2, vSubArguments, ss );
    ss << "double invest = arg1 + 1.0;\n\t";
    ss << "double reinvest = arg2 + 1.0;\n\t";
    ss << "double NPV_invest = 0.0;\n\t";
    ss << "double Pow_invest = 1.0;\n\t";
    ss << "double NPV_reinvest = 0.0;\n\t";
    ss << "double Pow_reinvest = 1.0;\n\t";
    ss << "int nCount = 0;\n\t";
    ss << "bool bHasPosValue = false;\n";
    ss << "bool bHasNegValue = false;\n";
    GenerateRangeArg( 0, vSubArguments, ss, SkipEmpty,
        "        if (arg > 0.0)\n"
        "        {\n"
        "            NPV_reinvest += arg * Pow_reinvest;\n"
        "            bHasPosValue = true;\n"
        "        }\n"
        "        else if (arg < 0.0)\n"
        "        {\n"
        "             NPV_invest += arg * Pow_invest;\n"
        "             bHasNegValue = true;\n"
        "        }\n"
        "        Pow_reinvest /= reinvest;\n"
        "        Pow_invest /= invest;\n"
        "        nCount++;\n"
        );
    ss << "if ( !( bHasPosValue && bHasNegValue ) )\n";
    ss << "    return CreateDoubleError(IllegalArgument);\n";
    ss << "tmp = ";
    ss << "-NPV_reinvest /NPV_invest * pow(reinvest,(double)nCount-1);\n\t";
    ss << "tmp =  pow(tmp, 1.0 / (nCount - 1)) - 1.0;\n\t";
    ss << "return (double)tmp;\n";
    ss << "}";
}

void OpEffective::GenSlidingWindowFunction(outputstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n\t";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    ss << "    tmp = pow(1.0 + arg0 / arg1, arg1)-1.0;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

 void OpTbilleq::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDiffDate360_Decl);decls.insert(GetDiffDate360Decl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDate_LocalBarrierDecl);
    decls.insert(DaysInMonthDecl);decls.insert(GetNullDateDecl);
    decls.insert(IsLeapYearDecl);
    funs.insert(GetDiffDate360_);funs.insert(GetDiffDate360);
    funs.insert(DateToDays);funs.insert(DaysToDate_LocalBarrier);
    funs.insert(DaysInMonth);funs.insert(GetNullDate);
    funs.insert(IsLeapYear);
}
void OpTbilleq::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "   int gid0 = get_global_id(0);\n";
    ss << "double tmp = 0;\n\t";
    GenerateArg( "tmp000", 0, vSubArguments, ss );
    GenerateArg( "tmp001", 1, vSubArguments, ss );
    GenerateArg( "tmp002", 2, vSubArguments, ss );
    ss<<"tmp001+=1.0;\n";
    ss<<"int   nDiff =GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
    ss<<"tmp =( 365 * tmp002 ) / ( 360 - ( tmp002 * ( nDiff ) ) );\n";
    ss << "return tmp;\n";
    ss << "}";
}
void OpCumprinc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetPMTDecl); decls.insert(GetFVDecl);
    funs.insert(GetPMT);funs.insert(GetFV);
}
void OpCumprinc::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fNumPeriouds", 1, vSubArguments, ss );
    GenerateArg( "fVal", 2, vSubArguments, ss );
    GenerateArg( "fStartPer", 3, vSubArguments, ss );
    GenerateArg( "fEndPer", 4, vSubArguments, ss );
    GenerateArg( "fPayType", 5, vSubArguments, ss );
    ss << "    int nNumPeriods = (int)fNumPeriods;\n";
    ss << "    int nStartPer = (int)fStartPer;\n";
    ss << "    int nEndPer = (int)fEndPer;\n";
    ss << "    int nPayType = (int)fPayType;\n";
    ss <<"    double fPmt;\n";
    ss <<"    fPmt = GetPMT( fRate, nNumPeriods,fVal,0.0,nPayType != 0 );\n";
    ss <<"    if(nStartPer == 1)\n";
    ss <<"    {\n";
    ss <<"        if( nPayType <= 0 )\n";
    ss <<"            tmp = fPmt + fVal * fRate;\n";
    ss <<"        else\n";
    ss <<"            tmp = fPmt;\n";
    ss <<"        nStartPer=nStartPer+1;\n";
    ss <<"    }\n";
    ss <<"    for( int i = nStartPer ; i <= nEndPer ; i++ )\n";
    ss <<"    {\n";
    ss <<"        if( nPayType > 0 )\n";
    ss <<"            tmp += fPmt - ( GetFV( fRate,i - 2,";
    ss <<"fPmt,fVal,1)- fPmt ) * fRate;\n";
    ss <<"        else\n";
    ss <<"            tmp += fPmt - GetFV( fRate, i - 1,";
    ss <<"fPmt,fVal,0 ) * fRate;\n";
    ss <<"    }\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpAccrint::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(GetDiffDateDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(GetDiffDate);
}
void OpAccrint::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 7, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    GenerateArg( "fStartDate", 0, vSubArguments, ss );
    // 1 is ignored
    GenerateArg( "fEndDate", 2, vSubArguments, ss );
    GenerateArg( "fRate", 3, vSubArguments, ss );
    GenerateArg( "fVal", 4, vSubArguments, ss );
    GenerateArg( "fFreq", 5, vSubArguments, ss );
    GenerateArg( "fMode", 6, vSubArguments, ss );
    ss << "    int nStartDate = fStartDate;\n";
    ss << "    int nEndDate = fEndDate;\n";
    ss << "    int mode = fMode;\n";
    ss << "    int freq = fFreq;\n";
    ss << "    int nDays1stYear=0;\n";
    ss <<"    int nNullDate=GetNullDate();\n";
    ss <<"    int nTotalDays = GetDiffDate(nNullDate,nStartDate,";
    ss <<"nEndDate, mode,&nDays1stYear);\n";
    ss <<"    tmp = fVal*fRate*convert_double(nTotalDays)";
    ss <<"/convert_double(nDays1stYear);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpAccrintm::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(GetDiffDateDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(GetDiffDate);
}
void OpAccrintm::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";

    GenerateArg( "fStartDate", 0, vSubArguments, ss );
    GenerateArg( "fEndDate", 1, vSubArguments, ss );
    GenerateArg( "fRate", 2, vSubArguments, ss );
    GenerateArg( "fVal", 3, vSubArguments, ss );
    GenerateArg( "fMode", 4, vSubArguments, ss );
    ss << "    int nStartDate = fStartDate;\n";
    ss << "    int nEndDate = fEndDate;\n";
    ss << "    int mode = fMode;\n";
    ss <<"int nDays1stYear=0;\n\t";
    ss <<"int nNullDate=GetNullDate();\n\t";
    ss <<"int nTotalDays = GetDiffDate(nNullDate,nStartDate,";
    ss <<"nEndDate, mode,&nDays1stYear);\n\t";
    ss <<"tmp = fVal*fRate*convert_double(nTotalDays)";
    ss <<"/convert_double(nDays1stYear);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpYield::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(getYield_Decl);decls.insert(getPriceDecl);
    decls.insert(coupnumDecl);decls.insert(coupdaysncDecl);
    decls.insert(coupdaybsDecl);decls.insert(coupdaysDecl);
    decls.insert(lcl_GetcoupnumDecl);decls.insert(lcl_GetcoupdaysDecl);
    decls.insert(lcl_GetcoupdaybsDecl);decls.insert(getDiffDecl);
    decls.insert(getDaysInYearRangeDecl);decls.insert(GetDaysInYearDecl);
    decls.insert(GetDaysInYearsDecl);decls.insert(getDaysInMonthRangeDecl);
    decls.insert(addMonthsDecl);decls.insert(ScaDateDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysToDateDecl);decls.insert(DaysInMonthDecl);
    decls.insert(IsLeapYearDecl);decls.insert(setDayDecl);
    decls.insert(checklessthanDecl);

    funs.insert(getYield_);funs.insert(getPrice);
    funs.insert(coupnum);funs.insert(coupdaysnc);
    funs.insert(coupdaybs);funs.insert(coupdays);
    funs.insert(lcl_Getcoupnum);funs.insert(lcl_Getcoupdays);
    funs.insert(lcl_Getcoupdaybs);funs.insert(getDiff);
    funs.insert(getDaysInYearRange);funs.insert(GetDaysInYear);
    funs.insert(GetDaysInYears);funs.insert(getDaysInMonthRange);
    funs.insert(addMonths);funs.insert(ScaDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysToDate);funs.insert(DaysInMonth);
    funs.insert(IsLeapYear);decls.insert(setDay);
    funs.insert(checklessthan);
}

void OpYield::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 7, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    GenerateArg( "tmp000", 0, vSubArguments, ss );
    GenerateArg( "tmp001", 1, vSubArguments, ss );
    GenerateArg( "tmp002", 2, vSubArguments, ss );
    GenerateArg( "tmp003", 3, vSubArguments, ss );
    GenerateArg( "tmp004", 4, vSubArguments, ss );
    GenerateArg( "tmp005", 5, vSubArguments, ss );
    GenerateArg( "tmp006", 6, vSubArguments, ss );
    ss << "tmp = getYield_(";
    ss << "GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005,tmp006);\n\t ";
    ss << "return tmp;\n";
    ss << "}";
}

void OpSLN::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "cost", 0, vSubArguments, ss );
    GenerateArg( "salvage", 1, vSubArguments, ss );
    GenerateArg( "life", 2, vSubArguments, ss );
    ss << "    tmp = (cost-salvage)/life;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

 void OpYieldmat::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    decls.insert(GetYieldmatDecl);

    funs.insert(GetYearFrac);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
    funs.insert(GetYieldmat);
}

void OpYieldmat::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    GenerateArg( "tmp000", 0, vSubArguments, ss );
    GenerateArg( "tmp001", 1, vSubArguments, ss );
    GenerateArg( "tmp002", 2, vSubArguments, ss );
    GenerateArg( "tmp003", 3, vSubArguments, ss );
    GenerateArg( "tmp004", 4, vSubArguments, ss );
    GenerateArg( "tmp005", 5, vSubArguments, ss );
    ss << "tmp = GetYieldmat(";
    ss<<"GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpPMT::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetPMTDecl);
    funs.insert(GetPMT);
}

void OpPMT::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss<< "    double tmp = 0;\n";
    ss<< "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fNper", 1, vSubArguments, ss );
    GenerateArg( "fPv", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fFv", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayInAdvance", 4, 0, vSubArguments, ss );
    ss << "    return GetPMT( fRate, fNper, fPv, fFv, fPayInAdvance != 0 );\n";
    ss << "}";
}

void OpNPV::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 31 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0.0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nCount = 1;\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateRangeArgs( 1, vSubArguments.size() - 1, vSubArguments, ss, SkipEmpty,
        "        tmp += arg / pow( 1 + arg0, nCount );\n"
        "        nCount += 1;\n"
        );
    ss << "    return tmp;\n";
    ss << "}";
}

 void OpPrice::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
 {
    decls.insert(getPriceDecl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl);
    decls.insert(DateToDaysDecl);
    decls.insert(ScaDateDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(addMonthsDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(coupnumDecl);
    decls.insert(DateToDaysDecl);
    decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(coupdaybsDecl);
    decls.insert(lcl_GetcoupdaysDecl);
    decls.insert(lcl_GetcoupdaybsDecl);
    decls.insert(coupdaysDecl);
    decls.insert(coupdaysncDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs);
    funs.insert(coupdaybs);
    funs.insert(lcl_Getcoupdays);
    funs.insert(coupdaysnc);
    funs.insert(coupdays);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupnum);
    funs.insert(coupnum);funs.insert(getPrice);
 }
void OpPrice::GenSlidingWindowFunction(outputstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    GenerateArg( "tmp4", 4, vSubArguments, ss );
    GenerateArg( "tmp5", 5, vSubArguments, ss );
    GenerateArgWithDefault( "tmp6", 6, 0, vSubArguments, ss );
    ss << "    if(tmp4*tmp5 == 0) return NAN;\n";
    ss << "    tmp = getPrice(tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpOddlprice::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
{
    decls.insert(GetOddlpriceDecl);decls.insert(GetDiffDateDecl);
    decls.insert(GetYearDiffDecl);decls.insert(IsLeapYearDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysToDateDecl);decls.insert(DaysInMonthDecl);
    decls.insert(GetYearFracDecl);
    funs.insert(GetOddlprice);funs.insert(GetDiffDate);
    funs.insert(GetYearDiff);funs.insert(IsLeapYear);
    funs.insert(GetNullDate);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetYearFrac);
}
void OpOddlprice::GenSlidingWindowFunction(outputstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 7, 8 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    double tmp = 0;\n";
    ss <<"    int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    GenerateArg( "tmp4", 4, vSubArguments, ss );
    GenerateArg( "tmp5", 5, vSubArguments, ss );
    GenerateArg( "tmp6", 6, vSubArguments, ss );
    GenerateArgWithDefault( "tmp7", 7, 0, vSubArguments, ss );
    ss <<"    int nNullDate = GetNullDate();\n";
    ss <<"    tmp = GetOddlprice(nNullDate,tmp0,tmp1,";
    ss <<"tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpOddlyield::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
{
    decls.insert(GetDiffDateDecl);decls.insert(DaysToDateDecl);
    decls.insert(GetYearDiffDecl);decls.insert(IsLeapYearDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysInMonthDecl);
    decls.insert(GetYearFracDecl);decls.insert(GetOddlyieldDecl);
    funs.insert(GetDiffDate);funs.insert(DaysToDate);
    funs.insert(GetYearDiff);funs.insert(IsLeapYear);
    funs.insert(GetNullDate);funs.insert(DaysInMonth);
    funs.insert(DateToDays);
    funs.insert(GetYearFrac);funs.insert(GetOddlyield);
}
void OpOddlyield::GenSlidingWindowFunction(outputstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 7, 8 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    double tmp = 0;\n";
    ss <<"    int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    GenerateArg( "tmp4", 4, vSubArguments, ss );
    GenerateArg( "tmp5", 5, vSubArguments, ss );
    GenerateArg( "tmp6", 6, vSubArguments, ss );
    GenerateArgWithDefault( "tmp7", 7, 0, vSubArguments, ss );
    ss <<"    int nNullDate = GetNullDate();\n";
    ss <<"    tmp = GetOddlyield(nNullDate,tmp0,tmp1";
    ss <<",tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpPriceDisc::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
{
    decls.insert(GetYearDiffDecl);decls.insert(getDiffDecl);
    decls.insert(getDaysInYearRangeDecl);decls.insert(GetDaysInYearDecl);
    decls.insert(GetDaysInYearsDecl);decls.insert(getDaysInMonthRangeDecl);
    decls.insert(addMonthsDecl);decls.insert(ScaDateDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysToDateDecl);decls.insert(DaysInMonthDecl);
    decls.insert(IsLeapYearDecl);decls.insert(GetDiffDateDecl);
    funs.insert(GetYearDiff);funs.insert(getDiff);
    funs.insert(getDaysInYearRange);funs.insert(GetDaysInYear);
    funs.insert(GetDaysInYears);funs.insert(getDaysInMonthRange);
    funs.insert(addMonths);funs.insert(ScaDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysToDate);funs.insert(DaysInMonth);
    funs.insert(IsLeapYear);funs.insert(GetDiffDate);
}
void OpPriceDisc::GenSlidingWindowFunction(outputstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    GenerateArgWithDefault( "tmp4", 4, 0, vSubArguments, ss );
    ss <<"    int nNullDate = GetNullDate();\n";
    ss <<"    tmp=tmp3* ( 1.0 -tmp2*GetYearDiff( nNullDate, ";
    ss <<"tmp0,tmp1,tmp4));\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpNper::GenSlidingWindowFunction(outputstream &ss,
         const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fPmt", 1, vSubArguments, ss );
    GenerateArg( "fPV", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fFV", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayInAdvance", 4, 0, vSubArguments, ss );
    ss << "    if ( fPV + fFV == 0.0 )\n";
    ss << "        return 0.0;\n";
    ss << "    else if (fRate == 0.0)\n";
    ss << "        return -(fPV + fFV)/fPmt;\n";
    ss << "    else if (fPayInAdvance != 0)\n";
    ss << "        return log(-(fRate*fFV-fPmt*(1.0+fRate))/(fRate*fPV+fPmt*(1.0+fRate)))\n";
    ss << "                  / log1p(fRate);\n";
    ss << "    else\n";
    ss << "        return log(-(fRate*fFV-fPmt)/(fRate*fPV+fPmt)) / log1p(fRate);\n";
    ss << "}\n";
 }

void OpPPMT::BinInlineFun(std::set<std::string>& decls,
        std::set<std::string>& funs)
{
    decls.insert(GetFVDecl);
    funs.insert(GetFV);
    decls.insert(GetPMTDecl);
    funs.insert(GetPMT);
    decls.insert(GetIpmtDecl);
    funs.insert(GetIpmt);
}

void OpPPMT::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg=0;\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fPer", 1, vSubArguments, ss );
    GenerateArg( "fNper", 2, vSubArguments, ss );
    GenerateArg( "fPv", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fFv", 4, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayInAdvance", 5, 0, vSubArguments, ss );
    ss << "    if (fPer < 1.0 || fPer > fNper)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        double fPmt;\n";
    ss << "        double fInterestPer = GetIpmt(fRate, fPer, fNper, fPv, fFv, fPayInAdvance != 0, &fPmt);\n";
    ss << "        return fPmt - fInterestPer;\n";
    ss << "    }\n";
    ss << "}\n";
}

void OpCoupdaybs::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_GetcoupdaybsDecl);
    decls.insert(coupdaybsDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs);
    funs.insert(coupdaybs);
}
void OpCoupdaybs::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    tmp = coupdaybs(nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpCoupdays::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_GetcoupdaysDecl);
    decls.insert(coupdaysDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(lcl_Getcoupdays);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(coupdays);
}
void OpCoupdays::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    tmp = coupdays(nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss << "}";
}
void OpCouppcd::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl);
    decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_GetCouppcdDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);
    funs.insert(ScaDate);
    funs.insert(addMonths);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_GetCouppcd);
}
void OpCouppcd::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    int nNullDate=693594;\n";
    ss <<"    tmp = lcl_GetCouppcd(nNullDate,nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpCoupncd::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl);
    decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_GetCoupncdDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);
    funs.insert(ScaDate);
    funs.insert(addMonths);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_GetCoupncd);
}
void OpCoupncd::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    int nNullDate=693594;\n";
    ss <<"    tmp = lcl_GetCoupncd(nNullDate,nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpCoupdaysnc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(DateToDaysDecl);
    decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(coupdaybsDecl);
    decls.insert(lcl_GetcoupdaysDecl);
    decls.insert(lcl_GetcoupdaybsDecl);
    decls.insert(coupdaysDecl);
    decls.insert(coupdaysncDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs);
    funs.insert(coupdaybs);
    funs.insert(lcl_Getcoupdays);
    funs.insert(coupdaysnc);
    funs.insert(coupdays);
}
void OpCoupdaysnc::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    tmp = coupdaysnc(nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss << "}";
}

void OpCoupnum::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl);
    decls.insert(DateToDaysDecl);
    decls.insert(ScaDateDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(addMonthsDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(coupnumDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(addMonths);funs.insert(lcl_Getcoupnum);
    funs.insert(coupnum);
}
void OpCoupnum::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 4 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fFreq", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 3, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int nFreq = fFreq;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    tmp = coupnum(nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss << "}";
}
void OpAmordegrc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(nCorrValDecl); decls.insert(RoundDecl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(GetYearFracDecl);
    funs.insert(Round);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(GetYearFrac);
}
void OpAmordegrc::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    GenerateArg( "fCost", 0, vSubArguments, ss );
    GenerateArg( "fDate", 1, vSubArguments, ss );
    GenerateArg( "fFirstPer", 2, vSubArguments, ss );
    GenerateArg( "fRestVal", 3, vSubArguments, ss );
    GenerateArg( "fPer", 4, vSubArguments, ss );
    GenerateArg( "fRate", 5, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 6, 0, vSubArguments, ss );
    ss << "    int nDate = fDate;\n";
    ss << "    int nFirstPer = fFirstPer;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    uint nPer = convert_int( fPer );\n";
    ss <<"    double fUsePer = 1.0 / fRate;\n";
    ss <<"    double fAmorCoeff;\n";
    ss <<"    if( fUsePer < 3.0 )\n";
    ss <<"        fAmorCoeff = 1.0;\n";
    ss <<"    else if( fUsePer < 5.0 )\n";
    ss <<"        fAmorCoeff = 1.5;\n";
    ss <<"    else if( fUsePer <= 6.0 )\n";
    ss <<"        fAmorCoeff = 2.0;\n";
    ss <<"    else\n";
    ss <<"        fAmorCoeff = 2.5;\n";
    ss <<"    fRate *= fAmorCoeff;\n";
    ss <<"    tmp = Round( GetYearFrac( 693594,";
    ss <<"nDate, nFirstPer, nBase ) * fRate * fCost);\n";
    ss <<"    fCost = fCost-tmp;\n";
    ss <<"    double fRest = fCost - fRestVal;\n";
    ss <<"    for( uint n = 0 ; n < nPer ; n++ )\n";
    ss <<"    {\n";
    ss <<"        tmp = Round( fRate * fCost);\n";
    ss <<"        fRest -= tmp;\n";
    ss <<"        if( fRest < 0.0 )\n";
    ss <<"        {\n";
    ss <<"            switch( nPer - n )\n";
    ss <<"            {\n";
    ss <<"                case 0:\n";
    ss <<"                case 1:\n";
    ss <<"                    tmp = Round( fCost * 0.5);\n";
    ss <<"                default:\n";
    ss <<"                    tmp = 0.0;\n";
    ss <<"            }\n";
    ss <<"        }\n";
    ss <<"        fCost -= tmp;\n";
    ss <<"    }\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpAmorlinc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(nCorrValDecl); decls.insert(RoundDecl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetYearFracDecl);
    funs.insert(Round);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetYearFrac);
}
void OpAmorlinc::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    GenerateArg( "fCost", 0, vSubArguments, ss );
    GenerateArg( "fDate", 1, vSubArguments, ss );
    GenerateArg( "fFirstPer", 2, vSubArguments, ss );
    GenerateArg( "fRestVal", 3, vSubArguments, ss );
    GenerateArg( "fPer", 4, vSubArguments, ss );
    GenerateArg( "fRate", 5, vSubArguments, ss );
    GenerateArgWithDefault( "fBase", 6, 0, vSubArguments, ss );
    ss << "    int nDate = fDate;\n";
    ss << "    int nFirstPer = fFirstPer;\n";
    ss << "    int nBase = fBase;\n";
    ss <<"    int  nPer = convert_int( fPer );\n";
    ss <<"    double fOneRate = fCost * fRate;\n";
    ss <<"    double fCostDelta = fCost - fRestVal;\n";
    ss <<"    double f0Rate = GetYearFrac( 693594,";
    ss <<"nDate, nFirstPer, nBase )* fRate * fCost;\n";
    ss <<"    int nNumOfFullPeriods = (int)";
    ss <<"( ( fCost - fRestVal - f0Rate) / fOneRate );\n";
    ss <<"    if( nPer == 0 )\n";
    ss <<"        tmp = f0Rate;\n";
    ss <<"    else if( nPer <= nNumOfFullPeriods )\n";
    ss <<"        tmp = fOneRate;\n";
    ss <<"    else if( nPer == nNumOfFullPeriods + 1 )\n";
    ss <<"        tmp = fCostDelta - fOneRate * nNumOfFullPeriods - f0Rate;\n";
    ss <<"    else\n";
    ss <<"        tmp = 0.0;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpReceived::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearDiffDecl);decls.insert(GetDiffDateDecl);
    decls.insert(DaysToDateDecl);decls.insert(DaysInMonthDecl);
    decls.insert(GetNullDateDecl);decls.insert(IsLeapYearDecl);
    decls.insert(DateToDaysDecl);
    funs.insert(GetDiffDate);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(IsLeapYear);
    funs.insert(GetYearDiff);
}

void OpReceived::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fSettle", 0, vSubArguments, ss );
    GenerateArg( "fMat", 1, vSubArguments, ss );
    GenerateArg( "fInvest", 2, vSubArguments, ss );
    GenerateArg( "fDisc", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fOB", 4, 0, vSubArguments, ss );
    ss << "    int nSettle = fSettle;\n";
    ss << "    int nMat = fMat;\n";
    ss << "    int rOB = fOB;\n";
    ss << "    double tmpvalue = (1.0-(fDisc";
    ss <<" * GetYearDiff( GetNullDate()";
    ss <<",nSettle,nMat,rOB)));\n";
    ss << "    tmp = fInvest/tmpvalue;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpYielddisc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);

    funs.insert(GetYearFrac);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}
void OpYielddisc::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(5,5);
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp000", 0, vSubArguments, ss );
    GenerateArg( "tmp001", 1, vSubArguments, ss );
    GenerateArg( "tmp002", 2, vSubArguments, ss );
    GenerateArg( "tmp003", 3, vSubArguments, ss );
    GenerateArg( "tmp004", 4, vSubArguments, ss );
    ss<< "\t";
    ss<< "if(tmp002 <= 0 || tmp003 <= 0 || tmp000 >= tmp001 )\n\t";
    ss<< "    return CreateDoubleError(IllegalArgument);\n\t";
    ss<< "tmp = (tmp003/tmp002)-1;\n\t";
    ss << "tmp /= GetYearFrac( GetNullDate(),tmp000,tmp001,tmp004);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpTbillprice::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);

    funs.insert(GetYearFrac);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpTbillprice::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";

    ss << "    int singleIndex = gid0;\n";
    ss << "    int doubleIndex = gid0;\n";
    ss << "    int i = gid0;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);

    ss << "    tmp1+=1.0;\n";
    ss << "    double  fFraction =GetYearFrac(693594,tmp0,tmp1,0);\n";
    ss << "    tmp = 100.0 * ( 1.0 - tmp2 * fFraction );\n";
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpRate::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(RateIterationDecl);
    funs.insert(RateIteration);
}

void OpRate::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fNper", 0, vSubArguments, ss );
    GenerateArg( "fPayment", 1, vSubArguments, ss );
    GenerateArg( "fPv", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fFv", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayType", 4, 0, vSubArguments, ss );
    ss << "    bool bPayType = fPayType != 0;\n";
    if( vSubArguments.size() == 6 )
    {
        GenerateArgWithDefault( "fGuess", 5, 0.1, vSubArguments, ss );
        ss << "    double fOrigGuess = fGuess;\n";
        ss << "    bool bDefaultGuess = false;\n";
    }
    else
    {
        ss << "    double fGuess = 0.1, fOrigGuess = 0.1;\n";
        ss << "    bool bDefaultGuess = true;\n";
    }
    ss << "    if( fNper <= 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    bool bValid = RateIteration(fNper, fPayment, fPv, fFv, bPayType, &fGuess);\n";
    ss << "    if (!bValid)\n";
    ss << "    {\n";
    ss << "        if (bDefaultGuess)\n";
    ss << "        {\n";
    ss << "            double fX = fOrigGuess;\n";
    ss << "            for (int nStep = 2; nStep <= 10 && !bValid; ++nStep)\n";
    ss << "            {\n";
    ss << "                fGuess = fX * nStep;\n";
    ss << "                bValid = RateIteration( fNper, fPayment, fPv, fFv, bPayType, &fGuess);\n";
    ss << "                if (!bValid)\n";
    ss << "                {\n";
    ss << "                    fGuess = fX / nStep;\n";
    ss << "                    bValid = RateIteration( fNper, fPayment, fPv, fFv, bPayType, &fGuess);\n";
    ss << "                }\n";
    ss << "            }\n";
    ss << "        }\n";
    ss << "        if (!bValid)\n";
    ss << "            return CreateDoubleError(NoConvergence);\n";
    ss << "    }\n";
    ss << "    return fGuess;\n";
    ss << "}\n";
}

void OpTbillyield::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDiffDate360Decl);decls.insert(IsLeapYearDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDate_LocalBarrierDecl);
    decls.insert(DaysInMonthDecl);decls.insert(GetNullDateDecl);
    decls.insert(GetDiffDate360_Decl);
    funs.insert(GetDiffDate360);funs.insert(DateToDays);
    funs.insert(DaysToDate_LocalBarrier);funs.insert(IsLeapYear);
    funs.insert(DaysInMonth);funs.insert(GetNullDate);
    funs.insert(GetDiffDate360_);

}

void OpTbillyield::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    GenerateArg( "tmp000", 0, vSubArguments, ss );
    GenerateArg( "tmp001", 1, vSubArguments, ss );
    GenerateArg( "tmp002", 2, vSubArguments, ss );
    ss <<"    int nDiff=GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
    ss <<"    nDiff++;\n";
    ss <<"    tmp=100.0;\n";
    ss <<"    tmp = tmp / tmp002;\n";
    ss <<"    tmp = tmp - 1.0;\n";
    ss <<"    tmp = tmp / nDiff;\n";
    ss <<"    tmp = tmp * 360.0;\n";
    ss <<"    return tmp;\n";
    ss << "}\n";
}

void OpDDB::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(ScGetDDBDecl);
    funs.insert(ScGetDDB);
}

void OpDDB::GenSlidingWindowFunction(outputstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    GenerateArg( "fCost", 0, vSubArguments, ss );
    GenerateArg( "fSalvage", 1, vSubArguments, ss );
    GenerateArg( "fLife", 2, vSubArguments, ss );
    GenerateArg( "fPeriod", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fFactor", 4, 2, vSubArguments, ss );
    ss << "    if (fCost < 0.0 || fSalvage < 0.0 || fFactor <= 0.0 || fSalvage > fCost\n";
    ss << "        || fPeriod < 1.0 || fPeriod > fLife)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "   return ScGetDDB( fCost, fSalvage, fLife, fPeriod, fFactor );\n";
    ss << "}\n";
}

void OpPV::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double result = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fRate", 0, vSubArguments, ss );
    GenerateArg( "fNper", 1, vSubArguments, ss );
    GenerateArg( "fPmt", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fFv", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "fPayInAdvance", 4, 0, vSubArguments, ss );
    ss << "    double fPv;\n";
    ss << "    if (fRate == 0.0)\n";
    ss << "        fPv = fFv + fPmt * fNper;\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        if (fPayInAdvance != 0)\n";
    ss << "            fPv = (fFv * pow(1.0 + fRate, -fNper))\n";
    ss << "                    + (fPmt * (1.0 - pow(1.0 + fRate, -fNper + 1.0)) / fRate)\n";
    ss << "                    + fPmt;\n";
    ss << "        else\n";
    ss << "            fPv = (fFv * pow(1.0 + fRate, -fNper))\n";
    ss << "                    + (fPmt * (1.0 - pow(1.0 + fRate, -fNper)) / fRate);\n";
    ss << "    }\n";
    ss << "    return -fPv;\n";
    ss << "}\n";
}

void OpVDB::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(is_representable_integerDecl);
    decls.insert(ScGetDDBDecl);decls.insert(approx_equalDecl);
    decls.insert(ScInterVDBDecl);decls.insert(VDBImplementDecl);
    funs.insert(is_representable_integer);
    funs.insert(ScGetDDB);funs.insert(approx_equal);
    funs.insert(ScInterVDB);funs.insert(VDBImplement);
}

void OpVDB::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 5, 7 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    GenerateArg( "fCost", 0, vSubArguments, ss );
    GenerateArg( "fSalvage", 1, vSubArguments, ss );
    GenerateArg( "fLife", 2, vSubArguments, ss );
    GenerateArg( "fStart", 3, vSubArguments, ss );
    GenerateArg( "fEnd", 4, vSubArguments, ss );
    GenerateArgWithDefault( "fFactor", 5, 2, vSubArguments, ss );
    GenerateArgWithDefault( "fNoSwitch", 6, 0, vSubArguments, ss );
    ss << "    if (fStart < 0.0 || fEnd < fStart || fEnd > fLife || fCost < 0.0\n";
    ss << "        || fSalvage > fCost || fFactor <= 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return VDBImplement(fCost, fSalvage, fLife, fStart, fEnd, fFactor, fNoSwitch != 0);\n";
    ss << "}";
}

void OpXirr::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArgWithDefault( "fResultRate", 2, 0.1, vSubArguments, ss );
    ss << "    if(fResultRate<=-1)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double fMaxEps = 1e-10;\n";
    ss << "    int nMaxIter = 50;\n";
    ss << "    int nIter = 0;\n";
    ss << "    double fResultValue;\n";
    ss << "    int nIterScan = 0;\n";
    ss << "    bool bContLoop = false;\n";
    ss << "    bool bResultRateScanEnd = false;\n";
    // Make 'V_0' and 'D_0' be the first elements of arguments 0 and 1.
    GenerateRangeArgElement( "V_0", 0, "0", vSubArguments, ss, EmptyIsZero );
    GenerateRangeArgElement( "D_0", 1, "0", vSubArguments, ss, EmptyIsZero );
    ss << "    do\n";
    ss << "    {\n";
    ss << "        if (nIterScan >=1)\n";
    ss << "            fResultRate = -0.99 + (nIterScan -1)* 0.01;\n";
    ss << "        do\n";
    ss << "        {\n";
    ss << "            double r = fResultRate + 1;\n";
    ss << "            fResultValue = V_0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "                fResultValue += arg1/pow(r,(arg2 - D_0)/365.0);\n"
        , "1" // start from 2nd element
        );
    ss << "            double fResultValue2 = 0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, SkipEmpty,
        "                double E_i = (arg2 - D_0)/365.0;\n"
        "                fResultValue2 -= E_i * arg1 / pow(r,E_i + 1.0);\n"
        , "1" // start from 2nd element
        );
    ss << "            double fNewRate = fResultRate - fResultValue / fResultValue2;\n";
    ss << "            double fRateEps = fabs( fNewRate - fResultRate );\n";
    ss << "            fResultRate = fNewRate;\n";
    ss << "            bContLoop = (fRateEps > fMaxEps) && (fabs( fResultValue ) > fMaxEps);\n";
    ss << "        } while( bContLoop && (++nIter < nMaxIter) );\n";
    ss << "        nIter = 0;\n";
    ss << "        if( isnan(fResultRate) || isinf(fResultRate) || isnan(fResultValue) || isinf(fResultValue))\n";
    ss << "            bContLoop = true;\n";
    ss << "        ++nIterScan;\n";
    ss << "        bResultRateScanEnd = (nIterScan >= 200);\n";
    ss << "    } while(bContLoop && !bResultRateScanEnd);\n";
    ss << "    if( bContLoop )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return fResultRate;\n";
    ss << "}";
}

void OpDB::GenSlidingWindowFunction(outputstream& ss,
            const std::string &sSymName, SubArguments& vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "fCost", 0, vSubArguments, ss );
    GenerateArg( "fSalvage", 1, vSubArguments, ss );
    GenerateArg( "fLife", 2, vSubArguments, ss );
    GenerateArg( "fPeriod", 3, vSubArguments, ss );
    GenerateArgWithDefault( "fMonths", 4, 12, vSubArguments, ss );
    ss << "    int nMonths = (int)fMonths;\n";
    ss << "    if (fMonths < 1.0 || fMonths > 12.0 || fLife > 1200.0 || fSalvage < 0.0 ||\n";
    ss << "        fPeriod > (fLife + 1.0) || fSalvage > fCost || fCost <= 0.0 ||\n";
    ss << "        fLife <= 0 || fPeriod <= 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    double tmp = 0;\n";
    ss <<"    double fDeprRate = 1.0 - pow(fSalvage / fCost, 1.0 / fLife);\n";
    ss <<"    fDeprRate = ((int)(fDeprRate * 1000.0 + 0.5)) / 1000.0;\n";
    ss <<"    double fFirstDeprRate = fCost * fDeprRate * nMonths / 12.0;\n";
    ss <<"    double fDb = 0.0;\n";
    ss <<"    if ((int)(fPeriod) == 1)\n";
    ss <<"        fDb = fFirstDeprRate;\n";
    ss <<"    else\n";
    ss <<"    {\n";
    ss <<"        double fSumDeprRate = fFirstDeprRate;\n";
    ss <<"        double fMin = fLife;\n";
    ss <<"        if (fMin > fPeriod) fMin = fPeriod;\n";
    ss <<"        int nMax = (int)fMin;\n";
    ss <<"        for (int i = 2; i <= nMax; i++)\n";
    ss <<"        {\n";
    ss <<"            fDb = (fCost - fSumDeprRate) * fDeprRate;\n";
    ss <<"            fSumDeprRate += fDb;\n";
    ss <<"        }\n";
    ss <<"        if (fPeriod > fLife)\n";
    ss <<"            fDb = ((fCost - fSumDeprRate)";
    ss <<"* fDeprRate * (12.0 - nMonths)) / 12.0;\n";
    ss <<"    }\n";
    ss <<"    tmp = fDb;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
