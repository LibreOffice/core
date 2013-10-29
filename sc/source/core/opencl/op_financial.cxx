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
#include <list>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <memory>
using namespace formula;

namespace sc { namespace opencl {

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
    ss << "double tmp = 0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss <<"double tmp0 = ";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n\t";
    ss <<"double tmp1 = ";
    ss <<vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n\t";
#ifdef ISNAN

    FormulaToken *tmpCur0 = vSubArguments[0]
        ->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    ss<<"if("<<tmpCurDVR0->GetArrayLength()<<"<=gid0||";
    ss <<"isNan(tmp0))\n\t\t";
    ss<<" tmp0= 0;\n\t";
    FormulaToken *tmpCur1 = vSubArguments[1]
        ->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss<<"if("<<tmpCurDVR1->GetArrayLength()<<"<=gid0||";
    ss <<"isNan(tmp1))\n\t\t";
    ss<<" tmp1= 0;\n\t";
#endif
    ss<<"if(tmp1==0)\n\t";
    ss<<"\treturn 0;\n\t";
    ss<<"tmp=( pow( tmp0+ 1.0, 1.0 / tmp1 ) - 1.0 ) *";
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
    ss << "int nNullDate = GetNullDate( 30 , 12 , 1899 );\n\t";
    ss << "tmp = 1.0 - price / redemp;\n\t";
    ss << "tmp /= GetYearFrac( nNullDate , settle , maturity , mode );\n\t";
    ss << "return tmp;\n";
    ss << "}";
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
    ss << "int nNullDate = GetNullDate(30,12,1899);\n\t";
    ss << "tmp = (redemp / price - 1.0) / GetYearDiff(nNullDate, settle, maturity, mode);\n\t";
    ss << "return tmp;\n";
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
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "#define  Epsilon   1.0E-7\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "double fSchaetzwert = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n\t";
    ss << "double fEps = 1.0;\n\t";
    ss << "double x = 0.0, xNeu = 0.0, fZaehler = 0.0, fNenner = 0.0;\n\t";
    ss << "double nCount = 0.0;\n\t";
    ss << "int argLen1 = " << pSVR1->GetArrayLength() << ";\n\t";
#ifdef ISNAN
    ss << "if (gid0 >= argLen1)\n\t\t";
    ss << "fSchaetzwert = 0.1;\n\t";
    ss << "if (isNan(fSchaetzwert))\n\t\t";
    ss << "x = 0.1;\n\t";
    ss << "else\n\t\t";
#endif
    ss << "x = fSchaetzwert;\n\t";
    ss << "unsigned short nItCount = 0;\n\t";
    ss << "while (fEps > Epsilon && nItCount < 20){\n\t\t";
    ss << "nCount = 0.0;\n\t\tfZaehler = 0.0;\n\t\tfNenner = 0.0;\n\t\t";
    ss << "double arg0;\n\t\t";
    ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
#ifdef ISNAN
    ss << "for (int i = 0; i + gid0 < arrayLength &&";
    ss << " i < " << nCurWindowSize << "; i++){\n\t\t\t";
#else
    ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t\t";
#endif
    ss << "arg0 = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef  ISNAN
    ss << "if (isNan(arg0))\n\t\t\t\t";
    ss << "continue;\n\t\t\t";
#endif
    ss << "fZaehler += arg0  / pow(1.0+x, nCount);\n\t\t\t";
    ss << "fNenner  += -nCount * arg0 / pow(1.0+x,nCount+1.0);\n\t\t\t";
    ss << "nCount+=1;\n";
    ss << "\n\t\t}\n\t\t";
    ss << "xNeu = x - fZaehler / fNenner;\n\t\t";
    ss << "fEps = fabs(xNeu - x);\n\t\t";
    ss << "x = xNeu;\n\t\t";
    ss << "nItCount++;\n\t}\n\t";
    ss << "if (fSchaetzwert == 0.0 && fabs(x) < Epsilon)\n\t\t";
    ss << "x = 0.0;\n\t";
    ss << "if (fEps < Epsilon)\n\t\t";
    ss << "return x;\n\t";
    ss << "else\n\t\t";
    ss << "return (double)523;\n";
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

void OpEffective::GenSlidingWindowFunction(
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
    ss << "double nominal = " << GetBottom() <<";\n\t";
    ss << "double period = "<< GetBottom() <<";\n\t";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur0);
    FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
    formula::SingleVectorRefToken *>(tmpCur1);
    ss << "int buffer_nominal_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n\t";
    ss << "int buffer_period_len = ";
    ss << tmpCurDVR1->GetArrayLength();
    ss << ";\n\t";
    ss << "if((gid0)>=buffer_nominal_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "nominal = 0;\n\telse \n\t\t";
#endif
    ss << "nominal = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss << "if((gid0)>=buffer_period_len || isNan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << "))\n\t\t";
    ss << "period = 0;\n\telse \n\t\t";
#endif
    ss << "period = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss << "tmp = pow(1.0 + nominal / period, period)-1.0;\t";
    ss << "\n\treturn tmp;\n";
    ss << "}";
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

void OpPMT::GenSlidingWindowFunction(
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
    ss << "double tmp = 0;\n\t";
    ss<<"double tFv=0,tType=0;\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
     if(vSubArguments.size()==4)
        ss<<"tFv="<<vSubArguments[3]->GenSlidingWindowDeclRef()<<";\n\t";
    else if(vSubArguments.size()==5)
    {
        ss<<"tType="<<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
        ss<<"tFv="<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
    }
    ss<<"if("<<vSubArguments[0]->GenSlidingWindowDeclRef()<<"==0.0)\n\t";
    ss<<"\treturn ("<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"+tFv)/";
    ss<<vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n\t";
    ss<<"double abl = pow(1.0+"<<vSubArguments[0]
        ->GenSlidingWindowDeclRef();
    ss << ","<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<");\n\ttmp-=tFv";
    ss<<";\n\ttmp-="<<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<"*abl;\n\t"<<"tmp =tmp/(1.0+"<<vSubArguments[0]
        ->GenSlidingWindowDeclRef();
    ss<<"*tType";
    ss<<") / ( (abl-1.0)/"<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ");\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpPPMT::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
{
    ArgVector argVector;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
        argVector.push_back(vSubArguments[i]->GenSlidingWindowDeclRef());
    }
    ss << ") {\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss<<"double tFv=0,tType=0;\n\t";
    if(vSubArguments.size()==5)
        ss<<"tFv="<<vSubArguments[4]->GenSlidingWindowDeclRef()<<";\n\t";
    else if(vSubArguments.size()==6)
    {
        ss<<"tType="<<vSubArguments[5]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
        ss<<"tFv="<<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
    }
    ss << "double tmp = 0.0;\n\t";
    ss <<"double pmt ;\n\t";
    ss<<"if("<<vSubArguments[0]->GenSlidingWindowDeclRef()<<"==0.0)\n\t";
    ss<<"\treturn ("<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"+tFv)/";
    ss<<vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n\t";
    ss <<"double temp1=0;\n\t";
    ss <<"double abl = pow(1.0+";
    ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<",";
    ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
    ss<<");\n\t";
    ss <<"temp1-=tFv";
    ss<<";\n\t";
    ss<<"temp1-=";
    ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"*abl;\n\t";
    ss <<"pmt = temp1/(1.0+";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<"*tType";
    ss<<") / ( (abl-1.0)/";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<");\n\t";
    ss <<"double temp = pow( 1+";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<" ,";
    ss<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"-2);\n\t";
    ss <<"double re;\n\t";
    ss <<"if (tType";
    ss<<" == 0)\n\t";
    ss <<"{\n\t";
    ss <<"    re = -";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<" * ";
    ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<"*pow(1+";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<" ,";
    vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"-1) - pmt * (pow(1+";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<" ,";
    ss<<vSubArguments[1]->GenSlidingWindowDeclRef();
    ss<<"-1)-1);\n\t";
    ss <<"}\n\t";
    ss <<"else\n\t";
    ss <<"{\n\t";
    ss <<"    re = -";
    ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
    ss<<" * (";
    ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
    ss<<" + pmt)* temp - pmt * (temp-1);\n\t";
    ss <<"}\n\t";
    ss <<"tmp = pmt - re;\n\t";
    ss << "return tmp;\n";
    ss << "}";
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

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
