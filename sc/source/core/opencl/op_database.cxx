/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_database.hxx"

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"
#include <sstream>


namespace sc { namespace opencl {

void OpDmax::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double max = -1000000000000;\n";
    ss << "    double value=0.0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() ==
    formula::svDoubleVectorRef)
    {
        formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= dynamic_cast<const
            formula::DoubleVectorRefToken *>(tmpCur);
        dataCol = pCurDVR1->GetArrays().size();
        dataRow = pCurDVR1->GetArrayLength();
    }
    else
        throw Unhandled();
    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() ==
     formula::svSingleVectorRef)
     {
         const formula::SingleVectorRefToken*pTmpDVR1= dynamic_cast<const
         formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
         GetFormulaToken());
         ss << "    tmp"<<dataCol<<"=";
         ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
         ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isNan(";
         ss << "tmp"<<dataCol<<"))\n";
         ss << "        tmp"<<dataCol<<"=0;\n";
     }
    else
        throw Unhandled();
    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() ==
    formula::svDoubleVectorRef)
    {
        formula::FormulaToken *tmpCur = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR2= dynamic_cast<const
            formula::DoubleVectorRefToken *>(tmpCur);
        conditionCol = pCurDVR2->GetArrays().size();
        conditionRow = pCurDVR2->GetArrayLength();
    }
    else{
        throw Unhandled();
    }
    if(dataCol!=conditionCol)
            throw Unhandled();
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= dynamic_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= dynamic_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";
            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled();
                formula::FormulaToken *tmpCur_tmp =
                    vSubArguments[i]->GetFormulaToken();
                const formula::DoubleVectorRefToken*pCurDVR_tmp=
                    dynamic_cast<const formula::DoubleVectorRefToken *>
                    (tmpCur_tmp);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isNan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled();
                formula::FormulaToken *tmpCur_tmp =
                    vSubArguments[i]->GetFormulaToken();
                const formula::DoubleVectorRefToken*pCurDVR_tmp=
                    dynamic_cast<const formula::DoubleVectorRefToken *>
                    (tmpCur_tmp);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isNan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            if(max<value)\n";
            ss << "                max=value;";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "max = -1;\n";
    }
    else
        ss << "max = -1;\n";
    ss << "    return max;\n";
    ss << "}";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
