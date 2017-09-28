/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_logical.hxx"

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include <formula/vectortoken.hxx>
#include <sstream>

using namespace formula;

namespace sc { namespace opencl {
void OpAnd::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double t = 1,tmp=0;\n";
    for(size_t j = 0; j< vSubArguments.size(); j++)
    {
        ss << "    double tmp"<<j<<" = 1;\n";
        FormulaToken *tmpCur0 = vSubArguments[j]->GetFormulaToken();
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
        const formula::SingleVectorRefToken*pCurDVR= static_cast<const
            formula::SingleVectorRefToken *>(tmpCur0);
        ss<< "    int buffer_len"<<j<<" = "<<pCurDVR->GetArrayLength();
        ss<< ";\n";
        ss <<"    if(gid0 >= buffer_len"<<j<<" || isnan(";
        ss <<vSubArguments[j]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        tmp = 1;\n    else\n";
        ss <<"        tmp = ";
        ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
        ss <<"    tmp"<<j<<" = tmp"<<j<<" && tmp;\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss <<"        tmp = ";
            ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp"<<j<<" = tmp"<<j<<" && tmp;\n";
        }
        else if(tmpCur0->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur0);
            size_t nCurWindowSize = pCurDVR->GetArrayLength() <
            pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
            pCurDVR->GetRefRowSize() ;
            ss << "    for(int i = ";
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSize << "; i++) {\n";
            }
            else if(pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()){
            ss << "0; i < gid0 + " << nCurWindowSize << "; i++) {\n";
            }
            else{
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            }
            if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                {
            ss <<"    if(isnan("<<vSubArguments[j]->GenSlidingWindowDeclRef();
            ss <<")||i+gid0>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 1;\n    else\n";
                }
            else
                {
            ss <<"    if(isnan("<<vSubArguments[j]->GenSlidingWindowDeclRef();
            ss <<")||i>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 1;\n    else\n";
                }
            ss <<"        tmp = ";
            ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp"<<j<<" = tmp"<<j<<" && tmp;\n";
            ss <<"    }\n";
        }
        else
        {
            ss <<"        tmp"<<j<<" = ";
            ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
        }
        ss <<"    t = t && tmp"<<j<<";\n";
    }
    ss << "    return t;\n";
    ss << "}\n";
}

void OpOr::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double t = 0,tmp=0;\n";
    for(size_t j = 0; j< vSubArguments.size(); j++)
    {
        ss << "    double tmp"<<j<<" = 0;\n";
        FormulaToken *tmpCur0 = vSubArguments[j]->GetFormulaToken();
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
        const formula::SingleVectorRefToken*pCurDVR= static_cast<const
            formula::SingleVectorRefToken *>(tmpCur0);
        ss<< "    int buffer_len"<<j<<" = "<<pCurDVR->GetArrayLength();
        ss<< ";\n";
        ss <<"    if(gid0 >= buffer_len"<<j<<" || isnan(";
        ss <<vSubArguments[j]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        tmp = 0;\n    else\n";
        ss <<"        tmp = ";
        ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
        ss <<"    tmp"<<j<<" = tmp"<<j<<" || tmp;\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss <<"        tmp = ";
            ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp"<<j<<" = tmp"<<j<<" || tmp;\n";
        }
        else if(tmpCur0->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur0);
            size_t nCurWindowSize = pCurDVR->GetArrayLength() <
            pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
            pCurDVR->GetRefRowSize() ;
            ss << "    for(int i = ";
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSize << "; i++) {\n";
            }
            else if(pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()){
            ss << "0; i < gid0 + " << nCurWindowSize << "; i++) {\n";
            }
            else{
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            }
            if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                {
            ss <<"    if(isnan("<<vSubArguments[j]->GenSlidingWindowDeclRef();
            ss <<")||i+gid0>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 0;\n    else\n";
                }
            else
                {
            ss <<"    if(isnan("<<vSubArguments[j]->GenSlidingWindowDeclRef();
            ss <<")||i>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 0;\n    else\n";
                }
            ss <<"        tmp = ";
            ss <<vSubArguments[j]->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp"<<j<<" = tmp"<<j<<" || tmp;\n";
            ss <<"    }\n";
        }
        ss <<"    t = t || tmp"<<j<<";\n";
    }
    ss << "    return t;\n";
    ss << "}\n";
}
void OpNot::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    double tmp=0;\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken*pCurDVR= static_cast<const
            formula::SingleVectorRefToken *>(tmpCur0);
        ss <<"    if(gid0 >= "<<pCurDVR->GetArrayLength()<<" || isnan(";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<"))\n";
        ss <<"        tmp = 0;\n    else\n";
        ss <<"        tmp = ";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n";
        ss <<"    tmp = (tmp == 0.0);\n";
    }
    else if(tmpCur0->GetType() == formula::svDouble)
    {
        ss <<"        tmp = ";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n";
        ss <<"    tmp = (tmp == 0.0);\n";
    }
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpXor::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << "    int t = 0,tmp0 = 0;\n";
    ss << "    double tmp = 0;\n";
    for(DynamicKernelArgumentRef & rArg : vSubArguments)
    {
        FormulaToken *tmpCur0 = rArg->GetFormulaToken();
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*pCurDVR= static_cast<const
                formula::SingleVectorRefToken *>(tmpCur0);
            ss <<"    if(gid0 >= "<<pCurDVR->GetArrayLength()<<" || isnan(";
            ss <<rArg->GenSlidingWindowDeclRef();
            ss <<"))\n";
            ss <<"        tmp = 0;\n    else\n";
            ss <<"        tmp = ";
            ss <<rArg->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp0 = (tmp != 0);\n";
            ss <<"    t = t ^tmp0;\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss <<"        tmp = ";
            ss <<rArg->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp0 = (tmp != 0);\n";
            ss <<"    t = t ^tmp0;\n";
        }
        else if(tmpCur0->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur0);
            size_t nCurWindowSize = pCurDVR->GetArrayLength() <
            pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
            pCurDVR->GetRefRowSize() ;
            ss << "    for(int i = ";
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < " << nCurWindowSize << "; i++) {\n";
            }
            else if(pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()){
            ss << "0; i < gid0 + " << nCurWindowSize << "; i++) {\n";
            }
            else{
            ss << "0; i < " << nCurWindowSize << "; i++) {\n";
            }
            if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                {
            ss <<"    if(isnan("<<rArg->GenSlidingWindowDeclRef();
            ss <<")||i+gid0>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 0;\n    else\n";
                }
            else
                {
            ss <<"    if(isnan("<<rArg->GenSlidingWindowDeclRef();
            ss <<")||i>="<<pCurDVR->GetArrayLength();
            ss <<")\n";
            ss <<"        tmp = 0;\n    else\n";
                }
            ss <<"        tmp = ";
            ss <<rArg->GenSlidingWindowDeclRef()<<";\n";
            ss <<"    tmp0 = (tmp != 0);\n";
            ss <<"    t = t ^tmp0;\n";
            ss <<"    }\n";
        }
    }
    ss << "    return t;\n";
    ss << "}\n";
}
void OpIf::GenSlidingWindowFunction(std::stringstream &ss,
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

    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    if(tmpCur0->GetType() == formula::svDoubleVectorRef)
    {
        throw UnhandledToken("unknown operand for ocPush", __FILE__, __LINE__);
    }
    if(vSubArguments.size()==3)
    {
        ss << "    if(isnan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ")||  ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " == 0)\n";
        ss << "         return ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss << "     else";
        ss <<"          return ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n";
    }
    if(vSubArguments.size()==2)
    {
        ss << "    if(isnan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ")||  ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " == 0)\n";
        ss << "         return 0;\n";
        ss << "     else";
        ss <<"          return ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n";
    }
    if(vSubArguments.size()==1)
    {
        ss << "    if(isnan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ")||  ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " == 0)\n";
        ss << "         return 0;\n";
        ss << "     else";
        ss <<"          return 1;\n";
    }
    ss << "}\n";
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
