/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_financial.hxx"

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
// Definitions of inline functions
#include "opinlinefun_finacial.cxx"

void RRI::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fv;\n\t";
    ss << "double pv;\n\t";
    ss << "double nper;\n\t";
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

    ss<< "int buffer_nper_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_pv_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_fv_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_nper_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"nper = 0;\n\telse \n\t\t";
#endif
    ss<<"nper = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_pv_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"pv = 0;\n\telse \n\t\t";
#endif
    ss<<"pv = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_pv_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fv = 0;\n\telse \n\t\t";
#endif
    ss<<"fv = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss << "tmp = pow(fv/pv,1.0/nper)-1;\n\t;";
    ss << "return tmp;\n";
    ss << "}";
}

void OpNominal::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &
vSubArguments)
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
    ss << "double tmp = 0;\n\t";
    ss << "double temp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp0=0,tmp1=0;\n";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
                const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
        }
        else
        {
#ifdef  ISNAN
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss <<"    temp="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
            ss <<"    if (isNan(temp))\n";
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
#else
    ss <<"    tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";
#endif
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    ss << "double dollar;\n\t";
    ss << "double fFrac;\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss<< "int buffer_dollar_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_frac_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss<<"if((gid0)>=buffer_dollar_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dollar = 0;\n\telse \n\t\t";
#endif
    ss<<"dollar = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if((gid0)>=buffer_frac_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fFrac = 0;\n\telse \n\t\t";
#endif
    ss<<"fFrac = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss << "tmp = modf( dollar , &fInt );\n\t";
    ss << "tmp /= fFrac;\n\t";
    ss << "tmp *= pow( 10.0 , ceil( log10(fFrac ) ) );\n\t";
    ss << "tmp += fInt;\t";
    ss << "\n\treturn tmp;\n";
    ss << "}";
}

void OpDollarfr::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fInt = " << GetBottom() <<";\n\t";
    ss << "double dollar;\n\t";
    ss << "double fFrac;\n\t";
#ifdef  ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss<< "int buffer_dollar_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_frac_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_dollar_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dollar = 0;\n\telse \n\t\t";
#endif
    ss<<"dollar = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_frac_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"fFrac = 0;\n\telse \n\t\t";
#endif
    ss<<"fFrac = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
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
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetYearFrac);funs.insert(DaysToDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpDISC::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double settle = " << GetBottom() <<";\n\t";
    ss << "double maturity = " << GetBottom() <<";\n\t";
    ss << "double price = " << GetBottom() <<";\n\t";
    ss << "double redemp = " << GetBottom() <<";\n\t";
    ss << "int mode = " << GetBottom() <<";\n\t";
#ifdef  ISNAN
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "int buffer_settle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_maturity_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_price_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_redemp_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_mode_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_settle_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"settle = 0;\n\telse \n\t\t";
#endif
    ss<<"settle = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_maturity_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"maturity = 0;\n\telse \n\t\t";
#endif
    ss<<"maturity = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_price_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"price = 0;\n\telse \n\t\t";
#endif
    ss<<"price = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_redemp_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"redemp = 0;\n\telse \n\t\t";
#endif
    ss<<"redemp = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef  ISNAN
    ss<<"if((gid0)>=buffer_mode_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"mode = 0;\n\telse \n\t\t";
#endif
    ss<<"mode = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss << "int nNullDate = GetNullDate();\n\t";
    ss << "tmp = 1.0 - price / redemp;\n\t";
    ss << "tmp /= GetYearFrac( nNullDate , settle , maturity , mode );\n\t";
    ss << "return tmp;\n";
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

void OpINTRATE::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double settle = " << GetBottom() <<";\n\t";
    ss << "double maturity = " << GetBottom() <<";\n\t";
    ss << "double price = " << GetBottom() <<";\n\t";
    ss << "double redemp = " << GetBottom() <<";\n\t";
    ss << "int mode = " << GetBottom() <<";\n\t";
#ifdef  ISNAN
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss << "int buffer_settle_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_maturity_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_price_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_redemp_len = ";
    ss << tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_mode_len = ";
    ss << tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef  ISNAN
    ss << "if((gid0)>=buffer_settle_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "settle = 0;\n\telse \n\t\t";
#endif
    ss << "settle = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef  ISNAN
    ss << "if((gid0)>=buffer_maturity_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "maturity = 0;\n\telse \n\t\t";
#endif
    ss << "maturity = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef  ISNAN
    ss << "if((gid0)>=buffer_price_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "price = 0;\n\telse \n\t\t";
#endif
    ss << "price = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef  ISNAN
    ss << "if((gid0)>=buffer_redemp_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "redemp = 0;\n\telse \n\t\t";
#endif
    ss << "redemp = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef  ISNAN
    ss << "if((gid0)>=buffer_mode_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "mode = 0;\n\telse \n\t\t";
#endif
    ss << "mode = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "int nNullDate = GetNullDate();\n\t";
    ss << "tmp = (redemp / price - 1.0) / GetYearDiff(nNullDate, settle, maturity, mode);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}
void OpFV::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetZwDecl);
    funs.insert(GetZw);
}

void OpFV::GenSlidingWindowFunction(std::stringstream& ss,
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
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isNan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
#endif
            ss << "        arg" << j << " = ";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << ";\n";
            }
        }
    ss << "    tmp = GetZw(arg0, arg1, arg2, arg3, arg4);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpIPMT::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetZwDecl);
    funs.insert(GetZw);
}

void OpIPMT::GenSlidingWindowFunction(std::stringstream& ss,
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
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isNan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
#endif
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
    ss << "            tmp = GetZw(arg0, arg1 - 2.0, pmt, arg3, 1.0)";
    ss << " - pmt;\n";
    ss << "        else\n";
    ss << "            tmp = GetZw(arg0, arg1 - 1.0, pmt, arg3, 0.0);\n";
    ss << "    }\n";
    ss << "    tmp = tmp * arg0;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpISPMT::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "    {\n";
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
        ss << "    arg" << i;
        ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
#endif
    }
    ss << "    tmp = arg3 * arg0 * ( arg1 - arg2) * pow(arg2, -1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpDuration::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "    {\n";
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
        ss << "    arg" << i;
        ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
#endif
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
#ifdef ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isNan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
#endif
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
    decls.insert(GetDurationDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(GetYearFracDecl);decls.insert(DaysToDateDecl);
    decls.insert(GetNullDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);
    funs.insert(GetDuration);funs.insert(lcl_Getcoupnum);
    funs.insert(GetYearFrac);funs.insert(DaysToDate);
    funs.insert(GetNullDate);funs.insert(DateToDays);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpMDuration::GenSlidingWindowFunction(std::stringstream& ss,
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
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(gid0 >= " << pSVR->GetArrayLength() << " || isNan(";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << "))\n";
            ss << "        arg" << j << " = " <<GetBottom() << ";\n";
            ss << "    else\n";
#endif
            ss << "        arg" << j << " = ";
            ss << vSubArguments[j]->GenSlidingWindowDeclRef();
            ss << ";\n";
            }
        }
    ss << "    int nNullDate = GetNullDate();\n";
    ss << "    tmp = GetDuration( nNullDate, (int)arg0, (int)arg1, arg2,";
    ss << " arg3, (int)arg4, (int)arg5);\n";
    ss << "    tmp /= 1.0 + arg3 / (int)arg4;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void Fvschedule::GenSlidingWindowFunction(
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
    ss << ") {\n\t";
    ss << "double tmp = 1.0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss << "if (isNan(arg0))\n\t\t";
    ss << "arg0 = 0;\n\t";
#endif
    ss << "double arg1;\n\t";
    ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
#ifdef ISNAN
    ss << "for (int i = 0; i + gid0 < arrayLength &&";
    ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
#else
    ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t";
#endif
    ss << "arg1 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
    ss << "if (isNan(arg1))\n\t\t\t\t";
    ss << "arg1 = 0;\n\t\t\t";
#endif
    ss << "tmp *= arg1 + 1.0;\n\t\t";
    ss << "}\n\t";
    ss << "return (double)tmp * arg0";
    ss << ";\n}";
}
void Cumipmt::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetRmzDecl); decls.insert(GetZwDecl);
    funs.insert(GetRmz);funs.insert(GetZw);
}
void Cumipmt::GenSlidingWindowFunction(
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

    ss << "   int gid0 = get_global_id(0);\n";
    ss << "double fRate,fVal;\n\t";
    ss <<"int nStartPer,nEndPer,nNumPeriods,nPayType;\n\t";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    ss<< "int buffer_rate_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_NumPeriods_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_Val_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_StartPer_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_EndPer_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_PayType_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_rate_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fRate = 0;\n\telse\n\t\t";
#endif
    ss <<"fRate = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_NumPeriods_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nNumPeriods = 0;\n\telse\n\t\t";
#endif
    ss <<"nNumPeriods = (int)";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_Val_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fVal = 0;\n\telse\n\t\t";
#endif
    ss <<"fVal = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_StartPer_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nStartPer = 0;\n\telse\n\t\t";
#endif
    ss <<"nStartPer = (int)"<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_EndPer_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nEndPer = 0;\n\telse\n\t\t";
#endif
    ss <<"nEndPer = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_PayType_len || isNan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nPayType = 0;\n\telse\n\t\t";
#endif
    ss <<"nPayType = (int)"<<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss << "   double fRmz;\n";
    ss << "fRmz = GetRmz( fRate, nNumPeriods, fVal, 0.0, nPayType );\n";
    ss << "double tmp = 0.0;\n";
    ss << "uint  nStart =  nStartPer ;\n";
    ss << "uint  nEnd =  nEndPer ;\n";
    ss << " if( nStart == 1 )\n";
    ss << "    {\n";
    ss << "        if( nPayType <= 0 )\n";
    ss << "            tmp = -fVal;\n";
    ss << "        nStart++;\n";
    ss << "    }\n";
    ss << " for( uint i = nStart ; i <= nEnd ; i++ )\n";
    ss << " {\n";
    ss << "     if( nPayType > 0 )\n";
    ss << "        tmp += GetZw( fRate, convert_double( i - 2 ), ";
    ss << "fRmz, fVal, 1 ) - fRmz;\n";
    ss << "     else\n";
    ss << "        tmp += GetZw( fRate, convert_double( i - 1 ), ";
    ss << "fRmz, fVal, 0 );\n";
    ss << " }\n";
    ss << "tmp *= fRate;\n";
    ss << "return tmp;\n";
    ss << "}";
}

void IRR::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    #define  Epsilon   1.0E-7\n";
    ss << "    int gid0 = get_global_id(0);\n";
    FormulaToken* pSur = vSubArguments[1]->GetFormulaToken();
    assert(pSur);
    ss << "    double fSchaetzwert = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    double fEps = 1.0;\n";
    ss << "    double x = 0.0, xNeu = 0.0, fZaehler = 0.0, fNenner = 0.0;\n";
    ss << "    double nCount = 0.0;\n";
    if (pSur->GetType() == formula::svSingleVectorRef)
    {
#ifdef  ISNAN
        const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pSur);
        ss << "    if (gid0 >= " << pSVR->GetArrayLength() << ")\n";
        ss << "        fSchaetzwert = 0.1;\n";
        ss << "    if (isNan(fSchaetzwert))\n";
        ss << "        x = 0.1;\n";
        ss << "    else\n";
#endif
    }
    else if (pSur->GetType() == formula::svDouble)
    {
#ifdef  ISNAN
        ss << "    if (isNan(fSchaetzwert))\n";
        ss << "        x = 0.1;\n";
        ss << "    else\n";
#endif
    }
    ss << "        x = fSchaetzwert;\n";
    ss << "    unsigned short nItCount = 0;\n";
    ss << "    while (fEps > Epsilon && nItCount < 20){\n";
    ss << "        nCount = 0.0; fZaehler = 0.0;  fNenner = 0.0;\n";
    ss << "        double arg0, arg1;\n";
    ss << "        int i = 0;\n";
    FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pDVR =
        dynamic_cast<const formula::DoubleVectorRefToken* >(pCur);
    size_t nCurWindowSize = pDVR->GetRefRowSize();
    ss << "        for ( ";
    if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
        ss << "i = gid0; i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize << " /2*2; i++){\n";
#else
        ss << "i = gid0; i < " << nCurWindowSize << " /2*2; i++)\n";
#endif
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;" << ";\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg0)){\n";
#endif
        ss << "            fZaehler += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg1)){\n";
#endif
        ss << "                fZaehler += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount += 1;\n";
        ss << "            }\n";
#ifdef ISNAN
        ss << "        }\n";
        ss << "if(i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize << ") ;{\n";
#else
        ss << " i < " << nCurWindowSize << "){\n";
#endif
    }
    else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
        ss << "; i < " << pDVR->GetArrayLength();
        ss << " && i < (gid0+" << nCurWindowSize << " )/2*2; i++){\n";
#else
        ss << "; i < gid0+" << nCurWindowSize << " /2*2; i++)\n";
#endif
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg0)){\n";
#endif
        ss << "            fZaehler += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
#ifdef  ISNAN
        ss << "            }\n";
#endif
        ss << "            i++;\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg1)){\n";
#endif
        ss << "                fZaehler += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
#ifdef ISNAN
        ss << "        }\n";
        ss << "        if(i < " << pDVR->GetArrayLength();
        ss << " && i < gid0+" << nCurWindowSize << "){\n";
#else
        ss << " i < " << nCurWindowSize << "){\n";
#endif
    }
    else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
        ss << " ; i + gid0 < " << pDVR->GetArrayLength();
        ss << " &&  i < " << nCurWindowSize << " /2*2; i++){\n";
#else
        ss << "; i < " << nCurWindowSize << " /2*2; i++)\n";
#endif
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;" << ";\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg0)){\n";
#endif
        ss << "            fZaehler += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg1)){\n";
#endif
        ss << "                fZaehler += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
#ifdef ISNAN
        ss << "        }\n";
        ss << "        if(i + gid0 < " << pDVR->GetArrayLength() << " &&";
        ss << " i < " << nCurWindowSize << "){\n";
#else
        ss << " i < " << nCurWindowSize << "){\n";
#endif

    } else {
#ifdef  ISNAN
        ss << "; i < " << nCurWindowSize << " /2*2; i++){\n";
#else
        ss << "; i < " << nCurWindowSize << " /2*2; i++)\n";
#endif
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "            i++;" << ";\n";
        ss << "            arg1 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg0)){\n";
#endif
        ss << "            fZaehler += arg0 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg0/pow(1.0+x,nCount+1.0);\n";
        ss << "            nCount += 1;\n";
        ss << "            }\n";
#ifdef  ISNAN
        ss << "            if (!isNan(arg1)){\n";
#endif
        ss << "                fZaehler += arg1 / pow(1.0 + x, nCount);\n";
        ss << "            fNenner+=-1*nCount*arg1/pow(1.0+x,nCount+1.0);\n";
        ss << "                nCount+=1;\n";
        ss << "            }\n";
#ifdef ISNAN
        ss << "        }\n";
        ss << "if(i<" << nCurWindowSize << "){\n";
#else
        ss << " i < " << nCurWindowSize << "){\n";
#endif

    }
    ss << "            arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
#ifdef  ISNAN
    ss << "        if (isNan(arg0))\n";
    ss << "            continue;\n";
#endif
    ss << "        fZaehler += arg0 / pow(1.0+x, nCount);\n";
    ss << "        fNenner  += -nCount * arg0 / pow(1.0+x,nCount+1.0);\n";
    ss << "        nCount+=1;\n";
    ss << "        }\n";
    ss << "        xNeu = x - fZaehler / fNenner;\n";
    ss << "        fEps = fabs(xNeu - x);\n";
    ss << "        x = xNeu;\n";
    ss << "        nItCount++;\n    }\n";
    ss << "        if (fSchaetzwert == 0.0 && fabs(x) < Epsilon)\n";
    ss << "            x = 0.0;\n";
    ss << "        if (fEps < Epsilon)\n";
    ss << "            return x;\n";
    ss << "        else\n";
    ss << "            return (double)523;\n";
    ss << "}";
}

void XNPV::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
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
    ss << "double result = 0.0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int i=0;\n\t";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
        ss<< "i=gid0;\n\t";
    }
    ss << "double date;\n\t";
    ss << "double value;\n\t";
    ss << "double rate;\n\t";
    ss << "double dateNull;\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);

    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::DoubleVectorRefToken *>(tmpCur1);

    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::DoubleVectorRefToken*tmpCurDVR2= dynamic_cast<const
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
#endif
#ifdef ISNAN
    ss<<"if((i+gid0)>=buffer_date_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dateNull = 0;\n\telse \n\t\t";
#endif
    ss<<"dateNull = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if((i+gid0)>=buffer_rate_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"rate = 0;\n\telse \n\t\t";
#endif
    ss<<"rate = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
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
    ss << "result += ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "/(pow((";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"+1),(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << "-dateNull)/365));\n\t";
    ss <<"}\n\t";

    ss << "return result;\n";
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double result=0;\n\t";
    ss<< "int nNullDate = GetNullDate( );\n\t";
    ss <<"int settle;\n\t";
    ss <<"int mat;\n\t";
    ss <<"int issue;\n\t";
    ss <<"double rate;\n\t";
    ss <<"double yield;\n\t";
    ss <<"int  nBase;\n\t";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
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
#endif
#ifdef ISNAN
    ss<<"if(gid0>=buffer_settle_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"settle = 0;\n\telse \n\t\t";
#endif
    ss<<"settle = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_mat_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"mat = 0;\n\telse \n\t\t";
#endif
    ss<<"mat = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_issue_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"issue = 0;\n\telse \n\t\t";
#endif
    ss<<"issue = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_rate_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"rate = 0;\n\telse \n\t\t";
#endif
    ss<<"rate = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_yield_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"yield = 0;\n\telse \n\t\t";
#endif
    ss<<"yield = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_base_len || isNan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"nBase = 0;\n\telse \n\t\t";
#endif
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
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double result=0;\n\t";
    ss << "double cost;\n\t";
    ss << "double salvage;\n\t";
    ss << "double life;\n\t";
    ss << "double period;\n\t";
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

    ss << "int buffer_cost_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss << "int buffer_salvage_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss << "int buffer_life_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_period_len = ";
    ss << tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
#endif

#ifdef ISNAN
    ss <<"if(gid0>=buffer_cost_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"cost = 0;\n\telse \n\t\t";
#endif
    ss <<"cost = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0>=buffer_salvage_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"salvage = 0;\n\telse \n\t\t";
#endif
    ss <<"salvage = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0>=buffer_life_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"life = 0;\n\telse \n\t\t";
#endif
    ss <<"life = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0>=buffer_period_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"period = 0;\n\telse \n\t\t";
#endif
    ss <<"period = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss <<"result = ((cost-salvage)*(life-period+1)/((life*(life+1))/2));\n\t";
    ss <<"return result;\n\t";
    ss <<"}\n";
}

void MIRR::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    const formula::DoubleVectorRefToken* pCurDVR =
    dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
    size_t nCurWindowSize = pCurDVR->GetRefRowSize();
    FormulaToken* pCur1 = vSubArguments[1]->GetFormulaToken();
    assert(pCur1);
    const formula::SingleVectorRefToken* pSVR1 =
        dynamic_cast< const formula::SingleVectorRefToken* >(pCur1);
    assert(pSVR1);
    FormulaToken* pCur2 = vSubArguments[2]->GetFormulaToken();
    assert(pCur2);
    const formula::SingleVectorRefToken* pSVR2 =
        dynamic_cast< const formula::SingleVectorRefToken* >(pCur2);
    assert(pSVR2);

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
#ifdef  ISNAN
    ss << "if (gid0 >= argLen1)\n\t\t";
    ss << "arg1 = 0.0;\n\t";
    ss << "if (gid0 >= argLen2)\n\t\t";
    ss << "arg2 = 0.0;\n\t";
    ss << "if (isNan(arg1))\n\t\t";
    ss << "arg1 = 0.0;\n\t";
    ss << "if (isNan(arg2))\n\t\t";
    ss << "arg2 = 0.0;\n\t";
#endif
    ss << "double invest = arg1 + 1.0;\n\t";
    ss << "double reinvest = arg2 + 1.0;\n\t";
    ss << "double NPV_invest = 0.0;\n\t";
    ss << "double Pow_invest = 1.0;\n\t";
    ss << "double NPV_reinvest = 0.0;\n\t";
    ss << "double Pow_reinvest = 1.0;\n\t";
    ss << "int nCount = 0;\n\t";
    ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
#ifdef  ISNAN
    ss << "for (int i = 0; i + gid0 < arrayLength &&";
    ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
#else
    ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t";
#endif
    ss << "arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t\t";
#ifdef ISNAN
    ss << "if (isNan(arg0))\n\t\t\t";
    ss << "continue;\n\t\t";
#endif
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
    const std::string sSymName, SubArguments& vSubArguments)
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
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    int gid0 = get_global_id(0);\n\t";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    ss << "    double arg1 = " << GetBottom() << ";\n";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "    {\n";
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
        ss << "    arg" << i;
        ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
#endif
    }
    ss << "    tmp = pow(1.0 + arg0 * pow(arg1, -1), arg1)-1.0;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

 void OpTbilleq::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDiffDate360_Decl);decls.insert(GetDiffDate360Decl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(GetNullDateDecl);
    decls.insert(IsLeapYearDecl);
    funs.insert(GetDiffDate360_);funs.insert(GetDiffDate360);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(GetNullDate);
    funs.insert(IsLeapYear);
}
void OpTbilleq::GenSlidingWindowFunction(
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
    ss << "   int gid0 = get_global_id(0);\n";
    ss << "double tmp = 0;\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";

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

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
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
    decls.insert(GetRmzDecl); decls.insert(GetZwDecl);
    funs.insert(GetRmz);funs.insert(GetZw);
}
void OpCumprinc::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fRate,fVal;\n\t";
    ss <<"int nStartPer,nEndPer,nNumPeriods,nPayType;\n\t";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    ss << "int buffer_rate_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_NumPeriods_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_Val_len = ";
    ss << tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_StartPer_len = ";
    ss << tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_EndPer_len = ";
    ss << tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_PayType_len = ";
    ss << tmpCurDVR5->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_rate_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fRate = 0;\n\telse\n\t\t";
#endif
    ss <<"fRate = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_NumPeriods_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nNumPeriods = 0;\n\telse\n\t\t";
#endif
    ss <<"nNumPeriods = (int)"<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_Val_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fVal = 0;\n\telse\n\t\t";
#endif
    ss <<"fVal = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_StartPer_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nStartPer = 0;\n\telse\n\t\t";
#endif
    ss <<"nStartPer = (int)"<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_EndPer_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nEndPer = 0;\n\telse\n\t\t";
#endif
    ss <<"nEndPer = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_PayType_len || isNan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nPayType = 0;\n\telse\n\t\t";
#endif
    ss <<"nPayType = (int)"<<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss <<"double fRmz;\n\t";
    ss <<"fRmz = GetRmz( fRate, nNumPeriods,fVal,0.0,nPayType );\n\t";
    ss <<"uint nStart = nStartPer;\n\t";
    ss <<"uint nEnd = nEndPer ;\n\t";
    ss <<"if(nStart == 1)\n\t";
    ss <<"{\n\t\t";
    ss <<"if( nPayType <= 0 )\n\t\t\t";
    ss <<"tmp = fRmz + fVal * fRate;\n\t\t";
    ss <<"else\n\t\t\t";
    ss <<"tmp = fRmz;\n\t\t";
    ss <<"nStart=nStart+1;\n\t";
    ss <<"}\n\t";
    ss <<"for( uint i = nStart ; i <= nEnd ; i++ )\n\t";
    ss <<"{\n\t\t";
    ss <<"if( nPayType > 0 )\n\t\t\t";
    ss <<"tmp += fRmz - ( GetZw( fRate,convert_double(i - 2),";
    ss <<"fRmz,fVal,1)- fRmz ) * fRate;";
    ss <<"\n\t\telse\n\t\t\t";
    ss <<"tmp += fRmz - GetZw( fRate, convert_double(i - 1),";
    ss <<"fRmz,fVal,0 ) * fRate;\n";
    ss <<"}\n\t";
    ss << "return tmp;\n";
    ss << "}";
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
    ss << "    int nStartDate,nEndDate,mode,freq;\n";
    ss << "    int nDays1stYear=0;\n";
    ss << "    double fVal,fRate;\n";
#ifdef ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken* tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur5);
    FormulaToken* tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= dynamic_cast<const
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
#endif
#ifdef ISNAN
    ss<<"    if(gid0 >= buffer_nIssue_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nStartDate = 0;\n    else\n";
#endif
    ss <<"        nStartDate=(int)";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nEndDate = 0;\n    else\n";
#endif
    ss <<"        nEndDate=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_fRate_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRate = 0;\n    else\n";
#endif
    ss <<"        fRate=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_fVal_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fVal = 0;\n    else\n";
#endif
    ss <<"        fVal=";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        freq = 0;\n    else\n";
#endif
    ss <<"        freq= (int)";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nMode_len || isNan(";
    ss <<vSubArguments[6]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        mode = 0;\n    else\n";
#endif
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int nStartDate,nEndDate,mode;\n\t";
    ss << "double fRate,fVal;\n\t";
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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
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
#endif
#ifdef ISNAN
     ss <<"if(gid0 >= buffer_nIssue_len || isNan(";
     ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
     ss <<"))\n\t\t";
     ss <<"nStartDate = 0;\n\telse\n\t\t";
#endif
    ss << "nStartDate=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nEndDate = 0;\n\telse\n\t\t";
#endif
    ss << "nEndDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";

#ifdef ISNAN
    ss <<"if(gid0 >= buffer_fRate_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fRate = 0;\n\telse\n\t\t";
#endif
    ss << "fRate=";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_fVal_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fVal = 0;\n\telse\n\t\t";
#endif
    ss << "fVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nMode_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"mode = 0;\n\telse\n\t\t";
#endif
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
    ss << "double tmp = " <<"0"<<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";
    ss << "double tmp005;\n\t";
    ss << "double tmp006;\n\t";


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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);

    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);

    FormulaToken *tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= dynamic_cast<const
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
#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp003_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp004_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp005_len || isNan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp005 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp005 = ";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp006_len || isNan(";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp006 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp006 = ";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss << "tmp = getYield_(";
    ss << "GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005,tmp006);\n\t ";
    ss << "return tmp;\n";
    ss << "}";
}

void OpSLN::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "double tmp = " <<"0"<<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double wert;\n\t";
    ss << "double rest;\n\t";
    ss << "double dauer;\n\t";

#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur2);
    ss<< "int buffer_wert_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_rest_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_dauer_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss<<"if(gid0>=buffer_wert_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"wert = 0;\n\telse \n\t\t";
#endif
    ss<<"wert = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_rest_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"rest = 0;\n\telse \n\t\t";
#endif
    ss<<"rest = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
#ifdef ISNAN
    ss<<"if(gid0>=buffer_dauer_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"dauer = 0;\n\telse \n\t\t";
#endif
    ss<<"dauer = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss << "tmp = (wert-rest)/dauer;\n\t";
    ss << "return tmp;\n";
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
    ss << "double tmp = " <<"0"<<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";
    ss << "double tmp005;\n\t";

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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);

    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
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

#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp003_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp004_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp005_len || isNan(";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp005 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp005 = ";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss << "tmp = GetYieldmat(";
    ss<<"GetNullDate(),tmp000,tmp001,tmp002,tmp003,tmp004,tmp005);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpPMT::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    size_t i = vSubArguments.size();
    ss <<"\n    ";
    //while (i-- > 1)
    for (i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
                const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
        }
        else
        {
#ifdef  ISNAN
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss <<"    temp="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n";
            ss <<"    if (isNan(temp))\n";
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
#else
    ss <<"    tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";

#endif
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
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
        }
        else
        {
#ifdef  ISNAN
            ss << "nCount += 1;\n";
#endif
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        double temp=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        double temp1=1.0;";
            ss << "        if (isNan(temp)){\n";
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
#else
            ss << "tmp +="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<" / pow(1.0f+ arg0 ,";
            ss <<" (double)nCount );\n";
            ss << "        nCount += 1;\n";
#endif
    }
        ss << "    return tmp;\n";
        ss << "}";
}

 void OpPrice::BinInlineFun(std::set<std::string>& decls,
     std::set<std::string>& funs)
 {
    decls.insert(getPrice_Decl);
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

    funs.insert(getPrice_);
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
void OpPrice::GenSlidingWindowFunction(std::stringstream &ss,
          const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
      if (i)
          ss << ", ";
      vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n    ";
    ss << "double tmp = 0;\n    ";
    ss << "int gid0 = get_global_id(0);\n    ";
    ss<<"double tmp0=0;\n    ";
    ss<<"double tmp1=0;\n    ";
    ss<<"double tmp2=0;\n    ";
    ss<<"double tmp3=0;\n    ";
    ss<<"double tmp4=0,tmp5=0;\n    ";
    ss<<"double tmp6=0;\n    ";
    size_t nItems = 0;
    ss <<"\n    ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n        ";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n        ";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            }
            nItems += nCurWindowSize;
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n        ";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n        ";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n            ";
            ss << "tmp"<<i<<"= 0;\n        ";
            ss << "else\n            ";
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n    }\n    ";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n    ";
        }
#else
        ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
        ss <<";\n    ";
#endif
    }
    ss << "tmp = getPrice_(tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);\n\t";
    ss << "return tmp;\n";
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
          const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    size_t nItems = 0;
    ss <<"    \n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            nItems += nCurWindowSize;
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
    ss << "        tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";
#endif
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
          const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    size_t nItems = 0;
    ss <<"    \n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            nItems += nCurWindowSize;
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
    ss << "        tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";
#endif
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
          const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    size_t nItems = 0;
    ss <<"    \n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n";
#endif
            }
            nItems += nCurWindowSize;
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
    ss << "        tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";
#endif
    }
    ss <<"    int nNullDate = GetNullDate();\n";
    ss <<"    tmp=tmp3* ( 1.0 -tmp2*GetYearDiff( nNullDate, ";
    ss <<"tmp0,tmp1,tmp4));\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpNper::GenSlidingWindowFunction(std::stringstream &ss,
         const std::string sSymName, SubArguments &vSubArguments)
 {
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    size_t nItems = 0;
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        if (isNan(";
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
#else
    ss << "        tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
    ss <<";\n";
#endif
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
    decls.insert(GetZwDecl);
    funs.insert(GetZw);
}

void OpPPMT::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n    ";
    ss << "double tmp = 0;\n    ";
    ss << "int gid0 = get_global_id(0);\n    ";
    ss<<"double tmp0=0;\n    ";
    ss<<"double tmp1=0;\n    ";
    ss<<"double tmp2=0;\n    ";
    ss<<"double tmp3=0;\n    ";
    ss<<"double tmp4=0,tmp5=0;\n    ";
    size_t nItems = 0;
    ss <<"\n    ";
    //while (i-- > 1)
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n        ";
#else
                ss << "gid0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n        ";
#endif
            } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            }
            else {
#ifdef  ISNAN
                ss << "0; i < "<< nCurWindowSize << "; i++){\n        ";
#else
                ss << "0; i < "<< nCurWindowSize << "; i++)\n        ";
#endif
            }
            nItems += nCurWindowSize;
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
#ifdef  ISNAN
            const formula::SingleVectorRefToken* pSVR =
                dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n        ";
#else
            nItems += 1;
#endif
        }
        else if (pCur->GetType() == formula::svDouble)
        {
#ifdef  ISNAN
            ss << "{\n        ";
#endif
            nItems += 1;
        }
        else
        {
#ifdef  ISNAN
#endif
            nItems += 1;
        }
#ifdef  ISNAN
        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "if (isNan(";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << "))\n            ";
            ss << "tmp"<<i<<"= 0;\n        ";
            ss << "else\n            ";
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n    }\n    ";
        }
        else
        {
            ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss <<";\n    ";
        }
#else
        ss << "tmp"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
        ss <<";\n    ";
#endif
    }
    ss <<"double pmt ;\n    ";
    ss<<"if(tmp0==0.0)\n    ";
    ss<<"    return -(tmp3+tmp4)/";
    ss<<"tmp2;\n    double temp1=0;\n    ";
    ss <<"double abl = pow(1.0+tmp0,tmp2);\n    ";
    ss <<"temp1-=tmp4";
    ss<<";\n    ";
    ss<<"temp1-=tmp3*abl;\n    ";
    ss <<"pmt = temp1/(1.0+tmp0*tmp5";
    ss<<") / ( (abl-1.0)/tmp0);\n    ";
    ss <<"double temp = pow( 1+tmp0,tmp1-2);\n    ";
    ss <<"double re;\n    ";
    ss <<"if(tmp1==1.0)\n    {\n    ";
    ss <<"if(tmp5>0.0)\n    ";
    ss <<"    re=0.0;\n    ";
    ss << "else\n        re=-tmp3;\n    }\n    ";
    ss <<"else\n    {\n    ";
    ss <<"if(tmp5>0.0)\n    ";
    ss <<"    re=GetZw(tmp0, tmp1-2.0, pmt, tmp3, 1.0) - pmt;\n    ";
    ss <<"else\n    ";
    ss <<"    re=GetZw(tmp0, tmp1-1.0, pmt, tmp3, 0.0);\n    ";
    ss <<"}\n    ";
    ss <<"re = re * tmp0;\n    ";
    ss <<"tmp = pmt - re;\n    ";
    ss << "return tmp;\n";
    ss << "}";
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
    decls.insert(lcl_GetcoupdaybsDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(lcl_Getcoupdaybs);
}
void OpCoupdaybs::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
    ss<< "int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
   ss << "nSettle=(int)";
   ss << vSubArguments[0]->GenSlidingWindowDeclRef();
   ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nMat = 0;\n\telse\n\t\t";
#endif
    ss << "nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
    ss << "nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nBase = 0;\n\telse\n\t\t";
#endif
    ss << "nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss <<"int nNullDate=GetNullDate();\n\t";
    ss <<"tmp = lcl_Getcoupdaybs(nNullDate,";
    ss <<"nSettle, nMat,nFreq,nBase);\n\t";
    ss << "return tmp;\n";
    ss << "}";
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
    decls.insert(lcl_GetcoupdaysDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(lcl_Getcoupdays);
}

void OpCoupdays::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
    ss<< "int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
   ss << "nSettle=(int)";
   ss << vSubArguments[0]->GenSlidingWindowDeclRef();
   ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nMat = 0;\n\telse\n\t\t";
#endif
    ss << "nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
    ss << "nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nBase = 0;\n\telse\n\t\t";
#endif
    ss << "nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss <<"int nNullDate=GetNullDate();\n\t";
    ss <<"if( nBase == 1 )\n\t\t";
    ss <<"tmp = lcl_Getcoupdays(nNullDate,";
    ss <<"nSettle, nMat,nFreq, nBase);\n\t";
    ss <<"else\n\t\t";
    ss <<"tmp = (double)GetDaysInYear(0,0,nBase)/nFreq;\n\t";
    ss << "return tmp;\n";
    ss << "}";
}
void OpCouppcd::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(ScaDate2Decl);
    decls.insert(lcl_GetCouppcdDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate2);
    funs.insert(lcl_GetCouppcd);
}
void OpCouppcd::GenSlidingWindowFunction(
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int bLastDayMode =1,bLastDay =0,b30Days= 0,bUSMode = 0;\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
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
    ss<< "    int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
#endif
   ss << "        nSettle=(int)";
   ss << vSubArguments[0]->GenSlidingWindowDeclRef();
   ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nMat = 0;\n    else\n";
#endif
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
#endif
    ss <<"        nFreq=(int)";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nBase = 0;\n    else\n";
#endif
    ss <<"        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    int nNullDate=GetNullDate();\n";
    ss <<"    ScaDate2(nNullDate, nMat, nBase,&bLastDayMode,&bLastDay,";
    ss <<"&b30Days,&bUSMode);\n";
    ss <<"    tmp = lcl_GetCouppcd(nNullDate,nSettle,nMat,nFreq,";
    ss <<"&bLastDayMode,&bLastDay,&b30Days,&bUSMode);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpCoupncd::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(ScaDate2Decl);
    decls.insert(lcl_GetCoupncdDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate2);
    funs.insert(lcl_GetCoupncd);
}
void OpCoupncd::GenSlidingWindowFunction(
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
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int bLastDayMode =1,bLastDay =0,b30Days= 0,bUSMode = 0;\n";
    ss << "    int nSettle,nMat,nFreq,nBase;\n";
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
    ss<< "    int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nSettle = 0;\n    else\n";
#endif
   ss << "        nSettle=(int)";
   ss << vSubArguments[0]->GenSlidingWindowDeclRef();
   ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nMat = 0;\n    else\n";
#endif
    ss <<"        nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nFreq = 0;\n    else\n";
#endif
    ss <<"        nFreq=(int)";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nBase = 0;\n    else\n";
#endif
    ss <<"        nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    int nNullDate=GetNullDate();\n";
    ss <<"    ScaDate2(nNullDate, nMat, nBase,&bLastDayMode,&bLastDay,";
    ss <<"&b30Days,&bUSMode);\n";
    ss <<"    tmp = lcl_GetCoupncd(nNullDate,nSettle,nMat,nFreq,";
    ss <<"&bLastDayMode,&bLastDay,&b30Days,&bUSMode);\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpCoupdaysnc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl); decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(ScaDateDecl);
    decls.insert(addMonthsDecl); decls.insert(getDaysInMonthRangeDecl);
    decls.insert(GetDaysInYearsDecl); decls.insert(GetDaysInYearDecl);
    decls.insert(getDaysInYearRangeDecl); decls.insert(getDiffDecl);
    decls.insert(lcl_GetcoupdaysDecl);decls.insert(lcl_GetcoupdaybsDecl);
    decls.insert(coupdaysDecl);decls.insert(coupdaybsDecl);
    decls.insert(coupdaysncDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(ScaDate);
    funs.insert(addMonths);funs.insert(getDaysInMonthRange);
    funs.insert(GetDaysInYears);funs.insert(GetDaysInYear);
    funs.insert(getDaysInYearRange);funs.insert(getDiff);
    funs.insert(lcl_Getcoupdays);funs.insert(lcl_Getcoupdaybs);
    funs.insert(coupdaysnc);funs.insert(coupdaybs);
    funs.insert(coupdays);
}
void OpCoupdaysnc::GenSlidingWindowFunction(
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
    ss<< "int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
   ss << "nSettle=(int)";
   ss << vSubArguments[0]->GenSlidingWindowDeclRef();
   ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nMat = 0;\n\telse\n\t\t";
#endif
    ss << "nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
    ss << "nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nBase = 0;\n\telse\n\t\t";
#endif
    ss << "nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss <<"tmp = coupdaysnc(nSettle,nMat,nFreq,nBase);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpCoupnum::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl);decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl);decls.insert(lcl_GetcoupnumDecl);
    decls.insert(coupnumDecl);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);
    funs.insert(lcl_Getcoupnum);
    funs.insert(coupnum);
}
void OpCoupnum::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
      if (i)
          ss << ",";
      vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n    ";
    ss << "double tmp = " << GetBottom() <<";\n    ";
    ss << "int gid0 = get_global_id(0);\n    ";
    ss << "int nSettle,nMat,nFreq,nBase;\n    ";
#ifdef ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    ss<< "int buffer_nSettle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n    ";
    ss<< "int buffer_nMat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n    ";
    ss<< "int buffer_nFreq_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n    ";
    ss<< "int buffer_nBase_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n    ";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n        ";
    ss <<"nSettle = 0;\n    else\n        ";
#endif
    ss << "nSettle=(int)";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n    ";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nMat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n        ";
    ss <<"nMat = 0;\n    else\n        ";
#endif
    ss << "nMat=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n    ";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n        ";
    ss <<"nFreq = 0;\n    else\n        ";
#endif
    ss << "nFreq=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n    ";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_nBase_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n        ";
    ss <<"nBase = 0;\n    else\n        ";
#endif
    ss << "nBase=(int)";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n    ";
    ss <<"tmp = coupnum(nSettle,nMat,nFreq,nBase);\n    ";
    ss << "return tmp;\n";
    ss << "}";
}
void OpAmordegrc::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(nKorrValDecl); decls.insert(RoundDecl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(GetYearFracDecl);
    funs.insert(Round);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(GetYearFrac);
}
void OpAmordegrc::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    FormulaToken *tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur6);
    ss<< "    int buffer_Cost_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Date_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_FirstPer_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_RestVal_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Per_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Rate_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Base_len = ";
    ss<< tmpCurDVR6->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Cost_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fCost = 0;\n    else\n";
 #endif
    ss << "        fCost=";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Date_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nDate = 0;\n    else\n";
#endif
    ss << "        nDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_FirstPer_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nFirstPer = 0;\n    else\n";
#endif
    ss << "        nFirstPer=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_RestVal_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRestVal = 0;\n    else\n";
#endif
    ss << "        fRestVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Per_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fPer = 0;\n    else\n";
#endif
    ss << "        fPer = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Rate_len || isNan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRate = 0;\n    else\n";
#endif
    ss << "        fRate=";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Base_len || isNan(";
    ss <<vSubArguments[6]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nBase = 0;\n    else\n";
#endif
    ss << "        nBase = (int)";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss << ";\n";
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
    ss <<"    tmp = Round( GetYearFrac( GetNullDate(),";
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
    decls.insert(nKorrValDecl); decls.insert(RoundDecl);
    decls.insert(IsLeapYearDecl);decls.insert(DaysInMonthDecl);
    decls.insert(DaysToDateDecl); decls.insert(DateToDaysDecl);
    decls.insert(GetNullDateDecl); decls.insert(GetYearFracDecl);
    funs.insert(Round);
    funs.insert(IsLeapYear);funs.insert(DaysInMonth);
    funs.insert(DaysToDate);funs.insert(DateToDays);
    funs.insert(GetNullDate);funs.insert(GetYearFrac);
}
void OpAmorlinc::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "    double tmp = " << GetBottom() <<";\n";
    ss << "    double fCost,fRestVal,fPer,fRate;\n";
    ss << "    int nDate,nFirstPer,nBase;\n";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    FormulaToken *tmpCur5 = vSubArguments[5]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR5= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur5);
    FormulaToken *tmpCur6 = vSubArguments[6]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR6= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur6);
    ss<< "    int buffer_Cost_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss<< ";\n";
    ss<< "    int buffer_Date_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_FirstPer_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_RestVal_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Per_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Rate_len = ";
    ss<< tmpCurDVR5->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_Base_len = ";
    ss<< tmpCurDVR6->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Cost_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fCost = 0;\n    else\n";
#endif
    ss << "        fCost=";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Date_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nDate = 0;\n    else\n";
#endif
    ss << "        nDate=(int)";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_FirstPer_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nFirstPer = 0;\n    else\n";
#endif
    ss << "        nFirstPer=(int)";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_RestVal_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRestVal = 0;\n    else\n";
#endif
    ss << "        fRestVal=";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Per_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fPer = 0;\n    else\n";
#endif
    ss << "        fPer = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Rate_len || isNan(";
    ss <<vSubArguments[5]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRate = 0;\n    else\n";
#endif
    ss << "        fRate=";
    ss << vSubArguments[5]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_Base_len || isNan(";
    ss <<vSubArguments[6]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nBase = 0;\n    else\n";
#endif
    ss << "        nBase = (int)";
    ss << vSubArguments[6]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss <<"    int  nPer = convert_int( fPer );\n";
    ss <<"    double fOneRate = fCost * fRate;\n";
    ss <<"    double fCostDelta = fCost - fRestVal;\n";
    ss <<"    double f0Rate = GetYearFrac( GetNullDate(),";
    ss <<"nDate, nFirstPer, nBase )* fRate * fCost;\n";
    ss <<"    int nNumOfFullPeriods = (int)";
    ss <<"( ( fCost - fRestVal - f0Rate) /fOneRate );\n";
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
        const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "int nSettle, nMat;\n\t";
    ss << "double fInvest,fDisc;\n\t";
    ss << "int rOB;\n\t";
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
    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "int buffer_settle_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_mat_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_invest_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_disc_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n\t";
    ss<< "int buffer_rob_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n\t";
#endif
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_settle_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
    ss <<"nSettle = (int)"<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_mat_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"nMat = 0;\n\telse\n\t\t";
#endif
    ss <<"nMat = (int)";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_invest_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fInvest = 0;\n\telse\n\t\t";
#endif
    ss <<"fInvest = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_disc_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"fDisc = 0;\n\telse\n\t\t";
#endif
    ss <<"fDisc = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
#ifdef ISNAN
    ss <<"if(gid0 >= buffer_rob_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n\t\t";
    ss <<"rOB = 0;\n\telse\n\t\t";
#endif
    ss <<"rOB = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n\t";
    ss << "tmp = fInvest/(1.0-(fDisc";
    ss <<" * GetYearDiff( GetNullDate()";
    ss <<",nSettle,nMat,rOB)));";
    ss << "\n\treturn tmp;\n";
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
    ss << "double tmp = " <<"0"<<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";
    ss << "double tmp003;\n\t";
    ss << "double tmp004;\n\t";


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

    FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
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

#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp003_len || isNan(";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp003 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp003 = ";
    ss << vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp004_len || isNan(";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp004 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp004 = ";
    ss << vSubArguments[4]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<< "tmp = (tmp003/tmp002)-1;\n\t";
    ss << "tmp /= GetYearFrac( GetNullDate(),tmp000,tmp001,tmp004);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

 void OpTbillprice::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetYearFracDecl);decls.insert(GetNullDateDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(IsLeapYearDecl);

    funs.insert(GetYearFrac);funs.insert(GetNullDate);
    funs.insert(DateToDays);funs.insert(DaysToDate);
    funs.insert(DaysInMonth);funs.insert(IsLeapYear);
}

void OpTbillprice::GenSlidingWindowFunction(
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";



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

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

    ss<<"tmp001+=1.0;\n";
    ss<<"double  fFraction =GetYearFrac(GetNullDate(),tmp000,tmp001,0);\n";
    ss<<"tmp = 100.0 * ( 1.0 - tmp002 * fFraction );\n";
    ss << "return tmp;\n";
    ss << "}\n";
}
 void RATE::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(approxEqualDecl);decls.insert(nKorrValDecl);
    decls.insert(SCdEpsilonDecl);decls.insert(RoundDecl);
    funs.insert(approxEqual);funs.insert(Round);
}

void RATE::GenSlidingWindowFunction(
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

    FormulaToken* pCur = vSubArguments[5]->GetFormulaToken();
    assert(pCur);
    const formula::SingleVectorRefToken* pSVR =
        dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
    assert(pSVR);
    ss << ") {\n\t";
    ss << "double result;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "bool bValid = true, bFound = false;\n\t";
    ss << "double fX, fXnew, fTerm, fTermDerivation;\n\t";
    ss << "double fGeoSeries, fGeoSeriesDerivation;\n\t";
    ss << "int nIterationsMax = 150;\n\t";
    ss << "int nCount = 0;\n\t";
    ss << "double fEpsilonSmall = 1.0E-14;\n\t";
    ss << "double arg0, arg1, arg2, arg3, arg4, arg5;\n\t";
    ss << "arg0=" << vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "arg1=" << vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "arg2=" << vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "arg3=" << vSubArguments[3]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "arg4=" << vSubArguments[4]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "arg5=" << vSubArguments[5]->GenSlidingWindowDeclRef()<<";\n\t";
    ss << "int guessLen = " << pSVR->GetArrayLength() << ";\n\t";
    #ifdef  ISNAN
    ss << "if (isNan(arg0) || isNan(arg1) || isNan(arg2)){\n\t\t";
    ss << "result = 523;\n\t\t";
    ss << "return result;\n\t}\n\t";
    ss << "if (isNan(arg3))\n\t\t";
    ss << "arg3 = 0.0;\n\t";
    ss << "if (isNan(arg4))\n\t\t";
    ss << "arg4 = 0.0;\n\t";
    ss << "if (isNan(arg5))\n\t\t";
    ss << "arg5 = 0.1;\n\t";
    ss << "if (gid0 >= guessLen)\n\t\t";
    ss << "arg5 = 0.1;\n\t";
    #endif
    ss << "arg3 = arg3 - arg1 * arg4;\n\t";
    ss << "arg2 = arg2 + arg1 * arg4;\n\t";
    ss << "if (arg0 == Round(arg0)){\n\t\t";
    ss << "fX = arg5;\n\t\t";
    ss << "double fPowN, fPowNminus1;\n\t\t";
    ss << "while (!bFound && nCount < nIterationsMax)\n\t\t";
    ss << "{\n\t\t\t";
    ss << "fPowNminus1 = pow( 1.0+fX, arg0-1.0);\n\t\t\t";
    ss << "fPowN = fPowNminus1 * (1.0+fX);\n\t\t\t";
    ss << "if (approxEqual( fabs(fX), 0.0))\n\t\t\t" << "{\n\t\t\t\t";
    ss << "fGeoSeries = arg0;\n\t\t\t\t";
    ss << "fGeoSeriesDerivation = arg0 * (arg0-1.0)/2.0;\n\t\t\t";
    ss << "}\n\t\t\t" << "else\n\t\t\t{\n\t\t\t\t";
    ss << "fGeoSeries = (fPowN-1.0)/fX;\n\t\t\t\t";
    ss << "fGeoSeriesDerivation =";
    ss << " arg0 * fPowNminus1 / fX - fGeoSeries /fX;";
    ss << "\n\t\t\t" << "}\n\t\t\t";
    ss << "fTerm = arg3 + arg2 *fPowN+ arg1 * fGeoSeries;\n\t\t\t";
    ss << "fTermDerivation = arg2 * arg0 * fPowNminus1 +";
    ss << " arg1 * fGeoSeriesDerivation;\n\t\t\t";
    ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
    ss << "bFound = true;\n\t\t\t";
    ss << "else\n\t\t\t{\n\t\t\t\t";
    ss << "if (approxEqual(fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
    ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
    ss << "else\n\t\t\t\t\tfXnew = fX - fTerm / fTermDerivation;\n\t\t";
    ss << "nCount++;\n\t\t\t\t";
    ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
    ss << "fX = fXnew;\n\t\t\t" << "}\n\t\t}\n\t}\n\telse\n\t{\n\t\t";
    ss << "fX = (arg5 < -1.0) ? -1.0 : arg5;\n\t\t";
    ss << "while (bValid && !bFound && nCount < nIterationsMax)\n\t\t";
    ss << "{\n\t\t\t";
    ss << "if (approxEqual(fabs(fX), 0.0)){\n\t\t\t\t";
    ss << "fGeoSeries = arg0;\n\t\t\t\t";
    ss << "fGeoSeriesDerivation = arg0 * (arg0-1.0)/2.0;\n\t\t\t";
    ss << "}else{\n\t\t\t\t";
    ss << "fGeoSeries = (pow( 1.0+fX, arg0) - 1.0) / fX;\n\t\t\t\t";
    ss << "fGeoSeriesDerivation =";
    ss << " arg0 * pow(1.0+fX,arg0-1.0) /";
    ss << " fX - fGeoSeries / fX;\n\t\t\t}\n\t\t\t";
    ss << "fTerm = arg3 + arg2 *pow(1.0+fX, arg0)";
    ss << "+ arg1 * fGeoSeries;\n\t\t\t";
    ss << "fTermDerivation =";
    ss << "arg2*arg0*pow(1.0+fX,arg0-1.0)";
    ss << "+arg1*fGeoSeriesDerivation;\n\t\t\t";
    ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
    ss << "bFound = true;\n\t\t\t";
    ss << "else{\n\t\t\t\t";
    ss << "if (approxEqual(fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
    ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
    ss << "else\n\t\t\t\t\t";
    ss << "fXnew = fX - fTerm / fTermDerivation;\n\t\t\t\t";
    ss << "nCount++;\n\t\t\t\t";
    ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
    ss << "fX = fXnew;\n\t\t\t\t";
    ss << "bValid = (fX >= -1.0);\n\t\t\t";
    ss << "}\n\t\t}\n\t}\n\t";
    ss << "if (bValid && bFound)\n\t\t";
    ss << "result = fX;\n\t";
    ss << "else\n\t\t";
    ss << "result = 523;\n\t";
    ss << "return result;\n";
    ss << "}";
}

 void OpTbillyield::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(GetDiffDate360Decl);decls.insert(IsLeapYearDecl);
    decls.insert(DateToDaysDecl);decls.insert(DaysToDateDecl);
    decls.insert(DaysInMonthDecl);decls.insert(GetNullDateDecl);
    decls.insert(GetDiffDate360_Decl);
    funs.insert(GetDiffDate360);funs.insert(DateToDays);
    funs.insert(DaysToDate);funs.insert(IsLeapYear);
    funs.insert(DaysInMonth);funs.insert(GetNullDate);
    funs.insert(GetDiffDate360_);

}

void OpTbillyield::GenSlidingWindowFunction(
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
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double tmp = 0;\n\t";
    ss << "double tmp000;\n\t";
    ss << "double tmp001;\n\t";
    ss << "double tmp002;\n\t";



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

    ss<< "int buffer_tmp000_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp001_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";

    ss<< "int buffer_tmp002_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n\t";
#endif

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp000_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp000 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp000 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp001_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp001 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp001 = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n\t";

#ifdef ISNAN
    ss<<"if(gid0>=buffer_tmp002_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n\t\t";
    ss<<"tmp002 = 0;\n\telse \n\t\t";
#endif
    ss<<"tmp002 = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n\t";
    ss <<"int nDiff=GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
    ss <<"nDiff++;\n";
    ss <<"tmp=100.0;\n";
    ss <<"tmp /= tmp002;\n";
    ss <<"tmp-=1.0;\n";
    ss <<"tmp= tmp/( nDiff );\n";
    ss <<"tmp *= 360.0;\n";
    ss <<"return tmp;\n";
    ss << "}\n";
}
void OpDDB::GenSlidingWindowFunction(std::stringstream& ss,
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
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = 0;\n";
    ss << "    double fWert,fRest,fDauer,fPeriode,fFaktor;\n";
    ss << "    double fGda, fZins, fAlterWert, fNeuerWert;\n";
#ifdef ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "    int buffer_wert_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_rest_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_dauer_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_periode_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_factor_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_wert_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fWert = 0;\n    else\n";
#endif
    ss <<"        fWert = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_rest_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fRest = 0;\n    else\n";
#endif
    ss <<"        fRest = ";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_dauer_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fDauer = 0;\n    else\n";
#endif
    ss <<"        fDauer = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_periode_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fPeriode = 0;\n    else\n";
#endif
    ss <<"        fPeriode = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_factor_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        fFaktor = 0;\n    else\n";
#endif
    ss <<"        fFaktor = "<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    fZins = fFaktor / fDauer;\n";
    ss <<"    if (fZins >= 1.0)\n";
    ss <<"    {\n";
    ss <<"        fZins = 1.0;\n";
    ss <<"        if (fPeriode == 1.0)\n";
    ss <<"            fAlterWert = fWert;\n";
    ss <<"        else\n";
    ss <<"            fAlterWert = 0.0;\n";
    ss <<"    }\n";
    ss <<"    else\n";
    ss <<"        fAlterWert = fWert * pow(1.0 - fZins, fPeriode - 1.0);\n";
    ss <<"    fNeuerWert = fWert * pow(1.0 - fZins, fPeriode);\n";
    ss <<"    if (fNeuerWert < fRest)\n";
    ss <<"        fGda = fAlterWert - fRest;\n";
    ss <<"    else\n";
    ss <<"        fGda = fAlterWert - fNeuerWert;\n";
    ss <<"    if (fGda < 0.0)\n";
    ss <<"        fGda = 0.0;\n";
    ss <<"    tmp = fGda;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
void OpPV::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &
vSubArguments)
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
    ss << "    double result = " <<"0"<<";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double zins;\n";
    ss << "    double zzr;\n";
    ss << "    double rmz;\n";
    ss << "    double zw;\n";
    ss << "    double flag;\n";

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

     const formula::SingleVectorRefToken*tmpCurDVR3;
     const formula::SingleVectorRefToken*tmpCurDVR4;

    if(vSubArguments.size()>3)
    {
        FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
        tmpCurDVR3= dynamic_cast<const formula::SingleVectorRefToken *>(
tmpCur3);
        ss<< "    int buffer_zw_len = ";
        ss<< tmpCurDVR3->GetArrayLength();
        ss << ";\n";
        }

    if(vSubArguments.size()>4)
    {
        FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
        tmpCurDVR4= dynamic_cast<const formula::SingleVectorRefToken *>(
tmpCur4);
        ss<< "    int buffer_flag_len = ";
        ss<< tmpCurDVR4->GetArrayLength();
        ss << ";\n";
    }

    ss<< "    int buffer_zins_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_zzr_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";

    ss<< "    int buffer_rmz_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";

#endif

#ifdef ISNAN
    ss<<"    if(gid0>=buffer_zins_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        zins = 0;\n    else \n";
#endif
    ss<<"        zins = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<";\n";

#ifdef ISNAN
    ss<<"    if(gid0>=buffer_zzr_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        zzr = 0;\n    else \n";
#endif
    ss<<"        zzr = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<";\n";

#ifdef ISNAN
    ss<<"    if(gid0>=buffer_rmz_len || isNan(";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"))\n";
    ss<<"        rmz = 0;\n    else \n";
#endif
    ss<<"        rmz = ";
    ss << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<";\n";

    if(vSubArguments.size()>3)
    {
#ifdef ISNAN
        ss<<"    if(gid0>=buffer_zw_len || isNan(";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"))\n";
        ss<<"        zw = 0;\n    else \n";
#endif
        ss<<"        zw = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<";\n";
    }else
    {
        ss<<"    zw = 0;\n";
    }

    if(vSubArguments.size()>4)
    {
#ifdef ISNAN
        ss<<"    if(gid0>=buffer_flag_len || isNan(";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<"))\n";
        ss<<"        flag = 0;\n    else \n";
#endif
        ss<<"        flag = ";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<";\n";
    }else
    {
        ss<<"        flag = 0;\n";
    }
    ss << "    if(zins == 0)\n";
    ss << "        result=zw+rmz*zzr;\n";
    ss << "    else if(flag > 0)\n";
    ss << "        result=(zw*pow(1+zins,-zzr))+";
    ss << "(rmz*(1-pow(1+zins,-zzr+1))/zins)+rmz;\n";
    ss << "    else\n";
    ss << "        result=(zw*pow(1+zins,-zzr))+";
    ss << "(rmz*(1-pow(1+zins,-zzr))/zins);\n";
    ss << "    return -result;\n";
    ss << "}";
}
 void OpVDB::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(ScGetGDADecl);decls.insert(DblMinDecl);
    decls.insert(ScInterVDBDecl);decls.insert(VDBImplementDecl);
    funs.insert(ScGetGDA);funs.insert(DblMin);
    funs.insert(ScInterVDB);funs.insert(VDBImplement);
}

void OpVDB::GenSlidingWindowFunction(
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
             const std::string sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= dynamic_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
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
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double nWert,nRest,nDauer,nPeriode;\n";
    ss << "    int nMonate;\n";
    ss << "    double tmp = 0;\n";
#ifdef ISNAN
    FormulaToken* tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken* tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    FormulaToken* tmpCur2 = vSubArguments[2]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur2);
    FormulaToken* tmpCur3 = vSubArguments[3]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur3);
    FormulaToken* tmpCur4 = vSubArguments[4]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur4);
    ss<< "    int buffer_wert_len = ";
    ss<< tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_rest_len = ";
    ss<< tmpCurDVR1->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_dauer_len = ";
    ss<< tmpCurDVR2->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_periode_len = ";
    ss<< tmpCurDVR3->GetArrayLength();
    ss << ";\n";
    ss<< "    int buffer_nMonate_len = ";
    ss<< tmpCurDVR4->GetArrayLength();
    ss << ";\n";
#endif
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_wert_len || isNan(";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nWert = 0;\n    else\n";
#endif
    ss <<"        nWert = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_rest_len || isNan(";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nRest = 0;\n    else\n";
#endif
    ss <<"        nRest = ";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_dauer_len || isNan(";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nDauer = 0;\n    else\n";
#endif
    ss <<"        nDauer = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_periode_len || isNan(";
    ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nPeriode = 0;\n    else\n";
#endif
    ss <<"        nPeriode = "<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss <<"    if(gid0 >= buffer_nMonate_len || isNan(";
    ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<"))\n";
    ss <<"        nMonate = 0;\n    else\n";
#endif
    ss <<"        nMonate = (int)"<<vSubArguments[4]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss <<"    double nAbRate = 1.0 - pow(nRest / nWert, 1.0 / nDauer);\n";
    ss <<"    nAbRate = ((int)(nAbRate * 1000.0 + 0.5)) / 1000.0;\n";
    ss <<"    double nErsteAbRate = nWert * nAbRate * nMonate / 12.0;\n";
    ss <<"    double nGda2 = 0.0;\n";
    ss <<"    if ((int)(nPeriode) == 1)\n";
    ss <<"        nGda2 = nErsteAbRate;\n";
    ss <<"    else\n";
    ss <<"    {\n";
    ss <<"        double nSummAbRate = nErsteAbRate;\n";
    ss <<"        double nMin = nDauer;\n";
    ss <<"        if (nMin > nPeriode) nMin = nPeriode;\n";
    ss <<"        int iMax = (int)nMin;\n";
    ss <<"        for (int i = 2; i <= iMax; i++)\n";
    ss <<"        {\n";
    ss <<"            nGda2 = (nWert - nSummAbRate) * nAbRate;\n";
    ss <<"            nSummAbRate += nGda2;\n";
    ss <<"        }\n";
    ss <<"        if (nPeriode > nDauer)\n";
    ss <<"            nGda2 = ((nWert - nSummAbRate)";
    ss <<"* nAbRate * (12.0 - nMonate)) / 12.0;\n";
    ss <<"    }\n";
    ss <<"    tmp = nGda2;\n";
    ss <<"    return tmp;\n";
    ss <<"}";
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
