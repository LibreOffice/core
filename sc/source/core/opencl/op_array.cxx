/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_array.hxx"

#include <formula/vectortoken.hxx>
#include <sstream>

using namespace formula;

namespace sc { namespace opencl {

void OpSumX2MY2::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss <<"     int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenTmpVariables(ss,vSubArguments);
    if(vSubArguments[0]->GetFormulaToken()->GetType() ==
    formula::svDoubleVectorRef)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur);
        size_t nCurWindowSize = pCurDVR->GetArrayLength() <
        pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
        pCurDVR->GetRefRowSize() ;
        ss << "    int i ;\n";
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
            ss << "    int doubleIndex =i+gid0;\n";
        }else
        {
            ss << "    int doubleIndex =i;\n";
        }

        CheckSubArgumentIsNan(ss,vSubArguments,0);
        CheckSubArgumentIsNan(ss,vSubArguments,1);
        ss << "     tmp +=pow(tmp0,2) - pow(tmp1,2);\n";
        ss <<"    }\n";
    }
    else
    {
        ss << "    int singleIndex =gid0;\n";
        CheckAllSubArgumentIsNan(ss, vSubArguments);
        ss << "    tmp = pow(tmp0,2) - pow(tmp1,2);\n";
    }
    ss << "return tmp;\n";
    ss << "}";
}

void OpSumX2PY2::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenTmpVariables(ss,vSubArguments);
    if(vSubArguments[0]->GetFormulaToken()->GetType() ==
    formula::svDoubleVectorRef)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur);
        size_t nCurWindowSize = pCurDVR->GetArrayLength() <
        pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
        pCurDVR->GetRefRowSize() ;
        ss << "    int i ;\n";
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
            ss << "    int doubleIndex =i+gid0;\n";
        }else
        {
            ss << "    int doubleIndex =i;\n";
        }

        CheckSubArgumentIsNan(ss,vSubArguments,0);
        CheckSubArgumentIsNan(ss,vSubArguments,1);
        ss << "     tmp +=pow(tmp0,2) + pow(tmp1,2);\n";
        ss <<"    }\n";
    }
    else
    {
        ss << "    int singleIndex =gid0;\n";
        CheckAllSubArgumentIsNan(ss, vSubArguments);
        ss << "    tmp = pow(tmp0,2) + pow(tmp1,2);\n";
    }
    ss << "    return tmp;\n";
    ss << "}";
}
void OpSumXMY2::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenTmpVariables(ss,vSubArguments);
    if(vSubArguments[0]->GetFormulaToken()->GetType() ==
    formula::svDoubleVectorRef)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur);
        size_t nCurWindowSize = pCurDVR->GetArrayLength() <
        pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
        pCurDVR->GetRefRowSize() ;
        ss << "    int i ;\n";
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
            ss << "    int doubleIndex =i+gid0;\n";
        }else
        {
            ss << "    int doubleIndex =i;\n";
        }

        CheckSubArgumentIsNan(ss,vSubArguments,0);
        CheckSubArgumentIsNan(ss,vSubArguments,1);
        ss << "     tmp +=pow((tmp0-tmp1),2);\n";
        ss <<"    }\n";
    }
    else
    {
        ss << "    int singleIndex =gid0;\n";
        CheckAllSubArgumentIsNan(ss, vSubArguments);
        ss << "    tmp = pow((tmp0-tmp1),2);\n";
    }
    ss << "    return tmp;\n";
    ss << "}";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
