/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_math.hxx"

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

void OpCos::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << "double tmp=cos(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpCosh::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss<< "    if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
#endif
    ss << "    double tmp=cosh(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCsc::GenSlidingWindowFunction(
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
    ss << ")\n{\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
#ifdef ISNAN
    ss<< "if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
#endif
    ss << "double tmp=1/sin(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}
void OpSumIfs::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    int i ;\n";
    GenTmpVariables(ss,vSubArguments);
    ss << "    for (i = ";
     if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
        ss << "gid0; i < "<< nCurWindowSize <<"; i++)\n";
     } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
        ss << "0; i < gid0+"<< nCurWindowSize <<"; i++)\n";
     } else {
        ss << "0; i < "<< nCurWindowSize <<"; i++)\n";
     }
     ss << "    {\n";
     if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
     {
        ss<< "    int doubleIndex =i+gid0;\n";
     }else
     {
        ss<< "    int doubleIndex =i;\n";
     }
     ss<< "    int singleIndex =gid0;\n";
     int m=0;
     for(unsigned j=1;j<vSubArguments.size();j+=2,m++)
     {
        CheckSubArgumentIsNan(ss,vSubArguments,j);
        CheckSubArgumentIsNan(ss,vSubArguments,j+1);
        ss <<"    if(isequal(";
        ss <<"tmp";
        ss <<j;
        ss <<" , ";
        ss << "tmp";
        ss << j+1;
        ss << ")){\n";
     }
     CheckSubArgumentIsNan(ss,vSubArguments,0);
    ss << "    tmp += tmp0;\n";
    for(unsigned j=1;j<=vSubArguments.size();j+=2,m--)
     {
         for(int n = 0;n<m+1;n++)
        {
            ss << "    ";
        }
        ss<< "}\n";
     }
    ss << "return tmp;\n";
    ss << "}";
}
void OpSinh::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss <<") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0 = " <<
        vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
#ifdef ISNAN
    ss<< "    if(isNan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
#endif
    ss << "    double tmp=( exp(arg0)-exp(-arg0) )/2;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpSin::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = "<< vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "    if(isNan(arg0)||(gid0>=";
    ss << tmpCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
#endif
    ss << "    double tmp=sin(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpAbs::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n    else \n";
#endif
    ss << "        tmp = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    return fabs(tmp);\n";
    ss << "}";
}
void OpArcCos::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = "<< tmpCurDVR0->GetArrayLength()<< ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef()<< "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef()<< ";\n";
    ss << "    return acos(tmp);\n";
    ss << "}";
}
void OpArcCosHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength()<< ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return acosh(tmp);\n";
    ss << "}";
}
void OpTan::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = "<< vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "    if(isNan(arg0)||(gid0>=";
    ss << tmpCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
#endif
    ss << "    double tmp=tan(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpTanH::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = "<< vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "    if(isNan(arg0)||(gid0>=";
    ss << tmpCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
#endif
    ss << "    double tmp=tanh(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpSqrt::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = "<< vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
#ifdef ISNAN
    ss << "    if(isNan(arg0)||(gid0>=";
    ss << tmpCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
#endif
    ss << "    double tmp=sqrt(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpArcCot::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength()<< ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() <<";\n";
    ss << "    return M_PI_2 - atan(tmp);\n";
    ss << "}";
}
void OpArcCotHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef()<< ";\n";
    ss << "    return 0.5 * log((tmp + 1.0) / (tmp - 1.0));\n";
    ss << "}";
}
void OpArcSin::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return asin(tmp);\n";
    ss << "}";
}
void OpArcSinHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
#endif
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return asinh(tmp);\n";
    ss << "}";
}
void OpArcTan::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
#ifdef ISNAN
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isNan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else\n    ";
#endif
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return atan(tmp);\n";
    ss << "}";
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
