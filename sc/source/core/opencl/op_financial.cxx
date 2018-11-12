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

namespace sc { namespace opencl {
// Definitions of inline functions
#include "opinlinefun_finacial.cxx"

void RRI::GenSlidingWindowFunction(
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
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fv;\n";
    ss << "    double pv;\n";
    ss << "    double nper;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);

    ss<< "    int buffer_nper_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_pv_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_fv_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";

    ss<<"    if(gid0>=buffer_nper_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        nper = 0;\n\telse \n";
    ss<<"        nper = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n";

    ss<<"    if(gid0>=buffer_pv_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        pv = 0;\n\telse \n";
    ss<<"        pv = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n";

    ss<<"    if(gid0>=buffer_pv_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        fv = 0;\n\telse \n";
    ss<<"        fv = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n";
    ss << "    tmp = pow(fv*pow(pv,-1),1.0*pow(nper,-1))-1;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpNominal::GenSlidingWindowFunction(
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
    ss << ") {\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "double temp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp0=0,tmp1=0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
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
            ss <<"    temp="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
            ss <<"    if (isnan(temp))\n";
            ss <<"        tmp"<<i<<"= 0;\n";
            ss <<"    else\n";
            ss <<"        tmp"<<i<<"=temp;\n";
            ss <<"    }\n";
        }
        else
        {
            ss <<"    tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef(
);
            ss <<";\n";
        }
    }
    ss<<"if(tmp1==0)\n\t";
    ss<<"\treturn 0;\n\t";
    ss<<"tmp=pow( tmp1,-1);\n\t";
    ss<<"tmp=( pow( tmp0+ 1.0, tmp ) - 1.0 ) *";
    ss<<"tmp1;\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpDollarde::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    ss << "double dollar;\n\t";
    ss << "double fFrac;\n\t";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss<< "int buffer_dollar_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_frac_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<<"if((gid0)>=buffer_dollar_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dollar = 0;\n\telse \n\t\t";
    ss<<"dollar = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if((gid0)>=buffer_frac_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fFrac = 0;\n\telse \n\t\t";
    ss<<"fFrac = (int)(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<");\n\t";
    ss << "tmp = modf( dollar , &fInt );\n\t";
    ss << "tmp /= fFrac;\n\t";
    ss << "tmp *= pow( 10.0 , ceil( log10(fFrac ) ) );\n\t";
    ss << "tmp += fInt;\t";
    ss << "\n\treturn tmp;\n";
    ss << "}";
}

void OpDollarfr::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ") {\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    ss << "double dollar;\n\t";
    ss << "double fFrac;\n\t";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss<< "int buffer_dollar_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_frac_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<<"if((gid0)>=buffer_dollar_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dollar = 0;\n\telse \n\t\t";
    ss<<"dollar = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if((gid0)>=buffer_frac_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fFrac = 0;\n\telse \n\t\t";
    ss<<"fFrac = (int)(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<");\n\t";
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

void OpDISC::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
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

void OpINTRATE::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
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
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
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

void OpFV::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    unsigned j = vSubArguments.size();
    while (j--)
        {
        FormulaToken* pCur = vSubArguments[j]->GetFormulaToken();
        assert(pCur);
        if(pCur->GetType() == formula::svSingleVectorRef)
            {
            const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isnan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
            ss << "        arg" << j << " = ";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << ";\n";
            }
        }
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

void OpIPMT::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    ss << "    double arg5 = " << GetBottom() << ";\n";
    unsigned j = vSubArguments.size();
    while (j--)
        {
        FormulaToken* pCur = vSubArguments[j]->GetFormulaToken();
        assert(pCur);
        if(pCur->GetType() == formula::svSingleVectorRef)
            {
            const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isnan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
            ss << "        arg" << j << " = ";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << ";\n";
            }
        }
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
void OpISPMT::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
    ss << "    tmp = arg3 * arg0 * ( arg1 - arg2) * pow(arg2, -1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpPDuration::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
    ss << "    tmp = log(arg2 * pow( arg1,-1)) / log(arg0 + 1.0);\n";
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

void OpDuration_ADD::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    ss << "    double arg5 = " << GetBottom() << ";\n";
    unsigned j = vSubArguments.size();
    while (j--)
    {
        FormulaToken* pCur = vSubArguments[j]->GetFormulaToken();
        assert(pCur);
        if(pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isnan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
            ss << "        arg" << j << " = ";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << ";\n";
            }
        }
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

void OpMDuration::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    ss << "    double arg2 = " << GetBottom() << ";\n";
    ss << "    double arg3 = " << GetBottom() << ";\n";
    ss << "    double arg4 = " << GetBottom() << ";\n";
    ss << "    double arg5 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
    ss << "    int nNullDate = 693594;\n";
    ss << "    tmp = GetDuration_new( nNullDate, (int)arg0, (int)arg1, arg2,";
    ss << " arg3, (int)arg4, (int)arg5);\n";
    ss << "    tmp = tmp * pow(1.0 + arg3 * pow((int)arg4, -1.0), -1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void Fvschedule::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    FormulaToken* pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    if(vSubArguments[0]->GetFormulaToken()->GetType() != formula::svDoubleVectorRef)
        throw Unhandled( __FILE__, __LINE__ );
    const formula::DoubleVectorRefToken* pCurDVR =
        static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
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
    ss << "    double fRate,fVal;\n";
    ss << "    int nStartPer,nEndPer,nNumPeriods,nPayType;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
        ss <<"    if(gid0 >= "<<tmpCurDVR0->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fRate = 0;\n    else\n";
    }
    ss <<"        fRate = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
        ss <<"    if(gid0 >= "<<tmpCurDVR1->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nNumPeriods = 0;\n    else\n";
    }
    ss <<"        nNumPeriods = (int)";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
        ss <<"    if(gid0 >= "<<tmpCurDVR2->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fVal  = 0;\n    else\n";
    }
    ss <<"        fVal = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
        ss <<"    if(gid0 >= "<<tmpCurDVR3->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nStartPer = 0;\n    else\n";
    }
    ss <<"        nStartPer = (int)";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur4->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
        ss <<"    if(gid0 >= "<<tmpCurDVR4->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nEndPer = 0;\n    else\n";
    }
    ss <<"        nEndPer = (int)";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";

    if(tmpCur5->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur5);
        ss <<"    if(gid0 >= "<<tmpCurDVR5->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nPayType = 0;\n    else\n";
    }
    ss <<"    nPayType = (int)"<<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n";
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

void IRR::GenSlidingWindowFunction(std::stringstream &ss,
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
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
        static_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }

    ss << ") {\n\t";
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double result=0;\n\t";
    ss<< "int nNullDate = GetNullDate( );\n\t";
    ss <<"int settle;\n\t";
    ss <<"int mat;\n\t";
    ss <<"int issue;\n\t";
    ss <<"double rate;\n\t";
    ss <<"double yield;\n\t";
    ss <<"int  nBase;\n\t";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);

    ss<< "int buffer_settle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_mat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_issue_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_rate_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_yield_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_base_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n\t";
    ss<<"if(gid0>=buffer_settle_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"settle = 0;\n\telse \n\t\t";
    ss<<"settle = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(gid0>=buffer_mat_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"mat = 0;\n\telse \n\t\t";
    ss<<"mat = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(gid0>=buffer_issue_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"issue = 0;\n\telse \n\t\t";
    ss<<"issue = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(gid0>=buffer_rate_len || isnan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"rate = 0;\n\telse \n\t\t";
    ss<<"rate = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(gid0>=buffer_yield_len || isnan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"yield = 0;\n\telse \n\t\t";
    ss<<"yield = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss<<"if(gid0>=buffer_base_len || isnan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"nBase = 0;\n\telse \n\t\t";
    ss<<"nBase = ";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
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

void OpSYD::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double result=0;\n";
    ss << "    double cost;\n";
    ss << "    double salvage;\n";
    ss << "    double life;\n";
    ss << "    double period;\n";
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

    ss << "    int buffer_cost_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";

    ss << "    int buffer_salvage_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n";

    ss << "    int buffer_life_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss << "    int buffer_period_len = ";
    ss << tmpCurDVR3->GetArrayLength();
    ss << ";\n";

    ss <<"    if(gid0>=buffer_cost_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        cost = 0;\n\telse \n";
    ss <<"        cost = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0>=buffer_salvage_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        salvage = 0;\n\telse \n";
    ss <<"        salvage = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0>=buffer_life_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        life = 0;\n\telse \n";
    ss <<"        life = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0>=buffer_period_len || isnan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        period = 0;\n\telse \n";
    ss <<"        period = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    double tmpvalue = ((life*(life+1))*pow(2.0,-1));\n";
    ss <<"    result = ((cost-salvage)*(life-period+1)";
    ss << "*pow(tmpvalue,-1));\n";
    ss <<"    return result;\n";
    ss <<"}\n";
}

void MIRR::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
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

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
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

void OpEffective::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
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
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n\t";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    {\n";
        }
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isnan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "            arg" << i << " = 0;\n";
            ss << "        else\n";
            ss << "            arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    arg" << i << " = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
        }
    }
    ss << "    tmp = pow(1.0 + arg0 * pow(arg1, -1), arg1)-1.0;\n";
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
    ss << "   int gid0 = get_global_id(0);\n";
    ss << "double tmp = 0;\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";

    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<<"if(gid0>=buffer_tmp000_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp001_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp002_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

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
void OpCumprinc::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double fRate,fVal;\n";
    ss << "    int nStartPer,nEndPer,nNumPeriods,nPayType;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
        ss <<"    if(gid0 >= "<<tmpCurDVR0->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fRate = 0;\n    else\n";
    }
    ss <<"        fRate = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
        ss <<"    if(gid0 >= "<<tmpCurDVR1->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nNumPeriods = 0;\n    else\n";
    }
    ss <<"        nNumPeriods = (int)";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
        ss <<"    if(gid0 >= "<<tmpCurDVR2->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fVal  = 0;\n    else\n";
    }
    ss <<"        fVal = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
        ss <<"    if(gid0 >= "<<tmpCurDVR3->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nStartPer = 0;\n    else\n";
    }
    ss <<"        nStartPer = (int)";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur4->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
        ss <<"    if(gid0 >= "<<tmpCurDVR4->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nEndPer = 0;\n    else\n";
    }
    ss <<"        nEndPer = (int)";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";

    if(tmpCur5->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur5);
        ss <<"    if(gid0 >= "<<tmpCurDVR5->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        nPayType = 0;\n    else\n";
    }
    ss <<"        nPayType = (int)";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n";
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
    ss << "    int nStartDate,nEndDate,mode,freq;\n";
    ss << "    int nDays1stYear=0;\n";
    ss << "    double fVal,fRate;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken* tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur5);
    FormulaToken* tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur6);
    ss<< "    int buffer_nIssue_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_nSettle_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_fRate_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_fVal_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_nFreq_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_nMode_len = ";
    ss<< tmpCurDVR6->GetArrayLength();
    ss << ";\n";
    ss<<"    if(gid0 >= buffer_nIssue_len || isnan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nStartDate = 0;\n    else\n";
    ss <<"        nStartDate=(int)";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_nSettle_len || isnan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nEndDate = 0;\n    else\n";
    ss <<"        nEndDate=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss <<"    if(gid0 >= buffer_fRate_len || isnan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRate = 0;\n    else\n";
    ss <<"        fRate=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_fVal_len || isnan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fVal = 0;\n    else\n";
    ss <<"        fVal=";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_nFreq_len || isnan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        freq = 0;\n    else\n";
    ss <<"        freq= (int)";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_nMode_len || isnan(";
    ss <<vSubArguments[6]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        mode = 0;\n    else\n";
    ss <<"        mode = (int)";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss <<";\n";
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int nStartDate,nEndDate,mode;\n\t";
    ss << "double fRate,fVal;\n\t";
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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "int buffer_nIssue_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_nSettle_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_fRate_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_fVal_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_nMode_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
    ss <<"if(gid0 >= buffer_nIssue_len || isnan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nStartDate = 0;\n\telse\n\t\t";
    ss << "nStartDate=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss <<"if(gid0 >= buffer_nSettle_len || isnan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nEndDate = 0;\n\telse\n\t\t";
    ss << "nEndDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";

    ss <<"if(gid0 >= buffer_fRate_len || isnan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fRate = 0;\n\telse\n\t\t";
    ss << "fRate=";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss <<"if(gid0 >= buffer_fVal_len || isnan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fVal = 0;\n\telse\n\t\t";
    ss << "fVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss <<"if(gid0 >= buffer_nMode_len || isnan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"mode = 0;\n\telse\n\t\t";
    ss << "mode = (int)";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
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
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";
    ss << "double tmp005;\n\t";
    ss << "double tmp006;\n\t";

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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);

    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);

    FormulaToken *tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur6);

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp003_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp004_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp005_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp006_len = ";
    ss<< tmpCurDVR6->GetArrayLength();
    ss << ";\n\t";

    ss<<"if(gid0>=buffer_tmp000_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp001_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp002_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp003_len || isnan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp004_len || isnan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp005_len || isnan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp005 = 0;\n\telse \n\t\t";
    ss<<"tmp005 = ";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp006_len || isnan(";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp006 = 0;\n\telse \n\t\t";
    ss<<"tmp006 = ";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss << "tmp = getYield_(";
    ss << "GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005,tmp006);\n\t ";
    ss << "return tmp;\n";
    ss << "}";
}

void OpSLN::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double cost;\n";
    ss << "    double salvage;\n";
    ss << "    double life;\n";

    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur2);
    ss<< "    int buffer_cost_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_salvage_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_life_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<<"    if(gid0>=buffer_cost_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        cost = 0;\n\telse \n";
    ss<<"        cost = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n";
    ss<<"    if(gid0>=buffer_salvage_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        salvage = 0;\n\telse \n";
    ss<<"        salvage = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n";
    ss<<"    if(gid0>=buffer_life_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        life = 0;\n\telse \n";
    ss<<"        life = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n";
    ss << "    tmp = (cost-salvage)*pow(life,-1);\n";
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
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";
    ss << "double tmp005;\n\t";

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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);

    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp003_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp004_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp005_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n\t";

    ss<<"if(gid0>=buffer_tmp000_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp001_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp002_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp003_len || isnan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp004_len || isnan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp005_len || isnan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp005 = 0;\n\telse \n\t\t";
    ss<<"tmp005 = ";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss << "tmp = GetYieldmat(";
    ss<<"GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpPMT::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss<<") {\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    double temp=0.0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    ss<<"    double tmp0=0,tmp1=0,tmp2=0;\n";
    ss<<"    double tmp3=0,tmp4=0;\n";
    ss <<"\n    ";
    //while (i-- > 1)
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
            ss <<"    temp="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
            ss <<"    if (isnan(temp))\n";
            ss <<"        tmp"<<i<<"= 0;\n";
            ss <<"    else\n";
            ss <<"        tmp"<<i<<"=temp;\n";
            ss <<"    }\n";
        }
        else
        {
            ss <<"    tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
    ss<<"    if(tmp0==0.0)\n";
    ss<<"        return -(tmp2+tmp3)/tmp1;\n";
    ss<<"    tmp-=tmp3;\n";
    ss<<"    tmp=tmp-tmp2*pow(1.0+tmp0,tmp1);\n";
    ss<<"    tmp=tmp*pow(( (1.0+tmp0*tmp4)* ";
    ss<<"( (pow(1.0+tmp0,tmp1)-1.0)/tmp0)),-1);\n";
    ss<<"    return tmp;\n";
    ss<<"}";
}

void OpNPV::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
      if (i)
          ss << ", ";
      vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0.0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nCount = 1;\n";
    ss << "    double arg0=";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
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
void OpPrice::GenSlidingWindowFunction(std::stringstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
      if (i)
          ss << ", ";
      vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss<<") {\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    ss<<"    double tmp0=0;\n";
    ss<<"    double tmp1=0;\n";
    ss<<"    double tmp2=0;\n";
    ss<<"    double tmp3=0;\n";
    ss<<"    double tmp4=0,tmp5=0;\n";
    ss<<"    double tmp6=0;\n";
    ss<<"\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
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
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n    }\n";
        }
        else
        {
            ss << "        tmp"<<i<<"=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
        }
    }
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
void OpOddlprice::GenSlidingWindowFunction(std::stringstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss <<") {\n";
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
void OpOddlyield::GenSlidingWindowFunction(std::stringstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss <<") {\n";
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
void OpPriceDisc::GenSlidingWindowFunction(std::stringstream &ss,
          const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss<<"    double tmp0=0;\n";
    ss<<"    double tmp1=0;\n";
    ss<<"    double tmp2=0;\n";
    ss<<"    double tmp3=0;\n";
    ss<<"    double tmp4=0;\n";
    ss <<"    \n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
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
    ss <<"    tmp=tmp3* ( 1.0 -tmp2*GetYearDiff( nNullDate, ";
    ss <<"tmp0,tmp1,tmp4));\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpNper::GenSlidingWindowFunction(std::stringstream &ss,
         const std::string &sSymName, SubArguments &vSubArguments)
 {
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
     if (i)
         ss << ", ";
     vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss <<"    double tmp0=0;\n";
    ss <<"    double tmp1=0;\n";
    ss <<"    double tmp2=0;\n";
    ss <<"    double tmp3=0;\n";
    ss <<"    double tmp4=0;\n";

    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
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
    ss <<"    if (tmp0 == 0.0)\n";
    ss <<"        tmp=(-1*(tmp2 + tmp3)/tmp1);\n";
    ss <<"    else if (tmp4 > 0.0)\n";
    ss <<"        tmp=log(-1*(tmp0*tmp3-tmp1*(1.0+tmp0))*";
    ss <<"pow((tmp0*tmp2+tmp1*(1.0+tmp0)),-1))/log(1.0+tmp0);\n";
    ss <<"    else\n";
    ss <<"        tmp=log(-1*(tmp0*tmp3-tmp1)*pow(tmp0*tmp2+tmp1,-1))";
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

void OpPPMT::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss<<") {\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    ss<<"    double arg=0;\n";
    ss<<"    double tmp0=0;\n";
    ss<<"    double tmp1=0;\n";
    ss<<"    double tmp2=0;\n";
    ss<<"    double tmp3=0;\n";
    ss<<"    double tmp4=0,tmp5=0;\n";
    ss <<"\n    ";
    //while (i-- > 1)
    for (size_t i = 0; i < vSubArguments.size(); i++)
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
            ss << "        arg=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(arg))\n";
            ss << "            tmp"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            tmp"<<i<<"=arg;\n";
            ss << "    }\n";
        }
        else
        {
            ss<<"    tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss<<";\n";
        }
    }
    ss<<"    double pmt=0 ;\n";
    ss<<"    if(tmp0==0.0)\n";
    ss<<"        return -(tmp3+tmp4)/tmp2;\n";
    ss<<"    pmt=pmt-tmp4-tmp3*pow(1.0+tmp0,tmp2);\n";
    ss<<"    pmt=pmt*pow(( (1.0+tmp0*tmp5)* ";
    ss<<"( (pow(1.0+tmp0,tmp2)-1.0)/tmp0)),-1);\n";
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
void OpCoupnum::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
    }
    ss <<"        nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nMat = 0;\n    else\n";
    }
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
    }
    ss << "        nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        nBase = 0;\n    else\n";
    }
    ss << "        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
void OpAmordegrc::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ") {\n    ";
    ss << "int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    double fCost,fRestVal,fPer,fRate;\n";
    ss << "    int nDate,nFirstPer,nBase;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    FormulaToken *tmpCur6 = vSubArguments.size() < 7 ? nullptr : vSubArguments[6]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        fCost = 0;\n    else\n";
    }
    ss << "        fCost=";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nDate = 0;\n    else\n";
    }
    ss << "        nDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFirstPer = 0;\n    else\n";
    }
    ss << "        nFirstPer=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        fRestVal = 0;\n    else\n";
    }
    ss << "        fRestVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur4->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss <<"    if(isnan(" <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR4->GetArrayLength()<<"))\n";
    ss <<"        fPer = 0;\n    else\n";
    }
    ss << "        fPer = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur5->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    ss <<"    if(isnan(" <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR5->GetArrayLength()<<"))\n";
    ss <<"        fRate = 0;\n    else\n";
    }
    ss << "        fRate=";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur6 == nullptr)
    {
        ss << "        nBase = 0;\n";
    }
    else
    {
        if(tmpCur6->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR6=
                static_cast<const formula::SingleVectorRefToken *>(tmpCur6);
            ss <<"    if(isnan(" <<vSubArguments[6]->GenSlidingWindowDeclRef();
            ss <<")||(gid0 >="<<tmpCurDVR6->GetArrayLength()<<"))\n";
            ss <<"        nBase = 0;\n    else\n";
        }
        ss << "        nBase = (int)";
        ss << vSubArguments[6]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss <<"    uint nPer = convert_int( fPer );\n";
    ss <<"    double fUsePer = 1.0 *pow( fRate,-1);\n";
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
void OpAmorlinc::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    ss << "    double fCost,fRestVal,fPer,fRate;\n";
    ss << "    int nDate,nFirstPer,nBase;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    FormulaToken *tmpCur6 = vSubArguments.size() < 7 ? nullptr : vSubArguments[6]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss <<"    if(isnan("<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR0->GetArrayLength()<<"))\n";
    ss <<"        fCost = 0;\n    else\n";
    }
    ss << "        fCost=";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss <<"    if(isnan("<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR1->GetArrayLength()<<"))\n";
    ss <<"        nDate = 0;\n    else\n";
    }
    ss << "        nDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    ss <<"    if(isnan("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR2->GetArrayLength()<<"))\n";
    ss <<"        nFirstPer = 0;\n    else\n";
    }
    ss << "        nFirstPer=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    ss <<"    if(isnan(" <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR3->GetArrayLength()<<"))\n";
    ss <<"        fRestVal = 0;\n    else\n";
    }
    ss << "        fRestVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur4->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss <<"    if(isnan(" <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR4->GetArrayLength()<<"))\n";
    ss <<"        fPer = 0;\n    else\n";
    }
    ss << "        fPer = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur5->GetType() == formula::svSingleVectorRef)
    {
    const formula::SingleVectorRefToken*tmpCurDVR5= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    ss <<"    if(isnan(" <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<")||(gid0 >="<<tmpCurDVR5->GetArrayLength()<<"))\n";
    ss <<"        fRate = 0;\n    else\n";
    }
    ss << "        fRate=";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if(tmpCur6 == nullptr)
    {
        ss << "        nBase = 0;\n";
    }
    else
    {
        if(tmpCur6->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR6=
                static_cast<const formula::SingleVectorRefToken *>(tmpCur6);
            ss <<"    if(isnan(" <<vSubArguments[6]->GenSlidingWindowDeclRef();
            ss <<")||(gid0 >="<<tmpCurDVR6->GetArrayLength()<<"))\n";
            ss <<"        nBase = 0;\n    else\n";
        }
        ss << "        nBase = (int)";
        ss << vSubArguments[6]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss <<"    int  nPer = convert_int( fPer );\n";
    ss <<"    double fOneRate = fCost * fRate;\n";
    ss <<"    double fCostDelta = fCost - fRestVal;\n";
    ss <<"    double f0Rate = GetYearFrac( 693594,";
    ss <<"nDate, nFirstPer, nBase )* fRate * fCost;\n";
    ss <<"    int nNumOfFullPeriods = (int)";
    ss <<"( ( fCost - fRestVal - f0Rate) *pow(fOneRate,-1) );\n";
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

void OpReceived::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int nSettle, nMat;\n";
    ss << "    double fInvest,fDisc;\n";
    ss << "    int rOB;\n";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "    int buffer_settle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_mat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_invest_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_disc_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_rob_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
    ss <<"    if(gid0 >= buffer_settle_len || isnan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nSettle = 0;\n\telse\n";
    ss <<"        nSettle = (int)"<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_mat_len || isnan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nMat = 0;\n\telse\n";
    ss <<"        nMat = (int)";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_invest_len || isnan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fInvest = 0;\n\telse\n";
    ss <<"        fInvest = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_disc_len || isnan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fDisc = 0;\n\telse\n";
    ss <<"        fDisc = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_rob_len || isnan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        rOB = 0;\n\telse\n";
    ss <<"        rOB = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss << "    double tmpvalue = (1.0-(fDisc";
    ss <<" * GetYearDiff( GetNullDate()";
    ss <<",nSettle,nMat,rOB)));\n";
    ss << "    tmp = fInvest*pow(tmpvalue,-1);\n";
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
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(5,5);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";

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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp003_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp004_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";

    ss<<"if(gid0>=buffer_tmp000_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp001_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp002_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp003_len || isnan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp004_len || isnan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<< "if(tmp002 <= 0 || tmp003 <= 0 || tmp000 >= tmp001 )\n";
    ss<< "    return CreateDoubleError(IllegalArgument);\n";
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
    ss << ") {\n";
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

    FormulaToken* pCur = vSubArguments[5]->GetFormulaToken();
    assert(pCur);
    const formula::SingleVectorRefToken* pSVR =
        static_cast< const formula::SingleVectorRefToken* >(pCur);
    assert(pSVR);
    ss << ") {\n";
    ss << "    double result;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    bool bValid = true, bFound = false;\n";
    ss << "    double fX, fXnew, fTerm, fTermDerivation;\n";
    ss << "    double fGeoSeries, fGeoSeriesDerivation;\n";
    ss << "    int nIterationsMax = 150;\n";
    ss << "    int nCount = 0;\n";
    ss << "    double fEpsilonSmall = 1.0E-14;\n";
    ss << "    double arg0, arg1, arg2, arg3, arg4, arg5;\n";
    ss << "    arg0=" << vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    arg1=" << vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    arg2=" << vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    arg3=" << vSubArguments[3]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    arg4=" << vSubArguments[4]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    arg5=" << vSubArguments[5]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    int guessLen = " << pSVR->GetArrayLength() << ";\n";
    ss << "    if (isnan(arg0) || isnan(arg1) || isnan(arg2)){\n";
    ss << "        result = 523;\n";
    ss << "        return result;\n";
    ss << "    }\n";
    ss << "    if (isnan(arg3))\n";
    ss << "        arg3 = 0.0;\n";
    ss << "    if (isnan(arg4))\n";
    ss << "        arg4 = 0.0;\n";
    ss << "    if (isnan(arg5))\n";
    ss << "        arg5 = 0.1;\n";
    ss << "    if (gid0 >= guessLen)\n";
    ss << "        arg5 = 0.1;\n";
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
    ss << "                fGeoSeriesDerivation = arg0 * (arg0-1.0)";
    ss << "*pow(2.0,-1);\n";
    ss << "            }\n";
    ss << "            else\n";
    ss << "            {";
    ss << "                fGeoSeries = (fPowN-1.0)*pow(fX,-1);\n";
    ss << "                fGeoSeriesDerivation =";
    ss << " arg0 * fPowNminus1 * pow( fX , -1) - fGeoSeries * pow(fX, -1);\n";
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
    ss << "                    fXnew = fX - fTerm ";
    ss << "*pow( fTermDerivation,-1);\n";
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
    ss << "(arg0-1.0)* pow(2.0,-1);\n";
    ss << "            }else{\n";
    ss << "                fGeoSeries = (pow( 1.0+fX, arg0) - 1.0)";
    ss << " *pow( fX,-1);\n";
    ss << "                fGeoSeriesDerivation =";
    ss << " arg0 * pow(1.0+fX,arg0-1.0) *pow(fX,-1)";
    ss << " - fGeoSeries *pow( fX,-1);\n";
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
    ss << "                    fXnew = fX - fTerm ";
    ss << "*pow( fTermDerivation,-1);\n";
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
    ss << "        result = 523;\n";
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";

    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";

    ss<<"if(gid0>=buffer_tmp000_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp001_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"if(gid0>=buffer_tmp002_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss <<"    int nDiff=GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
    ss <<"    nDiff++;\n";
    ss <<"    tmp=100.0;\n";
    ss <<"    tmp = tmp *pow( tmp002,-1);\n";
    ss <<"    tmp = tmp - 1.0;\n";
    ss <<"    tmp = tmp * pow( nDiff,-1.0 );\n";
    ss <<"    tmp = tmp * 360.0;\n";
    ss <<"    return tmp;\n";
    ss << "}\n";
}
void OpDDB::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double tmp = 0;\n";
    ss << "    double fCost, fSalvage, fLife, fPeriod, fFactor;\n";
    ss << "    double fRate, fOldValue, fNewValue;\n";

    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
        ss <<"    if(gid0 >= "<<tmpCurDVR0->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fCost = 0;\n    else\n";
    }
    ss <<"        fCost = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur1->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);
        ss <<"    if(gid0 >= "<<tmpCurDVR1->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fSalvage = 0;\n    else\n";
    }
    ss <<"        fSalvage = ";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur2->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
        ss <<"    if(gid0 >= "<<tmpCurDVR2->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fLife = 0;\n    else\n";
    }
    ss <<"        fLife = ";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur3->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
        ss <<"    if(gid0 >= "<<tmpCurDVR3->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fPeriod = 0;\n    else\n";
    }
    ss <<"        fPeriod = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    if(tmpCur4->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
        ss <<"    if(gid0 >= "<<tmpCurDVR4->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        fFactor = 0;\n    else\n";
    }
    ss <<"        fFactor = "<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    fRate = fFactor * pow(fLife,-1);\n";
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
    ss << "    double result = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double rate;\n";
    ss << "    double nper;\n";
    ss << "    double pmt;\n";
    ss << "    double fv;\n";
    ss << "    double type;\n";

    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);

    if(vSubArguments.size()>3)
    {
        FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
        const formula::SingleVectorRefToken* tmpCurDVR3= static_cast<const formula::SingleVectorRefToken *>(
tmpCur3);
        ss<< "    int buffer_fv_len = ";
        ss<< tmpCurDVR3->GetArrayLength();
        ss << ";\n";
        }

    if(vSubArguments.size()>4)
    {
        FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
        const formula::SingleVectorRefToken* tmpCurDVR4= static_cast<const formula::SingleVectorRefToken *>(
tmpCur4);
        ss<< "    int buffer_type_len = ";
        ss<< tmpCurDVR4->GetArrayLength();
        ss << ";\n";
    }

    ss<< "    int buffer_rate_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_nper_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_pmt_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";

    ss<<"    if(gid0>=buffer_rate_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        rate = 0;\n    else \n";
    ss<<"        rate = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n";

    ss<<"    if(gid0>=buffer_nper_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        nper = 0;\n    else \n";
    ss<<"        nper = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n";

    ss<<"    if(gid0>=buffer_pmt_len || isnan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        pmt = 0;\n    else \n";
    ss<<"        pmt = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n";

    if(vSubArguments.size()>3)
    {
        ss<<"    if(gid0>=buffer_fv_len || isnan(";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"))\n";
        ss<<"        fv = 0;\n    else \n";
        ss<<"        fv = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<";\n";
    }else
    {
        ss<<"    fv = 0;\n";
    }

    if(vSubArguments.size()>4)
    {
        ss<<"    if(gid0>=buffer_type_len || isnan(";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<"))\n";
        ss<<"        type = 0;\n    else \n";
        ss<<"        type = ";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<";\n";
    }else
    {
        ss<<"        type = 0;\n";
    }
    ss << "    if(rate == 0)\n";
    ss << "        result=fv+pmt*nper;\n";
    ss << "    else if(type > 0)\n";
    ss << "        result=(fv*pow(1+rate,-nper))+";
    ss << "(pmt*(1-pow(1+rate,-nper+1))*pow(rate,-1))+pmt;\n";
    ss << "    else\n";
    ss << "        result=(fv*pow(1+rate,-nper))+";
    ss << "(pmt*(1-pow(1+rate,-nper))*pow(rate,-1));\n";
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
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    if(vSubArguments.size()<5)
    {
        ss << "    result = -DBL_MAX;\n";
        ss << "    return result;\n";
    }else
    {
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

}

void OpXirr::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
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
    ss << "    int doubleIndex = gid0;\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    double result = 0;\n";
    ss << "    int i=0;\n";
    if(vSubArguments.size()<2)
    {
        ss << "    result = -DBL_MAX;\n";
        ss << "    return result;\n";
    }else
    {
        GenTmpVariables(ss,vSubArguments);
        if(vSubArguments.size() == 2)
        {
            ss << "    double tmp2  = 0.1;\n";
        }else
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

}
void OpDB::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    double fCost, fSalvage, fLife, fPeriod;\n";
    ss << "    int nMonths;\n";
    ss << "    double tmp = 0;\n";
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= static_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "    int buffer_cost_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_salvage_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_life_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_period_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_months_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
    ss <<"    if(gid0 >= buffer_cost_len || isnan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fCost = 0;\n    else\n";
    ss <<"        fCost = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_salvage_len || isnan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fSalvage = 0;\n    else\n";
    ss <<"        fSalvage = ";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_life_len || isnan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fLife = 0;\n    else\n";
    ss <<"        fLife = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_period_len || isnan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fPeriod = 0;\n    else\n";
    ss <<"        fPeriod = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    if(gid0 >= buffer_months_len || isnan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nMonths = 0;\n    else\n";
    ss <<"        nMonths = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
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
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
