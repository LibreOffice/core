/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_financial.hxx"

#include <formula/vectortoken.hxx>
#include <sstream>

using namespace formula;

namespace sc::opencl {
// Definitions of inline functions
#include "op_financial_helpers.hxx"

void RRI::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
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
    decls.insert(GetYearFrac_newDecl);decls.insert(DaysToDate_newDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetYearFrac_new);funs.insert(DaysToDate_new);
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
    ss << " GetYearFrac_new(nNullDate, (int)arg0, (int)arg1, (int)arg4);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpINTRATE::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearDiff_newDecl);decls.insert(GetDiffDate_newDecl);
    decls.insert(DaysToDate_newDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysInMonthDecl);
    decls.insert(IsLeapYearDecl);
    funs.insert(GetYearDiff_new);funs.insert(GetDiffDate_new);
    funs.insert(DaysToDate_new);funs.insert(GetNullDate);
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
    ss << "    tmp = ((arg3 / arg2) - 1) / GetYearDiff_new(nNullDate, (int)arg0,";
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
}

void OpIPMT::GenSlidingWindowFunction(outputstream& ss,
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
    ss << "    double pmt ;\n";
    ss << "    if(arg0 == 0.0)\n";
    ss << "        return 0;\n";
    ss << "    double temp1 = 0;\n";
    ss << "    double abl = pow(1.0 + arg0, arg2);\n";
    ss << "    temp1 -= arg4;\n";
    ss << "    temp1 -= arg3 * abl;\n";
    ss << "    pmt = temp1 / (1.0 + arg0 * arg5) /";
    ss << " ( (abl - 1.0) / arg0);\n";
    ss << "    double temp = pow( 1 + arg0, arg1 - 2);\n";
    ss << "    if(arg1 == 1.0)\n";
    ss << "    {\n";
    ss << "        if(arg5 > 0.0)\n";
    ss << "            tmp = 0.0;\n";
    ss << "        else\n";
    ss << "            tmp = -arg3;\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        if(arg5 > 0.0)\n";
    ss << "            tmp = GetFV(arg0, arg1 - 2.0, pmt, arg3, 1.0)";
    ss << " - pmt;\n";
    ss << "        else\n";
    ss << "            tmp = GetFV(arg0, arg1 - 1.0, pmt, arg3, 0.0);\n";
    ss << "    }\n";
    ss << "    tmp = tmp * arg0;\n";
    ss << "    return tmp;\n";
    ss << "}";
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
    decls.insert(GetDuration_newDecl);decls.insert(lcl_Getcoupnum_newDecl);
    decls.insert(addMonthsDecl);decls.insert(checklessthanDecl);
    decls.insert(setDayDecl);decls.insert(ScaDateDecl);
    decls.insert(GetYearFracDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetDuration_new);funs.insert(lcl_Getcoupnum_new);
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
    ss << "    tmp = GetDuration_new( nNullDate, (int)arg0, (int)arg1, arg2,";
    ss << " arg3, (int)arg4, (int)arg5);\n";
    ss << "    tmp = tmp * pow(1.0 + arg3 * pow((int)arg4, -1.0), -1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void Fvschedule::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    FormulaToken* pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    if(vSubArguments[0]->GetFormulaToken()->GetType() != formula::svDoubleVectorRef)
        throw Unhandled( __FILE__, __LINE__ );
    const formula::DoubleVectorRefToken* pCurDVR =
        static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = 1.0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "if (isnan(arg0))\n\t\t";
    ss << "arg0 = 0;\n\t";
    ss << "double arg1;\n\t";
    ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
    ss << "for (int i = 0; i + gid0 < arrayLength &&";
    ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
    ss << "arg1 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n\t\t\t";
    ss << "if (isnan(arg1))\n\t\t\t\t";
    ss << "arg1 = 0;\n\t\t\t";
    ss << "tmp *= arg1 + 1.0;\n\t\t";
    ss << "}\n\t";
    ss << "return (double)tmp * arg0";
    ss << ";\n}";
}
void Cumipmt::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetPMT_newDecl); decls.insert(GetFV_newDecl);
    funs.insert(GetPMT_new);funs.insert(GetFV_new);
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
    ss <<"    fPmt = GetPMT_new( fRate, nNumPeriods, fVal, 0.0, nPayType );\n";
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
    ss <<"            tmp += GetFV_new( fRate,  nStartPer - 2 , ";
    ss <<"fPmt, fVal, 1 ) - fPmt;\n";
    ss <<"        else\n";
    ss <<"            tmp += GetFV_new( fRate,  nStartPer - 1 , ";
    ss <<"fPmt, fVal, 0 );\n";
    ss <<"    }\n";
    ss <<"    tmp *= fRate;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void IRR::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    #define  Epsilon   1.0E-7\n";
    ss << "    int gid0 = get_global_id(0);\n";
    FormulaToken* pSur = vSubArguments[1]->GetFormulaToken();
    assert(pSur);
    ss << "    double fEstimated = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    double fEps = 1.0;\n";
    ss << "    double x = 0.0, xNew = 0.0, fNumerator = 0.0, fDenominator = 0.0;\n";
    ss << "    double nCount = 0.0;\n";
    if (pSur->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pSur);
        ss << "    if (gid0 >= " << pSVR->GetArrayLength() << ")\n";
        ss << "        fEstimated = 0.1;\n";
        ss << "    if (isnan(fEstimated))\n";
        ss << "        x = 0.1;\n";
        ss << "    else\n";
    }
    else if (pSur->GetType() == formula::svDouble)
    {
        ss << "    if (isnan(fEstimated))\n";
        ss << "        x = 0.1;\n";
        ss << "    else\n";
    }
    ss << "        x = fEstimated;\n";
    ss << "    unsigned short nItCount = 0;\n";
    ss << "    while (fEps > Epsilon && nItCount < 20){\n";
    ss << "        nCount = 0.0; fNumerator = 0.0;  fDenominator = 0.0;\n";
    ss << "        double arg0, arg1;\n";
    ss << "        int i = 0;\n";
    FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken* >(pCur);
    size_t nCurWindowSize = pDVR->GetRefRowSize();
    ss << "        for ( ";
    if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
        ss << "i = gid0; i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize << " /2*2; i++){\n";
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;;\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (!isnan(arg0)){\n";
        ss << "            fNumerator += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
        ss << "            if (!isnan(arg1)){\n";
        ss << "                fNumerator += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount += 1;\n";
        ss << "            }\n";
        ss << "        }\n";
        ss << "if(i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize << ") ;{\n";
    }
    else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
        ss << "; i < " << pDVR->GetArrayLength();
        ss << " && i < (gid0+" << nCurWindowSize << " )/2*2; i++){\n";
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (!isnan(arg0)){\n";
        ss << "            fNumerator += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
        ss << "            i++;\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (!isnan(arg1)){\n";
        ss << "                fNumerator += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
        ss << "        }\n";
        ss << "        if(i < " << pDVR->GetArrayLength();
        ss << " && i < gid0+" << nCurWindowSize << "){\n";
    }
    else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
        ss << " ; i + gid0 < " << pDVR->GetArrayLength();
        ss << " &&  i < " << nCurWindowSize << " /2*2; i++){\n";
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;;\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (!isnan(arg0)){\n";
        ss << "            fNumerator += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
        ss << "            if (!isnan(arg1)){\n";
        ss << "                fNumerator += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
        ss << "        }\n";
        ss << "        if(i + gid0 < " << pDVR->GetArrayLength() << " &&";
        ss << " i < " << nCurWindowSize << "){\n";

    } else {
        ss << "; i < " << nCurWindowSize << " /2*2; i++){\n";
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;;\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            if (!isnan(arg0)){\n";
        ss << "            fNumerator += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
        ss << "            if (!isnan(arg1)){\n";
        ss << "                fNumerator += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fDenominator+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
        ss << "        }\n";
        ss << "if(i<" << nCurWindowSize << "){\n";

    }
    ss << "            arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "        if (isnan(arg0))\n";
    ss << "            continue;\n";
    ss << "        fNumerator += arg0 / pow(1.0+x, nCount);\n";
    ss << "        fDenominator  += -nCount * arg0 / pow(1.0+x,nCount+1.0);\n";
    ss << "        nCount+=1;\n";
    ss << "        }\n";
    ss << "        xNew = x - fNumerator / fDenominator;\n";
    ss << "        fEps = fabs(xNew - x);\n";
    ss << "        x = xNew;\n";
    ss << "        nItCount++;\n    }\n";
    ss << "        if (fEstimated == 0.0 && fabs(x) < Epsilon)\n";
    ss << "            x = 0.0;\n";
    ss << "        if (fEps < Epsilon)\n";
    ss << "            return x;\n";
    ss << "        else\n";
    // FIXME: This is of course horribly wrong. 523 is the error code NoConvergence, and this should
    // be CreateDoubleError(523). Ditto for the other occurrences of 523 in the OpenCL code
    // generated in this file.
    ss << "            return (double)523;\n";
    ss << "}";
}

void XNPV::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double result = 0.0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int i=0;\n\t";
    ss << "double date;\n\t";
    ss << "double value;\n\t";
    ss << "double rate;\n\t";
    ss << "double dateNull;\n\t";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::DoubleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::DoubleVectorRefToken *>(tmpCur2);
    ss<< "int buffer_rate_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_value_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_date_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<<"if((gid0)>=buffer_date_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"return NAN;\n\telse \n";
    ss<<"dateNull = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if((gid0)>=buffer_rate_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"return NAN;\n\telse \n";
    ss<<"rate = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(1 == buffer_date_len )\n";
    ss<<"return ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss << "for (int i = ";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
    {
        ss << "gid0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
    }
    else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss << "0; i < gid0+"<< nCurWindowSize <<"; i++)\n\t\t";
    }
    else
    {
        ss << "0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
    }
    ss << "{\n\t";
    if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss <<  "if((i+gid0)>=buffer_value_len || (i+gid0)>=buffer_date_len)\n\t\t";
        ss <<  "return result;\n\telse \n\t\t";
    }
    else
    {
        ss <<  "if(i>=buffer_value_len || i>=buffer_date_len)\n\t\t";
        ss <<  "return result;\n\telse \n\t\t";
    }

    ss <<  "value = ";
    ss <<  vSubArguments[1]->GenSlidingWindowDeclRef(true);
    ss <<  ";\n";
    ss <<  " date = ";
    ss <<  vSubArguments[2]->GenSlidingWindowDeclRef(true);
    ss <<  ";\n";
    ss <<  "result += value/(pow((rate+1),(date-dateNull)/365));\n";
    ss <<  "}\n";
    ss <<  "return result;\n";
    ss <<  "}";
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

void MIRR::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 3 );
    FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
    static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    FormulaToken* pCur1 = vSubArguments[1]->GetFormulaToken();
    assert(pCur1);
    const formula::SingleVectorRefToken* pSVR1 =
        static_cast< const formula::SingleVectorRefToken* >(pCur1);
    assert(pSVR1);
    FormulaToken* pCur2 = vSubArguments[2]->GetFormulaToken();
    assert(pCur2);
    const formula::SingleVectorRefToken* pSVR2 =
        static_cast< const formula::SingleVectorRefToken* >(pCur2);
    assert(pSVR2);

    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double arg0, arg1, arg2;\n\t";
    ss << "arg1 = " << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "arg2 = " << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "int argLen1 = " << pSVR1->GetArrayLength() << ";\n\t";
    ss << "int argLen2 = " << pSVR2->GetArrayLength() << ";\n\t";
    ss << "if (gid0 >= argLen1)\n\t\t";
    ss << "arg1 = 0.0;\n\t";
    ss << "if (gid0 >= argLen2)\n\t\t";
    ss << "arg2 = 0.0;\n\t";
    ss << "if (isnan(arg1))\n\t\t";
    ss << "arg1 = 0.0;\n\t";
    ss << "if (isnan(arg2))\n\t\t";
    ss << "arg2 = 0.0;\n\t";
    ss << "double invest = arg1 + 1.0;\n\t";
    ss << "double reinvest = arg2 + 1.0;\n\t";
    ss << "double NPV_invest = 0.0;\n\t";
    ss << "double Pow_invest = 1.0;\n\t";
    ss << "double NPV_reinvest = 0.0;\n\t";
    ss << "double Pow_reinvest = 1.0;\n\t";
    ss << "int nCount = 0;\n\t";
    ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
    ss << "for (int i = 0; i + gid0 < arrayLength &&";
    ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
    ss << "arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t\t";
    ss << "if (isnan(arg0))\n\t\t\t";
    ss << "continue;\n\t\t";
    ss << "if (arg0 > 0.0)\n\t\t\t";
    ss << "NPV_reinvest += arg0 * Pow_reinvest;\n\t\t";
    ss << "else if (arg0 < 0.0)\n\t\t\t";
    ss << "NPV_invest += arg0 * Pow_invest;\n\t\t";
    ss << "Pow_reinvest /= reinvest;\n\t\t";
    ss << "Pow_invest /= invest;\n\t\t";
    ss << "nCount++;\n\t";
    ss << "}\n\t";
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
    decls.insert(GetPMT_newDecl); decls.insert(GetFV_newDecl);
    funs.insert(GetPMT_new);funs.insert(GetFV_new);
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
    ss <<"    fPmt = GetPMT_new( fRate, nNumPeriods,fVal,0.0,nPayType );\n";
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
    ss <<"            tmp += fPmt - ( GetFV_new( fRate,i - 2,";
    ss <<"fPmt,fVal,1)- fPmt ) * fRate;\n";
    ss <<"        else\n";
    ss <<"            tmp += fPmt - GetFV_new( fRate, i - 1,";
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
    decls.insert(getYield_Decl);decls.insert(getPrice_Decl);
    decls.insert(coupnumDecl);decls.insert(coupdaysncDecl);
    decls.insert(coupdaybsDecl);decls.insert(coupdaysDecl);
    decls.insert(lcl_GetcoupnumDecl);decls.insert(lcl_GetcoupdaysDecl);
    decls.insert(lcl_GetcoupdaybsDecl);decls.insert(getDiffDecl);
    decls.insert(getDaysInYearRangeDecl);decls.insert(GetDaysInYearDecl);
    decls.insert(GetDaysInYearsDecl);decls.insert(getDaysInMonthRangeDecl);
    decls.insert(addMonthsDecl);decls.insert(ScaDateDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysToDateDecl);decls.insert(DaysInMonthDecl);
    decls.insert(IsLeapYearDecl);

    funs.insert(getYield_);funs.insert(getPrice_);
    funs.insert(coupnum);funs.insert(coupdaysnc);
    funs.insert(coupdaybs);funs.insert(coupdays);
    funs.insert(lcl_Getcoupnum);funs.insert(lcl_Getcoupdays);
    funs.insert(lcl_Getcoupdaybs);funs.insert(getDiff);
    funs.insert(getDaysInYearRange);funs.insert(GetDaysInYear);
    funs.insert(GetDaysInYears);funs.insert(getDaysInMonthRange);
    funs.insert(addMonths);funs.insert(ScaDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysToDate);funs.insert(DaysInMonth);
    funs.insert(IsLeapYear);
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
    decls.insert(GetYearFrac_newDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDate_newDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    decls.insert(GetYieldmatDecl);

    funs.insert(GetYearFrac_new);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysToDate_new);
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

void OpPMT::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArgWithDefault( "tmp3", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "tmp4", 4, 0, vSubArguments, ss );
    ss<<"    if(tmp0==0.0)\n";
    ss<<"        return -(tmp2+tmp3)/tmp1;\n";
    ss<<"    tmp-=tmp3;\n";
    ss<<"    tmp=tmp-tmp2*pow(1.0+tmp0,tmp1);\n";
    ss<<"    tmp=tmp/( (1.0+tmp0*tmp4)* ";
    ss<<"( (pow(1.0+tmp0,tmp1)-1.0)/tmp0));\n";
    ss<<"    return tmp;\n";
    ss<<"}";
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
    //while (i-- > 1)
    for (size_t i = 1; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken *>(pCur);
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
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }
        else
        {
            ss << "nCount += 1;\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        double temp=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        double temp1=1.0;";
            ss << "        if (isnan(temp)){\n";
            ss << "            tmp += 0;}\n";
            ss << "        else{\n";
            ss << "            for(int i=1;i<nCount;i+=2)\n";
            ss << "                temp1*=pow(1.0f+ arg0 ,2);\n";
            ss << "            if(nCount%2)\n";
            ss << "                temp1*=1.0f+ arg0;\n";
            ss << "            tmp +=temp/ temp1;\n";
            ss << "        nCount += 1;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
        {
            ss << "        double temp=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    double temp1=1.0;";
            ss << "            for(int i=1;i<nCount;i+=2)";
            ss << "                temp1*=pow(1.0f+ arg0 ,2);\n";
            ss << "            if(nCount%2)";
            ss << "                temp1*=1.0f+ arg0;\n";
            ss << "            tmp +=temp/ temp1;\n";
            ss << "        nCount += 1;\n";
        }
    }
    ss << "    return tmp;\n";
    ss << "}";
}

 void OpPrice::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
 {
    decls.insert(getPrice_new_Decl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl);
    decls.insert(DateToDaysDecl);
    decls.insert(ScaDateDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(addMonthsDecl);decls.insert(lcl_Getcoupnum_newDecl);
    decls.insert(coupnum_newDecl);
    decls.insert(DateToDays_newDecl);
    decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(coupdaybs_newDecl);
    decls.insert(lcl_Getcoupdays_newDecl);
    decls.insert(lcl_Getcoupdaybs_newDecl);
    decls.insert(coupdays_newDecl);
    decls.insert(coupdaysnc_newDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth_new);
    funs.insert(DaysToDate);funs.insert(DateToDays_new);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs_new);
    funs.insert(coupdaybs_new);
    funs.insert(lcl_Getcoupdays_new);
    funs.insert(coupdaysnc_new);
    funs.insert(coupdays_new);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupnum_new);
    funs.insert(coupnum_new);funs.insert(getPrice_new);
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
    ss << "    tmp = getPrice_(tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);\n";
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
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    double tmp = 0;\n";
    ss <<"    int gid0 = get_global_id(0);\n";
    ss <<"    double tmp0=0;\n";
    ss <<"    double tmp1=0;\n";
    ss <<"    double tmp2=0;\n";
    ss <<"    double tmp3=0;\n";
    ss <<"    double tmp4=0;\n";
    ss <<"    double tmp5=0;\n";
    ss <<"    double tmp6=0;\n";
    ss <<"    double tmp7=0;\n";
    ss <<"    \n";
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
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            tmp"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            tmp"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss <<"    }\n";
        }
        else
        {
            ss << "        tmp"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
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
    CHECK_PARAMETER_COUNT( 8, 8 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    double tmp = 0;\n";
    ss <<"    int gid0 = get_global_id(0);\n";
    ss <<"    double tmp0=0;\n";
    ss <<"    double tmp1=0;\n";
    ss <<"    double tmp2=0;\n";
    ss <<"    double tmp3=0;\n";
    ss <<"    double tmp4=0;\n";
    ss <<"    double tmp5=0;\n";
    ss <<"    double tmp6=0;\n";
    ss <<"    double tmp7=0;\n";
    ss <<"    \n";
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
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            tmp"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            tmp"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss <<"    }\n";
        }
        else
        {
            ss << "        tmp"<<i<<"=";
            ss <<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
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
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArgWithDefault( "tmp3", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "tmp4", 4, 0, vSubArguments, ss );
    ss <<"    if (tmp0 == 0.0)\n";
    ss <<"        tmp=(-1*(tmp2 + tmp3)/tmp1);\n";
    ss <<"    else if (tmp4 > 0.0)\n";
    ss <<"        tmp=log(-1*(tmp0*tmp3-tmp1*(1.0+tmp0))";
    ss <<"/(tmp0*tmp2+tmp1*(1.0+tmp0)))/log(1.0+tmp0);\n";
    ss <<"    else\n";
    ss <<"        tmp=log(-1*(tmp0*tmp3-tmp1)/(tmp0*tmp2+tmp1))";
    ss <<"/log(1.0+tmp0);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
 }

void OpPPMT::BinInlineFun(std::set<std::string>& decls,
        std::set<std::string>& funs)
{
    decls.insert(GetFVDecl);
    funs.insert(GetFV);
}

void OpPPMT::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 4, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    ss<<"    double arg=0;\n";
    GenerateArg( "tmp0", 0, vSubArguments, ss );
    GenerateArg( "tmp1", 1, vSubArguments, ss );
    GenerateArg( "tmp2", 2, vSubArguments, ss );
    GenerateArg( "tmp3", 3, vSubArguments, ss );
    GenerateArgWithDefault( "tmp4", 4, 0, vSubArguments, ss );
    GenerateArgWithDefault( "tmp5", 5, 0, vSubArguments, ss );
    ss<<"    double pmt=0 ;\n";
    ss<<"    if(tmp0==0.0)\n";
    ss<<"        return -(tmp3+tmp4)/tmp2;\n";
    ss<<"    pmt=pmt-tmp4-tmp3*pow(1.0+tmp0,tmp2);\n";
    ss<<"    pmt=pmt/( (1.0+tmp0*tmp5)* ";
    ss<<"( (pow(1.0+tmp0,tmp2)-1.0)/tmp0));\n";
    ss<<"    double temp = pow( 1+tmp0,tmp1-2);\n";
    ss<<"    double re;\n";
    ss<<"    if(tmp1==1.0){\n";
    ss<<"        if(tmp5>0.0)\n";
    ss<<"            re=0.0;\n";
    ss<<"        else\n";
    ss<<"            re=-tmp3;\n";
    ss<<"    }\n";
    ss<<"    else\n";
    ss<<"    {\n";
    ss<<"        if(tmp5>0.0)\n    ";
    ss<<"            re=GetFV(tmp0, tmp1-2.0, pmt, tmp3, 1.0) - pmt;\n";
    ss<<"        else\n";
    ss<<"            re=GetFV(tmp0, tmp1-1.0, pmt, tmp3, 0.0);\n";
    ss<<"    }\n    ";
    ss<<"    re = re * tmp0;\n";
    ss<<"    tmp = pmt - re;\n";
    ss<<"    return tmp;\n";
    ss<<"}";
}

void OpCoupdaybs::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDays_newDecl);
    decls.insert(GetNullDate_newDecl); decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_Getcoupdaybs_newDecl);
    decls.insert(coupdaybs_newDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays_new);
    funs.insert(GetNullDate_new);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs_new);
    funs.insert(coupdaybs_new);
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
    ss <<"    tmp = coupdaybs_new(nSettle,nMat,nFreq,nBase);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpCoupdays::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDays_newDecl);
    decls.insert(GetNullDate_newDecl); decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(lcl_Getcoupdays_newDecl);
    decls.insert(coupdays_newDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays_new);
    funs.insert(GetNullDate_new);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(lcl_Getcoupdays_new);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(coupdays_new);
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
    ss <<"    tmp = coupdays_new(nSettle,nMat,nFreq,nBase);\n";
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
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonth_newDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(DateToDays_newDecl);
    decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(setDayDecl);decls.insert(checklessthanDecl);
    decls.insert(coupdaybs_newDecl);
    decls.insert(lcl_Getcoupdays_newDecl);
    decls.insert(lcl_Getcoupdaybs_newDecl);
    decls.insert(coupdays_newDecl);
    decls.insert(coupdaysnc_newDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth_new);
    funs.insert(DaysToDate);funs.insert(DateToDays_new);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(lcl_Getcoupdaybs_new);
    funs.insert(coupdaybs_new);
    funs.insert(lcl_Getcoupdays_new);
    funs.insert(coupdaysnc_new);
    funs.insert(coupdays_new);
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
    ss <<"    tmp = coupdaysnc_new(nSettle,nMat,nFreq,nBase);\n";
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
    decls.insert(addMonthsDecl);decls.insert(lcl_Getcoupnum_newDecl);
    decls.insert(coupnum_newDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth_new);
    funs.insert(DaysToDate);
    funs.insert(DateToDays);
    funs.insert(ScaDate);
    funs.insert(setDay);funs.insert(checklessthan);
    funs.insert(addMonths);funs.insert(lcl_Getcoupnum_new);
    funs.insert(coupnum_new);
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
    ss <<"    tmp = coupnum_new(nSettle,nMat,nFreq,nBase);\n";
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

void RATE::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(nCorrValDecl);
    decls.insert(SCdEpsilonDecl);decls.insert(RoundDecl);
    funs.insert(Round);
}

void RATE::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 6, 6 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "arg0", 0, vSubArguments, ss );
    GenerateArg( "arg1", 1, vSubArguments, ss );
    GenerateArg( "arg2", 2, vSubArguments, ss );
    GenerateArgWithDefault( "arg3", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg4", 4, 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg5", 5, 0.1, vSubArguments, ss );
    ss << "    double result;\n";
    ss << "    bool bValid = true, bFound = false;\n";
    ss << "    double fX, fXnew, fTerm, fTermDerivation;\n";
    ss << "    double fGeoSeries, fGeoSeriesDerivation;\n";
    ss << "    int nIterationsMax = 150;\n";
    ss << "    int nCount = 0;\n";
    ss << "    double fEpsilonSmall = 1.0E-14;\n";
    ss << "    if( arg0 <= 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    arg3 = arg3 - arg1 * arg4;\n";
    ss << "    arg2 = arg2 + arg1 * arg4;\n";
    ss << "    if (arg0 == Round(arg0)){\n";
    ss << "        fX = arg5;\n";
    ss << "        double fPowN, fPowNminus1;\n";
    ss << "        while (!bFound && nCount < nIterationsMax)\n";
    ss << "        {\n";
    ss << "            fPowNminus1 = pow( 1.0+fX, arg0-1.0);\n";
    ss << "            fPowN = fPowNminus1 * (1.0+fX);\n";
    ss << "            if (fX == 0.0)\n";
    ss << "            {\n";
    ss << "                fGeoSeries = arg0;\n";
    ss << "                fGeoSeriesDerivation = arg0 * (arg0-1.0) / 2.0;\n";
    ss << "            }\n";
    ss << "            else\n";
    ss << "            {";
    ss << "                fGeoSeries = (fPowN-1.0)/fX;\n";
    ss << "                fGeoSeriesDerivation =";
    ss << " arg0 * fPowNminus1 / fX - fGeoSeries / fX;\n";
    ss << "            }\n";
    ss << "            fTerm = arg3 + arg2 *fPowN+ arg1 * fGeoSeries;\n";
    ss << "            fTermDerivation = arg2 * arg0 * fPowNminus1 +";
    ss << "arg1 * fGeoSeriesDerivation;\n";
    ss << "            if (fabs(fTerm) < fEpsilonSmall)\n";
    ss << "                bFound = true;\n";
    ss << "            else\n";
    ss << "            {\n";
    ss << "                if (fTermDerivation == 0.0)\n";
    ss << "                    fXnew = fX + 1.1 * SCdEpsilon;\n";
    ss << "                else\n";
    ss << "                    fXnew = fX - fTerm / fTermDerivation;\n";
    ss << "                nCount++;\n";
    ss << "                bFound = (fabs(fXnew - fX) < SCdEpsilon);\n";
    ss << "                fX = fXnew;\n";
    ss << "            }\n";
    ss << "        }\n";
    ss << "    }\n";
    ss << "    else\n";
    ss << "    {";
    ss << "        fX = (arg5 < -1.0) ? -1.0 : arg5;\n";
    ss << "        while (bValid && !bFound && nCount < nIterationsMax)\n";
    ss << "        {\n";
    ss << "            if (fX == 0.0){\n";
    ss << "                fGeoSeries = arg0;\n";
    ss << "                fGeoSeriesDerivation = arg0 * ";
    ss << "(arg0-1.0) / 2.0;\n";
    ss << "            }else{\n";
    ss << "                fGeoSeries = (pow( 1.0+fX, arg0) - 1.0) / fX;\n";
    ss << "                fGeoSeriesDerivation =";
    ss << " arg0 * pow(1.0+fX,arg0-1.0) / fX";
    ss << " - fGeoSeries / fX;\n";
    ss << "            }\n";
    ss << "            fTerm = arg3 + arg2 *pow(1.0+fX, arg0)";
    ss << "+ arg1 * fGeoSeries;\n";
    ss << "            fTermDerivation =";
    ss << "arg2*arg0*pow(1.0+fX,arg0-1.0)";
    ss << "+arg1*fGeoSeriesDerivation;\n";
    ss << "            if (fabs(fTerm) < fEpsilonSmall)\n";
    ss << "                bFound = true;\n";
    ss << "            else{\n";
    ss << "                if (fTermDerivation == 0.0)\n";
    ss << "                    fXnew = fX + 1.1 * SCdEpsilon;\n";
    ss << "                else\n";
    ss << "                    fXnew = fX - fTerm / fTermDerivation;\n";
    ss << "                nCount++;\n";
    ss << "                bFound = (fabs(fXnew - fX) < SCdEpsilon);\n";
    ss << "                fX = fXnew;\n";
    ss << "                bValid = (fX >= -1.0);\n";
    ss << "            }\n";
    ss << "        }\n";
    ss << "    }\n";
    ss << "    if (bValid && bFound)\n";
    ss << "        result = fX;\n";
    ss << "    else\n";
    ss << "        return CreateDoubleError(NoConvergence);\n";
    ss << "    return result;\n";
    ss << "}";
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
    ss << "    double fRate, fOldValue, fNewValue;\n";
    ss <<"    fRate = fFactor / fLife;\n";
    ss <<"    if (fRate >= 1.0)\n";
    ss <<"    {\n";
    ss <<"        fRate = 1.0;\n";
    ss <<"        if (fPeriod == 1.0)\n";
    ss <<"            fOldValue = fCost;\n";
    ss <<"        else\n";
    ss <<"            fOldValue = 0.0;\n";
    ss <<"    }\n";
    ss <<"    else\n";
    ss <<"        fOldValue = fCost * pow(1.0 - fRate, fPeriod - 1);\n";
    ss <<"    fNewValue = fCost * pow(1.0 - fRate, fPeriod);\n";
    ss <<"    if (fNewValue < fSalvage)\n";
    ss <<"        tmp = fOldValue - fSalvage;\n";
    ss <<"    else\n";
    ss <<"        tmp = fOldValue - fNewValue;\n";
    ss <<"    if (tmp < 0.0)\n";
    ss <<"        tmp = 0.0;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}

void OpPV::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 3, 5 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double result = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "rate", 0, vSubArguments, ss );
    GenerateArg( "nper", 1, vSubArguments, ss );
    GenerateArg( "pmt", 2, vSubArguments, ss );
    GenerateArgWithDefault( "fv", 3, 0, vSubArguments, ss );
    GenerateArgWithDefault( "type", 4, 0, vSubArguments, ss );
    ss << "    if(rate == 0)\n";
    ss << "        result=fv+pmt*nper;\n";
    ss << "    else if(type > 0)\n";
    ss << "        result=(fv*pow(1+rate,-nper))+";
    ss << "(pmt*(1-pow(1+rate,-nper+1))/rate)+pmt;\n";
    ss << "    else\n";
    ss << "        result=(fv*pow(1+rate,-nper))+";
    ss << "(pmt*(1-pow(1+rate,-nper))/rate);\n";
    ss << "    return -result;\n";
    ss << "}";
}

void OpVDB::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(ScGetDDBDecl);decls.insert(DblMinDecl);
    decls.insert(ScInterVDBDecl);decls.insert(VDBImplementDecl);
    funs.insert(ScGetDDB);funs.insert(DblMin);
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
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    if(vSubArguments.size() <= 6)
    {
        ss << "    int tmp6  = 0;\n";
    }
    if(vSubArguments.size() == 5)
    {
        ss << "    double tmp5= 2.0;\n";
    }
    ss << "    if(tmp3 < 0 || tmp4<tmp3 || tmp4>tmp2 || tmp0<0 ||tmp1>tmp0";
    ss << "|| tmp5 <=0)\n";
    ss << "        result = -DBL_MAX;\n";
    ss << "    else\n";
    ss << "        result =";
    ss << "VDBImplement(tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);\n";
    ss << "    return result;\n";
    ss << "}";
}

void OpXirr::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int doubleIndex = gid0;\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    ss << "    int i=0;\n";
    GenTmpVariables(ss,vSubArguments);
    if(vSubArguments.size() == 2)
    {
        ss << "    double tmp2  = 0.1;\n";
    }
    else
    {
        CheckSubArgumentIsNan(ss,vSubArguments,2);
    }
    ss << "    if(tmp2<=-1)\n";
    ss << "        result = -DBL_MAX;\n";
    ss << "    else\n";
    ss << "    {\n";
    ss << "        double fMaxEps = 1e-10;\n";
    ss << "        int nMaxIter = 50;\n";
    ss << "        double fNewRate, fRateEps, fResultValue, fResultValue2;\n";
    ss << "        int nIter = 0;\n";
    ss << "        int bContLoop;\n";
    ss << "        int windowsSize = ";
    ss << nCurWindowSize;
    ss << ";\n";
    CheckSubArgumentIsNan(ss,vSubArguments,0);
    CheckSubArgumentIsNan(ss,vSubArguments,1);
    ss << "        double D_0 = tmp1;\n";
    ss << "        double V_0 = tmp0;\n";
    ss << "        double fResultRate = tmp2;\n";
    ss << "        double r;\n";
    ss << "        double fResult;\n";
    ss << "        do\n";
    ss << "        {\n";
    ss << "            fResultValue = V_0;\n";
    ss << "            r = fResultRate + 1;\n";
    ss << "            for (i = ";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
       ss << "gid0+1; i < "<< nCurWindowSize <<"; i++)\n";
    } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
       ss << "1; i < gid0+"<< nCurWindowSize <<"; i++)\n";
    } else {
       ss << "1; i < "<< nCurWindowSize <<"; i++)\n";
    }
    ss << "            {\n";
    if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
       ss<< "                doubleIndex =i+gid0;\n";
    }else
    {
       ss<< "                doubleIndex =i;\n";
    }
    CheckSubArgumentIsNan(ss,vSubArguments,0);
    CheckSubArgumentIsNan(ss,vSubArguments,1);
    ss << "                fResultValue += tmp0/pow(r,(tmp1 - D_0)/365.0);\n";
    ss << "            }\n";
    ss << "            fResultValue2 = 0;\n";

    ss << "            for (i = ";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
       ss << "gid0+1; i < "<< nCurWindowSize <<"; i++)\n";
    } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
       ss << "1; i < gid0+"<< nCurWindowSize <<"; i++)\n";
    } else {
       ss << "1; i < "<< nCurWindowSize <<"; i++)\n";
    }
    ss << "            {\n";
    if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
       ss<< "                doubleIndex =i+gid0;\n";
    }else
    {
       ss<< "                doubleIndex =i;\n";
    }
    CheckSubArgumentIsNan(ss,vSubArguments,0);
    CheckSubArgumentIsNan(ss,vSubArguments,1);
    ss << "                double E_i = (tmp1 - D_0)/365.0;\n";
    ss << "                fResultValue2 -= E_i * tmp0 / pow(r,E_i + 1.0);\n";
    ss << "            }\n";
    ss << "            fNewRate = fResultRate - fResultValue / fResultValue2;\n";
    ss << "            fRateEps = fabs( fNewRate - fResultRate );\n";
    ss << "            fResultRate = fNewRate;\n";
    ss << "            bContLoop = (fRateEps > fMaxEps) && (fabs( fResultValue ) > fMaxEps);\n";
    ss << "        }\n";
    ss << "        while( bContLoop && (++nIter < nMaxIter) );\n";
    ss << "        if( bContLoop )\n";
    ss << "            result = -DBL_MAX;\n";
    ss << "        result = fResultRate;\n";
    ss << "    }\n";
    ss << "    return result;\n";
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
